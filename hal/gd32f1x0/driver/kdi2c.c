/*
 * Copyright (C) 2020 Flandreunx@outlook.com
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

/**
 * @addtogroup Mutex support
 * @note none
 */
 
/*@{*/

static void mutexNew(kdi2c_t *kd) {
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    const osMutexAttr_t mutex_attr = {
        .attr_bits = osMutexRobust | osMutexPrioInherit
    };
    
    kd->_va->mutex = osMutexNew(&mutex_attr);
    ASSERT(kd->_va->mutex != NULL);
#endif
}


static void mutexDelete(kdi2c_t *kd) {
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    osMutexDelete(kd->_va->mutex);
    kd->_va->mutex = NULL;
#endif
}


static inline void mutexLock(kdi2c_t *kd) {
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    osMutexAcquire(kd->_va->mutex, osWaitForever);
#endif
}


static inline void mutexUnlock(kdi2c_t *kd) {
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    osMutexRelease(kd->_va->mutex);
#endif
}


static inline void threadYield(void) {
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    osThreadYield();
#endif
}

/*@}*/

/**
 * @addtogroup Soc Support
 * @note none
 */
 
/*@{*/

static inline int32_t halBlockFlagIs(kdi2c_t *kd, uint32_t flagWait) {
    qSTimer_t tmcd;
    
    qSTimer_Set(&tmcd, 1000);
    while (i2c_flag_get(kd->_instance.i2c, flagWait) 
            && (!qSTimer_Expired(&tmcd))) {
        threadYield();
    }
            
    if (qSTimer_Expired(&tmcd)) {
        return -1;
    } else {
        return 0;
    }
}


static inline int32_t halBlockDmaTxc(kdi2c_t *kd) {
    qSTimer_t tmcd;
    
    qSTimer_Set(&tmcd, 1000);
    while (!dma_flag_get(kd->_config.dma.channelTx, DMA_FLAG_FTF) 
            && (!qSTimer_Expired(&tmcd))) {
        threadYield();
    }
            
    if (qSTimer_Expired(&tmcd)) {
        return -1;
    } else {
        return 0;
    }
}


static inline int32_t halBlockDmaRxc(kdi2c_t *kd) {
    qSTimer_t tmcd;
    
    qSTimer_Set(&tmcd, 1000);
    while (!dma_flag_get(kd->_config.dma.channelRx, DMA_FLAG_FTF) 
            && (!qSTimer_Expired(&tmcd))) {
        threadYield();
    }
            
    if (qSTimer_Expired(&tmcd)) {
        return -1;
    } else {
        return 0;
    }
}


static inline int32_t halBlockFlagIsNot(kdi2c_t *kd, uint32_t flagWait) {
    qSTimer_t tmcd;

    qSTimer_Set(&tmcd, 1000);
    while (!i2c_flag_get(kd->_instance.i2c, flagWait) 
            && (!qSTimer_Expired(&tmcd))) {
        threadYield();
    }
            
    if (qSTimer_Expired(&tmcd)) {
        return -1;
    } else {
        return 0;
    }
}


static inline int32_t halBlockStop(kdi2c_t *kd) {
    qSTimer_t tmcd;
    
    qSTimer_Set(&tmcd, 1000);
    
    // i2c_stop_on_bus(kd->_instance.i2c);
    while ((I2C_CTL0(kd->_instance.i2c) & I2C_CTL0_STOP)
            && (!qSTimer_Expired(&tmcd))) {
        threadYield();
    }
    
    if (qSTimer_Expired(&tmcd)) {
        return -1;
    } else {
        return 0;
    }
}

/*@}*/

/**
 * @addtogroup BB support
 * @note none
 */
 
/*@{*/

#define SCL_L(d) \
    kdgpio_brr(d->_config.pin.scl.gpio)

#define SCL_H(d) \
    kdgpio_bsrr(d->_config.pin.scl.gpio)
    
#define SDA_L(d) \
    kdgpio_brr(d->_config.pin.sda.gpio)

#define SDA_H(d) \
    kdgpio_bsrr(d->_config.pin.sda.gpio)

#define READ_SDA(d) \
    kdgpio_input(d->_config.pin.sda.gpio)
    
#define READ_SCL(d) \
    kdgpio_input(d->_config.pin.scl.gpio)
    
#define DELAY(d) \
    bbLineDelay(d->_config.bb.delay)

__USED static inline void bbLineDelay(uint32_t d) {
    while (d--) {
        __NOP();
    }
}


static int32_t bbStart(kdi2c_t *kd) {
    register uint32_t wait = 0;
        
    SDA_H(kd);
    DELAY(kd);
    
    SCL_H(kd);
    DELAY(kd);
    
    while ((!READ_SCL(kd)) && (++wait <= kd->_config.bb.timeout)) {
        DELAY(kd);
    }

    if (!READ_SDA(kd)) {
        return -1;
    }
    
    SDA_L(kd);
    DELAY(kd);
    
    if (READ_SDA(kd)) {
        return -1;
    }

    SCL_L(kd);
    DELAY(kd);
    
    return 0;
}

static int32_t bbRestart(kdi2c_t *kd) {
    register uint32_t wait = 0;
        
    SDA_H(kd);
    DELAY(kd);
    
    SCL_H(kd);
    DELAY(kd);
    
    while ((!READ_SCL(kd)) && (++wait <= kd->_config.bb.timeout)) {
        DELAY(kd);
    }

    if (!READ_SDA(kd)) {
        return -1;
    }
    
    SDA_L(kd);
    DELAY(kd);
    
    if (READ_SDA(kd)) {
        return -1;
    }
    
    SCL_L(kd);
    DELAY(kd);
    
    return 0;
}


static void bbStop(kdi2c_t *kd) {
    register uint32_t wait = 0;
    
    SCL_L(kd);
    DELAY(kd);
    
    SDA_L(kd);
    DELAY(kd);
    
    SCL_H(kd);
    while ((!READ_SCL(kd)) && (++wait <= kd->_config.bb.timeout)) {
        DELAY(kd);
    }
    DELAY(kd);
   
    SDA_H(kd);
    DELAY(kd);
}


static void bbMack(kdi2c_t *kd) {
    register uint32_t wait = 0;
    
    SCL_L(kd);
    DELAY(kd);
    
    SDA_L(kd);
    DELAY(kd);
    
    SCL_H(kd);
    while ((!READ_SCL(kd)) && (++wait <= kd->_config.bb.timeout)) {
        DELAY(kd);
    }
    DELAY(kd);
   
    SCL_L(kd);
    DELAY(kd);
}


static void bbNack(kdi2c_t *kd) {
    register uint32_t wait = 0;
    
    SCL_L(kd);
    DELAY(kd);
    
    SDA_H(kd);
    DELAY(kd);
    
    SCL_H(kd);
    while ((!READ_SCL(kd)) && (++wait <= kd->_config.bb.timeout)) {
        DELAY(kd);
    }
    DELAY(kd);
    
    SCL_L(kd);
    DELAY(kd);
}


static int32_t bbWack(kdi2c_t *kd) {
    register uint32_t wait = 0;
    
    DELAY(kd);
    
    SCL_L(kd);
    DELAY(kd);
    
    SDA_H(kd);
    DELAY(kd);

    while ((READ_SDA(kd)) && (++wait <= kd->_config.bb.timeout)) {
        DELAY(kd);
    }
    
    if (!READ_SDA(kd)) {
        SCL_H(kd);
        while ((!READ_SCL(kd)) && (++wait <= kd->_config.bb.timeout)) {
            DELAY(kd);
        }
        DELAY(kd);
        SCL_L(kd);
        
        return 0;
    }
    
    return -1;
}


static void bbSendByte(kdi2c_t *kd, uint8_t byte) {
    register uint32_t i = 8;
    register uint32_t wait = 0;
    
    while (i--) {
        SCL_L(kd);
        DELAY(kd);
        
        if (byte & 0x80) {
            SDA_H(kd);
        } else {
            SDA_L(kd);
        }
        DELAY(kd);
        
        SCL_H(kd);
        DELAY(kd);
        
        while ((!READ_SCL(kd)) && (++wait <= kd->_config.bb.timeout)) {
            DELAY(kd);
        }
        DELAY(kd);
        
        byte <<= 1;
    }
    
    SCL_L(kd);
}


static uint8_t bbReadByte(kdi2c_t *kd) {
    register uint32_t i = 8;
    register uint8_t byte = 0;
    register uint32_t wait = 0;

    SDA_H(kd);
    
    while (i--) {
        byte <<= 1;
        
        SCL_L(kd);
        DELAY(kd);
        
        SCL_H(kd);
        while ((!READ_SCL(kd)) && (++wait <= kd->_config.bb.timeout)) {
            DELAY(kd);
        }
        DELAY(kd);
        
        if (READ_SDA(kd)) {
            byte |= 0x01;
        }
        
    }
    
    SCL_L(kd);

    return byte;
}


static void bbPinConfig(kdi2c_t *kd, uint8_t en) {
    if (en) {
        kdgpio_powerUp(kd->_config.pin.sda.gpio, KDGPIO_MODE_OUTPUT_OD, KDGPIO_PULL_NONE);
        kdgpio_powerUp(kd->_config.pin.scl.gpio, KDGPIO_MODE_OUTPUT_OD, KDGPIO_PULL_NONE);
    }
    
    if (!READ_SDA(kd)) {
        for (register uint32_t i = 0; i < 9; i++) {
            SCL_L(kd);
            DELAY(kd);
            SCL_H(kd);
            DELAY(kd);
        }
    }
    
    SCL_H(kd);
    SDA_H(kd);
    
    if (!en) {
        kdgpio_powerDown(kd->_config.pin.sda.gpio);
        kdgpio_powerDown(kd->_config.pin.scl.gpio);
    }
}

/*@}*/

/**
 * @addtogroup Public Functions
 * @note none
 */
 
/*@{*/

int32_t kdi2c_init(kdi2c_t *kd) {
    ASSERT(kd != NULL);


    if (emmkDriver_initRefsCountUp(&kd->_va->initRefs) != 0) {
        return -1;
    }
    
    kd->_instance.enableFunc(kd);
    
    if (kd->_instance.i2c != NULL) {
        i2c_clock_config(kd->_instance.i2c, kd->_instance.clockSpeed, kd->_instance.dutyCycle);
        i2c_mode_addr_config(kd->_instance.i2c, I2C_I2CMODE_ENABLE, I2C_ADDFORMAT_7BITS, 0);
    }
    
    mutexNew(kd);
    
    return 0;
}


int32_t kdi2c_finalize(kdi2c_t *kd) {
    ASSERT(kd != NULL);
    
    if (emmkDriver_initRefsCountDown(&kd->_va->initRefs) != 0) {
        return -1;
    }
    if (kd->_va->initRefsPower != 0) {
        kd->_va->initRefsPower = 1;
        kdi2c_powerDown(kd);
    }
    
    if (kd->_instance.i2c != NULL) {
        
    }

    kd->_instance.disableFunc(kd);
    
    mutexDelete(kd);
    
    return 0;
}


int32_t kdi2c_powerUp(kdi2c_t *kd) {
    if (kd->_instance.i2c != NULL) {
        kdgpio_powerUp(kd->_config.pin.sda.gpio, KDGPIO_MODE_AF_OD, KDGPIO_PULL_NONE);
        kdgpio_powerUp(kd->_config.pin.scl.gpio, KDGPIO_MODE_AF_OD, KDGPIO_PULL_NONE);
        
        i2c_enable(kd->_instance.i2c);
        i2c_ack_config(kd->_instance.i2c, I2C_ACK_ENABLE);

        if (kd->_config.dma.channelTx != 0xFF) {
            dma_init(kd->_config.dma.channelTx, &kd->_config.dma.initTx);
        } 
        if (kd->_config.dma.channelRx != 0xFF) {
            dma_init(kd->_config.dma.channelRx, &kd->_config.dma.initRx);
        }
    } else {
        bbPinConfig(kd, 1);
    }
    
    return 0;
}


int32_t kdi2c_powerDown(kdi2c_t *kd) {
    if (emmkDriver_initRefsCountDown(&kd->_va->initRefsPower) != 0) {
        return -1;
    }
    
    if (kd->_instance.i2c != NULL) {
        kdgpio_powerDown(kd->_config.pin.sda.gpio);
        kdgpio_powerDown(kd->_config.pin.scl.gpio);
        
        dma_channel_disable(kd->_config.dma.channelTx);
        dma_deinit(kd->_config.dma.channelTx);
        dma_channel_disable(kd->_config.dma.channelRx);
        dma_deinit(kd->_config.dma.channelRx);
        
        i2c_disable(kd->_instance.i2c);
    } else {
        bbPinConfig(kd, 0);
    }
    
    return 0;
}


void kdi2c_reset(kdi2c_t *kd) {
    if (kd->_instance.i2c != NULL) {
        i2c_deinit(kd->_instance.i2c);
        
        bbPinConfig(kd, 1);
        
        kdgpio_powerUp(kd->_config.pin.sda.gpio, KDGPIO_MODE_AF_OD, KDGPIO_PULL_NONE);
        kdgpio_powerUp(kd->_config.pin.scl.gpio, KDGPIO_MODE_AF_OD, KDGPIO_PULL_NONE);
        
        i2c_clock_config(kd->_instance.i2c, kd->_instance.clockSpeed, kd->_instance.dutyCycle);
        i2c_mode_addr_config(kd->_instance.i2c, I2C_I2CMODE_ENABLE, I2C_ADDFORMAT_7BITS, 0);
        i2c_enable(kd->_instance.i2c);
        i2c_ackpos_config(kd->_instance.i2c, I2C_ACKPOS_CURRENT);
        i2c_ack_config(kd->_instance.i2c, I2C_ACK_ENABLE);
    } else {
        bbStop(kd);
        bbPinConfig(kd, 1);
    }
}


void kdi2c_setSpeed(kdi2c_t *kd, uint32_t speed) {
    mutexLock(kd);
    
    if (kd->_instance.i2c != NULL) {
        halBlockFlagIs(kd, I2C_FLAG_I2CBSY);
       
        i2c_disable(kd->_instance.i2c);
        i2c_clock_config(kd->_instance.i2c, speed, kd->_instance.dutyCycle);
        i2c_enable(kd->_instance.i2c);
    }

    mutexUnlock(kd);
    return;
}


int32_t kdi2c_transmit(kdi2c_t *kd, uint8_t addr,
        uint8_t *wbuf, uint16_t wsize,
        uint8_t *rbuf, uint16_t rsize) { 
    int32_t rc = -1;
            
    mutexLock(kd);
    
    addr <<= 1;
            
    if (kd->_instance.i2c != NULL) {   
        if (halBlockFlagIs(kd, I2C_FLAG_I2CBSY) != 0) {
            goto l_exit;
        }
        
        if (rsize == 2) {
            i2c_ackpos_config(kd->_instance.i2c, I2C_ACKPOS_NEXT);
        }

        /// Write
        
        i2c_start_on_bus(kd->_instance.i2c);
        if (halBlockFlagIsNot(kd, I2C_FLAG_SBSEND) != 0) {
            goto l_exit;
        }

        i2c_master_addressing(kd->_instance.i2c, addr, I2C_TRANSMITTER);
        if (halBlockFlagIsNot(kd, I2C_FLAG_ADDSEND) != 0) {
            goto l_exit;
        }
        i2c_flag_clear(kd->_instance.i2c, I2C_FLAG_ADDSEND);

        if (halBlockFlagIsNot(kd, I2C_FLAG_TBE) != 0) {
            goto l_exit;
        }
        if (kd->_config.dma.channelTx != 0xFF) {
            i2c_dma_enable(kd->_instance.i2c, I2C_DMA_ON);
            dma_interrupt_flag_clear(
                kd->_config.dma.channelTx, 
                DMA_INT_FLAG_G | DMA_INT_FLAG_FTF | DMA_INT_FLAG_ERR
            );
            DMA_CHCTL(kd->_config.dma.channelTx) &= ~DMA_CHXCTL_CHEN;
            DMA_CHCNT(kd->_config.dma.channelTx) = (wsize);
            DMA_CHMADDR(kd->_config.dma.channelTx) = (uint32_t) wbuf;
            DMA_CHCTL(kd->_config.dma.channelTx) |= DMA_CHXCTL_CHEN;
            if (halBlockDmaTxc(kd) != 0) {
                dma_interrupt_flag_clear(
                    kd->_config.dma.channelTx, 
                    DMA_INT_FLAG_G | DMA_INT_FLAG_FTF | DMA_INT_FLAG_ERR
                );
                DMA_CHCTL(kd->_config.dma.channelTx) &= ~DMA_CHXCTL_CHEN;
                goto l_exit;
            }
            if (halBlockFlagIsNot(kd, I2C_FLAG_BTC) != 0) {
                goto l_exit;
            }
        } else {
            i2c_enable(kd->_instance.i2c);
            
            while (wsize--) {
                i2c_data_transmit(kd->_instance.i2c, *wbuf);
                wbuf++;
                if (halBlockFlagIsNot(kd, I2C_FLAG_BTC) != 0) {
                    goto l_exit;
                }
            }
        }
        
        /// Read
        i2c_start_on_bus(kd->_instance.i2c);
        if (halBlockFlagIsNot(kd, I2C_FLAG_SBSEND) != 0) {
            goto l_exit;
        }
        
        i2c_master_addressing(kd->_instance.i2c, addr, I2C_RECEIVER);
        if (kd->_config.dma.channelRx != 0xFF) {
            if (rsize < 2) {
               goto l_blockRead;
            } else {
                if (halBlockFlagIsNot(kd, I2C_FLAG_ADDSEND) != 0) {
                    goto l_exit;
                }
                i2c_flag_clear(kd->_instance.i2c, I2C_FLAG_ADDSEND);
            
                i2c_dma_enable(kd->_instance.i2c, I2C_DMA_ON);
                dma_interrupt_flag_clear(
                    kd->_config.dma.channelRx, 
                    DMA_INT_FLAG_G | DMA_INT_FLAG_FTF | DMA_INT_FLAG_ERR
                );
                DMA_CHCTL(kd->_config.dma.channelRx) &= ~DMA_CHXCTL_CHEN;
                DMA_CHCNT(kd->_config.dma.channelRx) = (rsize);
                DMA_CHMADDR(kd->_config.dma.channelRx) = (uint32_t) rbuf;
                i2c_dma_last_transfer_config(kd->_instance.i2c, I2C_DMALST_ON);
                DMA_CHCTL(kd->_config.dma.channelRx) |= DMA_CHXCTL_CHEN;
                if (halBlockDmaRxc(kd) != 0) {
                    dma_interrupt_flag_clear(
                        kd->_config.dma.channelRx, 
                        DMA_INT_FLAG_G | DMA_INT_FLAG_FTF | DMA_INT_FLAG_ERR
                    );
                    DMA_CHCTL(kd->_config.dma.channelRx) &= ~DMA_CHXCTL_CHEN;
                    goto l_exit;
                } 
            }
            
            i2c_stop_on_bus(kd->_instance.i2c);
            if (halBlockStop(kd) != 0) {
                goto l_exit;
            }
        } else {
            l_blockRead:
            if (rsize < 3) {
                i2c_ack_config(kd->_instance.i2c, I2C_ACK_DISABLE);
            } else {
                i2c_ack_config(kd->_instance.i2c, I2C_ACK_ENABLE);
            }
            if (halBlockFlagIsNot(kd, I2C_FLAG_ADDSEND) != 0) {
                goto l_exit;
            }
            i2c_flag_clear(kd->_instance.i2c, I2C_FLAG_ADDSEND);
            if (rsize == 1) {
                i2c_stop_on_bus(kd->_instance.i2c);
            }
            while (rsize) {
                if (rsize == 3) {
                    if (halBlockFlagIsNot(kd, I2C_FLAG_BTC) != 0) {
                        goto l_exit;
                    }
                    i2c_ack_config(kd->_instance.i2c, I2C_ACK_DISABLE);
                }
                if (rsize == 2) {
                    if (halBlockFlagIsNot(kd, I2C_FLAG_BTC) != 0) {
                        goto l_exit;
                    }
                    i2c_stop_on_bus(kd->_instance.i2c);
                }
                if (halBlockFlagIsNot(kd, I2C_FLAG_RBNE) != 0) {
                    goto l_exit;
                }
                *rbuf = i2c_data_receive(kd->_instance.i2c);
                rbuf++;
                rsize--;
            }
        
            if (halBlockStop(kd) != 0) {
                goto l_exit;
            }
        }
        
        rc = 0;
    } else {
        if (bbStart(kd) != 0) {
            rc = KDI2CBB_ERR_START;
            goto l_exit;
        }
        
        bbSendByte(kd, addr & 0xFE);
        if (bbWack(kd) != 0) {
            rc = KDI2CBB_ERR_WACK;
            goto l_exit;
        }
        
        for (rc = 0; rc < wsize; rc++) {
            bbSendByte(kd, wbuf[rc]);
            if (bbWack(kd) != 0) {
                rc = KDI2CBB_ERR_WACK;
                goto l_exit;
            }
        }
        
        if (bbRestart(kd) != 0) {
            rc = KDI2CBB_ERR_START;
            goto l_exit;
        }
        
        bbSendByte(kd, addr | 0x01);
        if (bbWack(kd) != 0) {
            rc = KDI2CBB_ERR_WACK;
            goto l_exit;
        }
        
        for (rc = 0; rc < rsize; rc++) {
            *(rbuf++) = bbReadByte(kd);
            if (rc == (rsize - 1)) {
                bbNack(kd);
            } else {
                bbMack(kd);
            }
        }
        
        bbStop(kd);
        
        rc = 0;
    }

l_exit:
    if (kd->_instance.i2c != NULL) {
        i2c_dma_enable(kd->_instance.i2c, I2C_DMA_OFF);
        i2c_ack_config(kd->_instance.i2c, I2C_ACK_ENABLE);
        i2c_ackpos_config(kd->_instance.i2c, I2C_ACKPOS_CURRENT);
    }
    
    if (rc != 0) {
        kdi2c_reset(kd);
        mutexUnlock(kd);
        return -1;
    }

    mutexUnlock(kd);
    return 0;
}
        

int32_t kdi2c_write(kdi2c_t *kd, uint8_t addr,
        uint8_t *wbuf, uint16_t wsize) {
    int32_t rc = -1;
            
    mutexLock(kd);
    
    addr <<= 1;
            
    if (kd->_instance.i2c != NULL) {
        if (halBlockFlagIs(kd, I2C_FLAG_I2CBSY) != 0) {
            goto l_exit;
        }
        
        i2c_start_on_bus(kd->_instance.i2c);
        if (halBlockFlagIsNot(kd, I2C_FLAG_SBSEND) != 0) {
            goto l_exit;
        }
        
        i2c_master_addressing(kd->_instance.i2c, addr, I2C_TRANSMITTER);
        if (halBlockFlagIsNot(kd, I2C_FLAG_ADDSEND) != 0) {
            goto l_exit;
        }
        i2c_flag_clear(kd->_instance.i2c, I2C_FLAG_ADDSEND);
        
        i2c_dma_enable(kd->_instance.i2c, I2C_DMA_ON);

        if (halBlockFlagIsNot(kd, I2C_FLAG_TBE) != 0) {
            goto l_exit;
        }
        
        if (kd->_config.dma.channelTx != 0xFF) {
            dma_interrupt_flag_clear(
                kd->_config.dma.channelTx, 
                DMA_INT_FLAG_G | DMA_INT_FLAG_FTF | DMA_INT_FLAG_ERR
            );
            DMA_CHCTL(kd->_config.dma.channelTx) &= ~DMA_CHXCTL_CHEN;
            DMA_CHCNT(kd->_config.dma.channelTx) = (wsize);
            DMA_CHMADDR(kd->_config.dma.channelTx) = (uint32_t) wbuf;
            DMA_CHCTL(kd->_config.dma.channelTx) |= DMA_CHXCTL_CHEN;
            if (halBlockDmaTxc(kd) != 0) {
                dma_interrupt_flag_clear(
                    kd->_config.dma.channelTx, 
                    DMA_INT_FLAG_G | DMA_INT_FLAG_FTF | DMA_INT_FLAG_ERR
                );
                DMA_CHCTL(kd->_config.dma.channelTx) &= ~DMA_CHXCTL_CHEN;
                goto l_exit;
            }
            DMA_CHCTL(kd->_config.dma.channelTx) &= ~DMA_CHXCTL_CHEN;
            if (halBlockFlagIsNot(kd, I2C_FLAG_BTC) != 0) {
                goto l_exit;
            }
        } else {
            while (wsize--) {
                i2c_data_transmit(kd->_instance.i2c, *wbuf);
                wbuf++;
                if (halBlockFlagIsNot(kd, I2C_FLAG_BTC) != 0) {
                    goto l_exit;
                }
            }
        }
        
        i2c_stop_on_bus(kd->_instance.i2c);
        if (halBlockStop(kd) != 0) {
            goto l_exit;
        }
        
        rc = 0;
    } else {
        if (bbStart(kd) != 0) {
            rc = KDI2CBB_ERR_START;
            goto l_exit;
        }

        bbSendByte(kd, addr & 0xFE);
        if (bbWack(kd) != 0) {
            rc = KDI2CBB_ERR_WACK;
            goto l_exit;
        }
        
        for (rc = 0; rc < wsize; rc++) {
            bbSendByte(kd, wbuf[rc]);
            if (bbWack(kd) != 0) {
                rc = KDI2CBB_ERR_WACK;
                goto l_exit;
            }
        }
        
        bbStop(kd);
        
        rc = 0;
    }
    
l_exit:
    if (kd->_instance.i2c != NULL) {
        i2c_dma_enable(kd->_instance.i2c, I2C_DMA_OFF);
        i2c_ack_config(kd->_instance.i2c, I2C_ACK_ENABLE);
        i2c_ackpos_config(kd->_instance.i2c, I2C_ACKPOS_CURRENT);
    }
    
    if (rc != 0) {
        kdi2c_reset(kd);
        mutexUnlock(kd);
        return -1;
    }

    mutexUnlock(kd);
    return 0;
}
        

int32_t kdi2c_read(kdi2c_t *kd, uint8_t addr,
        uint8_t *rbuf, uint16_t rsize) {
    int32_t rc = -1;
          
    mutexLock(kd);
    
    addr <<= 1;
            
    if (kd->_instance.i2c != NULL) {
        if (halBlockFlagIs(kd, I2C_FLAG_I2CBSY) != 0) {
            goto l_exit;
        }
        
        if (rsize == 2) {
            i2c_ackpos_config(kd->_instance.i2c, I2C_ACKPOS_NEXT);
        }
        
        i2c_start_on_bus(kd->_instance.i2c);
        if (halBlockFlagIsNot(kd, I2C_FLAG_SBSEND) != 0) {
            goto l_exit;
        }
        
        i2c_master_addressing(kd->_instance.i2c, addr, I2C_RECEIVER);
        if (kd->_config.dma.channelRx != 0xFF) {
            if (rsize < 2) {
                goto l_blockRead;
            } else {
                if (halBlockFlagIsNot(kd, I2C_FLAG_ADDSEND) != 0) {
                    goto l_exit;
                }
                i2c_flag_clear(kd->_instance.i2c, I2C_FLAG_ADDSEND);
                
                i2c_dma_enable(kd->_instance.i2c, I2C_DMA_ON);
                dma_interrupt_flag_clear(
                    kd->_config.dma.channelRx, 
                    DMA_INT_FLAG_G | DMA_INT_FLAG_FTF | DMA_INT_FLAG_ERR
                );
                DMA_CHCTL(kd->_config.dma.channelRx) &= ~DMA_CHXCTL_CHEN;
                DMA_CHCNT(kd->_config.dma.channelRx) = (rsize);
                DMA_CHMADDR(kd->_config.dma.channelRx) = (uint32_t) rbuf;
                i2c_dma_last_transfer_config(kd->_instance.i2c, I2C_DMALST_ON);
                DMA_CHCTL(kd->_config.dma.channelRx) |= DMA_CHXCTL_CHEN;
                if (halBlockDmaRxc(kd) != 0) {
                    dma_interrupt_flag_clear(
                        kd->_config.dma.channelRx, 
                        DMA_INT_FLAG_G | DMA_INT_FLAG_FTF | DMA_INT_FLAG_ERR
                    );
                    DMA_CHCTL(kd->_config.dma.channelRx) &= ~DMA_CHXCTL_CHEN;
                    goto l_exit;
                }
                DMA_CHCTL(kd->_config.dma.channelRx) &= ~DMA_CHXCTL_CHEN;
            }
            
            i2c_stop_on_bus(kd->_instance.i2c);
            if (halBlockStop(kd) != 0) {
                goto l_exit;
            }
        } else {
            l_blockRead:
            if (rsize < 3) {
                i2c_ack_config(kd->_instance.i2c, I2C_ACK_DISABLE);
            } else {
                i2c_ack_config(kd->_instance.i2c, I2C_ACK_ENABLE);
            }
            if (halBlockFlagIsNot(kd, I2C_FLAG_ADDSEND) != 0) {
                goto l_exit;
            }
            i2c_flag_clear(kd->_instance.i2c, I2C_FLAG_ADDSEND);
            if (rsize == 1) {
                i2c_stop_on_bus(kd->_instance.i2c);
            }
            while (rsize) {
                if (rsize == 3) {
                    if (halBlockFlagIsNot(kd, I2C_FLAG_BTC) != 0) {
                        goto l_exit;
                    }
                    i2c_ack_config(kd->_instance.i2c, I2C_ACK_DISABLE);
                }
                if (rsize == 2) {
                    if (halBlockFlagIsNot(kd, I2C_FLAG_BTC) != 0) {
                        goto l_exit;
                    }
                    i2c_stop_on_bus(kd->_instance.i2c);
                }
                if (halBlockFlagIsNot(kd, I2C_FLAG_RBNE) != 0) {
                    goto l_exit;
                }
                *rbuf = i2c_data_receive(kd->_instance.i2c);
                rbuf++;
                rsize--;
            }
        
            if (halBlockStop(kd) != 0) {
                goto l_exit;
            }
        }
        
        rc = 0;
    } else {
        if (bbStart(kd) != 0) {
            rc = KDI2CBB_ERR_START;
            goto l_exit;
        }
        
        bbSendByte(kd, addr | 0x01);
        if (bbWack(kd) != 0) {
            rc = KDI2CBB_ERR_WACK;
            goto l_exit;
        }
        
        for (rc = 0; rc < rsize; rc++) {
            *(rbuf++) = bbReadByte(kd);
            if (rc == (rsize - 1)) {
                bbNack(kd);
            } else {
                bbMack(kd);
            }
        }
        
        bbStop(kd);
        
        rc = 0;
    }
    
l_exit:
    if (kd->_instance.i2c != NULL) {
        i2c_dma_enable(kd->_instance.i2c, I2C_DMA_OFF);
        i2c_ack_config(kd->_instance.i2c, I2C_ACK_ENABLE);
        i2c_ackpos_config(kd->_instance.i2c, I2C_ACKPOS_CURRENT);
    }
    
    if (rc != 0) {
        kdi2c_reset(kd);
        mutexUnlock(kd);
        return -1;
    }

    mutexUnlock(kd);
    return 0;
}
        

int32_t kdi2c_regWrite(kdi2c_t *kd, uint8_t addr,
        uint32_t reg, uint8_t regLen,
        uint8_t *data, uint16_t len) {
    int32_t rc = -1;
            
    mutexLock(kd);
    
    addr <<= 1;
            
    uint8_t regs[4] = {0};
    switch (regLen) {
        case 1:
            regs[0] = reg;
            break;
        case 2:
            regs[0] = reg >> 8 & 0x00FF;
            regs[1] = reg & 0x00FF;
            break;
        case 3:
            regs[0] = reg >> 16 & 0x0000FF;
            regs[1] = reg >> 8 & 0x0000FF;
            regs[2] = reg & 0x0000FF;
            break;
        case 4:
            regs[0] = reg >> 24 & 0x000000FF;
            regs[1] = reg >> 16 & 0x000000FF;
            regs[2] = reg >> 8 & 0x000000FF;
            regs[3] = reg & 0x000000FF;
            break;
    }
            
    if (kd->_instance.i2c != NULL) {    
        if (halBlockFlagIs(kd, I2C_FLAG_I2CBSY) != 0) {
            goto l_exit;
        }
        
        i2c_start_on_bus(kd->_instance.i2c);
        if (halBlockFlagIsNot(kd, I2C_FLAG_SBSEND) != 0) {
            goto l_exit;
        }
        
        i2c_master_addressing(kd->_instance.i2c, addr, I2C_TRANSMITTER);
        if (halBlockFlagIsNot(kd, I2C_FLAG_ADDSEND) != 0) {
            goto l_exit;
        }
        i2c_flag_clear(kd->_instance.i2c, I2C_FLAG_ADDSEND);
        
        if (halBlockFlagIsNot(kd, I2C_FLAG_TBE) != 0) {
            goto l_exit;
        }
        for (register uint32_t i = 0; i < regLen; i++) {
            i2c_data_transmit(kd->_instance.i2c, regs[i]);
            if (halBlockFlagIsNot(kd, I2C_FLAG_BTC) != 0) {
                goto l_exit;
            }
        }
        
        if (kd->_config.dma.channelTx != 0xFF) {
            i2c_dma_enable(kd->_instance.i2c, I2C_DMA_ON);
            dma_interrupt_flag_clear(
                kd->_config.dma.channelTx, 
                DMA_INT_FLAG_G | DMA_INT_FLAG_FTF | DMA_INT_FLAG_ERR
            );
            DMA_CHCTL(kd->_config.dma.channelTx) &= ~DMA_CHXCTL_CHEN;
            DMA_CHCNT(kd->_config.dma.channelTx) = (len);
            DMA_CHMADDR(kd->_config.dma.channelTx) = (uint32_t) data;
            DMA_CHCTL(kd->_config.dma.channelTx) |= DMA_CHXCTL_CHEN;
            if (halBlockDmaTxc(kd) != 0) {
                dma_interrupt_flag_clear(
                    kd->_config.dma.channelTx, 
                    DMA_INT_FLAG_G | DMA_INT_FLAG_FTF | DMA_INT_FLAG_ERR
                );
                DMA_CHCTL(kd->_config.dma.channelTx) &= ~DMA_CHXCTL_CHEN;
                goto l_exit;
            }
            if (halBlockFlagIsNot(kd, I2C_FLAG_BTC) != 0) {
                goto l_exit;
            }
        } else {
            while (len--) {
                i2c_data_transmit(kd->_instance.i2c, *data);
                data++;
                if (halBlockFlagIsNot(kd, I2C_FLAG_BTC) != 0) {
                    goto l_exit;
                }
            }
        }

        i2c_stop_on_bus(kd->_instance.i2c);
        if (halBlockStop(kd) != 0) {
            goto l_exit;
        }
        
        rc = 0;
    } else {
        if (bbStart(kd) != 0) {
            rc = KDI2CBB_ERR_START;
            goto l_exit;
        }
        
        bbSendByte(kd, addr & 0xFE);
        if (bbWack(kd) != 0) {
            rc = KDI2CBB_ERR_WACK;
            goto l_exit;
        }
        
        for (rc = 0; rc < regLen; rc++) {
            bbSendByte(kd, regs[rc]);
            if (bbWack(kd) != 0) {
                rc = KDI2CBB_ERR_WACK;
                goto l_exit;
            }
        }
        
        for (rc = 0; rc < len; rc++) {
            bbSendByte(kd, data[rc]);
            if (bbWack(kd) != 0) {
                rc = KDI2CBB_ERR_WACK;
                goto l_exit;
            }
        }
        
        bbStop(kd);
        
        rc = 0;
    }

l_exit:
    if (kd->_instance.i2c != NULL) {
        i2c_dma_enable(kd->_instance.i2c, I2C_DMA_OFF);
        i2c_ack_config(kd->_instance.i2c, I2C_ACK_ENABLE);
        i2c_ackpos_config(kd->_instance.i2c, I2C_ACKPOS_CURRENT);
    }
    
    if (rc != 0) {
        kdi2c_reset(kd);
        mutexUnlock(kd);
        return -1;
    }

    mutexUnlock(kd);
    return 0;
}
        

int32_t kdi2c_regRead(kdi2c_t *kd, uint8_t addr,
        uint32_t reg, uint8_t regLen,
        uint8_t *data, uint16_t len) {
    uint8_t regs[4] = {0};
    switch (regLen) {
        case 1:
            regs[0] = reg;
            break;
        case 2:
            regs[0] = reg >> 8 & 0x00FF;
            regs[1] = reg & 0x00FF;
            break;
        case 3:
            regs[0] = reg >> 16 & 0x0000FF;
            regs[1] = reg >> 8 & 0x0000FF;
            regs[2] = reg & 0x0000FF;
            break;
        case 4:
            regs[0] = reg >> 24 & 0x000000FF;
            regs[1] = reg >> 16 & 0x000000FF;
            regs[2] = reg >> 8 & 0x000000FF;
            regs[3] = reg & 0x000000FF;
            break;
    }
    
    return kdi2c_transmit(kd, addr, regs, regLen, data, len);
}
        

int32_t kdi2c_start(kdi2c_t *kd, uint8_t addr8RW) {
    int32_t rc = -1;

    mutexLock(kd);
    
    if (kd->_instance.i2c != NULL) {
        if (halBlockFlagIs(kd, I2C_FLAG_I2CBSY) != 0) {
            goto l_exit;
        }
        
        i2c_start_on_bus(kd->_instance.i2c);
        if (halBlockFlagIsNot(kd, I2C_FLAG_SBSEND) != 0) {
            goto l_exit;
        }
       
        i2c_master_addressing(kd->_instance.i2c, addr8RW, I2C_TRANSMITTER);
        if (halBlockFlagIsNot(kd, I2C_FLAG_ADDSEND) != 0) {
            goto l_exit;
        }
        i2c_flag_clear(kd->_instance.i2c, I2C_FLAG_ADDSEND);
    } else {
        if (bbStart(kd) != 0) {
            rc = KDI2CBB_ERR_START;
            goto l_exit;
        }
        
        bbSendByte(kd, addr8RW);
        if (bbWack(kd) != 0) {
            rc = KDI2CBB_ERR_WACK;
            goto l_exit;
        }
        
        rc = 0;
    }

l_exit:
    if (kd->_instance.i2c != NULL) {
        i2c_ack_config(kd->_instance.i2c, I2C_ACK_ENABLE);
        i2c_ackpos_config(kd->_instance.i2c, I2C_ACKPOS_CURRENT);
    }
    if (rc != 0) {
        kdi2c_reset(kd);
        mutexUnlock(kd);
        return -1;
    }

    mutexUnlock(kd);
    return 0;
}


int32_t kdi2c_sendAndWaitAck(kdi2c_t *kd, uint8_t *wbuf, uint16_t wsize) {
    int32_t rc = -1;

    mutexLock(kd);
    
    if (kd->_instance.i2c != NULL) {
        if (halBlockFlagIsNot(kd, I2C_FLAG_TBE) != 0) {
            goto l_exit;
        }
        
        for (register uint32_t i = 0; i < wsize; i++) {
            i2c_data_transmit(kd->_instance.i2c, wbuf[i]);

            if (halBlockFlagIsNot(kd, I2C_FLAG_BTC) != 0) {
                goto l_exit;
            }
        }
    } else {
        for (uint16_t i = 0; i < wsize; i++) {
            bbSendByte(kd, wbuf[i]);
            if (bbWack(kd) != 0) {
                rc = KDI2CBB_ERR_WACK;
                goto l_exit;
            }
        }

        rc = 0;
    }
    
l_exit:
    if (kd->_instance.i2c != NULL) {
        i2c_ack_config(kd->_instance.i2c, I2C_ACK_ENABLE);
        i2c_ackpos_config(kd->_instance.i2c, I2C_ACKPOS_CURRENT);
    }
    if (rc != 0) {
        kdi2c_reset(kd);
        mutexUnlock(kd);
        return -1;
    }

    mutexUnlock(kd);
    return 0;
}


int32_t kdi2c_stop(kdi2c_t *kd) {
    mutexLock(kd);
    
    if (kd->_instance.i2c != NULL) {
        i2c_stop_on_bus(kd->_instance.i2c);
        halBlockStop(kd);
    } else {
        bbStop(kd);
    }

    mutexUnlock(kd);

    return 0;
}

/*@}*/
