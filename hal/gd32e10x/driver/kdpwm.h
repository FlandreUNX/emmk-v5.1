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

#include "gd32e10x_timer.h"
#include "gd32e10x_gpio.h"

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
} kdpwm_Event_t;

typedef struct {
    int8_t initRefs;
    
    uint8_t cbEventRegistedCount;
    kdpwm_SignalEvent_t cbEvent;
} kdpwm_HostVa_t;

typedef struct {
    int8_t initRefs;
} kdpwm_ChannelVa_t;

typedef struct {
    kdpwm_HostVa_t *va;
    
    void (*enableFunc)(kdpwm_t *kd);
    void (*disableFunc)(kdpwm_t *kd);
    
    uint32_t timer;
    timer_parameter_struct init;
    
    uint8_t autoReload;
} kdpwm_Host_t;

struct kdpwm {
    kdpwm_ChannelVa_t *_va;
    
    struct {
        struct {
            timer_oc_parameter_struct oc;
            uint32_t mode;
            uint32_t index;
            uint8_t shadowRelaod;
        } channel;

        struct {
            kdgpio_t *gpio;
        } pin;
        
        struct {
            kdgpio_t *gpio;
        } pinN;
    } _config;
    
    const kdpwm_Host_t *_host;
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
#define _KDPWM_HOST_NUMBER_TIM(x)       TIMER##x
#define _KDPWM_HOST_INAME(_name)        __kdpwm_host_##_name
#define _KDPWM_HOST_IVA(_hn)            __kdpwm_host_va_##_hn
#define _KDPWM_HOST_FUNC_ENABLE(x)      __kdpwm_host_enable_##x
#define _KDPWM_HOST_FUNC_DISABLE(x)     __kdpwm_host_disable_##x

#define _KDPWM_CHN_INAME(_name)         __kdpwm_chn_##_name
#define _KDPWM_CHN_IVA(_name)           __kdpwm_chn_va__##_name

#define KDPWM_CHANNEL_NUMBER(_x)         _x
#define _KDPWM_CHN(_c)                  TIMER_CH_##_c
#define _KDPWM_CHNX(_c, _isN)           ((_isN ? 0x04 : 0x01) << (_c * 4))

#define KDPWM_PRESCALER(x) x
#define KDPWM_PERIOD(x) x

#define KDPWM_AUTO_RELOAD(x) x

#define KDPWM_SHADOW_RELOAD(x) x

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

#define KDPWM_CHN_DEFINE(_name, _hn, _chnNumber, \
        _gpio, _gpioN, \
        _mode, \
        _pol, _idle, \
        _polN, _idleN, \
        _shadowReload) \
    static kdpwm_ChannelVa_t _KDPWM_CHN_IVA(_name) = {0}; \
    const kdpwm_t _KDPWM_CHN_INAME(_name) = { \
        ._va = &_KDPWM_CHN_IVA(_name), \
        ._host = &_KDPWM_HOST_INAME(_hn), \
        ._config = { \
            .channel = { \
                .shadowRelaod = _shadowReload, \
                .mode = _mode, \
                .oc = { \
                    .ocpolarity = _pol, \
                    .ocnpolarity = _polN, \
                    .outputstate = TIMER_CCX_DISABLE, \
                    .outputnstate = TIMER_CCXN_DISABLE, \
                    .ocidlestate = _idle, \
                    .ocnidlestate = _idleN, \
                }, \
                .index = _KDPWM_CHN(_chnNumber), \
            }, \
            .pin = { \
                .gpio = (kdgpio_t *) _gpio, \
            }, \
            .pinN = { \
                .gpio = (kdgpio_t *) _gpioN, \
            }, \
        }, \
    };


#define KDPWM_DEFINE_TIM(_hn, \
        _psc, _arr, _autoReload, \
        _enableFunc, _disableFunc) \
    static void _KDPWM_HOST_FUNC_ENABLE(_hn)(kdpwm_t *kd)  _enableFunc \
    static void _KDPWM_HOST_FUNC_DISABLE(_hn)(kdpwm_t *kd)  _disableFunc \
    static kdpwm_HostVa_t _KDPWM_HOST_IVA(_hn) = {0}; \
    const kdpwm_Host_t _KDPWM_HOST_INAME(_hn) = { \
        .enableFunc = _KDPWM_HOST_FUNC_ENABLE(_hn), \
        .disableFunc = _KDPWM_HOST_FUNC_DISABLE(_hn), \
        .timer = _KDPWM_HOST_NUMBER_TIM(_hn), \
        .va = &_KDPWM_HOST_IVA(_hn), \
        .init = { \
            .period = _arr - 1, \
            .clockdivision = TIMER_CKDIV_DIV1, \
            .counterdirection = TIMER_COUNTER_UP, \
            .prescaler = _psc - 1, \
            .alignedmode = TIMER_COUNTER_EDGE, \
            .repetitioncounter = 0, \
        }, \
        .autoReload = _autoReload, \
    };

    
/*@}*/

#endif
