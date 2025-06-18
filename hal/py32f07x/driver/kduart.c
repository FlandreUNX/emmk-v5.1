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
    
    timer_prescaler_config(kd->_config.timer.timer, kd->_config.timer.timerClock / 200000 - 1, TIMER_PSC_RELOAD_NOW);
    timer_autoreload_value_config(kd->_config.timer.timer, (arr / 5) - 1);
}


static void updateBaudRate(kduart_t *kd, uint32_t bd, uint32_t to) {
    usart_baudrate_set(kd->_config.uart.uart, bd);
    
    if (kd->_config.timer.timer != NULL) {
        updateTimer(kd, bd, to);
    } else {
        usart_receiver_timeout_threshold_config(kd->_config.uart.uart, to);
    }
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
    
    usart_baudrate_set(kd->_config.uart.uart, kd->_config.uart.init.baudRate);
    usart_parity_config(kd->_config.uart.uart, kd->_config.uart.init.parity);
    usart_word_length_set(kd->_config.uart.uart, kd->_config.uart.init.dataLen);
    usart_stop_bit_set(kd->_config.uart.uart, kd->_config.uart.init.stop);
    
    usart_transmit_config(kd->_config.uart.uart,
                          kd->_config.pin.tx.gpio != NULL
                          ? USART_TRANSMIT_ENABLE
                          : USART_TRANSMIT_DISABLE);
    usart_receive_config(kd->_config.uart.uart,
                         kd->_config.pin.rx.gpio != NULL
                         ? USART_RECEIVE_ENABLE
                         : USART_RECEIVE_DISABLE);
    
    if (kd->_config.timer.timer == NULL) {
        usart_receiver_timeout_threshold_config(kd->_config.uart.uart, kd->_config.rto * (11));
        usart_receiver_timeout_enable(kd->_config.uart.uart);
        
        usart_interrupt_enable(kd->_config.uart.uart, USART_INT_RT);
    }
    
    if (kd->buffer.recvLwrb != NULL) {
        usart_interrupt_enable(kd->_config.uart.uart, USART_INT_RBNE);
    }
    usart_interrupt_enable(kd->_config.uart.uart, USART_INT_ERR);
    usart_interrupt_enable(kd->_config.uart.uart, USART_INT_PERR);
    
    if (kd->_config.timer.timer != NULL) {
        timer_parameter_struct timerInit = {
                .prescaler = 0,
                .period = 0,
                .alignedmode = TIMER_COUNTER_EDGE,
                .counterdirection = TIMER_COUNTER_UP,
                .clockdivision = TIMER_CKDIV_DIV1,
                .repetitioncounter = 0,
        };
        
        timer_init(kd->_config.timer.timer, &timerInit);
        updateTimer(kd, kd->_config.uart.init.baudRate, kd->_config.rto);
        
        timer_disable(kd->_config.timer.timer);
        timer_interrupt_flag_clear(kd->_config.timer.timer, TIMER_INT_FLAG_UP);
        timer_interrupt_enable(kd->_config.timer.timer, TIMER_INT_UP);
    } else {
        if (kd->_config.dma.channelRx != 0xFF) {
            dma_init(kd->_config.dma.dmaRx, kd->_config.dma.channelRx, &kd->_config.dma.initRx);
            dma_circulation_disable(kd->_config.dma.dmaRx, kd->_config.dma.channelRx);
            dma_memory_to_memory_disable(kd->_config.dma.dmaRx, kd->_config.dma.channelRx);
            
            usart_dma_receive_config(kd->_config.uart.uart, USART_RECEIVE_DMA_ENABLE);
        }
        
        if (kd->_config.dma.channelTx != 0xFF) {
            dma_init(kd->_config.dma.dmaTx, kd->_config.dma.channelTx, &kd->_config.dma.initTx);
            dma_circulation_disable(kd->_config.dma.dmaTx, kd->_config.dma.channelTx);
            dma_memory_to_memory_disable(kd->_config.dma.dmaTx, kd->_config.dma.channelTx);
            dma_interrupt_enable(kd->_config.dma.dmaTx, kd->_config.dma.channelTx, DMA_INT_FTF);
            
            usart_dma_transmit_config(kd->_config.uart.uart, USART_TRANSMIT_DMA_ENABLE);
        }
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
    
    if (kd->_config.timer.timer != NULL) {
        timer_disable(kd->_config.timer.timer);
        timer_deinit(kd->_config.timer.timer);
    }
    
    if (kd->_config.dma.channelTx != 0xFF) {
        dma_channel_disable(kd->_config.dma.dmaTx, kd->_config.dma.channelTx);
        dma_deinit(kd->_config.dma.dmaTx, kd->_config.dma.channelTx);
    }
    
    if (kd->_config.dma.channelRx != 0xFF) {
        dma_channel_disable(kd->_config.dma.dmaRx, kd->_config.dma.channelRx);
        dma_deinit(kd->_config.dma.dmaRx, kd->_config.dma.channelRx);
    }
    
    usart_deinit(kd->_config.uart.uart);
    
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
    
    if (USART_CTL0(kd->_config.uart.uart) & USART_CTL0_UEN) {
        return -1;
    }
    
    if (kd->_config.pin.tx.gpio != NULL) {
        _gpio_modeConfig(kd->_config.pin.tx.gpio->_config.base.port,
                         kd->_config.pin.tx.gpio->_config.base.number,
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
                         kd->_config.pin.rx.gpio->_config.base.number,
                         kd->_config.pin.rx.gpio->_config.base.pin,
                         KDGPIO_MODE_AF_PP);
        _gpio_pullConfig(kd->_config.pin.rx.gpio->_config.base.port,
                         kd->_config.pin.rx.gpio->_config.base.pin,
                         KDGPIO_PULL_NONE);
        _gpio_afConfig(kd->_config.pin.rx.gpio->_config.base.port,
                       kd->_config.pin.rx.gpio->_config.base.number,
                       kd->_config.pin.rx.af);
    }
    
    if (kd->_config.timer.timer != NULL) {
        timer_disable(kd->_config.timer.timer);
        timer_counter_value_config(kd->_config.timer.timer, 0);
    }
    
    if (kd->_config.dma.channelTx != 0xFF) {
        dma_channel_disable(kd->_config.dma.dmaTx, kd->_config.dma.channelTx);
    }
    
    if (kd->_config.dma.channelRx != 0xFF) {
        dma_channel_disable(kd->_config.dma.dmaRx, kd->_config.dma.channelRx);
        dma_transfer_number_config(kd->_config.dma.dmaRx, kd->_config.dma.channelRx, kd->buffer.recvBufferSize);
        dma_memory_address_config(kd->_config.dma.dmaRx, kd->_config.dma.channelRx, (uint32_t) kd->buffer.recvBuffer);
        dma_channel_enable(kd->_config.dma.dmaRx, kd->_config.dma.channelRx);
    }
    
    if (kd->buffer.recvLwrb != NULL) {
        qBSBuffer_Setup(kd->buffer.recvLwrb, kd->buffer.recvBuffer, kd->buffer.recvBufferSize);
    }
    
    usart_interrupt_flag_clear(kd->_config.uart.uart, USART_INT_FLAG_RT);
    usart_interrupt_flag_clear(kd->_config.uart.uart, USART_INT_FLAG_RBNE);
    usart_data_receive(kd->_config.uart.uart);
    usart_enable(kd->_config.uart.uart);
    
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
    
    if (!(USART_CTL0(kd->_config.uart.uart) & USART_CTL0_UEN)) {
        return 0;
    }
    
    if (!(USART_CTL0(kd->_config.uart.uart) & USART_CTL0_UEN)) {
        goto l_exit;
    }
    
    if (kd->_config.timer.timer != NULL) {
        timer_disable(kd->_config.timer.timer);
        timer_counter_value_config(kd->_config.timer.timer, 0);
    }
    
    if (kd->_config.dma.channelTx != 0xFF) {
        dma_channel_disable(kd->_config.dma.dmaTx, kd->_config.dma.channelTx);
    }
    
    if (kd->_config.dma.channelRx != 0xFF) {
        dma_channel_disable(kd->_config.dma.dmaRx, kd->_config.dma.channelRx);
    }
    
    usart_interrupt_flag_clear(kd->_config.uart.uart, USART_INT_FLAG_RT);
    usart_interrupt_flag_clear(kd->_config.uart.uart, USART_INT_FLAG_RBNE);
    usart_data_receive(kd->_config.uart.uart);
    usart_disable(kd->_config.uart.uart);
    
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
    
    if (size == 1) {
        while (RESET == usart_flag_get(kd->_config.uart.uart, USART_FLAG_TBE));
        usart_data_transmit(kd->_config.uart.uart, *((char *) data));
        while (RESET == usart_flag_get(kd->_config.uart.uart, USART_FLAG_TBE));
        while (RESET == usart_flag_get(kd->_config.uart.uart, USART_FLAG_TC));

        osEventFlagsSet(kd->_va->flag, UART_FLAG_SEND_DMA_COMPLETE | UART_FLAG_SEND_IDLE);
        osMutexRelease(kd->_va->writeMutex);
        return 1;
    } else if (kd->_config.dma.channelTx == 0xFF) {
        for (uint32_t i = 0; i < size; i++) {
            while (RESET == usart_flag_get(kd->_config.uart.uart, USART_FLAG_TBE));
            usart_data_transmit(kd->_config.uart.uart, ((uint8_t *) data)[i]);
        }
        while (RESET == usart_flag_get(kd->_config.uart.uart, USART_FLAG_TBE));
        while (RESET == usart_flag_get(kd->_config.uart.uart, USART_FLAG_TC));
        
        osEventFlagsSet(kd->_va->flag, UART_FLAG_SEND_DMA_COMPLETE | UART_FLAG_SEND_IDLE);
        osMutexRelease(kd->_va->writeMutex);
        return size;
    }
    
    dma_channel_disable(kd->_config.dma.dmaTx, kd->_config.dma.channelTx);
    dma_transfer_number_config(kd->_config.dma.dmaTx, kd->_config.dma.channelTx, size);
    if (kd->buffer.writeBufferSize == 0) { 
        dma_memory_address_config(kd->_config.dma.dmaTx, kd->_config.dma.channelTx, (uint32_t) data);
    } else {
        memcpy(kd->buffer.writeBuffer, data, size);
        dma_memory_address_config(kd->_config.dma.dmaTx, kd->_config.dma.channelTx, (uint32_t) kd->buffer.writeBuffer);
    }
    dma_channel_enable(kd->_config.dma.dmaTx, kd->_config.dma.channelTx);

    if (timeout == 0) {
        len = size;
    } else {
        flag = osEventFlagsWait(kd->_va->flag, UART_FLAG_SEND_DMA_COMPLETE | UART_FLAG_SEND_IDLE, osFlagsWaitAll | osFlagsNoClear, timeout);
        if ((flag & 0x80000000) || ((flag & (UART_FLAG_SEND_DMA_COMPLETE | UART_FLAG_SEND_IDLE)) != (UART_FLAG_SEND_DMA_COMPLETE | UART_FLAG_SEND_IDLE))) {
            len = -1;
        } else {
            len = size;
        }
    }
    
    osMutexRelease(kd->_va->writeMutex);

    return len;
#else
    for (uint32_t i = 0; i < size; i++) {
        while (RESET == usart_flag_get(kd->_config.uart.uart, USART_FLAG_TBE)) {}
        usart_data_transmit(kd->_config.uart.uart, ((uint8_t *) data)[i]);
    }
    while (RESET == usart_flag_get(kd->_config.uart.uart, USART_FLAG_TBE)) {}
    while (RESET == usart_flag_get(kd->_config.uart.uart, USART_FLAG_TC)) {}
    
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
        flag = osEventFlagsWait(kd->_va->flag, UART_FLAG_RECV_COMPLETE, osFlagsWaitAll | osFlagsNoClear, timeout);
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
    }
    
    if (kd->_config.timer.timer == NULL && kd->_config.dma.channelRx != 0xFF) {
        osEventFlagsClear(kd->_va->flag, UART_FLAG_RECV_COMPLETE);
        
        rcv_len = kd->buffer.recvBufferSize - dma_transfer_number_get(kd->_config.dma.dmaRx, kd->_config.dma.channelRx);
        rcv_len = rcv_len > expect_size ? expect_size : rcv_len;
        
        if (recv_size != NULL) {
            *recv_size = rcv_len;
        }

        if (rcv_len == 0) {
            dma_channel_disable(kd->_config.dma.dmaRx, kd->_config.dma.channelRx);
            dma_transfer_number_config(kd->_config.dma.dmaRx, kd->_config.dma.channelRx, kd->buffer.recvBufferSize);
            dma_memory_address_config(kd->_config.dma.dmaRx, kd->_config.dma.channelRx, (uint32_t) kd->buffer.recvBuffer);
            dma_channel_enable(kd->_config.dma.dmaRx, kd->_config.dma.channelRx);
            
            osMutexRelease(kd->_va->recvMutex);
            return -1;
        }
        
        memcpy(data, kd->buffer.recvBuffer, rcv_len);
        
        dma_channel_disable(kd->_config.dma.dmaRx, kd->_config.dma.channelRx);
        dma_transfer_number_config(kd->_config.dma.dmaRx, kd->_config.dma.channelRx, kd->buffer.recvBufferSize);
        dma_memory_address_config(kd->_config.dma.dmaRx, kd->_config.dma.channelRx, (uint32_t) kd->buffer.recvBuffer);
        dma_channel_enable(kd->_config.dma.dmaRx, kd->_config.dma.channelRx);
    } else {
        uint32_t buffCount = qBSBuffer_Count(kd->buffer.recvLwrb);
        if (buffCount == 0) {
            osEventFlagsClear(kd->_va->flag, UART_FLAG_RECV_COMPLETE);
        
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
            osEventFlagsClear(kd->_va->flag, UART_FLAG_RECV_COMPLETE);
        }
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
    
    if (kd->_config.dma.channelRx != 0xFF) {
        dma_channel_disable(kd->_config.dma.dmaRx, kd->_config.dma.channelRx);
        dma_transfer_number_config(kd->_config.dma.dmaRx, kd->_config.dma.channelRx, kd->buffer.recvBufferSize);
        dma_memory_address_config(kd->_config.dma.dmaRx, kd->_config.dma.channelRx, (uint32_t) kd->buffer.recvBuffer);
        dma_channel_enable(kd->_config.dma.dmaRx, kd->_config.dma.channelRx);
    }
    
    if (kd->_config.dma.channelTx != 0xFF) {
        dma_channel_disable(kd->_config.dma.dmaTx, kd->_config.dma.channelTx);
    }
    
    if (kd->buffer.recvLwrb != NULL) {
        qBSBuffer_Setup(kd->buffer.recvLwrb, kd->buffer.recvBuffer, kd->buffer.recvBufferSize);
    }

#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    osEventFlagsClear(kd->_va->flag, UART_FLAG_RECV_COMPLETE);
    osEventFlagsSet(kd->_va->flag, UART_FLAG_SEND_DMA_COMPLETE | UART_FLAG_SEND_IDLE);
        
l_exit:
    osMutexRelease(kd->_va->writeMutex);
    osMutexRelease(kd->_va->recvMutex);
#else
    kd->_va->flag._ = 0;
    kd->_va->flag.isSendCompleted = 1;
#endif
    
    return 0;
}


int32_t kduart_hasRecvData(kduart_t *kd) {
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    int32_t flag = 0;

    flag = osEventFlagsGet(kd->_va->flag);
    
    if ((flag > 0) && (flag & (UART_FLAG_RECV_COMPLETE))) {
        if (kd->buffer.recvLwrb != NULL) {
            return qBSBuffer_Count(kd->buffer.recvLwrb);
        } else {
            return kd->buffer.recvBufferSize - dma_transfer_number_get(kd->_config.dma.dmaRx, kd->_config.dma.channelRx);
        }
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
    
    updateBaudRate(kd, bd, kd->_config.rto);

#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    l_exit:
        osMutexRelease(kd->_va->writeMutex);
        osMutexRelease(kd->_va->recvMutex);
#endif
}

/*@}*/
