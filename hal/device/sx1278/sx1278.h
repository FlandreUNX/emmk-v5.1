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
 
#ifndef _SX1278_H_
#define _SX1278_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "QuarkTS.h"

/**
 * @addtogroup Define & Typedef
 * @note none
 */
 
/*@{*/

#define SX1278_DEFAULT_FREQ       432000000
#define SX1278_DEFAULT_FREQ_2     434000000

#define SX1278_DEFAULT_SETTING { \
    SX1278_DEFAULT_FREQ,            /** RFFrequency                                                                               **/ \
    20,                   /** Power                                                                                     **/ \
    7,                    /** SignalBw [0: 7.8kHz, 1: 10.4 kHz, 2: 15.6 kHz, 3: 20.8 kHz, 4: 31.2 kHz,                  **/ \
                          /** 5: 41.6 kHz, 6: 62.5 kHz, 7: 125 kHz, 8: 250 kHz, 9: 500 kHz, other: Reserved]            **/ \
    12,                   /** SpreadingFactor [6: 64, 7: 128, 8: 256, 9: 512, 10: 1024, 11: 2048, 12: 4096  chips]      **/ \
    4,                    /** ErrorCoding [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]                                              **/ \
    1,                    /** CrcOn [0: OFF, 1: ON]                                                                     **/ \
    0,                    /** ImplicitHeaderOn [0: OFF, 1: ON]                                                          **/ \
    1,                    /** RxSingleOn [0: Continuous, 1 Single]                                                      **/ \
    0,                    /** FreqHopOn [0: OFF, 1: ON]                                                                 **/ \
    4,                    /** HopPeriod Hops every frequency hopping period symbols                                     **/ \
    3000,                 /** TxPacketTimeout                                                                           **/ \
    3000,                 /** RxPacketTimeout                                                                           **/ \
    128,                  /** PayloadLength (used for implicit header mode)                                             **/ \
}
#define SX1278_DEFAULT_PREAMBLE_LEN     16
#define SX1278_DEFAULT_TSM              32.768f  // (1 / (BW / 2^SF))
#define SX1278_DEFAULT_CAD_TIME         33.024f   // (2^SF + 32) / BW

#define SX1278_FLAG_RECV_COMPLETED      0x01u
#define SX1278_FLAG_CAD_COMPLETED       0x02u
#define SX1278_FLAG_CAD_DETECTED        0x04u
#define SX1278_FLAG_IS_IDLE             0x08u
#define SX1278_FLAG_RECV_FAILED         0x10u
#define SX1278_FLAG_HAL_FAILED          0x20u

#define SX1278_REGS_SIZE     0x70
#define RF_BUFFER_SIZE       32

#define SX1278_IMPL_RSSI    1

typedef struct SX1278 SX1278_t;

typedef enum {
    SX1278_DIO_0,
    SX1278_DIO_1,
    SX1278_DIO_2,
    SX1278_DIO_3,
    SX1278_DIO_4,
    SX1278_DIO_5,
} SX1278_DIOIndex_t;

typedef enum {
    SX1278_TRANSMIT_OPERATE_START,
    SX1278_TRANSMIT_OPERATE_GET_RESULT,
    
    SX1278_TRANSMIT_OPERATE_GET_RESULT_COUNT,
} SX1278_TransmitOperate_t;

typedef enum {
    SX1278_STATE_STANDBY = 0,
    
    SX1278_STATE_TX_INIT = 1,
    SX1278_STATE_TX_RUNNING = 2,
    SX1278_STATE_TX_DONE = 3,  
    SX1278_STATE_TX_TIMEOUT = 4,
    
    SX1278_STATE_RX_INIT = 5,
    SX1278_STATE_RX_RUNNING = 6,
    SX1278_STATE_RX_DONE = 7,
    SX1278_STATE_RX_TIMEOUT = 8,
    
    SX1278_STATE_CAD_INIT = 9,
    SX1278_STATE_CAD_RUNNING = 10,
    
    SX1278_STATE_TO_STANDBY = 11,
    SX1278_STATE_TO_SLEEP = 12,
    SX1278_STATE_SLEEP = 13,
} SX1278_State_t;

typedef enum {
    RF_IDLE,
    RF_BUSY,
    RF_RX_DONE,
    RF_RX_TIMEOUT,
    RF_TX_DONE,
    RF_TX_TIMEOUT,
    RF_LEN_ERROR,
    RF_CHANNEL_EMPTY,
    RF_CHANNEL_ACTIVITY_DETECTED,
    RF_HAL_FAILED,
} SX1278_ProcessReturnCodes_t;

typedef struct {
    uint32_t RFFrequency;
    int8_t Power;
    uint8_t SignalBw;                   // LORA [0: 7.8 kHz, 1: 10.4 kHz, 2: 15.6 kHz, 3: 20.8 kHz, 4: 31.2 kHz,
                                        // 5: 41.6 kHz, 6: 62.5 kHz, 7: 125 kHz, 8: 250 kHz, 9: 500 kHz, other: Reserved]  
    uint8_t SpreadingFactor;            // LORA [6: 64, 7: 128, 8: 256, 9: 512, 10: 1024, 11: 2048, 12: 4096  chips]
    uint8_t ErrorCoding;                // LORA [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
    uint8_t CrcOn;                      // [0: OFF, 1: ON]
    uint8_t ImplicitHeaderOn;           // [0: OFF, 1: ON]
    uint8_t RxSingleOn;                 // [0: Continuous, 1 Single]
    uint8_t FreqHopOn;                  // [0: OFF, 1: ON]
    uint8_t HopPeriod;                  // Hops every frequency hopping period symbols
    uint32_t TxPacketTimeout;
    uint32_t RxPacketTimeout;
    uint8_t PayloadLength;
} SX1278_Setting_t;

typedef struct {
    void (*onInit)(SX1278_t *handler);
    void (*onReset)(SX1278_t *handler);
    void (*onSends)(SX1278_t *handler, uint8_t *data, uint16_t dataLength);
    void (*onRecvs)(SX1278_t *handler, uint8_t *data, uint16_t dataLength);
    void (*onTransmit)(SX1278_t *handler, uint8_t *dataSend, uint8_t *dataRecv, uint16_t dataLength);
    void (*onSelect)(SX1278_t *handler);
    void (*onUnselect)(SX1278_t *handler);
    uint8_t (*onReadDIO)(SX1278_t *handler, SX1278_DIOIndex_t dio);
} SX1278_Interface_t;

struct SX1278 {
    const SX1278_Setting_t *setting;
    uint16_t rxPacketTimeout;

    struct {
        float tsmValue;
        float cadTime;
        uint8_t preambleLen;
    } calcConfig;

#if SX1278_IMPL_RSSI == 1
    uint8_t rxGain;
    int8_t rxPacketSnrEstimate;
    int16_t rxPacketRssiValue;
#endif
    
    SX1278_State_t currentState;
    
    uint8_t regBuffer[4];

    uint8_t transmitPacket[RF_BUFFER_SIZE];
    uint16_t transmitPacketSize;
    
    qSTimer_t timeout;
    
    uint8_t flag;
    const SX1278_Interface_t *interface;
};

/*@}*/

/**
 * @addtogroup Private constants
 * @note none
 */
 
/*@{*/



/*@}*/

/**
 * @addtogroup Private vars
 * @note none
 */
 
/*@{*/



/*@}*/

/**
 * @addtogroup Private funcs
 * @note none
 */
 
/*@{*/



/*@}*/

/**
 * @addtogroup Export funcs
 * @note none
 */
 
/*@{*/

extern void sx1278_init(SX1278_t *handler, 
        const SX1278_Setting_t *defaultSetting, 
        uint8_t preambleLen,
        const SX1278_Interface_t *interface);
extern SX1278_ProcessReturnCodes_t sx1278_poll(SX1278_t *handler);

extern int32_t sx1278_sends(SX1278_t *handler,
        uint8_t *data, uint16_t length, 
        uint32_t timeout);
extern int32_t sx1278_recvs(SX1278_t *handler, 
        SX1278_TransmitOperate_t op,
        uint8_t *data, uint16_t expectedRecvLength, uint16_t *actualRecvLength, 
        uint32_t timeout);
extern int32_t sx1278_cadDetect(SX1278_t *handler, SX1278_TransmitOperate_t op);
extern int32_t sx1278_standby(SX1278_t *handler);
extern int32_t sx1278_sleep(SX1278_t *handler);
extern uint16_t sx1278_getTpayload(SX1278_t *handler, uint8_t payloadLength);
extern uint16_t sx1278_getTpreamble(SX1278_t *handler);
extern uint16_t sx1278_getCadTime(SX1278_t *handler);

extern void sx1278_setFreq(SX1278_t *handler, uint32_t freq);

extern uint8_t sx1278_isReady(SX1278_t *handler);

/*@}*/

#endif
