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

#ifndef _KPWM_SOC_H_
#define _KPWM_SOC_H_

#include <stdint.h>
#include <driver/ledc.h>

#include "emmk-config.h"

#include "kdgpio.h"

/**
 * @addtogroup Typedef 
 * @note none
 */
 
/*@{*/

typedef struct kdpwm kdpwm_t;

typedef void (*kdpwm_SignalEvent_t)(kdpwm_t pin, uint32_t event);

typedef struct {
    int8_t initRefs;
} kdpwm_InstanceVa_t;

typedef struct {
    kdpwm_InstanceVa_t *va;
    ledc_timer_config_t config;
    int (*isInitFunc)(void *kd);
} kdpwm_Instance_t;

typedef enum {
    KDPWM_EVENT_NONE = 0,
    KDPWM_EVENT_UPDATE = KLBIT(0),
} kdpwm_Event_t;

typedef struct {
    int8_t initRefs;
} kdpwm_Va_t;

struct kdpwm {
    kdpwm_Va_t *_va;
    
    struct {
        ledc_channel_config_t channel;
        uint32_t idleLevel;

        const kdgpio_t *gpio;
    } _config;

    const kdpwm_Instance_t *_instance;
};

/*@}*/

/**
 * @addtogroup Instance define
 * @note none
 */
 
/*@{*/

#define _KDPWM_NEXTERN(_name)     extern const kdpwm_t __kdpwm_##_name
#define _KDPWM_NGET(_name)        __kdpwm_##_name
#define KDPWM_NEXTERN(_name)     _KDPWM_NEXTERN(_name)
#define KDPWM_NGET(_name)        _KDPWM_NGET(_name)

#define KDPWM_CFG_TIMER_NUM(x)      x
#define KDPWM_CFG_SPEED_MODE(x)      x
#define KDPWM_CFG_DUTY_RESOLUTIN(x)      x
#define KDPWM_CFG_FREQ(x)      x
#define KDPWM_CFG_CLK_MODE(x)      x

#define KDPWM_CHN_CFG_SPEED_MODE(x)      x
#define KDPWM_CHN_CFG_CHANNEL(x)      x
#define KDPWM_CHN_CFG_TIMER_SEL(x)      x
#define KDPWM_CHN_CFG_INIT_DUTY(x)      x
#define KDPWM_CHN_CFG_IDLE_LEVEL(x)      x

#define _KDPWM_CFG_TIMER_NUM(x)      LEDC_TIMER_##x

#define _KDPWM_INSTANCE(_hostNumber)        __kdpwm_instance_##_hostNumber
#define _KDPWM_INAME(_name)                 __kdpwm_##_name
#define _KDPWM_IVA(_name)                    __kdpwm_va_##_name
#define _KDPWM_INSTANCE_VA(_name)           __kdpwm_instance_va_##_name
#define _KDPWM_INSTANCE_IS_INIT(_name)           __kdpwm_instance_isInitFunc_##_name

extern int32_t kdpwm_init(kdpwm_t *kd);
extern int32_t kdpwm_finalize(kdpwm_t *kd);
extern int32_t kdpwm_powerUp(kdpwm_t *kd);
extern int32_t kdpwm_powerDown(kdpwm_t *kd);
extern void kdpwm_setPrescaler(kdpwm_t *kd, uint32_t prescaler);
extern void kdpwm_setAutoReload(kdpwm_t *kd, uint32_t autoReload);
extern void kdpwm_setDuty(kdpwm_t *kd, uint32_t duty);
extern uint32_t kdpwm_getDuty(kdpwm_t *kd);
extern uint32_t kdpwm_getAutoReload(kdpwm_t *kd);
extern void kdpwm_setFreq(kdpwm_t *kd, uint32_t freq);
extern void kdpwm_counter(kdpwm_t *kd, uint8_t enable);
extern void kdpwm_counterReset(kdpwm_t *kd);
extern void kdpwm_irqEnable(kdpwm_t *kd, kdpwm_Event_t evt, bool enable, kdpwm_SignalEvent_t cbEvent);


#define KDPWM_CHANNEL_DEFINE(_name, _timerNum, _channel, _speedMode, _duty, _idleLevel, _gpio) \
    static kdpwm_Va_t _KDPWM_IVA(_name) = {0}; \
    const kdpwm_t _KDPWM_INAME(_name) = { \
        ._va = &_KDPWM_IVA(_name), \
        ._config = {                                                                  \
            .idleLevel = _idleLevel, \
            .channel = {                                                   \
                 .speed_mode = _speedMode,                                     \
                 .channel = _channel,                                          \
                 .timer_sel = _KDPWM_CFG_TIMER_NUM(_timerNum),                                       \
                 .intr_type = LEDC_INTR_DISABLE, \
                 .gpio_num = (_gpio)->_config.base.number, \
                 .duty = _duty, \
                 .hpoint = 0 \
            },                                                              \
            .gpio = _gpio,                                                                    \
        },                                                                  \
        ._instance = &_KDPWM_INSTANCE(_timerNum), \
    };

#define KDPWM_DEFINE(_timerNum, _speedMode, _dutyRes, _freq, _clkCfg, _isInitFunc) \
    static kdpwm_InstanceVa_t _KDPWM_INSTANCE_VA(_timerNum) = {0}; \
    static int _KDPWM_INSTANCE_IS_INIT(_timerNum)(void *kd) _isInitFunc \
    const kdpwm_Instance_t _KDPWM_INSTANCE(_timerNum) = { \
        .va = &_KDPWM_INSTANCE_VA(_timerNum), \
        .isInitFunc = &_KDPWM_INSTANCE_IS_INIT(_timerNum), \
        .config = {                                                   \
            .speed_mode = _speedMode,                                 \
            .timer_num = _KDPWM_CFG_TIMER_NUM(_timerNum),                                   \
            .duty_resolution = _dutyRes,                              \
            .freq_hz = _freq,                                         \
            .clk_cfg = _clkCfg,\
        }, \
    };
    
/*@}*/

#endif
