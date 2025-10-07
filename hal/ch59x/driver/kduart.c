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
 * @addtogroup HalFunc
 * @note none
 */

/*@{*/

static void uart_baudRateCfg(kduart_t *kd, uint32_t baudRate) {
    uint32_t x;

    x = 10 * GetSysClock() / 8 / baudRate;
    x = (x + 5) / 10;
    R16_UART_DL(kd->_config.uart.uart) = (uint16_t) x;
}


static void tmr_set(kduart_t *kd, uint32_t bd, uint32_t toc) {
    R8_TMR_CTRL_MOD(kd->_config.tim.tim) |= RB_TMR_ALL_CLEAR;

    R32_TMR_COUNT(kd->_config.tim.tim) = 0;
    R8_TMR_INT_FLAG(kd->_config.tim.tim) = 0xFF;

    uint32_t arr = 10000.0f / (float) (bd) * 1000.0f * (float) toc;
    R32_TMR_CNT_END(kd->_config.tim.tim) = GetSysClock() / 1000000 * arr;

    R8_TMR_CTRL_MOD(kd->_config.tim.tim) &= ~RB_TMR_ALL_CLEAR;

    R8_TMR_INTER_EN(kd->_config.tim.tim) |= RB_TMR_IE_CYC_END;
}


static void tmr_reset(kduart_t *kd) {
    R8_TMR_CTRL_MOD(kd->_config.tim.tim) |= RB_TMR_ALL_CLEAR;
    R32_TMR_COUNT(kd->_config.tim.tim) = 0;
    R8_TMR_INT_FLAG(kd->_config.tim.tim) = 0xFF;
    R8_TMR_CTRL_MOD(kd->_config.tim.tim) &= ~RB_TMR_ALL_CLEAR;
    R8_TMR_CTRL_MOD(kd->_config.tim.tim) &= ~RB_TMR_COUNT_EN;
}

static void uart_byteTrigCfg(kduart_t *kd, UARTByteTRIGTypeDef b) {
    R8_UART_FCR(kd->_config.uart.uart) = (R8_UART_FCR(kd->_config.uart.uart) & ~RB_FCR_FIFO_TRIG) | (b << 6);
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

    kd->_instance.enableFunc(kd);

    R8_UART_IER(kd->_config.uart.uart) = RB_IER_RESET;

    uart_baudRateCfg(kd, kd->_config.uart.init.baudRate);
    R8_UART_FCR(kd->_config.uart.uart) |= (2 << 6) | RB_FCR_TX_FIFO_CLR | RB_FCR_RX_FIFO_CLR | RB_FCR_FIFO_EN;
    R8_UART_LCR(kd->_config.uart.uart) = RB_LCR_WORD_SZ;
    if (kd->_config.uart.init.parity) {
        R8_UART_LCR(kd->_config.uart.uart) &= ~RB_LCR_PAR_MOD;
        R8_UART_LCR(kd->_config.uart.uart) |= FIELD_PREP(RB_LCR_PAR_MOD, (uint32_t) kd->_config.uart.init.parity - 1);
        R8_UART_LCR(kd->_config.uart.uart) |= RB_LCR_PAR_EN;
    } else {
        R8_UART_LCR(kd->_config.uart.uart) &= ~RB_LCR_PAR_MOD;
        R8_UART_LCR(kd->_config.uart.uart) &= ~RB_LCR_PAR_EN;
    }

    R8_UART_LCR(kd->_config.uart.uart) &= ~RB_LCR_STOP_BIT;
    R8_UART_LCR(kd->_config.uart.uart) |= FIELD_PREP(RB_LCR_STOP_BIT, (uint32_t) kd->_config.uart.init.stop);

    R8_UART_LCR(kd->_config.uart.uart) &= ~RB_LCR_WORD_SZ;
    R8_UART_LCR(kd->_config.uart.uart) |= FIELD_PREP(RB_LCR_WORD_SZ, (uint32_t) kd->_config.uart.init.wordLength);

    if (kd->_config.pin.tx.gpio != NULL) {
        R8_UART_IER(kd->_config.uart.uart) = RB_IER_TXD_EN;
    } else {
        R8_UART_IER(kd->_config.uart.uart) = 0;
    }

    R8_UART_IER(kd->_config.uart.uart) |= RB_IER_RECV_RDY;
    R8_UART_DIV(kd->_config.uart.uart) = 1;
    uart_byteTrigCfg(kd, UART_4BYTE_TRIG);

    if (kd->_config.tim.tim != 0) {
        tmr_reset(kd);
        tmr_set(kd, kd->_config.uart.init.baudRate, kd->_config.tim.timeout);
    }

    kd->_va->flag._ = 0;
    kd->_va->flag.isSendCompleted = 1;

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

    if (kd->_va->initRefsPower != 0) {
        kd->_va->initRefsPower = 1;
        kduart_powerDown(kd);
    }

    if (kd->_config.tim.tim != 0) {
        tmr_reset(kd);
    }

    R8_UART_IER(kd->_config.uart.uart) = 0;

    kd->_instance.disableFunc(kd);

    kd->_va->flag._ = 0;

    return 0;
}


int32_t kduart_powerUp(kduart_t *kd) {
    ASSERT(kd != NULL);

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
        _gpio_afConfig(kd->_config.pin.tx.gpio->_config.base.port,
            kd->_config.pin.tx.gpio->_config.base.pin, 0);
    }

    if (kd->_config.pin.rx.gpio != NULL) {
        _gpio_modeConfig(kd->_config.pin.rx.gpio->_config.base.port,
            kd->_config.pin.rx.gpio->_config.base.pin,
            KDGPIO_MODE_INPUT);
        _gpio_pullConfig(kd->_config.pin.rx.gpio->_config.base.port,
            kd->_config.pin.rx.gpio->_config.base.pin,
            KDGPIO_PULL_NONE);
        _gpio_afConfig(kd->_config.pin.rx.gpio->_config.base.port,
            kd->_config.pin.rx.gpio->_config.base.pin, 0);
    }

    if (kd->buffer.recvLwrb != NULL) {
        qBSBuffer_Setup(kd->buffer.recvLwrb, kd->buffer.recvBuffer, kd->buffer.recvBufferSize);
    }

    if (kd->_config.tim.tim != 0) {
        tmr_reset(kd);
    }

    (void) R8_UART_IIR(kd->_config.uart.uart);
    R8_UART_MCR(kd->_config.uart.uart) |= RB_MCR_INT_OE;

    return 0;
}


int32_t kduart_powerDown(kduart_t *kd) {
    ASSERT(kd != NULL);

    if (emmkDriver_initRefsCountDown(&kd->_va->initRefsPower) != 0) {
        return -1;
    }

    if (kd->_config.tim.tim != 0) {
        tmr_reset(kd);
    }

    R8_UART_MCR(kd->_config.uart.uart) &= ~RB_MCR_INT_OE;
    (void) R8_UART_IIR(kd->_config.uart.uart);

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

    return 0;
}


int32_t kduart_sends(kduart_t *kd, const void *data, uint32_t size, uint32_t timeout) {
    if (kd->_config.pin.txPinMode == KDUART_TX_PIN_MODE_IN_IDLE) {
        _gpio_modeConfig(kd->_config.pin.tx.gpio->_config.base.port,
            kd->_config.pin.tx.gpio->_config.base.pin,
            KDGPIO_MODE_OUTPUT_PP);
        _gpio_pullConfig(kd->_config.pin.tx.gpio->_config.base.port,
            kd->_config.pin.tx.gpio->_config.base.pin,
            KDGPIO_PULL_NONE);
        _gpio_afConfig(kd->_config.pin.tx.gpio->_config.base.port,
            kd->_config.pin.tx.gpio->_config.base.pin, 0);
    }

    uint8_t *u8Data = (void *) data;
    uint32_t wLen = size;

    while (wLen) {
        if (R8_UART_TFC(kd->_config.uart.uart) != UART_FIFO_SIZE) {
            R8_UART_THR(kd->_config.uart.uart) = *u8Data++;
            wLen--;
        }
    }

    if (kd->_config.pin.txPinMode == KDUART_TX_PIN_MODE_IN_IDLE) {
        _gpio_afConfig(kd->_config.pin.tx.gpio->_config.base.port,
            kd->_config.pin.tx.gpio->_config.base.number,
            0);
        kdgpio_powerDown(kd->_config.pin.tx.gpio);
    }

    qSTimer_t wait;
    qSTimer_Set(&wait, 1000);
    while (!qSTimer_Expired(&wait)) {
        if (R8_UART_TFC(kd->_config.uart.uart) == 0) {
            break;
        }
    }

    return (int32_t) size;
}


int32_t kduart_recvs(kduart_t *kd, void *data, uint32_t expect_size, uint32_t *recv_size, uint32_t timeout) {
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
}


int32_t kduart_flush(kduart_t *kd) {
    if (kd->buffer.recvLwrb != NULL) {
        qBSBuffer_Setup(kd->buffer.recvLwrb, kd->buffer.recvBuffer, kd->buffer.recvBufferSize);
    }

    (void) R8_UART_IIR(kd->_config.uart.uart);
    tmr_reset(kd);

    kd->_va->flag.isSendCompleted = 0;
    kd->_va->flag.isRecving = 0;
    kd->_va->flag.isRecvCompleted = 0;

    return 0;
}


int32_t kduart_hasRecvData(kduart_t *kd) {
    if ((kd->_va->flag.isRecvCompleted) && !kd->_va->flag.isRecving) {
        return qBSBuffer_Count(kd->buffer.recvLwrb);
    }
    return 0;
}


int32_t kduart_isSendIdle(kduart_t *kd) {
    return kd->_va->flag.isSendCompleted;
}


void kduart_updateBaudRate(kduart_t *kd, uint32_t bd) {
    uart_baudRateCfg(kd, bd);
    if (kd->_config.tim.tim != 0) {
        tmr_reset(kd);
        tmr_set(kd, bd, kd->_config.tim.timeout);
    }
}

/*@}*/
