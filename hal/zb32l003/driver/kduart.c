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
 * @addtogroup Hal support
 * @note none
 */
 
/*@{*/

static inline void _threadYield(void) {
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    osThreadYield();
#endif
}

static void waitSendCompleted(kduart_t *kd) {
    qSTimer_t wait;
    qSTimer_Set(&wait, 100);
    
    while (!kd->_va->flag.isSendCompleted 
            && !qSTimer_Expired(&wait)) {
        _threadYield();
    }
    kd->_va->flag.isSendCompleted = 0;
}

static void updateBaudRate(kduart_t *kd, uint32_t bd) {
    if (!kd->_config.uart.isLp) {
        kd->_config.uart.uart->INTCLR |= 0x07u;
        kd->_config.uart.uart->BAUDCR = ((2u * SystemCoreClock) / bd / 32u - 1u) 
            | (0x01u << 16u);
    } else {
        uint8_t isEnable = kd->_config.uart.lpuart->SCON & (0x01u << 16u) ? 1 : 0;
        
        kd->_config.uart.lpuart->SCON &= ~(0x01u << 16u);
        
        kd->_config.uart.lpuart->INTCLR |= 0x07u;
        kd->_config.uart.lpuart->BAUDCR = ((2u * SystemCoreClock) / bd / 32u - 1u) 
            | (0x01u << 16u);
        
        if (isEnable) {
            kd->_config.uart.lpuart->SCON |= (0x01u << 16u);
        }
    }
    
    if (kd->_config.tim.isLp == 0) {
        kd->_config.tim.tim->CR |= 
            (0x01u << 6u)
            | (0x01u << 5u)
            | (0x01u << 4)
            | (0x07u << 0);
        kd->_config.tim.tim->INTCLR = 0x01;
        kd->_config.tim.tim->BGLOAD = 0xFFFFFFFF - ((((((uint64_t) 11000000u) * kd->_config.tim.timeout / bd) / 10u) - 1u));
        kd->_config.tim.tim->LOAD = kd->_config.tim.tim->BGLOAD;
    } else {
        uint32_t to = ((((((uint64_t) 11000000u) * kd->_config.tim.timeout / bd) / 10u) - 1u));
        if (to > 0xFFFF) {
            to = 0xFFFF;
        }
        to = 0xFFFF - to;
        kd->_config.tim.lptim->CR = 0;
        kd->_config.tim.lptim->CR =
            (0x01u << 9u)
            | (0x10 << 4)
            | (0x01u << 1u)
            ;
        // while (kd->_config.tim.lptim->CR & (0x01u << 16u));
        kd->_config.tim.lptim->INTCLR = 0x01;
        kd->_config.tim.lptim->LOAD = kd->_config.tim.lptim->BGLOAD = to;
    }
}

/*@}*/

/**
 * @addtogroup Public functions support
 * @note none
 */
 
/*@{*/

int32_t kduart_init(kduart_t *kd) {
    ASSERT(kd != NULL);
    
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    const osMutexAttr_t mutex_attr = {
        .attr_bits = osMutexRobust
    };
#endif
    
    if (emmkDriver_initRefsCountUp(&kd->_va->initRefs) != 0) {
        return -1;
    }
    
    kd->_instance.enableFunc(kd);
    
    if (kd->_config.tim.isLp == 0) {
        // 24Mhz = 0.041us
        uint32_t arr = (10000.0f / (float) (kd->_config.uart.init.baudRate)) * 1000.0f * kd->_config.tim.timeout;
        arr = UINT32_MAX - ((uint32_t) (arr / 0.041f));
        
        kd->_config.tim.tim->CR |= 
            (0x01u << 6u)
            | (0x01u << 5u)
            | (0x01u << 4);
            // | (0x07u << 0);
        kd->_config.tim.tim->INTCLR = 0x01;
        kd->_config.tim.tim->BGLOAD = arr;
        kd->_config.tim.tim->LOAD = kd->_config.tim.tim->BGLOAD;
    } else {
        // 24Mhz = 0.041us
        // MAX-Count = 2.686ms
        // 1byte(10bit)us
        // (1 / (bd / 10)) = Xms(1byte)
        uint32_t arr = (10000.0f / (float) (kd->_config.uart.init.baudRate)) * 1000.0f * kd->_config.tim.timeout;
        arr = 0xFFFF - ((uint32_t) (arr / 0.041f));
        kd->_config.tim.lptim->CR = 0;
        kd->_config.tim.lptim->CR =
            (0x01u << 9u)
            | (0x10 << 4)
            | KLBIT(8)
            | (0x01u << 1u)
            ;
        // while (kd->_config.tim.lptim->CR & (0x01u << 16u));
        kd->_config.tim.lptim->INTCLR = 0x01;
        kd->_config.tim.lptim->LOAD = kd->_config.tim.lptim->BGLOAD = arr;
    }
    
    if (!kd->_config.uart.isLp) {
        kd->_config.uart.uart->INTCLR |= 0x07u;
        kd->_config.uart.uart->BAUDCR = ((2u * SystemCoreClock) / kd->_config.uart.init.baudRate / 32u - 1u) 
            | (0x01u << 16u);
        kd->_config.uart.uart->SCON = (0x01u << 9u) 
            | (0x01u << 8u) 
            | (0x01u << 6u)
            | (0x01u << 4u)
            | (0x01u << 1u)
            | (0x01u << 0);
    } else {
        kd->_config.uart.lpuart->INTCLR |= 0x07u;
        kd->_config.uart.lpuart->BAUDCR = ((2u * SystemCoreClock) / kd->_config.uart.init.baudRate / 32u - 1u) 
            | (0x01u << 16u);
        kd->_config.uart.lpuart->SCON = (0x01u << 9u) 
            | (0x01u << 8u) 
            | (0x01u << 6u)
            | (0x01u << 4u)
            | (0x01u << 1u)
            | (0x01u << 0);
    }

#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    kd->_va->flag = osEventFlagsNew(NULL);
    ASSERT(kd->_va->flag != NULL);
    
    kd->_va->writeMutex = osMutexNew(&mutex_attr);
    ASSERT(kd->_va->writeMutex != NULL);
    
    kd->_va->recvMutex = osMutexNew(&mutex_attr);
    ASSERT(kd->_va->recvMutex != NULL);
#else
    kd->_va->flag._ = 0;
    /// @Note: CX32
    kd->_va->flag.isSendCompleted = 0;
#endif
    
    if (kd->buffer.recvLwrb != NULL) {
        qBSBuffer_Setup(kd->buffer.recvLwrb, kd->buffer.recvBuffer, kd->buffer.recvBufferSize);
    }

    kduart_flush(kd);
    
    return 0;
}


int32_t kduart_finalize(kduart_t *kd) {
    ASSERT(kd != NULL);
    
    if (emmkDriver_initRefsCountDown(&kd->_va->initRefs) != 0) {
        return -1;
    }
    
   if (!kd->_config.uart.isLp) {
        kd->_config.uart.uart->INTCLR |= 0x07u;
        kd->_config.uart.uart->SCON = 0;
    } else {
        kd->_config.uart.lpuart->INTCLR |= 0x07u;
        kd->_config.uart.lpuart->SCON = 0;
    }
    
    if (kd->_config.tim.isLp == 0) {
        kd->_config.tim.tim->CR &= ~(0x01u << 7u);
    } else {
        kd->_config.tim.lptim->CR &= ~(0x01u << 0u);
    }
    
    kd->_instance.disableFunc(kd);

#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    osEventFlagsDelete(kd->_va->flag);
    kd->_va->flag = NULL;
    
    osMutexDelete(kd->_va->recvMutex);
    kd->_va->recvMutex = NULL;
    
    osMutexDelete(kd->_va->writeMutex);
    kd->_va->writeMutex = NULL;
#endif
    
    kd->_va->flag._ = 0;
    
    return 0;
}


int32_t kduart_powerUp(kduart_t *kd) {
    ASSERT(kd != NULL);
    
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    if (osMutexAcquire(kd->_va->writeMutex, osWaitForever) != osOK) {
        goto l_exit;
    }
    if (osMutexAcquire(kd->_va->recvMutex, osWaitForever) != osOK) {
        goto l_exit;
    }
#endif
    
    if (kd->_config.pin.tx.gpio != NULL) {
        _gpio_modeConfig(kd->_config.pin.tx.gpio->_config.base.port, 
            kd->_config.pin.tx.gpio->_config.base.number, 
            kd->_config.pin.tx.gpio->_config.base.pin, 
            KDGPIO_MODE_AF_PP);
        _gpio_pullConfig(kd->_config.pin.tx.gpio->_config.base.port, 
            kd->_config.pin.tx.gpio->_config.base.number, 
            kd->_config.pin.tx.gpio->_config.base.pin, 
            KDGPIO_PULL_NONE);
        _gpio_afConfig(kd->_config.pin.tx.gpio->_config.base.port, 
            kd->_config.pin.tx.gpio->_config.base.number, 
            kd->_config.pin.tx.af);
    }
    
    if (kd->_config.pin.rx.gpio != NULL) {
        _gpio_modeConfig(kd->_config.pin.rx.gpio->_config.base.port, 
            kd->_config.pin.rx.gpio->_config.base.number, 
            kd->_config.pin.rx.gpio->_config.base.pin, 
            KDGPIO_MODE_INPUT);
        _gpio_pullConfig(kd->_config.pin.rx.gpio->_config.base.port, 
            kd->_config.pin.rx.gpio->_config.base.number, 
            kd->_config.pin.rx.gpio->_config.base.pin, 
            KDGPIO_PULL_NONE);
        _gpio_afConfig(kd->_config.pin.rx.gpio->_config.base.port, 
            kd->_config.pin.rx.gpio->_config.base.number, 
            kd->_config.pin.rx.af);
    }

    if (kd->buffer.recvLwrb != NULL) {
        qBSBuffer_Setup(kd->buffer.recvLwrb, kd->buffer.recvBuffer, kd->buffer.recvBufferSize);
    }

    if (!kd->_config.uart.isLp) {
    } else {
        kd->_config.uart.lpuart->SCON |= (0x01u << 16u);
    }
    
l_exit:
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    osMutexRelease(kd->_va->writeMutex);
    osMutexRelease(kd->_va->recvMutex);
#endif

    return 0;
}


int32_t kduart_powerDown(kduart_t *kd) {
    ASSERT(kd != NULL);
    
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    if (osMutexAcquire(kd->_va->writeMutex, osWaitForever) != osOK) {
        goto l_exit;
    }
    
    if (osMutexAcquire(kd->_va->recvMutex, osWaitForever) != osOK) {
        goto l_exit;
    }
#endif

    if (!kd->_config.uart.isLp) {
    } else {
        kd->_config.uart.lpuart->SCON &= ~(0x01u << 16u);
    }
    
    if (kd->_config.pin.tx.gpio != NULL) {
        _gpio_afConfig(kd->_config.pin.tx.gpio->_config.base.port, 
            kd->_config.pin.tx.gpio->_config.base.number, 
            0);
        kdgpio_powerDown(kd->_config.pin.tx.gpio);
    }
    
    if (kd->_config.pin.rx.gpio != NULL) {
        _gpio_afConfig(kd->_config.pin.rx.gpio->_config.base.port, 
            kd->_config.pin.rx.gpio->_config.base.number, 
            0);
        kdgpio_powerDown(kd->_config.pin.rx.gpio);
    }
    
l_exit:
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    osMutexRelease(kd->_va->writeMutex);
    osMutexRelease(kd->_va->recvMutex);
#endif

    return 0;
}


int32_t kduart_sends(kduart_t *kd, const void *data, uint32_t size, uint32_t timeout) {
    if (kd->_config.pin.txPinMode == KDUART_TX_PIN_MODE_IN_IDLE) {
        _gpio_modeConfig(kd->_config.pin.tx.gpio->_config.base.port, 
            kd->_config.pin.tx.gpio->_config.base.number, 
            kd->_config.pin.tx.gpio->_config.base.pin, 
            KDGPIO_MODE_OUTPUT_PP);
        _gpio_pullConfig(kd->_config.pin.tx.gpio->_config.base.port, 
            kd->_config.pin.tx.gpio->_config.base.number, 
            kd->_config.pin.tx.gpio->_config.base.pin, 
            KDGPIO_PULL_NONE);
        _gpio_afConfig(kd->_config.pin.tx.gpio->_config.base.port, 
            kd->_config.pin.tx.gpio->_config.base.number, 
            kd->_config.pin.tx.af);
    }
    
    if (!kd->_config.uart.isLp) {
        for (register uint32_t i = 0; i < size; i++) {
            kd->_config.uart.uart->SBUF = ((uint8_t *) data)[i];
            
            waitSendCompleted(kd);
        }
    } else {
        for (register uint32_t i = 0; i < size; i++) {
            kd->_config.uart.lpuart->SBUF = ((uint8_t *) data)[i];
            
            waitSendCompleted(kd);
        }
    }
    
    if (kd->_config.pin.txPinMode == KDUART_TX_PIN_MODE_IN_IDLE) {
        _gpio_afConfig(kd->_config.pin.tx.gpio->_config.base.port, 
            kd->_config.pin.tx.gpio->_config.base.number, 
            0);
        kdgpio_powerDown(kd->_config.pin.tx.gpio);
    }
    
    return size;
}


int32_t kduart_recvs(kduart_t *kd, void *data, uint32_t expect_size, uint32_t *recv_size, uint32_t timeout) {
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    uint32_t flag;
    uint32_t rcv_len;
    
    if (expect_size == 0) {
        if (recv_size != NULL) {
            *recv_size = 0;
        }
        
        return 0;
    }  
    
    if (osMutexAcquire(kd->_va->recvMutex, timeout) != osOK) {
        return -1;
    }

    if (timeout != 0) {
        flag = osEventFlagsWait(kd->_va->flag, UART_FLAG_RECV_COMPLETE, osFlagsWaitAll, timeout);
        if ((flag & 0x80000000) || ((flag & UART_FLAG_RECV_COMPLETE) != UART_FLAG_RECV_COMPLETE)) {
            if (recv_size != NULL) {
                *recv_size = 0;
            }
            
            osMutexRelease(kd->_va->recvMutex);
            return -1;
        }
    } else {
        flag = osEventFlagsGet(kd->_va->flag);
        if ((flag & 0x80000000) || ((flag & UART_FLAG_RECV_COMPLETE) != UART_FLAG_RECV_COMPLETE)) {
            if (recv_size != NULL) {
                *recv_size = 0;
            }
            
            osMutexRelease(kd->_va->recvMutex);
            return -1;
        }
        
        osEventFlagsClear(kd->_va->flag, UART_FLAG_RECV_COMPLETE);
    }

    if (klwrb_getFull(kd->buffer.recvLwrb) == 0) {
        if (recv_size != NULL) {
            *recv_size = 0;
        }
        
        osMutexRelease(kd->_va->recvMutex);
        return 0;
    }
    
    if (recv_size != NULL) {
        *recv_size = klwrb_read(kd->buffer.recvLwrb, data, expect_size);
    } else {
        klwrb_read(kd->buffer.recvLwrb, data, expect_size);
    }

    osMutexRelease(kd->_va->recvMutex);
    
    return 0;
#else
    uint32_t buffCount = qBSBuffer_Count(kd->buffer.recvLwrb);
    if (buffCount == 0) {
        kd->_va->flag.isRecvCompleted = 0;
    
        if (recv_size != NULL) {
            *recv_size = 0;
        }
        return 0;
    }
    buffCount = buffCount <= expect_size ? buffCount : expect_size;
    if (recv_size != NULL) {
        if (qBSBuffer_Read(kd->buffer.recvLwrb, data, buffCount) != qTrue) {
            *recv_size = 0;
        } else {
            *recv_size = buffCount;
        }
    } else {
        qBSBuffer_Read(kd->buffer.recvLwrb, data, buffCount);
    }
    
    if (qBSBuffer_Count(kd->buffer.recvLwrb) == 0) {
        kd->_va->flag.isRecvCompleted = 0;
    }
    
    return 0;
#endif
}


int32_t kduart_flush(kduart_t *kd) {
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    if (osMutexAcquire(kd->_va->writeMutex, osWaitForever) != osOK) {
        goto l_exit;
    }
    
    if (osMutexAcquire(kd->_va->recvMutex, osWaitForever) != osOK) {
        goto l_exit;
    }
#endif

    if (kd->_config.tim.isLp == 0) {
        kd->_config.tim.tim->CR &= ~(0x01 << 7);
        kd->_config.tim.tim->INTCLR |= 0x01;
    } else {
        kd->_config.tim.lptim->CR &= ~(0x01 << 0);
        kd->_config.tim.lptim->INTCLR |= 0x01;
    }
    
    if (!kd->_config.uart.isLp) {
        (void) kd->_config.uart.uart->SBUF;
        kd->_config.uart.uart->INTCLR |= 0x07;
    } else {
        (void) kd->_config.uart.lpuart->SBUF;
        kd->_config.uart.lpuart->INTCLR |= 0x07;
    }
    
    if (kd->buffer.recvLwrb != NULL) {
        qBSBuffer_Setup(kd->buffer.recvLwrb, kd->buffer.recvBuffer, kd->buffer.recvBufferSize);
    }
    kd->_va->flag._ = 0;
    /// @Note: CX32
    kd->_va->flag.isSendCompleted = 0;

#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    osEventFlagsClear(kd->_va->flag, UART_FLAG_RECV_COMPLETE);
    osEventFlagsSet(kd->_va->flag, UART_FLAG_SEND_DMA_COMPLETE | UART_FLAG_SEND_IDLE);
        
l_exit:
    osMutexRelease(kd->_va->writeMutex);
    osMutexRelease(kd->_va->recvMutex);
#endif
    
    return 0;
}


int32_t kduart_hasRecvData(kduart_t *kd) {
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    int32_t flag = 0;

    flag = osEventFlagsGet(kd->_va->flag);
    
    if ((flag > 0) && (flag & (UART_FLAG_RECV_COMPLETE))) {
        return klwrb_getFull(kd->buffer.recvLwrb);
    }
#else
    if ((kd->_va->flag.isRecvCompleted) && !kd->_va->flag.isRecving) {
        return qBSBuffer_Count(kd->buffer.recvLwrb);
    }
#endif

    return 0;
}


int32_t kduart_isSendIdle(kduart_t *kd) {
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    int32_t flag = 0;
    
    flag = osEventFlagsGet(kd->_va->flag);
   
    return (flag > 0) && (flag & (UART_FLAG_SEND_DMA_COMPLETE | UART_FLAG_SEND_IDLE));
#else
    return kd->_va->flag.isSendCompleted;
#endif
}


void kduart_updateBaudRate(kduart_t *kd, uint32_t bd) {
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    if (osMutexAcquire(kd->_va->writeMutex, osWaitForever) != osOK) {
        goto l_exit;
    }
    if (osMutexAcquire(kd->_va->recvMutex, osWaitForever) != osOK) {
        goto l_exit;
    }
#endif
    
    updateBaudRate(kd, bd);
    
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
l_exit:
    osMutexRelease(kd->_va->writeMutex);
    osMutexRelease(kd->_va->recvMutex);
#endif
}

/*@}*/
