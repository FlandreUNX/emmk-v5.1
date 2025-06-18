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

#include <stdint.h>

#include <driver/i2c.h>

#include "emmk-config.h"

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
                kdgpio_t *gpio;
            } sda;

            struct {
                kdgpio_t *gpio;
            } scl;
        } pinConfig;
    } _config;

    struct {
        uint32_t i2c;
        uint32_t clockSpeed;
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

#define KDI2C_PIN_NUM(_pinNumber)       _pinNumber
#define _KDI2C_PIN_NUM(_pinNumber)      GPIO_NUM_##_pinNumber
#define KDI2C_PIN_PULLUP(_x)            _x

#define KDI2C_MODULE(_number)           _number
#define _KDI2C_MODULE(_number)          I2C_NUM_##_number
#define _KDI2C(x)                       __kdi2c_##x
#define _KDI2C_IVA(x)                   __kdi2c_va_##x
#define _KDI2C_INAME(x)                 __kdi2c_##x

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
        _clockSpeed, \
        _sdaGpio, _sclGpio) \
    static kdi2c_Va_t _KDI2C_IVA(_name) = {0}; \
    const kdi2c_t _KDI2C_INAME(_name) = { \
        ._va = &_KDI2C_IVA(_name), \
        ._config = { \
            .pinConfig = { \
                .sda = { \
                    .gpio = (kdgpio_t *) _sdaGpio, \
                }, \
                .scl = { \
                    .gpio = (kdgpio_t *) _sclGpio, \
                }, \
            }, \
        }, \
        ._instance = { \
            .i2c = _KDI2C_MODULE(_moduleNumber),        \
            .clockSpeed = _clockSpeed, \
        }, \
    };

/*@}*/

#endif
