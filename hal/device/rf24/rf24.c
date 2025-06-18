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
#include "./rf24.h"

/**
 * @addtogroup Debug support
 * @note none
 */

/*@{*/

#undef DBG_SECTION_NAME
#define DBG_SECTION_NAME  "RF24"

/*@}*/

/**
 * @addtogroup ProgramProfile
 * @note none
 */

/*@{*/

#define CONFIG_DRIVER_USING_CONST        (1)

/*@}*/

/**
 * @addtogroup Define & Typedef
 * @note none
 */

/*@{*/

#define REG_READ_CMD       0x00
#define REG_WRITE_CMD      0x20

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

#if CONFIG_DRIVER_USING_CONST == 0
static kdSPI_t *gDevRf24Interface = NULL;
static kdGPIO_t *gDevRf24CsPin = NULL;
static kdGPIO_t *gDevRf24CePin = NULL;
#else
extern const kdspi_t *const gDevRf24Interface;
extern const kdgpio_t *const gDevRf24CsPin;
extern const kdgpio_t *const gDevRf24CePin;
#endif

struct {
    RF24_Chip_t chip;

    uint8_t addressWidth;
    RF24_OnRecviced_t onReceived;
    void (*onBlock)(void);
} mInstance;

/*@}*/

/**
 * @addtogroup PrivatHalFunc
 * @note none
 */

/*@{*/

__STATIC_FORCEINLINE void setCeLow(void) {
    kdgpio_output((void *) gDevRf24CePin, 0);
}


__STATIC_FORCEINLINE void setCeHigh(void) {
    kdgpio_output((void *) gDevRf24CePin, 1);
}


static inline void regReads(uint8_t reg, uint8_t *data, uint8_t len) {
    kdgpio_output((void *) gDevRf24CsPin, 0);
    kdspi_sendData((void *) gDevRf24Interface, &reg, 1, -1);
    kdspi_recvData((void *) gDevRf24Interface, data, len, -1);
    kdgpio_output((void *) gDevRf24CsPin, 1);
}


static inline uint8_t regRead(uint8_t reg) {
    uint8_t data;
    regReads(reg, &data, 1);
    return data;
}


static inline void regDataWrites(uint8_t reg, uint8_t *data, uint8_t len) {
    kdgpio_output((void *) gDevRf24CsPin, 0);
    kdspi_sendData((void *) gDevRf24Interface, &reg, 1, -1);
    kdspi_sendData((void *) gDevRf24Interface, data, len, -1);
    kdgpio_output((void *) gDevRf24CsPin, 1);
}


static inline void dataWrites(uint8_t *data, uint8_t len) {
    kdgpio_output((void *) gDevRf24CsPin, 0);
    kdspi_sendData((void *) gDevRf24Interface, data, len, -1);
    kdgpio_output((void *) gDevRf24CsPin, 1);
}


static inline void regWrite(uint8_t reg, uint8_t data) {
    regDataWrites(REG_WRITE_CMD + reg, &data, 1);
}


static inline void regWrites(uint8_t reg, uint8_t *data, uint8_t len) {
    regDataWrites(REG_WRITE_CMD + reg, data, len);
}

/*@}*/

/**
 * @addtogroup PrivateFunc
 * @note none
 */

/*@{*/

static void setAddress(uint8_t reg, uint64_t address, uint8_t addrW) {
    // must write the number of bytes specified by address width
    // LSB is written first

    uint8_t addr[5];

    for (uint8_t i = 0; i < addrW; i++) {
        addr[i] = address & 0xFF;
        address = address >> 8;
    }

    regWrites(reg, addr, addrW);
}


static void setAddressU8(uint8_t reg, uint8_t *address, uint8_t addrW) {
    regWrites(reg, address, addrW);
}


static void softReset(void) {
    static const uint8_t RSTQ1[] = {0x53, 0x5A};
    static const uint8_t RSTQ2[] = {0x53, 0xA5};

    dataWrites((uint8_t *) RSTQ1, 2);
    dataWrites((uint8_t *) RSTQ2, 2);
}

/*@}*/

/**
 * @addtogroup Special_XN297L
 * @note none
 */

/*@{*/

static void special_xn297_caliInit(void) {
    static const uint8_t BB_cal_data[] = {REG_WRITE_CMD + 0x1F, 0x12, 0xED, 0x67, 0x9C, 0x46};
    static const uint8_t RF_cal_data[] = {REG_WRITE_CMD + 0x1E, 0xF6, 0x3F, 0x5D};
    static const uint8_t RF_cal2_data[] = {REG_WRITE_CMD + 0x1A, 0x45, 0x21, 0xEF, 0x2C, 0x5A, 0x40};
    static const uint8_t Dem_cal_data[] = {REG_WRITE_CMD + 0x19, 0x01};
    static const uint8_t Dem_cal2_data[] = {REG_WRITE_CMD + 0x1B, 0x0B, 0xDF, 0x02};

    dataWrites((uint8_t *) BB_cal_data, sizeof(BB_cal_data));
    dataWrites((uint8_t *) RF_cal_data, sizeof(RF_cal_data));
    dataWrites((uint8_t *) RF_cal2_data, sizeof(RF_cal2_data));
    dataWrites((uint8_t *) Dem_cal_data, sizeof(Dem_cal_data));
    dataWrites((uint8_t *) Dem_cal2_data, sizeof(Dem_cal2_data));
}


static void special_xn297_magicInit(void) {
    regWrite(0x1C, 0x00);
    regWrite(0x1D, 0x00);
    regWrite(0x04, 0x00);
    regWrite(0x01, 0x00);
}

/*@}*/

/**
 * @addtogroup BaseFunc
 * @note none
 */

/*@{*/

void rf24_initBus(const char *ifPath, const char *csPath, const char *cePath) {
#if CONFIG_DRIVER_USING_CONST == 0
    if (gDevRf24Interface != NULL
            || gDevRf24CsPin != NULL
            || gDevRf24CePin != NULL) {
        return -1;
    }
#endif

#if CONFIG_DRIVER_USING_CONST == 0
    gDevRf24Interface = KDINSTANCE(SPI, ifPath);
    ASSERT(gDevRf24Interface != NULL);

    gDevRf24CsPin = KDINSTANCE(GPIO, csPath);
    ASSERT(gDevRf24CsPin != NULL);
    gDevRf24CePin = KDINSTANCE(GPIO, cePath);
    ASSERT(gDevRf24CePin != NULL);
#endif
    kdgpio_init((void *) gDevRf24CsPin);
    kdgpio_powerUp((void *) gDevRf24CsPin, KDGPIO_MODE_OUTPUT_PP, KDGPIO_PULL_NONE);
    kdgpio_output((void *) gDevRf24CsPin, 1);

    kdgpio_init((void *) gDevRf24CePin);
    kdgpio_powerUp((void *) gDevRf24CePin, KDGPIO_MODE_OUTPUT_PP, KDGPIO_PULL_NONE);
    kdgpio_output((void *) gDevRf24CePin, 0);

    kdspi_init((void *) gDevRf24Interface);
    kdspi_powerUp((void *) gDevRf24Interface);
}


void rf24_finalizeBus(void) {
#if CONFIG_DRIVER_USING_CONST == 0
    if (gDevRf24CePin == NULL
        || gDevRf24CsPin == NULL
        || gDevRf24Interface == NULL) {
        return;
    }
#endif
    kdgpio_output((void *) gDevRf24CePin, 0);
    kdgpio_powerDown((void *) gDevRf24CePin);
    kdgpio_finalize((void *) gDevRf24CePin);

    kdgpio_output((void *) gDevRf24CsPin, 0);
    kdgpio_powerDown((void *) gDevRf24CsPin);
    kdgpio_finalize((void *) gDevRf24CsPin);

    kdspi_powerDown((void *) gDevRf24Interface);
    kdspi_finalize((void *) gDevRf24Interface);

#if CONFIG_DRIVER_USING_CONST == 0
    gDevRf24CePin = NULL;
    gDevRf24CsPin = NULL;
    gDevRf24Interface = NULL:
#endif
}


void rf24_finalizeSoft(void) {
}


int32_t rf24_initSoft(RF24_Chip_t chip) {
    qSTimer_t wait;

    memset(&mInstance, 0x00, sizeof(mInstance));
    mInstance.chip = chip;

    softReset();
    qSTimer_Set(&wait, 10);
    while (!qSTimer_Expired(&wait));

    rf24_flow_flushTx();
    rf24_flow_flushRx();
    rf24_status_clearAll();

    rf24_rf_setFreqOffset(79);

    if (mInstance.chip == RF24_CHIP_XN297) {
        special_xn297_caliInit();
        special_xn297_magicInit();
    }

    if (rf24_rf_getFreqOffset() != 79) {
        return -1;
    }

    return 0;
}


void rf24_setOnReceived(RF24_OnRecviced_t on) {
    mInstance.onReceived = on;
}


void rf24_setOnBlock(void (*onBlock)(void)) {
    mInstance.onBlock = onBlock;
}


int32_t rf24_loop(void) {
    uint8_t st = regRead(0x07);
    if (st & 0x40) {
        if (mInstance.onReceived != NULL) {
            uint8_t regData = regRead(0x07);
            uint8_t pipe = (regData >> 1) & 0x07;

            mInstance.onReceived(pipe);
            return 1;
        }
    }

    return 0;
}

/*@}*/

/**
 * @addtogroup StatusFunc
 * @note none
 */

/*@{*/

void rf24_status_clearAll(void) {
    uint8_t regData = regRead(0x07);
    regWrite(0x07, 0x70);
}


void rf24_status_clearTxFlag(void) {
    uint8_t regData = regRead(0x07);
    regData &= ~(0x20 | 0x10);
    regWrite(0x07, regData);
}


void rf24_status_clearRxFlag(void) {
    uint8_t regData = regRead(0x07);
    regData &= ~(0x40);
    regWrite(0x07, regData);
}


RF24_Status_t rf24_status_get(void) {
    uint8_t regData = regRead(0x07);
    return regData;
}

/*@}*/

/**
 * @addtogroup AddressFunc
 * @note none
 */

/*@{*/

void rf24_addr_setWidth(uint8_t aw) {
    mInstance.addressWidth = aw; // store
    aw = (aw - 2) & 0x03;

    // now:
    // 5 -> 0b11,
    // 4 -> 0b10,
    // 3 -> 0b01,
    // 2 (if works) -> 0b00
    regWrite(0x03, aw);
}


void rf24_addr_setTxAddr(uint64_t addr) {
    setAddress(0x10, addr, mInstance.addressWidth);
}


void rf24_addr_setTxAddrU8(uint8_t *addr) {
    setAddressU8(0x10, addr, mInstance.addressWidth);
}

/*@}*/

/**
 * @addtogroup RfSetupFunc
 * @note none
 */

/*@{*/

inline void rf24_rf_setCe(uint8_t hl) {
    if (hl) {
        setCeHigh();
    } else {
        setCeLow();
    }
}


void rf24_rf_setDataRate(RF24_DateRate_t rate) {
    uint8_t regData = regRead(0x06);

    if (rate == RF24_DR_2M) {
        if (mInstance.chip == RF24_CHIP_SI24R1) {
            regData &= ~((0x03 << 3) | (0x03 << 5));
            regData |= (0x01 << 3);
        } else if (mInstance.chip == RF24_CHIP_XN297) {
            regData &= ~(0x03 << 6);
            regData |= (0x01 << 6);
        }
    } else if (rate == RF24_DR_250K) {
        if (mInstance.chip == RF24_CHIP_SI24R1) {
            regData &= ~((0x03 << 3) | (0x03 << 5));
            regData |= (0x01 << 5);
        } else if (mInstance.chip == RF24_CHIP_XN297) {
            regData &= ~(0x03 << 6);
            regData |= (0x03 << 6);
        }
    } else if (rate == RF24_DR_1M) {
        if (mInstance.chip == RF24_CHIP_SI24R1) {
            regData &= ~((0x03 << 3) | (0x03 << 5));
        } else if (mInstance.chip == RF24_CHIP_XN297) {
            regData &= ~(0x03 << 6);
        }
    }

    regWrite(0x06, regData);
}


void rf24_rf_setPowerLevel(RF24_RfPower_t power) {
    uint8_t regData = regRead(0x06);
    if (mInstance.chip == RF24_CHIP_SI24R1) {
        regData &= ~(0x07);
        regData |= power;
    } else if (mInstance.chip == RF24_CHIP_XN297) {
        regData &= ~(0x3F);
        regData |= power;
    }

    regWrite(0x06, regData);
}


void rf24_rf_setFreqOffset(uint8_t freq) {
    regWrite(0x05, (uint8_t) (freq & 0x7F));
}


uint8_t rf24_rf_getFreqOffset(void) {
    uint8_t regData = regRead(0x05);
    return regData & 0x7F;
}


void rf24_rf_setCrcSize(RF24_CrcSize_t crcSize) {
    uint8_t regData = regRead(0x00);

    if (crcSize == RF24_CRC_OFF) {
        regData &= ~0x08;

        if (mInstance.chip == RF24_CHIP_SI24R1) {
        } else if (mInstance.chip == RF24_CHIP_XN297) {
            regData |= 0x04 | 0x80;
        }

        regWrite(0x00, regData);
    } else if (crcSize == RF24_CRC_1BYTE) {
        if (mInstance.chip == RF24_CHIP_SI24R1) {
            regData &= ~(0x08 | 0x04);
            regData |= 0x08;

            regWrite(0x00, regData);
        } else if (mInstance.chip == RF24_CHIP_XN297) {
        }
    } else if (crcSize == RF24_CRC_2BYTE) {
        if (mInstance.chip == RF24_CHIP_SI24R1) {
            regData |= 0x04 | 0x08;
        } else if (mInstance.chip == RF24_CHIP_XN297) {
            regData |= 0x08;
            regData |= 0x04 | 0x80;
        }
        regWrite(0x00, regData);
    }
}


void rf24_rf_powerControl(uint8_t up) {
    uint8_t regData = regRead(0x00);
    if (up) {
        regData |= 0x02;
    } else {
        regData &= ~0x02;
    }
    if (mInstance.chip == RF24_CHIP_XN297) {
        regData |= 0x04 | 0x80;
    }
    regWrite(0x00, regData);
}


void rf24_rf_setMode(RF24_Mode_t mode) {
    uint8_t regData = regRead(0x00);
    if (mode == RF24_MODE_TX) {
        regData &= ~0x01;
    } else {
        regData |= 0x01;
    }
    // regData |= 0x04 | 0x80;
    regWrite(0x00, regData);
}

/*@}*/

/**
 * @addtogroup PipeFunc
 * @note none
 */

/*@{*/

void rf24_pipe_enable(uint8_t pipeNumber, uint8_t en) {
    if (pipeNumber >= 6) {
        return;
    }

    uint8_t regData = regRead(0x02);
    if (en) {
        regData |= (1 << pipeNumber);
    } else {
        regData &= ~(1 << pipeNumber);
    }
    regWrite(0x02, regData);
}


inline void rf24_pipe_setAddr(uint8_t pipeNumber, uint64_t addr) {
    setAddress(0x0A + pipeNumber, addr, mInstance.addressWidth);
}


inline void rf24_pipe_setAddrU8(uint8_t pipeNumber, uint8_t *addr) {
    setAddressU8(0x0A + pipeNumber, addr, mInstance.addressWidth);
}


void rf24_pipe_setPayloadLength(uint8_t pipeNumber, uint8_t payloadW) {
    if (pipeNumber >= 6) {
        return;
    }

    regWrite(0x11 + pipeNumber, payloadW);
}


void rf24_pipe_enableDynPayloadLength(uint8_t pipeNumber, uint8_t en) {
    if (pipeNumber >= 6) {
        return;
    }

    uint8_t regData = regRead(0x1C);
    if (en) {
        regData |= (1 << pipeNumber);
    } else {
        regData &= ~(1 << pipeNumber);
    }
    regWrite(0x1C, regData);
}


void rf24_pipe_enableAutoAck(uint8_t pipeNumber, uint8_t en) {
    if (pipeNumber >= 6) {
        return;
    }

    uint8_t regData = regRead(0x01);
    if (en) {
        regData |= (1 << pipeNumber);
    } else {
        regData &= ~(1 << pipeNumber);
    }
    regWrite(0x01, regData);
}

/*@}*/

/**
 * @addtogroup FeatureFunc
 * @note none
 */

/*@{*/

void rf24_feature_setDataLengthMode(RF24_FeatureDataLengthMode_t mode) {
    uint8_t regData = regRead(0x1D);

    if (mInstance.chip == RF24_CHIP_XN297) {
        if (mode == RF24_FEATURE_DATA_LEN_MODE_B64) {
            regData |= (0x03 << 3);
        } else {
            regData &= ~(0x03 << 3);
        }
        regWrite(0x1D, regData);
    } else if (mInstance.chip == RF24_CHIP_SI24R1) {
    }
}


void rf24_feature_enableDynPayloadLength(uint8_t en) {
    uint8_t regData = regRead(0x1D);

    if (en) {
        regData |= 0x04;
    } else {
        regData &= ~0x04;
    }
    regWrite(0x1D, regData);
}


void rf24_feature_enableAckWithPayload(uint8_t en) {
    uint8_t regData = regRead(0x1D);

    if (en) {
        regData |= 0x02;
    } else {
        regData &= ~0x02;
    }

    regWrite(0x1D, regData);
}


void rf24_feature_enableTxPayloadNoAck(uint8_t en) {
    uint8_t regData = regRead(0x1D);
    if (en) {
        regData |= 0x01;
    } else {
        regData &= ~0x01;
    }
    regWrite(0x1D, regData);
}


void rf24_feature_setAutoRetxDelay(uint16_t us) {
    us = (us / 250) - 1;

    // 250 -> 0b0000
    // 500 -> 0b0001
    // ...
    // 4000 -> 0b1111

    uint8_t regData = regRead(0x04);

    regData &= ~(0xF0);
    regData |= (us << 4) & 0xF0;

    regWrite(0x04, regData);
}


void rf24_feature_setAutoRetxCount(uint8_t count) {
    uint8_t regData = regRead(0x04);

    regData &= ~(0x0F);
    regData |= count & 0x0F;

    regWrite(0x04, regData);
}

/*@}*/

/**
 * @addtogroup TransmitFunc
 * @note none
 */

/*@{*/

int32_t rf24_flow_send(uint8_t *data, uint8_t length,
                       uint32_t timeout,
                       RF24_TransmitAckMode_t ack,
                       RF24_TransmitEndCeMode_t endCeMode) {
    int32_t rc = 0;
    uint8_t st = 0;
    qSTimer_t wait;

    setCeLow();

    rf24_flow_flushTx();

    if (ack == RF24_TRANSMIT_ACK_MODE_ACK) {
        regDataWrites(0xA0, data, length);
        setCeHigh();
    } else if (ack == RF24_TRANSMIT_ACK_MODE_NOACK) {
        regDataWrites(0xB0, data, length);
        setCeHigh();
    } else if (ack >= 10) {
        regDataWrites(0xA8 + (ack - 10), data, length);
    }
    qSTimer_Set(&wait, 3);
    while (!qSTimer_Expired(&wait)) {
        if (mInstance.onBlock) {
            mInstance.onBlock();
        }
    }

    if (ack == RF24_TRANSMIT_ACK_MODE_ACK || ack == RF24_TRANSMIT_ACK_MODE_NOACK) {
        uint8_t maxrt = 0;
        qSTimer_Set(&wait, timeout);
        while (!qSTimer_Expired(&wait)) {
            st = regRead(0x07);
            if (st & 0x20) {
                break;
            }
            if (st & 0x10) {
                maxrt = 1;
                break;
            }
            if (mInstance.onBlock) {
                mInstance.onBlock();
            }
        }

        if (qSTimer_Expired(&wait) || maxrt) {
            rc = -1;
        }

        // rf24_flow_flushTx();

        st |= 0x30;
        regWrite(0x07, st);
    }

    if (!endCeMode) {
        setCeLow();
    } else {
        setCeHigh();
    }

    return rc;
}


uint8_t rf24_flow_isTransmitSuccess(void) {
    uint8_t st = regRead(0x07);
    if (st & 0x20) {
        return 1;
    }

    return -1;
}


int32_t rf24_flow_hasRecvData(void) {
    uint8_t st = regRead(0x07);
    if (st & 0x40) {
        uint8_t regData = regRead(0x07);
        uint8_t pipe = (regData >> 1) & 0x07;
        return pipe;
    }

    return -1;
}


void rf24_flow_read(uint8_t *data, uint8_t length) {
    regReads(0x61, data, length);
}


uint8_t rf24_flow_getRxFifoLength(void) {
    uint8_t regData = regRead(0x60);
    return regData;
}


void inline rf24_flow_flushTx(void) {
    static const uint8_t FLUSHTX[] = {0xE1, 0x00};
    dataWrites((uint8_t *) FLUSHTX, 2);
}


void inline rf24_flow_flushRx(void) {
    static const uint8_t FLUSHRX[] = {0xE2, 0x00};
    dataWrites((uint8_t *) FLUSHRX, 2);
}

/*@}*/

/**
 * @addtogroup Debug support
 * @note none
 */

/*@{*/

#undef DBG_SECTION_NAME

/*@}*/
