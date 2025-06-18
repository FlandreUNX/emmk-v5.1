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
 
#include "./irmp.h"

#include <stdlib.h>
#include <string.h>
 
/**
 * @addtogroup Debug support
 * @note none
 */
 
/*@{*/

#undef DBG_SECTION_NAME
#define DBG_SECTION_NAME  "irmp"

/*@}*/

/**
 * @addtogroup Define & Typedef
 * @note none
 */
 
/*@{*/

//// NEC protocol bit inerval
#define NEC_ADDRESS_BITS             16 // 16 bit address or 8 bit address and 8 bit inverted address
#define NEC_COMMAND_BITS             16 // Command and inverted command
                                     
#define NEC_BITS                     (NEC_ADDRESS_BITS + NEC_COMMAND_BITS)
#define NEC_UNIT                     (560 / 10)
                                     
#define NEC_HEADER_MARK              (16 * NEC_UNIT) // 9000
#define NEC_HEADER_SPACE             (8 * NEC_UNIT)  // 4500
                                     
#define NEC_BIT_MARK                 NEC_UNIT
#define NEC_ONE_SPACE                (3 * NEC_UNIT)  // 1690
#define NEC_ZERO_SPACE               NEC_UNIT
                                     
#define NEC_REPEAT_HEADER_SPACE      (4 * NEC_UNIT)  // 2250
#define NEC_REPEAT_PERIOD            110000

//// The states for the state machine
#define IR_RECEIVER_STATE_WAITING_FOR_START_MARK        0
#define IR_RECEIVER_STATE_WAITING_FOR_START_SPACE       1
#define IR_RECEIVER_STATE_WAITING_FOR_FIRST_DATA_MARK   2
#define IR_RECEIVER_STATE_WAITING_FOR_DATA_SPACE        3
#define IR_RECEIVER_STATE_WAITING_FOR_DATA_MARK         4
#define IR_RECEIVER_STATE_WAITING_FOR_STOP_MARK         5

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

void irmp_init(irmp_t *instance, irmp_OnDecode_t onDecode) {
    memset(instance, 0x00, sizeof(irmp_t));
    instance->onDecode = onDecode;
}


void irmp_call(register uint32_t newPinValue, register uint32_t new10UsPerCount, irmp_t *irmp) {
    uint16_t tMicrosOfMarkOrSpace = new10UsPerCount - irmp->aux.lastChangeMicros;
    uint8_t tState = irmp->aux.irReceiverState;
    
    irmp->aux.lastChangeMicros = new10UsPerCount;
    
    if (!newPinValue) {
        // We have a mark here
        if (tMicrosOfMarkOrSpace > 2 * NEC_HEADER_MARK) {
            // timeout -> must reset state machine
            tState = IR_RECEIVER_STATE_WAITING_FOR_START_MARK;
        }
        
        if (tState == IR_RECEIVER_STATE_WAITING_FOR_START_MARK) {
            // We are at the beginning of the header mark, check timing at the next transition
            tState = IR_RECEIVER_STATE_WAITING_FOR_START_SPACE;
        } else if (tState == IR_RECEIVER_STATE_WAITING_FOR_FIRST_DATA_MARK) {
            if (tMicrosOfMarkOrSpace >= lowerValue25Percent(NEC_HEADER_SPACE)
                    && tMicrosOfMarkOrSpace <= upperValue25Percent(NEC_HEADER_SPACE)) {
                // We have a valid data header space here -> initialize data
                irmp->aux.irRawDataBitCounter = 0;
                        
                irmp->data.rawData.uLong = 0;
                irmp->data.rawDataMask = 1;
                irmp->data.repeatDetected = 0;
                tState = IR_RECEIVER_STATE_WAITING_FOR_DATA_SPACE;
            } else if (tMicrosOfMarkOrSpace >= lowerValue25Percent(NEC_REPEAT_HEADER_SPACE)
                    && tMicrosOfMarkOrSpace <= upperValue25Percent(NEC_REPEAT_HEADER_SPACE)
                    && irmp->aux.irRawDataBitCounter >= NEC_BITS) {
                // We have a repeat header here and no broken receive before -> set repeat flag
                irmp->data.repeatDetected = 1;
                tState = IR_RECEIVER_STATE_WAITING_FOR_DATA_SPACE;
            } else {
                // This parts are optimized by the compiler into jumps to one code :-)
                // Wrong length -> reset state
                tState = IR_RECEIVER_STATE_WAITING_FOR_START_MARK;
            }
        } else if (tState == IR_RECEIVER_STATE_WAITING_FOR_DATA_MARK) {
            // Check data space length
            if (tMicrosOfMarkOrSpace >= lowerValue(NEC_ZERO_SPACE) 
                    && tMicrosOfMarkOrSpace <= upperValue(NEC_ONE_SPACE)) {
                // We have a valid bit here
                tState = IR_RECEIVER_STATE_WAITING_FOR_DATA_SPACE;
                if (tMicrosOfMarkOrSpace >= 2 * NEC_UNIT) {
                    // we received a 1
                    irmp->data.rawData.uLong |= irmp->data.rawDataMask;
                } else {
                    // we received a 0 - empty code for documentation
                }
                // prepare for next bit
                irmp->data.rawDataMask = irmp->data.rawDataMask << 1;
                irmp->aux.irRawDataBitCounter++;
            } else {
                // Wrong length -> reset state
                tState = IR_RECEIVER_STATE_WAITING_FOR_START_MARK;
            }
        } else {
            // error wrong state for the received level, e.g. if we missed one change interrupt -> reset state
            tState = IR_RECEIVER_STATE_WAITING_FOR_START_MARK;
        }
    } else {
        // We have a space here
        if (tState == IR_RECEIVER_STATE_WAITING_FOR_START_SPACE) {
            // Check length of header mark here
            if (tMicrosOfMarkOrSpace >= lowerValue25Percent(NEC_HEADER_MARK)
                    && tMicrosOfMarkOrSpace <= upperValue25Percent(NEC_HEADER_MARK)) {
                tState = IR_RECEIVER_STATE_WAITING_FOR_FIRST_DATA_MARK;
            } else {
                // Wrong length of header mark -> reset state
                tState = IR_RECEIVER_STATE_WAITING_FOR_START_MARK;
            }
        } else if (tState == IR_RECEIVER_STATE_WAITING_FOR_DATA_SPACE) {
            // Check data mark length
            if (tMicrosOfMarkOrSpace >= lowerValue(NEC_BIT_MARK) 
                    && tMicrosOfMarkOrSpace <= upperValue(NEC_BIT_MARK)) {
                // We have a valid mark here, check for transmission complete
                if (irmp->aux.irRawDataBitCounter >= NEC_BITS || irmp->data.repeatDetected) {
                    // Code complete -> call callback, no parity check!
                    // Reset state for new start
                    tState = IR_RECEIVER_STATE_WAITING_FOR_START_MARK;
                    // Address reduction to 8 bit
                    if (irmp->data.rawData.uByte.lowByte == (uint8_t) (~irmp->data.rawData.uByte.midLowByte)) {
                        // standard 8 bit address NEC protocol
                        // Address is the first 8 bit
                        irmp->data.rawData.uByte.midLowByte = 0; 
                    }

                    // Call user provided callback here
                    if (irmp->onDecode != NULL) {
                        irmp->onDecode(irmp->data.rawData.uWord.lowWord, // address
                            irmp->data.rawData.uByte.midHighByte, // command
                            irmp->data.repeatDetected); // isRepeat
                    }
                } else {
                    // not finished yet
                    tState = IR_RECEIVER_STATE_WAITING_FOR_DATA_MARK;
                }
            } else {
                // Wrong length -> reset state
                tState = IR_RECEIVER_STATE_WAITING_FOR_START_MARK;
            }
        } else {
            // error wrong state for the received level, e.g. if we missed one change interrupt -> reset state
            tState = IR_RECEIVER_STATE_WAITING_FOR_START_MARK;
        }
    }

    irmp->aux.irReceiverState = tState;
}


uint8_t imrp_isReceiverIdle(irmp_t *irmp) {
    return (irmp->aux.irReceiverState == IR_RECEIVER_STATE_WAITING_FOR_START_MARK);
}

/*@}*/

/**
 * @addtogroup Debug support
 * @note none
 */
 
/*@{*/

#undef DBG_SECTION_NAME

/*@}*/
