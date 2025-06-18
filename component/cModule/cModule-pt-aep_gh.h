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
 
#ifndef _CM_MODULE_PT_AEP_GH_H_
#define _CM_MODULE_PT_AEP_GH_H_

#include <stdint.h>

#include "./cModule.h"

/**
 * @addtogroup Define
 * @note none
 */
 
/*@{*/

/*@}*/

/**
 * @addtogroup Define & Typedef-
 * @note none
 */
 
/*@{*/

typedef enum {
    CMODULE_REQ_PT_START = CMODULE_REQ_PT_ID_START + 0,
    
    CMODULE_REQ_PT_ON_REG_FAILED,
    CMODULE_REQ_PT_ON_TRANSMIT_SUCCESS,
    
    CMODULE_REQ_PT_GET_REGISTERED_STATE,
    CMODULE_REQ_PT_GET_DEVICE_ID8,
    CMODULE_REQ_PT_SET_DEVICE_ID8,
    CMODULE_REQ_PT_SET_REPORT_OPTIONS,
    CMODULE_REQ_PT_GET_SENSOR_TOTAL_COUNT,
    CMODULE_REQ_PT_SET_DIFF_OPTION,
    CMODULE_REQ_PT_SET_ALARM_THRESHOLD_OPTION,
    CMODULE_REQ_PT_ON_SAMPLE_ACTION,
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

typedef enum {
    CMODULE_PTAEPGH_REG_ST_NONE = 0,
    CMODULE_PTAEPGH_REG_ST_RELEASE = 1,
    CMODULE_PTAEPGH_REG_ST_DEBUG = 2,
    CMODULE_PTAEPGH_REG_ST_UNKNOW = 0xFF
} cModule_PtAepGh_RegState_t;

typedef enum {
    CMODULE_PTAEPGH_ST_OKAY = 0,
    CMODULE_PTAEPGH_ST_MAIN_PWR_FAULT = 0x11,
    CMODULE_PTAEPGH_ST_BAKUP_PWR_FAULT = 0x12,
    CMODULE_PTAEPGH_ST_CTLER_FAULT = 0x13,
    CMODULE_PTAEPGH_ST_EXP = 0x20,
    CMODULE_PTAEPGH_ST_POLE_FAULT = 0x21,
    CMODULE_PTAEPGH_ST_SENSOR_FAULT = 0x22,
    CMODULE_PTAEPGH_ST_EMC = 0x23,
} cModule_PtAepGh_State_t;

typedef enum {
    CMODULE_PTAEPGH_GASTYPE_CH4 = 'T',
    CMODULE_PTAEPGH_GASTYPE_CH3H8 = 'Y',
    CMODULE_PTAEPGH_GASTYPE_CO = 'M',
    CMODULE_PTAEPGH_GASTYPE_OTHER = 'Q',
} cModule_PtAepGh_GasType_t;

typedef enum {
    CMODULE_PTAEPGH_POWER_ST_NORMAL = 0,
    CMODULE_PTAEPGH_POWER_ST_UP = 1,
    CMODULE_PTAEPGH_POWER_ST_DOWN = 2,
    CMODULE_PTAEPGH_POWER_ST_BAKUP = 3,
} cModule_PtAepGh_PowerStatus_t;

typedef enum {
    CMODULE_PTAEPGH_ALARM_TYPE_NORMAL = 0,
    CMODULE_PTAEPGH_ALARM_TYPE_LOW = 1,
    CMODULE_PTAEPGH_ALARM_TYPE_HIGH = 2,
    CMODULE_PTAEPGH_ALARM_TYPE_CHANGE = 3,
} cModule_PtAepGh_AlarmType_t;

typedef struct {
    uint8_t mode;               // 0=Interval, 1=Timing
    uint8_t intervalMode;       // 0=Interval, 1=Daily
    uint8_t intervalTime;       // intervalMode=0, Mins; intervalMode=1, unvaild
    uint8_t timingStart;        // =FF, unvaild
    uint8_t timingEnd;          // =FF, unvaild
} cModule_PtAepGh_ReportOption_t;

typedef struct {
    uint8_t enable;             
    cModule_PtAepGh_GasType_t gasType;
    uint8_t sec;
    uint16_t diff;
} cModule_PtAepGh_DiffOption_t;

typedef struct {           
    cModule_PtAepGh_GasType_t gasType;
    uint16_t low;
    uint16_t high;
} cModule_PtAepGh_AlarmThresholdOption_t;

typedef enum {           
    CMODULE_PTAEPGH_DEV_TYPE_ALONE = 0,
    CMODULE_PTAEPGH_DEV_TYPE_CONTROL = 1,
    CMODULE_PTAEPGH_DEV_TYPE_CONTROL_SLAVE = 2,
} cModule_PtAepGh_DevType_t;

typedef enum {
    CMODULE_PTAEPGH_SAMPLE_ACTION_SLINCE,
    CMODULE_PTAEPGH_SAMPLE_ACTION_RESET,
} cModule_PtAepGh_SampleAction_t;

#define PTAEPGH_DATA_REPORT_CMD     0xA2
#define PTAEPGH_RUN_REPORT_CMD      0xA3
#define PTAEPGH_MONITE_REPORT_CMD   0xA4
typedef struct {
    uint16_t index;
    
    cModule_PtAepGh_State_t controlStatus;// A2, A3

    union {
        struct {
            int16_t value;
            cModule_PtAepGh_GasType_t gas;
            cModule_PtAepGh_State_t status;
        } a2;
        struct {
            // A3
            cModule_PtAepGh_PowerStatus_t power;
            cModule_PtAepGh_State_t status;
            
            // A4
            int16_t value;
            cModule_PtAepGh_GasType_t gas;
            cModule_PtAepGh_AlarmType_t alarm;
            uint16_t temp_x100;
            uint8_t humi;
        } a3_a4;
    };
} cModule_PtAepGh_DataReport_t;

/*@}*/

/**
 * @addtogroup ProtocolRequest
 * @note none
 */
 
/*@{*/

extern void cModule_ptAepGh_ntpUpdate(cModule_Instance_t *ins);
extern int32_t cModule_ptAepGh_regsiter(cModule_Instance_t *ins, cModule_PtAepGh_DevType_t devType, const char *devCode);
extern int32_t cModule_ptAepGh_report(cModule_Instance_t *ins, void *p, uint8_t cmd, uint32_t count, bool isContinue);

extern void cModule_protocol_init(void);

/*@}*/

#endif
