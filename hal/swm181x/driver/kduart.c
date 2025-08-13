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
#else
    kd->_va->flag._ = 0;
    kd->_va->flag.isSendCompleted = 1;
#endif

    if (kd->buffer.recvLwrb != NULL) {
        qBSBuffer_Setup(kd->buffer.recvLwrb, kd->buffer.recvBuffer, kd->buffer.recvBufferSize);
    }
    if (kd->buffer.writeLwrb != NULL) {
        qBSBuffer_Setup(kd->buffer.writeLwrb, kd->buffer.writeBuffer, kd->buffer.writeBufferSize);
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
#endif

    return 0;
}

int32_t kduart_powerUp(kduart_t *kd) {
    ASSERT(kd != NULL);

    if (emmkDriver_initRefsCountUp(&kd->_va->initRefsPower) != 0) {
        return -1;
    }

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

    if (kd->buffer.recvLwrb != NULL) {
        qBSBuffer_Setup(kd->buffer.recvLwrb, kd->buffer.recvBuffer, kd->buffer.recvBufferSize);
    }
    if (kd->buffer.writeLwrb != NULL) {
        qBSBuffer_Setup(kd->buffer.writeLwrb, kd->buffer.writeBuffer, kd->buffer.writeBufferSize);
    }

    UART_INTTXThresholdDis(kd->_config.uart.uart);
    UART_Open(kd->_config.uart.uart);

    return 0;
}

int32_t kduart_powerDown(kduart_t *kd) {
    ASSERT(kd != NULL);

    if (emmkDriver_initRefsCountDown(&kd->_va->initRefsPower) != 0) {
        return -1;
    }

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

    UART_INTTXThresholdDis(kd->_config.uart.uart);
    UART_Close(kd->_config.uart.uart);

    return 0;
}


int32_t kduart_sends(kduart_t *kd, const void *data, uint32_t size, uint32_t timeout) {
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    int32_t len;
    uint32_t flag;

    if (size == 0) {
        return -1;
    }

    if ((osEventFlagsGet(kd->_va->flag) & (UART_FLAG_SEND_DMA_COMPLETE | UART_FLAG_SEND_IDLE)) == (
            UART_FLAG_SEND_DMA_COMPLETE | UART_FLAG_SEND_IDLE)) {
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
    if (size == 0) {
        return -1;
    }
    qSTimer_t wait;


    if (kd->_va->flag.isSendCompleted == 0) {
        if (timeout == 0) {
            return -1;
        }
        qSTimer_Set(&wait, timeout);
        while (!qSTimer_Expired(&wait)) {
            if (kd->_va->flag.isSendCompleted) {
                break;
            }
        }
        if (qSTimer_Expired(&wait)) {
            return -1;
        }
    }
    kd->_va->flag.isSendCompleted = 0;

    if (size != 1
        && kd->buffer.writeBufferSize != 0
        && size <= kd->buffer.writeBufferSize) {
        for (uint32_t i = 0; i < size; i++) {
            qBSBuffer_Put(kd->buffer.writeLwrb, ((uint8_t *) data)[i]);
        }
        UART_INTTXThresholdEn(kd->_config.uart.uart);
    } else {
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
        kd->_va->flag.isSendCompleted = 1;
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

    if (klwrb_getFull(kd->buffer.recvLwrb) == 0) {
        if (recv_size != NULL) {
            *recv_size = 0;
        }

        mutexRecvUnlock(kd);
        return 0;
    }

    if (recv_size != NULL) {
        *recv_size = klwrb_read(kd->buffer.recvLwrb, data, expect_size);
    } else {
        klwrb_read(kd->buffer.recvLwrb, data, expect_size);
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
    if (kd->buffer.recvLwrb != NULL) {
        qBSBuffer_Setup(kd->buffer.recvLwrb, kd->buffer.recvBuffer, kd->buffer.recvBufferSize);
    }
    if (kd->buffer.writeLwrb != NULL) {
        qBSBuffer_Setup(kd->buffer.writeLwrb, kd->buffer.writeBuffer, kd->buffer.writeBufferSize);
    }

#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    osEventFlagsClear(kd->_va->flag, UART_FLAG_RECV_COMPLETE);
    osEventFlagsSet(kd->_va->flag, UART_FLAG_SEND_DMA_COMPLETE | UART_FLAG_SEND_IDLE);
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
        return klwrb_getFull(kd->buffer.recvLwrb);
    }
#else
    if ((kd->_va->flag.isRecvCompleted) && !kd->_va->flag.isRecving) {
        return qBSBuffer_Count(kd->buffer.recvLwrb);
    }
#endif
    return 0;
}


int32_t kduart_isSendIdle(kduart_t *kd, uint32_t wait) {
    if (kd->_va->flag.isSendCompleted) {
        return true;
    }
    if (wait == 0) {
        return false;
    }
    qSTimer_t waitTimer;
    qSTimer_Set(&waitTimer, wait);
    while (!qSTimer_Expired(&waitTimer)) {
        if (kd->_va->flag.isSendCompleted) {
            return true;
        }
    }
    return true;
}

void kduart_updateBaudRate(kduart_t *kd, uint32_t bd) {
    uint32_t isWorking = kd->_config.uart.uart->CTRL & 0x200u;

    UART_Close(kd->_config.uart.uart);

    UART_SetBaudrate(kd->_config.uart.uart, bd);

    if (isWorking) {
        UART_Open(kd->_config.uart.uart);
    }
}


bool kduart_sendBuffingVerify(kduart_t *kd, uint32_t dataSize) {
    if (kd->buffer.writeBufferSize == 0) {
        return false;
    }
    if (dataSize > kd->buffer.writeBufferSize) {
        return false;
    }
    return true;
}

/*@}*/

#pragma GCC pop_options
