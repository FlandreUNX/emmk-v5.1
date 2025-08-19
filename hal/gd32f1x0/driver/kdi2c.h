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
 
#ifndef _KI2C_H_
#define _KI2C_H_

#include <stdint.h>

#include "emmk-config.h"

#include "kdgpio.h"

#include "gd32f1x0_gpio.h"
#include "gd32f1x0_i2c.h"

/**
 * @addtogroup Define
 * @note none
 */
 
/*@{*/

#define KDI2CBB_ERR_START   -2
#define KDI2CBB_ERR_WACK    -3

/*@}*/

/**
 * @addtogroup typedef
 * @note none
 */
 
/*@{*/

typedef struct {
#if EMMK_FULL_RTOS_SUPPORT > EMMK_FULL_RTOS_QTS
    void *mutex;
#endif
    int8_t initRefs;
    int8_t initRefsPower;
} kdi2c_VA_t;

typedef struct kdi2c kdi2c_t;

struct kdi2c {
    kdi2c_VA_t *_va;
    
    struct {
        struct {
            struct {
                kdgpio_t *gpio;
            } sda;
            
            struct {
                kdgpio_t *gpio;
            } scl;
        } pin;
        
        struct {
            uint32_t delay;
            uint32_t timeout;
        } bb;
        
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
        uint32_t i2c;
        
        uint32_t clockSpeed;
        uint32_t dutyCycle;
        
        void (*enableFunc)(kdi2c_t *kd);
        void (*disableFunc)(kdi2c_t *kd);
    } _instance;
};

/*@}*/

/**
 * @addtogroup Instance define
 * @note none
 */
 
/*@{*/

#define _KDI2C_NGET(x)       __kdi2c_##x
#define _KDI2C_NEXTERN(x)    extern const kdi2c_t __kdi2c_##x
#define KDI2C_NGET(x)        _KDI2C_NGET(x)
#define KDI2C_NEXTERN(x)    _KDI2C_NEXTERN(x)

#define KDI2C_PIN_AF(_af)           _af
#define _KDI2C_PIN_AF(_af)          _af

#define KDI2C_MODULE(_number)       _number
#define _KDI2C_MODULE(_number)      I2C##_number

#define _KDI2C_IVA(x)               __kdi2c_va_##x
#define _KDI2C_INAME(x)             __kdi2c_##x

#define _KDI2C_FUNC_ENABLE(x)       __kdi2c_enable_##x
#define _KDI2C_FUNC_DISABLE(x)      __kdi2c_disable_##x
    
#define KDI2CBB_DELAY(x)            x
#define KDI2CBB_TIMEOUT(x)          x

#define KDI2C_DMA_MODULE(_number)           _number
#define _KDI2C_DMA_MODULE(_number)          DMA##_number
#define KDI2C_DMA_TX_STREAM_NUMBER(x)       x
#define KDI2C_DMA_TX_INSTANCE(x)            x
#define KDI2C_DMA_RX_STREAM_NUMBER(x)       x
#define KDI2C_DMA_RX_INSTANCE(x)            x


extern int32_t kdi2c_init(kdi2c_t *kd); \
extern int32_t kdi2c_finalize(kdi2c_t *kd); \
extern int32_t kdi2c_powerUp(kdi2c_t *kd); \
extern int32_t kdi2c_powerDown(kdi2c_t *kd); \
extern void kdi2c_reset(kdi2c_t *kd); \
extern void kdi2c_setSpeed(kdi2c_t *kd, uint32_t speed); \
extern int32_t kdi2c_transmit(kdi2c_t *kd, uint8_t addr, uint8_t *wbuf, uint16_t wsize, uint8_t *rbuf, uint16_t rsize); \
extern int32_t kdi2c_write(kdi2c_t *kd, uint8_t addr, uint8_t *wbuf, uint16_t wsize); \
extern int32_t kdi2c_read(kdi2c_t *kd, uint8_t addr, uint8_t *rbuf, uint16_t rsize); \
extern int32_t kdi2c_regWrite(kdi2c_t *kd, uint8_t addr, uint32_t reg, uint8_t regLen, uint8_t *data, uint16_t len); \
extern int32_t kdi2c_regRead(kdi2c_t *kd, uint8_t addr, uint32_t reg, uint8_t regLen, uint8_t *data, uint16_t len); \
extern int32_t kdi2c_start(kdi2c_t *kd, uint8_t addr8RW); \
extern int32_t kdi2c_sendAndWaitAck(kdi2c_t *kd, uint8_t *wbuf, uint16_t wsize); \
extern int32_t kdi2c_stop(kdi2c_t *kd); \
    
#define KDI2CBB_MASTER_BLOCK_DEFINE(_moduleNumber, _name, \
        _clockSpeed, _delay, _timeout, \
        _sdaGpio, \
        _sclGpio, \
        _enableFunc, _disableFunc) \
    static void _KDI2C_FUNC_ENABLE(_name)(kdi2c_t *kd) _enableFunc \
    static void _KDI2C_FUNC_DISABLE(_name)(kdi2c_t *kd) _disableFunc \
    static kdi2c_VA_t _KDI2C_IVA(_name) = {0}; \
    const kdi2c_t _KDI2C_INAME(_name) = { \
        ._va = &_KDI2C_VA(_name), \
        ._config = { \
            .pin = { \
                .sda = { \
                    .gpio = (kdgpio_t *) _sdaGpio, \
                    .af = _sdaAf, \
                }, \
                .scl = { \
                    .gpio = (kdgpio_t *) _sclGpio, \
                    .af = _sclAf, \
                }, \
            }, \
        }, \
        ._instance = { \
            .i2c = NULL, \
            .enableFunc = _KDI2C_FUNC_ENABLE(_name), \
            .disableFunc = _KDI2C_FUNC_DISABLE(_name), \
        }, \
    }
    
#define KDI2C_MASTER_BLOCK_DEFINE(_moduleNumber, _name, \
        _clockSpeed, _delay, _timeout, _dutyCycle, \
        _sdaGpio, _sclGpio, \
        _dmaTx, _dmaChannelTx, _dmaRx, _dmaChannelRx, \
        _enableFunc, _disableFunc) \
    static void _KDI2C_FUNC_ENABLE(_name)(kdi2c_t *kd) _enableFunc \
    static void _KDI2C_FUNC_DISABLE(_name)(kdi2c_t *kd) _disableFunc \
    static kdi2c_VA_t _KDI2C_IVA(_name) = {0}; \
    const kdi2c_t _KDI2C_INAME(_name) = { \
        ._va = &_KDI2C_IVA(_name), \
        ._config = { \
            .pin = { \
                .sda = { \
                    .gpio = (kdgpio_t *) _sdaGpio, \
                }, \
                .scl = { \
                    .gpio = (kdgpio_t *) _sclGpio, \
                }, \
            }, \
            .dma = { \
                .channelTx = _dmaChannelTx, \
                .channelRx = _dmaChannelRx, \
                .initTx = { \
                    .periph_addr = (uint32_t) &I2C_DATA(_KDI2C_MODULE(_moduleNumber)), \
                    .memory_addr = 0, \
                    .direction = DMA_MEMORY_TO_PERIPHERAL, \
                    .memory_width = DMA_MEMORY_WIDTH_8BIT, \
                    .periph_width = DMA_PERIPHERAL_WIDTH_8BIT, \
                    .priority = DMA_PRIORITY_ULTRA_HIGH, \
                    .number = 0, \
                    .periph_inc = DMA_PERIPH_INCREASE_DISABLE, \
                    .memory_inc = DMA_MEMORY_INCREASE_ENABLE, \
                }, \
                .initRx = { \
                    .periph_addr = (uint32_t) &I2C_DATA(_KDI2C_MODULE(_moduleNumber)), \
                    .memory_addr = 0, \
                    .direction = DMA_PERIPHERAL_TO_MEMORY, \
                    .memory_width = DMA_MEMORY_WIDTH_8BIT, \
                    .periph_width = DMA_PERIPHERAL_WIDTH_8BIT, \
                    .priority = DMA_PRIORITY_ULTRA_HIGH, \
                    .number = 0, \
                    .periph_inc = DMA_PERIPH_INCREASE_DISABLE, \
                    .memory_inc = DMA_MEMORY_INCREASE_ENABLE, \
                }, \
            }, \
        }, \
        ._instance = { \
            .i2c = NULL, \
            .enableFunc = _KDI2C_FUNC_ENABLE(_name), \
            .disableFunc = _KDI2C_FUNC_DISABLE(_name), \
            .clockSpeed = _clockSpeed, \
            .dutyCycle = _dutyCycle, \
        }, \
    }

/*@}*/


#endif
