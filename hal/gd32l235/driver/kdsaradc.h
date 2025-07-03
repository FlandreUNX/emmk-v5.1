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

#include "gd32l23x_dma.h"
#include "gd32l23x_adc.h"
#include "gd32l23x_gpio.h"

/**
 * @addtogroup Typedef
 * @note none
 */
 
/*@{*/

typedef struct kdsaradc kdsaradc_t;

typedef enum {
    KDSARADC_CHANNEL_0 = ADC_CHANNEL_0, 
    KDSARADC_CHANNEL_1 = ADC_CHANNEL_1, 
    KDSARADC_CHANNEL_2 = ADC_CHANNEL_2, 
    KDSARADC_CHANNEL_3 = ADC_CHANNEL_3, 
    KDSARADC_CHANNEL_4 = ADC_CHANNEL_4, 
    KDSARADC_CHANNEL_5 = ADC_CHANNEL_5, 
    KDSARADC_CHANNEL_6 = ADC_CHANNEL_6, 
    KDSARADC_CHANNEL_7 = ADC_CHANNEL_7, 
    KDSARADC_CHANNEL_8 = ADC_CHANNEL_8, 
    KDSARADC_CHANNEL_9 = ADC_CHANNEL_9, 
    KDSARADC_CHANNEL_10 = ADC_CHANNEL_10,
    KDSARADC_CHANNEL_11 = ADC_CHANNEL_11,
    KDSARADC_CHANNEL_12 = ADC_CHANNEL_12,
    KDSARADC_CHANNEL_13 = ADC_CHANNEL_13,
    KDSARADC_CHANNEL_14 = ADC_CHANNEL_14,
    KDSARADC_CHANNEL_15 = ADC_CHANNEL_15,
    KDSARADC_CHANNEL_16 = ADC_CHANNEL_16,
    KDSARADC_CHANNEL_17 = ADC_CHANNEL_17,
    KDSARADC_CHANNEL_RESERVED,
} kdsaradc_Channel_t;

typedef struct {
    int8_t initRefs;
    int8_t startRefs;
} kdsaradc_HostVa_t;

typedef struct {
    kdsaradc_HostVa_t *_va;
    
    struct {
        uint32_t adcBase;
        
        struct {
            dma_channel_enum channel;
            dma_parameter_struct init;
        } dma;
        
        struct {
            uint32_t enableChannels;
            uint32_t *channelSampleSpeed;
            uint8_t channelNumber;
            uint8_t isContinueMode;
        } adc;
        
        struct {
            uint16_t *data;
        } buffer;
    } init;
    
    uint16_t refVolt;

    void (*enableFunc)(void);
    void (*disableFunc)(void);
} kdsaradc_Host_t;

typedef struct {
    int8_t initRefs;
} kdsaradc_Va_t;

struct kdsaradc {
    kdsaradc_Va_t *_va;
    struct {
        kdsaradc_Host_t *host;
        
        kdsaradc_Channel_t channel;
        
        uint8_t bufferIndex;
        
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

#define KDSARADC_REFMV(_x)          _x

#define _KDSARADC_HOST(x)              __kdsaradc_instance_##x
#define _KDSARADC_HOST_VA(x)           __kdsaradc_instance_va_##x

#define _KDSARADC_INAME(_x)         __kdsaradc_##_x
#define _KDSARADC_IVA(_x)           __kdsaradc_va_##_x

#define _KDSARADC_INSTANCE_BUFFER(_x)       __kdsaradc_instanceBuffer_##_x
#define _KDSARADC_INSTANCE_CHANNELS(_x)     __kdsaradc_instanceChannels_##_x
#define _KDSARADC_INSTANCE_SAMPLE(_x)       __kdsaradc_instanceSamples_##_x
#define KDSARADC_INSTANCE_SAMPLE(_x)        static uint32_t _KDSARADC_INSTANCE_SAMPLE(_x)[]


#define _KDSARADC_MODULE(x)         ADC
#define KDSARADC_MODULE(x)          x

#define KDSARADC_CHANNEL_COUNT(_c)      _c
#define KDSARADC_CHANNEL_MAP(_c)        (1 << KDSARADC_CHANNEL_##_c)
#define KDSARADC_CHANNEL(_c)            _c
#define _KDSARADC_CHANNEL(_c)           KDSARADC_CHANNEL_##_c
#define KDSARADC_BUFFER_INEDX(_c)       _c  

#define KDSARADC_DMA_CHANNEL(x)     x

#define _KDSARADC_FUNC_ENABLE(x)     __kdsaradc_enableFunc_##x
#define _KDSARADC_FUNC_DISABLE(x)    __kdsaradc_disableFunc_##x

#define KDSARADC_INSTANCE_CONTINUE_DEFINE(_instance, _refVolt, \
        _channels, _channelNumber, \
        _dmaModule, _dmaChannel, \
        _enableFunc, _disableFunc, \
        _dmaIrq) \
    static void _KDSARADC_FUNC_ENABLE(_instance)(void) _enableFunc \
    static void _KDSARADC_FUNC_DISABLE(_instance)(void) _disableFunc \
    static kdsaradc_HostVa_t _KDSARADC_HOST_VA(_instance) = {0}; \
    static uint16_t _KDSARADC_INSTANCE_BUFFER(_instance)[_channelNumber]; \
    static const uint32_t _KDSARADC_INSTANCE_CHANNELS(_instance) = _channels; \
    static const kdsaradc_Host_t _KDSARADC_HOST(_instance) = { \
        ._va = &_KDSARADC_HOST_VA(_instance), \
        .init = { \
            .adcBase = _KDSARADC_MODULE(_instance), \
            .buffer = { \
                .data = _KDSARADC_INSTANCE_BUFFER(_instance), \
            }, \
            .dma = { \
                .channel = _dmaChannel, \
                .init = { \
                    .periph_addr = (uint32_t) (&ADC_RDATA), \
                    .periph_inc = DMA_PERIPH_INCREASE_DISABLE, \
                    .memory_addr = (uint32_t) (_KDSARADC_INSTANCE_BUFFER(_instance)), \
                    .memory_inc = DMA_MEMORY_INCREASE_ENABLE, \
                    .periph_width = DMA_PERIPHERAL_WIDTH_16BIT, \
                    .memory_width = DMA_MEMORY_WIDTH_16BIT, \
                    .direction = DMA_PERIPHERAL_TO_MEMORY, \
                    .number = _channelNumber, \
                    .priority = DMA_PRIORITY_HIGH, \
                }, \
            }, \
            .adc = { \
                .isContinueMode = 1, \
                .channelNumber = _channelNumber, \
                .enableChannels = _KDSARADC_INSTANCE_CHANNELS(_instance), \
                .channelSampleSpeed = (uint32_t *) _KDSARADC_INSTANCE_SAMPLE(_instance), \
            }, \
        }, \
        .refVolt = _refVolt, \
        .enableFunc = _KDSARADC_FUNC_ENABLE(_instance), \
        .disableFunc = _KDSARADC_FUNC_DISABLE(_instance), \
    }; \
    void _dmaIrq(void) { \
        dma_interrupt_flag_clear(_KDSARADC_HOST(_instance).init.dma.channel, DMA_INT_FLAG_FTF); \
        dma_interrupt_flag_clear(_KDSARADC_HOST(_instance).init.dma.channel, DMA_INT_FLAG_ERR); \
    }
    
#define KDSARADC_NOBLOCK_DEFINE(_instance, _chn, _name, \
        _bufferIndex, \
        _gpio) \
    static kdsaradc_Va_t _KDSARADC_IVA(_name) = {0}; \
    const kdsaradc_t _KDSARADC_INAME(_name) = { \
        ._va = (kdsaradc_Va_t *) &_KDSARADC_IVA(_name), \
        ._config = { \
            .channel = _KDSARADC_CHANNEL(_chn), \
            .bufferIndex = _bufferIndex, \
            .host = (kdsaradc_Host_t *) &_KDSARADC_HOST(_instance), \
            .pin = { \
                .gpio = ((kdgpio_t *) _gpio), \
            }, \
        }, \
    }; 

/*@}*/

#endif
