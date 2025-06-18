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

/**
 * @addtogroup DebugSupport
 * @note none
 */

/*@{*/

#undef TAG
#define TAG     "cModule-pt-wifi_mesh"

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
 * @addtogroup ProtocolConnectDefine
 * @note none
 */

/*@{*/

// DFS_地风升-MQTT-控制器
#define AEP_MQTT_BORKER_ADDRESS            "mqtt.ctwing.cn"
#define AEP_MQTT_BORKER_PORT               1883
#define AEP_MQTT_BASE_SUB_TOPIC            "mqtt/"CONFIG_AEP_MQTT_VSTORE_ID"/"CONFIG_AEP_MQTT_CLIENTID"/"CONFIG_AEP_MQTT_CLIENTID"%s/dn/device_control"

#define CONFIG_AEP_MQTT_VSTORE_ID          "2000117012"    // 租户ID
#define CONFIG_AEP_MQTT_CLIENTID           "17154312"      // 产品ID
#define CONFIG_AEP_MQTT_USERNAME           "unx"    // 随机
#define CONFIG_AEP_MQTT_PASSWORD           "HGuOwfVSY0c3yBRsIjxtxmsqkXh2uaLzQVcpnxY4hUE" // 产品特征串征串

/*@}*/

/**
 * @addtogroup ProtocolPrivate
 * @note none
 */

/*@{*/

static void contentCreateRootBase(cJSON *cjRoot) {
    cJSON *cjContent = cJSON_AddObjectToObject(cjRoot, "b");
    ASSERT(cjContent != NULL);
    
    char *t = calloc(1, 32);
    ASSERT(t != NULL);
    
    if (CREQUEST(PT_BASE_GET_SNX, { .u32 = 1 }).u32 == 0 && CREQUEST(PT_BASE_GET_SNX, { .u32 = 2 }).u32 == 0) {
        klPtf_sprintf(t, "\"900000000000\"");
    } else {
        klPtf_sprintf(t, "\"%04d%04d%04d\"",
                      CREQUEST(PT_BASE_GET_SNX, { .u32 = 0 }).u32,
                      CREQUEST(PT_BASE_GET_SNX, { .u32 = 1 }).u32,
                      CREQUEST(PT_BASE_GET_SNX, { .u32 = 2 }).u32);
    }
    ASSERT(cJSON_AddRawToObject(cjContent, "sn", t) != NULL);
    
    CREQUEST(PT_BASE_GET_FW_VERSION, { .ptr = t });
    ASSERT(cJSON_AddStringToObject(cjContent, "fw", t));
    
    uint8_t mac[6] = {0};
    CREQUEST(PT_BASE_GET_MAC6, { .ptr = mac });
    sprintf(t, "%02X:%02X:%02X:%02X:%02X:%02X", MAC2STR(mac));
    ASSERT(cJSON_AddStringToObject(cjContent, "mac", t));
    
    ASSERT(cJSON_AddStringToObject(cjContent, "imei", cModule_getModuleUId(CONFIG_CMODULE_INSTANCE())));
    ASSERT(cJSON_AddStringToObject(cjContent, "imsi", cModule_getModuleComId(CONFIG_CMODULE_INSTANCE())));
    ASSERT(cJSON_AddStringToObject(cjContent, "iccid", cModule_getModuleIccid(CONFIG_CMODULE_INSTANCE())));
    
    free(t);
}

static void contentCreateMesh(cJSON *cjRoot) {
    cJSON *cjMesh = cJSON_AddObjectToObject(cjRoot, "m");
    ASSERT(cjMesh != NULL);
    
    cJSON_AddNumberToObject(cjMesh, "id", CREQUEST(PT_MESH_GET_ID, {}).i32);
    cJSON_AddNumberToObject(cjMesh, "sc", CREQUEST(PT_MESH_GET_STA_COUNT, {}).i32);
    cJSON_AddNumberToObject(cjMesh, "ri", CREQUEST(PT_MESH_GET_RSSI, {}).i32);
    cJSON_AddNumberToObject(cjMesh, "lv", CREQUEST(PT_MESH_GET_LEVEL, {}).i32);
    cJSON_AddNumberToObject(cjMesh, "gri", cModule_getRssi(CONFIG_CMODULE_INSTANCE()));
    
    char *pmac = calloc(1, 32);
    ASSERT(pmac != NULL);
    CREQUEST(PT_MESH_GET_PMAC, { .ptr = pmac });
    cJSON_AddStringToObject(cjMesh, "pmac", pmac);
    free(pmac);
    
    char *ssid = calloc(1, 32);
    ASSERT(ssid != NULL);
    CREQUEST(PT_MESH_GET_SSID, { .ptr = ssid });
    ASSERT(cJSON_AddStringToObject(cjMesh, "ssid", ssid) != NULL);
    free(ssid);
    
    wifi_ap_record_t ap = {0};
    esp_wifi_sta_get_ap_info(&ap);
    ASSERT(cJSON_AddStringToObject(cjMesh, "cssid", (char *) ap.ssid) != NULL);
}

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

    cModule_PtOtaRequest_t *ota = calloc(1, sizeof(cModule_PtOtaRequest_t));
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

    if (CREQUEST(PT_OTA_ON_REQUEST, { .ptr = ota }).i32 != 0) {
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


void cModule_aepOtaRequestFree(cModule_PtOtaRequest_t *ota) {
    free(ota->version);
    free(ota->md5);
    free(ota->url);
    free(ota);
}


int32_t cModule_ptOtaHttpGet(cModule_Instance_t *ins, cModule_ReqId_HttpGetReq_t *req) {
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
 * @addtogroup ProtocolRequest
 * @note none
 */

/*@{*/

int32_t cModule_pt_phoneCallRequest(cModule_Instance_t *ins, char *number, uint8_t numberCount) {
    ASSERT(ins != NULL);
    
    cModule_TransmitPackageInfo_t *pack = NULL;
    if (!_cModule_isAllocReqPackAccess(ins)) {
        return -1;
    }
    pack = calloc(1, sizeof(cModule_TransmitPackageInfo_t));
    if (pack == NULL) {
        return -1;
    }
    pack->payload = number;
    pack->payloadLength = numberCount;
    pack->flag.requestId = TRANSMIT_PACK_REQ_ID_ATD;
    
    klist_init(&pack->list);
    klist_addTail(&ins->transmit.packageReqList, &pack->list);
    ins->transmit.queneReqCount++;
    
    return 0;
}

int32_t cModule_pt_response(cModule_Instance_t *ins, uint8_t rqCode, int32_t result) {
    ASSERT(ins != NULL);
    if (!_cModule_isAllocReqPackAccess(ins)) {
        return -1;
    }
    
    cJSON *cjRoot = NULL;
    cModule_TransmitPackageInfo_t *pack = NULL;
    
    _cModule_packMutexLock(ins, true);
    pack = calloc(1, sizeof(cModule_TransmitPackageInfo_t));
    ASSERT(pack != NULL);
    
    cjRoot = cJSON_CreateObject();
    ASSERT(cjRoot != NULL);
    
    uint16_t msgId = (*_cModule_getGenMsgId())++;
    
    cJSON_AddNumberToObject(cjRoot, "v", 241022);
    cJSON_AddNumberToObject(cjRoot, "mid", msgId);
    contentCreateRootBase(cjRoot);
    contentCreateMesh(cjRoot);
    
    cJSON *cjPayload = cJSON_AddObjectToObject(cjRoot, "p");
    ASSERT(cjPayload != NULL);
    cJSON_AddNumberToObject(cjPayload, "sid", CMODULE_PT_SID_RESPONSE);
    
    cJSON_AddNumberToObject(cjPayload, "rq", rqCode);
    cJSON_AddNumberToObject(cjPayload, "rc", result);
    
    cModule_ProtocolMqttMessage_t *msg = calloc(1, sizeof(cModule_ProtocolMqttMessage_t));
    ASSERT(msg != NULL);
    msg->payload = cJSON_PrintUnformatted(cjRoot);
    ASSERT(msg->payload != NULL);
    msg->len = strlen(msg->payload);
    msg->isPayloadString = true;
    msg->qos = 1;
    msg->topic = "1";
    msg->topicConstant = true;
    
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
    _cModule_packMutexLock(ins, false);
    
    cJSON_Delete(cjRoot);
    
    return msgId;
}

int32_t cModule_ptNode_reportJson(cModule_Instance_t *ins, cModule_Sid_t sid, cJSON *cjNode) {
    ASSERT(ins != NULL);
    if (!_cModule_isAllocReqPackAccess(ins)) {
        return -1;
    }
    
    cJSON *cjRoot = NULL;
    cModule_TransmitPackageInfo_t *pack = NULL;
    
    _cModule_packMutexLock(ins, true);
    pack = calloc(1, sizeof(cModule_TransmitPackageInfo_t));
    ASSERT(pack != NULL);
    
    cjRoot = cJSON_CreateObject();
    ASSERT(cjRoot != NULL);
    
    uint16_t msgId = (*_cModule_getGenMsgId())++;
    
    cJSON_AddNumberToObject(cjRoot, "v", 241022);
    cJSON_AddNumberToObject(cjRoot, "mid", msgId);
    contentCreateRootBase(cjRoot);
    contentCreateMesh(cjRoot);
    
    cJSON *cjPayload = cJSON_AddObjectToObject(cjRoot, "p");
    ASSERT(cjPayload != NULL);
    cJSON_AddNumberToObject(cjPayload, "sid", sid);
    
    cJSON_AddItemReferenceToObject(cjPayload, "node", cjNode);
    
    cModule_ProtocolMqttMessage_t *msg = calloc(1, sizeof(cModule_ProtocolMqttMessage_t));
    ASSERT(msg != NULL);
    msg->payload = cJSON_PrintUnformatted(cjRoot);
    ASSERT(msg->payload != NULL);
    msg->len = strlen(msg->payload);
    msg->isPayloadString = true;
    msg->qos = 1;
    msg->topic = "1";
    msg->topicConstant = true;

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
    _cModule_packMutexLock(ins, false);
    
    cJSON_Delete(cjRoot);
    
    return msgId;
}

int32_t cModule_pt_reportVin(cModule_Instance_t *ins, bool vinState) {
    ASSERT(ins != NULL);
    if (!_cModule_isAllocReqPackAccess(ins)) {
        return -1;
    }
    
    cJSON *cjRoot = NULL;
    cModule_TransmitPackageInfo_t *pack = NULL;
    
    _cModule_packMutexLock(ins, true);
    pack = calloc(1, sizeof(cModule_TransmitPackageInfo_t));
    ASSERT(pack != NULL);
    
    cjRoot = cJSON_CreateObject();
    ASSERT(cjRoot != NULL);
    
    uint16_t msgId = (*_cModule_getGenMsgId())++;
    
    cJSON_AddNumberToObject(cjRoot, "v", 241022);
    cJSON_AddNumberToObject(cjRoot, "mid", msgId);
    contentCreateRootBase(cjRoot);
    contentCreateMesh(cjRoot);
    
    cJSON *cjPayload = cJSON_AddObjectToObject(cjRoot, "p");
    ASSERT(cjPayload != NULL);
    cJSON_AddNumberToObject(cjPayload, "sid", CMODULE_PT_SID_VIN_REPORT);
    
    cJSON_AddNumberToObject(cjPayload, "vin", vinState);
    
    cModule_ProtocolMqttMessage_t *msg = calloc(1, sizeof(cModule_ProtocolMqttMessage_t));
    ASSERT(msg != NULL);
    msg->payload = cJSON_PrintUnformatted(cjRoot);
    ASSERT(msg->payload != NULL);
    msg->len = strlen(msg->payload);
    msg->isPayloadString = true;
    msg->qos = 1;
    msg->topic = "1";
    msg->topicConstant = true;
    
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
    _cModule_packMutexLock(ins, false);
    
    cJSON_Delete(cjRoot);
    
    return msgId;
}

/*@}*/

/**
 * @addtogroup ProtocolReceived
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
    return 0;
}

int32_t _cModule_onProtocolReceived(cModule_Instance_t *ins, cModule_TransmitPackageInfo_t *info) {
    if (info->flag.packHasVar && info->flag.varIsLengthItem) {
        if (strstr(info->var.ptr, "ota/upgrade")) {
            return aepOtaUpgradeReceived(ins, info->payload, info->payloadLength);
        }
    }

    cJSON *cjRoot = cJSON_Parse((char *) info->payload);
    cJSON *cjPayload = cJSON_GetObjectItemCaseSensitive(cjRoot, "p");
    if (cjRoot == NULL || cjPayload == NULL) {
        goto l_exit;
    }
    cJSON *cjVersion = cJSON_GetObjectItemCaseSensitive(cjRoot, "v");
    cJSON *cjSid = cJSON_GetObjectItemCaseSensitive(cjPayload, "sid");
    cJSON *cjRq = cJSON_GetObjectItemCaseSensitive(cjPayload, "rq");
    if (cjSid == NULL || cjVersion == NULL || cjRq == NULL
        || cjVersion->valueint != 241022
        || cjSid->valueint != CMODULE_PT_SID_REQUEST) {
        goto l_exit;
    }
    cJSON *cjTargetMac = NULL, *cjTargetSn = NULL;
    cJSON *cjForward = cJSON_GetObjectItemCaseSensitive(cjPayload, "forward");
    if (cjForward != NULL) {
        cjTargetMac = cJSON_GetObjectItemCaseSensitive(cjForward, "target-mac");
        cjTargetSn = cJSON_GetObjectItemCaseSensitive(cjForward, "target-sn");
    }
    
    switch (cjRq->valueint) {
        case CMODULE_REQ_CODE_GEN: {
            cModule_pt_response(ins, CMODULE_REQ_CODE_GEN, 0);
            break;
        }
        default: {
            char jsonStr[8] = {0};
            sprintf(jsonStr, "rq-%d", cjRq->valueint);
            cJSON *cjRqPayload = cJSON_GetObjectItemCaseSensitive(cjPayload, jsonStr);
            
            cModule_ReqPayload_t payload = {
                    .forward = {
                            .targetSn = cjTargetSn != NULL ? cjTargetSn->valuestring : NULL,
                    },
                    .reqCode = cjRq->valueint,
                    .pack = cjRqPayload,
            };
            if (cjTargetMac != NULL) {
                char *macStr = calloc(1, 32);
                ASSERT(macStr != NULL);
                strncpy(macStr, cjTargetMac->valuestring, 17);
                klStr_convertMacStr2Hex(macStr, payload.forward.targetMacHex);
                free(macStr);
            }
            CREQUEST(PT_ON_REQUEST_CALL, { .ptr = &payload });
            break;
        }
    }
    
    l_exit:
    if (cjRoot != NULL) {
        cJSON_Delete(cjRoot);
    }
    return 0;
}

/*@}*/

/**
 * @addtogroup DebugSupport
 * @note none
 */

/*@{*/

#undef TAG

/*@}*/
