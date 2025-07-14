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
 * @addtogroup Version
 * @note none
 */

/*@{*/

#define PT_RDAX_VERISON_STR      "R241115"

/*@}*/

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
    
    CMODULE_REQ_PT_ON_TRANSMIT_SUCCESS,

    CMODULE_REQ_PT_ON_SELF_TEST,
    CMODULE_REQ_PT_ON_ALARM_SILENCE,
    CMODULE_REQ_PT_ON_RESET,
    CMODULE_REQ_PT_ON_OPERATE_VALVE,
    CMODULE_REQ_PT_ON_OPERATE_VALVE_NUM,
    CMODULE_REQ_PT_SET_ALARM_HI,
    CMODULE_REQ_PT_SET_ALARM_LO,
    CMODULE_REQ_PT_SET_SAMPLING_PERIOPD,
    CMODULE_REQ_PT_SET_UPDATIME_MIN,
    
    CMODULE_REQ_PT_EXTMOD81_PAIRC,
    CMODULE_REQ_PT_EXTMOD81_PAIRSS,
} cModule_RequestType_Pt_t;

typedef enum {
    CMODULE_TPACK_RID_PT = TRANSMIT_PACK_REQ_ID_START + 0,
    
    CMODULE_TPACK_RID_PT_AT_RDA1_REPORT,
} cModule_TransmitPackageRequestId_Pt_t;

/*@}*/

/**
 * @addtogroup ProtocolDefine
 * @note none
 */
 
/*@{*/

typedef struct {
    uint8_t comStatus: 1;
    uint8_t isMoveAlarm: 1;
    uint8_t isLiquidAlarm: 1;
    uint8_t isGasAlarm: 1;
    uint8_t isBatteryLow: 1;
    float batLvl;
    int8_t temp;
    int16_t gasValue;
    uint8_t alarmL;
    uint8_t alarmH;
    uint16_t updateTimeMin;
    uint16_t measureTimeMin;
    float latitude;
    float longitude;
    float height;
} ProtocolRdaUnderWell_t;

typedef enum {
    PROTOCOL_RDA_STATE_NORMAL = 0,
    PROTOCOL_RDA_STATE_ALARM_L = 1,
    PROTOCOL_RDA_STATE_FAULT = 2,
    PROTOCOL_RDA_STATE_ISO = 3,
    PROTOCOL_RDA_STATE_EXP = 4,
    PROTOCOL_RDA_STATE_ALARM_H = 5,
    PROTOCOL_RDA_STATE_ALARM_STEL = 6,
    PROTOCOL_RDA_STATE_ALARM_TWA = 7,
    PROTOCOL_RDA_STATE_OVL = 8,
    PROTOCOL_RDA_STATE_SENSOR_FAULT = 9,
    PROTOCOL_RDA_STATE_COM_FAULT = 10,
} ProtocolRda1_State_t;

typedef enum {
    PROTOCOL_RDA_UNIT_PPM = 0,
    PROTOCOL_RDA_UNIT_PERCENT = 1,
    PROTOCOL_RDA_UNIT_VOL = 2,
    PROTOCOL_RDA_UNIT_LEL = 3,
    PROTOCOL_RDA_UNIT_C = 4,
    PROTOCOL_RDA_UNIT_NONE = 5,
    PROTOCOL_RDA_UNIT_U_MOL = 6,
    PROTOCOL_RDA_UNIT_MG_M3 = 7,
} ProtocolRda1_Unit_t;

typedef enum {
    PROTOCOL_RDA_DOX_STATE_NORMAL = 0,
    PROTOCOL_RDA_DOX_STATE_LP = 1,
    PROTOCOL_RDA_DOX_STATE_OFFLINE = 2,
} ProtocolRda1_DoxState_t;

typedef enum {
    PROTOCOL_RDA_TYPE_CH4 = 0,
    PROTOCOL_RDA_TYPE_CO = 1,
} ProtocolRda1_Type_t;

typedef struct {
    int16_t value;
    int16_t valueCo;
    int16_t temp;
    ProtocolRda1_State_t state;
    ProtocolRda1_Unit_t unit;
    ProtocolRda1_Type_t type;
    uint16_t alarmL;
    uint16_t alarmH;
    uint16_t updateTimeMin;
    uint8_t isPowerDown: 1;
    uint8_t isComFault: 1;
    
    uint16_t auxCoLtm;

    uint8_t doxEnableCount;
    struct {
        char *macStr;
        uint8_t sw: 1;
        ProtocolRda1_DoxState_t st: 2;
    } dox[9];
} ProtocolRda1_t;

typedef enum {
    PROTOCOL_RDA_X_STATE_WARMING = 0,
    PROTOCOL_RDA_X_STATE_NORMAL = 1,
    PROTOCOL_RDA_X_STATE_ALARM_L = 2,
    PROTOCOL_RDA_X_STATE_ALARM_H = 3,
    PROTOCOL_RDA_X_STATE_SENSOR_FAULT = 4,
    PROTOCOL_RDA_X_STATE_POLE_OFFLINE = 5,
    PROTOCOL_RDA_X_STATE_UNUSED = 6,
} ProtocolRdaX_State_t;

typedef enum {
    PROTOCOL_RDA_X_UNIT_LEL = 1,
    PROTOCOL_RDA_X_UNIT_VOL = 2,
    PROTOCOL_RDA_X_UNIT_PPM = 3,
    PROTOCOL_RDA_X_UNIT_U_MOL = 4,
    PROTOCOL_RDA_X_UNIT_MG_M = 5,
    PROTOCOL_RDA_X_UNIT_PPM_M = 6,
    PROTOCOL_RDA_X_UNIT_LEL_M = 7,
    PROTOCOL_RDA_X_UNIT_TEMP_C = 8,
    PROTOCOL_RDA_X_UNIT_PERCENT = 9,
    PROTOCOL_RDA_X_UNIT_RH = 10,
    PROTOCOL_RDA_X_UNIT_PPB = 11,
} ProtocolRdaX_Unit_t;

typedef struct {
    uint16_t index;
    int16_t value;
    ProtocolRdaX_State_t state;
    uint16_t unit;
} ProtocolRdaX_t;

typedef struct {
    uint8_t isAlarm: 1;
    uint8_t isBatteryLow: 1;
    float batLvl;
    uint16_t updateTimeMin;
} ProtocolRdaTamper_t;

/*@}*/

/**
 * @addtogroup ProtocolRequest
 * @note none
 */
 
/*@{*/

extern int32_t cModule_protocolTransmit_UndergroundWell(cModule_Instance_t *ins, ProtocolRdaUnderWell_t *protocol, bool forceWrite);
extern int32_t cModule_protocolTransmit_control(cModule_Instance_t *ins, ProtocolRda1_t *protocol, bool forceWrite);
extern int32_t cModule_protocolTransmit_controlX(cModule_Instance_t *ins, ProtocolRdaX_t *protocol, uint32_t count);
extern int32_t cModule_protocolTransmit_tamper(cModule_Instance_t *ins, ProtocolRdaTamper_t *protocol, bool forceWrite);

extern void cModule_protocol_init(void);
extern bool cModule_protocol_isCaching(void);

/*@}*/

#endif
