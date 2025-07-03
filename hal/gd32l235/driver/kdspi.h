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
 
#ifndef _KDSPI_H_
#define _KDSPI_H_

#include "emmk-config.h"

#include "kdgpio.h"

#include "gd32l23x_gpio.h"
#include "gd32l23x_spi.h"
#include "gd32l23x_dma.h"

/**
 * @addtogroup Define
 * @note none
 */
 
/*@{*/

#define KDSPI_FLAG_TRANSMIT_COMPLETE     0x00000001u

/*@}*/

/**
 * @addtogroup type defin
 * @note none
 */
 
/*@{*/

typedef struct kdspi kdspi_t;

typedef struct {
    int8_t initRefs;
    int8_t initRefsPower;

    union {
        union {
            struct {
                uint32_t isSendCompleted: 1;
                uint32_t isRecving: 1;
                uint32_t isRecvCompleted: 1;
            };
            uint32_t _;
        };
        void *ptr;
    } flag;
    void *mutex;
} kdspi_Va_t;

typedef struct {
    uint32_t af;
    kdgpio_t *gpio;
} kdspi_PinCfg_t;

struct kdspi {
    kdspi_Va_t *_va;
    
    struct {
        struct {
            kdspi_PinCfg_t clk;
            kdspi_PinCfg_t mosi;
            kdspi_PinCfg_t miso;
        } pin;

        struct {
            uint32_t spi;
            uint32_t defualtSpeed;
            spi_parameter_struct init;
        } spi;
        
        struct {
            dma_channel_enum channelTx;
            dma_channel_enum channelRx;
            dma_parameter_struct initTx;
            dma_parameter_struct initRx;
        } dma;
    } _config;

    struct {
        void (*enableFunc)(kdspi_t *kd);
        void (*disableFunc)(kdspi_t *kd);
    } _instance;
};

/*@}*/

/**
 * @addtogroup Instance define
 * @note none
 */
 
/*@{*/

#define _KDSPI_NEXTERN(_name)       extern const kdspi_t __kdspi_##_name
#define KDSPI_NEXTERN(_name)        _KDSPI_NEXTERN(_name)
#define _KDSPI_NGET(_name)          __kdspi_##_name
#define KDSPI_NGET(_name)           _KDSPI_NGET(_name)

#define KDSPI_PIN_AF(_af)   _af
#define _KDSPI_PIN_AF(_af)  _af

#define KDSPI_MOSI_DEFINE(_gpio) \
    .mosi = { \
        .gpio = ((kdgpio_t *) _gpio), \
    }
    
#define KDSPI_MISO_DEFINE(_gpio) \
    .miso = { \
        .gpio = ((kdgpio_t *) _gpio), \
    }
    
#define KDSPI_CLK_DEFINE(_gpio) \
    .clk = { \
        .gpio = ((kdgpio_t *) _gpio), \
    }

#define KDSPI_MODULE(_number)       _number
#define _KDSPI_MODULE(_number)      SPI##_number

#define _KDSPI_IVA(x)               __kdspi_va_##x
#define _KDSPI_FUNC_ENABLE(x)       __kdspi_enable_##x
#define _KDSPI_FUNC_DISABLE(x)      __kdspi_disable_##x
#define _KDSPI_INAME(x)             __kdspi_##x
    
#define KDSPI_BAUDRATE(x)           x
    
#define KDSPI_DMA_TX_STREAM_NUMBER(x)        x
#define KDSPI_DMA_TX_INSTANCE(x)             x
#define KDSPI_DMA_RX_STREAM_NUMBER(x)        x
#define KDSPI_DMA_RX_INSTANCE(x)             x

extern int32_t kdspi_init(kdspi_t *kd); \
extern int32_t kdspi_finalize(kdspi_t *kd); \
extern int32_t kdspi_powerUp(kdspi_t *kd); \
extern int32_t kdspi_powerDown(kdspi_t *kd); \
extern void kdspi_select(kdspi_t *kd); \
extern void kdspi_unselect(kdspi_t *kd); \
extern int32_t kdspi_transmit(kdspi_t *kd, uint8_t *wbuf, uint8_t *rbuf, uint16_t len, uint32_t timeout); \
extern int32_t kdspi_sendData(kdspi_t *kd, uint8_t *data, uint16_t len, uint32_t timeout); \
extern int32_t kdspi_recvData(kdspi_t *kd, uint8_t *data, uint16_t len, uint32_t timeout); \
extern void kdspi_setBaudRate(kdspi_t *kd, uint32_t br); \

#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
#define KDSPI_ACTIVE_TX_DMA_HANDLE(x, _action) \
    dma_interrupt_flag_clear(x._config.dma.dmaTx, x._config.dma.channelTx, DMA_INT_FLAG_G | DMA_INT_FLAG_FTF | DMA_INT_FLAG_ERR); \
    dma_channel_disable(x._config.dma.dmaTx, x._config.dma.channelTx); \
    if (_action) { \
        osEventFlagsSet(x._va->flag.ptr, KDSPI_FLAG_TRANSMIT_COMPLETE); \
    }
    
#define KDSPI_ACTIVE_RX_DMA_HANDLE(x, _action) \
    dma_interrupt_flag_clear(x._config.dma.dmaRx, x._config.dma.channelRx, DMA_INT_FLAG_G | DMA_INT_FLAG_FTF | DMA_INT_FLAG_ERR); \
    dma_channel_disable(x._config.dma.dmaRx, x._config.dma.channelRx); \
    if (_action) { \
        osEventFlagsSet(x._va->flag.ptr, KDSPI_FLAG_TRANSMIT_COMPLETE); \
    } 
#else
#define KDSPI_ACTIVE_TX_DMA_HANDLE(x, _action) \
    dma_interrupt_flag_clear(x._config.dma.channelTx, DMA_INT_FLAG_G | DMA_INT_FLAG_FTF | DMA_INT_FLAG_ERR); \
    dma_channel_disable(x._config.dma.channelTx); \
    if (_action) { \
        x._va->flag.isSendCompleted = 1; \
        x._va->flag.isRecvCompleted = 1; \
    } 

    
#define KDSPI_ACTIVE_RX_DMA_HANDLE(x, _action) \
    dma_interrupt_flag_clear(x._config.dma.channelRx, DMA_INT_FLAG_G | DMA_INT_FLAG_FTF | DMA_INT_FLAG_ERR); \
    dma_channel_disable(x._config.dma.channelRx); \
    if (_action) { \
        x._va->flag.isSendCompleted = 1; \
        x._va->flag.isRecvCompleted = 1; \
    } 
#endif

#define KDSPI_DEFINE(_moduleNumber, _name, \
        _baudRate, \
        _dmaChannelTx, _dmaChannelRx, \
        _mode, _polarityPhase, _nss, _prescale, _endian, \
        _clk, _mosi, _miso, \
        _enableFunc, _disableFunc, \
        _dmaTxIrq, _dmaRxIrq \
    ) \
    static void _KDSPI_FUNC_ENABLE(_moduleNumber)(kdspi_t *kd) _enableFunc \
    static void _KDSPI_FUNC_DISABLE(_moduleNumber)(kdspi_t *kd) _disableFunc \
    static kdspi_Va_t _KDSPI_IVA(_name) = {0}; \
    const kdspi_t _KDSPI_INAME(_name) = { \
        ._va = &_KDSPI_IVA(_name), \
        ._config = { \
            .pin = { \
                _clk, _mosi, _miso, \
            }, \
            .spi = { \
                .spi = _KDSPI_MODULE(_moduleNumber), \
                .defualtSpeed = _baudRate, \
                .init = { \
                    .trans_mode = _mode, \
                    .device_mode = SPI_MASTER, \
                    .frame_size = SPI_FRAMESIZE_8BIT, \
                    .clock_polarity_phase = _polarityPhase, \
                    .nss = _nss, \
                    .prescale = _prescale, \
                    .endian = _endian, \
                }, \
            }, \
            .dma = { \
                .channelTx = _dmaChannelTx, \
                .channelRx = _dmaChannelRx, \
                .initTx = { \
                    .periph_addr = (uint32_t) &SPI_DATA(_KDSPI_MODULE(_moduleNumber)), \
                    .memory_addr = 0, \
                    .direction = DMA_MEMORY_TO_PERIPHERAL, \
                    .memory_width = DMA_MEMORY_WIDTH_8BIT, \
                    .periph_width = DMA_PERIPHERAL_WIDTH_8BIT, \
                    .priority = DMA_PRIORITY_LOW, \
                    .number = 0, \
                    .periph_inc = DMA_PERIPH_INCREASE_DISABLE, \
                    .memory_inc = DMA_MEMORY_INCREASE_ENABLE, \
                }, \
                .initRx = { \
                    .periph_addr = (uint32_t) &SPI_DATA(_KDSPI_MODULE(_moduleNumber)), \
                    .memory_addr = 0, \
                    .direction = DMA_PERIPHERAL_TO_MEMORY, \
                    .memory_width = DMA_MEMORY_WIDTH_8BIT, \
                    .periph_width = DMA_PERIPHERAL_WIDTH_8BIT, \
                    .priority = DMA_PRIORITY_LOW, \
                    .number = 0, \
                    .periph_inc = DMA_PERIPH_INCREASE_DISABLE, \
                    .memory_inc = DMA_MEMORY_INCREASE_ENABLE, \
                }, \
            }, \
        }, \
        ._instance = { \
            .enableFunc = _KDSPI_FUNC_ENABLE(_moduleNumber), \
            .disableFunc = _KDSPI_FUNC_DISABLE(_moduleNumber), \
        }, \
    }; \
    void _dmaTxIrq(void) { \
        KDSPI_ACTIVE_TX_DMA_HANDLE(_KDSPI_INAME(_name), 1); \
    }
    
#define KDSPI_TRANSMIT_TIMEOUT_BLOCK        ((uint32_t) -1)
#define KDSPI_TRANSMIT_TIMEOUT_NOBLOCK      ((uint32_t) 0)
#define KDSPI_TRANSMIT_TIMEOUT_WAIT(x)      ((uint32_t) x)
    
/*@}*/

#endif
