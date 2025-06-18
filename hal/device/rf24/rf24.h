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
 
#ifndef _RF24_H_
#define _RF24_H_

#include <stdint.h>

/**
 * @addtogroup Typedef
 * @note none
 */
 
/*@{*/

typedef enum { 
    RF24_CHIP_XN297,
    RF24_CHIP_SI24R1,
} RF24_Chip_t;

typedef enum { 
    RF24_ST_RXDR = 0x40,
    RF24_ST_TXDS = 0x20,
    RF24_ST_MAXRT = 0x10,
    RF24_ST_RXPNO = (0x07 << 1),
    RF24_ST_TXFULL = 0x01,
} RF24_Status_t;

typedef enum { 
    RF24_DR_2M = 0,
    RF24_DR_1M = 1,
    RF24_DR_250K = 2,
} RF24_DateRate_t;

typedef enum { 
    RF24_MODE_TX = 0,
    RF24_MODE_RX = 1,
} RF24_Mode_t;

typedef enum { 
    RF24_CRC_OFF = 0,
    RF24_CRC_1BYTE = 1,
    RF24_CRC_2BYTE = 2,
} RF24_CrcSize_t;

typedef enum { 
    RF24_FEATURE_DATA_LEN_MODE_B32 = 0,
    RF24_FEATURE_DATA_LEN_MODE_B64 = 1,
} RF24_FeatureDataLengthMode_t;

typedef enum { 
    RF24_RF_XN297_POWER_P_11 = 0x27,  // XN297_11dbm
    RF24_RF_XN297_POWER_P_10 = 0x26,  // XN297_10dbm
    RF24_RF_XN297_POWER_P_9 = 0x15,   // XN297_9dbm
    RF24_RF_XN297_POWER_P_5 = 0x2c,   // XN297_5dbm
    RF24_RF_XN297_POWER_P_4 = 0x14,   // XN297_4dbm
    RF24_RF_XN297_POWER_N_1 = 0x2A,   // XN297_-1dbm
    RF24_RF_XN297_POWER_N_9 = 0x29,   // XN297_-9dbm
    RF24_RF_XN297_POWER_N_10 = 0x19,  // XN297_-10dbm
    RF24_RF_XN297_POWER_N_23 = 0x30,  // XN297_-23dbm
    
    RF24_RF_SI24R1_POWER_P_7 = 7,  // SI24R1_7dbm
    RF24_RF_SI24R1_POWER_P_6 = 6,  // SI24R1_4
    RF24_RF_SI24R1_POWER_P_3 = 5,   // SI24R1_3	
    RF24_RF_SI24R1_POWER_P_1 = 4,   // SI24R1_1	
    RF24_RF_SI24R1_POWER_P_0 = 3,   // SI24R1_0	
    RF24_RF_SI24R1_POWER_N_4 = 2,   // SI24R1_-4
    RF24_RF_SI24R1_POWER_N_6 = 1,   // SI24R1_-6
    RF24_RF_SI24R1_POWER_N_12 = 0,  // SI24R1_-12
} RF24_RfPower_t;

typedef enum { 
    RF24_TRANSMIT_ACK_MODE_ACK = 0,
    RF24_TRANSMIT_ACK_MODE_NOACK = 1,
    RF24_TRANSMIT_ACK_MODE_ACK_PAYLOAD = 10,
} RF24_TransmitAckMode_t;

typedef enum { 
    RF24_TRANSMIT_END_CE_MODE_LOW = 0,
    RF24_TRANSMIT_END_CE_MODE_HIGH = 1,
} RF24_TransmitEndCeMode_t;

typedef void (*RF24_OnRecviced_t)(uint8_t pipe);

/*@}*/

/**
 * @addtogroup baseFunc
 * @note none
 */
 
/*@{*/

extern void rf24_initBus(const char *ifPath, const char *csPath, const char *cePath);
extern int32_t rf24_initSoft(RF24_Chip_t chip);

extern void rf24_finalizeSoft(void);
extern void rf24_finalizeBus(void);

extern void rf24_setOnReceived(RF24_OnRecviced_t on);
extern void rf24_setOnBlock(void (*onBlock)(void));

extern int32_t rf24_loop(void);

/*@}*/

/**
 * @addtogroup RfFunc
 * @note none
 */
 
/*@{*/

extern void rf24_rf_setCe(uint8_t hl);
extern void rf24_rf_setDataRate(RF24_DateRate_t rate);
extern void rf24_rf_setPowerLevel(RF24_RfPower_t power);
extern void rf24_rf_setFreqOffset(uint8_t freq);
extern uint8_t rf24_rf_getFreqOffset(void);
extern void rf24_rf_setCrcSize(RF24_CrcSize_t crcSize);
extern void rf24_rf_powerControl(uint8_t up);
extern void rf24_rf_setMode(uint8_t isTxMode);

/*@}*/

/**
 * @addtogroup StatusFunc
 * @note none
 */
 
/*@{*/

extern void rf24_status_clearAll(void);
extern void rf24_status_clearTxFlag(void);
extern void rf24_status_clearRxFlag(void);
extern RF24_Status_t rf24_status_get(void);

/*@}*/

/**
 * @addtogroup AddressFunc
 * @note none
 */
 
/*@{*/

extern void rf24_addr_setWidth(uint8_t aw);

extern void rf24_addr_setTxAddr(uint64_t addr);
extern void rf24_addr_setTxAddrU8(uint8_t *addr);

/*@}*/

/**
 * @addtogroup PipeFunc
 * @note none
 */
 
/*@{*/

extern void rf24_pipe_enable(uint8_t pipeNumber, uint8_t en);
extern void rf24_pipe_setAddr(uint8_t pipeNumber, uint64_t addr);
extern void rf24_pipe_setAddrU8(uint8_t pipeNumber, uint8_t *addr);
extern void rf24_pipe_setPayloadLength(uint8_t pipeNumber, uint8_t payloadW);
extern void rf24_pipe_enableDynPayloadLength(uint8_t pipeNumber, uint8_t en);
extern void rf24_pipe_enableAutoAck(uint8_t pipeNumber, uint8_t en);

/*@}*/

/**
 * @addtogroup FeatureFunc
 * @note none
 */
 
/*@{*/

extern void rf24_feature_setDataLengthMode(RF24_FeatureDataLengthMode_t mode);
extern void rf24_feature_enableDynPayloadLength(uint8_t en);
extern void rf24_feature_enableAckWithPayload(uint8_t en);
extern void rf24_feature_enableTxPayloadNoAck(uint8_t en);

extern void rf24_feature_setAutoRetxDelay(uint16_t us);
extern void rf24_feature_setAutoRetxCount(uint8_t count);

/*@}*/

/**
 * @addtogroup FlowFunc
 * @note none
 */
 
/*@{*/

extern int32_t rf24_flow_send(uint8_t *data, uint8_t length, 
        uint32_t timeout, 
        RF24_TransmitAckMode_t ack,
        RF24_TransmitEndCeMode_t endCeMode);
extern int32_t rf24_flow_hasRecvData(void);
extern void rf24_flow_read(uint8_t *data, uint8_t length);
extern uint8_t rf24_flow_getRxFifoLength(void);
extern void rf24_flow_flushTx(void);
extern void rf24_flow_flushRx(void);

/*@}*/

#endif
