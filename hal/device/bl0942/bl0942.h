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
 
#ifndef _BL0942_H_
#define _BL0942_H_

#include <stdint.h>

/**
 * @addtogroup Typedef
 * @note none
 */
 
/*@{*/

typedef struct {
    void (*onDataTx)(void *data, uint32_t txSize);
    void (*onDataRx)(void *data, uint32_t rxSize);
    void (*onReset)(void);

    struct {
        uint32_t r2;
        uint32_t r1;
        float rl;
    } channelArg;
} BL0942_Instance_t;

typedef enum {
    BL0942_ST_BIT_CF_REVP_F = 0x01,     // 有功脉冲 CF 能量反向指示，负能量时置 1
    BL0942_ST_BIT_CREEP_F = 0x02,       // 有功功率小于有功防潜动值时置 1
    BL0942_ST_BIT_I_ZX_LTH_F = 0x100,   // 电流过零输出状态指示
    BL0942_ST_BIT_V_ZX_LTH_F = 0x200,   // 电压过零输出状态指示
} BL0942_State_t;

typedef struct {
    float v;
    float a;
    float w;
    float e;
    BL0942_State_t st;
} BL0942_Data_t;

/*@}*/

/**
 * @addtogroup ExportFuncs
 * @note none
 */
 
/*@{*/

extern int32_t bl0942_initSoft(BL0942_Instance_t *instance);
extern void bl0942_finalizeSoft(BL0942_Instance_t *instance);

/*@}*/

/**
 * @addtogroup BaseFunc
 * @note none
 */
 
/*@{*/

extern int32_t bl0942_read(BL0942_Instance_t *instance, BL0942_Data_t *data);

/*@}*/
    
#endif
