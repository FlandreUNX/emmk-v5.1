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

__STATIC_FORCEINLINE void mutexNew(kdi2c_t *kd) {
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    const osMutexAttr_t mutex_attr = {
        .attr_bits = osMutexRobust | osMutexPrioInherit
    };
    
    kd->_va->mutex = osMutexNew(&mutex_attr);
    ASSERT(kd->_va->mutex != NULL);
#endif
}


__STATIC_FORCEINLINE void mutexDelete(kdi2c_t *kd) {
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    osMutexDelete(kd->_va->mutex);
    kd->_va->mutex = NULL;
#endif
}


__STATIC_FORCEINLINE void mutexLock(kdi2c_t *kd) {
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    osMutexAcquire(kd->_va->mutex, osWaitForever);
#endif
}


__STATIC_FORCEINLINE void mutexUnlock(kdi2c_t *kd) {
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    osMutexRelease(kd->_va->mutex);
#endif
}


__STATIC_FORCEINLINE void threadYield(void) {
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    osThreadYield();
#else
    // __WFI();
#endif
}

/*@}*/

/**
 * @addtogroup Soc Support
 * @note none
 */
 
/*@{*/

static int32_t halBlockFlagIs(kdi2c_t *kd, uint32_t flagWait) {
    qSTimer_t tmcd = QSTIMER_INITIALIZER;
    
    qSTimer_Set(&tmcd, 100);
    while ((kd->_instance.i2c->MSTCMD & flagWait)
            && (!qSTimer_Expired(&tmcd))) {
        threadYield();
    }
            
    if (qSTimer_Expired(&tmcd)) {
        return -1;
    } else {
        return 0;
    }
}


static int32_t halBlockFlagIsNot(kdi2c_t *kd, uint32_t flagWait) {
    qSTimer_t tmcd = QSTIMER_INITIALIZER;
    
    qSTimer_Set(&tmcd, 100);
    while (!(kd->_instance.i2c->MSTCMD & flagWait)
            && (!qSTimer_Expired(&tmcd))) {
        threadYield();
    }
            
    if (qSTimer_Expired(&tmcd)) {
        return -1;
    } else {
        return 0;
    }
}


static int32_t halBlockStop(kdi2c_t *kd) {
    qSTimer_t tmcd = QSTIMER_INITIALIZER;
    
    qSTimer_Set(&tmcd, 100);
    
    kd->_instance.i2c->MSTCMD = (1 << I2C_MSTCMD_STO_Pos);
    while ((kd->_instance.i2c->MSTCMD & I2C_MSTCMD_TIP_Msk)
            && (!qSTimer_Expired(&tmcd))) {
        threadYield();
    }
    
    if (qSTimer_Expired(&tmcd)) {
        return -1;
    } else {
        return 0;
    }
}


static inline uint32_t _halGetAckFlag(kdi2c_t *kd, uint32_t flag) {
    return (kd->_instance.i2c->MSTCMD & flag) ? 0 : 1;
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
        I2C_Init(kd->_instance.i2c, &kd->_instance.init);
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

    kd->_instance.disableFunc(kd);
    
    mutexDelete(kd);
    
    return 0;
}


int32_t kdi2c_powerUp(kdi2c_t *kd) {
    if (emmkDriver_initRefsCountUp(&kd->_va->initRefsPower) != 0) {
        return -1;
    }
    
    if (kd->_instance.i2c != NULL) {
        PORT_Init(kd->_config.pin.sda.gpio->_config.base.port, 
            kd->_config.pin.sda.gpio->_config.base.number, 
            kd->_config.pin.sda.af, 1);
        _gpio_modeConfig(kd->_config.pin.sda.gpio->_config.base.port,
            kd->_config.pin.sda.gpio->_config.base.gpio,
            kd->_config.pin.sda.gpio->_config.base.pin,
            KDGPIO_MODE_OUTPUT_OD);
        _gpio_pullConfig(kd->_config.pin.sda.gpio->_config.base.port,
            kd->_config.pin.sda.gpio->_config.base.pin,
            KDGPIO_PULL_NONE);
        
        PORT_Init(kd->_config.pin.scl.gpio->_config.base.port, 
            kd->_config.pin.scl.gpio->_config.base.number, 
            kd->_config.pin.scl.af, 1);
        _gpio_modeConfig(kd->_config.pin.scl.gpio->_config.base.port,
            kd->_config.pin.scl.gpio->_config.base.gpio,
            kd->_config.pin.scl.gpio->_config.base.pin,
            KDGPIO_MODE_OUTPUT_OD);
        _gpio_pullConfig(kd->_config.pin.scl.gpio->_config.base.port,
            kd->_config.pin.scl.gpio->_config.base.pin,
            KDGPIO_PULL_NONE);
        
        I2C_Open(kd->_instance.i2c);
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
        bbPinConfig(kd, 0);
        
        I2C_Close(kd->_instance.i2c);
    } else {
        bbPinConfig(kd, 0);
    }
    
    return 0;
}


void kdi2c_reset(kdi2c_t *kd) {
    if (kd->_instance.i2c != NULL) {
        I2C_Close(kd->_instance.i2c);
        I2C_Open(kd->_instance.i2c);
    } else {
        bbStop(kd);
        bbPinConfig(kd, 1);
    }
}


void kdi2c_setSpeed(kdi2c_t *kd, uint32_t speed) {
    mutexLock(kd);
    
    if (kd->_instance.i2c != NULL) {
        uint8_t isEnable = kd->_instance.i2c->CTRL & (1u << 7u) ? 1 : 0;
        I2C_Close(kd->_instance.i2c);
        kd->_instance.i2c->CLKDIV = SystemCoreClock / 1000 / 5 * (speed / 100) - 1;
        if (isEnable) {
            I2C_Open(kd->_instance.i2c);
        }
    }
    
    mutexUnlock(kd);
}


int32_t kdi2c_transmit(kdi2c_t *kd, uint8_t addr,
        uint8_t *wbuf, uint16_t wsize,
        uint8_t *rbuf, uint16_t rsize) {     
    int32_t rc = -1;
            
    mutexLock(kd);
    
    addr <<= 1;
    
    if (kd->_instance.i2c != NULL) {
        uint8_t ack;
        
        kd->_instance.i2c->MSTDAT = addr & 0xFE;
        kd->_instance.i2c->MSTCMD = (1 << I2C_MSTCMD_STA_Pos) | (1 << I2C_MSTCMD_WR_Pos);
        if (halBlockFlagIs(kd, I2C_MSTCMD_TIP_Msk) != 0) {
            goto l_exit;
        }
        ack = _halGetAckFlag(kd, I2C_MSTCMD_RXACK_Msk);
        if (ack == 0) {
            goto l_exit;
        }
        
        for (uint16_t i = 0; i < wsize; i++) {
            kd->_instance.i2c->MSTDAT = wbuf[i] & 0xFF;
            kd->_instance.i2c->MSTCMD = (1 << I2C_MSTCMD_WR_Pos);
            
            if (halBlockFlagIs(kd, I2C_MSTCMD_TIP_Msk) != 0) {
                goto l_exit;
            }
            ack = _halGetAckFlag(kd, I2C_MSTCMD_RXACK_Msk);
            if (ack == 0) {
                goto l_exit;
            }
        }
        
        kd->_instance.i2c->MSTDAT = addr | 0x01;
        kd->_instance.i2c->MSTCMD = (1 << I2C_MSTCMD_STA_Pos) | (1 << I2C_MSTCMD_WR_Pos);
        if (halBlockFlagIs(kd, I2C_MSTCMD_TIP_Msk) != 0) {
            goto l_exit;
        }
        ack = _halGetAckFlag(kd, I2C_MSTCMD_RXACK_Msk);
        if (ack == 0) {
            goto l_exit;
        }
        
        while (rsize > 1) {
            kd->_instance.i2c->MSTCMD = (1 << I2C_MSTCMD_RD_Pos) | ((0) << I2C_MSTCMD_ACK_Pos);

            if (halBlockFlagIs(kd, I2C_MSTCMD_TIP_Msk) != 0) {
                goto l_exit;
            }
            
            *rbuf++ = kd->_instance.i2c->MSTDAT & 0xFF;
            rsize--;
        }
        
        while (rsize) {
            kd->_instance.i2c->MSTCMD = (1 << I2C_MSTCMD_RD_Pos) | ((1) << I2C_MSTCMD_ACK_Pos);
            
            if (halBlockFlagIs(kd, I2C_MSTCMD_TIP_Msk) != 0) {
                goto l_exit;
            }
            
            *rbuf++ = kd->_instance.i2c->MSTDAT & 0xFF;
            rsize--;
        }
        
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
    if (rc < 0) {
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
        uint8_t ack;
        
        kd->_instance.i2c->MSTDAT = addr & 0xFE;
        kd->_instance.i2c->MSTCMD = (1 << I2C_MSTCMD_STA_Pos) | (1 << I2C_MSTCMD_WR_Pos);
        if (halBlockFlagIs(kd, I2C_MSTCMD_TIP_Msk) != 0) {
            goto l_exit;
        }
        ack = _halGetAckFlag(kd, I2C_MSTCMD_RXACK_Msk);
        if (ack == 0) {
            goto l_exit;
        }
        
        for (uint16_t i = 0; i < wsize; i++) {
            kd->_instance.i2c->MSTDAT = wbuf[i] & 0xFF;
            kd->_instance.i2c->MSTCMD = (1 << I2C_MSTCMD_WR_Pos);
            
            if (halBlockFlagIs(kd, I2C_MSTCMD_TIP_Msk) != 0) {
                goto l_exit;
            }
            ack = _halGetAckFlag(kd, I2C_MSTCMD_RXACK_Msk);
            if (ack == 0) {
                goto l_exit;
            }
        }
        
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
    if (rc < 0) {
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
        uint8_t ack;
        
        kd->_instance.i2c->MSTDAT = addr | 0x01;
        kd->_instance.i2c->MSTCMD = (1 << I2C_MSTCMD_STA_Pos) | (1 << I2C_MSTCMD_WR_Pos);
        if (halBlockFlagIs(kd, I2C_MSTCMD_TIP_Msk) != 0) {
            goto l_exit;
        }
        ack = _halGetAckFlag(kd, I2C_MSTCMD_RXACK_Msk);
        if (ack == 0) {
            goto l_exit;
        }
        
        while (rsize > 1) {
            kd->_instance.i2c->MSTCMD = (1 << I2C_MSTCMD_RD_Pos) | ((0) << I2C_MSTCMD_ACK_Pos);

            if (halBlockFlagIs(kd, I2C_MSTCMD_TIP_Msk) != 0) {
                goto l_exit;
            }
                
            *rbuf++ = kd->_instance.i2c->MSTDAT & 0xFF;
            rsize--;
        }
        
        while (rsize) {
            kd->_instance.i2c->MSTCMD = (1 << I2C_MSTCMD_RD_Pos) | ((1) << I2C_MSTCMD_ACK_Pos);
            
            if (halBlockFlagIs(kd, I2C_MSTCMD_TIP_Msk) != 0) {
                goto l_exit;
            }
            
            *rbuf++ = kd->_instance.i2c->MSTDAT & 0xFF;
            rsize--;
        }
        
        if (halBlockStop(kd) != 0) {
            goto l_exit;
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
    if (rc < 0) {
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
        uint8_t ack;
        
        kd->_instance.i2c->MSTDAT = addr & 0xFE;
        kd->_instance.i2c->MSTCMD = (1 << I2C_MSTCMD_STA_Pos) | (1 << I2C_MSTCMD_WR_Pos);
        if (halBlockFlagIs(kd, I2C_MSTCMD_TIP_Msk) != 0) {
            goto l_exit;
        }
        ack = _halGetAckFlag(kd, I2C_MSTCMD_RXACK_Msk);
        if (ack == 0) {
            goto l_exit;
        }
        
        for (uint16_t i = 0; i < regLen; i++) {
            kd->_instance.i2c->MSTDAT = regs[i] & 0xFF;
            kd->_instance.i2c->MSTCMD = (1 << I2C_MSTCMD_WR_Pos);
            
            if (halBlockFlagIs(kd, I2C_MSTCMD_TIP_Msk) != 0) {
                goto l_exit;
            }
            ack = _halGetAckFlag(kd, I2C_MSTCMD_RXACK_Msk);
            if (ack == 0) {
                goto l_exit;
            }
        }
        
        for (uint16_t i = 0; i < len; i++) {
            kd->_instance.i2c->MSTDAT = data[i] & 0xFF;
            kd->_instance.i2c->MSTCMD = (1 << I2C_MSTCMD_WR_Pos);
            
            if (halBlockFlagIs(kd, I2C_MSTCMD_TIP_Msk) != 0) {
                goto l_exit;
            }
            ack = _halGetAckFlag(kd, I2C_MSTCMD_RXACK_Msk);
            if (ack == 0) {
                goto l_exit;
            }
        }

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
    if (rc < 0) {
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
    register int32_t rc = -1;

    mutexLock(kd);
    
    if (kd->_instance.i2c != NULL) {
        uint8_t ack;
        
        kd->_instance.i2c->MSTDAT = addr8RW;
        kd->_instance.i2c->MSTCMD = (1 << I2C_MSTCMD_STA_Pos) | (1 << I2C_MSTCMD_WR_Pos);
        if (halBlockFlagIs(kd, I2C_MSTCMD_TIP_Msk) != 0) {
            goto l_exit;
        }
        ack = _halGetAckFlag(kd, I2C_MSTCMD_RXACK_Msk);
        if (ack == 0) {
            goto l_exit;
        }
        
        rc = 0;
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
    if (rc != 0) {
        kdi2c_reset(kd);
        mutexUnlock(kd);
        return -1;
    }

    mutexUnlock(kd);
    return 0;
}


int32_t kdi2c_sendAndWaitAck(kdi2c_t *kd, uint8_t *wbuf, uint16_t wsize) {
    register int32_t rc = -1;

    mutexLock(kd);
    
    if (kd->_instance.i2c != NULL) {
        for (register uint32_t i = 0; i < wsize; i++) {
            kd->_instance.i2c->MSTDAT = wbuf[i] & 0xFF;
            kd->_instance.i2c->MSTCMD = (1 << I2C_MSTCMD_WR_Pos);
            
            if (halBlockFlagIs(kd, I2C_MSTCMD_TIP_Msk) != 0) {
                goto l_exit;
            }
            uint8_t ack = _halGetAckFlag(kd, I2C_MSTCMD_RXACK_Msk);
            if (ack == 0) {
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
        halBlockStop(kd);
    } else {
        bbStop(kd);
    }

    mutexUnlock(kd);

    return 0;
}

/*@}*/
