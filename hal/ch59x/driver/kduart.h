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

#include "CH59x_uart.h"
#include "CH59x_gpio.h"
#include "CH59x_timer.h"

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
            uint32_t uart;
            struct {
                uint32_t baudRate;
                uint8_t stop;
                uint8_t parity;
                uint8_t wordLength;
            } init;
        } uart;

        struct {
            uint32_t tim;
            uint32_t timeout;
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

#define _KDUART_NGET(x)  __kduart_##x
#define _KDUART_NEXTERN(x)  extern const kduart_t __kduart_##x
#define KDUART_NGET(x)  _KDUART_NGET(x)
#define KDUART_NEXTERN(x)  _KDUART_NEXTERN(x)

#define KDUART_LCR_PARITY_NONE    0
#define KDUART_LCR_PARITY_ODD     1
#define KDUART_LCR_PARITY_EVEN    2
#define KDUART_LCR_PARITY_MARK    3
#define KDUART_LCR_PARITY_SPACE   4

#define KDUART_LCR_STOP_BIT_1     0
#define KDUART_LCR_STOP_BIT_2     1

#define KDUART_LCR_WORD_LEN5      0
#define KDUART_LCR_WORD_LEN6      1
#define KDUART_LCR_WORD_LEN7      2
#define KDUART_LCR_WORD_LEN8      3

#define KDUART_MODULE(_number) _number
#define _KDUART_MODULE(_number) BA_UART##_number

#define KDUART_TIM_MODULE(_number) _number
#define _KDUART_TIM_MODULE(_number) BA_TMR##_number

#define _KDUART_IRQN(x)  UART##x##_IRQn

#define _KDUART_INAME(x)            __kduart_##x
#define _KDUART_IVA(x)              __kduart_va_##x
#define _KDUART_FUNC_ENABLE(x)      __kduart_enable_##x
#define _KDUART_FUNC_DISABLE(x)     __kduart_disable_##x

#define KDUART_BAUDRATE(x)          x
#define KDUART_TIMEOUT_CHAR(x)      x

#define KDUART_RX_BUFFER(x) x
#define _KDUART_RX_BUFFER(x)        __kduart_rx_buffer_##x
#define _KDUART_RX_LWRB(x)          __kduart_rx_lwrb_##x

#define KDUART_TX_BUFFER(x) x
#define _KDUART_TX_BUFFER(x)  __kduart_tx_buffer_##x

#define KDUART_TX_DEFINE(_gpio) \
    .tx = { \
        .gpio = ((kdgpio_t *) _gpio), \
    }

#define KDUART_RX_DEFINE(_gpio) \
    .rx = { \
        .gpio = ((kdgpio_t *) _gpio), \
    }

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

#define KDUART_RTO_DEFINE(_uartNumber, _name, \
        _tim, \
        _baudrate, _rto, \
        _txBufferSize, _rxBufferSize, \
        _wordLength, _parity, _stopBits, \
        _tx, _txPinMode, _rx, \
        _enableFunc, _disableFunc, \
        _irqUART, _irqITM) \
    static void _KDUART_FUNC_ENABLE(_name)(kduart_t *kd) _enableFunc \
    static void _KDUART_FUNC_DISABLE(_name)(kduart_t *kd) _disableFunc \
    static kduart_VA_t _KDUART_IVA(_name) = {0}; \
    static qBSBuffer_t _KDUART_RX_LWRB(_name); \
    static uint8_t AT_NONCACHEABLE_SECTION_ALIGN(_KDUART_RX_BUFFER(_name)[_rxBufferSize], 4); \
    const kduart_t _KDUART_INAME(_name) = { \
    ._va = &_KDUART_IVA(_name), \
        ._config = { \
            .uart = { \
                .uart = (uint32_t) _KDUART_MODULE(_uartNumber), \
                .init = { \
                    .baudRate = _baudrate, \
                    .parity = _parity, \
                    .stop = _stopBits, \
                    .wordLength = _wordLength, \
                }, \
            }, \
            .tim = { \
                .tim = 0, \
                .timeout = _rto, \
            }, \
            .pin = { \
                _tx, _rx, \
                .txPinMode = _txPinMode, \
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
    __INTERRUPT void _irqUART(void) { \
        volatile uint32_t flag = (uint32_t) R8_UART_IIR(_KDUART_INAME(_name)._config.uart.uart) & RB_IIR_INT_MASK;\
        if (flag == UART_II_RECV_RDY) {          \
            _KDUART_INAME(_name)._va->flag.isRecving = 1; \
            _KDUART_INAME(_name)._va->flag.isRecvCompleted = 0;                                             \
            uint8_t count = R8_UART_RFC(_KDUART_INAME(_name)._config.uart.uart);                                        \
            for (volatile uint32_t i = 0; i < count; i++) { \
                if (R8_UART_LSR(_KDUART_INAME(_name)._config.uart.uart) & RB_LSR_DATA_RDY) { \
                    uint8_t r = R8_UART_RBR(_KDUART_INAME(_name)._config.uart.uart);                   \
                    qBSBuffer_Put(_KDUART_INAME(_name).buffer.recvLwrb, r); \
                } else {                                \
                    break;                                        \
                }                               \
            }                                     \
        } else if (flag == UART_II_RECV_TOUT) {   \
            _KDUART_INAME(_name)._va->flag.isRecving = 1; \
            _KDUART_INAME(_name)._va->flag.isRecvCompleted = 0;                                             \
            uint8_t count = R8_UART_RFC(_KDUART_INAME(_name)._config.uart.uart);                                        \
            for (volatile uint32_t i = 0; i < count; i++) { \
                if (R8_UART_LSR(_KDUART_INAME(_name)._config.uart.uart) & RB_LSR_DATA_RDY) { \
                    uint8_t r = R8_UART_RBR(_KDUART_INAME(_name)._config.uart.uart);                   \
                    qBSBuffer_Put(_KDUART_INAME(_name).buffer.recvLwrb, r); \
                } else {                                \
                    break;                                        \
                }                               \
            }                                     \
            _KDUART_INAME(_name)._va->flag.isRecving = 0; \
            _KDUART_INAME(_name)._va->flag.isRecvCompleted = 1;                                    \
        } else {                                  \
            R8_UART_LSR(_KDUART_INAME(_name)._config.uart.uart);                 \
            R8_UART_RBR(_KDUART_INAME(_name)._config.uart.uart);                     \
            R8_UART_MCR(_KDUART_INAME(_name)._config.uart.uart);                                          \
        }                                         \
    }

#define KDUART_TMR_DEFINE(_uartNumber, _name, \
        _tim, \
        _baudrate, _rto, \
        _txBufferSize, _rxBufferSize, \
        _wordLength, _parity, _stopBits, \
        _tx, _txPinMode, _rx, \
        _enableFunc, _disableFunc, \
        _irqUART, _irqITM) \
    static void _KDUART_FUNC_ENABLE(_name)(kduart_t *kd) _enableFunc \
    static void _KDUART_FUNC_DISABLE(_name)(kduart_t *kd) _disableFunc \
    static kduart_VA_t _KDUART_IVA(_name) = {0}; \
    static qBSBuffer_t _KDUART_RX_LWRB(_name); \
    static uint8_t AT_NONCACHEABLE_SECTION_ALIGN(_KDUART_RX_BUFFER(_name)[_rxBufferSize], 4); \
    const kduart_t _KDUART_INAME(_name) = { \
        ._va = &_KDUART_IVA(_name), \
        ._config = { \
            .uart = { \
                .uart = (uint32_t) _KDUART_MODULE(_uartNumber), \
                .init = { \
                    .baudRate = _baudrate, \
                    .parity = _parity, \
                    .stop = _stopBits, \
                    .wordLength = _wordLength, \
                }, \
            }, \
            .tim = { \
                .tim = (uint32_t) _KDUART_TIM_MODULE(_tim), \
                .timeout = _rto, \
            }, \
            .pin = { \
                _tx, _rx, \
                .txPinMode = _txPinMode, \
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
    __INTERRUPT void _irqUART(void) { \
        volatile uint32_t flag = (uint32_t) R8_UART_IIR(_KDUART_INAME(_name)._config.uart.uart) & RB_IIR_INT_MASK;\
        if (flag == UART_II_RECV_RDY) {          \
            _KDUART_INAME(_name)._va->flag.isRecving = 1; \
            _KDUART_INAME(_name)._va->flag.isRecvCompleted = 0;                                             \
            uint8_t count = R8_UART_RFC(_KDUART_INAME(_name)._config.uart.uart);                                        \
            for (volatile uint32_t i = 0; i < count; i++) { \
                if (R8_UART_LSR(_KDUART_INAME(_name)._config.uart.uart) & RB_LSR_DATA_RDY) { \
                    uint8_t r = R8_UART_RBR(_KDUART_INAME(_name)._config.uart.uart);                   \
                    qBSBuffer_Put(_KDUART_INAME(_name).buffer.recvLwrb, r); \
                } else {                                \
                    break;                                        \
                }                               \
            }                                     \
             if (_KDUART_INAME(_name)._config.tim.tim != 0) {                                      \
                R32_TMR_COUNT(_KDUART_INAME(_name)._config.tim.tim) = 0;             \
                *((__IO uint8_t *) (_KDUART_INAME(_name)._config.tim.tim + TMR_INT_FLAG)) |= RB_TMR_IF_CYC_END;                             \
                *((__IO uint8_t *) (_KDUART_INAME(_name)._config.tim.tim + TMR_CTRL_MOD)) |= RB_TMR_COUNT_EN;      \
            }                                          \
        } else if (flag == UART_II_RECV_TOUT) {   \
            _KDUART_INAME(_name)._va->flag.isRecving = 1; \
            _KDUART_INAME(_name)._va->flag.isRecvCompleted = 0;                                             \
            uint8_t count = R8_UART_RFC(_KDUART_INAME(_name)._config.uart.uart);                                        \
            for (volatile uint32_t i = 0; i < count; i++) { \
                if (R8_UART_LSR(_KDUART_INAME(_name)._config.uart.uart) & RB_LSR_DATA_RDY) { \
                    uint8_t r = R8_UART_RBR(_KDUART_INAME(_name)._config.uart.uart);                   \
                    qBSBuffer_Put(_KDUART_INAME(_name).buffer.recvLwrb, r); \
                } else {                                \
                    break;                                        \
                }                               \
            }                                     \
            if (_KDUART_INAME(_name)._config.tim.tim != 0) {                                      \
                R32_TMR_COUNT(_KDUART_INAME(_name)._config.tim.tim) = 0;             \
                *((__IO uint8_t *) (_KDUART_INAME(_name)._config.tim.tim + TMR_INT_FLAG)) |= RB_TMR_IF_CYC_END;                             \
                *((__IO uint8_t *) (_KDUART_INAME(_name)._config.tim.tim + TMR_CTRL_MOD)) |= RB_TMR_COUNT_EN;      \
            } else {                              \
                _KDUART_INAME(_name)._va->flag.isRecving = 0; \
                _KDUART_INAME(_name)._va->flag.isRecvCompleted = 1;                   \
            }                                            \
        } else {                                  \
            R8_UART_LSR(_KDUART_INAME(_name)._config.uart.uart);                 \
            R8_UART_RBR(_KDUART_INAME(_name)._config.uart.uart);                     \
            R8_UART_MCR(_KDUART_INAME(_name)._config.uart.uart);                                          \
        }                                         \
    } \
    __INTERRUPT void _irqITM(void) {              \
        *((__IO uint8_t *) (_KDUART_INAME(_name)._config.tim.tim + TMR_INT_FLAG)) |= RB_TMR_IF_CYC_END;                             \
        *((__IO uint8_t *) (_KDUART_INAME(_name)._config.tim.tim + TMR_CTRL_MOD)) &= ~RB_TMR_COUNT_EN;       \
        _KDUART_INAME(_name)._va->flag.isRecving = 0; \
        _KDUART_INAME(_name)._va->flag.isRecvCompleted = 1;                   \
    }

/*@}*/

#endif
