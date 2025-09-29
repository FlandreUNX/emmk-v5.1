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

#include "emmk-config.h"

#include "kdgpio.h"

#include "../SWM180_StdPeriph_Driver/SWM180.h"
#include "SWM180_pwm.h"
#include "SWM180_gpio.h"
#include "SWM180_port.h"

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
} kdpwm_Va_t;

typedef struct {
    PWM_TypeDef *pwmx;
    uint32_t mode;
    uint8_t pwmNumber;
    uint32_t mask;
} kdpwm_Host_t;

struct kdpwm {
    kdpwm_Va_t *_va;
    
    struct {
        struct {
            uint8_t numberX;
            uint16_t cycle;
            uint16_t duty;
        } channel;
        
        struct {
            kdgpio_t *gpio;
            uint32_t af;
        } pin;
        
        const kdpwm_Host_t *host;
    } _config;
};

/*@}*/

/**
 * @addtogroup Instance define
 * @note none
 */
 
/*@{*/

#define _KDPWM_NEXTERN(_name)       extern const kdpwm_t __kdpwm_##_name
#define KDPWM_NEXTERN(_name)        _KDPWM_NEXTERN(_name)
#define _KDPWM_NGET(_name)          __kdpwm_##_name
#define KDPWM_NGET(_name)           _KDPWM_NGET(_name)

#define KDPWM_HOST_NUMBER(x)        x
#define _KDPWM_HOST(x)              __kdpwm_host_##_name
#define _KDPWM_HOST_NUMBER(x)       PWM##x
#define _KDPWM_HOST_CHN_MASK(x)     (PWMG_CHEN_PWM##x##A_Msk | PWMG_CHEN_PWM##x##B_Msk)

#define KDPWM_CHANNEL_NUMBER(x)     x

#define _KDPWM_INAME(_name)         __kdpwm_##_name
#define _KDPWM_IVA(_name)           __kdpwm_va_##_name
#define _KDPWM_FUNC_ENABLE(x)       __kdpwm_enable_##x
#define _KDPWM_FUNC_DISABLE(x)      __kdpwm_disable_##x

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


#define KDPWM_DEFINE(_name, _hostNumber, _channelNumber, \
        _cycle, _duty, \
        _gpio, _af) \
    static kdpwm_Va_t _KDPWM_IVA(_name) = {0}; \
    const kdpwm_t _KDPWM_INAME(_name) = { \
        ._va = &_KDPWM_IVA(_name), \
        ._config = { \
            .channel = { \
                .numberX = _channelNumber, \
                .cycle = _cycle + 1, \
                .duty = _duty, \
            }, \
            .host = (kdpwm_Host_t *) &_KDPWM_HOST(_hostNumber), \
            .pin = { \
                .gpio = (kdgpio_t *) &_gpio, \
                .af = _af, \
            }, \
        }, \
    };
    
#define KDPWM_HOST_DEFINE(_hostNumber, _mode) \
    static const kdpwm_Host_t _KDPWM_HOST(_hostNumber) = { \
        .pwmx = _KDPWM_HOST_NUMBER(_hostNumber), \
        .pwmNumber = _hostNumber, \
        .mode = _mode, \
        .mask = _KDPWM_HOST_CHN_MASK(_hostNumber), \
    };
    
#define KDPWM_G_DEFINE(_divClk) \
     const uint32_t gPWMDivClk = _divClk;

/*@}*/

#endif
