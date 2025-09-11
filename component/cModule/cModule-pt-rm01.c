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

#include "../global.h"

#include "./cModule-pt-rm01.h"

#if CONFIG_CMODULE_PT_SETTER == CONFIG_CMODULE_PT_SETTER_EW_WIFI

/**
 * @addtogroup DebugSupport
 * @note none
 */

/*@{*/

#undef TAG
#define TAG     "cModule-pt-rm01"

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
#define CREQUEST(type, var, ...)          _MODULE_REQUEST_CALL((uint32_t) _MODULE_REQUEST_TYPE(type), (_MODULE_REQUEST_VARS) var, ##__VA_ARGS__)

extern _MODULE_REQUEST_VARS _MODULE_REQUEST_CALL(_MODULE_REQUEST_TYPED type, _MODULE_REQUEST_VARS var, ...);

/*@}*/

/**
 * @addtogroup Vars
 * @note none
 */

/*@{*/

static uint32_t mMessageId = 0;

/*@}*/

/**
 * @addtogroup ProtocolPrivate
 * @note none
 */

/*@{*/

static void contentSetApMac(cJSON *cjContent) {
    cJSON_AddStringToObject(cjContent, "mac", "");
}


static void contentSetBase(cModule_Instance_t *ins, cJSON *cjContent) {
    cJSON *cjBase = cJSON_AddObjectToObject(cjContent, "b");
    ASSERT(cjBase != NULL);
    contentSetApMac(cjBase);
    
    char *tmpBuffer = calloc(1, 32);
    ASSERT(tmpBuffer != NULL);
    sprintf(tmpBuffer, "\"%s\"", PROJECT_FW_FULL_NAME);
    ASSERT(cJSON_AddRawToObject(cjContent, "fw", tmpBuffer) != NULL);
    
    CREQUEST(PT_GET_SN, { .ptr = tmpBuffer });
    ASSERT(cJSON_AddStringToObject(cjBase, "sn", tmpBuffer) != NULL);

    klPtf_sprintf(tmpBuffer, "\"%s\"", ins->modemInfo.imsi);
    ASSERT(cJSON_AddRawToObject(cjBase, "imsi", tmpBuffer) != NULL);

    klPtf_sprintf(tmpBuffer, "\"%s\"", ins->modemInfo.iccid);
    ASSERT(cJSON_AddRawToObject(cjBase, "iccid", tmpBuffer) != NULL);

    klPtf_sprintf(tmpBuffer, "\"%s\"", ins->modemInfo.imei);
    ASSERT(cJSON_AddRawToObject(cjBase, "imei", tmpBuffer) != NULL);
    
    free(tmpBuffer);
}


static void contentSetMesh(cJSON *cjContent) {
    cJSON *cjMesh = cJSON_AddObjectToObject(cjContent, "m");
    ASSERT(cjMesh != NULL);
    cJSON_AddNumberToObject(cjMesh, "id", CREQUEST(PT_GET_MESH_ID, {}).i32);
    cJSON_AddNumberToObject(cjMesh, "sc", CREQUEST(PT_GET_MESH_STA_COUNT, {}).i32);
    cJSON_AddNumberToObject(cjMesh, "ri", CREQUEST(PT_GET_MESH_RSSI, {}).i32);
    cJSON_AddNumberToObject(cjMesh, "lv", CREQUEST(PT_GET_MESH_LEVEL, {}).i32);
    
    char *mac = calloc(1, 32);
    ASSERT(mac != NULL);
        
    CREQUEST(PT_GET_MESH_PARENT_MAC, { .ptr = mac });
    cJSON_AddStringToObject(cjMesh, "pmac", mac);
    
    CREQUEST(PT_GET_MESH_MAC, { .ptr = mac });
    cJSON_AddStringToObject(cjMesh, "mac", mac);
    
    free(mac);
    
    char *ssid = calloc(1, 32);
    ASSERT(ssid != NULL);
    CREQUEST(PT_GET_CONNECT_SSID, { .ptr = ssid });
    ASSERT(cJSON_AddStringToObject(cjMesh, "ssid", ssid) != NULL);
    free(ssid);
    
    ASSERT(cJSON_AddStringToObject(cjMesh, "cssid", "") != NULL);
}

/*@}*/

/**
 * @addtogroup ProtocolRequest
 * @note none
 */

/*@{*/

static int32_t recvAccess(cModule_Instance_t *ins, char *data, uint16_t dl) {
    cJSON *cjRoot = cJSON_Parse(data);
    if (cjRoot == NULL) {
        LOG_W("Parse, cjRoot");
        goto l_exit;
    }
    int32_t rc = -1;
    cJSON *cjItem = NULL;
    cJSON *cjPayload = NULL;
    cjItem = cJSON_GetObjectItemCaseSensitive(cjRoot, "v");
    if (cjItem == NULL) {
        LOG_W("Parse, [v]");
        goto l_exit;
    }
    if (cjItem->valueint == 241022) {
        LOG_I("RequestVerison=241022");
        
        cjPayload = cJSON_GetObjectItemCaseSensitive(cjRoot, "p");
        if (cjPayload == NULL) {
            LOG_W("Parse, [p]");
            goto l_exit;
        }
        cjItem = cJSON_GetObjectItemCaseSensitive(cjPayload, "sid");
        if (cjItem == NULL) {
            LOG_W("Parse, [sid]");
            goto l_exit;
        }
        if (cjItem->valueint == 8651) {
            LOG_I("RequestSid=8651");
            
            cjItem = cJSON_GetObjectItemCaseSensitive(cjPayload, "rq");
            if (cjItem == NULL) {
                LOG_W("Parse, [rq]");
                goto l_exit;
            }
            LOG_I("RequestId=%d", cjItem->valueint);
            
            if (cjItem->valueint == 0) {
                cModule_response_RM01_v241022(ins, 0, 0);
            } else if (cjItem->valueint == 1) {
                cJSON *cjRqx = cJSON_GetObjectItemCaseSensitive(cjPayload, "rq-1");
                if (cjRqx == NULL) {
                    LOG_W("Parse, [rq-1]");
                    goto l_exit;
                }
                uint8_t index;
                uint8_t set;
                cjItem = cJSON_GetObjectItemCaseSensitive(cjRqx, "n");
                if (cjItem == NULL) {
                    LOG_W("Parse, [rq-1.n]");
                    goto l_exit;
                }
                index = cjItem->valueint;
                cjItem = cJSON_GetObjectItemCaseSensitive(cjRqx, "c");
                if (cjItem == NULL) {
                    LOG_W("Parse, [rq-1.c]");
                    goto l_exit;
                }
                set = cjItem->valueint;
                CREQUEST(PT_RQ1_AC2_SWITCH_CONTROL, { .u16[0] = index, .u16[1] = set });
            } else if (cjItem->valueint == 4) {
                CREQUEST(PT_RQ4_DEVICE_RESET, {});
            } else if (cjItem->valueint == 5) {
                CREQUEST(PT_RQ5_ERROR_RESET, {});
            } else if (cjItem->valueint == 6) {
                cJSON *cjRqx = cJSON_GetObjectItemCaseSensitive(cjPayload, "rq-6");
                if (cjRqx == NULL) {
                    LOG_W("Parse, [rq-6]");
                    goto l_exit;
                }
                cJSON *cjSsid = cJSON_GetObjectItemCaseSensitive(cjRqx, "ssid");
                cJSON *cjPwd = cJSON_GetObjectItemCaseSensitive(cjRqx, "pwd");
                if (cjSsid == NULL || cjPwd == NULL) {
                    LOG_W("Parse, [rq-6.ssid/pwd]");
                    goto l_exit;
                }
                cModule_Rq_SetPair_t setPair = {
                        .ssid = cjSsid->valuestring,
                        .pwd = cjPwd->valuestring
                };
                CREQUEST(PT_RQ6_SET_PAIR, { .ptr = &setPair });
            } else if (cjItem->valueint == 7) {
                cJSON *cjRqx = cJSON_GetObjectItemCaseSensitive(cjPayload, "rq-7");
                if (cjRqx == NULL) {
                    LOG_W("Parse, [rq-7]");
                    goto l_exit;
                }
                cJSON *cjHold = cJSON_GetObjectItemCaseSensitive(cjRqx, "hold");
                if (cjHold == NULL) {
                    LOG_W("Parse, [rq-7.hold]");
                    goto l_exit;
                }
                CREQUEST(PT_RQ7_AC2_SET_SWITCH_HOLD, { .u32 = cjHold->valueint });
            } else if (cjItem->valueint == 8) {
                CREQUEST(PT_RQ8_OTA_ROLLBACK, {});
            } else if (cjItem->valueint == 13) {
                cJSON *cjRqx = cJSON_GetObjectItemCaseSensitive(cjPayload, "rq-13");
                if (cjRqx == NULL) {
                    LOG_W("Parse, [rq-13]");
                    goto l_exit;
                }
                cJSON *cjMode = cJSON_GetObjectItemCaseSensitive(cjRqx, "mode");
                if (cjMode == NULL) {
                    LOG_W("Parse, [rq-13.mode]");
                    goto l_exit;
                }
                cJSON *cjSec = cJSON_GetObjectItemCaseSensitive(cjRqx, "sec");
                if (cjSec == NULL) {
                    LOG_W("Parse, [rq-13.sec]");
                    goto l_exit;
                }
                cJSON *cjSpeed = cJSON_GetObjectItemCaseSensitive(cjRqx, "spd");
                if (cjSpeed == NULL) {
                    LOG_W("Parse, [rq-13.spd]");
                    goto l_exit;
                }
                cModule_Rq_Mc1Ctl_t ctl = {
                        .mode = cjMode->valueint,
                        .sec = cjSec->valueint,
                        .speed = cjSpeed->valueint,
                };
                CREQUEST(PT_RQ13_MC1_CONTROL, { .ptr = &ctl });
            } else if (cjItem->valueint == 14) {
                cJSON *cjRqx = cJSON_GetObjectItemCaseSensitive(cjPayload, "rq-14");
                if (cjRqx == NULL) {
                    LOG_W("Parse, [rq-14]");
                    goto l_exit;
                }
                cJSON *cjSpeed = cJSON_GetObjectItemCaseSensitive(cjRqx, "spd");
                if (cjSpeed == NULL) {
                    LOG_W("Parse, [rq-14.spd]");
                    goto l_exit;
                }
                CREQUEST(PT_RQ14_MC1_SET_SPEED, { .u32 = cjSpeed->valueint });
            } else if (cjItem->valueint == 10) {
                cJSON *cjRqx = cJSON_GetObjectItemCaseSensitive(cjPayload, "rq-10");
                if (cjRqx == NULL) {
                    LOG_W("Parse, [rq-10]");
                    goto l_exit;
                }
                cJSON *cjCaliItem = cJSON_GetObjectItemCaseSensitive(cjRqx, "im");
                if (cjCaliItem == NULL) {
                    LOG_W("Parse, [rq-10.im]");
                    goto l_exit;
                }
                cModule_Rq_Pio1Cali_t cali = {
                        .mode = cjCaliItem->valueint,
                };
                CREQUEST(PT_RQ10_PIO1_CALI_DO, { .ptr = &cali });
            } else if (cjItem->valueint == 11) {
                cJSON *cjRqx = cJSON_GetObjectItemCaseSensitive(cjPayload, "rq-11");
                if (cjRqx == NULL) {
                    LOG_W("Parse, [rq-11]");
                    goto l_exit;
                }
                cJSON *cjCaliItem = cJSON_GetObjectItemCaseSensitive(cjRqx, "im");
                if (cjCaliItem == NULL) {
                    LOG_W("Parse, [rq-11.im]");
                    goto l_exit;
                }
                cModule_Rq_Pio1Cali_t cali = {
                        .mode = cjCaliItem->valueint,
                };
                CREQUEST(PT_RQ11_PIO1_CALI_PH, { .ptr = &cali });
            } else if (cjItem->valueint == 12) {
                cJSON *cjRqx = cJSON_GetObjectItemCaseSensitive(cjPayload, "rq-12");
                if (cjRqx == NULL) {
                    LOG_W("Parse, [rq-12]");
                    goto l_exit;
                }
                cJSON *cjCaliItem = cJSON_GetObjectItemCaseSensitive(cjRqx, "im");
                if (cjCaliItem == NULL) {
                    LOG_W("Parse, [rq-12.im]");
                    goto l_exit;
                }
                cJSON *cjCaliValue = cJSON_GetObjectItemCaseSensitive(cjRqx, "cv");
                if (cjCaliItem == NULL) {
                    LOG_W("Parse, [rq-12.cv]");
                    goto l_exit;
                }
                cModule_Rq_Pio1Cali_t cali = {
                        .mode = cjCaliItem->valueint,
                        .var1.i32 = cjCaliValue->valueint
                };
                CREQUEST(PT_RQ12_PIO1_CALI_ORP, { .ptr = &cali });
            } else if (cjItem->valueint == 15) {
                cJSON *cjRqx = cJSON_GetObjectItemCaseSensitive(cjPayload, "rq-15");
                if (cjRqx == NULL) {
                    LOG_W("Parse, [rq-15]");
                    goto l_exit;
                }
                cJSON *cjAlg = cJSON_GetObjectItemCaseSensitive(cjRqx, "alg");
                if (cjAlg == NULL) {
                    LOG_W("Parse, [rq-15.alg]");
                    goto l_exit;
                }
                CREQUEST(PT_RQ15_PIO1_STILL_MEAUSRE, { .u32 = cjAlg->valueint });
            } else if (cjItem->valueint == 16) {
                cJSON *cjRqx = cJSON_GetObjectItemCaseSensitive(cjPayload, "rq-16");
                if (cjRqx == NULL) {
                    LOG_W("Parse, [rq-16]");
                    goto l_exit;
                }
                cJSON *cjSi = cJSON_GetObjectItemCaseSensitive(cjRqx, "si");
                if (cjSi == NULL) {
                    LOG_W("Parse, [rq-16.si]");
                    goto l_exit;
                }
                CREQUEST(PT_RQ16_PIO1_SALT_OFFSET, { .u32 = cjSi->valueint });
            }
        }
    }
    
    rc = 0;
    
    l_exit:
    return rc;
}

int32_t cModule_response_RM01_v241022(cModule_Instance_t *ins, int8_t rq, int32_t result) {
    int32_t rc = -1;
    if (!_cModule_isAllocReqPackAccess(ins)) {
        LOG_W("Alloc req pack failed");
        return -1;
    }
    _cModule_packMutexLock(ins, true);
    
    cJSON *cjRoot = cJSON_CreateObject();
    ASSERT(cjRoot != NULL);
    
    /// @perfix
    cJSON_AddNumberToObject(cjRoot, "v", 241022);
    uint16_t msgId = mMessageId++;
    cJSON_AddNumberToObject(cjRoot, "mid", msgId);
    
    /// @base
    contentSetBase(ins, cjRoot);
    contentSetMesh(cjRoot);
    
    /// @payload
    cJSON *cjPl = cJSON_AddObjectToObject(cjRoot, "p");
    ASSERT(cjPl != NULL);
    cJSON_AddNumberToObject(cjPl, "sid", 8651);
    cJSON_AddNumberToObject(cjPl, "rq", rq);
    cJSON_AddNumberToObject(cjPl, "rc", result);

    cModule_ProtocolMqttMessage_t *msg = calloc(1, sizeof(cModule_ProtocolMqttMessage_t));
    ASSERT(msg != NULL);
    msg->payload = (void *) cjRoot;
    msg->isPayloadJson = true;
    msg->qos = 1;
    msg->topic = "1";
    msg->topicConstant = true;
    msg->timeout = 30000;
    cModule_TransmitPackageInfo_t *pack = NULL;
    pack = calloc(1, sizeof(cModule_TransmitPackageInfo_t));
    ASSERT(pack != NULL);
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
    return rc;
}


int32_t cModule_reportData_RM01_online_v241022(cModule_Instance_t *ins) {
    int32_t rc = -1;
    if (!_cModule_isAllocReqPackAccess(ins)) {
        LOG_W("Alloc req pack failed");
        return -1;
    }
    _cModule_packMutexLock(ins, true);
    
    cJSON *cjRoot = cJSON_CreateObject();
    ASSERT(cjRoot != NULL);
    
    /// @perfix
    cJSON_AddNumberToObject(cjRoot, "v", 241022);
    uint16_t msgId = mMessageId++;
    cJSON_AddNumberToObject(cjRoot, "mid", msgId);
    
    /// @base
    contentSetBase(ins, cjRoot);
    contentSetMesh(cjRoot);
    
    /// @payload
    cJSON *cjPl = cJSON_AddObjectToObject(cjRoot, "p");
    ASSERT(cjPl != NULL);
    cJSON_AddNumberToObject(cjPl, "sid", 8600);
    
    cModule_ProtocolMqttMessage_t *msg = calloc(1, sizeof(cModule_ProtocolMqttMessage_t));
    ASSERT(msg != NULL);
    msg->payload = (void *) cjRoot;
    msg->isPayloadJson = true;
    msg->qos = 1;
    msg->topic = "1";
    msg->topicConstant = true;
    msg->timeout = 30000;
    cModule_TransmitPackageInfo_t *pack = NULL;
    pack = calloc(1, sizeof(cModule_TransmitPackageInfo_t));
    ASSERT(pack != NULL);
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
    return rc;
}


int32_t cModule_reportData_RM01_data01_v241022(cModule_Instance_t *ins, cModule_RM01_Data01_V1_t *d) {
    int32_t rc = -1;
    if (!_cModule_isAllocReqPackAccess(ins)) {
        LOG_W("Alloc req pack failed");
        return -1;
    }
    _cModule_packMutexLock(ins, true);
    
    cJSON *cjRoot = cJSON_CreateObject();
    ASSERT(cjRoot != NULL);
    
    /// @perfix
    cJSON_AddNumberToObject(cjRoot, "v", 241022);
    uint16_t msgId = mMessageId++;
    cJSON_AddNumberToObject(cjRoot, "mid", msgId);
    
    /// @base
    contentSetBase(ins, cjRoot);
    contentSetMesh(cjRoot);
    
    /// @payload
    cJSON *cjPl = cJSON_AddObjectToObject(cjRoot, "p");
    ASSERT(cjPl != NULL);
    
    char *buf = calloc(1, 16);
    ASSERT(buf != NULL);
    
    cJSON_AddNumberToObject(cjPl, "sid", d->sid);
    if (d->sid == CMODULE_PT_RM01_AC2_SID) {
        klPtf_sprintf(buf, "%.1f", (float) d->ac2.vp_x10 / 10.0f);
        cJSON_AddRawToObject(cjPl, "v", buf);
        
        klPtf_sprintf(buf, "%.1f", (float) d->ac2.amp_x10 / 10.0f);
        cJSON_AddRawToObject(cjPl, "i", buf);
        
        klPtf_sprintf(buf, "%.1f", (float) d->ac2.watt_x10 / 10.0f);
        cJSON_AddRawToObject(cjPl, "w", buf);
        
        cJSON_AddNumberToObject(cjPl, "c", d->ac2.control);
        cJSON_AddNumberToObject(cjPl, "s", d->ac2.status);
        
        klPtf_sprintf(buf, "%.1f", (float) d->ac2.temp_x10 / 10.0f);
        cJSON_AddRawToObject(cjPl, "t", buf);
    } else if (d->sid == CMODULE_PT_RM01_MC1_SID) {
        cJSON_AddNumberToObject(cjPl, "run", d->mc1.runStatus);
        
        klPtf_sprintf(buf, "%.1f", (float) d->mc1.temp_x10 / 10.0f);
        cJSON_AddRawToObject(cjPl, "t", buf);
        
        cJSON_AddNumberToObject(cjPl, "vin", d->mc1.chgStatus);
        cJSON_AddNumberToObject(cjPl, "rsec", d->mc1.runSec);
        cJSON_AddNumberToObject(cjPl, "spd", d->mc1.speed);
    } else if (d->sid == CMODULE_PT_RM01_PIO1_XXX_SID) {
        cJSON_AddNumberToObject(cjPl, "addr", d->pio1.addr);
        if (d->pio1.addr == CMODULE_PT_RM01_PIO1_DO_ADDR) {
            cJSON *cjItem = cJSON_AddObjectToObject(cjPl, "i");
            
            klPtf_sprintf(buf, "%.1f", (float) d->pio1.value.dox.do_x10 / 10.0f);
            cJSON_AddRawToObject(cjItem, "do", buf);
            
            klPtf_sprintf(buf, "%.1f", (float) d->pio1.value.dox.po_x10 / 10.0f);
            cJSON_AddRawToObject(cjItem, "po", buf);
            
            klPtf_sprintf(buf, "%.1f", (float) d->pio1.value.dox.fdo_x10 / 10.0f);
            cJSON_AddRawToObject(cjItem, "fdo", buf);
            
            cJSON_AddNumberToObject(cjItem, "ad", d->pio1.value.dox.ad);
            
            klPtf_sprintf(buf, "%.1f", (float) d->pio1.value.dox.temp_x10 / 10.0f);
            cJSON_AddRawToObject(cjItem, "tp", buf);
            
            cJSON_AddNumberToObject(cjItem, "fad", d->pio1.value.dox.fcAd);
            cJSON_AddNumberToObject(cjItem, "fdet", d->pio1.value.dox.fcDet);
        } else if (d->pio1.addr == CMODULE_PT_RM01_PIO1_ZD_ADDR) {
            cJSON *cjItem = cJSON_AddObjectToObject(cjPl, "i");
            
            klPtf_sprintf(buf, "%.1f", (float) d->pio1.value.zdx.ntu_x10 / 10.0f);
            cJSON_AddRawToObject(cjItem, "ntu", buf);
            
            klPtf_sprintf(buf, "%.1f", (float) d->pio1.value.zdx.temp_x10 / 10.0f);
            cJSON_AddRawToObject(cjItem, "t", buf);
        } else if (d->pio1.addr == CMODULE_PT_RM01_PIO1_PH_ADDR) {
            cJSON *cjItem = cJSON_AddObjectToObject(cjPl, "i");
            
            klPtf_sprintf(buf, "%.2f", (float) d->pio1.value.phx.ph_x100 / 100.0f);
            cJSON_AddRawToObject(cjItem, "ph", buf);
            
            klPtf_sprintf(buf, "%.1f", (float) d->pio1.value.phx.ad_x10 / 10.0f);
            cJSON_AddRawToObject(cjItem, "ad", buf);
            
            klPtf_sprintf(buf, "%.1f", 0);
            cJSON_AddRawToObject(cjItem, "tp", buf);

            cJSON_AddNumberToObject(cjItem, "bsoc", d->pio1.battety.soc);
            cJSON_AddNumberToObject(cjItem, "bmv", d->pio1.battety.volt);
        } else if (d->pio1.addr == CMODULE_PT_RM01_PIO1_ORP_ADDR) {
            cJSON *cjItem = cJSON_AddObjectToObject(cjPl, "i");
            
            klPtf_sprintf(buf, "%.2f", (float) d->pio1.value.orpx.mv_x10 / 10.0f);
            cJSON_AddRawToObject(cjItem, "mv", buf);
            
            klPtf_sprintf(buf, "%.1f", (float) d->pio1.value.orpx.ad_x10 / 10.0f);
            cJSON_AddRawToObject(cjItem, "ad", buf);
            
            klPtf_sprintf(buf, "%.1f", 0);
            cJSON_AddRawToObject(cjItem, "tp", buf);

            cJSON_AddNumberToObject(cjItem, "bsoc", d->pio1.battety.soc);
            cJSON_AddNumberToObject(cjItem, "bmv", d->pio1.battety.volt);
        } else if (d->pio1.addr == CMODULE_PT_RM01_PIO1_YD_ADDR) {
            cJSON *cjItem = cJSON_AddObjectToObject(cjPl, "i");

            klPtf_sprintf(buf, "%.2f", (float) d->pio1.value.ydx.ppt_x100 / 100.0f);
            cJSON_AddRawToObject(cjItem, "ppt", buf);

            klPtf_sprintf(buf, "%.1f", (float) d->pio1.value.ydx.temp_x10 / 10.0f);
            cJSON_AddRawToObject(cjItem, "t", buf);

            cJSON_AddNumberToObject(cjItem, "bsoc", d->pio1.battety.soc);
            cJSON_AddNumberToObject(cjItem, "bmv", d->pio1.battety.volt);
        }
    }
    
    free(buf);
    
    cModule_ProtocolMqttMessage_t *msg = calloc(1, sizeof(cModule_ProtocolMqttMessage_t));
    ASSERT(msg != NULL);
    msg->payload = (void *) cjRoot;
    msg->isPayloadJson = true;
    msg->qos = 1;
    msg->topic = "1";
    msg->topicConstant = true;
    msg->timeout = 30000;
    cModule_TransmitPackageInfo_t *pack = NULL;
    pack = calloc(1, sizeof(cModule_TransmitPackageInfo_t));
    ASSERT(pack != NULL);
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
    return rc;
}

/*@}*/

/**
 * @addtogroup ProtocolCallback
 * @note none
 */

/*@{*/

#define AEP_MQTT_BORKER_ADDRESS            "mqtt.ctwing.cn"
#define AEP_MQTT_BORKER_PORT               1883
#define AEP_MQTT_BASE_SUB_TOPIC            "mqtt/"CONFIG_AEP_MQTT_VSTORE_ID"/"CONFIG_AEP_MQTT_CLIENTID"/"CONFIG_AEP_MQTT_CLIENTID"%s/dn/device_control"

#if CONFIG_CMODULE_PT_SETTER_EW_WIFI_MODE_4G == 1
#define CONFIG_AEP_MQTT_VSTORE_ID          "2000117012"    // 租户ID
#define CONFIG_AEP_MQTT_CLIENTID           "17154312"      // 产品ID
#define CONFIG_AEP_MQTT_USERNAME           "unx"    // 随机
#define CONFIG_AEP_MQTT_PASSWORD           "HGuOwfVSY0c3yBRsIjxtxmsqkXh2uaLzQVcpnxY4hUE" // 产品特征串
#endif
#if CONFIG_CMODULE_PT_SETTER_EW_WIFI_MODE_WIFI_MESH == 1
#define CONFIG_AEP_MQTT_VSTORE_ID          "2000117012"    // 租户ID
#define CONFIG_AEP_MQTT_CLIENTID           "17095074"      // 产品ID
#define CONFIG_AEP_MQTT_USERNAME           "unx"    // 随机
#define CONFIG_AEP_MQTT_PASSWORD           "rU8wa7fLXuEmPA2KBA0TAAAwi57ZKQtocQrCC_gnrFk" // 产品特征串
#endif

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
    return 0;
}


int32_t _cModule_onProtocolReceived(cModule_Instance_t *ins, cModule_TransmitPackageInfo_t *info) {
    return recvAccess(ins, info->payload, info->payloadLength);
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
 * @addtogroup DebugSupport
 * @note none
 */

/*@{*/

#undef TAG

/*@}*/

#endif