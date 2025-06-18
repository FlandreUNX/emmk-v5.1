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

#define KDSPI_FLAG_TRANSMIT_COMPLETE     0x00000001u

/*@}*/

/**
 * @addtogroup type defin
 * @note none
 */
 
/*@{*/

typedef struct kdspi kdspi_t;

typedef enum {
    KDSPI_MODE_MASTER = (1u << 4u),
    KDSPI_MODE_SLAVE = 0,
} ksdpi_Mode_t;

typedef enum {
    KDSPI_CPOL_L = 0,
    KDSPI_CPOL_H = (1u << 3u),
} ksdpi_CPOL_t;

typedef enum {
    KDSPI_CPHA_1 = 0,
    KDSPI_CPHA_2 = (1u << 2u),
} ksdpi_CPHA_t;

typedef enum {
    KDSPI_CLKSEL_2 = 0,
    KDSPI_CLKSEL_4 = 0x01,
    KDSPI_CLKSEL_8 = 0x02,
    KDSPI_CLKSEL_16 = 0x03,
    KDSPI_CLKSEL_32 = 0x80,
    KDSPI_CLKSEL_64 = 0x80 | 0x01,
    KDSPI_CLKSEL_128 = 0x80 | 0x02,
} kdspi_ClkSel_t;

typedef struct {
    int8_t initRefs;
    int8_t initRefsPower;

    union {
        struct {
            uint32_t isSendCompleted: 1;
            uint32_t isRecving: 1;
            uint32_t isRecvCompleted: 1;
        };
        uint32_t _;
        void *ptr;
    } volatile flag;
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
            SPI_TypeDef *spi;
            uint32_t defualtSpeed;
            struct {
                ksdpi_Mode_t mstr;
                ksdpi_CPOL_t cpol;
                ksdpi_CPHA_t cpha;
                kdspi_ClkSel_t clksel;
            } init;
        } spi;
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

#define KDSPI_MOSI_DEFINE(_gpio, _af) \
    .mosi = { \
        .af = _KDSPI_PIN_AF(_af), \
        .gpio = ((kdgpio_t *) _gpio), \
    }
    
#define KDSPI_MISO_DEFINE(_gpio, _af) \
    .miso = { \
        .af = _KDSPI_PIN_AF(_af), \
        .gpio = ((kdgpio_t *) _gpio), \
    }
    
#define KDSPI_CLK_DEFINE(_gpio, _af) \
    .clk = { \
        .af = _KDSPI_PIN_AF(_af), \
        .gpio = ((kdgpio_t *) _gpio), \
    }

#define KDSPI_MODULE(_number)       _number
#define _KDSPI_MODULE(_number)      SPI##_number

#define _KDSPI_IVA(x)               __kdspi_va_##x
#define _KDSPI_FUNC_ENABLE(x)       __kdspi_enable_##x
#define _KDSPI_FUNC_DISABLE(x)      __kdspi_disable_##x
#define _KDSPI_INAME(x)             __kdspi_##x

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

#define KDSPI_BLOCK_DEFINE(_moduleNumber, _name, \
        _baudRate, \
        _mode, _cpol, _cpha, _prescale, \
        _clk, _mosi, _miso, \
        _enableFunc, _disableFunc \
    ) \
    static void _KDSPI_FUNC_ENABLE(_name)(kdspi_t *kd) _enableFunc \
    static void _KDSPI_FUNC_DISABLE(_name)(kdspi_t *kd) _disableFunc \
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
                    .mstr = _mode, \
                    .cpol = _cpol, \
                    .cpha = _cpha, \
                    .clksel = _prescale, \
                }, \
            }, \
        }, \
        ._instance = { \
            .enableFunc = _KDSPI_FUNC_ENABLE(_name), \
            .disableFunc = _KDSPI_FUNC_DISABLE(_name), \
        }, \
    };
    
#define KDSPI_TRANSMIT_TIMEOUT_BLOCK        ((uint32_t) -1)
#define KDSPI_TRANSMIT_TIMEOUT_NOBLOCK      ((uint32_t) 0)
#define KDSPI_TRANSMIT_TIMEOUT_WAIT(x)      ((uint32_t) x)
    
/*@}*/

#endif
