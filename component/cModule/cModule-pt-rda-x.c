/*
 * Copyright (C) 2018 Flandreunx@outlook.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
#include "global.h"

/**
 * @addtogroup Debug support
 * @note none
 */
 
/*@{*/

#undef DBG_SECTION_NAME
#define DBG_SECTION_NAME  "cModule-pt-rda-x"

/*@}*/

#if CONFIG_CMODULE_PROTOCOL_IS_RDA_1_AT == 1
 
/**
 * @addtogroup ComponentSupport
 * @note none
 */
 
/*@{*/

#define _MODULE_REQUEST_TYPE(x)           CMODULE_REQ_PT_##x
#define _MODULE_REQUEST_VARS              cModule_RequestVar_t
#define _MODULE_REQUEST_TYPED             cModule_RequestType_t
#define _MODULE_REQUEST_CALL              cModule_onRequestCallback
#define CREQUEST(type, var, ...)        _MODULE_REQUEST_CALL((uint32_t) _MODULE_REQUEST_TYPE(type), (_MODULE_REQUEST_VARS) var, ##__VA_ARGS__)
extern _MODULE_REQUEST_VARS _MODULE_REQUEST_CALL(_MODULE_REQUEST_TYPED type, _MODULE_REQUEST_VARS var, ...);

/*@}*/

/**
 * @addtogroup ProtocolRequest
 * @note none
 */

/*@{*/

int32_t cModule_protocolTransmit_control(cModule_Instance_t *ins, ProtocolRda1_t *protocol, bool forceWrite) {
    if (!_cModule_isAllocReqPackAccess(ins)) {
        return -1;
    }
    
    int32_t rc = -1;
    char *payload = calloc(1, 32);
    ASSERT(payload != NULL);
    rc = klPtf_sprintf(payload, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", 0,
        protocol->value, protocol->state, protocol->unit, protocol->type, 
        protocol->alarmL, protocol->alarmH, 
        protocol->updateTimeMin, protocol->temp,
        protocol->valueCo, protocol->auxCoLtm);
    ASSERT(rc <= 31);

    _cModule_packMutexLock(ins, true);
    cModule_TransmitPackageInfo_t *pack = calloc(1, sizeof(cModule_TransmitPackageInfo_t));
    ASSERT(pack != NULL);
    pack->payloadLength = strlen(payload);
    pack->payload = payload;
    pack->flag.packIsDynData = 1;
    pack->flag.requestId = (uint32_t) CMODULE_TPACK_RID_PT_AT_RDA1_REPORT;
    pack->aux.gen.msgId = (*_cModule_getGenMsgId())++;
    pack->aux.gen.sendRetryCount = 1;
    pack->aux.gen.sendRetryCountReload = 1;
    pack->aux.gen.sendFailedAfterReboot = 2;
    pack->aux.gen.sendRetrySec = 3;
    pack->aux.gen.hasConfirmFrame = 0;
    pack->aux.gen.waitConfirmFrameTimeoutSec = 0;
    pack->aux.gen.waitConfirmFrameTimeoutCount = 0;
    pack->aux.gen.waitConfirmFrameTimeoutAfterReboot = 0;
    klist_init(&pack->list);
    klist_addTail(&ins->transmit.packageReqList, &pack->list);
    ins->transmit.queneReqCount++;
    _cModule_packMutexLock(ins, false);
    
    free(protocol);
    
    return pack->aux.gen.msgId;
}

/*@}*/

/**
 * @addtogroup ProtocolReceived
 * @note none
 */

/*@{*/

int32_t _cModule_onProtocolTransmited(cModule_Instance_t *ins, cModule_TransmitPackageInfo_t *info) {
    return 0;
}


int32_t _cModule_onProtocolReceived(cModule_Instance_t *ins, cModule_TransmitPackageInfo_t *info) {
    if (strstr(info->payload, "4")) {
        CREQUEST(ON_SELF_TEST, {});
    } else if (strstr(info->payload, "3")) {
        CREQUEST(ON_ALARM_SILENCE, {});
    } else if (strstr(info->payload, "2")) {
        CREQUEST(ON_RESET, {});
    } else if (strstr(info->payload, "1")) {
        CREQUEST(ON_OPERATE_VALVE, {});
    } 
    return 0;
}

/*@}*/

/**
 * @addtogroup Public
 * @note none
 */
 
/*@{*/

void cModule_protocol_init(void) {
}

/*@}*/

#endif

#if CONFIG_CMODULE_PROTOCOL_IS_RDA_MQTT_MSK4G == 1
 
/**
 * @addtogroup ComponentSupport
 * @note none
 */
 
/*@{*/

#define _MODULE_REQUEST_TYPE(x)           CMODULE_REQ_PT_##x
#define _MODULE_REQUEST_VARS              cModule_RequestVar_t
#define _MODULE_REQUEST_TYPED             cModule_RequestType_t
#define _MODULE_REQUEST_CALL              cModule_onRequestCallback
#define CREQUEST(type, var, ...)        _MODULE_REQUEST_CALL((uint32_t) _MODULE_REQUEST_TYPE(type), (_MODULE_REQUEST_VARS) var, ##__VA_ARGS__)
extern _MODULE_REQUEST_VARS _MODULE_REQUEST_CALL(_MODULE_REQUEST_TYPED type, _MODULE_REQUEST_VARS var, ...);

/*@}*/

/**
 * @addtogroup ProtocolConnectDefine
 * @note none
 */

/*@{*/

#define RDA_MQTT_BORKER_ADDRESS             "rda3000.rynon.com.cn"
#ifdef RDA_MQTT_BORKER_ADDRESS

#define RDA_MQTT_PRODUCT_KEY                "standard-config"

#define RDA_MQTT_SUB_TOPIC_VAR_WRITE        ""RDA_MQTT_PRODUCT_KEY"/%s/var_write"
#define RDA_MQTT_SUB_TOPIC_DOWN             ""RDA_MQTT_PRODUCT_KEY"/%s/down"
#define RDA_MQTT_SUB_TOPIC_CONFIG           ""RDA_MQTT_PRODUCT_KEY"/%s/config"
static const char *SUB_TOPIC[] = {
    RDA_MQTT_SUB_TOPIC_VAR_WRITE,
    RDA_MQTT_SUB_TOPIC_DOWN,
    RDA_MQTT_SUB_TOPIC_CONFIG,
};
#endif

/*@}*/

/**
 * @addtogroup Define
 * @note none
 */
 
/*@{*/

#define DIRECT_WRITE      rilat_directWritePhy(instance, (uint8_t *) &buffer[0], len); // LOG_D("%s", buffer);
#define HEX_WRITE         memset(&buffer[128], 0x00, 128); len = klStr_hex2str((uint8_t *) buffer, len, &buffer[128]); rilat_directWritePhy(instance, (uint8_t *) &buffer[128], len); // LOG_I("HEX-> %s", buffer);


#define RDA_MQTT_PRODUCT_KEY                "standard-config"
#define RDA_MQTT_PUB_TOPIC_DATA             ""RDA_MQTT_PRODUCT_KEY"/%s/data"
#define RDA_MQTT_PUB_TOPIC_UP               ""RDA_MQTT_PRODUCT_KEY"/%s/up"
#define RDA_MQTT_PUB_TOPIC_VAR_WRITE_ACK    ""RDA_MQTT_PRODUCT_KEY"/%s/var_write_ack"

typedef struct {
    cModule_Instance_t *ins;
    ProtocolRda1_t *protocol;
    bool isWriten: 1;
    uint16_t messageId;
    klist_t list;
} PtBufferCollection_t;

/*@}*/

/**
 * @addtogroup Vars
 * @note none
 */

/*@{*/

static klist_t mPtBufferList;

/*@}*/

/**
 * @addtogroup ProtocolPrivate
 * @note none
 */

/*@{*/

static int32_t ack(cModule_Instance_t *ins, uint8_t value, char *messageId, char *devicesId) {
    int32_t rc = -1;
    cModule_ProtocolMqttMessage_t *msg = calloc(1, sizeof(cModule_ProtocolMqttMessage_t));
    ASSERT(msg != NULL);

    char *buffer = calloc(1, 128);
    ASSERT(buffer != NULL);
    uint16_t len = 0;
    len += klPtf_sprintf(&buffer[len], "{");
        len += klPtf_sprintf(&buffer[len], "\"message_id\":\"%s\",", messageId);
        len += klPtf_sprintf(&buffer[len], "\"devices\":{");
            len += klPtf_sprintf(&buffer[len], "\"%s\":{", devicesId);
                len += klPtf_sprintf(&buffer[len], "\"variables\":{");
                    len += klPtf_sprintf(&buffer[len], "\"control\":{");
                        len += klPtf_sprintf(&buffer[len], "\"value\":\"%d\",", value);
                        len += klPtf_sprintf(&buffer[len], "\"result\":\"success\"");
                    len += klPtf_sprintf(&buffer[len], "}");
                len += klPtf_sprintf(&buffer[len], "}");
            len += klPtf_sprintf(&buffer[len], "}");
        len += klPtf_sprintf(&buffer[len], "}");
    len += klPtf_sprintf(&buffer[len], "}");
    ASSERT(len <= 128);
    msg->payload = buffer;
    msg->len = len;
    
    char *topic = calloc(1, strlen(RDA_MQTT_PUB_TOPIC_VAR_WRITE_ACK) + strlen(ins->modemInfo.imei) + 1);
    ASSERT(topic != NULL);
    klPtf_sprintf(topic, RDA_MQTT_PUB_TOPIC_VAR_WRITE_ACK, ins->modemInfo.imei);
    msg->topic = topic;
    msg->topicConstant = false;
    msg->qos = 1;
    msg->isPayloadString = true;
    
    msg->timeout = 15000;
    
    _cModule_packMutexLock(ins, true);
    cModule_TransmitPackageInfo_t *pack = calloc(1, sizeof(cModule_TransmitPackageInfo_t));
    ASSERT(pack != NULL);
    pack->payloadLength = sizeof(cModule_ProtocolMqttMessage_t);
    pack->payload = msg;
    pack->flag.packIsDynMqttMsg = 1;
    pack->flag.requestId = TRANSMIT_PACK_REQ_ID_TX;
    pack->aux.gen.msgId = *_cModule_getGenMsgId();
    pack->aux.gen.sendRetryCount = 1;
    pack->aux.gen.sendRetryCountReload = 1;
    pack->aux.gen.sendFailedAfterReboot = 2;
    pack->aux.gen.sendRetrySec = 3;
    pack->aux.gen.hasConfirmFrame = 0;
    pack->aux.gen.waitConfirmFrameTimeoutSec = 0;
    pack->aux.gen.waitConfirmFrameTimeoutCount = 0;
    pack->aux.gen.waitConfirmFrameTimeoutAfterReboot = 0;
    klist_init(&pack->list);
    klist_addTail(&ins->transmit.packageReqList, &pack->list);
    ins->transmit.queneReqCount++;
    _cModule_packMutexLock(ins, false);
    return 0;
}


static void directWrite(Rilat_Instance_t *instance, uint32_t packMsgId) {
    klist_t *pos = NULL, *tmp = NULL;
    klist_forEachSafe(pos, tmp, &mPtBufferList) {
        PtBufferCollection_t *ud = klist_entry(pos, PtBufferCollection_t, list);
        if (packMsgId != ud->messageId) {
            continue;
        }
        
        char *buffer = calloc(1, 64);
        ASSERT(buffer != NULL);
        uint16_t len = 0;

        len = klPtf_sprintf(&buffer[0], "{"); DIRECT_WRITE;
            len = klPtf_sprintf(&buffer[0], "\"rssi\":%03d,", ud->ins->modemInfo.rssi); DIRECT_WRITE;
            len = klPtf_sprintf(&buffer[0], "\"iccid\":\"%s\",", ud->ins->modemInfo.iccid); DIRECT_WRITE;
            len = klPtf_sprintf(&buffer[0], "\"updateTime\":%d,", ud->protocol->updateTimeMin); DIRECT_WRITE;
            len = klPtf_sprintf(&buffer[0], "\"powerDown\":%d,", ud->protocol->isPowerDown); DIRECT_WRITE;
            len = klPtf_sprintf(&buffer[0], "\"devices\":{"); DIRECT_WRITE;
                len = klPtf_sprintf(&buffer[0], "\"%s\":{", ud->ins->modemInfo.imei); DIRECT_WRITE;
                    len = klPtf_sprintf(&buffer[0], "\"comm_status\":%d,", ud->protocol->isComFault); DIRECT_WRITE;
                    len = klPtf_sprintf(&buffer[0], "\"variables\":{"); DIRECT_WRITE;
                        len = klPtf_sprintf(&buffer[0], "\"mid\":%d,", packMsgId); DIRECT_WRITE;
                        len = klPtf_sprintf(&buffer[0], "\"GasStatus\":%d,", ud->protocol->state); DIRECT_WRITE;
                        if (ud->protocol->type == PROTOCOL_RDA_TYPE_CO) {
                            len = klPtf_sprintf(&buffer[0], "\"measuredValue\":%d,", ud->protocol->valueCo); DIRECT_WRITE;
                        } else {
                            len = klPtf_sprintf(&buffer[0], "\"measuredValue\":%d,", ud->protocol->value); DIRECT_WRITE;
                        }
                        len = klPtf_sprintf(&buffer[0], "\"AlarmL\":%d,", ud->protocol->alarmL); DIRECT_WRITE;
                        len = klPtf_sprintf(&buffer[0], "\"Range\":100,"); DIRECT_WRITE;
                        len = klPtf_sprintf(&buffer[0], "\"Decimals\":0,"); DIRECT_WRITE;
                        len = klPtf_sprintf(&buffer[0], "\"AlarmH\":%d,", ud->protocol->alarmH); DIRECT_WRITE;
                        len = klPtf_sprintf(&buffer[0], "\"Unit\":%d,", ud->protocol->unit); DIRECT_WRITE;
                        
                        len = klPtf_sprintf(&buffer[0], "\"temp\":%d,", ud->protocol->temp); DIRECT_WRITE;
                        if (ud->protocol->type == PROTOCOL_RDA_TYPE_CO) {
                            len = klPtf_sprintf(&buffer[0], "\"auxCoLtm\":%d,", ud->protocol->auxCoLtm); DIRECT_WRITE;
                        }
                        
                        len = klPtf_sprintf(&buffer[0], "\"GasType\":%d", ud->protocol->type); DIRECT_WRITE;
                        if (ud->protocol->doxEnableCount != 0) {
                            len = klPtf_sprintf(&buffer[0], ",", ud->protocol->type); DIRECT_WRITE;
                            for (uint8_t doxIdx = 0; doxIdx < ud->protocol->doxEnableCount; doxIdx++) {
                                len = klPtf_sprintf(&buffer[0], "\"DO%d\":%d,", doxIdx + 1, ud->protocol->dox[doxIdx].sw); DIRECT_WRITE;
                                if (ud->protocol->dox[doxIdx].macStr != NULL) {
                                    len = klPtf_sprintf(&buffer[0], "\"DO%dMAC\":\"%s\",", doxIdx + 1, ud->protocol->dox[doxIdx].macStr); DIRECT_WRITE;
                                }
                                len = klPtf_sprintf(&buffer[0], "\"do%dStatus\":%d", doxIdx + 1, ud->protocol->dox[doxIdx].st); DIRECT_WRITE;
                                if (doxIdx != ud->protocol->doxEnableCount - 1) {
                                    len = klPtf_sprintf(buffer, ","); DIRECT_WRITE;
                                }
                            }
                        }
                    len = klPtf_sprintf(&buffer[0], "}"); DIRECT_WRITE;
                len = klPtf_sprintf(&buffer[0], "}"); DIRECT_WRITE;
            len = klPtf_sprintf(&buffer[0], "}"); DIRECT_WRITE;
        len = klPtf_sprintf(&buffer[0], "}"); DIRECT_WRITE;

        ud->isWriten = true;
        free(buffer);
        break;
    } 
}


static uint32_t directGetPayloadLength(Rilat_Instance_t *instance, uint32_t packMsgId) {
    klist_t *pos = NULL, *tmp = NULL;
    klist_forEachSafe(pos, tmp, &mPtBufferList) {
        PtBufferCollection_t *ud = klist_entry(pos, PtBufferCollection_t, list);
        if (packMsgId != ud->messageId) {
            continue;
        }
        
        char *buffer = calloc(1, 64);
        ASSERT(buffer != NULL);
        uint16_t len = 0;

        len += klPtf_sprintf(&buffer[0], "{");
            len += klPtf_sprintf(&buffer[0], "\"rssi\":%03d,", ud->ins->modemInfo.rssi);
            len += klPtf_sprintf(&buffer[0], "\"iccid\":\"%s\",", ud->ins->modemInfo.iccid);
            len += klPtf_sprintf(&buffer[0], "\"updateTime\":%d,", ud->protocol->updateTimeMin);
            len += klPtf_sprintf(&buffer[0], "\"powerDown\":%d,", ud->protocol->isPowerDown);
            len += klPtf_sprintf(&buffer[0], "\"devices\":{");
                len += klPtf_sprintf(&buffer[0], "\"%s\":{", ud->ins->modemInfo.imei);
                    len += klPtf_sprintf(&buffer[0], "\"comm_status\":%d,", ud->protocol->isComFault);
                    len += klPtf_sprintf(&buffer[0], "\"variables\":{");
                        len += klPtf_sprintf(&buffer[0], "\"mid\":%d,", packMsgId);
                        len += klPtf_sprintf(&buffer[0], "\"GasStatus\":%d,", ud->protocol->state);
                        if (ud->protocol->type == PROTOCOL_RDA_TYPE_CO) {
                            len += klPtf_sprintf(&buffer[0], "\"measuredValue\":%d,", ud->protocol->valueCo);
                        } else {
                            len += klPtf_sprintf(&buffer[0], "\"measuredValue\":%d,", ud->protocol->value);
                        }
                        len += klPtf_sprintf(&buffer[0], "\"AlarmL\":%d,", ud->protocol->alarmL);
                        len += klPtf_sprintf(&buffer[0], "\"Range\":100,");
                        len += klPtf_sprintf(&buffer[0], "\"Decimals\":0,");
                        len += klPtf_sprintf(&buffer[0], "\"AlarmH\":%d,", ud->protocol->alarmH);
                        len += klPtf_sprintf(&buffer[0], "\"Unit\":%d,", ud->protocol->unit);
                        
                        len += klPtf_sprintf(&buffer[0], "\"temp\":%d,", ud->protocol->temp);
                        if (ud->protocol->type == PROTOCOL_RDA_TYPE_CO) {
                            len += klPtf_sprintf(&buffer[0], "\"auxCoLtm\":%d,", ud->protocol->auxCoLtm);
                        }
                        
                        len += klPtf_sprintf(&buffer[0], "\"GasType\":%d", ud->protocol->type);
                        if (ud->protocol->doxEnableCount != 0) {
                            len += klPtf_sprintf(&buffer[0], ",", ud->protocol->type);
                            for (uint8_t doxIdx = 0; doxIdx < ud->protocol->doxEnableCount; doxIdx++) {
                                len += klPtf_sprintf(&buffer[0], "\"DO%d\":%d,", doxIdx + 1, ud->protocol->dox[doxIdx].sw);
                                if (ud->protocol->dox[doxIdx].macStr != NULL) {
                                    len += klPtf_sprintf(&buffer[0], "\"DO%dMAC\":\"%s\",", doxIdx + 1, ud->protocol->dox[doxIdx].macStr);
                                }
                                len += klPtf_sprintf(&buffer[0], "\"do%dStatus\":%d", doxIdx + 1, ud->protocol->dox[doxIdx].st);
                                if (doxIdx != ud->protocol->doxEnableCount - 1) {
                                    len += klPtf_sprintf(buffer, ",");
                                }
                            }
                        }
                    len += klPtf_sprintf(&buffer[0], "}");
                len += klPtf_sprintf(&buffer[0], "}");
            len += klPtf_sprintf(&buffer[0], "}");
        len += klPtf_sprintf(&buffer[0], "}");

        free(buffer);
                    
        return len;
    } 
    
    return 0;
}


static void directWriteResponse(Rilat_Instance_t *instance, uint32_t packMsgId, bool isSuccess, bool isForce) {
    PtBufferCollection_t *ud = NULL;
    klist_t *pos = NULL, *tmp = NULL;
    klist_forEachSafe(pos, tmp, &mPtBufferList) {
        ud = klist_entry(pos, PtBufferCollection_t, list);
        if (isForce) {
            klist_delete(&ud->list);
            free(ud->protocol);
            free(ud);
            ud = NULL;
            continue;
        } 
        
        if (!ud->isWriten) {
            ud = NULL;
            continue;
        }
        if (packMsgId != ud->messageId) {
            ud = NULL;
            continue;
        }
        klist_delete(&ud->list);
        break;
    }
    if (ud != NULL) {
        uint16_t messageId = ud->messageId;
        free(ud->protocol);
        free(ud);
        CREQUEST(ON_TRANSMIT_SUCCESS, {.u16[0] = messageId, .u16[1] = isSuccess});
    }
}

/*@}*/

/**
 * @addtogroup ProtocolRequest
 * @note none
 */

/*@{*/

int32_t cModule_protocolTransmit_control(cModule_Instance_t *ins, ProtocolRda1_t *protocol, bool forceWrite) {
    if (!_cModule_isAllocReqPackAccess(ins) && !forceWrite) {
        return -1;
    }
    
    int32_t rc = -1;
    cModule_ProtocolMqttMessage_t *msg = calloc(1, sizeof(cModule_ProtocolMqttMessage_t));
    ASSERT(msg != NULL);
    msg->payload = NULL;
    msg->qos = 1;
    msg->topic = calloc(1, strlen(RDA_MQTT_PUB_TOPIC_DATA) + strlen(ins->modemInfo.imei) + 1);
    ASSERT(msg->topic != NULL);
    klPtf_sprintf(msg->topic, RDA_MQTT_PUB_TOPIC_DATA, ins->modemInfo.imei);
    msg->topicConstant = false;
    msg->writer.onDirectWrite = directWrite;
    msg->writer.onDorectGetLength = directGetPayloadLength;
    msg->writer.onDirectWriteResponse = directWriteResponse;
    msg->isPayloadString = true;
    msg->timeout = forceWrite ? 0 : 15000;
    
    PtBufferCollection_t *ud = calloc(1, sizeof(PtBufferCollection_t));
    ASSERT(ud != NULL);
    klist_init(&ud->list);
    ud->ins = ins;
    ud->protocol = protocol;
    ud->messageId = (*_cModule_getGenMsgId())++;
    if (forceWrite) {
        klist_add(&mPtBufferList, &ud->list);
    } else {
        klist_addTail(&mPtBufferList, &ud->list);
    }
    
    cModule_TransmitPackageInfo_t *pack = calloc(1, sizeof(cModule_TransmitPackageInfo_t));
    ASSERT(pack != NULL);
    pack->payloadLength = sizeof(cModule_ProtocolMqttMessage_t);
    pack->payload = msg;
    pack->flag.packIsDynMqttMsg = 1;
    pack->flag.requestId = TRANSMIT_PACK_REQ_ID_TX;
    pack->aux.gen.msgId = ud->messageId;
    pack->aux.gen.sendRetryCountReload = 1;
    pack->aux.gen.sendFailedAfterReboot = 2;
    pack->aux.gen.sendRetrySec = 3;
    pack->aux.gen.hasConfirmFrame = 0;
    pack->aux.gen.waitConfirmFrameTimeoutSec = 0;
    pack->aux.gen.waitConfirmFrameTimeoutCount = 0;
    pack->aux.gen.waitConfirmFrameTimeoutAfterReboot = 0;
    
    if (!forceWrite) {
        _cModule_packMutexLock(ins, true);
        pack->aux.gen.sendRetryCount = 1;
        klist_init(&pack->list);
        klist_addTail(&ins->transmit.packageReqList, &pack->list);
        ins->transmit.queneReqCount++;
        _cModule_packMutexLock(ins, false);
    } else {
        ins->callback->onPtPackIdTransmit(pack);
        ins->callback->onPtPackPayloadFree(pack, false);
        free(msg);
        free(pack);
    }
    
    return pack->aux.gen.msgId;
}

/*@}*/

/**
 * @addtogroup ProtocolReceived
 * @note none
 */

/*@{*/

static int32_t getFieldValueInt(void *data, const char *fieldIdStr) {
    int32_t v = 0;
    
    char *str = strstr((char *) data, fieldIdStr);
    if (str == NULL) {
        return INT16_MIN;
    }
    str += strlen(fieldIdStr);
    while (*str != '\0' && *str != ':') {
        (str)++;
    }
    str++;
    v = atoi(str);
    return v;
}

static int32_t getFieldValueString(void *data, const char *fieldIdStr, char **out) {
    char *str = strstr((char *) data, fieldIdStr);
    if (str == NULL) {
        return -1;
    }
    str += strlen(fieldIdStr);
    while (*str != '\0' && *str != '\"') {
        (str)++;
    }
    str++;
    char *end = str;
    while (*end != '\0' && *end != '\"') {
        (end)++;
    }
    *out = calloc(1, (uint32_t) end - (uint32_t) str + 1);
    ASSERT(*out != NULL);
    memcpy(*out, str, (uint32_t) end - (uint32_t) str);
    return 0;
}

void _cModule_onProtocolMqttConnectAlloc(cModule_Instance_t *ins, 
        char **borkerIp,
        char **clientId, 
        char **userName, char **password) {
    *password = calloc(1, 32 + 1);
    ASSERT(*password != NULL);
}

void _cModule_onProtocolMqttConnect(cModule_Instance_t *ins, 
        char **borkerIp, uint16_t *port,
        char **clientId, 
        char **userName, char **password) {
    uint8_t *raw = calloc(1, strlen(ins->modemInfo.imei) * 2 + 1);
    ASSERT(raw != NULL);
    memcpy(raw, ins->modemInfo.imei, 15);
    memcpy(raw + 15, ins->modemInfo.imei, 15);
            
    size_t base64Size = 0;
    mbedtls_base64_encode(NULL, 0, &base64Size, raw, strlen(ins->modemInfo.imei) * 2);

    char *base64Str = calloc(1, base64Size + 1);
    ASSERT(base64Str != NULL);
    
    mbedtls_base64_encode((uint8_t *) base64Str, base64Size, NULL, raw, strlen(ins->modemInfo.imei) * 2);
    
    uint8_t *md5 = calloc(1, 16);
    ASSERT(md5 != NULL);
    mbedtls_md5_context *md5Ctx = calloc(1, sizeof(mbedtls_md5_context));
    ASSERT(md5Ctx != NULL);
    mbedtls_md5_init(md5Ctx);
    mbedtls_md5_starts(md5Ctx);
    mbedtls_md5_update(md5Ctx, (uint8_t *) base64Str, strlen(base64Str));
    mbedtls_md5_finish(md5Ctx, md5);
    mbedtls_md5_free(md5Ctx);
    free(md5Ctx);
    free(base64Str);
    free(raw);
    
    klStr_hex2str(md5, 16, *password);
    free(md5);
    
    *borkerIp = RDA_MQTT_BORKER_ADDRESS;
    *port = 1883;
    *clientId = ins->modemInfo.imei;
    *userName = ins->modemInfo.imei;
}
        
void _cModule_onProtocolMqttConnectFree(cModule_Instance_t *ins, 
        char **borkerIp,
        char **clientId, 
        char **userName, char **password) {
    free(*password);
}    

uint8_t _cModule_onProtocolMqttSubCount(cModule_Instance_t *ins) {
    return LWMEM_ARRAYSIZE(SUB_TOPIC);
}

void _cModule_onProtocolMqttSubAlloc(cModule_Instance_t *ins, char **topic, uint8_t index) {
    *topic = calloc(1, 64);
    ASSERT(topic);
}

void _cModule_onProtocolMqttSub(cModule_Instance_t *ins, char **topic, uint8_t *qos, uint8_t index) {
    *qos = 1;
    klPtf_sprintf(*topic, SUB_TOPIC[index], ins->modemInfo.imei);
}

void _cModule_onProtocolMqttSubFree(cModule_Instance_t *ins, char **topic, uint8_t index) {
    free(*topic);
}

int32_t _cModule_onProtocolTransmited(cModule_Instance_t *ins, cModule_TransmitPackageInfo_t *info) {
    return 0;
}

int32_t _cModule_onProtocolReceived(cModule_Instance_t *ins, cModule_TransmitPackageInfo_t *info) {
#if CONFIG_CMODULE_PROTOCOL_IS_RDA_MQTT_MSK4G == 1
    static const char *FIELD_MESSAGE_ID = "\"message_id\"";
    static const char *FIELD_CONTROL = "\"control\"";
    static const char *FIELD_DOX = "\"DO";

    char *messageIdStr = NULL;
    if (getFieldValueString(info->payload, FIELD_MESSAGE_ID, &messageIdStr) != 0) {
        return -1;
    }
    int16_t control = getFieldValueInt(info->payload, FIELD_CONTROL);
    
    ack(ins, control, messageIdStr, cModule_getModuleUId(ins));
    free(messageIdStr);

    char *doxStr = NULL;
    if (getFieldValueString(info->payload, FIELD_DOX, &doxStr) == 0) {
        uint32_t dox = atoi(doxStr + strlen(FIELD_DOX));
        if (dox <= 9) {
            int16_t doxVl = getFieldValueInt(info->payload, FIELD_DOX);
            if (doxVl != INT16_MIN) {
                CREQUEST(ON_OPERATE_VALVE_NUM, {.u16[0] = dox, .u16[1] = doxVl});
            }
        }
    }
    free(doxStr);

    if (control == 4) {
        CREQUEST(ON_SELF_TEST, {});
    } else if (control == 3) {
        CREQUEST(ON_ALARM_SILENCE, {});
    } else if (control == 2) {
        CREQUEST(ON_RESET, {});
    } else if (control == 1) {
        CREQUEST(ON_OPERATE_VALVE, {});
    } 
    return 0;
#endif
    return -1;
}

/*@}*/

/**
 * @addtogroup Public
 * @note none
 */
 
/*@{*/

void cModule_protocol_init(void) {
    klist_init(&mPtBufferList);
}

/*@}*/

#endif

#if CONFIG_CMODULE_PROTOCOL_IS_RDA_MQTT_CTLX == 1

/**
 * @addtogroup ComponentSupport
 * @note none
 */
 
/*@{*/

#define _MODULE_REQUEST_TYPE(x)           CMODULE_REQ_PT_##x
#define _MODULE_REQUEST_VARS              cModule_RequestVar_t
#define _MODULE_REQUEST_TYPED             cModule_RequestType_t
#define _MODULE_REQUEST_CALL              cModule_onRequestCallback
#define CREQUEST(type, var, ...)        _MODULE_REQUEST_CALL((uint32_t) _MODULE_REQUEST_TYPE(type), (_MODULE_REQUEST_VARS) var, ##__VA_ARGS__)
extern _MODULE_REQUEST_VARS _MODULE_REQUEST_CALL(_MODULE_REQUEST_TYPED type, _MODULE_REQUEST_VARS var, ...);

/*@}*/

/**
 * @addtogroup ProtocolConnectDefine
 * @note none
 */

/*@{*/

#define RDA_MQTT_BORKER_ADDRESS             "rda3000.rynon.com.cn"
#ifdef RDA_MQTT_BORKER_ADDRESS

#define RDA_MQTT_PRODUCT_KEY                "standard-config"

#define RDA_MQTT_SUB_TOPIC_VAR_WRITE        ""RDA_MQTT_PRODUCT_KEY"/%s/var_write"
#define RDA_MQTT_SUB_TOPIC_DOWN             ""RDA_MQTT_PRODUCT_KEY"/%s/down"
#define RDA_MQTT_SUB_TOPIC_CONFIG           ""RDA_MQTT_PRODUCT_KEY"/%s/config"
static const char *SUB_TOPIC[] = {
    RDA_MQTT_SUB_TOPIC_VAR_WRITE,
    RDA_MQTT_SUB_TOPIC_DOWN,
    RDA_MQTT_SUB_TOPIC_CONFIG,
};
#endif

/*@}*/

/**
 * @addtogroup Define
 * @note none
 */
 
/*@{*/

#define DIRECT_WRITE      rilat_directWritePhy(instance, (uint8_t *) &buffer[0], len); LOG_D("%s", buffer);

#define RDA_MQTT_PRODUCT_KEY                "standard-config"
#define RDA_MQTT_PUB_TOPIC_DATA             ""RDA_MQTT_PRODUCT_KEY"/%s/data"
#define RDA_MQTT_PUB_TOPIC_UP               ""RDA_MQTT_PRODUCT_KEY"/%s/up"
#define RDA_MQTT_PUB_TOPIC_VAR_WRITE_ACK    ""RDA_MQTT_PRODUCT_KEY"/%s/var_write_ack"

typedef struct {
    cModule_Instance_t *ins;
    ProtocolRdaX_t *protocol;
    uint16_t sensorCount;
    bool isWriten: 1;
    uint16_t messageId;
    klist_t list;
} PtBufferCollection_t;

/*@}*/

/**
 * @addtogroup Vars
 * @note none
 */

/*@{*/

static klist_t mPtBufferList;

/*@}*/

/**
 * @addtogroup ProtocolPrivate
 * @note none
 */

/*@{*/

static void directWrite(Rilat_Instance_t *instance, uint32_t packMsgId) {
    klist_t *pos = NULL, *tmp = NULL;
    klist_forEachSafe(pos, tmp, &mPtBufferList) {
        PtBufferCollection_t *ud = klist_entry(pos, PtBufferCollection_t, list);
        if (packMsgId != ud->messageId) {
            continue;
        }
        
        char *buffer = calloc(1, 64);
        ASSERT(buffer != NULL);
        uint16_t len = 0;

        len = klPtf_sprintf(&buffer[0], "{"); DIRECT_WRITE;
            len = klPtf_sprintf(&buffer[0], "\"iccid\":\"%s\",", ud->ins->modemInfo.iccid); DIRECT_WRITE;
            len = klPtf_sprintf(&buffer[0], "\"rssi\":%03d,", ud->ins->modemInfo.rssi); DIRECT_WRITE;
            len = klPtf_sprintf(&buffer[0], "\"devices\":{"); DIRECT_WRITE;
                len = klPtf_sprintf(&buffer[0], "\"1\":{"); DIRECT_WRITE;
                    len = klPtf_sprintf(&buffer[0], "\"comm_status\":1,"); DIRECT_WRITE;
                    len = klPtf_sprintf(&buffer[0], "\"variables\":{"); DIRECT_WRITE;
                        for (uint16_t i = 0; i < ud->sensorCount; i++) {
                            len = klPtf_sprintf(&buffer[0], "\"M%dStatus\":%d,", ud->protocol[i].index, ud->protocol[i].state); DIRECT_WRITE;
                            len = klPtf_sprintf(&buffer[0], "\"M%dValue\":%d,", ud->protocol[i].index, ((int16_t) ud->protocol[i].value)); DIRECT_WRITE;
                            len = klPtf_sprintf(&buffer[0], "\"M%dUnit\":%d,", ud->protocol[i].index, ud->protocol[i].unit); DIRECT_WRITE;
                        }
                        len = klPtf_sprintf(&buffer[0], "\"DO1\":0,"); DIRECT_WRITE;
                        len = klPtf_sprintf(&buffer[0], "\"DO2\":0,"); DIRECT_WRITE;
                        len = klPtf_sprintf(&buffer[0], "\"DO3\":0,"); DIRECT_WRITE;
                        len = klPtf_sprintf(&buffer[0], "\"HostState\":0"); DIRECT_WRITE;
                    len = klPtf_sprintf(&buffer[0], "}"); DIRECT_WRITE;
                len = klPtf_sprintf(&buffer[0], "}"); DIRECT_WRITE;
            len = klPtf_sprintf(&buffer[0], "}"); DIRECT_WRITE;
        len = klPtf_sprintf(&buffer[0], "}"); DIRECT_WRITE;

        ud->isWriten = true;
        free(buffer);
        break;
    } 
}


static uint32_t directGetPayloadLength(Rilat_Instance_t *instance, uint32_t packMsgId) {    
    klist_t *pos = NULL, *tmp = NULL;
    klist_forEachSafe(pos, tmp, &mPtBufferList) {
        PtBufferCollection_t *ud = klist_entry(pos, PtBufferCollection_t, list);
        if (packMsgId != ud->messageId) {
            continue;
        }
        
        char *buffer = calloc(1, 64);
        ASSERT(buffer != NULL);
        uint16_t len = 0;

        len += klPtf_sprintf(&buffer[0], "{");
            len += klPtf_sprintf(&buffer[0], "\"iccid\":\"%s\",", ud->ins->modemInfo.iccid);
            len += klPtf_sprintf(&buffer[0], "\"rssi\":%03d,", ud->ins->modemInfo.rssi);
            len += klPtf_sprintf(&buffer[0], "\"devices\":{");
                len += klPtf_sprintf(&buffer[0], "\"1\":{");
                    len += klPtf_sprintf(&buffer[0], "\"comm_status\":1,");
                    len += klPtf_sprintf(&buffer[0], "\"variables\":{");
                        for (uint16_t i = 0; i < ud->sensorCount; i++) {
                            len += klPtf_sprintf(&buffer[0], "\"M%dStatus\":%d,", ud->protocol[i].index, ud->protocol[i].state);
                            len += klPtf_sprintf(&buffer[0], "\"M%dValue\":%d,", ud->protocol[i].index, ((int16_t) ud->protocol[i].value));
                            len += klPtf_sprintf(&buffer[0], "\"M%dUnit\":%d,", ud->protocol[i].index, ud->protocol[i].unit);
                        }
                        len += klPtf_sprintf(&buffer[0], "\"DO1\":0,");
                        len += klPtf_sprintf(&buffer[0], "\"DO2\":0,");
                        len += klPtf_sprintf(&buffer[0], "\"DO3\":0,");
                        len += klPtf_sprintf(&buffer[0], "\"HostState\":0");
                    len += klPtf_sprintf(&buffer[0], "}");
                len += klPtf_sprintf(&buffer[0], "}");
            len += klPtf_sprintf(&buffer[0], "}");
        len += klPtf_sprintf(&buffer[0], "}");

        free(buffer);
                    
        return len;
    } 
    
    return 0;
}


static void directWriteResponse(Rilat_Instance_t *instance, uint32_t packMsgId, bool isSuccess, bool isForce) {    
    PtBufferCollection_t *ud = NULL;
    klist_t *pos = NULL, *tmp = NULL;
    klist_forEachSafe(pos, tmp, &mPtBufferList) {
        ud = klist_entry(pos, PtBufferCollection_t, list);
        if (isForce) {
            klist_delete(&ud->list);
            free(ud->protocol);
            free(ud);
            ud = NULL;
            continue;
        } 
        
        if (!ud->isWriten) {
            ud = NULL;
            continue;
        }
        if (packMsgId != ud->messageId) {
            ud = NULL;
            continue;
        }
        klist_delete(&ud->list);
        break;
    }
    if (ud != NULL) {
        uint16_t messageId = ud->messageId;
        free(ud->protocol);
        free(ud);
        CREQUEST(ON_TRANSMIT_SUCCESS, {.u16[0] = messageId, .u16[1] = isSuccess});
    }
}

/*@}*/

/**
 * @addtogroup ProtocolRequest
 * @note none
 */

/*@{*/

int32_t cModule_protocolTransmit_controlX(cModule_Instance_t *ins, ProtocolRdaX_t *protocol, uint32_t count) {
    if (!_cModule_isAllocReqPackAccess(ins)) {
        return -1;
    }
    
    _cModule_packMutexLock(ins, true);
    
    cModule_ProtocolMqttMessage_t *msg = calloc(1, sizeof(cModule_ProtocolMqttMessage_t));
    ASSERT(msg != NULL);
    msg->payload = NULL;
    msg->qos = 1;
    msg->topic = calloc(1, strlen(RDA_MQTT_PUB_TOPIC_DATA) + strlen(ins->modemInfo.imei) + 1);
    ASSERT(msg->topic != NULL);
    klPtf_sprintf(msg->topic, RDA_MQTT_PUB_TOPIC_DATA, ins->modemInfo.imei);
    msg->topicConstant = false;
    msg->writer.onDirectWrite = directWrite;
    msg->writer.onDorectGetLength = directGetPayloadLength;
    msg->writer.onDirectWriteResponse = directWriteResponse;
    msg->isPayloadString = true;
    msg->timeout = 30000;
    
    PtBufferCollection_t *ud = calloc(1, sizeof(PtBufferCollection_t));
    ASSERT(ud != NULL);
    klist_init(&ud->list);
    ud->ins = ins;
    ud->protocol = protocol;
    ud->sensorCount = count;
    ud->messageId = (*_cModule_getGenMsgId())++;
    klist_addTail(&mPtBufferList, &ud->list);
    
    
    cModule_TransmitPackageInfo_t *pack = calloc(1, sizeof(cModule_TransmitPackageInfo_t));
    ASSERT(pack != NULL);
    pack->payloadLength = sizeof(cModule_ProtocolMqttMessage_t);
    pack->payload = msg;
    pack->flag.packIsDynMqttMsg = 1;
    pack->flag.requestId = TRANSMIT_PACK_REQ_ID_TX;
    pack->aux.gen.msgId = ud->messageId;
    pack->aux.gen.sendRetryCount = 1;
    pack->aux.gen.sendRetryCountReload = 1;
    pack->aux.gen.sendFailedAfterReboot = 2;
    pack->aux.gen.sendRetrySec = 3;
    pack->aux.gen.hasConfirmFrame = 0;
    pack->aux.gen.waitConfirmFrameTimeoutSec = 0;
    pack->aux.gen.waitConfirmFrameTimeoutCount = 0;
    pack->aux.gen.waitConfirmFrameTimeoutAfterReboot = 0;
    klist_init(&pack->list);
    klist_addTail(&ins->transmit.packageReqList, &pack->list);
    ins->transmit.queneReqCount++;
    _cModule_packMutexLock(ins, false);
    
    return pack->aux.gen.msgId;
}

/*@}*/

/**
 * @addtogroup ProtocolReceived
 * @note none
 */

/*@{*/

static int32_t getFieldValueInt(void *data, const char *fieldIdStr) {
    int32_t v = 0;
    
    char *str = strstr((char *) data, fieldIdStr);
    if (str == NULL) {
        return INT16_MIN;
    }
    str += strlen(fieldIdStr);
    while (*str != '\0' && *str != ':') {
        (str)++;
    }
    str++;
    v = atoi(str);
    return v;
}


static int32_t getFieldValueString(void *data, const char *fieldIdStr, char **out) {
    char *str = strstr((char *) data, fieldIdStr);
    if (str == NULL) {
        return -1;
    }
    str += strlen(fieldIdStr);
    while (*str != '\0' && *str != '\"') {
        (str)++;
    }
    str++;
    char *end = str;
    while (*end != '\0' && *end != '\"') {
        (end)++;
    }
    *out = calloc(1, (uint32_t) end - (uint32_t) str + 1);
    ASSERT(*out != NULL);
    memcpy(*out, str, (uint32_t) end - (uint32_t) str);
    return 0;
}


void _cModule_onProtocolMqttConnectAlloc(cModule_Instance_t *ins, 
        char **borkerIp,
        char **clientId, 
        char **userName, char **password) {
    *password = calloc(1, 32 + 1);
    ASSERT(*password != NULL);
}


void _cModule_onProtocolMqttConnect(cModule_Instance_t *ins, 
        char **borkerIp, uint16_t *port,
        char **clientId, 
        char **userName, char **password) {
    uint8_t *raw = calloc(1, strlen(ins->modemInfo.imei) * 2 + 1);
    ASSERT(raw != NULL);
    memcpy(raw, ins->modemInfo.imei, 15);
    memcpy(raw + 15, ins->modemInfo.imei, 15);
            
    size_t base64Size = 0;
    mbedtls_base64_encode(NULL, 0, &base64Size, raw, strlen(ins->modemInfo.imei) * 2);

    char *base64Str = calloc(1, base64Size + 1);
    ASSERT(base64Str != NULL);
    
    mbedtls_base64_encode((uint8_t *) base64Str, base64Size, NULL, raw, strlen(ins->modemInfo.imei) * 2);
    
    uint8_t *md5 = calloc(1, 16);
    ASSERT(md5 != NULL);
    mbedtls_md5_context *md5Ctx = calloc(1, sizeof(mbedtls_md5_context));
    ASSERT(md5Ctx != NULL);
    mbedtls_md5_init(md5Ctx);
    mbedtls_md5_starts(md5Ctx);
    mbedtls_md5_update(md5Ctx, (uint8_t *) base64Str, strlen(base64Str));
    mbedtls_md5_finish(md5Ctx, md5);
    mbedtls_md5_free(md5Ctx);
    free(md5Ctx);
    free(base64Str);
    free(raw);
    
    klStr_hex2str(md5, 16, *password);
    free(md5);
    
    *borkerIp = RDA_MQTT_BORKER_ADDRESS;
    *port = 1883;
    *clientId = ins->modemInfo.imei;
    *userName = ins->modemInfo.imei;
}
        

void _cModule_onProtocolMqttConnectFree(cModule_Instance_t *ins, 
        char **borkerIp,
        char **clientId, 
        char **userName, char **password) {
    free(*password);
}
        

uint8_t _cModule_onProtocolMqttSubCount(cModule_Instance_t *ins) {
    return LWMEM_ARRAYSIZE(SUB_TOPIC);
}


void _cModule_onProtocolMqttSubAlloc(cModule_Instance_t *ins, char **topic, uint8_t index) {
    *topic = calloc(1, 64);
    ASSERT(topic);
}


void _cModule_onProtocolMqttSub(cModule_Instance_t *ins, char **topic, uint8_t *qos, uint8_t index) {
    *qos = 1;
    klPtf_sprintf(*topic, SUB_TOPIC[index], ins->modemInfo.imei);
}


void _cModule_onProtocolMqttSubFree(cModule_Instance_t *ins, char **topic, uint8_t index) {
    free(*topic);
}


int32_t _cModule_onProtocolTransmited(cModule_Instance_t *ins, cModule_TransmitPackageInfo_t *info) {
    return 0;
}


int32_t _cModule_onProtocolReceived(cModule_Instance_t *ins, cModule_TransmitPackageInfo_t *info) {
    return -1;
}

/*@}*/

/**
 * @addtogroup Public
 * @note none
 */
 
/*@{*/

void cModule_protocol_init(void) {
    klist_init(&mPtBufferList);
}

/*@}*/

#endif

#if CONFIG_CMODULE_PROTOCOL_IS_RDA_MQTT_UNDER == 1
 
/**
 * @addtogroup ComponentSupport
 * @note none
 */
 
/*@{*/

#define _MODULE_REQUEST_TYPE(x)           CMODULE_REQ_##x
#define _MODULE_REQUEST_VARS              cModule_RequestVar_t
#define _MODULE_REQUEST_TYPED             cModule_RequestType_t
#define _MODULE_REQUEST_CALL              cModule_onRequestCallback
#define CREQUEST(type, var, ...)        _MODULE_REQUEST_CALL((uint32_t) _MODULE_REQUEST_TYPE(type), (_MODULE_REQUEST_VARS) var, ##__VA_ARGS__)
extern _MODULE_REQUEST_VARS _MODULE_REQUEST_CALL(_MODULE_REQUEST_TYPED type, _MODULE_REQUEST_VARS var, ...);

/*@}*/

/**
 * @addtogroup ProtocolConnectDefine
 * @note none
 */

/*@{*/

#define RDA_MQTT_BORKER_ADDRESS             "rda3000.rynon.com.cn"
#ifdef RDA_MQTT_BORKER_ADDRESS

#define RDA_MQTT_PRODUCT_KEY                "standard-config"

#define RDA_MQTT_SUB_TOPIC_VAR_WRITE        ""RDA_MQTT_PRODUCT_KEY"/%s/var_write"
#define RDA_MQTT_SUB_TOPIC_DOWN             ""RDA_MQTT_PRODUCT_KEY"/%s/down"
#define RDA_MQTT_SUB_TOPIC_CONFIG           ""RDA_MQTT_PRODUCT_KEY"/%s/config"
static const char *SUB_TOPIC[] = {
    RDA_MQTT_SUB_TOPIC_VAR_WRITE,
    RDA_MQTT_SUB_TOPIC_DOWN,
    RDA_MQTT_SUB_TOPIC_CONFIG,
};
#endif

/*@}*/

/**
 * @addtogroup Define
 * @note none
 */
 
/*@{*/

#define DIRECT_WRITE      rilat_directWritePhy(instance, (uint8_t *) &buffer[0], len); // LOG_D("%s", buffer);
#define HEX_WRITE         memset(&buffer[128], 0x00, 128); len = klStr_hex2str((uint8_t *) buffer, len, &buffer[128]); rilat_directWritePhy(instance, (uint8_t *) &buffer[128], len); // LOG_I("HEX-> %s", buffer);


#define RDA_MQTT_PRODUCT_KEY                "standard-config"
#define RDA_MQTT_PUB_TOPIC_DATA             ""RDA_MQTT_PRODUCT_KEY"/%s/data"
#define RDA_MQTT_PUB_TOPIC_UP               ""RDA_MQTT_PRODUCT_KEY"/%s/up"
#define RDA_MQTT_PUB_TOPIC_VAR_WRITE_ACK    ""RDA_MQTT_PRODUCT_KEY"/%s/var_write_ack"

typedef struct {
    cModule_Instance_t *ins;
    ProtocolRdaUnderWell_t *protocol;
    bool isWriten: 1;
    uint16_t messageId;
    klist_t list;
} PtBufferCollection_t;

/*@}*/

/**
 * @addtogroup Vars
 * @note none
 */

/*@{*/

static klist_t mPtBufferList;

/*@}*/

/**
 * @addtogroup ProtocolPrivate
 * @note none
 */

/*@{*/

static int32_t ack(cModule_Instance_t *ins,
        int8_t controlValue, 
        const char *fieldStr, int32_t filedValue,
        const char *fieldStr2, int32_t filedValue2,
        const char *fieldStr3, int32_t filedValue3,
        const char *fieldStr4, int32_t filedValue4,
        char *messageId, char *devicesId) {
    int32_t rc = -1;
    cModule_ProtocolMqttMessage_t *msg = calloc(1, sizeof(cModule_ProtocolMqttMessage_t));
    ASSERT(msg != NULL);

    uint8_t fieldContinue = 0;
    char *buffer = calloc(1, 256);
    ASSERT(buffer != NULL);
    uint16_t len = 0;
    len += klPtf_sprintf(&buffer[len], "{");
        len += klPtf_sprintf(&buffer[len], "\"message_id\":\"%s\",", messageId);
        len += klPtf_sprintf(&buffer[len], "\"devices\":{");
            len += klPtf_sprintf(&buffer[len], "\"%s\":{", devicesId);
                len += klPtf_sprintf(&buffer[len], "\"variables\":{");
                    if (controlValue >= 0) {
                        len += klPtf_sprintf(&buffer[len], "\"control\":{");
                            len += klPtf_sprintf(&buffer[len], "\"value\":%d,", controlValue);
                            len += klPtf_sprintf(&buffer[len], "\"result\":\"success\"");
                        len += klPtf_sprintf(&buffer[len], "}");
                        fieldContinue = 1;
                    }
                    if (fieldStr != NULL) {
                        len += klPtf_sprintf(&buffer[len], fieldContinue ? ",\"%s\":{" : "\"%s\":{", fieldStr);
                            len += klPtf_sprintf(&buffer[len], "\"value\":%d,", filedValue);
                            len += klPtf_sprintf(&buffer[len], "\"result\":\"success\"");
                        len += klPtf_sprintf(&buffer[len], "}");
                        fieldContinue = 1;
                    }
                    if (fieldStr2 != NULL) {
                        len += klPtf_sprintf(&buffer[len], fieldContinue ? ",\"%s\":{" : "\"%s\":{", fieldStr2);
                            len += klPtf_sprintf(&buffer[len], "\"value\":%d,", filedValue2);
                            len += klPtf_sprintf(&buffer[len], "\"result\":\"success\"");
                        len += klPtf_sprintf(&buffer[len], "}");
                        fieldContinue = 1;
                    }
                    if (fieldStr3 != NULL) {
                        len += klPtf_sprintf(&buffer[len], fieldContinue ? ",\"%s\":{" : "\"%s\":{", fieldStr3);
                            len += klPtf_sprintf(&buffer[len], "\"value\":\"%d\",", filedValue3);
                            len += klPtf_sprintf(&buffer[len], "\"result\":\"success\"");
                        len += klPtf_sprintf(&buffer[len], "}");
                        fieldContinue = 1;
                    }
                    if (fieldStr4 != NULL) {
                        len += klPtf_sprintf(&buffer[len], fieldContinue ? ",\"%s\":{" : "\"%s\":{", fieldStr4);
                            len += klPtf_sprintf(&buffer[len], "\"value\":\"%d\",", filedValue4);
                            len += klPtf_sprintf(&buffer[len], "\"result\":\"success\"");
                        len += klPtf_sprintf(&buffer[len], "}");
                        fieldContinue = 1;
                    }
                len += klPtf_sprintf(&buffer[len], "}");
            len += klPtf_sprintf(&buffer[len], "}");
        len += klPtf_sprintf(&buffer[len], "}");
    len += klPtf_sprintf(&buffer[len], "}");
    ASSERT(len <= 256);
    msg->payload = buffer;
    msg->len = len;
    
    char *topic = calloc(1, strlen(RDA_MQTT_PUB_TOPIC_VAR_WRITE_ACK) + strlen(ins->modemInfo.imei) + 1);
    ASSERT(topic != NULL);
    klPtf_sprintf(topic, RDA_MQTT_PUB_TOPIC_VAR_WRITE_ACK, ins->modemInfo.imei);
    msg->topic = topic;
    msg->topicConstant = false;
    msg->qos = 1;
    msg->isPayloadString = true;
    
    msg->timeout = 15000;
    
    _cModule_packMutexLock(ins, true);
    cModule_TransmitPackageInfo_t *pack = calloc(1, sizeof(cModule_TransmitPackageInfo_t));
    ASSERT(pack != NULL);
    pack->payloadLength = sizeof(cModule_ProtocolMqttMessage_t);
    pack->payload = msg;
    pack->flag.packIsDynMqttMsg = 1;
    pack->flag.requestId = TRANSMIT_PACK_REQ_ID_TX;
    pack->aux.gen.msgId = *_cModule_getGenMsgId();
    pack->aux.gen.sendRetryCount = 1;
    pack->aux.gen.sendRetryCountReload = 1;
    pack->aux.gen.sendFailedAfterReboot = 2;
    pack->aux.gen.sendRetrySec = 3;
    pack->aux.gen.hasConfirmFrame = 0;
    pack->aux.gen.waitConfirmFrameTimeoutSec = 0;
    pack->aux.gen.waitConfirmFrameTimeoutCount = 0;
    pack->aux.gen.waitConfirmFrameTimeoutAfterReboot = 0;
    klist_init(&pack->list);
    klist_addTail(&ins->transmit.packageReqList, &pack->list);
    ins->transmit.queneReqCount++;
    _cModule_packMutexLock(ins, false);
    return 0;
}


static void directWrite(Rilat_Instance_t *instance, uint32_t packMsgId) {
    klist_t *pos = NULL, *tmp = NULL;
    klist_forEachSafe(pos, tmp, &mPtBufferList) {
        PtBufferCollection_t *ud = klist_entry(pos, PtBufferCollection_t, list);
        if (packMsgId != ud->messageId) {
            continue;
        }
        
        char *buffer = calloc(1, 64);
        ASSERT(buffer != NULL);
        uint16_t len = 0;

        len = klPtf_sprintf(&buffer[0], "{"); DIRECT_WRITE;
            len = klPtf_sprintf(&buffer[0], "\"rssi\":%d,", ud->ins->modemInfo.rssi); DIRECT_WRITE;
            len = klPtf_sprintf(&buffer[0], "\"iccid\":\"%s\",", ud->ins->modemInfo.iccid); DIRECT_WRITE;
            len = klPtf_sprintf(&buffer[0], "\"devices\":{"); DIRECT_WRITE;
                len = klPtf_sprintf(&buffer[0], "\"%s\":{", ud->ins->modemInfo.imei); DIRECT_WRITE;
                    len = klPtf_sprintf(&buffer[0], "\"comm_status\":%d,", ud->protocol->comStatus); DIRECT_WRITE;
                    len = klPtf_sprintf(&buffer[0], "\"variables\":{"); DIRECT_WRITE;
                        len = klPtf_sprintf(&buffer[0], "\"manhole_cover_position_state\":%d,", ud->protocol->isMoveAlarm); DIRECT_WRITE;
                        len = klPtf_sprintf(&buffer[0], "\"water_level_state\":%d,", ud->protocol->isLiquidAlarm); DIRECT_WRITE;
                        len = klPtf_sprintf(&buffer[0], "\"battery_voltage\":%.2f,", ud->protocol->batLvl / 1000.0f); DIRECT_WRITE;
                        len = klPtf_sprintf(&buffer[0], "\"batState\":%d,", ud->protocol->isBatteryLow ? 1 : 0); DIRECT_WRITE;
                        len = klPtf_sprintf(&buffer[0], "\"temperature\":%d,", ud->protocol->temp); DIRECT_WRITE;
                        len = klPtf_sprintf(&buffer[0], "\"gas_type\":%d,", ud->protocol->isGasAlarm ? 1 : 0); DIRECT_WRITE;
                        len = klPtf_sprintf(&buffer[0], "\"gas_concentration\":%d,", ud->protocol->gasValue); DIRECT_WRITE;
                        len = klPtf_sprintf(&buffer[0], "\"gas_sampling_period\":%d,",ud->protocol->measureTimeMin); DIRECT_WRITE;
                        len = klPtf_sprintf(&buffer[0], "\"gas_alarm_hi\":%d,", ud->protocol->alarmH); DIRECT_WRITE;
                        len = klPtf_sprintf(&buffer[0], "\"gas_alarm_lo\":%d,", ud->protocol->alarmL); DIRECT_WRITE;
                        len = klPtf_sprintf(&buffer[0], "\"latitude\":%d,", ud->protocol->latitude); DIRECT_WRITE;
                        len = klPtf_sprintf(&buffer[0], "\"longitude\":%d,", ud->protocol->longitude); DIRECT_WRITE;
                        len = klPtf_sprintf(&buffer[0], "\"height\":%d,", ud->protocol->height); DIRECT_WRITE;
                        len = klPtf_sprintf(&buffer[0], "\"updateTime\":%d", ud->protocol->updateTimeMin); DIRECT_WRITE;
                    len = klPtf_sprintf(&buffer[0], "}"); DIRECT_WRITE;
                len = klPtf_sprintf(&buffer[0], "}"); DIRECT_WRITE;
            len = klPtf_sprintf(&buffer[0], "}"); DIRECT_WRITE;
        len = klPtf_sprintf(&buffer[0], "}"); DIRECT_WRITE;

        ud->isWriten = true;
        free(buffer);
        break;
    } 
}


static uint32_t directGetPayloadLength(Rilat_Instance_t *instance, uint32_t packMsgId) {
    klist_t *pos = NULL, *tmp = NULL;
    klist_forEachSafe(pos, tmp, &mPtBufferList) {
        PtBufferCollection_t *ud = klist_entry(pos, PtBufferCollection_t, list);
        if (packMsgId != ud->messageId) {
            continue;
        }
       
        char *buffer = calloc(1, 64);
        ASSERT(buffer != NULL);
        uint16_t len = 0;

        len += klPtf_sprintf(&buffer[0], "{");
            len += klPtf_sprintf(&buffer[0], "\"rssi\":%d,", ud->ins->modemInfo.rssi);
            len += klPtf_sprintf(&buffer[0], "\"iccid\":\"%s\",", ud->ins->modemInfo.iccid);
            len += klPtf_sprintf(&buffer[0], "\"devices\":{");
                len += klPtf_sprintf(&buffer[0], "\"%s\":{", ud->ins->modemInfo.imei);
                    len += klPtf_sprintf(&buffer[0], "\"comm_status\":%d,", ud->protocol->comStatus);
                    len += klPtf_sprintf(&buffer[0], "\"variables\":{");
                        len += klPtf_sprintf(&buffer[0], "\"manhole_cover_position_state\":%d,", ud->protocol->isMoveAlarm);
                        len += klPtf_sprintf(&buffer[0], "\"water_level_state\":%d,", ud->protocol->isLiquidAlarm);
                        len += klPtf_sprintf(&buffer[0], "\"battery_voltage\":%.2f,", ud->protocol->batLvl / 1000.0f);
                        len += klPtf_sprintf(&buffer[0], "\"batState\":%d,", ud->protocol->isBatteryLow ? 1 : 0);
                        len += klPtf_sprintf(&buffer[0], "\"temperature\":%d,", ud->protocol->temp);
                        len += klPtf_sprintf(&buffer[0], "\"gas_type\":%d,", ud->protocol->isGasAlarm ? 1 : 0);
                        len += klPtf_sprintf(&buffer[0], "\"gas_concentration\":%d,", ud->protocol->gasValue);
                        len += klPtf_sprintf(&buffer[0], "\"gas_sampling_period\":%d,",ud->protocol->measureTimeMin);
                        len += klPtf_sprintf(&buffer[0], "\"gas_alarm_hi\":%d,", ud->protocol->alarmH);
                        len += klPtf_sprintf(&buffer[0], "\"gas_alarm_lo\":%d,", ud->protocol->alarmL);
                        len += klPtf_sprintf(&buffer[0], "\"latitude\":%d,", ud->protocol->latitude);
                        len += klPtf_sprintf(&buffer[0], "\"longitude\":%d,", ud->protocol->alarmL);
                        len += klPtf_sprintf(&buffer[0], "\"height\":%d,", ud->protocol->height);
                        len += klPtf_sprintf(&buffer[0], "\"updateTime\":%d", ud->protocol->updateTimeMin);
                    len += klPtf_sprintf(&buffer[0], "}");
                len += klPtf_sprintf(&buffer[0], "}");
            len += klPtf_sprintf(&buffer[0], "}");
        len += klPtf_sprintf(&buffer[0], "}");

        free(buffer);
                    
        return len;
    } 
    
    return 0;
}


static void directWriteResponse(Rilat_Instance_t *instance, uint32_t packMsgId, bool isSuccess, bool isForce) {
    PtBufferCollection_t *ud = NULL;
    klist_t *pos = NULL, *tmp = NULL;
    klist_forEachSafe(pos, tmp, &mPtBufferList) {
        ud = klist_entry(pos, PtBufferCollection_t, list);
        if (isForce) {
            klist_delete(&ud->list);
            free(ud->protocol);
            free(ud);
            ud = NULL;
            continue;
        }
        if (!ud->isWriten) {
            ud = NULL;
            continue;
        }
        if (packMsgId != ud->messageId) {
            ud = NULL;
            continue;
        }
        klist_delete(&ud->list);
        break;
    }
    if (ud != NULL) {
        free(ud->protocol);
        free(ud);
        CREQUEST(PT_ON_TRANSMIT_SUCCESS, {.u16[0] = ud->messageId, .u16[1] = isSuccess});
    }
}

/*@}*/

/**
 * @addtogroup ProtocolRequest
 * @note none
 */

/*@{*/

int32_t cModule_protocolTransmit_UndergroundWell(cModule_Instance_t *ins, ProtocolRdaUnderWell_t *protocol, bool forceWrite) {
    if (!_cModule_isAllocReqPackAccess(ins)) {
        return -1;
    }
    
    int32_t rc = -1;
    cModule_ProtocolMqttMessage_t *msg = calloc(1, sizeof(cModule_ProtocolMqttMessage_t));
    ASSERT(msg != NULL);
    msg->payload = NULL;
    msg->qos = 1;
    msg->topic = calloc(1, strlen(RDA_MQTT_PUB_TOPIC_DATA) + strlen(ins->modemInfo.imei) + 1);
    ASSERT(msg->topic != NULL);
    klPtf_sprintf(msg->topic, RDA_MQTT_PUB_TOPIC_DATA, ins->modemInfo.imei);
    msg->topicConstant = false;
    msg->writer.onDirectWrite = directWrite;
    msg->writer.onDorectGetLength = directGetPayloadLength;
    msg->writer.onDirectWriteResponse = directWriteResponse;
    msg->isPayloadString = true;
    msg->timeout = forceWrite ? 0 : 15000;
    
    PtBufferCollection_t *ud = calloc(1, sizeof(PtBufferCollection_t));
    ASSERT(ud != NULL);
    klist_init(&ud->list);
    ud->ins = ins;
    ud->protocol = protocol;
    ud->messageId = (*_cModule_getGenMsgId())++;
    if (forceWrite) {
        klist_add(&mPtBufferList, &ud->list);
    } else {
        klist_addTail(&mPtBufferList, &ud->list);
    }
    _cModule_packMutexLock(ins, true);
    cModule_TransmitPackageInfo_t *pack = calloc(1, sizeof(cModule_TransmitPackageInfo_t));
    ASSERT(pack != NULL);
    pack->payloadLength = sizeof(cModule_ProtocolMqttMessage_t);
    pack->payload = msg;
    pack->flag.packIsDynMqttMsg = 1;
    pack->flag.requestId = TRANSMIT_PACK_REQ_ID_TX;
    pack->aux.gen.msgId = ud->messageId;
    pack->aux.gen.sendRetryCount = 1;
    pack->aux.gen.sendRetryCountReload = 1;
    pack->aux.gen.sendFailedAfterReboot = 2;
    pack->aux.gen.sendRetrySec = 3;
    pack->aux.gen.hasConfirmFrame = 0;
    pack->aux.gen.waitConfirmFrameTimeoutSec = 0;
    pack->aux.gen.waitConfirmFrameTimeoutCount = 0;
    pack->aux.gen.waitConfirmFrameTimeoutAfterReboot = 0;
    klist_init(&pack->list);
    klist_addTail(&ins->transmit.packageReqList, &pack->list);
    ins->transmit.queneReqCount++;
    _cModule_packMutexLock(ins, false);
    
    return pack->aux.gen.msgId;
}

/*@}*/

/**
 * @addtogroup ProtocolReceived
 * @note none
 */

/*@{*/

static int32_t getFieldValueInt(void *data, const char *fieldIdStr) {
    int32_t v = 0;
    
    char *str = strstr((char *) data, fieldIdStr);
    if (str == NULL) {
        return INT16_MIN;
    }
    str += strlen(fieldIdStr);
    while (*str != '\0' && *str != ':') {
        (str)++;
    }
    str++;
    v = atoi(str);
    return v;
}


static int32_t getFieldValueString(void *data, const char *fieldIdStr, char **out) {
    char *str = strstr((char *) data, fieldIdStr);
    if (str == NULL) {
        return -1;
    }
    str += strlen(fieldIdStr);
    while (*str != '\0' && *str != '\"') {
        (str)++;
    }
    str++;
    char *end = str;
    while (*end != '\0' && *end != '\"') {
        (end)++;
    }
    *out = calloc(1, (uint32_t) end - (uint32_t) str + 1);
    ASSERT(*out != NULL);
    memcpy(*out, str, (uint32_t) end - (uint32_t) str);
    return 0;
}


void _cModule_onProtocolMqttConnectAlloc(cModule_Instance_t *ins, 
        char **borkerIp,
        char **clientId, 
        char **userName, char **password) {
    *password = calloc(1, 32 + 1);
    ASSERT(*password != NULL);
}


void _cModule_onProtocolMqttConnect(cModule_Instance_t *ins, 
        char **borkerIp, uint16_t *port,
        char **clientId, 
        char **userName, char **password) {
    uint8_t *raw = calloc(1, strlen(ins->modemInfo.imei) * 2 + 1);
    ASSERT(raw != NULL);
    memcpy(raw, ins->modemInfo.imei, 15);
    memcpy(raw + 15, ins->modemInfo.imei, 15);
            
    size_t base64Size = 0;
    mbedtls_base64_encode(NULL, 0, &base64Size, raw, strlen(ins->modemInfo.imei) * 2);

    char *base64Str = calloc(1, base64Size + 1);
    ASSERT(base64Str != NULL);
    
    mbedtls_base64_encode((uint8_t *) base64Str, base64Size, NULL, raw, strlen(ins->modemInfo.imei) * 2);
    
    uint8_t *md5 = calloc(1, 16);
    ASSERT(md5 != NULL);
    mbedtls_md5_context *md5Ctx = calloc(1, sizeof(mbedtls_md5_context));
    ASSERT(md5Ctx != NULL);
    mbedtls_md5_init(md5Ctx);
    mbedtls_md5_starts(md5Ctx);
    mbedtls_md5_update(md5Ctx, (uint8_t *) base64Str, strlen(base64Str));
    mbedtls_md5_finish(md5Ctx, md5);
    mbedtls_md5_free(md5Ctx);
    free(md5Ctx);
    free(base64Str);
    free(raw);
    
    klStr_hex2str(md5, 16, *password);
    free(md5);
    
    *borkerIp = RDA_MQTT_BORKER_ADDRESS;
    *port = 1883;
    *clientId = ins->modemInfo.imei;
    *userName = ins->modemInfo.imei;
}
        

void _cModule_onProtocolMqttConnectFree(cModule_Instance_t *ins, 
        char **borkerIp,
        char **clientId, 
        char **userName, char **password) {
    free(*password);
}
        

uint8_t _cModule_onProtocolMqttSubCount(cModule_Instance_t *ins) {
    return LWMEM_ARRAYSIZE(SUB_TOPIC);
}


void _cModule_onProtocolMqttSubAlloc(cModule_Instance_t *ins, char **topic, uint8_t index) {
    *topic = calloc(1, 64);
    ASSERT(topic);
}


void _cModule_onProtocolMqttSub(cModule_Instance_t *ins, char **topic, uint8_t *qos, uint8_t index) {
    *qos = 1;
    klPtf_sprintf(*topic, SUB_TOPIC[index], ins->modemInfo.imei);
}


void _cModule_onProtocolMqttSubFree(cModule_Instance_t *ins, char **topic, uint8_t index) {
    free(*topic);
}


int32_t _cModule_onProtocolTransmited(cModule_Instance_t *ins, cModule_TransmitPackageInfo_t *info) {
    return 0;
}


int32_t _cModule_onProtocolReceived(cModule_Instance_t *ins, cModule_TransmitPackageInfo_t *info) {
    static const char *FIELD_MESSAGE_ID = "\"message_id\"";
    static const char *FIELD_CONTROL = "\"device_control\"";
    static const char *FIELD_DEVICE_ID = "\"device\"";
    static const char *FIELD_GAS_SAMPLING_PERIOD = "\"gas_sampling_period\"";
    static const char *FIELD_GAS_ALARM_HI = "\"gas_alarm_hi\"";
    static const char *FIELD_GAS_ALARM_LO = "\"gas_alarm_lo\"";
    static const char *FIELD_UPDATE_TIME = "\"updateTime\"";
    
char *deviceIdStr = NULL;
    getFieldValueString(info, FIELD_DEVICE_ID, &deviceIdStr);
    char *messageIdStr = NULL;
    if (getFieldValueString(info->payload, FIELD_MESSAGE_ID, &messageIdStr) != 0) {
        return -1;
    }
    
    int16_t control = getFieldValueInt(info->payload, FIELD_CONTROL);
    int16_t gasSamplingPeriod = getFieldValueInt(info->payload, FIELD_GAS_SAMPLING_PERIOD);
    int16_t gasAlarmHi = getFieldValueInt(info->payload, FIELD_GAS_ALARM_HI);
    int16_t gasAlarmLo = getFieldValueInt(info->payload, FIELD_GAS_ALARM_LO);
    int16_t updateTime = getFieldValueInt(info->payload, FIELD_UPDATE_TIME);
    
    ack(ins, 
        control != INT16_MIN ? control : -1, 
        gasSamplingPeriod != INT16_MIN ? FIELD_GAS_SAMPLING_PERIOD : NULL, gasSamplingPeriod, 
        gasAlarmHi != INT16_MIN ? FIELD_GAS_ALARM_HI : NULL, gasAlarmHi, 
        gasAlarmLo != INT16_MIN ? FIELD_GAS_ALARM_LO : NULL, gasAlarmLo, 
        updateTime != INT16_MIN ? FIELD_UPDATE_TIME : NULL, updateTime, 
        messageIdStr, deviceIdStr != NULL ? deviceIdStr : cModule_getModuleUId(ins));
    free(messageIdStr);
    free(deviceIdStr);
    
    if (control == 3) {
        CREQUEST(PT_ON_SELF_TEST, {});
    } else if (control == 2) {
        CREQUEST(PT_ON_ALARM_SILENCE, {});
    } else if (control == 1) {
        CREQUEST(PT_ON_RESET, {});
    }               
    if (gasAlarmHi != INT16_MIN) {
        CREQUEST(PT_SET_ALARM_HI, {.u32 = gasAlarmHi});
    }
    if (gasAlarmLo != INT16_MIN) {
        CREQUEST(PT_SET_ALARM_LO, {.u32 = gasAlarmLo});
    }
    if (gasSamplingPeriod != INT16_MIN) {
        CREQUEST(PT_SET_SAMPLING_PERIOPD, {.u32 = gasSamplingPeriod});
    }
    if (updateTime != INT16_MIN) {
        CREQUEST(PT_SET_UPDATIME_MIN, {.u32 = updateTime});
    }
    return 0;
}

/*@}*/

/**
 * @addtogroup Public
 * @note none
 */
 
/*@{*/

void cModule_protocol_init(void) {
    klist_init(&mPtBufferList);
}

bool cModule_protocol_isCaching(void) {
    return klist_empty(&mPtBufferList) ? false : true;
}    

/*@}*/

#endif

#if CONFIG_CMODULE_PROTOCOL_IS_RDA_MQTT_TAMPER == 1

/**
 * @addtogroup ComponentSupport
 * @note none
 */
 
/*@{*/

#define _MODULE_REQUEST_TYPE(x)           CMODULE_REQ_##x
#define _MODULE_REQUEST_VARS              cModule_RequestVar_t
#define _MODULE_REQUEST_TYPED             cModule_RequestType_t
#define _MODULE_REQUEST_CALL              cModule_onRequestCallback
#define CREQUEST(type, var, ...)        _MODULE_REQUEST_CALL((uint32_t) _MODULE_REQUEST_TYPE(type), (_MODULE_REQUEST_VARS) var, ##__VA_ARGS__)
extern _MODULE_REQUEST_VARS _MODULE_REQUEST_CALL(_MODULE_REQUEST_TYPED type, _MODULE_REQUEST_VARS var, ...);

/*@}*/

/**
 * @addtogroup ProtocolConnectDefine
 * @note none
 */

/*@{*/

#define RDA_MQTT_BORKER_ADDRESS             "rda3000.rynon.com.cn"
#ifdef RDA_MQTT_BORKER_ADDRESS

#define RDA_MQTT_PRODUCT_KEY                "standard-config"

#define RDA_MQTT_SUB_TOPIC_VAR_WRITE        ""RDA_MQTT_PRODUCT_KEY"/%s/var_write"
#define RDA_MQTT_SUB_TOPIC_DOWN             ""RDA_MQTT_PRODUCT_KEY"/%s/down"
#define RDA_MQTT_SUB_TOPIC_CONFIG           ""RDA_MQTT_PRODUCT_KEY"/%s/config"
static const char *SUB_TOPIC[] = {
    RDA_MQTT_SUB_TOPIC_VAR_WRITE,
    RDA_MQTT_SUB_TOPIC_DOWN,
    RDA_MQTT_SUB_TOPIC_CONFIG,
};
#endif

/*@}*/

/**
 * @addtogroup Define
 * @note none
 */
 
/*@{*/

#define DIRECT_WRITE      rilat_directWritePhy(instance, (uint8_t *) &buffer[0], len); // LOG_D("%s", buffer);
#define HEX_WRITE         memset(&buffer[128], 0x00, 128); len = klStr_hex2str((uint8_t *) buffer, len, &buffer[128]); rilat_directWritePhy(instance, (uint8_t *) &buffer[128], len); // LOG_I("HEX-> %s", buffer);

#define RDA_MQTT_PRODUCT_KEY                "standard-config"
#define RDA_MQTT_PUB_TOPIC_DATA             ""RDA_MQTT_PRODUCT_KEY"/%s/data"
#define RDA_MQTT_PUB_TOPIC_UP               ""RDA_MQTT_PRODUCT_KEY"/%s/up"
#define RDA_MQTT_PUB_TOPIC_VAR_WRITE_ACK    ""RDA_MQTT_PRODUCT_KEY"/%s/var_write_ack"

typedef struct {
    cModule_Instance_t *ins;
    ProtocolRdaTamper_t *protocol;
    bool isWriten: 1;
    uint16_t messageId;
    klist_t list;
} PtBufferCollection_t;

/*@}*/

/**
 * @addtogroup Vars
 * @note none
 */

/*@{*/

static klist_t mPtBufferList;

/*@}*/

/**
 * @addtogroup ProtocolPrivate
 * @note none
 */

/*@{*/

static int32_t ack(cModule_Instance_t *ins, uint8_t value, char *messageId, char *devicesId) {
    int32_t rc = -1;
    cModule_ProtocolMqttMessage_t *msg = calloc(1, sizeof(cModule_ProtocolMqttMessage_t));
    ASSERT(msg != NULL);

    char *buffer = calloc(1, 128);
    ASSERT(buffer != NULL);
    uint16_t len = 0;
    len += klPtf_sprintf(&buffer[len], "{");
        len += klPtf_sprintf(&buffer[len], "\"message_id\":\"%s\",", messageId);
        len += klPtf_sprintf(&buffer[len], "\"devices\":{");
            len += klPtf_sprintf(&buffer[len], "\"%s\":{", devicesId);
                len += klPtf_sprintf(&buffer[len], "\"variables\":{");
                    len += klPtf_sprintf(&buffer[len], "\"control\":{");
                        len += klPtf_sprintf(&buffer[len], "\"value\":\"%d\",", value);
                        len += klPtf_sprintf(&buffer[len], "\"result\":\"success\"");
                    len += klPtf_sprintf(&buffer[len], "}");
                len += klPtf_sprintf(&buffer[len], "}");
            len += klPtf_sprintf(&buffer[len], "}");
        len += klPtf_sprintf(&buffer[len], "}");
    len += klPtf_sprintf(&buffer[len], "}");
    ASSERT(len <= 128);
    msg->payload = buffer;
    msg->len = len;
    
    char *topic = calloc(1, strlen(RDA_MQTT_PUB_TOPIC_VAR_WRITE_ACK) + strlen(ins->modemInfo.imei) + 1);
    ASSERT(topic != NULL);
    klPtf_sprintf(topic, RDA_MQTT_PUB_TOPIC_VAR_WRITE_ACK, ins->modemInfo.imei);
    msg->topic = topic;
    msg->topicConstant = false;
    msg->qos = 1;
    msg->isPayloadString = true;
    
    msg->timeout = 15000;
    
    _cModule_packMutexLock(ins, true);
    cModule_TransmitPackageInfo_t *pack = calloc(1, sizeof(cModule_TransmitPackageInfo_t));
    ASSERT(pack != NULL);
    pack->payloadLength = sizeof(cModule_ProtocolMqttMessage_t);
    pack->payload = msg;
    pack->flag.packIsDynMqttMsg = 1;
    pack->flag.requestId = TRANSMIT_PACK_REQ_ID_TX;
    pack->aux.gen.msgId = *_cModule_getGenMsgId();
    pack->aux.gen.sendRetryCount = 1;
    pack->aux.gen.sendRetryCountReload = 1;
    pack->aux.gen.sendFailedAfterReboot = 2;
    pack->aux.gen.sendRetrySec = 3;
    pack->aux.gen.hasConfirmFrame = 0;
    pack->aux.gen.waitConfirmFrameTimeoutSec = 0;
    pack->aux.gen.waitConfirmFrameTimeoutCount = 0;
    pack->aux.gen.waitConfirmFrameTimeoutAfterReboot = 0;
    klist_init(&pack->list);
    klist_addTail(&ins->transmit.packageReqList, &pack->list);
    ins->transmit.queneReqCount++;
    _cModule_packMutexLock(ins, false);
    return 0;
}


static void directWrite(Rilat_Instance_t *instance, uint32_t packMsgId) {
    klist_t *pos = NULL, *tmp = NULL;
    klist_forEachSafe(pos, tmp, &mPtBufferList) {
        PtBufferCollection_t *ud = klist_entry(pos, PtBufferCollection_t, list);
        if (packMsgId != ud->messageId) {
            continue;
        }
        
        char *buffer = calloc(1, 64);
        ASSERT(buffer != NULL);
        uint16_t len = 0;

        len = klPtf_sprintf(&buffer[0], "{"); DIRECT_WRITE;
            len = klPtf_sprintf(&buffer[0], "\"rssi\":%d,", ud->ins->modemInfo.rssi); DIRECT_WRITE;
            len = klPtf_sprintf(&buffer[0], "\"iccid\":\"%s\",", ud->ins->modemInfo.iccid); DIRECT_WRITE;
            len = klPtf_sprintf(&buffer[0], "\"version\":\"%s\",", "v1.1.1.1"); DIRECT_WRITE;
            len = klPtf_sprintf(&buffer[0], "\"devices\":{"); DIRECT_WRITE;
                len = klPtf_sprintf(&buffer[0], "\"%s\":{", "1"); DIRECT_WRITE;
                    len = klPtf_sprintf(&buffer[0], "\"comm_status\":%d,", 1); DIRECT_WRITE;
                    len = klPtf_sprintf(&buffer[0], "\"variables\":{"); DIRECT_WRITE;
                        len = klPtf_sprintf(&buffer[0], "\"devState\":%d,", ud->protocol->isAlarm); DIRECT_WRITE; 
                        len = klPtf_sprintf(&buffer[0], "\"batState\":%d,", ud->protocol->isBatteryLow); DIRECT_WRITE;
                        len = klPtf_sprintf(&buffer[0], "\"battery_voltage\":%.3f,", ud->protocol->batLvl / 1000.0f); DIRECT_WRITE;
                        len = klPtf_sprintf(&buffer[0], "\"updateTime\":%d", ud->protocol->updateTimeMin); DIRECT_WRITE;
                    len = klPtf_sprintf(&buffer[0], "}"); DIRECT_WRITE;
                len = klPtf_sprintf(&buffer[0], "}"); DIRECT_WRITE;
            len = klPtf_sprintf(&buffer[0], "}"); DIRECT_WRITE;
        len = klPtf_sprintf(&buffer[0], "}"); DIRECT_WRITE;

        ud->isWriten = true;
        free(buffer);
        break;
    } 
}


static uint32_t directGetPayloadLength(Rilat_Instance_t *instance, uint32_t packMsgId) {
    klist_t *pos = NULL, *tmp = NULL;
    klist_forEachSafe(pos, tmp, &mPtBufferList) {
        PtBufferCollection_t *ud = klist_entry(pos, PtBufferCollection_t, list);
        if (packMsgId != ud->messageId) {
            continue;
        }
       
        char *buffer = calloc(1, 64);
        ASSERT(buffer != NULL);
        uint16_t len = 0;

        len += klPtf_sprintf(&buffer[0], "{");
            len += klPtf_sprintf(&buffer[0], "\"rssi\":%d,", ud->ins->modemInfo.rssi);
            len += klPtf_sprintf(&buffer[0], "\"iccid\":\"%s\",", ud->ins->modemInfo.iccid);
            len += klPtf_sprintf(&buffer[0], "\"version\":\"%s\",", "v1.1.1.1");
            len += klPtf_sprintf(&buffer[0], "\"devices\":{");
                len += klPtf_sprintf(&buffer[0], "\"%s\":{", "1");
                    len += klPtf_sprintf(&buffer[0], "\"comm_status\":%d,", 1);
                    len += klPtf_sprintf(&buffer[0], "\"variables\":{");
                        len += klPtf_sprintf(&buffer[0], "\"devState\":%d,", ud->protocol->isAlarm);
                        len += klPtf_sprintf(&buffer[0], "\"batState\":%d,", ud->protocol->isBatteryLow);
                        len += klPtf_sprintf(&buffer[0], "\"battery_voltage\":%.3f,", ud->protocol->batLvl / 1000.0f);
                        len += klPtf_sprintf(&buffer[0], "\"updateTime\":%d", ud->protocol->updateTimeMin);
                    len += klPtf_sprintf(&buffer[0], "}");
                len += klPtf_sprintf(&buffer[0], "}");
            len += klPtf_sprintf(&buffer[0], "}");
        len += klPtf_sprintf(&buffer[0], "}");

        free(buffer);
                    
        return len;
    } 
    
    return 0;
}


static void directWriteResponse(Rilat_Instance_t *instance, uint32_t packMsgId, bool isSuccess, bool isForce) {
    PtBufferCollection_t *ud = NULL;
    klist_t *pos = NULL, *tmp = NULL;
    klist_forEachSafe(pos, tmp, &mPtBufferList) {
        ud = klist_entry(pos, PtBufferCollection_t, list);
        if (isForce) {
            klist_delete(&ud->list);
            free(ud->protocol);
            free(ud);
            ud = NULL;
            continue;
        }
        if (!ud->isWriten) {
            ud = NULL;
            continue;
        }
        if (packMsgId != ud->messageId) {
            ud = NULL;
            continue;
        }
        klist_delete(&ud->list);
        break;
    }
    if (ud != NULL) {
        free(ud->protocol);
        free(ud);
        CREQUEST(PT_ON_TRANSMIT_SUCCESS, {.u16[0] = ud->messageId, .u16[1] = isSuccess});
    }
}

/*@}*/

/**
 * @addtogroup ProtocolRequest
 * @note none
 */

/*@{*/

int32_t cModule_protocolTransmit_tamper(cModule_Instance_t *ins, ProtocolRdaTamper_t *protocol, bool forceWrite) {
    if (!_cModule_isAllocReqPackAccess(ins)) {
        return -1;
    }
    
    int32_t rc = -1;
    cModule_ProtocolMqttMessage_t *msg = calloc(1, sizeof(cModule_ProtocolMqttMessage_t));
    ASSERT(msg != NULL);
    msg->payload = NULL;
    msg->qos = 1;
    msg->topic = calloc(1, strlen(RDA_MQTT_PUB_TOPIC_DATA) + strlen(ins->modemInfo.imei) + 1);
    ASSERT(msg->topic != NULL);
    klPtf_sprintf(msg->topic, RDA_MQTT_PUB_TOPIC_DATA, ins->modemInfo.imei);
    msg->topicConstant = false;
    msg->writer.onDirectWrite = directWrite;
    msg->writer.onDorectGetLength = directGetPayloadLength;
    msg->writer.onDirectWriteResponse = directWriteResponse;
    msg->isPayloadString = true;
    msg->timeout = forceWrite ? 0 : 15000;
    
    PtBufferCollection_t *ud = calloc(1, sizeof(PtBufferCollection_t));
    ASSERT(ud != NULL);
    klist_init(&ud->list);
    ud->ins = ins;
    ud->protocol = protocol;
    ud->messageId = (*_cModule_getGenMsgId())++;
    if (forceWrite) {
        klist_add(&mPtBufferList, &ud->list);
    } else {
        klist_addTail(&mPtBufferList, &ud->list);
    }
    _cModule_packMutexLock(ins, true);
    cModule_TransmitPackageInfo_t *pack = calloc(1, sizeof(cModule_TransmitPackageInfo_t));
    ASSERT(pack != NULL);
    pack->payloadLength = sizeof(cModule_ProtocolMqttMessage_t);
    pack->payload = msg;
    pack->flag.packIsDynMqttMsg = 1;
    pack->flag.requestId = TRANSMIT_PACK_REQ_ID_TX;
    pack->aux.gen.msgId = ud->messageId;
    pack->aux.gen.sendRetryCount = 1;
    pack->aux.gen.sendRetryCountReload = 1;
    pack->aux.gen.sendFailedAfterReboot = 2;
    pack->aux.gen.sendRetrySec = 3;
    pack->aux.gen.hasConfirmFrame = 0;
    pack->aux.gen.waitConfirmFrameTimeoutSec = 0;
    pack->aux.gen.waitConfirmFrameTimeoutCount = 0;
    pack->aux.gen.waitConfirmFrameTimeoutAfterReboot = 0;
    klist_init(&pack->list);
    klist_addTail(&ins->transmit.packageReqList, &pack->list);
    ins->transmit.queneReqCount++;
    _cModule_packMutexLock(ins, false);
    
    return pack->aux.gen.msgId;
}

/*@}*/

/**
 * @addtogroup ProtocolReceived
 * @note none
 */

/*@{*/

static int32_t getFieldValueInt(void *data, const char *fieldIdStr) {
    int32_t v = 0;
    
    char *str = strstr((char *) data, fieldIdStr);
    if (str == NULL) {
        return INT16_MIN;
    }
    str += strlen(fieldIdStr);
    while (*str != '\0' && *str != ':') {
        (str)++;
    }
    str++;
    v = atoi(str);
    return v;
}


static int32_t getFieldValueString(void *data, const char *fieldIdStr, char **out) {
    char *str = strstr((char *) data, fieldIdStr);
    if (str == NULL) {
        return -1;
    }
    str += strlen(fieldIdStr);
    while (*str != '\0' && *str != '\"') {
        (str)++;
    }
    str++;
    char *end = str;
    while (*end != '\0' && *end != '\"') {
        (end)++;
    }
    *out = calloc(1, (uint32_t) end - (uint32_t) str + 1);
    ASSERT(*out != NULL);
    memcpy(*out, str, (uint32_t) end - (uint32_t) str);
    return 0;
}


void _cModule_onProtocolMqttConnectAlloc(cModule_Instance_t *ins, 
        char **borkerIp,
        char **clientId, 
        char **userName, char **password) {
    *password = calloc(1, 32 + 1);
    ASSERT(*password != NULL);
}


void _cModule_onProtocolMqttConnect(cModule_Instance_t *ins, 
        char **borkerIp, uint16_t *port,
        char **clientId, 
        char **userName, char **password) {
    uint8_t *raw = calloc(1, strlen(ins->modemInfo.imei) * 2 + 1);
    ASSERT(raw != NULL);
    memcpy(raw, ins->modemInfo.imei, 15);
    memcpy(raw + 15, ins->modemInfo.imei, 15);
            
    size_t base64Size = 0;
    mbedtls_base64_encode(NULL, 0, &base64Size, raw, strlen(ins->modemInfo.imei) * 2);

    char *base64Str = calloc(1, base64Size + 1);
    ASSERT(base64Str != NULL);
    
    mbedtls_base64_encode((uint8_t *) base64Str, base64Size, NULL, raw, strlen(ins->modemInfo.imei) * 2);
    
    uint8_t *md5 = calloc(1, 16);
    ASSERT(md5 != NULL);
    mbedtls_md5_context *md5Ctx = calloc(1, sizeof(mbedtls_md5_context));
    ASSERT(md5Ctx != NULL);
    mbedtls_md5_init(md5Ctx);
    mbedtls_md5_starts(md5Ctx);
    mbedtls_md5_update(md5Ctx, (uint8_t *) base64Str, strlen(base64Str));
    mbedtls_md5_finish(md5Ctx, md5);
    mbedtls_md5_free(md5Ctx);
    free(md5Ctx);
    free(base64Str);
    free(raw);
    
    klStr_hex2str(md5, 16, *password);
    free(md5);
    
    *borkerIp = RDA_MQTT_BORKER_ADDRESS;
    *port = 1883;
    *clientId = ins->modemInfo.imei;
    *userName = ins->modemInfo.imei;
}
        

void _cModule_onProtocolMqttConnectFree(cModule_Instance_t *ins, 
        char **borkerIp,
        char **clientId, 
        char **userName, char **password) {
    free(*password);
}
        

uint8_t _cModule_onProtocolMqttSubCount(cModule_Instance_t *ins) {
//    return LWMEM_ARRAYSIZE(SUB_TOPIC);
    return 0;
}


void _cModule_onProtocolMqttSubAlloc(cModule_Instance_t *ins, char **topic, uint8_t index) {
//    *topic = calloc(1, 64);
//    ASSERT(topic);
}


void _cModule_onProtocolMqttSub(cModule_Instance_t *ins, char **topic, uint8_t *qos, uint8_t index) {
//    *qos = 1;
//    klPtf_sprintf(*topic, SUB_TOPIC[index], ins->modemInfo.imei);
}


void _cModule_onProtocolMqttSubFree(cModule_Instance_t *ins, char **topic, uint8_t index) {
//    free(*topic);
}


int32_t _cModule_onProtocolTransmited(cModule_Instance_t *ins, cModule_TransmitPackageInfo_t *info) {
    return 0;
}


int32_t _cModule_onProtocolReceived(cModule_Instance_t *ins, cModule_TransmitPackageInfo_t *info) {
    return 0;
}

/*@}*/

/**
 * @addtogroup Public
 * @note none
 */
 
/*@{*/

void cModule_protocol_init(void) {
    klist_init(&mPtBufferList);
}

bool cModule_protocol_isCaching(void) {
    return klist_empty(&mPtBufferList) ? false : true;
}    

/*@}*/

#endif

/**
 * @addtogroup Debug support
 * @note none
 */
 
/*@{*/

#undef DBG_SECTION_NAME

/*@}*/

