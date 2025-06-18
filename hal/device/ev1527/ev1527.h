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

#ifndef _EV1527_H_
#define _EV1527_H_

#include <stdint.h>
#include <stdbool.h>

/**
 * @addtogroup Define & Typedef
 * @note none
 */

/*@{*/

typedef struct {
    void (*init)(void);
    void (*finalize)(void);
    void (*onCounterClean)(void);
} ev1527_Callback_t;

typedef struct {
    void *pinData;

    bool isDataReady;
    uint32_t data;

    union {
        struct {
            uint8_t step1;
            uint8_t bitCount;
            uint32_t decodeData;
            uint8_t baseTiming_per200us;
        } timingMode;
        struct {
            uint32_t lastChangeMicros;
            uint8_t receiverState;
            uint8_t rawDataBitCounter;
            uint32_t rawData;
        } pinMode;
    } aux;

    const ev1527_Callback_t *callback;
} ev1527_Instance_t;

typedef struct {
    uint32_t clkCount;

    struct {
        void (*timerInit_1us)(void);
        void (*timerFinalize)(void);
        void (*timerWait)(uint32_t tick_per1us);
        void (*gpioInit)(void);
        void (*gpioFinalize)(void);
        void (*gpioSet)(uint32_t level);
    } callback;
} ev1527_Encoder_t;

/*@}*/

/**
 * @addtogroup Export funcs
 * @note none
 */

/*@{*/

extern void ev1527_init(ev1527_Instance_t *ins, const void *pinData, const ev1527_Callback_t *callback) ;
extern void ev1527_finalize(ev1527_Instance_t *ins);
extern void ev1527_counter(ev1527_Instance_t *ins);
extern void ev1527_decode(ev1527_Instance_t *ins);
extern void ev1527_startUp(ev1527_Instance_t *ins);
extern void ev1527_down(ev1527_Instance_t *ins);
extern uint32_t ev1527_read(ev1527_Instance_t *ins);
extern uint8_t ev1527_isDataReady(ev1527_Instance_t *ins);
extern void ev1527_clearDataReady(ev1527_Instance_t *ins);

extern void ev1527_pinDecode(ev1527_Instance_t *ins, register uint32_t newPinValue, register uint32_t new10UsPerCount);

/*@}*/

/**
 * @addtogroup EncodeFunc-Export
 * @note none
 */

/*@{*/

extern void ev1527_encode_init(ev1527_Encoder_t *enc);
extern void ev1527_encode_finalize(ev1527_Encoder_t *enc);

extern void ev1527_encode_sendBlock(ev1527_Encoder_t *enc, uint32_t b20, uint32_t b4, uint32_t repeat);

/*@}*/

#endif
