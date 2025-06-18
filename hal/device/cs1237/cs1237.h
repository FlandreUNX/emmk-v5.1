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
 
#ifndef _CS1237_H_
#define _CS1237_H_
 
#include <stdint.h>
 
/**
 * @addtogroup Define & Typedef
 * @note none
 */
 
/*@{*/

typedef enum {
    CS1237_PGA_1 = 0,
    CS1237_PGA_2 = 1,
    CS1237_PGA_64 = 2,
    CS1237_PGA_128 = 3,
} cs1237_Pga_t;

typedef enum {
    CS1237_SPEED_10HZ = 0,
    CS1237_SPEED_40HZ = 1,
    CS1237_SPEED_640HZ = 2,
    CS1237_SPEED_1280HZ = 3,
} cs1237_Speed_t;

typedef enum {
    CS1237_REF_OUT_OFF = 1,
    CS1237_REF_OUT_ON = 0,
} cs1237_RefOut_t;

typedef enum {
    CS1237_CHANNEL_A = 0,
    CS1237_CHANNEL_RESERVE = 1,
    CS1237_CHANNEL_TEMP = 2,
    CS1237_CHANNEL_SHORT = 3,
} cs1237_Channel_t;

typedef union {
    uint8_t _;
    struct {
        uint8_t RESERVER1: 1;
        cs1237_RefOut_t refOut: 1;
        cs1237_Speed_t speed: 2;
        cs1237_Pga_t pga: 2;
        cs1237_Channel_t channel: 2;
    };
} cs1237_RegValue_t;

typedef struct {
    void *pinData;
    void *pinClk;

    int32_t adcRaw;
    uint8_t regStatus;
    cs1237_RegValue_t regValue;
} cs1237_Instance_t;

/*@}*/

/**
 * @addtogroup Export funcs
 * @note none
 */
 
/*@{*/

extern void cs1237_initBus(cs1237_Instance_t *ins, const void *pinData, const void *pinClk);
extern void cs1237_finalizeBus(const cs1237_Instance_t *ins);

extern int32_t cs1237_initSoft(cs1237_Instance_t *ins);
extern void cs1237_finalizeSoft(cs1237_Instance_t *ins);

extern int32_t cs1237_read(cs1237_Instance_t *ins);

/*@}*/

/**
 * @addtogroup Get/Set
 * @note none
 */
 
/*@{*/

extern uint8_t cs1237_getPga(const cs1237_Instance_t *ins);
extern int32_t cs1237_setPga(const cs1237_Instance_t *ins, cs1237_Pga_t pga);

extern int32_t cs1237_setSpeed(const cs1237_Instance_t *ins, cs1237_Speed_t speed);

extern int32_t cs1237_getRaw(const cs1237_Instance_t *ins);

extern cs1237_RegValue_t cs1237_getRegValue(const cs1237_Instance_t *ins);

/*@}*/

#endif
