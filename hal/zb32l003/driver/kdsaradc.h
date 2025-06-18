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

#ifndef _KADC_SARADC_H_
#define _KADC_SARADC_H_

#include <stdint.h>

#include "emmk-config.h"

#include "kdgpio.h"

#ifdef CONFIG_USE_ZB32L003
#include "zb32l003.h"
#endif
#ifdef CONFIG_USE_ZB32L030
#include "zb32l030.h"
#endif

/**
 * @addtogroup Typedef
 * @note none
 */
 
/*@{*/

typedef struct kdsaradc kdsaradc_t;

typedef enum {
    KDSARADC_CHANNEL_0 = 0,
    KDSARADC_CHANNEL_1 = 1,
    KDSARADC_CHANNEL_2 = 2,
    KDSARADC_CHANNEL_3 = 3,
    KDSARADC_CHANNEL_4 = 4,
    KDSARADC_CHANNEL_5 = 5,
    KDSARADC_CHANNEL_6 = 6,
    
    KDSARADC_CHANNEL_FAKE_BASE = 80,
    
    KDSARADC_CHANNEL_RESERVER,   
} kdsaradc_Channel_t;

typedef struct {
    int8_t initRefs;
    int8_t startRefs;
} kdsaradc_HostVa_t;

typedef struct {
    kdsaradc_HostVa_t *_va;
    
    struct {
        uint8_t clockDiv;
        uint16_t refV;
    } init;

    void (*enableFunc)(kdsaradc_t *kd);
    void (*disableFunc)(kdsaradc_t *kd);
} kdsaradc_Host_t;

typedef struct {
    int8_t initRefs;
} kdsaradc_Va_t;

struct kdsaradc {
    kdsaradc_Va_t *_va;
    struct {
        kdsaradc_Host_t *host;
        
        kdsaradc_Channel_t channel;

        struct {
            kdgpio_t *gpio;
        } pin;
    } _config;
};

/*@}*/

/**
 * @addtogroup Instance define
 * @note none
 */
 
/*@{*/

#define KDSARADC_NEXTERN(_x)        extern const kdsaradc_t __kdsaradc_##_x
#define KDSARADC_NGET(_x)           __kdsaradc_##_x

extern int32_t kdsaradc_init(kdsaradc_t *kd);
extern int32_t kdsaradc_finalize(kdsaradc_t *kd);
extern int32_t kdsaradc_powerUp(kdsaradc_t *kd);
extern int32_t kdsaradc_powerDown(kdsaradc_t *kd);
extern uint16_t kdsaradc_getRaw(kdsaradc_t *kd);
extern float kdsaradc_convertRawToVolt(kdsaradc_t *kd, uint16_t raw);
extern uint8_t kdsaradc_isConvertCompleted(kdsaradc_t *kd, uint32_t wait);
extern void kdsaradc_convertStart(kdsaradc_t *kd);
extern void kdsaradc_convertStop(kdsaradc_t *kd);

#define KDSARADC_SEQINDEX(_c)    _c
#define _KDSARADC_SEQINDEX(_c)   _c

#define KDSARADC_SAMPLE_SELECT(_c)    _c
#define _KDSARADC_SAMPLE_SELECT(_c)   _c

#define KDSARADC_REFMV(_x)    _x

#define _KDSARADC_HOST      __kdsaradc_instance
#define _KDSARADC_HOST_VA   __kdsaradc_instance_va

#define _KDSARADC_INAME(_x)   __kdsaradc_##_x
#define _KDSARADC_IVA(_x)     __kdsaradc_va_##_x

#define KDSARADC_CLOCK_SRC(x)       x

#define KDSARADC_INSTANCE_CONTINUE_DEFINE(_clockSrc, \
        _refV, \
        _enableFunc, _disableFunc) \
    static void __kdsaradc_enableFunc(kdsaradc_t *kd) _enableFunc \
    static void __kdsaradc_disableFunc(kdsaradc_t *kd) _disableFunc \
    static kdsaradc_HostVa_t _KDSARADC_HOST_VA = {0}; \
    static const kdsaradc_Host_t _KDSARADC_HOST = { \
        ._va = &_KDSARADC_HOST_VA, \
        .init = { \
            .clockDiv = _clockSrc, \
            .refV = _refV, \
        }, \
        .enableFunc = __kdsaradc_enableFunc, \
        .disableFunc = __kdsaradc_disableFunc, \
    };
    
#define KDSARADC_NOBLOCK_DEFINE(_chn, _name, \
        _gpio) \
    static kdsaradc_Va_t _KDSARADC_IVA(_name) = {0}; \
    const kdsaradc_t _KDSARADC_INAME(_name) = { \
        ._va = (kdsaradc_Va_t *) &_KDSARADC_IVA(_name), \
        ._config = { \
            .channel = _chn, \
            .host = (kdsaradc_Host_t *) &_KDSARADC_HOST, \
            .pin = { \
                .gpio = ((kdgpio_t *) _gpio), \
            }, \
        }, \
    }; 
    
#define KDSARADC_REUSE_DEFINE(_chn, _name, \
        _gpio) \
    const kdsaradc_t _KDSARADC_INAME(_name) = { \
        ._va = NULL, \
        ._config = { \
            .channel = KDSARADC_CHANNEL_FAKE_BASE + _chn, \
            .host = (kdsaradc_Host_t *) &_KDSARADC_HOST, \
            .pin = { \
                .gpio = ((kdgpio_t *) _gpio), \
            }, \
        }, \
    }; 

/*@}*/

#endif
