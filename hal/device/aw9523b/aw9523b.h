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
 
#ifndef _AW9523B_H_
#define _AW9523B_H_

#include <stdint.h>

/**
 * @addtogroup Typedef
 * @note none
 */
 
/*@{*/

typedef union {
    uint16_t u16;
    uint8_t u8[2];
    struct {
        uint16_t p00: 1;
        uint16_t p01: 1;
        uint16_t p02: 1;
        uint16_t p03: 1;
        uint16_t p04: 1;
        uint16_t p05: 1;
        uint16_t p06: 1;
        uint16_t p07: 1;
        
        uint16_t p10: 1;
        uint16_t p11: 1;
        uint16_t p12: 1;
        uint16_t p13: 1;
        uint16_t p14: 1;
        uint16_t p15: 1;
        uint16_t p16: 1;
        uint16_t p17: 1;
    };
} aw9523b_BitData_t;


typedef enum {
    AW9523B_PIN_00 = 1 << 0,
    AW9523B_PIN_01 = 1 << 1,
    AW9523B_PIN_02 = 1 << 2,
    AW9523B_PIN_03 = 1 << 3,
    AW9523B_PIN_04 = 1 << 4,
    AW9523B_PIN_05 = 1 << 5,
    AW9523B_PIN_06 = 1 << 6,
    AW9523B_PIN_07 = 1 << 7,
    AW9523B_PIN_10 = 1 << 8,
    AW9523B_PIN_11 = 1 << 9,
    AW9523B_PIN_12 = 1 << 10,
    AW9523B_PIN_13 = 1 << 11,
    AW9523B_PIN_14 = 1 << 12,
    AW9523B_PIN_15 = 1 << 13,
    AW9523B_PIN_16 = 1 << 14,
    AW9523B_PIN_17 = 1 << 15,
} aw9523b_Pin_t;


typedef enum {
    AW9523B_DIR_INPUT = 1,
    AW9523B_DIR_OUTPUT = 0,
    
    AW9523B_DIR_ALL_INPUT = 0xFFFF,
    AW9523B_DIR_ALL_OUTPUT = 0x0000,
} aw9523b_Dir_t;


typedef enum {
    AW9523B_B7_ADDRESS_PIN_A0 = 0x01,
    AW9523B_B7_ADDRESS_PIN_A1 = 0x02,
} aw9523b_B7AddressPin_t;

typedef struct {
    void *i2cIf;
    uint8_t address;
    aw9523b_BitData_t portDir;
    aw9523b_BitData_t inputData;
    aw9523b_BitData_t outputData;
} aw9523b_Instance_t;

/*@}*/

/**
 * @addtogroup Export funcs
 * @note none
 */
 
/*@{*/

extern void aw9523b_initBus(aw9523b_Instance_t *instnace, void *ifPath);
extern int32_t aw9523b_initSoft(aw9523b_Instance_t *instnace, aw9523b_B7AddressPin_t addressSet, uint16_t defDir, uint16_t defOut);
extern void aw9523b_finalizeBus(aw9523b_Instance_t *instnace);
extern void aw9523b_finalizeSoft(aw9523b_Instance_t *instnace);

extern int32_t aw9523b_softReset(aw9523b_Instance_t *instnace);

extern int32_t aw9523b_setPinDir(aw9523b_Instance_t *instnace, aw9523b_Pin_t pin, aw9523b_Dir_t dir);
extern int32_t aw9523b_setPinPortDir(aw9523b_Instance_t *instnace, uint16_t pinPort);

extern int32_t aw9523b_updateInput(aw9523b_Instance_t *instnace);
extern uint16_t aw9523b_readPin(aw9523b_Instance_t *instnace, aw9523b_Pin_t pin);
extern uint16_t aw9523b_readPinPort(aw9523b_Instance_t *instnace);

extern int32_t aw9523b_updateOutput(aw9523b_Instance_t *instnace);
extern int32_t aw9523b_setPin(aw9523b_Instance_t *instnace, aw9523b_Pin_t pin, uint8_t vl);
extern int32_t aw9523b_setPinPort(aw9523b_Instance_t *instnace, uint16_t pinPort);

/*@}*/

#endif
