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
 
#ifndef _KI2C_SOC_H_
#define _KI2C_SOC_H_

#include "emmk-config.h"

#include "kdgpio.h"

#include "../SWM180_StdPeriph_Driver/SWM180.h"
#include "SWM180_i2c.h"
#include "SWM180_gpio.h"                // Synwit::Device:Driver:GPIO
#include "SWM180_port.h"                // Synwit::Device:Driver:Port

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
    void *mutex;
    
    int8_t initRefs;
    int8_t initRefsPower;
} kdi2c_Va_t;

typedef struct kdi2c kdi2c_t;

struct kdi2c {
    kdi2c_Va_t *_va;
    
    struct {
        struct {
            struct {
                uint32_t af;
                kdgpio_t *gpio;
            } sda;
            
            struct {
                uint32_t af;
                kdgpio_t *gpio;
            } scl;
        } pin;
        
        struct {
            uint32_t delay;
            uint32_t timeout;
        } bb;
    } _config;
    
    struct {
        I2C_TypeDef *i2c;
        I2C_InitStructure init;
        
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

#define _KDI2C_NGET(x)      __kdi2c_##x
#define _KDI2C_NEXTERN(x)    extern const kdi2c_t __kdi2c_##x
#define KDI2C_NGET(x)        _KDI2C_NGET(x)
#define KDI2C_NEXTERN(x)    _KDI2C_NEXTERN(x)

#define KDI2C_MODULE(_number)       _number
#define _KDI2C_MODULE(_number)      I2C##_number

#define _KDI2C_IVA(x)               __kdi2c_va_##x
#define _KDI2C_INAME(x)             __kdi2c_##x

#define _KDI2C_FUNC_ENABLE(x)   __kdi2c_enable_##x
#define _KDI2C_FUNC_DISABLE(x)  __kdi2c_disable_##x

#define KDI2CBB_DELAY(x)     x
#define KDI2CBB_TIMEOUT(x)   x

extern int32_t kdi2c_init(kdi2c_t *kd);
extern int32_t kdi2c_finalize(kdi2c_t *kd);
extern int32_t kdi2c_powerUp(kdi2c_t *kd);
extern int32_t kdi2c_powerDown(kdi2c_t *kd);
extern void kdi2c_reset(kdi2c_t *kd);
extern void kdi2c_setSpeed(kdi2c_t *kd, uint32_t speed);
extern int32_t kdi2c_transmit(kdi2c_t *kd, uint8_t addr, uint8_t *wbuf, uint16_t wsize, uint8_t *rbuf, uint16_t rsize);
extern int32_t kdi2c_write(kdi2c_t *kd, uint8_t addr, uint8_t *wbuf, uint16_t wsize);
extern int32_t kdi2c_read(kdi2c_t *kd, uint8_t addr, uint8_t *rbuf, uint16_t rsize);
extern int32_t kdi2c_regWrite(kdi2c_t *kd, uint8_t addr, uint32_t reg, uint8_t regLen, uint8_t *data, uint16_t len);
extern int32_t kdi2c_regRead(kdi2c_t *kd, uint8_t addr, uint32_t reg, uint8_t regLen, uint8_t *data, uint16_t len);
extern int32_t kdi2c_start(kdi2c_t *kd, uint8_t addr8RW);
extern int32_t kdi2c_sendAndWaitAck(kdi2c_t *kd, uint8_t *wbuf, uint16_t wsize);
extern int32_t kdi2c_stop(kdi2c_t *kd); 

#define KDI2C_MASTER_BLOCK_DEFINE(_moduleNumber, _name, \
        _clockSpeed, _delay, _timeout, \
        _sdaGpio, _sdaAf, \
        _sclGpio, _sclAf, \
        _enableFunc, _disableFunc) \
    static void _KDI2C_FUNC_ENABLE(_name)(kdi2c_t *kd) _enableFunc \
    static void _KDI2C_FUNC_DISABLE(_name)(kdi2c_t *kd) _disableFunc \
    static kdi2c_Va_t _KDI2C_IVA(_name) = {0}; \
    const kdi2c_t _KDI2C_INAME(_name) = { \
        ._va = &_KDI2C_IVA(_name), \
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
            .i2c = _KDI2C_MODULE(_moduleNumber), \
            .init = { \
                .Master = 1, \
                .Addr7b = 1, \
                .MstClk = _clockSpeed, \
                .MstIEn = 0, \
            }, \
            .enableFunc = _KDI2C_FUNC_ENABLE(_name), \
            .disableFunc = _KDI2C_FUNC_DISABLE(_name), \
        }, \
    };
    
    
#define KDI2CBB_MASTER_BLOCK_DEFINE(_moduleNumber, _name, \
        _clockSpeed, _delay, _timeout, \
        _sdaGpio, _sdaAf, \
        _sclGpio, _sclAf, \
        _enableFunc, _disableFunc) \
    static void _KDI2C_FUNC_ENABLE(_name)(kdi2c_t *kd) _enableFunc \
    static void _KDI2C_FUNC_DISABLE(_name)(kdi2c_t *kd) _disableFunc \
    static kdi2c_Va_t _KDI2C_IVA(_name) = {0}; \
    const kdi2c_t _KDI2C_INAME(_name) = { \
        ._va = &_KDI2C_IVA(_name), \
        ._config = { \
            .bb = { \
                .delay = _delay, \
                .timeout = _timeout, \
            }, \
            .pin = { \
                .sda = { \
                    .gpio = _sdaGpio, \
                    .af = _sdaAf, \
                }, \
                .scl = { \
                    .gpio = _sclGpio, \
                    .af = _sclAf, \
                }, \
            }, \
        }, \
        ._instance = { \
            .i2c = NULL, \
            .enableFunc = _KDI2C_FUNC_ENABLE(_name), \
            .disableFunc = _KDI2C_FUNC_DISABLE(_name), \
        }, \
    };

/*@}*/

#endif
