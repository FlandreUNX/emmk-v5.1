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

static void updateTimer(kduart_t *kd, uint32_t bd, uint32_t to) {
    uint32_t arr;
    
    // 1byte(10bit)us
    // (1 / (bd / 10)) = Xms(1byte)
    arr = (10000.0f / (float) (bd)) * 1000.0f * to;
    
    // 1cnt=5us
    kd->_config.tim.btim->PSC = SystemCoreClock / 200000 - 1;
    kd->_config.tim.btim->ARR = (arr / 5) - 1;
    kd->_config.tim.btim->CNT = 0;
}


static void updateBaudRate(kduart_t *kd, uint32_t bd) {
    uint32_t freq = SystemCoreClock;
    float temp = (float) (freq) / (16 * (bd));
    uint16_t integerdivider = (uint16_t) temp;
    uint16_t fractionaldivider = (uint16_t) ((temp - integerdivider) * 16 + 0.5);
    kd->_config.uart.uart->BRRI = (uint16_t) integerdivider;
    kd->_config.uart.uart->BRRF = (uint16_t) fractionaldivider;
    
    updateTimer(kd, bd, kd->_config.rto);
}

/*@}*/

/**
 * @addtogroup Public functions support
 * @note none
 */
 
/*@{*/

int32_t kduart_init(kduart_t *kd) {
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    const osMutexAttr_t mutex_attr = {
        .attr_bits = osMutexRobust
    };
#endif
    
    if (emmkDriver_initRefsCountUp(&kd->_va->initRefs) != 0) {
        return -1;
    }
    
    kd->_instance.enableFunc(kd);
    
    REGBITS_MODIFY(kd->_config.uart.uart->CR1, 
            UARTx_CR1_SOURCE_Msk | UARTx_CR1_OVER_Msk 
            | UARTx_CR1_START_Msk | UARTx_CR1_STOP_Msk 
            | UARTx_CR1_PARITY_Msk | UARTx_CR1_RXEN_Msk 
            | UARTx_CR1_TXEN_Msk,
        UART_Source_PCLK 
        | (uint32_t) UART_Over_16 
        | UART_StartBit_FE
        | kd->_config.uart.init.stop
        | kd->_config.uart.init.parity);
    
    if (kd->_config.uart.init.parity != UART_Parity_No) {
        kd->_config.uart.uart->CR1_f.CHLEN = 1;
    } else {
        kd->_config.uart.uart->CR1_f.CHLEN = 0;
    }
    
    kd->_config.uart.uart->IER |= UART_IT_RC | UART_IT_PE | UART_IT_FE | UART_IT_TIMOV;
    
    if (kd->_config.uart.levelInv & UART_InvPin_Tx) {
        kd->_config.uart.uart->CR2 |= UARTx_CR2_TXINV_Msk;
    } else {
        kd->_config.uart.uart->CR2 &= ~UARTx_CR2_TXINV_Msk;
    }
    
    if (kd->_config.uart.levelInv & UART_InvPin_Rx) {
        kd->_config.uart.uart->CR2 |= UARTx_CR2_RXINV_Msk;
    } else {
        kd->_config.uart.uart->CR2 &= ~UARTx_CR2_RXINV_Msk;
    }
    
    if (kd->_config.uart.pinInv) {
        kd->_config.uart.uart->CR2 |= UARTx_CR2_SWAP_Msk;
    } else {
        kd->_config.uart.uart->CR2 &= ~UARTx_CR2_SWAP_Msk;
    }
    
    kd->_config.uart.uart->TIMARR = 0;

#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    kd->_va->flag = osEventFlagsNew(NULL);
    ASSERT(kd->_va->flag != NULL);
    
    kd->_va->writeMutex = osMutexNew(&mutex_attr);
    ASSERT(kd->_va->writeMutex != NULL);
    
    kd->_va->recvMutex = osMutexNew(&mutex_attr);
    ASSERT(kd->_va->recvMutex != NULL);
#else
    kd->_va->flag._ = 0;
    kd->_va->flag.isSendCompleted = 1;
#endif
    
    if (kd->buffer.recvLwrb != NULL) {
        qBSBuffer_Setup(kd->buffer.recvLwrb, kd->buffer.recvBuffer, kd->buffer.recvBufferSize);
    }

    kduart_flush(kd);
    
    return 0;
}


int32_t kduart_finalize(kduart_t *kd) {
    if (emmkDriver_initRefsCountDown(&kd->_va->initRefs) != 0) {
        return -1;
    }
    
    kd->_config.uart.uart->CR1 = 0;
    kd->_config.uart.uart->CR2 = 0;
    kd->_config.uart.uart->TIMARR = 0;

    kd->_instance.disableFunc(kd);

#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    osEventFlagsDelete(kd->_va->flag);
    kd->_va->flag = NULL;
    
    osMutexDelete(kd->_va->recvMutex);
    kd->_va->recvMutex = NULL;
    
    osMutexDelete(kd->_va->writeMutex);
    kd->_va->writeMutex = NULL;
#endif
    
    return 0;
}


int32_t kduart_powerUp(kduart_t *kd) {
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    if (osMutexAcquire(kd->_va->writeMutex, osWaitForever) != osOK) {
        goto l_exit;
    }
    if (osMutexAcquire(kd->_va->recvMutex, osWaitForever) != osOK) {
        goto l_exit;
    }
#endif

    if (kd->_config.uart.uart->CR1 & UARTx_CR1_TXEN_Msk
            || kd->_config.uart.uart->CR1 & UARTx_CR1_RXEN_Msk) {
        return -1;
    }
    
    if (kd->_config.pin.tx.gpio != NULL) {
        _gpio_modeConfig(kd->_config.pin.tx.gpio->_config.base.port, 
            kd->_config.pin.tx.gpio->_config.base.pin, 
            KDGPIO_MODE_OUTPUT_PP);
        _gpio_pullConfig(kd->_config.pin.tx.gpio->_config.base.port, 
            kd->_config.pin.tx.gpio->_config.base.pin, 
            KDGPIO_PULL_NONE);
        _gpio_afConfig(kd->_config.pin.tx.gpio->_config.base.port, 
            kd->_config.pin.tx.gpio->_config.base.number, 
            kd->_config.pin.tx.af);
        kd->_config.uart.uart->CR1 |= UARTx_CR1_TXEN_Msk;
    } else {
        kd->_config.uart.uart->CR1 &= ~UARTx_CR1_TXEN_Msk;
    }
    
    if (kd->_config.pin.rx.gpio != NULL) {
        _gpio_modeConfig(kd->_config.pin.rx.gpio->_config.base.port, 
            kd->_config.pin.rx.gpio->_config.base.pin, 
            KDGPIO_MODE_INPUT);
        _gpio_pullConfig(kd->_config.pin.rx.gpio->_config.base.port, 
            kd->_config.pin.rx.gpio->_config.base.pin, 
            KDGPIO_PULL_NONE);
        _gpio_afConfig(kd->_config.pin.rx.gpio->_config.base.port, 
            kd->_config.pin.rx.gpio->_config.base.number, 
            kd->_config.pin.rx.af);
        kd->_config.uart.uart->CR1 |= UARTx_CR1_RXEN_Msk;
    } else {
        kd->_config.uart.uart->CR1 &= ~UARTx_CR1_RXEN_Msk;
    }

    if (kd->buffer.recvLwrb != NULL) {
        qBSBuffer_Setup(kd->buffer.recvLwrb, kd->buffer.recvBuffer, kd->buffer.recvBufferSize);
    }

    kd->_config.uart.uart->ICR = 0;
    (void) kd->_config.uart.uart->RDR;
    kd->_config.uart.uart->CR2 &= ~UARTx_CR2_TIMCR_Msk;
    kd->_config.uart.uart->TIMARR = 0;
    
    if (kd->_config.tim.btim != NULL) {
        kd->_config.tim.btim->CR1 &= ~KLBIT(0);
        kd->_config.tim.btim->CNT = 0;
        kd->_config.tim.btim->IER |= KLBIT(0);
        kd->_config.tim.btim->ICR &= ~KLBIT(0);
    }
    
l_exit:
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    osMutexRelease(kd->_va->writeMutex);
    osMutexRelease(kd->_va->recvMutex);
#endif

    return 0;
}


int32_t kduart_powerDown(kduart_t *kd) {
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    if (osMutexAcquire(kd->_va->writeMutex, osWaitForever) != osOK) {
        goto l_exit;
    }
    
    if (osMutexAcquire(kd->_va->recvMutex, osWaitForever) != osOK) {
        goto l_exit;
    }
#endif

    if (!(kd->_config.uart.uart->CR1 & UARTx_CR1_TXEN_Msk)
            && !(kd->_config.uart.uart->CR1 & UARTx_CR1_RXEN_Msk)) {
        return 0;
    }

    kd->_config.uart.uart->ICR = 0;
    (void) kd->_config.uart.uart->RDR;
    
    kd->_config.uart.uart->CR1 &= ~(UARTx_CR1_TXEN_Msk | UARTx_CR1_RXEN_Msk);
    
    if (kd->_config.tim.btim != NULL) {
        kd->_config.tim.btim->CR1 &= ~KLBIT(0);
        kd->_config.tim.btim->IER &= ~KLBIT(0);
        kd->_config.tim.btim->ICR &= ~KLBIT(0);
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
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    int32_t len;
    uint32_t flag;
    
    if (size == 0) {
        return -1;
    }
    
    if ((osEventFlagsGet(kd->_va->flag) & (UART_FLAG_SEND_DMA_COMPLETE | UART_FLAG_SEND_IDLE)) == (UART_FLAG_SEND_DMA_COMPLETE | UART_FLAG_SEND_IDLE)) {
        osMutexRelease(kd->_va->writeMutex);
    }

    if (osMutexAcquire(kd->_va->writeMutex, timeout) != osOK) {
        return -1;
    }

    osEventFlagsClear(kd->_va->flag, UART_FLAG_SEND_DMA_COMPLETE | UART_FLAG_SEND_IDLE);
    
    for (uint32_t i = 0; i < size; i++) {
        while (kd->_config.uartConfig.uart->CSR & UART_CSR_TXFULL) {
            osThreadYield();
        }
        
        kd->_config.uartConfig.uart->TDR = ((uint8_t *) data)[i];
    }
    
    while (!(kd->_config.uartConfig.uart->CSR & UART_CSR_TXC)) {
        osThreadYield();
    }
    
    osEventFlagsSet(kd->_va->flag, UART_FLAG_SEND_DMA_COMPLETE | UART_FLAG_SEND_IDLE);
    osMutexRelease(kd->_va->writeMutex);
    return size;
#else
    if (kd->_config.pin.txPinMode == KDUART_TX_PIN_MODE_IN_IDLE) {
        _gpio_modeConfig(kd->_config.pin.tx.gpio->_config.base.port, 
            kd->_config.pin.tx.gpio->_config.base.pin, 
            KDGPIO_MODE_AF_PP);
        _gpio_pullConfig(kd->_config.pin.tx.gpio->_config.base.port, 
            kd->_config.pin.tx.gpio->_config.base.pin, 
            KDGPIO_PULL_NONE);
        _gpio_afConfig(kd->_config.pin.tx.gpio->_config.base.port, 
            kd->_config.pin.tx.gpio->_config.base.number, 
            kd->_config.pin.tx.af);
    }

    for (uint32_t i = 0; i < size; i++) {
        while (!(kd->_config.uart.uart->ISR & UARTx_ISR_TXE_Msk)) {
        }
        kd->_config.uart.uart->TDR = ((uint8_t *) data)[i];
    }
    
    while (!(kd->_config.uart.uart->ISR & UARTx_ISR_TC_Msk)) {
    }
    
    if (kd->_config.pin.txPinMode == KDUART_TX_PIN_MODE_IN_IDLE) {
        _gpio_afConfig(kd->_config.pin.tx.gpio->_config.base.port, 
            kd->_config.pin.tx.gpio->_config.base.number, 
            0);
        kdgpio_powerDown(kd->_config.pin.tx.gpio);
    }
    
    return size;
#endif
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

    kd->_config.uart.uart->ICR = 0;
    (void) kd->_config.uart.uart->RDR;
    
    if (kd->_config.tim.btim != NULL) {
        kd->_config.tim.btim->CR1 &= ~KLBIT(0);
        kd->_config.tim.btim->ICR &= ~KLBIT(0);
        kd->_config.tim.btim->CNT = 0;
    }
    
    if (kd->buffer.recvLwrb != NULL) {
        qBSBuffer_Setup(kd->buffer.recvLwrb, kd->buffer.recvBuffer, kd->buffer.recvBufferSize);
    }
    kd->_va->flag._ = 0;
    kd->_va->flag.isSendCompleted = 1;

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
