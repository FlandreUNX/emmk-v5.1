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

static void mutexNew(kduart_t *kd) {
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    const osMutexAttr_t mutex_attr = {
        .attr_bits = osMutexRobust | osMutexPrioInherit
    };
    
    kd->_va->writeMutex = osMutexNew(&mutex_attr);
    ASSERT(kd->_va->writeMutex != NULL);
    
    kd->_va->recvMutex = osMutexNew(&mutex_attr);
    ASSERT(kd->_va->recvMutex != NULL);
#endif
}


static void mutexDelete(kduart_t *kd) {
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    osMutexDelete(kd->_va->writeMutex);
    kd->_va->writeMutex = NULL;
    
    osMutexDelete(kd->_va->recvMutex);
    kd->_va->recvMutex = NULL;
#endif
}


__STATIC_FORCEINLINE void mutexWriteLock(kduart_t *kd) {
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    osMutexAcquire(kd->_va->writeMutex, osWaitForever);
#endif
}


__STATIC_FORCEINLINE void mutexWriteUnlock(kduart_t *kd) {
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    osMutexRelease(kd->_va->writeMutex);
#endif
}


__STATIC_FORCEINLINE void mutexRecvLock(kduart_t *kd) {
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    osMutexAcquire(kd->_va->recvMutex, osWaitForever);
#endif
}


__STATIC_FORCEINLINE void mutexRecvUnlock(kduart_t *kd) {
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    osMutexRelease(kd->_va->recvMutex);
#endif
}


__STATIC_FORCEINLINE void threadYield(void) {
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    osThreadYield();
#else
    // __WFI();
#endif
}


static int32_t halBlockTxIsFull(kduart_t *kd) {
    qSTimer_t tmcd;
    
    qSTimer_Set(&tmcd, 100);
    while ((kd->_config.uart.uart->CTRL & UART_CTRL_TXFF_Msk)
            && (!qSTimer_Expired(&tmcd))) {
        threadYield();
    }
            
    if (qSTimer_Expired(&tmcd)) {
        return -1;
    } else {
        return 0;
    }
}


static int32_t halBlockTxCompleted(kduart_t *kd) {
    qSTimer_t tmcd;
    
    qSTimer_Set(&tmcd, 100);
    while (!(kd->_config.uart.uart->CTRL & 0x01u)
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
 * @addtogroup Public functions support
 * @note none
 */
 
/*@{*/

int32_t kduart_init(kduart_t *kd) {
    ASSERT(kd != NULL);

    if (emmkDriver_initRefsCountUp(&kd->_va->initRefs) != 0) {
        return -1;
    }
    
    kd->_base.baseEnable(kd);
    
    UART_Init(kd->_config.uart.uart, &kd->_config.uart.init);

#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    kd->_va->flag = osEventFlagsNew(NULL);
    ASSERT(kd->_va->flag != NULL);
    
    mutexNew(kd);
#else
    kd->_va->flag._ = 0;
    kd->_va->flag.isSendCompleted = 1;
#endif
    
    if (kd->_buffer.recvLwrb != NULL) {
        qBSBuffer_Setup(kd->_buffer.recvLwrb, kd->_buffer.recvBuffer, kd->_buffer.recvBufferSize);
    }
    kduart_flush(kd);
    
    return 0;
}

int32_t kduart_finalize(kduart_t *kd) {
    ASSERT(kd != NULL);
    
    if (emmkDriver_initRefsCountDown(&kd->_va->initRefs) != 0) {
        return -1;
    }
    
    if (kd->_va->initRefsPower != 0) {
        kd->_va->initRefsPower = 1;
        kduart_powerDown(kd);
    }
    
    kd->_base.baseDisable(kd);

#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    osEventFlagsDelete(kd->_va->flag);
    kd->_va->flag = NULL;
    
    mutexDelete(kd);
#endif
    
    return 0;
}

int32_t kduart_powerUp(kduart_t *kd) {
    ASSERT(kd != NULL);
    
    int32_t rc = -1;
    
    if (emmkDriver_initRefsCountUp(&kd->_va->initRefsPower) != 0) {
        return -1;
    }
    
    mutexWriteLock(kd);
    mutexRecvLock(kd);

    if (kd->_config.pin.tx.gpio != NULL) {
        PORT_Init(kd->_config.pin.tx.gpio->_config.base.port, 
            kd->_config.pin.tx.gpio->_config.base.number,
            kd->_config.pin.tx.af, 1);
    }
    
    if (kd->_config.pin.rx.gpio != NULL) {
        PORT_Init(kd->_config.pin.rx.gpio->_config.base.port, 
            kd->_config.pin.rx.gpio->_config.base.number,
            kd->_config.pin.rx.af, 1);
    }
    
    if (kd->_buffer.recvLwrb != NULL) {
        qBSBuffer_Setup(kd->_buffer.recvLwrb, kd->_buffer.recvBuffer, kd->_buffer.recvBufferSize);
    }
    UART_Open(kd->_config.uart.uart);
    
    rc = 0;
    
l_exit:
    mutexWriteUnlock(kd);
    mutexRecvUnlock(kd);
    
    return rc;
}

int32_t kduart_powerDown(kduart_t *kd) {
    ASSERT(kd != NULL);
    
    int32_t rc = -1;
    
    if (emmkDriver_initRefsCountDown(&kd->_va->initRefsPower) != 0) {
        return -1;
    }
    
    mutexWriteLock(kd);
    mutexRecvLock(kd);
    
    if (kd->_config.pin.tx.gpio != NULL) {
        PORT_Init(kd->_config.pin.tx.gpio->_config.base.port, 
            kd->_config.pin.tx.gpio->_config.base.number,
            0, 0);
        kdgpio_powerDown(kd->_config.pin.tx.gpio);
    }
    
    if (kd->_config.pin.rx.gpio != NULL) {
        PORT_Init(kd->_config.pin.rx.gpio->_config.base.port, 
            kd->_config.pin.rx.gpio->_config.base.number,
            0, 0);
        kdgpio_powerDown(kd->_config.pin.rx.gpio);
    }
    
    UART_Close(kd->_config.uart.uart);
    
    rc = 0;
    
l_exit:
    mutexWriteUnlock(kd);
    mutexRecvUnlock(kd);
    
    return rc;
}


int32_t kduart_sends(kduart_t *kd, const void *data, uint32_t size, uint32_t timeout) {
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    int32_t len;
    uint32_t flag;
    
    if (size == 0) {
        return -1;
    }
    
    if ((osEventFlagsGet(kd->_va->flag) & (UART_FLAG_SEND_DMA_COMPLETE | UART_FLAG_SEND_IDLE)) == (UART_FLAG_SEND_DMA_COMPLETE | UART_FLAG_SEND_IDLE)) {
        mutexWriteUnlock(kd);
    }

    if (osMutexAcquire(kd->_va->writeMutex, timeout) != osOK) {
        return -1;
    }

    osEventFlagsClear(kd->_va->flag, UART_FLAG_SEND_DMA_COMPLETE | UART_FLAG_SEND_IDLE);
    
    for (uint32_t i = 0; i < size; i++) {
        if (halBlockTxIsFull(kd) != 0) {
            UART_Close(kd->_config.uart.uart);
            UART_Open(kd->_config.uart.uart);
            mutexWriteUnlock(kd);
            return -1;
        }

        UART_WriteByte(kd->_config.uart.uart, ((uint8_t *) data)[i]);
    }
    
    if (halBlockTxCompleted(kd) != 0) {
        UART_Close(kd->_config.uart.uart);
        UART_Open(kd->_config.uart.uart);
        mutexWriteUnlock(kd);
        return -1;
    }

    osEventFlagsSet(kd->_va->flag, UART_FLAG_SEND_DMA_COMPLETE | UART_FLAG_SEND_IDLE);
    mutexWriteUnlock(kd);
    return size;
#else
    for (register uint32_t i = 0; i < size; i++) {
        if (halBlockTxIsFull(kd) != 0) {
            UART_Close(kd->_config.uart.uart);
            UART_Open(kd->_config.uart.uart);
            return -1;
        }

        UART_WriteByte(kd->_config.uart.uart, ((uint8_t *) data)[i]);
    }

    while (UART_IsTXBusy(kd->_config.uart.uart)) {
        threadYield();
    }

    return size;
#endif
}

int32_t kduart_tsends9(kduart_t *kd, uint16_t *data, uint32_t size, uint32_t timeout) {
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
    
    mutexWriteLock(kd);

    if (timeout != 0) {
        flag = osEventFlagsWait(kd->_va->flag, UART_FLAG_RECV_COMPLETE, osFlagsWaitAll, timeout);
        if ((flag & 0x80000000) || ((flag & UART_FLAG_RECV_COMPLETE) != UART_FLAG_RECV_COMPLETE)) {
            if (recv_size != NULL) {
                *recv_size = 0;
            }
            
            mutexRecvUnlock(kd);
            return -1;
        }
    } else {
        flag = osEventFlagsGet(kd->_va->flag);
        if ((flag & 0x80000000) || ((flag & UART_FLAG_RECV_COMPLETE) != UART_FLAG_RECV_COMPLETE)) {
            if (recv_size != NULL) {
                *recv_size = 0;
            }
            
            mutexRecvUnlock(kd);
            return -1;
        }
        
        osEventFlagsClear(kd->_va->flag, UART_FLAG_RECV_COMPLETE);
    }

    if (klwrb_getFull(kd->_buffer.recvLwrb) == 0) {
        if (recv_size != NULL) {
            *recv_size = 0;
        }
        
        mutexRecvUnlock(kd);
        return 0;
    }
    
    if (recv_size != NULL) {
        *recv_size = klwrb_read(kd->_buffer.recvLwrb, data, expect_size);
    } else {
        klwrb_read(kd->_buffer.recvLwrb, data, expect_size);
    }

    mutexRecvUnlock(kd);
    return 0;
#else
    if (kd->_va->flag.isRecving || !kd->_va->flag.isRecvCompleted) {
        if (recv_size != NULL) {
            *recv_size = 0;
        }
        return -1;
    }
            
    uint32_t buffCount = qBSBuffer_Count(kd->_buffer.recvLwrb);
    if (buffCount == 0) {
        kd->_va->flag.isRecvCompleted = 0;
    
        if (recv_size != NULL) {
            *recv_size = 0;
        }
        return 0;
    }
    buffCount = buffCount <= expect_size ? buffCount : expect_size;
    if (recv_size != NULL) {
        if (qBSBuffer_Read(kd->_buffer.recvLwrb, data, buffCount) != qTrue) {
            *recv_size = 0;
        } else {
            *recv_size = buffCount;
        }
    } else {
        qBSBuffer_Read(kd->_buffer.recvLwrb, data, buffCount);
    }
    
    if (qBSBuffer_Count(kd->_buffer.recvLwrb) == 0) {
        kd->_va->flag.isRecvCompleted = 0;
    }
    
    return 0;
#endif
}


int32_t kduart_flush(kduart_t *kd) {
    mutexWriteLock(kd);
    mutexRecvLock(kd);
    
    if (kd->_buffer.recvLwrb != NULL) {
        qBSBuffer_Setup(kd->_buffer.recvLwrb, kd->_buffer.recvBuffer, kd->_buffer.recvBufferSize);
    }

#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    osEventFlagsClear(kd->_va->flag, UART_FLAG_RECV_COMPLETE);
    osEventFlagsSet(kd->_va->flag, UART_FLAG_SEND_DMA_COMPLETE | UART_FLAG_SEND_IDLE);
#else
    kd->_va->flag._ = 0;
    kd->_va->flag.isSendCompleted = 1;
#endif
    mutexWriteUnlock(kd);
    mutexRecvUnlock(kd);
    
    return 0;
}


int32_t kduart_hasRecvData(kduart_t *kd) {
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    int32_t flag = 0;

    flag = osEventFlagsGet(kd->_va->flag);
    
    if ((flag > 0) && (flag & (UART_FLAG_RECV_COMPLETE))) {
        return klwrb_getFull(kd->_buffer.recvLwrb);
    }
#else
    if ((kd->_va->flag.isRecvCompleted) && !kd->_va->flag.isRecving) {
        return qBSBuffer_Count(kd->_buffer.recvLwrb);
    }
#endif
    return 0;
}

int32_t kduart_tisSendIdle(kduart_t *kd) {
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    int32_t flag = 0;
    
    flag = osEventFlagsGet(kd->_va->flag);
   
    return (flag > 0) && (flag & (UART_FLAG_SEND_DMA_COMPLETE | UART_FLAG_SEND_IDLE));
#else
    return kd->_va->flag.isSendCompleted;
#endif
}

void kduart_updateBaudRate(kduart_t *kd, uint32_t bd) {
    mutexWriteLock(kd);
    mutexRecvLock(kd);
    
    uint32_t isWorking = kd->_config.uart.uart->CTRL & 0x200u;

    UART_Close(kd->_config.uart.uart);
    
    UART_SetBaudrate(kd->_config.uart.uart, bd);
    
    if (isWorking) {
        UART_Open(kd->_config.uart.uart);
    }

    mutexWriteUnlock(kd);
    mutexRecvUnlock(kd);
}

/*@}*/

#pragma GCC pop_options
