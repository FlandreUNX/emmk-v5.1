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

#define CMODULE_PT_GSKL_VERISON_STR      "R241015"

/*@}*/

/**
 * @addtogroup Define & Typedef
 * @note none
 */
 
/*@{*/

typedef enum {
    CMODULE_REQ_PT_START = CMODULE_REQ_PT_ID_START + 0,
    CMODULE_REQ_PT_ON_TRANSMIT_SUCCESS,
    CMODULE_REQ_PT_GET_DEVSN,
    CMODULE_REQ_PT_GET_IP,
    CMODULE_REQ_PT_GET_PORT,
    CMODULE_REQ_PT_GET_ENCRYPT_KEY,
    CMODULE_REQ_PT_GET_HOST_TARGET,
    CMODULE_REQ_PT_GET_TIMING,
    CMODULE_REQ_PT_GET_ALARM_TH,
    CMODULE_REQ_PT_IS_ENCRYPT_KEY,
    CMODULE_REQ_PT_ON_UPDATE_TS,
    CMODULE_REQ_PT_ON_UPDATE_TIMING,
    CMODULE_REQ_PT_ON_UPDATE_ALARM_TH,
    CMODULE_REQ_PT_ON_UPDATE_HOST_TARGET,
    CMODULE_REQ_PT_ON_REGISTER_COMPLETED,
    CMODULE_REQ_PT_ON_KEY_RESET,
    CMODULE_REQ_PT_ON_KEY_SET,
} cModule_RequestType_Pt_t;

/*@}*/

/**
 * @addtogroup ProtocolDefine
 * @note none
 */
 
/*@{*/

typedef enum {
    PT_GSKL_STBM_ALARM_NORMAL = 0u,
    PT_GSKL_STBM_ALARM_PRE = 1u,
    PT_GSKL_STBM_ALARM_L = 2u,
    PT_GSKL_STBM_ALARM_H = 3u,
    
    PT_GSKL_STBM_CTL_NORMAL = 0u,
    PT_GSKL_STBM_CTL_COM_FAULT = 1u << 2u,
    PT_GSKL_STBM_CTL_POWER_DOWN = 2u << 2u,
    
    PT_GSKL_STBM_POLEST_NORMAL = 0u,
    PT_GSKL_STBM_POLEST_SHORT = 1u << 4u,
    PT_GSKL_STBM_POLEST_OPEN = 2u << 4u,
    PT_GSKL_STBM_POLEST_AGING = 3u << 4u,
    PT_GSKL_STBM_POLEST_UNCALI = 4u << 4u,
    PT_GSKL_STBM_POLEST_ZERO_OFFSET = 4u << 4u,
    PT_GSKL_STBM_POLEST_EXP = 5u << 4u,
    
    PT_GSKL_STBM_POLEMODE_NORMAL = 0u,
    PT_GSKL_STBM_POLEMODE_WARM = 1u << 8u,
    PT_GSKL_STBM_POLEMODE_SELFTESET = 2u << 8u,
    PT_GSKL_STBM_POLEMODE_EXP = 3u << 8u,
    PT_GSKL_STBM_POLEMODE_PWR_FAULT = 4u << 8u,
    PT_GSKL_STBM_POLEMODE_OVP = 5u << 8u,
    
    PT_GSKL_STBM_WATERDET_NORMAL = 0u,
    PT_GSKL_STBM_WATERDET_LV1 = 1u << 11u,
    PT_GSKL_STBM_WATERDET_LV2 = 2u << 11u,
    
    PT_GSKL_STBM_COVERDET_NORMAL = 0u,
    PT_GSKL_STBM_COVERDET_LV1 = 1u << 13u,
} Protocol_Gskl_StateBitMap_t;

typedef enum {
    GSKL_OBJID_REGISTER = 0x9001,
    GSKL_OBJID_SET_ADDR = 0x9011,
    GSKL_OBJID_GET_ADDR = 0x9012,
    GSKL_OBJID_SET_OPTS = 0x9013,
    GSKL_OBJID_GET_OPTS = 0x9014,
    GSKL_OBJID_REPORT_SELF = 0x9031,
    GSKL_OBJID_REPORT_ALARM = 0x9032,
    
    GSKL_OBJID_SET_KEY = 0xA014,
    GSKL_OBJID_RESET_KEY = 0xA015,
} Protocol_Gskl_ObjId_t;

typedef struct {
    uint8_t channel;
    uint16_t status;    // Protocol_Gskl_StateBitMap_t
    uint16_t lelAvg_x100;
    uint16_t lelMax_x100;
    uint16_t lelMin_x100;
    uint16_t bvatVolt_x100;
    uint32_t ts;
} Protocol_Gskl_9031_t;

typedef struct {
    uint8_t uploadTime[2];
    uint8_t discreteTime;
    uint16_t heartBeat;
    uint16_t idleMeasureInterval;
    uint16_t idleReportInterval;
    uint16_t preAlarmMeasureInterval;
    uint16_t preAlarmReportInterval;
    uint16_t alarmMeasureInterval;
    uint16_t alarmReportInterval;
} Protocol_Gskl_Timing_t;

typedef struct {
    float ll;
    float l;
    float h;
} Protocol_Gskl_Threshold_t;

typedef struct {
    uint8_t target1;
    uint8_t host1[31];
    uint16_t port1;
    
    uint8_t target2;
    uint8_t host2[31];
    uint16_t port2;
} Protocol_Gskl_NetTarget_t;

/*@}*/

/**
 * @addtogroup ProtocolRequest
 * @note none
 */
 
/*@{*/

extern void cModule_protocol_init(void);
extern bool cModule_protocol_isCaching(void);

extern const char *cModule_protocol_getDefaultKey(void);

extern int32_t cModule_protocolTransmit_gskl(cModule_Instance_t *ins, Protocol_Gskl_ObjId_t objId, void *obj, uint32_t objCount);

/*@}*/

#endif
