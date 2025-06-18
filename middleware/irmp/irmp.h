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
 
#ifndef _IRMP_H_
#define _IRMP_H_

#include <stdint.h>

/**
 * @addtogroup Constant
 * @note none
 */
 
/*@{*/

///////////////////////////////////////////////////////
//// !!!! CALL "irmp_call" EACH PIN RISE/FALL !!!! ////
///////////////////////////////////////////////////////

/*@}*/


/**
 * @addtogroup Confiure
 * @note none
 */
 
/*@{*/

/*@}*/

/**
 * @addtogroup Define & Typedef
 * @note none
 */
 
/*@{*/

typedef void (*irmp_OnDecode_t)(uint16_t address, uint8_t command, uint8_t isRepetition);

typedef union {
    struct {
        uint8_t lowByte;
        uint8_t highByte;
    } uByte;
    struct {
        int8_t lowByte;
        int8_t highByte;
    } byte;
    uint8_t uBytes[2];
    int8_t bytes[2];
    uint16_t uWord;
    int16_t word;
    uint8_t *bytePointer;
} irmp_WordUnion_t;

typedef union {
    struct {
        uint8_t lowByte;
        uint8_t midLowByte;
        uint8_t midHighByte;
        uint8_t highByte;
    } uByte;
    struct {
        int8_t lowByte;
        int8_t midLowByte;
        int8_t midHighByte;
        int8_t highByte;
    } byte;
    struct {
        uint8_t lowByte;
        irmp_WordUnion_t midWord;
        uint8_t highByte;
    } byteWord;
    struct {
        int16_t lowWord;
        int16_t highWord;
    } word;
    struct {
        irmp_WordUnion_t lowWord;
        irmp_WordUnion_t highWord;
    } wordUnion;
    struct {
        uint16_t lowWord;
        uint16_t highWord;
    } uWord;
    
    uint8_t uBytes[4]; // seems to have the same code size as using struct UByte
    int8_t bytes[4];
    uint16_t uWords[2];
    int16_t words[2];
    uint32_t uLong;
    int32_t _long;
} irmp_LongUnion_t;

typedef struct {
    struct {
        uint32_t lastChangeMicros;     // microseconds of last Pin Change Interrupt.
        uint8_t irReceiverState;       // the state of the state machine.
        uint8_t irRawDataBitCounter;
    } aux;
    
    struct {
        uint32_t rawDataMask;
        irmp_LongUnion_t rawData;
        uint8_t repeatDetected;
    } data;
    
    irmp_OnDecode_t onDecode;
} irmp_t;

/*@}*/

/**
 * @addtogroup Export funcs
 * @note none
 */
 
/*@{*/

extern void irmp_init(irmp_t *instance, irmp_OnDecode_t onDecode);
extern void irmp_call(register uint32_t newPinValue, register uint32_t new10UsPerCount, irmp_t *irmp);
extern uint8_t imrp_isReceiverIdle(irmp_t *irmp);

/*@}*/

#endif

