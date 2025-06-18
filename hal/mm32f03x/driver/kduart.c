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
    kd->_config.tim.tim->PSC = kd->_config.tim.timerClock / 200000 - 1;
    
    kd->_config.tim.tim->ARR = (arr / 5) - 1;
}

static void halUpdateBaudRate(kduart_t *kd, uint32_t bd, uint32_t to) {
    RCC_ClocksTypeDef clock;
    uint32_t apbclock;
    uint32_t t1, t2;
    
    RCC_GetClocksFreq(&clock);
    
    if ((uint32_t) kd->_config.uart.uart == UART1_BASE) {
        apbclock = clock.PCLK2_Frequency;
    } else {
        apbclock = clock.PCLK1_Frequency;
    }
    
    t1 = (apbclock / bd) / 16; 
    t2 = (apbclock / bd) % 16;
    
    if (kd->_config.uart.uart->GCR & UART_GCR_UARTEN) {
        kd->_config.uart.uart->GCR &= ~UART_GCR_UARTEN;
        
        kd->_config.uart.uart->BRR = t1;
        kd->_config.uart.uart->FRA = t2;
        
        kd->_config.uart.uart->GCR |= UART_GCR_UARTEN;
    } else {
        kd->_config.uart.uart->BRR = t1;
        kd->_config.uart.uart->FRA = t2;
    }
    
    updateTimer(kd, bd, to);
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
    
    UART_Init(kd->_config.uart.uart, &kd->_config.uart.init);
    UART_ITConfig(kd->_config.uart.uart, UART_IT_RXIEN | UART_IT_PE | UART_OVER_ERR | UART_IT_ERR, ENABLE);
    
    TIM_TimeBaseInitTypeDef timerInit = {
        .TIM_RepetitionCounter = 0,
        .TIM_Period = 1,
        .TIM_ClockDivision = TIM_CKD_DIV1,
        .TIM_CounterMode = TIM_CounterMode_Up,
        .TIM_Prescaler = 1,
    };
    TIM_TimeBaseInit(kd->_config.tim.tim, &timerInit);

    updateTimer(kd, kd->_config.uart.init.UART_BaudRate, kd->_config.rto);

    kd->_config.tim.tim->CR1 &= ~TIM_CR1_CEN;
    kd->_config.tim.tim->SR &= ~TIM_SR_UIF;
    kd->_config.tim.tim->CNT = 0;
    TIM_ITConfig(kd->_config.tim.tim, TIM_IT_Update, ENABLE);

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
    ASSERT(kd != NULL);
    
    if (emmkDriver_initRefsCountDown(&kd->_va->initRefs) != 0) {
        return -1;
    }
    
    kd->_config.tim.tim->CR1 &= ~TIM_CR1_CEN;
    TIM_DeInit(kd->_config.tim.tim);

    UART_DeInit(kd->_config.uart.uart);
    
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
    ASSERT(kd != NULL);
    
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    if (osMutexAcquire(kd->_va->writeMutex, osWaitForever) != osOK) {
        goto l_exit;
    }
    
    if (osMutexAcquire(kd->_va->recvMutex, osWaitForever) != osOK) {
        goto l_exit;
    }
#endif

    if (kd->_config.uart.uart->GCR & UART_GCR_UARTEN) {
        return -1;
    }
    
    if (kd->_config.pin.tx.gpio != NULL) {
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
    }
    
    kd->_config.tim.tim->CR1 &= ~TIM_CR1_CEN;
    kd->_config.tim.tim->SR &= ~TIM_SR_UIF;
    kd->_config.tim.tim->CNT = 0;

    if (kd->buffer.recvLwrb != NULL) {
        qBSBuffer_Setup(kd->buffer.recvLwrb, kd->buffer.recvBuffer, kd->buffer.recvBufferSize);
    }

    kd->_config.uart.uart->ICR |= 
        UART_ICR_TXICLR | UART_ICR_RXICLR 
        | UART_ICR_TIMEOUTCLR 
        | UART_ICR_RXOERRCLR | UART_ICR_RXPERRCLR 
        | UART_ICR_RXFERRCLR | UART_ICR_RXBRKCLR;
    (void) kd->_config.uart.uart->RDR;
    
    kd->_config.uart.uart->GCR |= UART_GCR_UARTEN;
    
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

    if (!(kd->_config.uart.uart->GCR & UART_GCR_UARTEN)) {
        return 0;
    }

    kd->_config.tim.tim->CR1 &= ~TIM_CR1_CEN;
    kd->_config.tim.tim->SR &= ~TIM_SR_UIF;
    kd->_config.tim.tim->CNT = 0;

    kd->_config.uart.uart->ICR |= 
        UART_ICR_TXICLR | UART_ICR_RXICLR 
        | UART_ICR_TIMEOUTCLR 
        | UART_ICR_RXOERRCLR | UART_ICR_RXPERRCLR 
        | UART_ICR_RXFERRCLR | UART_ICR_RXBRKCLR;
    (void) kd->_config.uart.uart->RDR;
    
    kd->_config.uart.uart->GCR &= ~UART_GCR_UARTEN;
    
    GPIO_InitTypeDef init = {
        .GPIO_Mode = GPIO_Mode_IN_FLOATING,
    };
    
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
        while (kd->_config.uart.uart->CSR & UART_CSR_TXFULL) {
            osThreadYield();
        }
        
        kd->_config.uart.uart->TDR = ((uint8_t *) data)[i];
    }
    
    while (!(kd->_config.uart.uart->CSR & UART_CSR_TXC)) {
        osThreadYield();
    }
    
    osEventFlagsSet(kd->_va->flag, UART_FLAG_SEND_DMA_COMPLETE | UART_FLAG_SEND_IDLE);
    osMutexRelease(kd->_va->writeMutex);
    return size;
#else
    for (uint32_t i = 0; i < size; i++) {
        while (kd->_config.uart.uart->CSR & UART_CSR_TXFULL) {
        }
        
        kd->_config.uart.uart->TDR = ((uint8_t *) data)[i];
    }
    
    while (!(kd->_config.uart.uart->CSR & UART_CSR_TXC)) {
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
    
    kd->_config.uart.uart->GCR &= ~(UART_GCR_TXEN | UART_GCR_RXEN);
    
    if (kd->buffer.recvLwrb != NULL) {
        qBSBuffer_Setup(kd->buffer.recvLwrb, kd->buffer.recvBuffer, kd->buffer.recvBufferSize);
    }
    kd->_va->flag._ = 0;
    kd->_va->flag.isSendCompleted = 1;
    
    kd->_config.uart.uart->GCR |= (UART_GCR_TXEN | UART_GCR_RXEN);
    
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
    if ((kd->_va->flag.isRecvCompleted)) {
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
    
    halUpdateBaudRate(kd, bd, kd->_config.rto);
    
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
l_exit:
    osMutexRelease(kd->_va->writeMutex);
    osMutexRelease(kd->_va->recvMutex);
#endif
}

/*@}*/
