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

#include "cw32l010.h"
#include "cw32l010_gpio.h"
#include "cw32l010_atim.h"
#include "cw32l010_gtim.h"

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

typedef enum {
    KDPWM_AUTO_RELOAD_ENABLE = 1,
    KDPWM_AUTO_RELOAD_DISABLE = 0,
} kdpwm_AutoReload_t;

typedef enum {
    KDPWM_CHN_PRELOAD_ENABLE = 1,
    KDPWM_CHN_PRELOAD_DISABLE = 0,
} kdpwm_ChnPreLoad_t;

typedef struct {
    int8_t initRefs;
} kdpwm_ChannelVa_t;

typedef struct {
    kdpwm_HostVa_t *va;
    
    void (*enableFunc)(kdpwm_t *kd);
    void (*disableFunc)(kdpwm_t *kd);
    
    union {
        ATIM_TypeDef *atim;
        GTIM_TypeDef *gtim;
    } tim;
    uint8_t isAtim;
    
    uint8_t autoReload;
    uint16_t psc;
    uint16_t arr;
} kdpwm_Host_t;

struct kdpwm {
    kdpwm_ChannelVa_t *_va;
    
    struct {
        struct {
            struct {
                uint8_t mode;
                uint8_t pol;
                uint8_t polN;
                uint8_t preLoad;
            } init;
            uint8_t index;
        } channel;

        struct {
            kdgpio_t *gpio;
            uint8_t af;
        } pin;
        
        struct {
            kdgpio_t *gpio;
            uint8_t af;
        } pinN;
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
#define _KDPWM_HOST_NUMBER_GTIM(x)      CW_GTIM##x
#define _KDPWM_HOST_NUMBER_ATIM(x)      CW_ATIM
#define _KDPWM_HOST_INAME(_name)        __kdpwm_host_##_name
#define _KDPWM_HOST_IVA(_hn)            __kdpwm_host_va_##_hn
#define _KDPWM_HOST_FUNC_ENABLE(x)      __kdpwm_host_enable_##x
#define _KDPWM_HOST_FUNC_DISABLE(x)     __kdpwm_host_disable_##x

#define KDPWM_CHN_NUMBER(x)             x
#define _KDPWM_CHN_INAME(_name)         __kdpwm_##_name
#define _KDPWM_CHN_IVA(_name)           __kdpwm_va_##_name

#define KDPWM_PIN_AF(_af)   _af
#define _KDPWM_PIN_AF(_af)  _af

#define KDPWM_PRESCALER(x)      x
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
        _gpio, _af, _gpioN, _afN, \
        _mode, _pol, _polN, _preLoad) \
    static kdpwm_ChannelVa_t _KDPWM_CHN_IVA(_name) = {0}; \
    const kdpwm_t _KDPWM_CHN_INAME(_name) = { \
        ._va = &_KDPWM_CHN_IVA(_name), \
        ._host = (void *) &_thost, \
        ._config = { \
            .channel = { \
                .init = { \
                    .mode = _mode, \
                    .pol = _pol, \
                    .preLoad = _preLoad, \
                }, \
                .index = KDPWM_CHN_NUMBER(_chnNumber) - 1, \
            }, \
            .pin = { \
                .gpio = (kdgpio_t *) _gpio, \
                .af = _KDPWM_PIN_AF(_af), \
            }, \
            .pinN = { \
                .gpio = (kdgpio_t *) _gpioN, \
                .af = _KDPWM_PIN_AF(_afN), \
            }, \
        }, \
    };


#define KDPWM_DEFINE_ATIM(_hn, \
        _psc, _arr, _autoReload, \
        _enableFunc, _disableFunc) \
    static void _KDPWM_HOST_FUNC_ENABLE(_hn)(kdpwm_t *kd)  _enableFunc \
    static void _KDPWM_HOST_FUNC_DISABLE(_hn)(kdpwm_t *kd)  _disableFunc \
    static kdpwm_HostVa_t _KDPWM_HOST_IVA(_hn) = {0}; \
    const kdpwm_Host_t _KDPWM_HOST_INAME(_hn) = { \
        .enableFunc = _KDPWM_HOST_FUNC_ENABLE(_hn), \
        .disableFunc = _KDPWM_HOST_FUNC_DISABLE(_hn), \
        .tim.atim = _KDPWM_HOST_NUMBER_ATIM(_hn), \
        .va = &_KDPWM_HOST_IVA(_hn), \
        .psc = _psc, \
        .arr = _arr, \
        .autoReload = _autoReload, \
        .isAtim = true, \
    };

#define KDPWM_DEFINE_GTIM(_hn, \
        _psc, _arr, _autoReload, \
        _enableFunc, _disableFunc) \
    static void _KDPWM_HOST_FUNC_ENABLE(_hn)(kdpwm_t *kd)  _enableFunc \
    static void _KDPWM_HOST_FUNC_DISABLE(_hn)(kdpwm_t *kd)  _disableFunc \
    static kdpwm_HostVa_t _KDPWM_HOST_IVA(_hn) = {0}; \
    const kdpwm_Host_t _KDPWM_HOST_INAME(_hn) = { \
        .enableFunc = _KDPWM_HOST_FUNC_ENABLE(_hn), \
        .disableFunc = _KDPWM_HOST_FUNC_DISABLE(_hn), \
        .tim.gtim = _KDPWM_HOST_NUMBER_GTIM(_hn), \
        .va = &_KDPWM_HOST_IVA(_hn), \
        .psc = _psc, \
        .arr = _arr, \
        .autoReload = _autoReload, \
        .isAtim = false, \
    };
    
/*@}*/

#endif
