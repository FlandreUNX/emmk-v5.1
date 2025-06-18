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


static inline void _threadYield(void) {
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    osThreadYield();
#endif
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
    ASSERT(kd != NULL);
    
    if (kd->_instance.i2c != NULL) {
        
    } else {
        bbPinConfig(kd, 1);
    }
    
    return 0;
}


int32_t kdi2c_powerDown(kdi2c_t *kd) {
    ASSERT(kd != NULL);
    
    if (emmkDriver_initRefsCountDown(&kd->_va->initRefsPower) != 0) {
        return -1;
    }
    
    if (kd->_instance.i2c != NULL) {
        
    } else {
        bbPinConfig(kd, 0);
    }
    
    return 0;
}


void kdi2c_reset(kdi2c_t *kd) {
    if (kd->_instance.i2c != NULL) {
        
    } else {
        bbStop(kd);
        bbPinConfig(kd, 1);
    }
}


void kdi2c_setSpeed(kdi2c_t *kd, uint32_t speed) {
    mutexLock(kd);
    
    if (kd->_instance.i2c != NULL) {
        
    }

l_exit:
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
    int32_t rc = -1;

    mutexLock(kd);
    
    if (kd->_instance.i2c != NULL) {
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
    } else {
        bbStop(kd);
    }

    mutexUnlock(kd);

    return 0;
}

/*@}*/
