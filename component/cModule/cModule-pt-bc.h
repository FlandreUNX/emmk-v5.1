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
    
    CMODULE_REQ_PT_ON_TRANSMIT_SUCCESS,
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
    PROTOCOL_BC_STATE_SELF_TEST = 0,
    PROTOCOL_BC_STATE_WARM,
    PROTOCOL_BC_STATE_NORMAL,
    PROTOCOL_BC_STATE_ALARM_L,
    PROTOCOL_BC_STATE_ALARM_H,
    PROTOCOL_BC_STATE_OVER_LOAD,
    PROTOCOL_BC_STATE_SENSOR_FAULT,
    PROTOCOL_BC_STATE_COMM_FAULT,
    PROTOCOL_BC_STATE_POWER_DOWN,
    PROTOCOL_BC_STATE_OVER_TIME,
    PROTOCOL_BC_STATE_INVALID,
    PROTOCOL_BC_STATE_OTHER,
} ProtocolBcState_t;


typedef enum {
    PROTOCOL_BC_DEVICE_STATE_WARM = 0,
    PROTOCOL_BC_DEVICE_STATE_NORMAL = 1,
    PROTOCOL_BC_DEVICE_STATE_POWER_DOWN = 2,
} ProtocolBc_DeviceStatus_t;

typedef struct {
    uint8_t addr;
    int16_t value;
    ProtocolBcState_t state;
} ProtocolBc_Sensor_t;

typedef struct {
    ProtocolBc_Sensor_t *sensor;
    uint8_t sensorCount;
    ProtocolBc_DeviceStatus_t deviceStatus;
} ProtocolBc_t;

/*@}*/

/**
 * @addtogroup ProtocolRequest
 * @note none
 */
 
/*@{*/

extern int32_t cModule_protocolTransmit_bc(cModule_Instance_t *ins, ProtocolBc_t *protocol);

extern void cModule_protocol_init(void);

/*@}*/

#endif
