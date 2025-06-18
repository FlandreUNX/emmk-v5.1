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

#if CONFIG_CMODULE_PROTOCOL_IS_BCPT_HTTP == 1
 
/**
 * @addtogroup Debug support
 * @note none
 */
 
/*@{*/

#undef DBG_SECTION_NAME
#define DBG_SECTION_NAME  "cModule-pt-bc"

#define CASSERT(err) ASSERT(err)

/*@}*/

/**
 * @addtogroup ComponentSupport
 * @note none
 */
 
/*@{*/

#define _MODULE_REQUEST_TYPE(x)           CMODULE_REQ_##x
#define _MODULE_REQUEST_VARS              cModule_RequestVar_t
#define _MODULE_REQUEST_TYPED             cModule_RequestType_t
#define _MODULE_REQUEST_CALL              cModule_onRequestCallback
#define CREQUEST(type, var, ...)        _MODULE_REQUEST_CALL(_MODULE_REQUEST_TYPE(type), (_MODULE_REQUEST_VARS) var, ##__VA_ARGS__)
extern _MODULE_REQUEST_VARS _MODULE_REQUEST_CALL(_MODULE_REQUEST_TYPED type, _MODULE_REQUEST_VARS var, ...);

/*@}*/

/**
 * @addtogroup Define
 * @note none
 */

/*@{*/

typedef struct {
    uint32_t timestamp;
    cModule_Instance_t *ins;
    ProtocolBc_t *protocol;
    bool isWriten: 1;
    klist_t list;
} PtBufferCollection_t;

static const char *DEVICE_STATUS_STR[] = {
    "PREHEAT", "NORMAL", "POWER DOWN"
};
static const char *SENSOR_STATUS_STR[] = {
    "SELF TEST", "PREHEAT", "NORMAL", "LOW ALARM", "HIGH ALARM", "OVER LOAD", "SENSOR ERROR", "COMM ERROR", "POWER DOWN", "OVER TIME", "INVALID", "OTHER"
};

#define DIRECT_WRITE      rilat_directWritePhy(instance, (uint8_t *) &buffer[0], len); LOG_I("STR-> %s", buffer);
#define HEX_WRITE         memset(&buffer[128], 0x00, 128); len = klStr_hex2str((uint8_t *) buffer, len, &buffer[128]); rilat_directWritePhy(instance, (uint8_t *) &buffer[128], len); LOG_I("HEX-> %s", buffer);
#define LOG_WRITE         LOG_I("STR-> %s", buffer);

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

static uint32_t protocolGetPayloadLength(cModule_Instance_t *ins, ProtocolBc_t *protocol, uint16_t *contentLength, uint32_t tsRaw) {
    char *buffer = calloc(1, 128);
    ASSERT(buffer != NULL);
    uint16_t len = 0;
    
    uint32_t ts = tsRaw + SECS_PER_HOUR * 8;
    klDateTime_SampleTm_t tm;
    klDateTime_bktime(ts, &tm);
    
    len += klPtf_sprintf(buffer, "{");  // 1
        len += klPtf_sprintf(buffer, "\"event\":\"dataChanged\","); LOG_WRITE;
        len += klPtf_sprintf(buffer, "\"termId\":\"999999\","); LOG_WRITE;
        len += klPtf_sprintf(buffer, "\"deviceCode\":\"%s\",", ins->modemInfo.imei); LOG_WRITE;
        len += klPtf_sprintf(buffer, "\"deviceType\":\"9999\","); LOG_WRITE;
        len += klPtf_sprintf(buffer, "\"eventTime\":\"%02d-%02d-%02d %02d:%02d:%02d\",", tm.year, tm.mon, tm.day, tm.hour, tm.min, tm.sec); LOG_WRITE;
        len += klPtf_sprintf(buffer, "\"payload\":\"{"); LOG_WRITE;    // 2
            len += klPtf_sprintf(buffer, "\\\"softworkVersion\\\":\\\"1\\\","); LOG_WRITE;
            len += klPtf_sprintf(buffer, "\\\"hardworkVersion\\\":\\\"1\\\","); LOG_WRITE;
            len += klPtf_sprintf(buffer, "\\\"deviceType\\\":\\\"CA-2100D\\\","); LOG_WRITE;
            len += klPtf_sprintf(buffer, "\\\"wirelessMode\\\":\\\"NBIot\\\","); LOG_WRITE;
            len += klPtf_sprintf(buffer, "\\\"ssid\\\":%d,", ins->modemInfo.rssi); LOG_WRITE;
            len += klPtf_sprintf(buffer, "\\\"uploadCycle\\\":3600,"); LOG_WRITE;
            len += klPtf_sprintf(buffer, "\\\"ccid\\\":\\\"%s\\\",", ins->modemInfo.iccid); LOG_WRITE;
            len += klPtf_sprintf(buffer, "\\\"imei\\\":\\\"%s\\\",", ins->modemInfo.imei); LOG_WRITE;
            len += klPtf_sprintf(buffer, "\\\"dataTime\\\":\\\"%02d-%02d-%02d %02d:%02d:%02d\\\",", tm.year, tm.mon, tm.day, tm.hour, tm.min, tm.sec); LOG_WRITE;
            len += klPtf_sprintf(buffer, "\\\"dataStatus\\\":\\\"%s\\\",", DEVICE_STATUS_STR[protocol->deviceStatus]); LOG_WRITE;
            len += klPtf_sprintf(buffer, "\\\"companyCode\\\":\\\"008\\\","); LOG_WRITE;
            if (protocol->sensorCount != 0) {
                len += klPtf_sprintf(buffer, "\\\"sensorTotalNumber\\\":%d,", protocol->sensorCount); LOG_WRITE;
                len += klPtf_sprintf(buffer, "\\\"sensorData\\\":["); LOG_WRITE;    // 3
                for (uint8_t i = 0; i < protocol->sensorCount; i++) {
                    len += klPtf_sprintf(buffer, "{\\\"addr\\\":%d,",  protocol->sensor[i].addr); LOG_WRITE;
                    len += klPtf_sprintf(buffer, "\\\"name\\\":\\\"CH4\\\","); LOG_WRITE;
                    len += klPtf_sprintf(buffer, "\\\"val\\\":\\\"%d\\\",", protocol->sensor[i].value); LOG_WRITE;
                    len += klPtf_sprintf(buffer, "\\\"unit\\\":\\\"LEL\\\","); LOG_WRITE;
                    len += klPtf_sprintf(buffer, "\\\"status\\\":\\\"%s\\\"}", SENSOR_STATUS_STR[protocol->sensor[i].state]); LOG_WRITE;
                    if (i != protocol->sensorCount - 1) {
                        len += klPtf_sprintf(buffer, ","); LOG_WRITE;
                    }
                }
                len += klPtf_sprintf(buffer, "]"); LOG_WRITE;    // 3
            } else {
                len += klPtf_sprintf(buffer, "\\\"sensorTotalNumber\\\":%d", 0); LOG_WRITE;
            }
        len += klPtf_sprintf(buffer, "}\""); LOG_WRITE;    // 2
    len += klPtf_sprintf(buffer, "}"); LOG_WRITE;    // 1
                
    *contentLength = len;
    
    free(buffer);
                
    return len;
}
    

static void protocolTransmit_bc_directWrite(Rilat_Instance_t *instance) {
    klist_t *pos = NULL, *tmp = NULL;
    klist_forEachSafe(pos, tmp, &mPtBufferList) {
        PtBufferCollection_t *ud = klist_entry(pos, PtBufferCollection_t, list);
        
        char *buffer = calloc(1, 256);
        ASSERT(buffer != NULL);
        uint16_t len = 0;
        
        uint32_t ts = ud->timestamp + SECS_PER_HOUR * 8;
        klDateTime_SampleTm_t tm;
        klDateTime_bktime(ts, &tm);

#if CONFIG_CMODULE_PROTOCOL_BCPT_HTTP_OVER_HEX == 0
        len = klPtf_sprintf(buffer, "{"); DIRECT_WRITE;  // 1
            len = klPtf_sprintf(buffer, "\"event\":\"dataChanged\","); DIRECT_WRITE;
            len = klPtf_sprintf(buffer, "\"termId\":\"999999\","); DIRECT_WRITE;
            len = klPtf_sprintf(buffer, "\"deviceCode\":\"%s\",", ud->ins->modemInfo.imei); DIRECT_WRITE;
            len = klPtf_sprintf(buffer, "\"deviceType\":\"9999\","); DIRECT_WRITE;
            len = klPtf_sprintf(buffer, "\"eventTime\":\"%02d-%02d-%02d %02d:%02d:%02d\",", tm.year, tm.mon, tm.day, tm.hour, tm.min, tm.sec); DIRECT_WRITE;
            len = klPtf_sprintf(buffer, "\"payload\":\"{"); DIRECT_WRITE;    // 2
                len = klPtf_sprintf(buffer, "\\\"softworkVersion\\\":\\\"1\\\","); DIRECT_WRITE;
                len = klPtf_sprintf(buffer, "\\\"hardworkVersion\\\":\\\"1\\\","); DIRECT_WRITE;
                len = klPtf_sprintf(buffer, "\\\"deviceType\\\":\\\"CA-2100D\\\","); DIRECT_WRITE;
                len = klPtf_sprintf(buffer, "\\\"wirelessMode\\\":\\\"NBIot\\\","); DIRECT_WRITE;
                len = klPtf_sprintf(buffer, "\\\"ssid\\\":%d,", ud->ins->modemInfo.rssi); DIRECT_WRITE;
                len = klPtf_sprintf(buffer, "\\\"uploadCycle\\\":3600,"); DIRECT_WRITE;
                len = klPtf_sprintf(buffer, "\\\"ccid\\\":\\\"%s\\\",", ud->ins->modemInfo.iccid); DIRECT_WRITE;
                len = klPtf_sprintf(buffer, "\\\"imei\\\":\\\"%s\\\",", ud->ins->modemInfo.imei); DIRECT_WRITE;
                len = klPtf_sprintf(buffer, "\\\"dataTime\\\":\\\"%02d-%02d-%02d %02d:%02d:%02d\\\",", tm.year, tm.mon, tm.day, tm.hour, tm.min, tm.sec); DIRECT_WRITE;
                len = klPtf_sprintf(buffer, "\\\"dataStatus\\\":\\\"%s\\\",", DEVICE_STATUS_STR[ud->protocol->deviceStatus]); DIRECT_WRITE;
                len = klPtf_sprintf(buffer, "\\\"companyCode\\\":\\\"008\\\","); DIRECT_WRITE;
                if (ud->protocol->sensorCount != 0) {
                    len = klPtf_sprintf(buffer, "\\\"sensorTotalNumber\\\":%d,", ud->protocol->sensorCount); DIRECT_WRITE;
                    len = klPtf_sprintf(buffer, "\\\"sensorData\\\":["); DIRECT_WRITE;    // 3
                    for (uint8_t i = 0; i < ud->protocol->sensorCount; i++) {
                        len = klPtf_sprintf(buffer, "{\\\"addr\\\":%d,",  ud->protocol->sensor[i].addr); DIRECT_WRITE;
                        len = klPtf_sprintf(buffer, "\\\"name\\\":\\\"CH4\\\","); DIRECT_WRITE;
                        len = klPtf_sprintf(buffer, "\\\"val\\\":\\\"%d\\\",", ud->protocol->sensor[i].value); DIRECT_WRITE;
                        len = klPtf_sprintf(buffer, "\\\"unit\\\":\\\"LEL\\\","); DIRECT_WRITE;
                        len = klPtf_sprintf(buffer, "\\\"status\\\":\\\"%s\\\"}", SENSOR_STATUS_STR[ud->protocol->sensor[i].state]); DIRECT_WRITE;
                        if (i != ud->protocol->sensorCount - 1) {
                            len = klPtf_sprintf(buffer, ","); DIRECT_WRITE;
                        }
                    }
                    len = klPtf_sprintf(buffer, "]"); DIRECT_WRITE;    // 3
                } else {
                    len = klPtf_sprintf(buffer, "\\\"sensorTotalNumber\\\":%d", 0); DIRECT_WRITE;
                }
            len = klPtf_sprintf(buffer, "}\""); DIRECT_WRITE;    // 2
        len = klPtf_sprintf(buffer, "}"); DIRECT_WRITE;    // 1
#else
        len = klPtf_sprintf(buffer, "{"); HEX_WRITE;  // 1
            len = klPtf_sprintf(buffer, "\"event\":\"dataChanged\","); HEX_WRITE;
            len = klPtf_sprintf(buffer, "\"termId\":\"999999\","); HEX_WRITE;
            len = klPtf_sprintf(buffer, "\"deviceCode\":\"%s\",", ud->ins->modemInfo.imei); HEX_WRITE;
            len = klPtf_sprintf(buffer, "\"deviceType\":\"9999\","); HEX_WRITE;
            len = klPtf_sprintf(buffer, "\"eventTime\":\"%02d-%02d-%02d %02d:%02d:%02d\",", tm.year, tm.mon, tm.day, tm.hour, tm.min, tm.sec); HEX_WRITE;
            len = klPtf_sprintf(buffer, "\"payload\":\"{"); HEX_WRITE;    // 2
                len = klPtf_sprintf(buffer, "\\\"softworkVersion\\\":\\\"1\\\","); HEX_WRITE;
                len = klPtf_sprintf(buffer, "\\\"hardworkVersion\\\":\\\"1\\\","); HEX_WRITE;
                len = klPtf_sprintf(buffer, "\\\"deviceType\\\":\\\"CA-2100D\\\","); HEX_WRITE;
                len = klPtf_sprintf(buffer, "\\\"wirelessMode\\\":\\\"NBIot\\\","); HEX_WRITE;
                len = klPtf_sprintf(buffer, "\\\"ssid\\\":%d,", ud->ins->modemInfo.rssi); HEX_WRITE;
                len = klPtf_sprintf(buffer, "\\\"uploadCycle\\\":3600,"); HEX_WRITE;
                len = klPtf_sprintf(buffer, "\\\"ccid\\\":\\\"%s\\\",", ud->ins->modemInfo.iccid); HEX_WRITE;
                len = klPtf_sprintf(buffer, "\\\"imei\\\":\\\"%s\\\",", ud->ins->modemInfo.imei); HEX_WRITE;
                len = klPtf_sprintf(buffer, "\\\"dataTime\\\":\\\"%02d-%02d-%02d %02d:%02d:%02d\\\",", tm.year, tm.mon, tm.day, tm.hour, tm.min, tm.sec); HEX_WRITE;
                len = klPtf_sprintf(buffer, "\\\"dataStatus\\\":\\\"%s\\\",", DEVICE_STATUS_STR[ud->protocol->deviceStatus]); HEX_WRITE;
                len = klPtf_sprintf(buffer, "\\\"companyCode\\\":\\\"008\\\","); HEX_WRITE;
                if (ud->protocol->sensorCount != 0) {
                len = klPtf_sprintf(buffer, "\\\"sensorTotalNumber\\\":%d,", ud->protocol->sensorCount); HEX_WRITE;
                    len = klPtf_sprintf(buffer, "\\\"sensorData\\\":["); HEX_WRITE;    // 3
                    for (uint8_t i = 0; i < ud->protocol->sensorCount; i++) {
                        len = klPtf_sprintf(buffer, "{\\\"addr\\\":%d,",  ud->protocol->sensor[i].addr); HEX_WRITE;
                        len = klPtf_sprintf(buffer, "\\\"name\\\":\\\"CH4\\\","); HEX_WRITE;
                        len = klPtf_sprintf(buffer, "\\\"val\\\":\\\"%d\\\",", ud->protocol->sensor[i].value); HEX_WRITE;
                        len = klPtf_sprintf(buffer, "\\\"unit\\\":\\\"LEL\\\","); HEX_WRITE;
                        len = klPtf_sprintf(buffer, "\\\"status\\\":\\\"%s\\\"}", SENSOR_STATUS_STR[ud->protocol->sensor[i].state]); HEX_WRITE;
                        if (i != ud->protocol->sensorCount - 1) {
                            len = klPtf_sprintf(buffer, ","); HEX_WRITE;
                        }
                    }
                } else {
                    len = klPtf_sprintf(buffer, "\\\"sensorTotalNumber\\\":%d", 0); HEX_WRITE;
                }
                len = klPtf_sprintf(buffer, "]"); HEX_WRITE;    // 3
            len = klPtf_sprintf(buffer, "}\""); HEX_WRITE;    // 2
        len = klPtf_sprintf(buffer, "}"); HEX_WRITE;    // 1
#endif
        ud->isWriten = true;
        free(buffer);
        break;
    } 
}


static void protocolTransmit_bc_directWriteResponse(Rilat_Instance_t *instance, bool isSuccess) {
    klist_t *pos = NULL, *tmp = NULL;
    klist_forEachSafe(pos, tmp, &mPtBufferList) {
        PtBufferCollection_t *ud = klist_entry(pos, PtBufferCollection_t, list);
        if (!ud->isWriten) {
            continue;
        }
        klist_delete(&ud->list);
        free(ud->protocol->sensor);
        free(ud->protocol);
        free(ud);
        break;
    }
}
    
/*@}*/

/**
 * @addtogroup ProtocolPublic
 * @note none
 */

/*@{*/

int32_t cModule_protocolTransmit_bc(cModule_Instance_t *ins, ProtocolBc_t *protocol) {
#if CONFIG_CMODULE_INSTANCE_PMU_SUPPORT != 0
    if (ins->state.currentState >= CMODULE_STATE_FAILED_RES) {
        return -1;
    } else if (ins->state.currentState <= CMODULE_STATE_CONING) {
        return 1;
    } else if (ins->state.currentState == CMODULE_STATE_DISCON) {
        cModule_sleep(COMPONENT_SLEEP_LV_RUN);
        return 1;
    }
#endif

    if (!_cModule_isAllocReqPackAccess(ins)) {
        return -1;
    }
    
    int32_t rc = -1;
    cModule_ProtocolHttpMessage_t *msg = calloc(1, sizeof(cModule_ProtocolHttpMessage_t));
    if (msg == NULL) {
        return -1;
    }
    uint32_t ts = ins->aux.timeStamp;
    msg->host = "27.128.224.148";
    msg->hostContant = true;
    msg->port = 23457;
    msg->pathContant = true;
    msg->path = "/";
    protocolGetPayloadLength(ins, protocol, &msg->payloadLength, ts);
    msg->onDirectWrite = (void *) &protocolTransmit_bc_directWrite;
    msg->onDirectWriteResponse = (void *) &protocolTransmit_bc_directWriteResponse;
    
    PtBufferCollection_t *ud = calloc(1, sizeof(PtBufferCollection_t));
    if (ud == NULL) {
        free(msg);
        return -1;
    }
    klist_init(&ud->list);
    ud->ins = ins;
    ud->protocol = protocol;
    ud->timestamp = ts;
    klist_addTail(&mPtBufferList, &ud->list);
    
    _cModule_packMutexLock(ins, true);
    cModule_TransmitPackageInfo_t *pack = calloc(1, sizeof(cModule_TransmitPackageInfo_t));
    if (pack == NULL) {
        free(msg);
        free(ud);
        return -1;
    }

    pack->payloadLength = sizeof(cModule_ProtocolHttpMessage_t);
    pack->payload = msg;
    pack->flag.packIsDynHttpMsg = 1;
    pack->flag.requestId = TRANSMIT_PACK_REQ_ID_TX;
    
    pack->aux.gen.msgId = *_cModule_getGenMsgId();
    pack->aux.gen.sendRetryCount = 0;
    pack->aux.gen.sendRetryCountReload = 0;
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
 * @addtogroup ProtocolReceived
 * @note none
 */

/*@{*/

int32_t _cModule_onProtocolReceived(cModule_Instance_t *ins, cModule_TransmitPackageInfo_t *info) {
    if ((uint32_t ) ins == (uint32_t ) CONFIG_CMODULE_INSTANCE()) {
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
    klist_init(&mPtBufferList);
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
