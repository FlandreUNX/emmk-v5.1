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
#include "kdgpio.h"

#include "py32f0xx_hal.h"
#include "gd32e10x_usart.h"
#include "gd32e10x_dma.h"
#include "gd32e10x_timer.h"

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
            uint32_t uart;
            struct {
                uint32_t baudRate;
                uint16_t stop;
                uint16_t parity;
                uint8_t dataLen;
            } init;
        } uart;
        
        struct {
            uint32_t timer;
            uint32_t timerClock;
        } timer;
        
        struct {
            uint32_t dmaTx;
            uint32_t dmaRx;
            dma_channel_enum channelTx;
            dma_channel_enum channelRx;
            dma_parameter_struct initTx;
            dma_parameter_struct initRx;
        } dma;
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
#define KDUART_NEXTERN(x)    _KDUART_NEXTERN(x)
#define KDUART_NGET(x)    _KDUART_NGET(x)

#define KDUART_MODULE(_number)      _number
#define _KDUART_MODULE(_number)     USART##_number

#define KDUART_(_number)      _number

#define KDUART_TIM_MODULE(_number)      _number
#define _KDUART_TIM_MODULE(_number)     TIM##_number

#define KDUART_DMA_TX_MODULE(_number)       _number
#define KDUART_DMA_RX_MODULE(_number)       _number
#define _KDUART_DMA_MODULE(_number)         (uint32_t) DMA##_number

#define KDUART_DMA_TX_STREAM(_number)       _number
#define KDUART_DMA_RX_STREAM(_number)       _number

#define KDUART_BAUDRATE(x)          x
#define KDUART_TIMEOUT_CHAR(x)      x
#define KDUART_TIM_CLOCK_SRC(x) x

#define _KDUART_INAME(x)            __kduart_##x
#define _KDUART_IVA(x)              __kduart_va_##x
#define _KDUART_FUNC_ENABLE(x)      __kduart_enable_##x
#define _KDUART_FUNC_DISABLE(x)     __kduart_disable_##x

#define KDUART_RX_BUFFER(x) x
#define _KDUART_RX_BUFFER(x)        __kduart_rx_buffer_##x
#define _KDUART_RX_LWRB(x)          __kduart_rx_lwrb_##x
        
#define KDUART_TX_BUFFER(x) x
#define _KDUART_TX_BUFFER(x)  __kduart_tx_buffer_##x

#define KDUART_GPIO_AF(x) x

#define KDUART_TX_DEFINE(_gpio, _af) \
    .tx = { \
        .gpio = ((kdgpio_t *) _gpio), \
        .af = _af, \
    }
    
#define KDUART_RX_DEFINE(_gpio, _af) \
    .rx = { \
        .gpio = ((kdgpio_t *) _gpio), \
        .af = _af, \
    }

#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
#define KDUART_ISR(name) \
    if (usart_interrupt_flag_get(_KDUART_INAME(name)._config.uart.uart, USART_INT_FLAG_RT)) { \
        usart_interrupt_flag_clear(_KDUART_INAME(name)._config.uart.uart, USART_INT_FLAG_RT); \
        osEventFlagsSet(_KDUART_INAME(name)._va->flag, UART_FLAG_RECV_COMPLETE); \
    } else if (usart_interrupt_flag_get(_KDUART_INAME(name)._config.uart.uart, USART_INT_FLAG_RBNE) == SET) { \
        uint8_t data = usart_data_receive(_KDUART_INAME(name)._config.uart.uart); \
        qBSBuffer_Put(_KDUART_INAME(name).buffer.recvLwrb, data); \
        if (_KDUART_INAME(name)._config.timer.timer != NULL) { \
            TIMER_CNT(_KDUART_INAME(name)._config.timer.timer) = 0; \
            TIMER_CTL0(_KDUART_INAME(name)._config.timer.timer) |= (uint32_t) TIMER_CTL0_CEN; \
        } \
        osEventFlagsClear(_KDUART_INAME(name)._va->flag, UART_FLAG_RECV_COMPLETE); \
    } else { \
        usart_interrupt_flag_clear(_KDUART_INAME(name)._config.uart.uart, USART_INT_FLAG_ERR_NERR); \
        usart_interrupt_flag_clear(_KDUART_INAME(name)._config.uart.uart, USART_INT_FLAG_ERR_FERR); \
        usart_interrupt_flag_clear(_KDUART_INAME(name)._config.uart.uart, USART_INT_FLAG_ERR_ORERR); \
        usart_data_receive(_KDUART_INAME(name)._config.uart.uart); \
    }
    
#define KDUART_DMA_TX_ISR(name) \
    if (dma_interrupt_flag_get(_KDUART_INAME(name)._config.dma.dmaTx, _KDUART_INAME(name)._config.dma.channelTx, DMA_FLAG_FTF)) { \
        dma_interrupt_flag_clear(_KDUART_INAME(name)._config.dma.dmaTx, _KDUART_INAME(name)._config.dma.channelTx, DMA_INT_FLAG_FTF); \
        dma_interrupt_flag_clear(_KDUART_INAME(name)._config.dma.dmaTx, _KDUART_INAME(name)._config.dma.channelTx, DMA_INT_FLAG_ERR); \
        dma_channel_disable(_KDUART_INAME(name)._config.dma.dmaTx, _KDUART_INAME(name)._config.dma.channelTx); \
        osEventFlagsSet(_KDUART_INAME(name)._va->flag, UART_FLAG_SEND_DMA_COMPLETE | UART_FLAG_SEND_IDLE); \
    }

#define KDUART_TIMER_ISR(name) \
    TIMER_CTL0(_KDUART_INAME(name)._config.timer.timer) &= (~(uint32_t) TIMER_CTL0_CEN); \
    osEventFlagsSet(_KDUART_INAME(name)._va->flag, UART_FLAG_RECV_COMPLETE); \
    TIMER_INTF(_KDUART_INAME(name)._config.timer.timer) = (~(uint32_t) TIMER_INT_FLAG_UP);
    
#else

#define KDUART_ISR(name) \
    if (usart_interrupt_flag_get(_KDUART_INAME(name)._config.uart.uart, USART_INT_FLAG_RT) == SET) { \
        usart_interrupt_flag_clear(_KDUART_INAME(name)._config.uart.uart, USART_INT_FLAG_RT); \
        _KDUART_INAME(name)._va->flag.isRecving = 0; \
        _KDUART_INAME(name)._va->flag.isRecvCompleted = 1; \
    } else if (usart_interrupt_flag_get(_KDUART_INAME(name)._config.uart.uart, USART_INT_FLAG_RBNE) == SET) { \
        uint8_t data = usart_data_receive(_KDUART_INAME(name)._config.uart.uart); \
        qBSBuffer_Put(_KDUART_INAME(name).buffer.recvLwrb, data); \
        if (_KDUART_INAME(name)._config.timer.timer != NULL) { \
            TIMER_CNT(_KDUART_INAME(name)._config.timer.timer) = 0; \
            TIMER_CTL0(_KDUART_INAME(name)._config.timer.timer) |= (uint32_t) TIMER_CTL0_CEN; \
        } \
    } else { \
        usart_interrupt_flag_clear(_KDUART_INAME(name)._config.uart.uart, USART_INT_FLAG_ERR_NERR); \
        usart_interrupt_flag_clear(_KDUART_INAME(name)._config.uart.uart, USART_INT_FLAG_ERR_FERR); \
        usart_interrupt_flag_clear(_KDUART_INAME(name)._config.uart.uart, USART_INT_FLAG_ERR_ORERR); \
        usart_data_receive(_KDUART_INAME(name)._config.uart.uart); \
    }
    
#define KDUART_DMA_TX_ISR(name) \
    if (dma_interrupt_flag_get(_KDUART_INAME(name)._config.dma.dmaTx, _KDUART_INAME(name)._config.dma.channelTx, DMA_FLAG_FTF)) { \
        dma_interrupt_flag_clear(_KDUART_INAME(name)._config.dma.dmaTx, _KDUART_INAME(name)._config.dma.channelTx, DMA_INT_FLAG_FTF); \
        dma_interrupt_flag_clear(_KDUART_INAME(name)._config.dma.dmaTx, _KDUART_INAME(name)._config.dma.channelTx, DMA_INT_FLAG_ERR); \
        dma_channel_disable(_KDUART_INAME(name)._config.dma.dmaTx, _KDUART_INAME(name)._config.dma.channelTx); \
        _KDUART_INAME(name)._va->flag.isSendCompleted = 1; \
    }
    
#define KDUART_TIMER_ISR(name) \
    TIMER_CTL0(_KDUART_INAME(name)._config.timer.timer) &= (~(uint32_t) TIMER_CTL0_CEN); \
    _KDUART_INAME(name)._va->flag.isRecving = 0; \
    _KDUART_INAME(name)._va->flag.isRecvCompleted = 1; \
    TIMER_INTF(_KDUART_INAME(name)._config.timer.timer) = (~(uint32_t) TIMER_INT_FLAG_UP);

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
    
#define KDUART_DEFINE_IT_TIMER_BLOCK(_uartNumber, _name, \
        _timModuleNumber, \
        _dmaTxModuleNumber, _dmaTxChannel, \
        _dmaRxModuleNumber, _dmaRxChannel, \
        _txBufferSize, _rxBufferSize, \
        _baudRate, _timerClock, _rto, \
        _dataWidth, _parity, _stopBits, \
        _tx, _rx, \
        _enableFunc, _disableFunc, \
        _uartIrq, _dmaTxIrq, _timerIrq) \
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
                .uart = (uint32_t) _KDUART_MODULE(_uartNumber), \
                .init = { \
                    .dataLen = _dataWidth, \
                    .baudRate = _baudRate, \
                    .parity = _parity, \
                    .stop = _stopBits, \
                }, \
            }, \
            .timer = { \
                .timer = (uint32_t) _KDUART_TIM_MODULE(_timModuleNumber), \
                .timerClock = _timerClock, \
            }, \
            .dma = { \
                .dmaTx = _dmaTxModuleNumber != 0xFF ? _KDUART_DMA_MODULE(_dmaTxModuleNumber): 0, \
                .dmaRx = _dmaRxModuleNumber != 0xFF ? _KDUART_DMA_MODULE(_dmaRxModuleNumber): 0, \
                .channelTx = _dmaTxChannel, \
                .channelRx = _dmaRxChannel, \
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
    void _dmaTxIrq(void) { \
        KDUART_DMA_TX_ISR(_name); \
    } \
    void _timerIrq(void) { \
        KDUART_TIMER_ISR(_name); \
    }
    
/*@}*/

#endif
