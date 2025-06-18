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

#ifndef _CM_MODULE_PT_JW_X_H_
#define _CM_MODULE_PT_JW_X_H_

#include <stdint.h>

#include "./cModule.h"

/**
 * @addtogroup Define
 * @note none
 */

/*@{*/

/*@}*/

/**
 * @addtogroup Define & Typedef
 * @note none
 */

/*@{*/

typedef enum {
    CMODULE_REQ_PT_START = CMODULE_REQ_PT_ID_START,
    
    CMODULE_REQ_PT_BASE_GET_SNX,
    CMODULE_REQ_PT_BASE_GET_FW_VERSION,
    CMODULE_REQ_PT_BASE_GET_MAC6,

    CMODULE_REQ_PT_MESH_GET_ID,
    CMODULE_REQ_PT_MESH_GET_STA_COUNT,
    CMODULE_REQ_PT_MESH_GET_RSSI,
    CMODULE_REQ_PT_MESH_GET_LEVEL,
    CMODULE_REQ_PT_MESH_GET_PMAC,
    CMODULE_REQ_PT_MESH_GET_SSID,
    
    CMODULE_REQ_PT_ON_REQUEST_CALL,

    CMODULE_REQ_PT_OTA_ON_REQUEST,
} cModule_RequestType_Pt_t;

/*@}*/

/**
 * @addtogroup ProtocolDefine
 * @note none
 */

/*@{*/

typedef enum {
    CMODULE_PT_SID_NODE_BROADCAST_REPORT = 8610,
    CMODULE_PT_SID_NODE_PAIR_REPORT = 8608,
    CMODULE_PT_SID_NODE_DATA_REPORT = 8607,
    
    CMODULE_PT_SID_VIN_REPORT = 8609,
    
    CMODULE_PT_SID_REQUEST = 8651,
    CMODULE_PT_SID_RESPONSE = 8651,
} cModule_Sid_t;

typedef enum {
    CMODULE_REQ_CODE_GEN = 0,
    
    CMODULE_REQ_CODE_DEV_RESET_ALL = 4,
    CMODULE_REQ_CODE_DEV_CLEAN_ERROR = 5,
    CMODULE_REQ_CODE_DEV_SET_PAIR_P = 6,
    CMODULE_REQ_CODE_DEV_OTA_ROLLBACK = 8,
    CMODULE_REQ_CODE_DEV_PHONE_CALL = 3,
    
    CMODULE_REQ_CODE_ACX_CONTROL_P = 1,
    CMODULE_REQ_CODE_ACX_SET_PAIR_P = 2,
    CMODULE_REQ_CODE_ACX_SET_DEFAULT_CONTROL_P = 7,
    
    CMODULE_REQ_CODE_MC_CONTROL_P = 13,
    CMODULE_REQ_CODE_MC_SET_SPEED_P = 14,
    
    CMODULE_REQ_CODE_PIO_DO_CALI_P = 10,
    CMODULE_REQ_CODE_PIO_PH_CALI_P = 11,
    CMODULE_REQ_CODE_PIO_ORP_CALI_P = 12,
    CMODULE_REQ_CODE_PIO_SET_MODE_P = 15,
    CMODULE_REQ_CODE_PIO_SET_SALT_P = 16,
    
    CMODULE_REQ_CODE_OTA_ENTER = 17,
} cModule_ReqCode_t;

typedef struct {
    struct {
        char *targetSn;
        uint8_t targetMacHex[6];
    } forward;
    cModule_ReqCode_t reqCode;
    cJSON *pack;
} cModule_ReqPayload_t;

typedef struct {
    char *numbers[5];
    uint8_t numberCount;
} cModule_PhoneNumberCallRequest_t;

typedef struct {
    char *url;
    char *md5;

    char *version;

    uint32_t taskId;
} cModule_PtOtaRequest_t;

/*@}*/

/**
 * @addtogroup ProtocolRequest
 * @note none
 */

/*@{*/

extern int32_t cModule_ptOtaHttpGet(cModule_Instance_t *ins, cModule_ReqId_HttpGetReq_t *req);
extern void cModule_aepOtaRequestFree(cModule_PtOtaRequest_t *ota);
extern int32_t cModule_aepOtaReportProcess(cModule_Instance_t *ins, uint32_t taskId, int32_t step, const char *desc);


extern int32_t cModule_pt_phoneCallRequest(cModule_Instance_t *ins, char *number, uint8_t numberCount);

extern int32_t cModule_pt_response(cModule_Instance_t *ins, uint8_t rqCode, int32_t result);

extern int32_t cModule_ptNode_reportJson(cModule_Instance_t *ins, cModule_Sid_t sid, cJSON *cjNode);

extern int32_t cModule_pt_reportVin(cModule_Instance_t *ins, bool vinState);

/*@}*/

#endif
