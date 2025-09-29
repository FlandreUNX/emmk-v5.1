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

#include "../SWM180_StdPeriph_Driver/SWM180.h"
#include "SWM180_port.h"                // Synwit::Device:Driver:PORT
#include "SWM180_gpio.h"                // Synwit::Device:Driver:GPIO
#include "SWM180_spi.h"                 // Synwit::Device:Driver:SPI

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
    
    void *mutex;
} kdspi_Va_t;

typedef struct {
    kdgpio_t *gpio;
    uint32_t af;
} kdspi_PinCfg_t;

struct kdspi {
    kdspi_Va_t *_va;
    
    struct {
        struct {
            kdspi_PinCfg_t clk;
            kdspi_PinCfg_t mosi;
            kdspi_PinCfg_t miso;
        } pinConfig;

        struct {
            SPI_TypeDef *spi;
            uint32_t defualtSpeed;
            SPI_InitStructure init;
        } spiConfig;
    } _config;

    struct {
        void (*enableFunc)(kdspi_t *kdrv);
        void (*disableFunc)(kdspi_t *kdrv);
    } _instance;
};

/*@}*/

/**
 * @addtogroup Instance define
 * @note none
 */
 
/*@{*/

#define _KDSPI_NEXTERN(_name)       extern const kdspi_t __kdspi_##_name
#define KDSPI_NEXTERN(_name)        _KDSPI_NEXTERN(_port, _pin)
#define _KDSPI_NGET(_name)          __kdspi_##_name
#define KDSPI_NGET(_name)           _KDSPI_NGET(_port, _pin)

#define KDSPI_MOSI_DEFINE(_gpio, _af) \
    .mosi = { \
        .gpio = (kdgpio_t *) _gpio, \
        .af = _af, \
    }
    
#define KDSPI_MISO_DEFINE(_gpio, _af) \
    .miso = { \
        .gpio = (kdgpio_t *) _gpio, \
        .af = _af, \
    }
    
#define KDSPI_CLK_DEFINE(_gpio, _af) \
    .clk = { \
        .gpio = (kdgpio_t *) _gpio, \
        .af = _af, \
    }

#define KDSPI_MODULE(_number)       _number
#define _KDSPI_MODULE(_number)      SPI##_number

#define _KDSPI_INAME(x)             __kdspi_##x
#define _KDSPI_IVA(x)               __kdspi_va_##x
#define _KDSPI_FUNC_ENABLE(x)       __kdspi_enable_##x
#define _KDSPI_FUNC_DISABLE(x)      __kdspi_disable_##x

extern int32_t kdspi_init(kdspi_t *kd); \
extern int32_t kdspi_finalize(kdspi_t *kd); \
extern int32_t kdspi_powerUp(kdspi_t *kd); \
extern int32_t kdspi_powerDown(kdspi_t *kd); \
extern void kdspi_select(kdspi_t *kd); \
extern void kdspi_unselect(kdspi_t *kd); \
extern int32_t kdspi_transmit(kdspi_t *kd, uint8_t *wbuf, uint8_t *rbuf, uint16_t len, uint32_t timeout); \
extern int32_t kdspi_sendData(kdspi_t *kd, uint8_t *data, uint16_t len, uint32_t timeout); \
extern int32_t kdspi_recvData(kdspi_t *kd, uint8_t *data, uint16_t len, uint32_t timeout); \
extern void kdspi_setBaudRate(kdspi_t *kd, uint32_t br); 

#define KDSPI_DEFINE(_moduleNumber, _name, \
        _baud_rate, _clkDiv, \
        _mode, _sampleEdge, _idleLevel, _wordSize, \
        _clk, _mosi, _miso, \
        _enable_func, _disable_func \
    ) \
    static void _KDSPI_FUNC_ENABLE(_name)(kdspi_t *kdrv) _enable_func \
    static void _KDSPI_FUNC_DISABLE(_name)(kdspi_t *kdrv) _disable_func \
    static kdspi_Va_t _KDSPI_IVA(_name) = { \
    }; \
    const kdspi_t _KDSPI_INAME(_name) = { \
        ._va = &_KDSPI_IVA(_name), \
        ._config = { \
            .pinConfig = { \
                _clk, _mosi, _miso, \
            }, \
            .spiConfig = { \
                .spi = _KDSPI_MODULE(_moduleNumber), \
                .defualtSpeed = _baud_rate, \
                .init = { \
                    .clkDiv = _clkDiv, \
                    .FrameFormat = _mode, \
                    .SampleEdge = _sampleEdge, \
                    .IdleLevel = _idleLevel, \
                    .WordSize = _wordSize, \
                    .Master = 1, \
                    .RXHFullIEn = 0, \
                    .TXEmptyIEn = 0, \
                    .TXCompleteIEn = 0, \
                }, \
            }, \
        }, \
        ._instance = { \
            .enableFunc = _KDSPI_FUNC_ENABLE(_name), \
            .disableFunc = _KDSPI_FUNC_DISABLE(_name), \
        }, \
    };
    
    
#define KDSPI_TRANSMIT_BUFFER_NO_CACHE(_name) AT_NONCACHEABLE_SECTION_ALIGN_INIT(_name, 4)
    
#define KDSPI_TRANSMIT_TIMEOUT_BLOCK   ((uint32_t) -1)
#define KDSPI_TRANSMIT_TIMEOUT_NOBLOCK ((uint32_t) 0)
#define KDSPI_TRANSMIT_TIMEOUT_WAIT(x) ((uint32_t) x)
    
/*@}*/

#endif
