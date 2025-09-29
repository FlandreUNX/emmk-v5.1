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

#include "kdgpio.h"

#include "../SWM180_StdPeriph_Driver/SWM180.h"
#include "SWM180_adc.h"                 // Synwit::Device:Driver:ADC
#include "SWM180_gpio.h"                // Synwit::Device:Driver:GPIO
#include "SWM180_port.h"                // Synwit::Device:Driver:Port

/**
 * @addtogroup Typedef
 * @note none
 */
 
/*@{*/

typedef struct kdsaradc kdsaradc_t;

typedef enum {
    KDSARADC_CHANNEL_0 = ADC_CH0,
    KDSARADC_CHANNEL_1 = ADC_CH1,
    KDSARADC_CHANNEL_2 = ADC_CH2,
    KDSARADC_CHANNEL_3 = ADC_CH3,
    KDSARADC_CHANNEL_4 = ADC_CH4,
    KDSARADC_CHANNEL_5 = ADC_CH5,
    KDSARADC_CHANNEL_6 = ADC_CH6,
    KDSARADC_CHANNEL_7 = ADC_CH7,
} kdsaradc_Channel_t;

typedef enum {
    KDSARADC_CONTINUE_MODE_1 = 1,
    KDSARADC_CONTINUE_MODE_0 = 0,
} kdsaradc_ContinueMode_t;

typedef enum {
    KDSARADC_IRQ_ENABLE_1 = 1,
    KDSARADC_IRQ_ENABLE_0 = 0,
} kdsaradc_IrqEnable_t;

typedef struct {
    int8_t initRefs;
    int8_t startRefs;
} kdsaradc_HostVa_t;

typedef struct {
    kdsaradc_HostVa_t *_va;
    
    struct {
       ADC_InitStructure cfg;
    } init;
    
    uint16_t refVolt;
    
    void (*enableFunc)(kdsaradc_t *kd);
    void (*disableFunc)(kdsaradc_t *kd);
} kdsaradc_Host_t;

typedef struct {
    int8_t initRefs;
} kdsaradc_Va_t;

struct kdsaradc {
    struct {
        kdsaradc_Host_t *instance;
        kdsaradc_Channel_t channel;
        
        struct {
            uint32_t af;
            kdgpio_t *gpio;
        } pin;
    } _config;
    
    kdsaradc_Va_t *_va;
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

#define KDSARADC_CHANNEL(_c)    _c
#define _KDSARADC_CHANNEL(_c)    KDSARADC_CHANNEL_##_c

#define _KDSARADC_HOST      __kdsaradc_instance
#define _KDSARADC_HOST_VA   __kdsaradc_instance_va

#define _KDSARADC_INAME(_x)   __kdsaradc_##_x
#define _KDSARADC_IVA(_x)     __kdsaradc_va_##_x

#define KDSARADC_CLOCK_SRC(x)       x
#define KDSARADC_CLOCK_REF_VOLT(x)  x

#define _KDSARADC_PIN_AF(_port, _pin, _ch) PORT##_port##_PIN##_pin##_ADC_CH##_ch

#define KDSARADC_INSTANCE_CONTINUE_DEFINE(_clockSrc, _refVolt, _triggerSrc, _continueMode, \
        _smapleAvg, \
        _irqEOCEnable, _irqOVFEnable, \
        _enableFunc, _disableFunc) \
    static void __kdsaradc_enableFunc(kdsaradc_t *kd) _enableFunc \
    static void __kdsaradc_disableFunc(kdsaradc_t *kd) _disableFunc \
    static kdsaradc_HostVa_t _KDSARADC_HOST_VA = { \
        .initRefs = 0, \
        .startRefs = 0, \
    }; \
    static const kdsaradc_Host_t _KDSARADC_HOST = { \
        ._va = &_KDSARADC_HOST_VA, \
        .init = { \
            .cfg = { \
                .clk_src = _clockSrc, \
                .channels = 0, \
                .samplAvg = _smapleAvg, \
                .trig_src = _triggerSrc, \
                .Continue = _continueMode, \
                .EOC_IEn = 0, \
                .OVF_IEn = 0, \
            }, \
        }, \
        .refVolt = _refVolt, \
        .enableFunc = __kdsaradc_enableFunc, \
        .disableFunc = __kdsaradc_disableFunc, \
    };
    
#define KDSARADC_NOBLOCK_DEFINE(_chn, _name, \
        _gpio, _port, _pin) \
    static kdsaradc_Va_t _KDSARADC_IVA(_name) = { \
        .initRefs = 0, \
    }; \
    const kdsaradc_t _KDSARADC_INAME(_name) = { \
        ._va = (kdsaradc_Va_t *) &_KDSARADC_IVA(_name), \
        ._config = { \
            .channel = _KDSARADC_CHANNEL(_chn), \
            .instance = (kdsaradc_Host_t *) &_KDSARADC_HOST, \
            .pin = { \
                .gpio = (kdgpio_t *) _gpio, \
                .af = _KDSARADC_PIN_AF(_port, _pin, _chn), \
            }, \
        }, \
    }; 

/*@}*/

#endif
