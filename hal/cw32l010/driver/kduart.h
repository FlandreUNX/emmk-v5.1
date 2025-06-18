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
 
#ifndef _UART_SOC_H_
#define _UART_SOC_H_

#include <stdint.h>

#include "emmk-config.h"

#include "./kdgpio.h"

#include "cw32l010.h"
#include "cw32l010_gpio.h"
#include "cw32l010_atim.h"
#include "cw32l010_gtim.h"
#include "cw32l010_uart.h"

/**
 * @addtogroup Define
 * @note none
 */
 
/*@{*/

#define UART_FLAG_SEND_DMA_COMPLETE       0x00000001u
#define UART_FLAG_SEND_IDLE               0x00000002u
#define UART_FLAG_RECV_COMPLETE           0x00000004u
#define UART_FLAG_SEND_REQUEST            0x00000008u

/*@}*/

/**
 * @addtogroup Typedef
 * @note none
 */
 
/*@{*/

typedef struct kduart kduart_t;

typedef enum {
    KDUART_TX_PIN_MODE_AF_IDLE = 1,
    KDUART_TX_PIN_MODE_IN_IDLE = 2,
} kduart_TxPinMode_t;

typedef struct {
    uint8_t af;
    kdgpio_t *gpio;
} kduart_Pin_t;

typedef struct {
#if EMMK_FULL_RTOS_SUPPORT > EMMK_FULL_RTOS_QTS
    void *flag;
   
    void *recvMutex;
    void *writeMutex;
#else
    union {
        struct {
            uint8_t isSendCompleted: 1;
            uint8_t isRecving: 1;
            uint8_t isRecvCompleted: 1;
        };
        uint8_t _;
    } volatile flag;
#endif
    
    int8_t initRefs;
    int8_t initRefsPower;
} kduart_VA_t;

struct kduart {
    kduart_VA_t *_va;

    struct {
        uint8_t rto;

        struct {
            kduart_Pin_t tx;
            kduart_TxPinMode_t txPinMode;
            
            kduart_Pin_t rx;
        } pin;
        
        struct {
            UART_TypeDef *uart;
            struct {
                uint32_t baudRate;
                uint16_t stop;
                uint16_t parity;
            } init;
            uint16_t levelInv;
            uint8_t pinInv;
        } uart;
        struct {
            BTIM_TypeDef *btim;
        } tim;
    } _config;
    
    struct {
        void (*enableFunc)(kduart_t *kd);
        void (*disableFunc)(kduart_t *kd);
    } _instance;

    struct {
        uint16_t recvBufferSize;
        uint8_t *recvBuffer;
        
        uint16_t writeBufferSize;
        uint8_t *writeBuffer;

        qBSBuffer_t *recvLwrb;
    } buffer;
};

/*@}*/

/**
 * @addtogroup Instance define
 * @note none
 */
 
/*@{*/

#define _KDUART_NGET(x)      __kduart_##x
#define _KDUART_NEXTERN(x)   extern const kduart_t __kduart_##x
#define KDUART_NGET(x)       _KDUART_NGET(x)
#define KDUART_NEXTERN(x)    _KDUART_NEXTERN(x)

#define KDUART_PIN_AF(_af)      _af
#define _KDUART_PIN_AF(_af)     _af

#define KDUART_UART(_number)    _number
#define _KDUART_UART(_number)   CW_UART##_number

#define KDUART_BTIM(_number)    _number
#define _KDUART_BTIM(_number)   CW_BTIM##_number

#define KDUART_BAUDRATE(x)          x
#define KDUART_TIMEOUT_CHAR(x)      x

#define _KDUART_INAME(x)            __kduart_##x
#define _KDUART_IVA(x)              __kduart_va_##x
#define _KDUART_FUNC_ENABLE(x)      __kduart_enable_##x
#define _KDUART_FUNC_DISABLE(x)     __kduart_disable_##x

#define KDUART_RX_BUFFER(x) x
#define _KDUART_RX_BUFFER(x)        __kduart_rx_buffer_##x
#define _KDUART_RX_LWRB(x)          __kduart_rx_lwrb_##x
        
#define KDUART_TX_BUFFER(x) x
#define _KDUART_TX_BUFFER(x)  __kduart_tx_buffer_##x

#define KDUART_TX_DEFINE(_gpio, _af) \
    .tx = { \
        .gpio = ((kdgpio_t *) _gpio), \
        .af = _KDUART_PIN_AF(_af), \
    }
    
#define KDUART_RX_DEFINE(_gpio, _af) \
    .rx = { \
        .gpio = ((kdgpio_t *) _gpio), \
        .af = _KDUART_PIN_AF(_af), \
    }

#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
#define KDUART_ISR(name) \
    if ((KDUART_GET_INSTANCE(name)._config.uartConfig.uart->ISR & UART_ISR_RX_INTF) \
            && KDUART_GET_INSTANCE(name)._config.timerConfig.timer != NULL) { \
        KDUART_GET_INSTANCE(name)._config.uartConfig.uart->ICR |= UART_ICR_RXICLR; \
        uint8_t data = KDUART_GET_INSTANCE(name)._config.uartConfig.uart->RDR; \
        klwrb_write(KDUART_GET_INSTANCE(name).buffer.recvLwrb, &data, 1); \
        KDUART_GET_INSTANCE(name)._config.timerConfig.timer->CR1 &= ~TIM_CR1_CEN; \
        KDUART_GET_INSTANCE(name)._config.timerConfig.timer->SR &= ~TIM_SR_UIF; \
        KDUART_GET_INSTANCE(name)._config.timerConfig.timer->CNT  = 0; \
        KDUART_GET_INSTANCE(name)._config.timerConfig.timer->CR1 |= TIM_CR1_CEN; \
    } else { \
        KDUART_GET_INSTANCE(name)._config.uartConfig.uart->ICR |= \
            UART_ICR_TXICLR | UART_ICR_RXICLR | UART_ICR_TIMEOUTCLR | UART_ICR_RXOERRCLR | UART_ICR_RXPERRCLR | UART_ICR_RXFERRCLR | UART_ICR_RXBRKCLR; \
        (void) KDUART_GET_INSTANCE(name)._config.uartConfig.uart->RDR; \
    }


#define KDUART_TIMER_ISR(name) \
    KDUART_GET_INSTANCE(name)._config.timerConfig.timer->CR1 &= ~TIM_CR1_CEN; \
    KDUART_GET_INSTANCE(name)._config.timerConfig.timer->SR &= ~TIM_SR_UIF; \
    KDUART_GET_INSTANCE(name)._config.timerConfig.timer->CNT  = 0; \
    osEventFlagsSet(KDUART_GET_INSTANCE(name)._va->flag, UART_FLAG_RECV_COMPLETE); \
    
#else

#define KDUART_ISR(name) \
    uint32_t isr = _KDUART_INAME(name)._config.uart.uart->ISR; \
    if (isr & UARTx_ISR_TIMOV_Msk) { \
        _KDUART_INAME(name)._config.uart.uart->ICR &= ~(UARTx_ICR_TIMOV_Msk); \
        _KDUART_INAME(name)._config.uart.uart->TIMARR = 0; \
        REGBITS_CLR(_KDUART_INAME(name)._config.uart.uart->CR2, UARTx_CR2_TIMCR_Msk); \
        _KDUART_INAME(name)._config.uart.uart->ICR &= ~UARTx_ICR_TIMOV_Msk; \
        _KDUART_INAME(name)._va->flag.isRecving = 0; \
        _KDUART_INAME(name)._va->flag.isRecvCompleted = 1; \
    } else if (isr & UARTx_ISR_RC_Msk) { \
        _KDUART_INAME(name)._config.uart.uart->ICR &= ~(UARTx_ICR_RC_Msk); \
        _KDUART_INAME(name)._va->flag.isRecving = 1; \
        uint8_t data = _KDUART_INAME(name)._config.uart.uart->RDR & 0xFF; \
        qBSBuffer_Put(_KDUART_INAME(name).buffer.recvLwrb, data); \
        if (_KDUART_INAME(name)._config.tim.btim == NULL) { \
            if (_KDUART_INAME(name)._config.uart.uart->TIMARR == 0) { \
                _KDUART_INAME(name)._config.uart.uart->ICR &= ~UARTx_ICR_TIMOV_Msk; \
                _KDUART_INAME(name)._config.uart.uart->TIMARR = _KDUART_INAME(name)._config.rto; \
                REGBITS_MODIFY(_KDUART_INAME(name)._config.uart.uart->CR2, \
                    UARTx_CR2_TIMCR_Msk, \
                    (uint32_t) UART_TimerMode_TimeOut); \
            } \
        } else { \
            _KDUART_INAME(name)._config.tim.btim->CR1 &= ~KLBIT(0); \
            _KDUART_INAME(name)._config.tim.btim->ICR &= ~KLBIT(0); \
            _KDUART_INAME(name)._config.tim.btim->CNT = 0; \
            _KDUART_INAME(name)._config.tim.btim->CR1 |= KLBIT(0); \
        } \
    } else { \
        _KDUART_INAME(name)._config.uart.uart->ICR = 0; \
        (void) _KDUART_INAME(name)._config.uart.uart->RDR; \
    }

#endif
    
extern int32_t kduart_init(kduart_t *kd);
extern int32_t kduart_finalize(kduart_t *kd);
extern int32_t kduart_powerUp(kduart_t *kd);
extern int32_t kduart_powerDown(kduart_t *kd);
extern int32_t kduart_sends(kduart_t *kd, const void *data, uint32_t size, uint32_t timeout);
extern int32_t kduart_recvs(kduart_t *kd, void *data, uint32_t expectSize, uint32_t *recvSize, uint32_t timeout);
extern int32_t kduart_flush(kduart_t *kd);
extern int32_t kduart_hasRecvData(kduart_t *kd);
extern int32_t kduart_isSendIdle(kduart_t *kd);
extern void kduart_updateBaudRate(kduart_t *kd, uint32_t bd);
    
#define KDUART_DEFINE_RTO_BLOCK(_uartNumber, _name, \
        _btimNumber, \
        _enableFunc, _disableFunc, \
        _txBufferSize, _rxBufferSize, \
        _baudRate, _rto, \
        _parity, _stopBits, _pinInv, _levelInv, \
        _tx, _txPinMode, _rx, \
        _uartIrq, _irqITM) \
    static void _KDUART_FUNC_ENABLE(_name)(kduart_t *kd) _enableFunc \
    static void _KDUART_FUNC_DISABLE(_name)(kduart_t *kd) _disableFunc \
    static kduart_VA_t _KDUART_IVA(_name) = {0}; \
    static qBSBuffer_t _KDUART_RX_LWRB(_name); \
    static uint8_t AT_NONCACHEABLE_SECTION_ALIGN(_KDUART_RX_BUFFER(_name)[_rxBufferSize], 4); \
    const kduart_t _KDUART_INAME(_name) = { \
        ._va = &_KDUART_IVA(_name), \
        ._config = { \
            .rto = _rto, \
            .uart = { \
                .uart = _KDUART_UART(_uartNumber), \
                .init = { \
                    .baudRate = _baudRate, \
                    .parity = _parity, \
                    .stop = _stopBits, \
                }, \
                .pinInv = _pinInv, \
                .levelInv = _levelInv, \
            }, \
            .tim = { \
                .btim = NULL, \
            }, \
            .pin = { \
                .txPinMode = _txPinMode, \
                _tx, _rx, \
            }, \
        }, \
        ._instance = { \
            .enableFunc = _KDUART_FUNC_ENABLE(_name), \
            .disableFunc = _KDUART_FUNC_DISABLE(_name), \
        }, \
        .buffer = { \
            .recvBufferSize = _rxBufferSize, \
            .recvBuffer = _KDUART_RX_BUFFER(_name), \
            .writeBufferSize = 0, \
            .writeBuffer = NULL, \
            .recvLwrb = &_KDUART_RX_LWRB(_name), \
        }, \
    }; \
    void _uartIrq(void) { \
        KDUART_ISR(_name); \
    } \
    
#define KDUART_DEFINE_BTIM_BLOCK(_uartNumber, _name, \
        _btimNumber, \
        _enableFunc, _disableFunc, \
        _txBufferSize, _rxBufferSize, \
        _baudRate, _rto, \
        _parity, _stopBits, _pinInv, _levelInv, \
        _tx, _txPinMode, _rx, \
        _uartIrq, _irqITM) \
    static void _KDUART_FUNC_ENABLE(_name)(kduart_t *kd) _enableFunc \
    static void _KDUART_FUNC_DISABLE(_name)(kduart_t *kd) _disableFunc \
    static kduart_VA_t _KDUART_IVA(_name) = {0}; \
    static qBSBuffer_t _KDUART_RX_LWRB(_name); \
    static uint8_t AT_NONCACHEABLE_SECTION_ALIGN(_KDUART_RX_BUFFER(_name)[_rxBufferSize], 4); \
    const kduart_t _KDUART_INAME(_name) = { \
        ._va = &_KDUART_IVA(_name), \
        ._config = { \
            .rto = _rto, \
            .uart = { \
                .uart = _KDUART_UART(_uartNumber), \
                .init = { \
                    .baudRate = _baudRate, \
                    .parity = _parity, \
                    .stop = _stopBits, \
                }, \
                .pinInv = _pinInv, \
                .levelInv = _levelInv, \
            }, \
            .tim = { \
                .btim = _KDUART_BTIM(_btimNumber), \
            }, \
            .pin = { \
                .txPinMode = _txPinMode, \
                _tx, _rx, \
            }, \
        }, \
        ._instance = { \
            .enableFunc = _KDUART_FUNC_ENABLE(_name), \
            .disableFunc = _KDUART_FUNC_DISABLE(_name), \
        }, \
        .buffer = { \
            .recvBufferSize = _rxBufferSize, \
            .recvBuffer = _KDUART_RX_BUFFER(_name), \
            .writeBufferSize = 0, \
            .writeBuffer = NULL, \
            .recvLwrb = &_KDUART_RX_LWRB(_name), \
        }, \
    }; \
    void _uartIrq(void) { \
        KDUART_ISR(_name); \
    } \
    void _irqITM(void) { \
        _KDUART_BTIM(_btimNumber)->CR1 &= ~KLBIT(0); \
        _KDUART_BTIM(_btimNumber)->ICR &= ~KLBIT(0); \
        _KDUART_IVA(_name).flag.isRecving = 0; \
        _KDUART_IVA(_name).flag.isRecvCompleted = 1; \
    }
    
/*@}*/

#endif
