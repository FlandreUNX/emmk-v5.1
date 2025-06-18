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
    CMODULE_REQ_PT_START = CMODULE_REQ_PT_ID_START + 0,
    
    CMODULE_REQ_PT_GET_DEV_TYPE_ID,
    CMODULE_REQ_PT_GET_MAC,
    CMODULE_REQ_PT_GET_SN1,
    CMODULE_REQ_PT_GET_SN2,
    CMODULE_REQ_PT_GET_SN3,
    CMODULE_REQ_PT_GET_SN4,

    CMODULE_REQ_PT_MESH_ON_PAIR_TIMEOUT,
    CMODULE_REQ_PT_MESH_ON_NODE_JOIN,
    CMODULE_REQ_PT_MESH_ON_NODE_BROADCAST,
    
    CMODULE_REQ_PT_ON_CALI_DO,
    CMODULE_REQ_PT_ON_CALI_PH,
    CMODULE_REQ_PT_ON_CALI_CL,
    CMODULE_REQ_PT_ON_CALI_ZD,
    CMODULE_REQ_PT_ON_CALI_NH,
    CMODULE_REQ_PT_ON_SET_UPDATE_TIME_MIN,
    CMODULE_REQ_PT_ON_WRITE_REGS,
    
    CMODULE_REQ_PT_EC_DO_SET_CALI_DATA,
    CMODULE_REQ_PT_EC_DO_SET_MEASURE_ALG,
    CMODULE_REQ_PT_EC_DO_SET_SALT,
    CMODULE_REQ_PT_EC_X_CALI_START,
    
    CMODULE_REQ_PT_OTA_ON_REQUEST,
} cModule_RequestType_Pt_t;

typedef enum {
    CMODULE_TPACK_RID_PT = TRANSMIT_PACK_REQ_ID_START + 0,
} cModule_TransmitPackageRequestId_Pt_t;

/*@}*/

/**
 * @addtogroup ProtocolDefine
 * @note none
 */
 
/*@{*/

#define CMODULE_PT_REPORT_NOR_SID   8700
#define CMODULE_PT_REQ_SID          8750

typedef struct {
    int32_t rc;
    const char *sn;
    const char *mac;
    uint16_t dti;
} cModule_NodeRepJoin_t;

typedef struct {
    struct {
        uint8_t hasPaired: 1;
        uint8_t hasOnline: 1;
    } flag;
    const char *sn;
    const char *mac;
    uint16_t dti;
    uint32_t aliveTs;
} cModule_NodeRepBroadcast_t;

typedef struct {
    uint8_t addr;
    uint8_t sif;
    uint16_t vl_x10;
} cModule_PtCaliCl_t;

typedef struct {
    uint8_t addr;
    uint8_t sif;
    uint8_t item;
    int16_t os_x10;
} cModule_PtCaliZd_t;

typedef struct {
    uint8_t addr;
    uint8_t sif;
    uint8_t item;
    float value;
} cModule_PtCaliNh_t;

typedef struct {
    uint32_t newUpdateTimeSec;
} cModule_PtReqSetUpdatTime_t;

typedef struct {
    uint8_t addr;
    uint8_t sif;
    uint8_t funcCode;
    uint16_t regAddrStart;
    uint8_t regCount;
    uint16_t *regData;
} cModule_PtWriteReg_t;

/// PIO_EC_DO
typedef struct {
    uint16_t dov_x10;
    uint16_t pov_x10;
    uint16_t adv;
    uint16_t fdov_x10;
    int16_t temp_x10;
    uint8_t batp;
    uint16_t batv;
    uint16_t fcAdv;
    uint8_t fcIsDet;
} cModule_EcDoNormalDataV1_t;
typedef struct {
    int16_t zeroTemp_x10;
    uint16_t zeroAdv;
    int16_t spanTemp_x10;
    uint16_t spanAdv;
    uint16_t poleOpenSec;
    uint16_t poleCloseSec;
    uint16_t poleReadPointSec;
    uint16_t poleCycleSec;
    
    uint8_t algControl;
    
    uint8_t saltMgl;
} cModule_EcDoCaliDataV1_t;
typedef struct {
    uint16_t poleOpenSec;
    uint16_t poleCloseSec;
    uint16_t poleReadPointSec;
    uint16_t poleCycleSec;
} cModule_EcDoRequstCaliDataArgsV1_t;
typedef struct {
    uint8_t stillWaterMode;
} cModule_EcDoRequstAlgControlV1_t;
typedef struct {
    uint8_t saltMgl;
} cModule_EcDoRequstSaltControlV1_t;

typedef struct {
    char *url;
    char *md5;

    char *version;

    uint32_t taskId;
} cModule_OtaRequest_t;

/*@}*/

/**
 * @addtogroup ProtocolRequest
 * @note none
 */
 
/*@{*/

extern int32_t cModule_otaHttpGet(cModule_Instance_t *ins, cModule_ReqId_HttpGetReq_t *req);
extern void cModule_aepOtaRequestFree(cModule_OtaRequest_t *ota);
extern int32_t cModule_aepOtaReportProcess(cModule_Instance_t *ins, uint32_t taskId, int32_t step, const char *desc);

extern int32_t cModule_ptRequestResponseV1(cModule_Instance_t *ins, uint32_t osid, int32_t result);
extern int32_t cModule_ptReportNor(cModule_Instance_t *ins, void *cjPl);


extern void cModule_protocol_init(void);

/*@}*/

#endif
