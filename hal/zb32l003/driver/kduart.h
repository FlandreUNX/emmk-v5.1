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

#ifdef CONFIG_USE_ZB32L003
#include "zb32l003.h"
#endif
#ifdef CONFIG_USE_ZB32L030
#include "zb32l030.h"
#endif

/**
 * @addtogroup Define
 * @note none
 */
 
/*@{*/

#define UART_FLAG_SEND_COMPLETE       0x00000001u
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
    KDUART_TX_PIN_MODE_AF_IDLE,
    KDUART_TX_PIN_MODE_IN_IDLE,
} kduart_TxPinMode_t;

typedef struct {
    uint8_t af;
    kdgpio_t *gpio;
} kduart_Pin_t;

typedef struct {
#if EMMK_FULL_RTOS_SUPPORT > EMMK_FULL_RTOS_QTS
    void *flag;
#else
    union {
        struct {
            uint8_t isSendIrq: 1;
            uint8_t isSendCompleted: 1;
            uint8_t isRecving: 1;
            uint8_t isRecvCompleted: 1;
        };
        uint8_t _;
    } volatile flag;
#endif
    
    int8_t initRefs;
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
            uint8_t isLp;
            union {
                UART_TypeDef *uart;
                LPUART_TypeDef *lpuart;
            };
            
            struct {
                uint32_t baudRate;
            } init;
        } uart;
        
        struct {
            uint8_t isLp;
            union {
                BASETIM_TypeDef *tim;
                LPTIM_TypeDef *lptim;
            };
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
        qBSBuffer_t *writeLwrb;
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
#define _KDUART_UART(_number)   UART##_number
#define _KDLPUART_MODULE(_number) _number##UART

#define KDUART_TIM(_number)      _number
#define _KDUART_TIM_MODULE(_number)     TIM##_number
#define _KDUART_LPTIM_MODULE(_number)   _number##TIM

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
#define _KDUART_TX_LWRB(x)          __kduart_tx_lwrb_##x

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
    
extern int32_t kduart_init(kduart_t *kd);
extern int32_t kduart_finalize(kduart_t *kd);
extern int32_t kduart_powerUp(kduart_t *kd);
extern int32_t kduart_powerDown(kduart_t *kd);
extern int32_t kduart_sends(kduart_t *kd, const void *data, uint32_t size, uint32_t timeout);
extern int32_t kduart_recvs(kduart_t *kd, void *data, uint32_t expectSize, uint32_t *recvSize, uint32_t timeout);
extern int32_t kduart_flush(kduart_t *kd);
extern int32_t kduart_hasRecvData(kduart_t *kd);
extern int32_t kduart_isSendIdle(kduart_t *kd, uint32_t wait);
extern void kduart_updateBaudRate(kduart_t *kd, uint32_t bd);
extern bool kduart_sendBuffingVerify(kduart_t *kd, uint32_t dataSize);

#define KDUART_TIM_BLOCK_DEFINE(_uartNumber, _timNumber, _name, \
        _enableFunc, _disableFunc, \
        _txBufferSize, _rxBufferSize, \
        _baudRate, _rto, \
        _tx, _txPinMode, _rx, \
        _irqUART, _irqITM) \
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
                .isLp = 0, \
                .uart = _KDUART_UART(_uartNumber), \
                .init = { \
                    .baudRate = _baudRate, \
                }, \
            }, \
            .tim = { \
                .isLp = 0, \
                .tim = _KDUART_TIM_MODULE(_timNumber), \
                .timeout = _rto, \
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
    void _irqUART(void) { \
        if (_KDUART_UART(_uartNumber)->INTSR & 0x02) { \
            _KDUART_UART(_uartNumber)->INTCLR = 0x02; \
            if (_KDUART_IVA(_name).flag.isSendIrq) {  \
                if (qBSBuffer_Empty(_KDUART_INAME(_name).buffer.writeLwrb)) { \
                    _KDUART_IVA(_name).flag.isSendCompleted = 1; \
                } else { \
                    uint8_t data; \
                    qBSBuffer_Get(_KDUART_INAME(_name).buffer.writeLwrb, &data); \
                    _KDUART_UART(_uartNumber)->SBUF = data; \
                } \
            } else { \
                _KDUART_IVA(_name).flag.isSendCompleted = 1; \
            } \
        } else if (_KDUART_UART(_uartNumber)->INTSR & 0x01) { \
            uint8_t recvByte = _KDUART_UART(_uartNumber)->SBUF; \
            qBSBuffer_Put(_KDUART_INAME(_name).buffer.recvLwrb, recvByte); \
            _KDUART_UART(_uartNumber)->INTCLR = 0x01; \
            _KDUART_TIM_MODULE(_timNumber)->INTCLR |= 0x01; \
            _KDUART_TIM_MODULE(_timNumber)->CR &= ~(0x01 << 7); \
            _KDUART_TIM_MODULE(_timNumber)->LOAD = _KDUART_TIM_MODULE(_timNumber)->BGLOAD; \
            _KDUART_TIM_MODULE(_timNumber)->CR |= (0x01 << 7); \
            _KDUART_IVA(_name).flag.isRecving = 1; \
        } else if (_KDUART_UART(_uartNumber)->INTSR & 0x04) { \
            _KDUART_UART(_uartNumber)->INTCLR = 0x04; \
        } \
    } \
    void _irqITM(void) { \
        _KDUART_TIM_MODULE(_timNumber)->CR &= ~(0x01 << 7); \
        _KDUART_TIM_MODULE(_timNumber)->INTCLR |= 0x01; \
        _KDUART_IVA(_name).flag.isRecving = 0; \
        _KDUART_IVA(_name).flag.isRecvCompleted = 1; \
    }


#define KDUART_TIM_NOBLOCK_DEFINE(_uartNumber, _timNumber, _name, \
        _enableFunc, _disableFunc, \
        _txBufferSize, _rxBufferSize, \
        _baudRate, _rto, \
        _tx, _txPinMode, _rx, \
        _irqUART, _irqITM) \
    static void _KDUART_FUNC_ENABLE(_name)(kduart_t *kd) _enableFunc \
    static void _KDUART_FUNC_DISABLE(_name)(kduart_t *kd) _disableFunc \
    static kduart_VA_t _KDUART_IVA(_name) = {0}; \
    static qBSBuffer_t _KDUART_RX_LWRB(_name); \
    static uint8_t AT_NONCACHEABLE_SECTION_ALIGN(_KDUART_RX_BUFFER(_name)[_rxBufferSize], 4); \
    static qBSBuffer_t _KDUART_TX_LWRB(_name); \
    static uint8_t AT_NONCACHEABLE_SECTION_ALIGN(_KDUART_TX_BUFFER(_name)[_txBufferSize], 4); \
    const kduart_t _KDUART_INAME(_name) = { \
        ._va = &_KDUART_IVA(_name), \
        ._config = { \
            .rto = _rto, \
            .uart = { \
                .isLp = 0, \
                .uart = _KDUART_UART(_uartNumber), \
                .init = { \
                    .baudRate = _baudRate, \
                }, \
            }, \
            .tim = { \
                .isLp = 0, \
                .tim = _KDUART_TIM_MODULE(_timNumber), \
                .timeout = _rto, \
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
            .writeBufferSize = _txBufferSize, \
            .writeBuffer = _KDUART_TX_BUFFER(_name), \
            .recvLwrb = &_KDUART_RX_LWRB(_name), \
            .writeLwrb = &_KDUART_TX_LWRB(_name), \
        }, \
    }; \
    void _irqUART(void) { \
        if (_KDUART_UART(_uartNumber)->INTSR & 0x02) { \
            _KDUART_UART(_uartNumber)->INTCLR = 0x02; \
            if (_KDUART_IVA(_name).flag.isSendIrq) {  \
                if (qBSBuffer_Empty(_KDUART_INAME(_name).buffer.writeLwrb)) { \
                    _KDUART_IVA(_name).flag.isSendCompleted = 1; \
                } else { \
                    uint8_t data; \
                    qBSBuffer_Get(_KDUART_INAME(_name).buffer.writeLwrb, &data); \
                    _KDUART_UART(_uartNumber)->SBUF = data; \
                } \
            } else { \
                _KDUART_IVA(_name).flag.isSendCompleted = 1; \
            } \
        } else if (_KDUART_UART(_uartNumber)->INTSR & 0x01) { \
            uint8_t recvByte = _KDUART_UART(_uartNumber)->SBUF; \
            qBSBuffer_Put(_KDUART_INAME(_name).buffer.recvLwrb, recvByte); \
            _KDUART_UART(_uartNumber)->INTCLR = 0x01; \
            _KDUART_TIM_MODULE(_timNumber)->INTCLR |= 0x01; \
            _KDUART_TIM_MODULE(_timNumber)->CR &= ~(0x01 << 7); \
            _KDUART_TIM_MODULE(_timNumber)->LOAD = _KDUART_TIM_MODULE(_timNumber)->BGLOAD; \
            _KDUART_TIM_MODULE(_timNumber)->CR |= (0x01 << 7); \
            _KDUART_IVA(_name).flag.isRecving = 1; \
        } else if (_KDUART_UART(_uartNumber)->INTSR & 0x04) { \
            _KDUART_UART(_uartNumber)->INTCLR = 0x04; \
        } \
    } \
    void _irqITM(void) { \
        _KDUART_TIM_MODULE(_timNumber)->CR &= ~(0x01 << 7); \
        _KDUART_TIM_MODULE(_timNumber)->INTCLR |= 0x01; \
        _KDUART_IVA(_name).flag.isRecving = 0; \
        _KDUART_IVA(_name).flag.isRecvCompleted = 1; \
    }

#define KDLPUART_LPTIM_BLOCK_DEFINE(_uartNumber, _timNumber, _name, \
        _enableFunc, _disableFunc, \
        _txBufferSize, _rxBufferSize, \
        _baudRate, _rto, \
        _tx, _txPinMode, _rx, \
        _irqUART, _irqITM) \
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
                .isLp = 1, \
                .lpuart = _KDLPUART_MODULE(_uartNumber), \
                .init = { \
                    .baudRate = _baudRate, \
                }, \
            }, \
            .tim = { \
                .isLp = 1, \
                .lptim = _KDUART_LPTIM_MODULE(_timNumber), \
                .timeout = _rto, \
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
    void _irqUART(void) { \
        if (_KDLPUART_MODULE(_uartNumber)->INTSR & 0x02) { \
            _KDLPUART_MODULE(_uartNumber)->INTCLR = 0x02; \
            if (_KDUART_IVA(_name).flag.isSendIrq) {  \
                if (qBSBuffer_Empty(_KDUART_INAME(_name).buffer.writeLwrb)) { \
                    _KDUART_IVA(_name).flag.isSendCompleted = 1; \
                } else { \
                    uint8_t data; \
                    qBSBuffer_Get(_KDUART_INAME(_name).buffer.writeLwrb, &data); \
                    _KDLPUART_MODULE(_uartNumber)->SBUF = data; \
                } \
            } else { \
                _KDUART_IVA(_name).flag.isSendCompleted = 1; \
            } \
        } else if (_KDLPUART_MODULE(_uartNumber)->INTSR & 0x01) { \
            uint8_t recvByte = _KDLPUART_MODULE(_uartNumber)->SBUF; \
            qBSBuffer_Put(_KDUART_INAME(_name).buffer.recvLwrb, recvByte); \
            _KDLPUART_MODULE(_uartNumber)->INTCLR = 0x01; \
            _KDUART_LPTIM_MODULE(_timNumber)->INTCLR |= 0x01; \
            _KDUART_LPTIM_MODULE(_timNumber)->CR &= ~(0x01 << 0); \
            _KDUART_LPTIM_MODULE(_timNumber)->LOAD = _KDUART_LPTIM_MODULE(_timNumber)->BGLOAD; \
            _KDUART_LPTIM_MODULE(_timNumber)->CR |= (0x01 << 0); \
            _KDUART_IVA(_name).flag.isRecving = 1; \
        } else if (_KDLPUART_MODULE(_uartNumber)->INTSR & 0x04) { \
            _KDLPUART_MODULE(_uartNumber)->INTCLR = 0x04; \
        } \
    } \
    void _irqITM(void) { \
        _KDUART_LPTIM_MODULE(_timNumber)->CR &= ~(0x01 << 0); \
        _KDUART_LPTIM_MODULE(_timNumber)->INTCLR |= 0x01; \
        _KDUART_IVA(_name).flag.isRecving = 0; \
        _KDUART_IVA(_name).flag.isRecvCompleted = 1; \
    }

#define KDLPUART_LPTIM_NOBLOCK_DEFINE(_uartNumber, _timNumber, _name, \
        _enableFunc, _disableFunc, \
        _txBufferSize, _rxBufferSize, \
        _baudRate, _rto, \
        _tx, _txPinMode, _rx, \
        _irqUART, _irqITM) \
    static void _KDUART_FUNC_ENABLE(_name)(kduart_t *kd) _enableFunc \
    static void _KDUART_FUNC_DISABLE(_name)(kduart_t *kd) _disableFunc \
    static kduart_VA_t _KDUART_IVA(_name) = {0}; \
    static qBSBuffer_t _KDUART_RX_LWRB(_name); \
    static uint8_t AT_NONCACHEABLE_SECTION_ALIGN(_KDUART_RX_BUFFER(_name)[_rxBufferSize], 4); \
    static qBSBuffer_t _KDUART_TX_LWRB(_name); \
    static uint8_t AT_NONCACHEABLE_SECTION_ALIGN(_KDUART_TX_BUFFER(_name)[_txBufferSize], 4); \
    const kduart_t _KDUART_INAME(_name) = { \
        ._va = &_KDUART_IVA(_name), \
        ._config = { \
            .rto = _rto, \
            .uart = { \
                .isLp = 1, \
                .lpuart = _KDLPUART_MODULE(_uartNumber), \
                .init = { \
                    .baudRate = _baudRate, \
                }, \
            }, \
            .tim = { \
                .isLp = 1, \
                .lptim = _KDUART_LPTIM_MODULE(_timNumber), \
                .timeout = _rto, \
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
            .writeBufferSize = _txBufferSize, \
            .writeBuffer = _KDUART_TX_BUFFER(_name), \
            .recvLwrb = &_KDUART_RX_LWRB(_name), \
            .writeLwrb = &_KDUART_TX_LWRB(_name), \
        }, \
    }; \
    void _irqUART(void) { \
        if (_KDLPUART_MODULE(_uartNumber)->INTSR & 0x02) { \
            _KDLPUART_MODULE(_uartNumber)->INTCLR = 0x02; \
            if (_KDUART_IVA(_name).flag.isSendIrq) {  \
                if (qBSBuffer_Empty(_KDUART_INAME(_name).buffer.writeLwrb)) { \
                    _KDUART_IVA(_name).flag.isSendCompleted = 1; \
                } else { \
                    uint8_t data; \
                    qBSBuffer_Get(_KDUART_INAME(_name).buffer.writeLwrb, &data); \
                    _KDLPUART_MODULE(_uartNumber)->SBUF = data; \
                } \
            } else { \
                _KDUART_IVA(_name).flag.isSendCompleted = 1; \
            } \
        } else if (_KDLPUART_MODULE(_uartNumber)->INTSR & 0x01) { \
            uint8_t recvByte = _KDLPUART_MODULE(_uartNumber)->SBUF; \
            qBSBuffer_Put(_KDUART_INAME(_name).buffer.recvLwrb, recvByte); \
            _KDLPUART_MODULE(_uartNumber)->INTCLR = 0x01; \
            _KDUART_LPTIM_MODULE(_timNumber)->INTCLR |= 0x01; \
            _KDUART_LPTIM_MODULE(_timNumber)->CR &= ~(0x01 << 0); \
            _KDUART_LPTIM_MODULE(_timNumber)->LOAD = _KDUART_LPTIM_MODULE(_timNumber)->BGLOAD; \
            _KDUART_LPTIM_MODULE(_timNumber)->CR |= (0x01 << 0); \
            _KDUART_IVA(_name).flag.isRecving = 1; \
        } else if (_KDLPUART_MODULE(_uartNumber)->INTSR & 0x04) { \
            _KDLPUART_MODULE(_uartNumber)->INTCLR = 0x04; \
        } \
    } \
    void _irqITM(void) { \
        _KDUART_LPTIM_MODULE(_timNumber)->CR &= ~(0x01 << 0); \
        _KDUART_LPTIM_MODULE(_timNumber)->INTCLR |= 0x01; \
        _KDUART_IVA(_name).flag.isRecving = 0; \
        _KDUART_IVA(_name).flag.isRecvCompleted = 1; \
    }
/*@}*/

#endif
