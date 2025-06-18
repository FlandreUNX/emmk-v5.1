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

#ifndef _KDGPIO_SOC_H_
#define _KDGPIO_SOC_H_

#include <stdint.h>

#include "emmk-config.h"

#include "HAL_gpio.h"
#include "HAL_exti.h"
#include "HAL_syscfg.h"

/**
 * @addtogroup Typedef
 * @note none
 */
 
/*@{*/

typedef struct kdgpio kdgpio_t;

typedef enum {
    KDGPIO_MODE_INPUT,
    KDGPIO_MODE_OUTPUT_PP,
    KDGPIO_MODE_OUTPUT_OD,
    
    KDGPIO_MODE_AF_PP,
    KDGPIO_MODE_AF_OD,
    KDGPIO_MODE_AIN,
} kdgpio_Mode_t;

typedef enum {
    KDGPIO_PULL_NONE = 0,
    KDGPIO_PULL_UP = 1,
    KDGPIO_PULL_DOWN = 2,
} kdgpio_PullResistor_t;

typedef enum {
    KDGPIO_DOWN_LEVEL_NONE,
    KDGPIO_DOWN_LEVEL_LOW,
    KDGPIO_DOWN_LEVEL_HIGH,
} kdgpio_DownLevel_t;

typedef enum {
    KDGPIO_TRIGGER_NONE,
    KDGPIO_TRIGGER_RISING,
    KDGPIO_TRIGGER_FALLING,
    KDGPIO_TRIGGER_RISING_FALLING,
    KDGPIO_TRIGGER_LOW,
    KDGPIO_TRIGGER_HIGH,
} kdgpio_EventTrigger_t;

typedef enum {
    KDGPIO_EVENT_RISING = KLBIT(0),
    KDGPIO_EVENT_FALLING = KLBIT(1),
    KDGPIO_EVENT_RISING_FALLING = KLBIT(2),
    KDGPIO_EVENT_LOW = KLBIT(3),
    KDGPIO_EVENT_HIGH = KLBIT(4),
    KDGPIO_EVENT_IRQ = KLBIT(5),
} kdgpio_Event_t;

typedef void (*kdgpio_SignalEvent_t)(kdgpio_t pin, uint32_t event);

typedef struct {
    kdgpio_SignalEvent_t cbEvt;
} kdgpio_Va_t;

struct kdgpio {
    kdgpio_Va_t *_va;
    struct {
        struct {
            GPIO_TypeDef *port;
            uint32_t pin;
            uint8_t number;
        } base;

        struct {
            kdgpio_Mode_t mode;
            kdgpio_PullResistor_t pull;
            kdgpio_DownLevel_t outputLevel;
        } downCfg;
        
        struct {
            uint32_t extiLinePin;
            uint32_t pinSourcePin;
            uint32_t pinSourcePort;
        } exti;
    } _config;
};

/*@}*/

/**
 * @addtogroup Instance define
 * @note none
 */
 
/*@{*/

#define _KDGPIO_NEXTERN(_name)      extern const kdgpio_t __kdgpio_##_name
#define KDGPIO_NEXTERN(_name)       _KDGPIO_NEXTERN(_name)
#define _KDGPIO_NGET(_name)         __kdgpio_##_name
#define KDGPIO_NGET(_name)          _KDGPIO_NGET(_name)
   
#define _KDGPIO_INAME(_name)            __kdgpio_##_name
#define _KDGPIO_IVA(_name)              __kdgpio_va_##_name

#define KDGPIO_PORT(_port)      _port
#define _KDGPIO_PORT(_port)     GPIO##_port

#define KDGPIO_PIN(_pin)        _pin
#define _KDGPIO_PIN(_pin)       GPIO_Pin_##_pin

#define _KDGPIO_EXTI_MODE(_pin)                 EXTI_Line##_pin
#define _KDGPIO_EXTI_PIN_SOURCE_PIN(_pin)       EXTI_PinSource##_pin
#define _KDGPIO_EXTI_PIN_SOURCE_PORT(_port)     EXTI_PortSourceGPIO##_port
    
extern int32_t kdgpio_init(kdgpio_t *kd);
extern int32_t kdgpio_finalize(kdgpio_t *kd);
extern int32_t kdgpio_powerUp(kdgpio_t *kd, kdgpio_Mode_t mode, kdgpio_PullResistor_t pull);
extern int32_t kdgpio_powerDown(kdgpio_t *kd);
extern void kdgpio_setPull(kdgpio_t *kd, kdgpio_PullResistor_t pull);
extern void kdgpio_setMode(kdgpio_t *kd, kdgpio_Mode_t mode);
extern void kdgpio_output(kdgpio_t *kd, uint8_t v);
extern void kdgpio_brr(kdgpio_t *kd);
extern void kdgpio_bsrr(kdgpio_t *kd);
extern uint32_t kdgpio_input(kdgpio_t *kd);
extern void kdgpio_toggle(kdgpio_t *kd);
extern void kdgpio_irqEnable(kdgpio_t *kd, kdgpio_EventTrigger_t t, kdgpio_SignalEvent_t cbEvent);
extern kdgpio_Event_t kdgpio_irqStatusSelect(kdgpio_t *kd);
extern void kdgpio_irqEventClean(kdgpio_t *kd, kdgpio_Event_t e);
    
#define KDGPIO_DEFINE(_port, _pin, _name, \
        _dmode, _dpull, _dlevel) \
    static kdgpio_Va_t _KDGPIO_IVA(_name) = {0}; \
    const kdgpio_t _KDGPIO_INAME(_name) = { \
        ._va = &_KDGPIO_IVA(_name), \
        ._config = { \
            .base = { \
                .port = _KDGPIO_PORT(_port), \
                .number = _pin, \
                .pin = _KDGPIO_PIN(_pin), \
            }, \
            .downCfg = { \
                .mode = _dmode, \
                .pull = _dpull, \
                .outputLevel = _dlevel, \
            }, \
            .exti = { \
                .extiLinePin = _KDGPIO_EXTI_MODE(_pin), \
                .pinSourcePin = _KDGPIO_EXTI_PIN_SOURCE_PIN(_pin), \
                .pinSourcePort = _KDGPIO_EXTI_PIN_SOURCE_PORT(_port), \
            }, \
        }, \
    };
    
#define KDGPIO_DEFINE_AF(_port, _pin, _name, \
        _dmode, _dpull, _dlevel) \
    const kdgpio_t _KDGPIO_INAME(_name) = { \
        ._va = NULL, \
        ._config = { \
            .base = { \
                .port = _KDGPIO_PORT(_port), \
                .number = _pin, \
                .pin = _KDGPIO_PIN(_pin), \
            }, \
            .downCfg = { \
                .mode = _dmode, \
                .pull = _dpull, \
                .outputLevel = _dlevel, \
            }, \
            .exti = { \
                .extiLinePin = _KDGPIO_EXTI_MODE(_pin), \
                .pinSourcePin = _KDGPIO_EXTI_PIN_SOURCE_PIN(_pin), \
                .pinSourcePort = _KDGPIO_EXTI_PIN_SOURCE_PORT(_port), \
            }, \
        }, \
    };
    
/*@}*/

extern void _gpio_modeConfig(GPIO_TypeDef *port, uint32_t pin, kdgpio_Mode_t mode);
extern void _gpio_pullConfig(GPIO_TypeDef *port, uint32_t pin, kdgpio_PullResistor_t pull);
extern void _gpio_output(GPIO_TypeDef *port, uint32_t pin, bool set);
extern void _gpio_afConfig(GPIO_TypeDef *port, uint32_t number, uint32_t af);
    
#endif
