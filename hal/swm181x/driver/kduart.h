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

#include "SWM181.h"                     // Device header
#include "SWM181_gpio.h"                // Synwit::Device:Driver:GPIO
#include "SWM181_port.h"                // Synwit::Device:Driver:Port
#include "SWM181_uart.h"                // Synwit::Device:Driver:UART

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
    kdgpio_t *gpio;
    uint32_t af;
} kduart_Pin_t;

typedef struct {
#if EMMK_FULL_RTOS_SUPPORT > EMMK_FULL_RTOS_QTS
    void *flag;
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
} kduart_Va_t;

struct kduart {
    kduart_Va_t *_va;

    struct {
        struct {
            kduart_Pin_t tx;
            kduart_Pin_t rx;
        } pin;
        
        struct {
            UART_TypeDef *uart;
            UART_InitStructure init;
        } uart;
    } _config;
    
    struct {
        void (*baseEnable)(kduart_t *kd);
        void (*baseDisable)(kduart_t *kd);
    } _base;

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

#define _KDUART_NGET(x)         __kduart_##x
#define _KDUART_NEXTERN(x)      extern const kduart_t __kduart_##x
#define KDUART_NGET(x)          _KDUART_NGET(x)
#define KDUART_NEXTERN(x)       _KDUART_NEXTERN(x)

#define KDUART_TX_DEFINE(_gpio, _af) \
    .tx = { \
        .gpio = (kdgpio_t *) _gpio, \
        .af = _af, \
    }
    
#define KDUART_RX_DEFINE(_gpio, _af) \
    .rx = { \
        .gpio = (kdgpio_t *) _gpio, \
        .af = _af, \
    }

#define KDUART_RX_BUFFER(x)         x
#define _KDUART_RX_BUFFER(x)        __kduart_rx_buffer_##x
#define _KDUART_RX_LWRB(x)          __kduart_rx_lwrb_##x
    
#define KDUART_TX_BUFFER(x) x
#define _KDUART_TX_BUFFER(x)  __kduart_tx_buffer_##x
#define _KDUART_TX_LWRB(x)          __kduart_tx_lwrb_##x

#define KDUART_BAUDRATE(x)          x
#define KDUART_TIMEOUT_CHAR(x)      x
    
#define KDUART_RX_FIFO_THRESHOLD(x)         x
#define KDUART_TX_FIFO_THRESHOLD(x)         x
    
#define KDUART_MODULE(_number)          _number
#define _KDUART_MODULE(_number)         UART##_number

#define _KDUART_INAME(x)            __kduart_##x
#define _KDUART_IVA(x)              __kduart_va_##x
#define _KDUART_FUNC_ENABLE(x)      __kduart_enable_##x
#define _KDUART_FUNC_DISABLE(x)     __kduart_disable_##x

#define _KDUART_IRQ_HANDLER(x)   __kd_kuart_irq_handler_##x


extern int32_t kduart_init(kduart_t *kd);
extern int32_t kduart_finalize(kduart_t *kd);
extern int32_t kduart_powerUp(kduart_t *kd);
extern int32_t kduart_powerDown(kduart_t *kd);
extern int32_t kduart_sends(kduart_t *kd, const void *data, uint32_t size, uint32_t timeout);
extern int32_t kduart_sends9(kduart_t *kd, uint16_t *data, uint32_t size, uint32_t timeout);
extern int32_t kduart_recvs(kduart_t *kd, void *data, uint32_t expect_size, uint32_t *recv_size, uint32_t timeout);
extern int32_t kduart_flush(kduart_t *kd);
extern int32_t kduart_hasRecvData(kduart_t *kd);
extern int32_t kduart_isSendIdle(kduart_t *kd, uint32_t wait);
extern void kduart_updateBaudRate(kduart_t *kd, uint32_t bd);
extern bool kduart_sendBuffingVerify(kduart_t *kd, uint32_t dataSize);

#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
#define KDUART_RTO_DEFINE(_instance, _name, \
        _baudrate, _dataBits, _parity, _stopBits, \
        _rxThreshold, _txThreshold, \
        _timeoutTime, \
        _rxBufferSize, \
        _tx, _rx, \
        _enableFunc, _disableFunc, _irqName) \
    static qBSBuffer_t _KDUART_RX_LWRB(_name); \
    static uint8_t AT_NONCACHEABLE_SECTION_ALIGN(_KDUART_RX_BUFFER(_name)[_rxBufferSize], 4); \
    static void _KDUART_FUNC_ENABLE(_name)(kduart_t *kd) _enableFunc \
    static void _KDUART_FUNC_DISABLE(_name)(kduart_t *kd) _disableFunc \
    static kduart_Va_t _KDUART_IVA(_name) = {0}; \
    const kduart_t _KDUART_INAME(_name) = { \
        ._va = &_KDUART_IVA(_name), \
        ._config = { \
            .uart = { \
                .uart = _KDUART_MODULE(_instance), \
                .init = { \
                    .Baudrate = _baudrate, \
                    .RXThreshold = _rxThreshold, \
                    .RXThresholdIEn = 1, \
                    .TXThreshold = _txThreshold, \
                    .TXThresholdIEn = 0, \
                    .TimeoutTime = _timeoutTime, \
                    .TimeoutIEn = 1, \
                }, \
            }, \
            .pin = { \
                _tx, _rx, \
            }, \
        }, \
        ._base = { \
            .baseEnable = _KDUART_FUNC_ENABLE(_name), \
            .baseDisable = _KDUART_FUNC_DISABLE(_name), \
        }, \
        .buffer = { \
            .recvBufferSize = _rxBufferSize, \
            .recvBuffer = _KDUART_RX_BUFFER(_name), \
            .recvLwrb = &_KDUART_RX_LWRB(_name), \
        }, \
    }; \
    void _irqName(void) { \
        if (UART_INTRXThresholdStat(_KDUART_MODULE(_instance))) { \
            while ((_KDUART_MODULE(_instance)->FIFO & UART_FIFO_RXLVL_Msk) > 1) { \
                uint8_t data = _KDUART_MODULE(_instance)->DATA & UART_DATA_DATA_Msk; \
                klwrb_write(_KDUART_INAME(_name).buffer.recvLwrb, &data, 1); \
            } \
        } else if (UART_INTTimeoutStat(_KDUART_MODULE(_instance))) { \
            while (UART_IsRXFIFOEmpty(_KDUART_MODULE(_instance)) == 0) { \
                uint8_t data = _KDUART_MODULE(_instance)->DATA & UART_DATA_DATA_Msk; \
                klwrb_write(_KDUART_INAME(_name).buffer.recvLwrb, &data, 1); \
            } \
             osEventFlagsSet(_KDUART_INAME(_name)._va->flag, UART_FLAG_RECV_COMPLETE); \
        } \
    }
#else

#define KDUART_RTO_TIRQ_DEFINE(_instance, _name, \
        _baudrate, _dataBits, _parity, _stopBits, \
        _rxThreshold, _txThreshold, \
        _timeoutTime, \
        _txBufferSize, _rxBufferSize, \
        _tx, _rx, \
        _enableFunc, _disableFunc, _irqName) \
    static qBSBuffer_t _KDUART_RX_LWRB(_name); \
    static uint8_t AT_NONCACHEABLE_SECTION_ALIGN(_KDUART_RX_BUFFER(_name)[_rxBufferSize], 4); \
    static qBSBuffer_t _KDUART_TX_LWRB(_name); \
    static uint8_t AT_NONCACHEABLE_SECTION_ALIGN(_KDUART_TX_BUFFER(_name)[_txBufferSize], 4); \
    static void _KDUART_FUNC_ENABLE(_name)(kduart_t *kd) _enableFunc \
    static void _KDUART_FUNC_DISABLE(_name)(kduart_t *kd) _disableFunc \
    static kduart_Va_t _KDUART_IVA(_name) = {0}; \
    const kduart_t _KDUART_INAME(_name) = { \
        ._va = &_KDUART_IVA(_name), \
        ._config = { \
            .uart = { \
                .uart = _KDUART_MODULE(_instance), \
                .init = { \
                    .Baudrate = _baudrate, \
                    .RXThreshold = _rxThreshold, \
                    .RXThresholdIEn = 1, \
                    .TXThreshold = _txThreshold, \
                    .TXThresholdIEn = 0, \
                    .TimeoutTime = _timeoutTime, \
                    .TimeoutIEn = 1, \
                }, \
            }, \
            .pin = { \
                _tx, _rx, \
            }, \
        }, \
        ._base = { \
            .baseEnable = _KDUART_FUNC_ENABLE(_name), \
            .baseDisable = _KDUART_FUNC_DISABLE(_name), \
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
    void _irqName(void) { \
        if (UART_INTTXThresholdStat(_KDUART_MODULE(_instance))) { \
            while (UART_IsTXFIFOFull(_KDUART_MODULE(_instance)) == 0) { \
                if (qBSBuffer_Empty(_KDUART_INAME(_name).buffer.writeLwrb)) { \
                    UART_INTTXThresholdDis(_KDUART_MODULE(_instance)); \
                    _KDUART_INAME(_name)._va->flag.isSendCompleted = 1; \
                    break; \
                } else { \
                    uint8_t data; \
                    qBSBuffer_Get(_KDUART_INAME(_name).buffer.writeLwrb, &data); \
                    UART_WriteByte(_KDUART_MODULE(_instance), data); \
                } \
            } \
        } else if (UART_INTRXThresholdStat(_KDUART_MODULE(_instance))) { \
            _KDUART_INAME(_name)._va->flag.isRecving = 1; \
            _KDUART_INAME(_name)._va->flag.isRecvCompleted = 0; \
            while ((_KDUART_MODULE(_instance)->FIFO & UART_FIFO_RXLVL_Msk) > 1) { \
                uint8_t data = _KDUART_MODULE(_instance)->DATA & UART_DATA_DATA_Msk; \
                qBSBuffer_Put(_KDUART_INAME(_name).buffer.recvLwrb, data); \
            } \
        } else if (UART_INTTimeoutStat(_KDUART_MODULE(_instance))) { \
            while (UART_IsRXFIFOEmpty(_KDUART_MODULE(_instance)) == 0) { \
                uint8_t data = _KDUART_MODULE(_instance)->DATA & UART_DATA_DATA_Msk; \
                qBSBuffer_Put(_KDUART_INAME(_name).buffer.recvLwrb, data); \
            } \
            _KDUART_INAME(_name)._va->flag.isRecving = 0; \
            _KDUART_INAME(_name)._va->flag.isRecvCompleted = 1; \
        } \
    }

#define KDUART_RTO_TBLOCK_DEFINE(_instance, _name, \
        _baudrate, _dataBits, _parity, _stopBits, \
        _rxThreshold, _txThreshold, \
        _timeoutTime, \
        _txBufferSize, _rxBufferSize, \
        _tx, _rx, \
        _enableFunc, _disableFunc, _irqName) \
    static qBSBuffer_t _KDUART_RX_LWRB(_name); \
    static uint8_t AT_NONCACHEABLE_SECTION_ALIGN(_KDUART_RX_BUFFER(_name)[_rxBufferSize], 4); \
    static void _KDUART_FUNC_ENABLE(_name)(kduart_t *kd) _enableFunc \
    static void _KDUART_FUNC_DISABLE(_name)(kduart_t *kd) _disableFunc \
    static kduart_Va_t _KDUART_IVA(_name) = {0}; \
    const kduart_t _KDUART_INAME(_name) = { \
        ._va = &_KDUART_IVA(_name), \
        ._config = { \
            .uart = { \
                .uart = _KDUART_MODULE(_instance), \
                .init = { \
                    .Baudrate = _baudrate, \
                    .RXThreshold = _rxThreshold, \
                    .RXThresholdIEn = 1, \
                    .TXThreshold = _txThreshold, \
                    .TXThresholdIEn = 0, \
                    .TimeoutTime = _timeoutTime, \
                    .TimeoutIEn = 1, \
                }, \
            }, \
            .pin = { \
                _tx, _rx, \
            }, \
        }, \
        ._base = { \
            .baseEnable = _KDUART_FUNC_ENABLE(_name), \
            .baseDisable = _KDUART_FUNC_DISABLE(_name), \
        }, \
        .buffer = { \
            .recvBufferSize = _rxBufferSize, \
            .recvBuffer = _KDUART_RX_BUFFER(_name), \
            .writeBufferSize = 0, \
            .writeBuffer = NULL, \
            .recvLwrb = &_KDUART_RX_LWRB(_name), \
            .writeLwrb = NULL, \
        }, \
    }; \
    void _irqName(void) { \
        if (UART_INTRXThresholdStat(_KDUART_MODULE(_instance))) { \
            _KDUART_INAME(_name)._va->flag.isRecving = 1; \
            _KDUART_INAME(_name)._va->flag.isRecvCompleted = 0; \
            while ((_KDUART_MODULE(_instance)->FIFO & UART_FIFO_RXLVL_Msk) > 1) { \
                uint8_t data = _KDUART_MODULE(_instance)->DATA & UART_DATA_DATA_Msk; \
                qBSBuffer_Put(_KDUART_INAME(_name).buffer.recvLwrb, data); \
            } \
        } else if (UART_INTTimeoutStat(_KDUART_MODULE(_instance))) { \
            while (UART_IsRXFIFOEmpty(_KDUART_MODULE(_instance)) == 0) { \
                uint8_t data = _KDUART_MODULE(_instance)->DATA & UART_DATA_DATA_Msk; \
                qBSBuffer_Put(_KDUART_INAME(_name).buffer.recvLwrb, data); \
            } \
            _KDUART_INAME(_name)._va->flag.isRecving = 0; \
            _KDUART_INAME(_name)._va->flag.isRecvCompleted = 1; \
        } \
    }
#endif
    
/*@}*/

#endif
