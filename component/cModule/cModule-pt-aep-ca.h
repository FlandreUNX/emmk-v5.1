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
 
#ifndef _CM_MODULE_PT_AEP_CA_H_
#define _CM_MODULE_PT_AEP_CA_H_

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
    
    CMODULE_REQ_PT_ON_TRANSMIT_SUCCESS,

    CMODULE_REQ_PT_ON_SELF_TEST,
    CMODULE_REQ_PT_ON_ALARM_SILENCE,
    CMODULE_REQ_PT_ON_RESET,
    CMODULE_REQ_PT_ON_OPERATE_VALVE,
    CMODULE_REQ_PT_SET_ALARM_HI,
    CMODULE_REQ_PT_SET_ALARM_LO,
    CMODULE_REQ_PT_SET_SAMPLING_PERIOPD,
    CMODULE_REQ_PT_SET_UPDATIME_MIN,
} cModule_RequestType_Pt_t;

typedef enum {
    CMODULE_TPACK_RID_PT = TRANSMIT_PACK_REQ_ID_START,
    
    CMODULE_TPACK_RID_PT_AT_RDA1_REPORT,
} cModule_TransmitPackageRequestId_Pt_t;

/*@}*/

/**
 * @addtogroup ProtocolDefine
 * @note none
 */
 
/*@{*/

#define CAAEP_PAYLOAD_LENGTH               43
#define CAAEP_PAYLOAD_PREFIX_POS           0
#define CAAEP_PAYLOAD_IMEI_POS             CAAEP_PAYLOAD_PREFIX_POS + 1 
#define CAAEP_PAYLOAD_IMSI_POS             CAAEP_PAYLOAD_IMEI_POS + 15
#define CAAEP_PAYLOAD_SEQ_POS              CAAEP_PAYLOAD_IMSI_POS + 15
#define CAAEP_PAYLOAD_RSSI_POS             CAAEP_PAYLOAD_SEQ_POS + 2  
#define CAAEP_PAYLOAD_BAT_POS              CAAEP_PAYLOAD_RSSI_POS + 2   
#define CAAEP_PAYLOAD_DEV_TYPE_POS         CAAEP_PAYLOAD_BAT_POS + 1   
#define CAAEP_PAYLOAD_CMD_POS              CAAEP_PAYLOAD_DEV_TYPE_POS + 2     
#define CAAEP_PAYLOAD_LEL_POS              CAAEP_PAYLOAD_CMD_POS + 1
#define CAAEP_PAYLOAD_MINDEX1_POS          CAAEP_PAYLOAD_LEL_POS + 2
#define CAAEP_PAYLOAD_MINDEX2_POS          CAAEP_PAYLOAD_MINDEX1_POS + 1

typedef enum {
    PROTOCOL_CAAEP_STATE_NORMAL = 0,
    PROTOCOL_CAAEP_STATE_ALARM = 1,
    PROTOCOL_CAAEP_STATE_FAULT = 2,
    PROTOCOL_CAAEP_STATE_CALI = 5,
    PROTOCOL_CAAEP_STATE_BAT_LOW = 6,
} ProtocolCAAEP_State_t;

typedef struct {
    struct {
        uint8_t battery;
    } system;

    struct {
        uint16_t value;
        ProtocolCAAEP_State_t state;
    } gas;

    struct {
        uint8_t r1;
        uint8_t r2;
    } reserve;
} ProtocolCAAEP_t;

/*@}*/

/**
 * @addtogroup ProtocolRequest
 * @note none
 */
 
/*@{*/

extern int32_t cModule_protocolTransmit(cModule_Instance_t *ins, ProtocolCAAEP_t *protocol);

#define cModule_protocol_init()

/*@}*/

#endif
