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

#include "emmk-config.h"
#include "emmk-driver.h"

#include "./ev1527.h"

/**
 * @addtogroup Debug support
 * @note none
 */

/*@{*/

#undef TAG
#undef DBG_SECTION_NAME
#define DBG_SECTION_NAME        "ev1527"
#define TAG DBG_SECTION_NAME

/*@}*/

/**
 * @addtogroup Define
 * @note none
 */

/*@{*/

// START, START_CODE -> LowLevel > 8000us
#define EV1527_BIT_IS_START  (ins->aux.timingMode.baseTiming_per200us >= 40)

// START, TIMEOUT1 -> LowLevel > 14000us
#define EV1527_BIT_IS_TIMEOUT1 (ins->aux.timingMode.baseTiming_per200us >= 70)

// DATA, HIGH_CODE -> LowLevel > 800us
#define EV1527_BIT_IS_HIGH (ins->aux.timingMode.baseTiming_per200us >= 4)

// DATA, TIMEOUT2 -> LowLevel > 1400us
#define EV1527_BIT_IS_TIMEOUT2 (ins->aux.timingMode.baseTiming_per200us >= 7)

//// NEC protocol bit inerval
#define NEC_UNIT                     (400 / 10)         // 1unit = 4clk

#define NEC_HEADER_MARK              (1 * NEC_UNIT)     // H=4CLK
#define NEC_HEADER_SPACE             (31 * NEC_UNIT)    // L=124CLK

#define NEC_ONE_SPACE                (1 * NEC_UNIT)     // H=12CLK, L=4CLK
#define NEC_ZERO_SPACE               (3 * NEC_UNIT)     // H=4CLK, L=12CLK


//// The states for the state machine
#define RECEIVER_STATE_WAITING_FOR_START_MARK        0
#define RECEIVER_STATE_WAITING_FOR_START_SPACE       1
#define RECEIVER_STATE_WAITING_FOR_FIRST_DATA_SPACE   2
#define RECEIVER_STATE_WAITING_FOR_DATA_SPACE         3

//// Macros for comparing timing values
#define lowerValue25Percent(aDuration)   (aDuration - (aDuration / 4))
#define upperValue25Percent(aDuration)   (aDuration + (aDuration / 4))
#define lowerValue(aDuration)   (aDuration - (aDuration / 2))
#define upperValue(aDuration)   (aDuration + (aDuration / 2))

/*@}*/

/**
 * @addtogroup Export funcs
 * @note none
 */

/*@{*/

void ev1527_init(ev1527_Instance_t *ins, const void *pinData, const ev1527_Callback_t *callback, uint8_t bitSize) {
    ASSERT(ins != NULL);
    ASSERT(pinData != NULL);

    ins->pinData = (void *) pinData;
    ins->callback = callback;
    ins->aux.pinMode.bitSize = bitSize;
}


void ev1527_finalize(ev1527_Instance_t *ins) {
}


void ev1527_counter(ev1527_Instance_t *ins) {
    ins->aux.timingMode.baseTiming_per200us++;
}

void ev1527_pinDecode(ev1527_Instance_t *ins, const register uint32_t newPinValue,
                      const register uint32_t new10UsPerCount) {
    const uint16_t tMicrosOfMarkOrSpace = new10UsPerCount - ins->aux.pinMode.lastChangeMicros;
    uint8_t tState = ins->aux.pinMode.receiverState;

    ins->aux.pinMode.lastChangeMicros = new10UsPerCount;

    if (!newPinValue) {
        if (tState == RECEIVER_STATE_WAITING_FOR_START_MARK) {
            if (tMicrosOfMarkOrSpace > NEC_HEADER_MARK) {
                tState = RECEIVER_STATE_WAITING_FOR_START_SPACE;
            } else {
                tState = RECEIVER_STATE_WAITING_FOR_START_MARK;
            }
        } else if (tState == RECEIVER_STATE_WAITING_FOR_FIRST_DATA_SPACE) {
            ins->aux.pinMode.rawDataBitCounter = 0;
            ins->aux.pinMode.rawData = 0;
            tState = RECEIVER_STATE_WAITING_FOR_DATA_SPACE;
        } else if (tState == RECEIVER_STATE_WAITING_FOR_DATA_SPACE) {
        } else {
            tState = RECEIVER_STATE_WAITING_FOR_START_MARK;
        }
    } else {
        if (tState == RECEIVER_STATE_WAITING_FOR_START_SPACE) {
            if (tMicrosOfMarkOrSpace >= lowerValue25Percent(NEC_HEADER_SPACE)
                && tMicrosOfMarkOrSpace <= upperValue25Percent(NEC_HEADER_SPACE)) {
                tState = RECEIVER_STATE_WAITING_FOR_FIRST_DATA_SPACE;
            } else {
                tState = RECEIVER_STATE_WAITING_FOR_START_MARK;
            }
        } else if (tState == RECEIVER_STATE_WAITING_FOR_DATA_SPACE) {
            if (tMicrosOfMarkOrSpace >= lowerValue(NEC_ONE_SPACE)
                && tMicrosOfMarkOrSpace <= upperValue(NEC_ZERO_SPACE)) {
                if (tMicrosOfMarkOrSpace < 2 * NEC_UNIT) {
                    // BIT=1
                    ins->aux.pinMode.rawData |= 0x01 << (ins->aux.pinMode.rawDataBitCounter);
                } else {
                    // BIT=0
                }
                ins->aux.pinMode.rawDataBitCounter++;
                if (ins->aux.pinMode.rawDataBitCounter >= ins->aux.pinMode.bitSize) {
                    ins->data = ins->aux.pinMode.rawData;
                    ins->isDataReady = true;
                    tState = RECEIVER_STATE_WAITING_FOR_START_MARK;
                    ins->callback->onCounterClean();
                }
            } else {
                tState = RECEIVER_STATE_WAITING_FOR_START_MARK;
            }
        } else {
            tState = RECEIVER_STATE_WAITING_FOR_START_MARK;
        }
    }

    ins->aux.pinMode.receiverState = tState;
}

void ev1527_decode(ev1527_Instance_t *ins) {
    const uint32_t pinLevel = kdgpio_input(ins->pinData);

    switch (ins->aux.timingMode.step1) {
        case 0: {
            if (pinLevel) {
                ins->aux.timingMode.step1++;
            }
            break;
        }
        case 1: {
            if (!pinLevel) {
                ins->aux.timingMode.baseTiming_per200us = 0;
                ins->aux.timingMode.step1++;
            }
            break;
        }
        case 2: {
            if (EV1527_BIT_IS_TIMEOUT1) {
                ins->aux.timingMode.step1 = 0;
            } else if (pinLevel) {
                if (EV1527_BIT_IS_START) {
                    ins->aux.timingMode.baseTiming_per200us = 0;
                    ins->aux.timingMode.step1++;

                    ins->aux.timingMode.bitCount = 0;
                    ins->aux.timingMode.decodeData = 0;
                } else {
                    ins->aux.timingMode.step1 = 0;
                }
            }
            break;
        }
        case 3: {
            if (EV1527_BIT_IS_TIMEOUT2) {
                ins->aux.timingMode.step1 = 0;
            } else if (!pinLevel) {
                if (EV1527_BIT_IS_HIGH) {
                    ins->aux.timingMode.decodeData |= 0x01 << ((24 - ins->aux.timingMode.bitCount) - 1);
                }
                ins->aux.timingMode.bitCount++;
                if (ins->aux.timingMode.bitCount != 24) {
                    ins->aux.timingMode.step1 = 4;
                    ins->aux.timingMode.baseTiming_per200us = 0;
                } else {
                    ins->aux.timingMode.step1 = 0;
                    ins->data = ins->aux.timingMode.decodeData;
                    ins->isDataReady = true;
                    break;
                }
            }
            break;
        }
        case 4: {
            if (EV1527_BIT_IS_TIMEOUT2) {
                ins->aux.timingMode.step1 = 0;
            } else if (pinLevel) {
                ins->aux.timingMode.baseTiming_per200us = 0;
                ins->aux.timingMode.step1 = 3;
            }
            break;
        }
        default: break;
    }
}


void ev1527_startUp(ev1527_Instance_t *ins) {
    ins->isDataReady = false;
    ins->data = 0;
    ins->aux.timingMode.decodeData = 0;
    ins->aux.timingMode.bitCount = 0;
    ins->aux.timingMode.baseTiming_per200us = 0;
    ins->aux.timingMode.step1 = 0;

    ins->callback->init();
}

void ev1527_down(ev1527_Instance_t *ins) {
    ins->callback->finalize();

    ins->isDataReady = false;
    ins->data = 0;
    ins->aux.timingMode.decodeData = 0;
    ins->aux.timingMode.bitCount = 0;
    ins->aux.timingMode.baseTiming_per200us = 0;
    ins->aux.timingMode.step1 = 0;
}


uint32_t ev1527_read(ev1527_Instance_t *ins) {
    return ins->data;
}


uint8_t ev1527_isDataReady(ev1527_Instance_t *ins) {
    return ins->isDataReady;
}


void ev1527_clearDataReady(ev1527_Instance_t *ins) {
    ins->isDataReady = 0;
    ins->data = 0;
}

/*@}*/

/**
 * @addtogroup DecodeFunc-Private
 * @note none
 */

/*@{*/

static void encode_sync(ev1527_Encoder_t *enc) {
    uint32_t c124 = enc->c4 * 31;
    enc->callback.gpioSet(1);
    enc->callback.timerWait(enc->c4);
    
    enc->callback.gpioSet(0);
    enc->callback.timerWait(c124);
}


static void encode_bit(ev1527_Encoder_t *enc, const uint8_t bit) {
    if (bit) {
        enc->callback.gpioSet(1);
        enc->callback.timerWait(enc->c12);

        enc->callback.gpioSet(0);
        enc->callback.timerWait(enc->c4);
    } else {
        enc->callback.gpioSet(1);
        enc->callback.timerWait(enc->c4);

        enc->callback.gpioSet(0);
        enc->callback.timerWait(enc->c12);
    }
}

/*@}*/

/**
 * @addtogroup DecodeFunc-Export
 * @note none
 */

/*@{*/

void ev1527_encode_init(ev1527_Encoder_t *enc, uint32_t tick) {
    ASSERT(enc != NULL);
    ASSERT(enc->callback.timerInit_1us != NULL);
    ASSERT(enc->callback.timerFinalize != NULL);
    ASSERT(enc->callback.timerWait != NULL);
    ASSERT(enc->callback.gpioSet != NULL);
    enc->clkCount = tick;
    enc->c12 = tick * 3;
    enc->c4 = tick * 1;
}


void ev1527_encode_finalize(ev1527_Encoder_t *enc) {

}


void ev1527_encode_sendBlock(ev1527_Encoder_t *enc, uint32_t data, uint8_t bitSize, uint32_t repeat) {
    ASSERT(enc != NULL);

    enc->callback.timerInit_1us();
    enc->callback.gpioInit();
    for (uint32_t i = 0; i < repeat; i++) {
        encode_sync(enc);
        for (int x = 0; x < bitSize; x++) {
            encode_bit(enc, (data >> x) & 0x01);
        }
    }
    enc->callback.timerFinalize();
    enc->callback.gpioSet(0);
    enc->callback.gpioFinalize();
}

/*@}*/

/**
 * @addtogroup Debug support
 * @note none
 */

/*@{*/

#undef TAG
#undef DBG_SECTION_NAME

/*@}*/

