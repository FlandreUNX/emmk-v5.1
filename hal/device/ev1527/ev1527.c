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
#define NEC_ADDRESS_BITS             20
#define NEC_COMMAND_BITS             4

#define NEC_BITS                     (NEC_ADDRESS_BITS + NEC_COMMAND_BITS)
#define NEC_UNIT                     (350 / 10)         // 1unit = 4clk

#define NEC_HEADER_MARK              (1 * NEC_UNIT)     // H=4CLK
#define NEC_HEADER_SPACE             (31 * NEC_UNIT)    // L=124CLK

#define NEC_ONE_SPACE                (1 * NEC_UNIT)     // H=12CLK, L=4CLK
#define NEC_ZERO_SPACE               (4 * NEC_UNIT)     // H=4CLK, L=12CLK


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

void ev1527_init(ev1527_Instance_t *ins, const void *pinData, const ev1527_Callback_t *callback) {
    ASSERT(ins != NULL);
    ASSERT(pinData != NULL);

    ins->pinData = (void *) pinData;
    ins->callback = callback;
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
                if (tMicrosOfMarkOrSpace >= 2 * NEC_UNIT) {
                    // BIT=1
                    ins->aux.pinMode.rawData |= 0x01 << ((24 - ins->aux.pinMode.rawDataBitCounter) - 1);
                } else {
                    // BIT=0
                }
                ins->aux.pinMode.rawDataBitCounter++;
                if (ins->aux.pinMode.rawDataBitCounter >= NEC_BITS) {
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

static inline void encode_delay(ev1527_Encoder_t *enc) {
    enc->callback.timerWait(enc->clkCount);
}


static void encode_sync(ev1527_Encoder_t *enc) {
    enc->callback.gpioSet(1);
    for (int i = 0; i < 4; i++) {
        encode_delay(enc);
    }

    enc->callback.gpioSet(0);
    for (int i = 0; i < 12; i++) {
        encode_delay(enc);
    }
}


static void encode_bit(ev1527_Encoder_t *enc, const uint8_t bit) {
    if (bit) {
        enc->callback.gpioSet(1);
        for (int i = 0; i < 12; i++) {
            encode_delay(enc);
        }

        enc->callback.gpioSet(0);
        for (int i = 0; i < 4; i++) {
            encode_delay(enc);
        }
    } else {
        enc->callback.gpioSet(0);
        for (int i = 0; i < 4; i++) {
            encode_delay(enc);
        }

        enc->callback.gpioSet(1);
        for (int i = 0; i < 12; i++) {
            encode_delay(enc);
        }
    }
}

/*@}*/

/**
 * @addtogroup DecodeFunc-Export
 * @note none
 */

/*@{*/

void ev1527_encode_init(ev1527_Encoder_t *enc) {
    ASSERT(enc != NULL);
    ASSERT(enc->callback.timerInit_1us != NULL);
    ASSERT(enc->callback.timerFinalize != NULL);
    ASSERT(enc->callback.timerWait != NULL);
    ASSERT(enc->callback.gpioSet != NULL);
}


void ev1527_encode_finalize(ev1527_Encoder_t *enc) {

}


void ev1527_encode_sendBlock(ev1527_Encoder_t *enc, uint32_t b20, uint32_t b4, uint32_t repeat) {
    ASSERT(enc != NULL);

    b20 = (b20 << 4) | (b4 & 0x0000000F);

    enc->callback.timerInit_1us();
    enc->callback.gpioInit();
    for (uint32_t i = 0; i < repeat; i++) {
        encode_sync(enc);
        for (int x = 0; x < 24; x++) {
            encode_bit(enc, (b20 >> (23 - x)) & 0x01);
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

