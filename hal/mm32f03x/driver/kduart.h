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

#include "HAL_uart.h"
#include "HAL_gpio.h"
#include "HAL_tim.h"

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
            kduart_Pin_t rx;
        } pin;
        
        struct {
            UART_TypeDef *uart;
            UART_InitTypeDef init;
        } uart;
        
        struct {
            TIM_TypeDef *tim;
            uint32_t timerClock;
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

#define KDUART_PIN_AF(_af)       _af
#define _KDUART_PIN_AF(_af)      _af

#define KDUART_BAUDRATE(x)          x
#define KDUART_TIMEOUT_CHAR(x)      x

#define KDUART_UART(_number)    _number
#define _KDUART_UART(_number)   UART##_number

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

#define KDUART_TX_BUFFER(x) x
#define KDUART_RX_BUFFER(x) x
#define KDUART_BAUDRATE(x) x
#define KDUART_TIM_CLOCK_SRC(x) x
#define KDUART_TIMEOUT_CHAR(x) x
    
#define _KDUART_PATH(x) "tty/"#x
    
#define KDUART_MODULE(_number) _number
#define _KDUART_MODULE(_number) UART##_number

#define KDUART_TIM_MODULE(_number) _number
#define _KDUART_TIM_MODULE(_number) TIM##_number

#define _KDUART(x)  __kduart_##x
#define _KDUART_INAME(x)  __kduart_##x
#define _KDUART_VA(x)  __kduart_va_##x
#define _KDUART_FUNC_ENABLE(x)  __kduart_enable_##x
#define _KDUART_FUNC_DISABLE(x)  __kduart_disable_##x
#define _KDUART_RX_BUFFER(x)  __kduart_rx_buffer_##x
#define _KDUART_TX_BUFFER(x)  __kduart_tx_buffer_##x
#define _KDUART_RX_LWRB(x)  __kduart_rx_lwrb_##x
    

#define KDUART_ISR(name) \
    if ((_KDUART_INAME(name)._config.uart.uart->ISR & UART_ISR_RX_INTF) \
            && _KDUART_INAME(name)._config.tim.tim != NULL) { \
        _KDUART_INAME(name)._config.uart.uart->ICR |= UART_ICR_RXICLR; \
        uint8_t data = _KDUART_INAME(name)._config.uart.uart->RDR; \
        qBSBuffer_Put(_KDUART_INAME(name).buffer.recvLwrb, data); \
        _KDUART_INAME(name)._config.tim.tim->CR1 &= ~TIM_CR1_CEN; \
        _KDUART_INAME(name)._config.tim.tim->SR &= ~TIM_SR_UIF; \
        _KDUART_INAME(name)._config.tim.tim->CNT = 0; \
        _KDUART_INAME(name)._config.tim.tim->CR1 |= TIM_CR1_CEN; \
    } else { \
        _KDUART_INAME(name)._config.uart.uart->ICR |= \
            UART_ICR_TXICLR | UART_ICR_RXICLR | UART_ICR_TIMEOUTCLR | UART_ICR_RXOERRCLR | UART_ICR_RXPERRCLR | UART_ICR_RXFERRCLR | UART_ICR_RXBRKCLR; \
        (void) _KDUART_INAME(name)._config.uart.uart->RDR; \
    }

#define KDUART_TIMER_ISR(name) \
    _KDUART_INAME(name)._config.tim.tim->CR1 &= ~TIM_CR1_CEN; \
    _KDUART_INAME(name)._config.tim.tim->SR &= ~TIM_SR_UIF; \
    _KDUART_INAME(name)._config.tim.tim->CNT  = 0; \
    _KDUART_INAME(name)._va->flag.isRecving = 0; \
    _KDUART_INAME(name)._va->flag.isRecvCompleted = 1; \

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

    
#define KDUART_DEFINE_IT_TIMER_BLOCK(_uartModuleNumber, _name, \
        _timModuleNumber, \
        _enableFunc, _disableFunc, \
        _txBufferSize, _rxBufferSize, \
        _baudRate, _timerClock, _timeout, \
        _dataWidth, _parity, _stopBits, \
        _tx, _rx, \
        _uartIrq, _timerIrq) \
    static void _KDUART_FUNC_ENABLE(_name)(kduart_t *kd) _enableFunc \
    static void _KDUART_FUNC_DISABLE(_name)(kduart_t *kd) _disableFunc \
    static kduart_VA_t _KDUART_VA(_name) = {0}; \
    static qBSBuffer_t _KDUART_RX_LWRB(_name); \
    static uint8_t AT_NONCACHEABLE_SECTION_ALIGN(_KDUART_RX_BUFFER(_name)[_rxBufferSize], 4); \
    const kduart_t _KDUART_INAME(_name) = { \
        ._va = &_KDUART_VA(_name), \
        ._config = { \
            .rto = _timeout, \
            .uart = { \
                .uart = _KDUART_MODULE(_uartModuleNumber), \
                .init = { \
                    .UART_BaudRate = _baudRate, \
                    .UART_WordLength = _dataWidth, \
                    .UART_Parity = _parity, \
                    .UART_StopBits = _stopBits, \
                    .UART_Mode = UART_Mode_Tx | UART_Mode_Rx, \
                    .UART_HardwareFlowControl = UART_HardwareFlowControl_None, \
                }, \
            }, \
            .tim = { \
                .tim = _KDUART_TIM_MODULE(_timModuleNumber), \
                .timerClock = _timerClock, \
            }, \
            .pin = { \
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
    void _timerIrq(void) { \
       KDUART_TIMER_ISR(_name); \
    } 
    
/*@}*/

#endif
