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

/**
 * @addtogroup Interface define
 * @note none
 */
 
/*@{*/

/// @Note1: BR=9600, 0.2 * (1 / 9600) = 20.8333333 uS -->>> timerPreTick
/// @Note2: timingHandler must call in interrupt every 0.2 * (1 / BR)

static const uint8_t CONFIG_KDSTTY_OVERSAMPLING = 5;
static const uint8_t CONFIG_KDSTTY_OVERSAMPLING_MIDDLE = CONFIG_KDSTTY_OVERSAMPLING / 2;

/*@}*/

/**
 * @addtogroup Private func
 * @note none
 */
 
/*@{*/

static void resetIns(kdstty_t *kd) {
    kd->_va->state.tx.isBusy = false;
    kd->_va->state.tx.isEnable = false;
    kd->_va->state.tx.bitShift = 0;
    kd->_va->state.tx.bitCount = 0;
    kd->_va->state.tx.parityCount = 0;
    kd->_va->state.tx.launchBit = 0;
    kd->_va->state.tx.bufferDataLen = 0;
    kd->_va->state.tx.bufferIndex = 0;
    
    kd->_va->state.rx.isBusy = false;
    kd->_va->state.rx.isWaitIdle = false;
    kd->_va->state.rx.isCompleted = false;
    kd->_va->state.rx.bitShift = 0;
    kd->_va->state.rx.bitCount = 0;
    kd->_va->state.rx.launchData = 0;
    kd->_va->state.rx.launchBitMiddleTiming = 0;
    kd->_va->state.rx.isLaunchBitMiddleTimingSet = 0;
}

static inline void pinSet(kdstty_Pin_t *pin, uint32_t set) {
    if (pin->reverse) {
        set = !set;
    }
    if (set) {
        pin->gpio->_config.base.port->BSRR = pin->gpio->_config.base.pin;
    } else {
        pin->gpio->_config.base.port->BRR = pin->gpio->_config.base.pin;
    }
}

static inline uint32_t pinRead(kdstty_Pin_t *pin) {
    register uint32_t pinSet = pin->gpio->_config.base.port->IDR &  pin->gpio->_config.base.pin;
    if (pinSet) {
        return pin->reverse ? 0 : pinSet;
    } else {
        return pin->reverse ? pinSet : 0;
    }
}

static void writeProcess(kdstty_t *kd) {
    if (kd->_config.pin.tx.gpio == NULL || !kd->_va->state.tx.isEnable) {
        return;
    }
    if (kd->_va->state.tx.bitCount == 0) {  // Start
        kd->_va->state.tx.isBusy = true;
        kd->_va->state.tx.bitShift = 0;
        kd->_va->state.tx.bitCount++;
        kd->_va->state.tx.parityCount = 0;
        pinSet(&kd->_config.pin.tx, 0);
    } else if (kd->_va->state.tx.bitCount < kd->_config.uart.dataLen + 1) {  // Data...
        kd->_va->state.tx.launchBit = (kd->_buffer.tx.pool[kd->_va->state.tx.bufferIndex] >> kd->_va->state.tx.bitShift) & 0x01;
        kd->_va->state.tx.bitCount++;
        kd->_va->state.tx.bitShift++;
        pinSet(&kd->_config.pin.tx, kd->_va->state.tx.launchBit);
        if (kd->_va->state.tx.launchBit) {
            kd->_va->state.tx.parityCount++;
        }
    } else if (kd->_va->state.tx.bitCount < kd->_config.uart.idefC1Len) {   // Parity
        kd->_va->state.tx.launchBit = kd->_va->state.tx.parityCount % 2;
        if (kd->_config.uart.parity == KDSTTY_PARITY_ODD) {
            kd->_va->state.tx.launchBit = !kd->_va->state.tx.launchBit;
        }
        pinSet(&kd->_config.pin.tx, kd->_va->state.tx.launchBit);
        kd->_va->state.tx.bitCount++;
    } else if (kd->_va->state.tx.bitCount < kd->_config.uart.idefC2Len) {   // Stop
        pinSet(&kd->_config.pin.tx, 1);
        kd->_va->state.tx.bitCount++;
    } else if (kd->_va->state.tx.bitCount == kd->_config.uart.idefC2Len) {   // Idle
        kd->_va->state.tx.bitCount = 0;
        kd->_va->state.tx.bufferIndex++;
        if (kd->_va->state.tx.bufferDataLen <= kd->_va->state.tx.bufferIndex) {
            kd->_va->state.tx.isBusy = false;
            kd->_va->state.tx.isEnable = false;
        }
    }
}

static void recvProces(kdstty_t *kd, uint32_t bit) {
    if (kd->_config.pin.rx.gpio == NULL) {
        return;
    }
    if (kd->_va->state.rx.bitCount == 0 || kd->_va->state.rx.isWaitIdle) {  // Start
        if (!bit) {
            kd->_va->state.rx.isBusy = true;
            kd->_va->state.rx.isWaitIdle = false;
            kd->_va->state.rx.isCompleted = false;
            kd->_va->state.rx.bitShift = 0;
            kd->_va->state.rx.bitCount = 1;
            kd->_va->state.rx.launchData = 0;
        } else {
            if (kd->_va->state.rx.isWaitIdle) {
                if (kd->_va->state.rx.bitShift < kd->_config.uart.rxIdleCount) {
                    kd->_va->state.rx.bitShift++;
                } else {
                    kd->_va->state.rx.isWaitIdle = false;
                    kd->_va->state.rx.isBusy = false;
                    kd->_va->state.rx.isCompleted = true;
                }
            }
        }
    } else if (kd->_va->state.rx.bitCount < (kd->_config.uart.dataLen + 1)) { // Data...
        kd->_va->state.rx.launchData |= (bit >> kd->_config.pin.rx.gpio->_config.base.pin) << kd->_va->state.rx.bitShift;
        kd->_va->state.rx.bitCount++;
        kd->_va->state.rx.bitShift++;
    } else if (kd->_va->state.rx.bitCount < kd->_config.uart.idefC1Len) {   // Parity
        // TODO: ...
        kd->_va->state.rx.bitCount++;
    } else if (kd->_va->state.rx.bitCount < kd->_config.uart.idefC2Len) {   // Stop
        kd->_va->state.rx.bitShift = 0;
        kd->_va->state.rx.bitCount = 0;
        kd->_va->state.rx.isLaunchBitMiddleTimingSet = 0;
        if (bit) {
            qBSBuffer_Put(kd->_buffer.rx.buffer, kd->_va->state.rx.launchData);
        } else {
            // TODO: ERROR
        }
        kd->_va->state.rx.isWaitIdle = true;
    }
}

void kdstty_pinRxIrqHandler(kdstty_t *kd) {
    if (kd->_va->state.rx.isLaunchBitMiddleTimingSet) {
        return;
    }
    register uint32_t bit = pinRead(&kd->_config.pin.rx);
    if (kd->_va->state.rx.bitCount == 0 && !bit) {
        kd->_va->state.rx.launchBitMiddleTiming = (kd->_host->_va->timingDiv + CONFIG_KDSTTY_OVERSAMPLING_MIDDLE) % CONFIG_KDSTTY_OVERSAMPLING;
        kd->_va->state.rx.isLaunchBitMiddleTimingSet = 1;
    }
}

void kdstty_timingHandler(kdstty_Host_t *host, void *devList) {
    register uint32_t rxPinBuffer = 0;
    register uint32_t bit = 0;

    for (kdstty_t **kd = (kdstty_t **) devList; *kd != NULL; kd++) {
        if ((*kd)->_va->initRefsPower == 0 || (*kd)->_config.pin.rx.gpio == NULL) {
            continue;
        }
        
        bit = pinRead(&(*kd)->_config.pin.rx);
        rxPinBuffer |= bit;
        
        if ((*kd)->_config.uart.rxPinIrqMode == 0) {
            if ((*kd)->_va->state.rx.bitCount == 0 && (*kd)->_va->state.rx.isLaunchBitMiddleTimingSet == 0 && !bit) {
                (*kd)->_va->state.rx.launchBitMiddleTiming = ((*kd)->_host->_va->timingDiv + CONFIG_KDSTTY_OVERSAMPLING_MIDDLE) % CONFIG_KDSTTY_OVERSAMPLING;
                (*kd)->_va->state.rx.isLaunchBitMiddleTimingSet = 1;
            }
        }
        
        if ((*kd)->_va->state.rx.isLaunchBitMiddleTimingSet 
                && (*kd)->_va->state.rx.launchBitMiddleTiming == host->_va->timingDiv) {
            recvProces((*kd), rxPinBuffer & (*kd)->_config.pin.rx.gpio->_config.base.pin);
        }
    }
    
    if (host->_va->timingDiv == 0) {
        for (kdstty_t **kd = (kdstty_t **) devList; *kd != NULL; kd++) {
            if ((*kd)->_va->initRefsPower == 0 || (*kd)->_config.pin.tx.gpio == NULL) {
                continue;
            }
            writeProcess((*kd));
        }
    }
    
    host->_va->timingDiv++;
    if (host->_va->timingDiv >= CONFIG_KDSTTY_OVERSAMPLING) {
        host->_va->timingDiv = 0;
    }
}

/*@}*/

/**
 * @addtogroup Public functions support
 * @note none
 */
 
/*@{*/

int32_t kdstty_init(kdstty_t *kd) {
    ASSERT(kd != NULL);
    
    if (emmkDriver_initRefsCountUp(&kd->_host->_va->initRefs) == 0) {
        kd->_host->_base.baseEnable(kd->_host);
        float arr = ((1000000 / CONFIG_KDSTTY_OVERSAMPLING) / ((float) kd->_host->_config.baudrate));
        
        // 24Mhz = 0.041us
        // MAX-Count = 2.686ms
        // 1byte(10bit)us
        // (1 / (bd / 10)) = Xms(1byte)
        arr = ((uint32_t) (arr / 0.041f));
        kd->_host->_config.tim.tim->PSC = 1;
        kd->_host->_config.tim.tim->ARR = arr;
        kd->_host->_config.tim.tim->CNT = 0;
    }
    
    if (emmkDriver_initRefsCountUp(&kd->_va->initRefs) != 0) {
        return -1;
    }

    if (kd->_buffer.rx.buffer != NULL) {
        qBSBuffer_Setup(kd->_buffer.rx.buffer, kd->_buffer.rx.pool, kd->_buffer.rx.poolSize);
    }
    memset(kd->_buffer.tx.pool, 0x00, kd->_buffer.tx.poolSize);
 
    resetIns(kd);

    return 0;
}


int32_t kdstty_finalize(kdstty_t *kd) {
    ASSERT(kd != NULL);
    
    if (emmkDriver_initRefsCountDown(&kd->_host->_va->initRefs) == 0) {
        kd->_host->_config.tim.tim->CR1 &= ~KLBIT(0);

        kd->_host->_base.baseDisable(kd->_host);
    }
    
    if (emmkDriver_initRefsCountDown(&kd->_va->initRefs) != 0) {
        return -1;
    }
    
    if (kd->_va->initRefsPower != 0) {
        kd->_va->initRefsPower = 1;
        kdstty_powerDown(kd);
    }

    return 0;
}


int32_t kdstty_powerUp(kdstty_t *kd) {
    ASSERT(kd != NULL);
    
    if (emmkDriver_initRefsCountUp(&kd->_host->_va->initRefsPower) == 0) {
        kd->_host->_va->timingDiv = 0;

        kd->_host->_config.tim.tim->CR1 &= ~KLBIT(0);
        kd->_host->_config.tim.tim->CNT = 0;
        kd->_host->_config.tim.tim->IER |= KLBIT(0);
        kd->_host->_config.tim.tim->ICR &= ~KLBIT(0);
        kd->_host->_config.tim.tim->CR1 |= KLBIT(0);
    }
    
    if (emmkDriver_initRefsCountUp(&kd->_va->initRefsPower) != 0) {
        return -1;
    }
    
    if (kd->_config.pin.tx.gpio != NULL) {
        _gpio_modeConfig(kd->_config.pin.tx.gpio->_config.base.port, 
            kd->_config.pin.tx.gpio->_config.base.pin, 
            KDGPIO_MODE_OUTPUT_PP);
        _gpio_pullConfig(kd->_config.pin.tx.gpio->_config.base.port, 
            kd->_config.pin.tx.gpio->_config.base.pin, 
            KDGPIO_PULL_NONE);
        
        pinSet(&kd->_config.pin.tx, 1);
    }
    if (kd->_config.pin.rx.gpio != NULL) {
        _gpio_modeConfig(kd->_config.pin.rx.gpio->_config.base.port, 
            kd->_config.pin.rx.gpio->_config.base.pin, 
            KDGPIO_MODE_INPUT);
        _gpio_pullConfig(kd->_config.pin.rx.gpio->_config.base.port, 
            kd->_config.pin.rx.gpio->_config.base.pin, 
            KDGPIO_PULL_NONE);
        
        if (kd->_config.uart.rxPinIrqMode) {
            kdgpio_irqEnable(kd->_config.pin.rx.gpio, KDGPIO_TRIGGER_RISING_FALLING, NULL);
        }
    }

    kdstty_flush(kd);

    return 0;
}


int32_t kdstty_powerDown(kdstty_t *kd) {
    ASSERT(kd != NULL);
    
    if (emmkDriver_initRefsCountDown(&kd->_host->_va->initRefsPower) == 0) {
        kd->_host->_va->timingDiv = 0;

        kd->_host->_config.tim.tim->CR1 &= ~KLBIT(0);
        kd->_host->_config.tim.tim->IER &= ~KLBIT(0);
        kd->_host->_config.tim.tim->ICR &= ~KLBIT(0);
    }
    
    if (emmkDriver_initRefsCountDown(&kd->_va->initRefsPower) != 0) {
        return -1;
    }
    
    if (kd->_config.pin.tx.gpio != NULL) {
        kdgpio_powerDown(kd->_config.pin.tx.gpio);
    }
    if (kd->_config.pin.rx.gpio != NULL) {
        if (kd->_config.uart.rxPinIrqMode) {
            kdgpio_irqEnable(kd->_config.pin.rx.gpio, KDGPIO_TRIGGER_NONE, NULL);
        }
        
        kdgpio_powerDown(kd->_config.pin.rx.gpio);
    }
    
    resetIns(kd);

    return 0;
}


int32_t kdstty_sends(kdstty_t *kd, const void *data, uint32_t size, uint32_t timeout) {
    if (size >= kd->_buffer.tx.poolSize) {
        return -1;
    }
    if (kd->_va->state.tx.isBusy) {
        return -2;
    }
    kd->_va->state.tx.isBusy = true;
    
    kd->_va->state.tx.bufferIndex = 0;
    kd->_va->state.tx.bufferDataLen = size;
    memcpy(kd->_buffer.tx.pool, data, size);
    
    kd->_va->state.tx.isEnable = true;
    
    return size;
}


int32_t kdstty_recvs(kdstty_t *kd, void *data, uint32_t expect_size, uint32_t *recv_size, uint32_t timeout) {
    if (kd->_va->state.rx.isBusy || !kd->_va->state.rx.isCompleted) {
        if (recv_size != NULL) {
            *recv_size = 0;
        }
        return -1;
    }
            
    uint32_t buffCount = qBSBuffer_Count(kd->_buffer.rx.buffer);
    if (buffCount == 0) {
        kd->_va->state.rx.isCompleted = 0;
        
        if (recv_size != NULL) {
            *recv_size = 0;
        }
        return 0;
    }
    buffCount = buffCount <= expect_size ? buffCount : expect_size;
    if (recv_size != NULL) {
        if (qBSBuffer_Read(kd->_buffer.rx.buffer, data, buffCount) != qTrue) {
            *recv_size = 0;
        } else {
            *recv_size = buffCount;
        }
    } else {
        qBSBuffer_Read(kd->_buffer.rx.buffer, data, buffCount);
    }
    
    if (qBSBuffer_Count(kd->_buffer.rx.buffer) == 0) {
        kd->_va->state.rx.isCompleted = 0;
    }

    return 0;
}


int32_t kdstty_flush(kdstty_t *kd) {
    kd->_va->state.tx.isEnable = 0;
    kd->_va->state.tx.isBusy = 0;
    memset(kd->_buffer.tx.pool, 0x00, kd->_buffer.tx.poolSize);

    kd->_va->state.rx.isCompleted = 0;
    kd->_va->state.rx.isBusy = 0;
    kd->_va->state.rx.isWaitIdle = 0;
    if (kd->_buffer.rx.buffer != NULL) {
        qBSBuffer_Setup(kd->_buffer.rx.buffer, kd->_buffer.rx.pool, kd->_buffer.rx.poolSize);
    }

    return 0;
}


int32_t kdstty_hasRecvData(kdstty_t *kd) {
    if (kd->_va->state.rx.isCompleted) {
        return qBSBuffer_Count(kd->_buffer.rx.buffer);
    }
    return 0;
}


int32_t kdstty_isSendIdle(kdstty_t *kd) {
    return kd->_va->state.tx.isBusy ? false : true;
}


void kdstty_updateBaudRate(kdstty_t *kd, uint32_t bd) {
}

/*@}*/
