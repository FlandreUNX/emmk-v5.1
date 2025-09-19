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

#ifndef _KDPWM_SOC_H_
#define _KDPWM_SOC_H_

#include "emmk-config.h"

#include "kdgpio.h"

#include "CH59x_gpio.h"
#include "CH59x_pwm.h"
#include "CH59x_timer.h"

/**
 * @addtogroup Typedef 
 * @note none
 */
 
/*@{*/

typedef struct kdpwm kdpwm_t;

typedef void (*kdpwm_SignalEvent_t)(kdpwm_t pin, uint32_t event);

typedef enum {
    KDPWM_EVENT_NONE = 0,
    KDPWM_EVENT_UPDATE = KLBIT(0),
    KDPWM_EVENT_PWM_ACT_END = KLBIT(1),
} kdpwm_Event_t;

typedef struct {
    int8_t initRefs;
    
    uint8_t cbEventRegistedCount;
    kdpwm_SignalEvent_t cbEvent;
} kdpwm_HostVa_t;

typedef enum {
    KDPWM_AUTO_RELOAD_ENABLE = 1,
    KDPWM_AUTO_RELOAD_DISABLE = 0,
} kdpwm_AutoReload_t;

typedef enum {
    KDPWM_CHN_PRELOAD_ENABLE = 1,
    KDPWM_CHN_PRELOAD_DISABLE = 0,
} kdpwm_ChnPreLoad_t;

typedef enum {
    KDPWM_OUTPUT_POL_H = 0,
    KDPWM_OUTPUT_POL_L = 1,
} kdpwm_OutputPol_t;

typedef struct {
    int8_t initRefs;
} kdpwm_ChannelVa_t;

typedef struct {
    kdpwm_HostVa_t *va;
    
    void (*enableFunc)(kdpwm_t *kd);
    void (*disableFunc)(kdpwm_t *kd);
    int (*isInitFunc)(kdpwm_t *kd);

    uint8_t isTimx;
    uint32_t sysFreq;

    union {
        uint32_t timx;
        uint32_t pwmx;
    } ins;

    union {
        struct {
            uint8_t outputPol;
            uint16_t arr;
        } timx;
    } cfg;
} kdpwm_Host_t;

struct kdpwm {
    kdpwm_ChannelVa_t *_va;
    
    struct {
        struct {
            struct {
                uint8_t preLoad;
            } init;
            uint8_t index;
        } channel;

        struct {
            kdgpio_t *gpio;
        } pin;
    } _config;
    
    kdpwm_Host_t *_host;
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

#define KDPWM_HOST_NUMBER(x)            x
#define _KDPWM_HOST_NUMBER(x)           BA_TMR##x
#define _KDPWM_HOST_INAME(_name)        __kdpwm_host_##_name
#define _KDPWM_HOST_IVA(_hn)            __kdpwm_host_va_##_hn
#define _KDPWM_HOST_FUNC_ENABLE(x)      __kdpwm_host_enable_##x
#define _KDPWM_HOST_FUNC_DISABLE(x)     __kdpwm_host_disable_##x
#define _KDPWM_HOST_FUNC_IS_INIT(x)     __kdpwm_host_isInit_##x

#define KDPWM_CHN_NUMBER(x)             x
#define _KDPWM_CHN_INAME(_name)         __kdpwm_##_name
#define _KDPWM_CHN_IVA(_name)           __kdpwm_va_##_name

#define KDPWM_PRESCALER(x)      x
#define KDPWM_SYSFREQ(x)      x
#define KDPWM_PERIOD(x)         x
#define KDPWM_PRELOAD(x)         x

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

#define KDPWM_CHN_DEFINE(_name, _thost, _chnNumber, \
        _gpio, _preLoad) \
    static kdpwm_ChannelVa_t _KDPWM_CHN_IVA(_name) = {0}; \
    const kdpwm_t _KDPWM_CHN_INAME(_name) = { \
        ._va = &_KDPWM_CHN_IVA(_name), \
        ._host = (void *) &_thost, \
        ._config = { \
            .channel = { \
                .init = { \
                    .preLoad = _preLoad, \
                }, \
                .index = KDPWM_CHN_NUMBER(_chnNumber) - 1, \
            }, \
            .pin = { \
                .gpio = (kdgpio_t *) _gpio, \
            }, \
        }, \
    };


#define KDPWM_DEFINE_TMR(_hn, \
        _sysFreq, \
        _arr, _outputPol, \
        _enableFunc, _disableFunc, _isInitFunc) \
    static void _KDPWM_HOST_FUNC_ENABLE(_hn)(kdpwm_t *kd)  _enableFunc \
    static void _KDPWM_HOST_FUNC_DISABLE(_hn)(kdpwm_t *kd)  _disableFunc \
    static int _KDPWM_HOST_FUNC_IS_INIT(_hn)(kdpwm_t *kd)  _isInitFunc \
    static kdpwm_HostVa_t _KDPWM_HOST_IVA(_hn) = {0}; \
    const kdpwm_Host_t _KDPWM_HOST_INAME(_hn) = { \
        .enableFunc = _KDPWM_HOST_FUNC_ENABLE(_hn), \
        .disableFunc = _KDPWM_HOST_FUNC_DISABLE(_hn), \
        .isInitFunc = _KDPWM_HOST_FUNC_IS_INIT(_hn), \
        .va = &_KDPWM_HOST_IVA(_hn), \
        .isTimx = 1, \
        .sysFreq = _sysFreq, \
        .ins = { \
            .timx = (uint32_t) _KDPWM_HOST_NUMBER(_hn), \
        }, \
        .cfg = { \
            .timx = { \
                .arr = _arr, \
                .outputPol = _outputPol, \
            }, \
        }, \
    };

/*@}*/

#endif
