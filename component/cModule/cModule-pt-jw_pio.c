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

#if CONFIG_CMODULE_PT_SETTER == CONFIG_CMODULE_PT_SETTER_JW_PIO \
    || CONFIG_CMODULE_PT_SETTER == CONFIG_CMODULE_PT_SETTER_JW_PH
 
/**
 * @addtogroup Debug support
 * @note none
 */
 
/*@{*/

#undef DBG_SECTION_NAME
#define DBG_SECTION_NAME  "cModule-pt-jw_pio"

#define CMASSERT(err) ASSERT(err)

/*@}*/

/**
 * @addtogroup ComponentSupport
 * @note none
 */
 
/*@{*/

#define _MODULE_REQUEST_TYPE(x)           CMODULE_REQ_PT_##x
#define _MODULE_REQUEST_VARS              cModule_RequestVar_t
#define _MODULE_REQUEST_TYPED             cModule_RequestType_t
#define _MODULE_REQUEST_CALL              cModule_onRequestCallback
#define CREQUEST(type, var, ...)          _MODULE_REQUEST_CALL((uint32_t) _MODULE_REQUEST_TYPE(type), (_MODULE_REQUEST_VARS) var, ##__VA_ARGS__)
extern _MODULE_REQUEST_VARS _MODULE_REQUEST_CALL(_MODULE_REQUEST_TYPED type, _MODULE_REQUEST_VARS var, ...);

/*@}*/

/**
 * @addtogroup ProtocolConnectDefine
 * @note none
 */

/*@{*/


#define AEP_MQTT_BORKER_ADDRESS            "mqtt.ctwing.cn"
#define AEP_MQTT_BORKER_PORT               1883
#define AEP_MQTT_BASE_SUB_TOPIC            "mqtt/"CONFIG_AEP_MQTT_VSTORE_ID"/"CONFIG_AEP_MQTT_CLIENTID"/"CONFIG_AEP_MQTT_CLIENTID"%s/dn/device_control"

// AEP_地风升-MQTT-DO
#define CONFIG_AEP_MQTT_VSTORE_ID          "2000117012"    // 租户ID
#define CONFIG_AEP_MQTT_CLIENTID           "15491373"      // 产品ID
#define CONFIG_AEP_MQTT_USERNAME           "unx"    // 随机
#define CONFIG_AEP_MQTT_PASSWORD           "aKbGu3blTqJC7A2w_SqcwSaUkgOf2edTHRV0d5Y8CkQ" // 产品特征串

// AEP_Flandreunx-MQTT
//#define CONFIG_PROTOCOL_MQTT_VSTORE_ID          "2000085867"    // 租户ID
//#define CONFIG_PROTOCOL_MQTT_CLIENTID           "15391288"      // 产品ID
//#define CONFIG_PROTOCOL_MQTT_USERNAME           "unx"    // 随机
//#define CONFIG_PROTOCOL_MQTT_PASSWORD           "5nhHf63UGc4IHHsDOWX6e2Fr0_6PT3hSi4d2R5RmlEI" // 产品特征串

/*@}*/

/**
 * @addtogroup OtaProtocolSupport
 * @note none
 */

/*@{*/

static int32_t aepOtaUpgradeReceived(cModule_Instance_t *ins, char *data, uint16_t dataLength) {
    int32_t rc = -1;
    LOG_I("Parser, aepOtaUpgradeReceived");
    cJSON *cjRoot = cJSON_Parse((char *) data);
    if (cjRoot == NULL) {
        LOG_W("Parse, [cjRoot] == NULL");
        goto l_exit;
    }

    cJSON *cjTaskId = cJSON_GetObjectItemCaseSensitive(cjRoot, "taskId");
    if (cjTaskId == NULL) {
        LOG_W("Parse, [cjTaskId] == NULL");
        goto l_exit;
    }
    cJSON *cjPayload = cJSON_GetObjectItemCaseSensitive(cjRoot, "payload");
    if (cjPayload == NULL) {
        LOG_W("Parse, [cjPayload] == NULL");
        goto l_exit;
    }
    cJSON *cjToken = cJSON_GetObjectItemCaseSensitive(cjPayload, "token");
    if (cjToken == NULL) {
        LOG_W("Parse, [cjToken] == NULL");
        goto l_exit;
    }
    cJSON *cjMd5 = cJSON_GetObjectItemCaseSensitive(cjPayload, "md5");
    if (cjMd5 == NULL) {
        LOG_W("Parse, [cjMd5] == NULL");
        goto l_exit;
    }
    cJSON *cjVersion = cJSON_GetObjectItemCaseSensitive(cjPayload, "version");
    if (cjMd5 == NULL) {
        LOG_W("Parse, [cjVersion] == NULL");
        goto l_exit;
    }
    cJSON *cjUrl = cJSON_GetObjectItemCaseSensitive(cjPayload, "url");
    if (cjUrl == NULL) {
        LOG_W("Parse, [cjUrl] == NULL");
        goto l_exit;
    }

    cModule_OtaRequest_t *ota = calloc(1, sizeof(cModule_OtaRequest_t));
    ASSERT(ota != NULL);

    ota->url = calloc(1, 256);
    ASSERT(ota->url != NULL);
    klPtf_sprintf(ota->url, "%s&deviceId=%s%s&accessToken=%s",
                  cjUrl->valuestring, CONFIG_AEP_MQTT_CLIENTID, ins->modemInfo.imei, cjToken->valuestring);

    ota->taskId = cjTaskId->valueint;

    ota->md5 = calloc(1, strlen(cjMd5->valuestring) + 1);
    ASSERT(ota->md5 != NULL);
    strcpy(ota->md5, cjMd5->valuestring);

    ota->version = calloc(1, strlen(cjVersion->valuestring) + 1);
    ASSERT(ota->version != NULL);
    strcpy(ota->version, cjVersion->valuestring);

    if (CREQUEST(OTA_ON_REQUEST, { .ptr = ota }).i32 != 0) {
        free(ota->version);
        free(ota->md5);
        free(ota->url);
        free(ota);
    }

    rc = 0;

    l_exit:
    if (rc != 0) {
        LOG_W("RxCmdFormattingError");
    }
    if (cjRoot != NULL) {
        cJSON_Delete(cjRoot);
    }

    return rc;
}


void cModule_aepOtaRequestFree(cModule_OtaRequest_t *ota) {
    free(ota->version);
    free(ota->md5);
    free(ota->url);
    free(ota);
}


int32_t cModule_otaHttpGet(cModule_Instance_t *ins, cModule_ReqId_HttpGetReq_t *req) {
    ASSERT(ins != NULL);
    ASSERT(req != NULL);

    if (ins->aux.flag.pollBlock || ins->aux.flag.passiveRecvMode) {
        return -1;
    }

    cModule_TransmitPackageInfo_t *pack = NULL;
    if (!_cModule_isAllocReqPackAccess(ins)) {
        return -1;
    }
    pack = calloc(1, sizeof(cModule_TransmitPackageInfo_t));
    ASSERT(pack != NULL);
    pack->payload = req;
    pack->flag.requestId = TRANSMIT_PACK_REQ_ID_HTTP_GET;
    pack->aux.gen.msgId = (*_cModule_getGenMsgId())++;
    klist_init(&pack->list);
    klist_addTail(&ins->transmit.packageReqList, &pack->list);
    ins->transmit.queneReqCount++;

    return pack->aux.gen.msgId;
}


// step=-1, 升级失败
// step=-2, 下载失败
// step=-3, 校验失败
// step=-4, 烧写失败
int32_t cModule_aepOtaReportProcess(cModule_Instance_t *ins, uint32_t taskId, int32_t step, const char *desc) {
    ASSERT(ins != NULL);
    if (ins->aux.flag.pollBlock || ins->aux.flag.passiveRecvMode) {
        return -1;
    }

    cJSON *cjRoot = NULL;
    cModule_TransmitPackageInfo_t *pack = NULL;

    _cModule_packMutexLock(ins, true);
    pack = calloc(1, sizeof(cModule_TransmitPackageInfo_t));
    ASSERT(pack != NULL);

    cjRoot = cJSON_CreateObject();
    ASSERT(cjRoot != NULL);
    cJSON_AddNumberToObject(cjRoot, "taskId", taskId);
    cJSON *cjPars = cJSON_AddObjectToObject(cjRoot, "params");
    ASSERT(cjPars != NULL);
    cJSON_AddNumberToObject(cjPars, "step", step);
    cJSON_AddStringToObject(cjPars, "module", "MCU");
    cJSON_AddStringToObject(cjPars, "desc", desc);

    cModule_ProtocolMqttMessage_t *msg = calloc(1, sizeof(cModule_ProtocolMqttMessage_t));
    ASSERT(msg != NULL);
    msg->payload = (void *) cjRoot;
    msg->isPayloadJson = true;
    msg->qos = 1;
    msg->topic = "/ota/progress";
    msg->topicConstant = true;
    msg->timeout = 30000;
    
    pack->payload = msg;
    pack->payloadLength = 0;
    pack->flag.packIsDynMqttMsg = 1;
    pack->flag.requestId = TRANSMIT_PACK_REQ_ID_TX;
    pack->aux.gen.msgId = (*_cModule_getGenMsgId())++;
    pack->aux.gen.sendRetryCount = 1;
    pack->aux.gen.sendRetryCountReload = 1;
    pack->aux.gen.sendFailedAfterReboot = 1;
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


int32_t cModule_aepOtaReportInform(cModule_Instance_t *ins, const char *versionStr, const char *moduleStr) {
    ASSERT(ins != NULL);

    if (ins->aux.flag.pollBlock || ins->aux.flag.passiveRecvMode) {
        return -1;
    }

    cJSON *cjRoot = NULL;
    cModule_TransmitPackageInfo_t *pack = NULL;

    _cModule_packMutexLock(ins, true);
    pack = calloc(1, sizeof(cModule_TransmitPackageInfo_t));
    ASSERT(pack != NULL);

    cjRoot = cJSON_CreateObject();
    ASSERT(cjRoot != NULL);
    cJSON_AddNumberToObject(cjRoot, "taskId", -1);
    cJSON *cjPars = cJSON_AddObjectToObject(cjRoot, "params");
    ASSERT(cjPars != NULL);
    cJSON_AddStringToObject(cjPars, "version", versionStr);
    cJSON_AddStringToObject(cjPars, "module", moduleStr);

    cModule_ProtocolMqttMessage_t *msg = calloc(1, sizeof(cModule_ProtocolMqttMessage_t));
    ASSERT(msg != NULL);
    msg->payload = (void *) cjRoot;
    msg->isPayloadJson = true;
    msg->qos = 1;
    msg->topic = "/ota/inform";
    msg->topicConstant = true;
    msg->timeout = 30000;
    
    pack->payload = msg;
    pack->payloadLength = 0;
    pack->flag.packIsDynMqttMsg = 1;
    pack->flag.requestId = TRANSMIT_PACK_REQ_ID_TX;
    pack->aux.gen.msgId = (*_cModule_getGenMsgId())++;
    pack->aux.gen.sendRetryCount = 1;
    pack->aux.gen.sendRetryCountReload = 1;
    pack->aux.gen.sendFailedAfterReboot = 1;
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
 * @addtogroup ProtocolPrivate 
 * @note none
 */

/*@{*/

static int32_t ptJwRequestRecvived(cModule_Instance_t *ins, char *data, uint16_t dl) {
    int32_t rc = -1;
    
    cJSON *cjRoot = cJSON_Parse((char *) data);
    if (cjRoot == NULL) {
        goto l_exit;
    }
    cJSON *cjSid = cJSON_GetObjectItemCaseSensitive(cjRoot, "sid");
    if (cjSid == NULL) {
        LOG_E("Parse, [cjSid] == NULL");
        goto l_exit;
    }
    
    switch (cjSid->valueint) {
        /// @Id: 8101, SetCaliArgs
        case 8101: {
            cJSON *cjVersion = cJSON_GetObjectItemCaseSensitive(cjRoot, "v");
            if (cjVersion == NULL) {
                LOG_E("Parse, [cjVersion] == NULL");
                goto l_exit;
            }
            
            switch (cjVersion->valueint) {
                case 1: {
                    cJSON *cjCycleSec = cJSON_GetObjectItemCaseSensitive(cjRoot, "cys");
                    if (cjSid == NULL) {
                        LOG_E("Parse, [cjCycleSec] == NULL");
                        goto l_exit;
                    }
                    cJSON *cjCheckPointSec = cJSON_GetObjectItemCaseSensitive(cjRoot, "cps");
                    if (cjSid == NULL) {
                        LOG_E("Parse, [cjCheckPointSec] == NULL");
                        goto l_exit;
                    }
                    cJSON *cjCloseSec = cJSON_GetObjectItemCaseSensitive(cjRoot, "cs");
                    if (cjSid == NULL) {
                        LOG_E("Parse, [cjCloseSec] == NULL");
                        goto l_exit;
                    }
                    cJSON *cjOpenSec = cJSON_GetObjectItemCaseSensitive(cjRoot, "os");
                    if (cjSid == NULL) {
                        LOG_E("Parse, [cjOpenSec] == NULL");
                        goto l_exit;
                    }

                    cModule_EcDoRequstCaliDataArgsV1_t args = {
                        .poleCloseSec = cjCloseSec->valueint,
                        .poleCycleSec = cjCycleSec->valueint,
                        .poleOpenSec = cjOpenSec->valueint,
                        .poleReadPointSec = cjCheckPointSec->valueint,
                    };
                    CREQUEST(EC_DO_SET_CALI_DATA, {.ptr = &args});
                    LOG_I("8101, SetPioCaliData");
                } break;
            }
        } break;
        
        /// @Id: 8102, SetMeasureAlg
        case 8102: {
            cJSON *cjVersion = cJSON_GetObjectItemCaseSensitive(cjRoot, "v");
            if (cjVersion == NULL) {
                LOG_E("Parse, [cjVersion] == NULL");
                goto l_exit;
            }
            
            switch (cjVersion->valueint) {
                case 1: {
                    cJSON *cjAlg = cJSON_GetObjectItemCaseSensitive(cjRoot, "alg");
                    if (cjAlg == NULL) {
                        LOG_E("Parse, [cjAlg] == NULL");
                        goto l_exit;
                    }

                    cModule_EcDoRequstAlgControlV1_t args = {
                        .stillWaterMode = cjAlg->valueint,
                    };
                    CREQUEST(EC_DO_SET_MEASURE_ALG, {.ptr = &args});
                    LOG_I("8101, SetPioMeasureAlg");
                } break;
            }
        } break;
        
        /// @Id: 8103, SetSalt
        case 8103: {
            cJSON *cjVersion = cJSON_GetObjectItemCaseSensitive(cjRoot, "v");
            if (cjVersion == NULL) {
                LOG_E("Parse, [cjVersion] == NULL");
                goto l_exit;
            }
            
            switch (cjVersion->valueint) {
                case 1: {
                    cJSON *cjSaltMgl = cJSON_GetObjectItemCaseSensitive(cjRoot, "si");
                    if (cjSaltMgl == NULL) {
                        LOG_E("Parse, [cjSaltMgl] == NULL");
                        goto l_exit;
                    }

                    cModule_EcDoRequstSaltControlV1_t args = {
                        .saltMgl = cjSaltMgl->valueint,
                    };
                    CREQUEST(EC_DO_SET_SALT, {.ptr = &args});
                    LOG_I("8103, SetPioSalt");
                } break;
            }
        } break;
        
        case 8200:
        case 8300:
        case 8100: {
            cJSON *cjVersion = cJSON_GetObjectItemCaseSensitive(cjRoot, "v");
            if (cjVersion == NULL) {
                LOG_E("Parse, [cjVersion] == NULL");
                goto l_exit;
            }
            
            switch (cjVersion->valueint) {
                case 1: {
                    uint8_t storageSwitch = 0;
                    cJSON *cjCaliStorageSwitch = cJSON_GetObjectItemCaseSensitive(cjRoot, "sw");
                    if (cjCaliStorageSwitch == NULL) {
                        LOG_E("Parse, [cjCaliStorageSwitch] == NULL, storageSwitch = 0");
                    } else {
                        storageSwitch = cjCaliStorageSwitch->valueint;
                    }
                    
                    cJSON *cjCaliItem = cJSON_GetObjectItemCaseSensitive(cjRoot, "ci");
                    if (cjCaliItem == NULL) {
                        LOG_E("Parse, [cjCaliItem] == NULL");
                        goto l_exit;
                    }
                    CREQUEST(EC_X_CALI_START, {
                        .u8[0] = cjCaliItem->valueint,
                        .u8[1] = storageSwitch,
                    });
                    LOG_I("%d, SetAnalogCali", cjSid->valueint);
                } break;
            }
        } break;
    }

    rc = 0;
    
    l_exit:
    if (cjRoot != NULL) {
        cJSON_Delete(cjRoot);
    }
    
    return rc;
}

/*@}*/

/**
 * @addtogroup ProtocolRequest
 * @note none
 */

/*@{*/

int32_t cModule_ptReportNor(cModule_Instance_t *ins, void *cjRoot) {
    CMASSERT(cjRoot != NULL);
    CMASSERT(ins != NULL);
    
    if (ins->aux.flag.pollBlock || ins->aux.flag.passiveRecvMode) {
        return -1;
    }
    
    if (!_cModule_isAllocReqPackAccess(ins)) {
        return -1;
    }


    int32_t rc = -1;
    cModule_TransmitPackageInfo_t *pack = NULL;

    _cModule_packMutexLock(ins, true);

    pack = calloc(1, sizeof(cModule_TransmitPackageInfo_t));
    if (pack == NULL) {
        LOG_W("%s, pack == NULL", __FUNCTION__);
        goto l_exit;
    }
    uint16_t msgId = (*_cModule_getGenMsgId())++;
    cJSON_AddNumberToObject(cjRoot, "v", 1);
    cJSON_AddNumberToObject(cjRoot, "mid", msgId);
    
    cModule_ProtocolMqttMessage_t *msg = calloc(1, sizeof(cModule_ProtocolMqttMessage_t));
    ASSERT(msg != NULL);
    msg->payload = (void *) cjRoot;
    msg->isPayloadJson = true;
    msg->qos = 1;
    msg->topic = "1";
    msg->topicConstant = true;
    msg->timeout = 30000;
    
    pack->payload = msg;
    pack->payloadLength = 0;
    pack->flag.packIsDynMqttMsg = 1;
    pack->flag.requestId = TRANSMIT_PACK_REQ_ID_TX;
    pack->aux.gen.msgId = msgId;
    pack->aux.gen.sendRetryCount = 1;
    pack->aux.gen.sendRetryCountReload = 1;
    pack->aux.gen.sendFailedAfterReboot = 1;
    pack->aux.gen.sendRetrySec = 3;

    pack->aux.gen.hasConfirmFrame = 0;
    pack->aux.gen.waitConfirmFrameTimeoutSec = 0;
    pack->aux.gen.waitConfirmFrameTimeoutCount = 0;
    pack->aux.gen.waitConfirmFrameTimeoutAfterReboot = 0;

    klist_init(&pack->list);
    klist_addTail(&ins->transmit.packageReqList, &pack->list);
    ins->transmit.queneReqCount++;

    rc = msgId;

    l_exit:
    _cModule_packMutexLock(ins, false);
    if (rc < 0) {
        if (cjRoot != NULL) {
            cJSON_Delete(cjRoot);
        }
        if (pack != NULL) {
            free(pack);
        }
    }
    return rc;
}

/*@}*/

/**
 * @addtogroup ProtocolCallback
 * @note none
 */

/*@{*/

void _cModule_onProtocolMqttConnectAlloc(cModule_Instance_t *ins,
                                         char **borkerIp,
                                         char **clientId,
                                         char **userName, char **password) {
    *clientId = calloc(1, 128);
    ASSERT(*clientId);
}


void _cModule_onProtocolMqttConnect(cModule_Instance_t *ins,
                                    char **borkerIp, uint16_t *port,
                                    char **clientId,
                                    char **userName, char **password) {
    klPtf_sprintf(*clientId, "%s%s", CONFIG_AEP_MQTT_CLIENTID, ins->modemInfo.imei);
    *borkerIp = AEP_MQTT_BORKER_ADDRESS;
    *port = AEP_MQTT_BORKER_PORT;
    *userName = CONFIG_AEP_MQTT_USERNAME;
    *password = CONFIG_AEP_MQTT_PASSWORD;
}


void _cModule_onProtocolMqttConnectFree(cModule_Instance_t *ins,
                                        char **borkerIp,
                                        char **clientId,
                                        char **userName, char **password) {
    free(*clientId);
}


uint8_t _cModule_onProtocolMqttSubCount(cModule_Instance_t *ins) {
    return 2;
}


void _cModule_onProtocolMqttSubAlloc(cModule_Instance_t *ins, char **topic, uint8_t index) {
    *topic = calloc(1, 128);
    ASSERT(*topic);
}


void _cModule_onProtocolMqttSub(cModule_Instance_t *ins, char **topic, uint8_t *qos, uint8_t index) {
    *qos = 1;
    if (index == 0) {
        strcpy(*topic, "ota/upgrade");
    } else {
        klPtf_sprintf(*topic, AEP_MQTT_BASE_SUB_TOPIC, ins->modemInfo.imei);
    }
}


void _cModule_onProtocolMqttSubFree(cModule_Instance_t *ins, char **topic, uint8_t index) {
    free(*topic);
}


int32_t _cModule_onProtocolTransmited(cModule_Instance_t *ins, cModule_TransmitPackageInfo_t *info) {
    cModule_ProtocolMqttMessage_t *msg = info->payload;
    cJSON *cjValue = (void *) msg->payload;
        
    if (cJSON_GetObjectItemCaseSensitive(cjValue, "imsi") != NULL) {
        return 0;
    }
    char *tmpBuffer = calloc(1, 32);
    ASSERT(tmpBuffer != NULL);

    klPtf_sprintf(tmpBuffer, "\"%s\"", ins->modemInfo.imsi);
    ASSERT(cJSON_AddRawToObject(cjValue, "imsi", tmpBuffer) != NULL);

    klPtf_sprintf(tmpBuffer, "\"%s\"", ins->modemInfo.iccid);
    ASSERT(cJSON_AddRawToObject(cjValue, "iccid", tmpBuffer) != NULL);

    klPtf_sprintf(tmpBuffer, "\"%s\"", ins->modemInfo.imei);
    ASSERT(cJSON_AddRawToObject(cjValue, "imei", tmpBuffer) != NULL);

    klPtf_sprintf(tmpBuffer, "%d", ins->modemInfo.rssi);
    ASSERT(cJSON_AddRawToObject(cjValue, "ri", tmpBuffer) != NULL);

    klPtf_sprintf(tmpBuffer, "%u", (unsigned int) ins->aux.timeStamp);
    ASSERT(cJSON_AddRawToObject(cjValue, "time", tmpBuffer) != NULL);
    
    if (CREQUEST(GET_SN2, {}).u32 == 0 && CREQUEST(GET_SN3, {}).u32 == 0) {
        klPtf_sprintf(tmpBuffer, "\"9%03d%03d%s\"", 
            CREQUEST(GET_SN1, {}).u32, 
            0, 
            &ins->modemInfo.imei[10]);
    } else {
        klPtf_sprintf(tmpBuffer, "\"%04d%04d%04d\"", 
            CREQUEST(GET_SN1, {}).u32, 
            CREQUEST(GET_SN2, {}).u32, 
            CREQUEST(GET_SN3, {}).u32);
    }   
    ASSERT(cJSON_AddRawToObject(cjValue, "id", tmpBuffer) != NULL);

    free(tmpBuffer);

    return 0;
}


int32_t _cModule_onProtocolReceived(cModule_Instance_t *ins, cModule_TransmitPackageInfo_t *info) {
    if (info->flag.packHasVar && info->flag.varIsLengthItem) {
        if (strstr(info->var.ptr, "ota/upgrade")) {
            return aepOtaUpgradeReceived(ins, info->payload, info->payloadLength);
        } else {
            return ptJwRequestRecvived(ins, info->payload, info->payloadLength);
        }
    } else {
        return ptJwRequestRecvived(ins, info->payload, info->payloadLength);
    }
    
    return -1;
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

/**
 * @addtogroup Debug support
 * @note none
 */
 
/*@{*/

#undef DBG_SECTION_NAME

/*@}*/

#endif
