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
 
#ifndef _BL0910_H_
#define _BL0910_H_

#include <stdint.h>

/**
 * @addtogroup Typedef
 * @note none
 */
 
/*@{*/

typedef enum {
    BL0910_CHANNEL_GAIN1_OFFSET_V = 0,
    BL0910_CHANNEL_GAIN1_OFFSET_1 = 4,
    BL0910_CHANNEL_GAIN1_OFFSET_2 = 8,
    BL0910_CHANNEL_GAIN1_OFFSET_3 = 12,
    BL0910_CHANNEL_GAIN1_OFFSET_4 = 16,
    BL0910_CHANNEL_GAIN1_OFFSET_5 = 20,
    
    BL0910_CHANNEL_GAIN2_OFFSET_6 = 100 + 0,
    BL0910_CHANNEL_GAIN2_OFFSET_7 = 100 + 4,
    BL0910_CHANNEL_GAIN2_OFFSET_8 = 100 + 8,
    BL0910_CHANNEL_GAIN2_OFFSET_9 = 100 + 12,
    BL0910_CHANNEL_GAIN2_OFFSET_10 = 100 + 16,
} BL0910_ChannelGainOffset_t;

typedef enum {
    BL0910_CHANNEL_GAIN_1 = 0,
    BL0910_CHANNEL_GAIN_2 = 1,
    BL0910_CHANNEL_GAIN_8 = 2,
    BL0910_CHANNEL_GAIN_16 = 3,
    BL0910_CHANNEL_GAIN_MARK = 3,
} BL0910_ChannelGain_t;

static const char BL0910_CHANNEL_GAIN_X[] = {1, 2, 8, 16};

typedef enum {
    BL0910_RMS_CHANNEL_1,
    BL0910_RMS_CHANNEL_2,
    BL0910_RMS_CHANNEL_3,
    BL0910_RMS_CHANNEL_4,
    BL0910_RMS_CHANNEL_5,
    BL0910_RMS_CHANNEL_6,
    BL0910_RMS_CHANNEL_7,
    BL0910_RMS_CHANNEL_8,
    BL0910_RMS_CHANNEL_9,
    BL0910_RMS_CHANNEL_10,
    BL0910_RMS_CHANNEL_11,
} BL0910_RMSChannel_t;

typedef enum {
    BL0910_WATT_CHANNEL_1,
    BL0910_WATT_CHANNEL_2,
    BL0910_WATT_CHANNEL_3,
    BL0910_WATT_CHANNEL_4,
    BL0910_WATT_CHANNEL_5,
    BL0910_WATT_CHANNEL_6,
    BL0910_WATT_CHANNEL_7,
    BL0910_WATT_CHANNEL_8,
    BL0910_WATT_CHANNEL_9,
    BL0910_WATT_CHANNEL_10,
    BL0910_WATT_CHANNEL_TOTAL,
} BL0910_WattChannel_t;

typedef enum {
    BL0910_CFCNT_CHANNEL_1,
    BL0910_CFCNT_CHANNEL_2,
    BL0910_CFCNT_CHANNEL_3,
    BL0910_CFCNT_CHANNEL_4,
    BL0910_CFCNT_CHANNEL_5,
    BL0910_CFCNT_CHANNEL_6,
    BL0910_CFCNT_CHANNEL_7,
    BL0910_CFCNT_CHANNEL_8,
    BL0910_CFCNT_CHANNEL_9,
    BL0910_CFCNT_CHANNEL_10,
} BL0910_CfCntChannel_t;

typedef enum {
    BL0910_ST1_OFFSET_SAG,
    BL0910_ST1_OFFSET_ZXTO,
    BL0910_ST1_OFFSET_ZX1,
    BL0910_ST1_OFFSET_ZX2,
    BL0910_ST1_OFFSET_ZX3,
    BL0910_ST1_OFFSET_ZX4,
    BL0910_ST1_OFFSET_ZX5,
    BL0910_ST1_OFFSET_ZX6,
    BL0910_ST1_OFFSET_ZX7,
    BL0910_ST1_OFFSET_ZX8,
    BL0910_ST1_OFFSET_ZX9,
    BL0910_ST1_OFFSET_ZX10,
    BL0910_ST1_OFFSET_ZX11,
    BL0910_ST1_OFFSET_PK1,
    BL0910_ST1_OFFSET_PK2,
    BL0910_ST1_OFFSET_PK3,
    BL0910_ST1_OFFSET_PK4,
    BL0910_ST1_OFFSET_PK5,
    BL0910_ST1_OFFSET_PK6,
    BL0910_ST1_OFFSET_PK7,
    BL0910_ST1_OFFSET_PK8,
    BL0910_ST1_OFFSET_PK9,
    BL0910_ST1_OFFSET_PK10,
    BL0910_ST1_OFFSET_PK11,
    BL0910_ST1_OFFSET_PKV,
} BL0910_Status1Offset_t;

typedef enum {
    BL0910_ST3_OFFSET_1 = 1u << 0u,
    BL0910_ST3_OFFSET_2 = 1u << 1u,
    BL0910_ST3_OFFSET_3 = 1u << 2u,
    BL0910_ST3_OFFSET_4 = 1u << 3u,
    BL0910_ST3_OFFSET_5 = 1u << 4u,
    BL0910_ST3_OFFSET_6 = 1u << 5u,
    BL0910_ST3_OFFSET_7 = 1u << 6u,
    BL0910_ST3_OFFSET_8 = 1u << 7u,
    BL0910_ST3_OFFSET_9 = 1u << 8u,
    BL0910_ST3_OFFSET_10 = 1u << 9u,
} BL0910_Status3Offset_t;

typedef enum {
    BL0910_CONTROL_PRIORITY_1 = 1u << 12u,
    BL0910_CONTROL_PRIORITY_2 = 1u << 13u,
    BL0910_CONTROL_PRIORITY_3 = 1u << 14u,
    BL0910_CONTROL_PRIORITY_4 = 1u << 15u,
    BL0910_CONTROL_PRIORITY_5 = 1u << 16u,
    BL0910_CONTROL_PRIORITY_6 = 1u << 17u,
    BL0910_CONTROL_PRIORITY_7 = 1u << 18u,
    BL0910_CONTROL_PRIORITY_8 = 1u << 19u,
    BL0910_CONTROL_PRIORITY_9 = 1u << 20u,
    BL0910_CONTROL_PRIORITY_10 = 1u << 21u,
    BL0910_CONTROL_PRIORITY_ALL = 0x7FFu << 12u,
} BL0910_ControlPriority_t;


typedef enum {
    BL0910_CONTROL_1 = 1u << 0u,
    
    BL0910_CONTROL_2 = 1u << 1u, // 1u10i-1
    BL0910_CONTROL_3 = 1u << 2u, // 1u10i-2
    BL0910_CONTROL_4 = 1u << 3u, // 1u10i-3
    
    BL0910_CONTROL_5 = 1u << 4u,
    BL0910_CONTROL_6 = 1u << 5u,
    
    BL0910_CONTROL_7 = 1u << 6u, // 1u10i-4
    BL0910_CONTROL_8 = 1u << 7u, // 1u10i-5
    BL0910_CONTROL_9 = 1u << 8u, // 1u10i-6
    
    BL0910_CONTROL_10 = 1u << 9u,
    BL0910_CONTROL_ALL = 0x7FFu,
    BL0910_CONTROL_3U6I_I1 = BL0910_CONTROL_2 | BL0910_CONTROL_3 | BL0910_CONTROL_4,
    BL0910_CONTROL_3U6I_I2 = BL0910_CONTROL_7 | BL0910_CONTROL_8 | BL0910_CONTROL_9,
} BL0910_Control_t;

typedef enum {
    BL0910_IF_SPI,
    BL0910_IF_SERIAL,
} BL0910_InterfaceMode_t;

typedef struct {
    const BL0910_InterfaceMode_t ifMode;
    void (*onSpiSelect)(BL0910_InterfaceMode_t ifMode, bool isSelect);
    void (*onDataTx)(BL0910_InterfaceMode_t ifMode, void *data, uint32_t txSize);
    void (*onDataRx)(BL0910_InterfaceMode_t ifMode, void *data, uint32_t rxSize);
    void (*onIfReset)(void);

    struct {
        float vrl_r; // vrl_r 电压负载电阻(R)
        float vrv_k; // vrv_k 电压互感器分压电阻(R)
        
        float irl_r; // irl_r 电流互感器负载电阻(R)
        float ct_rt; // ct_rt 电流互感器变比
        
        BL0910_ChannelGain_t gain_i;
        BL0910_ChannelGain_t gain_v;
        
        BL0910_ChannelGainOffset_t *voltGain;
        uint8_t voltGainCount;
        BL0910_ChannelGainOffset_t *ampGain;
        uint8_t AmpGainCount;
    } channelArg;
} BL0910_Instance_t;

typedef struct {
    uint32_t raw_ua;
    uint32_t raw_ub;
    uint32_t raw_uc;
    uint32_t raw_ix[2][3];
    int32_t raw_watt[2][3];
    uint32_t raw_cf[2][3];
    
    float vl_ua;
    float vl_ub;
    float vl_uc;
    float vl_ix[2][3];
    float vl_watt[2][3];
    float vl_eng[2][3];
    
    union {
        uint8_t _;
        struct {
            uint8_t i1a: 1;
            uint8_t i1b: 1;
            uint8_t i1c: 1;
            uint8_t i1_all: 1;
            
            uint8_t i2a: 1;
            uint8_t i2b: 1;
            uint8_t i2c: 1;
            uint8_t i2_all: 1;
        };
    } control;
    
    union {
        uint8_t _;
        struct {
            uint8_t i1a: 1;
            uint8_t i1b: 1;
            uint8_t i1c: 1;
            uint8_t i1_any: 1;
            
            uint8_t i2a: 1;
            uint8_t i2b: 1;
            uint8_t i2c: 1;
            uint8_t i2_any: 1;
        };
    } leak;
    
    union {
        uint8_t _;
        struct {
            uint8_t i1a: 1;
            uint8_t i1b: 1;
            uint8_t i1c: 1;
            uint8_t i1_any: 1;
            
            uint8_t i2a: 1;
            uint8_t i2b: 1;
            uint8_t i2c: 1;
            uint8_t i2_any: 1;
        };
    } ipk;
    
    union {
        uint8_t _;
        struct {
            uint8_t a: 1;
            uint8_t b: 1;
            uint8_t c: 1;
            uint8_t any: 1;
        };
    } vpk;
} BL0910_3U6IData_t;


typedef struct {
    uint32_t raw_u;
    uint32_t raw_ix[6];
    int32_t raw_watt[6];
    uint32_t raw_cf[6];
    
    float vl_u;
    float vl_ix[6];
    float vl_watt[6];
    float vl_eng[6];
    
    union {
        uint8_t _;
        struct {
            uint8_t i1: 1;
            uint8_t i2: 1;
            uint8_t i3: 1;
            uint8_t i4: 1;
            uint8_t i5: 1;
            uint8_t i6: 1;
        };
    } control;
    
    union {
        uint8_t _;
        struct {
            uint8_t i1: 1;
            uint8_t i2: 1;
            uint8_t i3: 1;
            uint8_t i4: 1;
            uint8_t i5: 1;
            uint8_t i6: 1;
        };
    } leak;
    
    union {
        uint8_t _;
        struct {
            uint8_t i1: 1;
            uint8_t i2: 1;
            uint8_t i3: 1;
            uint8_t i4: 1;
            uint8_t i5: 1;
            uint8_t i6: 1;
        };
    } ipk;
    
    union {
        uint8_t _;
        struct {
            uint8_t any: 1;
        };
    } vpk;
} BL0910_1U10IData_t;

/*@}*/

/**
 * @addtogroup ExportFuncs
 * @note none
 */
 
/*@{*/

extern void bl0910_initBus(BL0910_Instance_t *instnace, 
    const char *kdSpiPath, const char *kdCsPinPath,
    const char *kdIrq1PinPath, const char *kdIrq2PinPath);
extern int32_t bl0910_initSoft(BL0910_Instance_t *instnace);
extern void bl0910_finalizeBus(BL0910_Instance_t *instnace);
extern void bl0910_finalizeSoft(BL0910_Instance_t *instnace);
    
/*@}*/

/**
 * @addtogroup BaseFunc
 * @note none
 */
 
/*@{*/
    
extern int32_t bl0910_getControlPriorty(BL0910_Instance_t *instance, BL0910_ControlPriority_t mark);
extern int32_t bl0910_softReset(BL0910_Instance_t *instance);  
extern int32_t bl0910_setModeAc3(BL0910_Instance_t *instance, uint8_t e);
extern int32_t bl0910_setChannelControl(BL0910_Instance_t *instance, uint32_t channel, uint8_t enable);
extern int32_t bl0910_readChannelControl(BL0910_Instance_t *instance, uint32_t channel);
extern float bl0910_measureTempInternal(BL0910_Instance_t *instance);
extern float bl0910_readChannelVoltCt(BL0910_Instance_t *instance, BL0910_RMSChannel_t channel);
extern float bl0910_readChannelAmpCt(BL0910_Instance_t *instance, BL0910_RMSChannel_t channel);
extern float bl0910_readChannelWattCt(BL0910_Instance_t *instance, BL0910_WattChannel_t channel);
extern float bl0910_readChannelEngCt(BL0910_Instance_t *instance, BL0910_CfCntChannel_t channel);
extern int32_t bl0910_cleanEng(BL0910_Instance_t *instance);

/*@}*/
    
#endif
