//
// Created by unx on 2023/4/29.
//

#ifndef GET_STARTED_KDGPIO_H
#define GET_STARTED_KDGPIO_H

#include "driver/gpio.h"

#include "emmk-config.h"

/**
 * @addtogroup Typedef
 * @note none
 */

/*@{*/

typedef struct kdgpio kdgpio_t;

typedef enum {
    KDGPIO_MODE_INPUT = GPIO_MODE_INPUT,
    KDGPIO_MODE_OUTPUT_PP = GPIO_MODE_OUTPUT,
    KDGPIO_MODE_OUTPUT_OD = GPIO_MODE_OUTPUT_OD,

    KDGPIO_MODE_AF_PP = GPIO_MODE_INPUT_OUTPUT,
    KDGPIO_MODE_AF_OD = GPIO_MODE_INPUT_OUTPUT_OD,

    KDGPIO_MODE_AIN = GPIO_MODE_DISABLE,
} kdgpio_Mode_t;

typedef enum {
    KDGPIO_PULL_NONE,
    KDGPIO_PULL_UP,
    KDGPIO_PULL_DOWN,
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
            uint64_t pin;
            uint8_t number;
        } base;

        struct {
            kdgpio_Mode_t mode;
            kdgpio_PullResistor_t pull;
            kdgpio_DownLevel_t outputLevel;
        } downCfg;
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

#define _KDGPIO_NUM(_pinNumber)     GPIO_NUM_##_pinNumber
#define KDGPIO_NUM(_pinNumber)      _pinNumber

#define KDGPIO_PU(x)      x
#define KDGPIO_PD(x)      x

#define _KDGPIO(_pin, _name)  __kdgpio_##_pin##_##_name
#define _KDGPIO_INAME(_name)  __kdgpio_##_name
#define _KDGPIO_VA(_pin)  __kdgpio_va_##_pin

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

extern void esp32_kdgpio_irqHandleReg(kdgpio_t *kd, void (*gpio_isr_t)(void *arg), void *arg);

#define KDGPIO_DEFINE(_pinNumber, _name, \
        _dmode, _dpull, _dlevel) \
    const kdgpio_t _KDGPIO_INAME(_name) = { \
        ._config = { \
            .base = { \
                .pin = BIT64(_KDGPIO_NUM(_pinNumber)), \
                .number = _KDGPIO_NUM(_pinNumber), \
            }, \
            .downCfg = { \
                .mode = _dmode, \
                .pull = _dpull, \
                .outputLevel = _dlevel, \
            }, \
        }, \
    };

/*@}*/

extern void _gpio_modeConfig(gpio_config_t *ioConfig, kdgpio_Mode_t mode);
extern void _gpio_pullConfig(gpio_config_t *ioConfig, kdgpio_PullResistor_t pull);

#endif //GET_STARTED_KDGPIO_H
