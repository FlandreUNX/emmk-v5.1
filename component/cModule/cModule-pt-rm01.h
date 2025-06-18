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
 * @addtogroup Define & Typedef
 * @note none
 */

/*@{*/

typedef enum {
    CMODULE_REQ_PT_START = CMODULE_REQ_PT_ID_START,

    CMODULE_REQ_PT_GET_SN,
    CMODULE_REQ_PT_GET_CONNECT_SSID,
    CMODULE_REQ_PT_GET_MESH_ID,
    CMODULE_REQ_PT_GET_MESH_LEVEL,
    CMODULE_REQ_PT_GET_MESH_STA_COUNT,
    CMODULE_REQ_PT_GET_MESH_RSSI,
    CMODULE_REQ_PT_GET_MESH_PARENT_MAC,
    CMODULE_REQ_PT_GET_MESH_MAC,

    CMODULE_REQ_PT_OTA_ON_REQUEST,

    CMODULE_REQ_PT_RQ1_AC2_SWITCH_CONTROL,
    CMODULE_REQ_PT_RQ4_DEVICE_RESET,
    CMODULE_REQ_PT_RQ5_ERROR_RESET,
    CMODULE_REQ_PT_RQ6_SET_PAIR,
    CMODULE_REQ_PT_RQ7_AC2_SET_SWITCH_HOLD,
    CMODULE_REQ_PT_RQ8_OTA_ROLLBACK,
    CMODULE_REQ_PT_RQ13_MC1_CONTROL,
    CMODULE_REQ_PT_RQ14_MC1_SET_SPEED,
    CMODULE_REQ_PT_RQ10_PIO1_CALI_DO,
    CMODULE_REQ_PT_RQ11_PIO1_CALI_PH,
    CMODULE_REQ_PT_RQ12_PIO1_CALI_ORP,
    CMODULE_REQ_PT_RQ15_PIO1_STILL_MEAUSRE,
    CMODULE_REQ_PT_RQ16_PIO1_SALT_OFFSET,
} cModule_RequestType_Pt_t;

typedef struct {
    const char *ssid;
    const char *pwd;
} cModule_Rq_SetPair_t;

typedef enum {
    CMODULE_RQ_MC1_CTLMODE_REMOTE_ON = 2,
    CMODULE_RQ_MC1_CTLMODE_REMOTE_OFF = 3,
    CMODULE_RQ_MC1_CTLMODE_TIMER_ON = 4,
} cModule_Rq_Mc1CtlMode_t;

typedef struct {
    cModule_Rq_Mc1CtlMode_t mode;
    uint32_t sec;
    uint8_t speed;
} cModule_Rq_Mc1Ctl_t;

typedef struct {
    uint8_t mode;
    cModule_RequestVar_t var1;
} cModule_Rq_Pio1Cali_t;

#define CMODULE_PT_RM01_AC2_SID       8604
#define CMODULE_PT_RM01_MC1_SID       8605

#define CMODULE_PT_RM01_PIO1_XXX_SID      8606
#define CMODULE_PT_RM01_PIO1_DO_ADDR      3
#define CMODULE_PT_RM01_PIO1_PH_ADDR      4
#define CMODULE_PT_RM01_PIO1_ORP_ADDR     5
#define CMODULE_PT_RM01_PIO1_ZD_ADDR      33

typedef struct {
    uint16_t sid;
    union {
        struct {
            int16_t temp_x10;
            int16_t vp_x10;
            int16_t amp_x10;
            int32_t watt_x10;
            uint8_t control;
            uint8_t status;
        } ac2;
        struct {
            int8_t runStatus;
            int16_t temp_x10;
            uint8_t chgStatus;
            uint32_t runSec;
            uint8_t speed;
        } mc1;
        struct {
            uint8_t addr;
            struct {
                uint8_t soc;
                uint16_t volt;
            } battety;
            union {
                struct {
                    uint16_t do_x10;
                    uint16_t po_x10;
                    uint16_t fdo_x10;
                    int16_t temp_x10;
                    int16_t ad;
                    int16_t fcAd;
                    uint8_t fcDet;
                } dox;
                struct {
                    int16_t ph_x100;
                    int16_t ad_x10;
                } phx;
                struct {
                    int16_t mv_x10;
                    int16_t ad_x10;
                } orpx;
                struct {
                    uint16_t ntu_x10;
                    int16_t temp_x10;
                } zdx;
            } value;
        } pio1;
    };
} cModule_RM01_Data01_V1_t;

/*@}*/

/**
 * @addtogroup ProtocolRequest
 * @note none
 */

/*@{*/

extern void cModule_protocol_init(void);

extern int32_t cModule_response_RM01_v241022(cModule_Instance_t *ins, int8_t rq, int32_t result);

extern int32_t cModule_reportData_RM01_online_v241022(cModule_Instance_t *ins);
extern int32_t cModule_reportData_RM01_data01_v241022(cModule_Instance_t *ins, cModule_RM01_Data01_V1_t *d);

/*@}*/

#endif
