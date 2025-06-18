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

#include "py32f0xx_hal.h"
#include "kdgpio.h"
#include "emmk-config.h"

#include "gd32e10x_dma.h"

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
    KDSARADC_CHANNEL_7 = 7,
    KDSARADC_CHANNEL_8 = 8,
    KDSARADC_CHANNEL_9 = 9,
    KDSARADC_CHANNEL_10 = 10,
    KDSARADC_CHANNEL_11 = 11,
    KDSARADC_CHANNEL_12 = 12,
    KDSARADC_CHANNEL_13 = 13,
    KDSARADC_CHANNEL_14 = 14,
    KDSARADC_CHANNEL_15 = 15,
    KDSARADC_CHANNEL_16 = 16,
    KDSARADC_CHANNEL_17 = 17,
    KDSARADC_CHANNEL_18 = 18,
    KDSARADC_CHANNEL_19 = 19,
    KDSARADC_CHANNEL_20 = 20,
    KDSARADC_CHANNEL_21 = 21,
    KDSARADC_CHANNEL_22 = 22,
    KDSARADC_CHANNEL_23 = 23,
    KDSARADC_CHANNEL_RESERVED
} kdSARADC_Channel_t;

typedef struct {
    int8_t initRefs;
    int8_t startRefs;
} kdsaradc_HostVa_t;

typedef struct {
    kdsaradc_HostVa_t *_va;
    
    struct {
        ADC_TypeDef *adcBase;
        
        struct {
            DMA_TypeDef *module;
            dma_channel_enum channel;
            dma_parameter_struct init;
        } dma;
        
        struct {
            uint32_t enableChannels;
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
        
        uint32_t channel;
        uint32_t sampleSpeed; 
        uint8_t bufferIndex; 
        
        kdgpio_t *gpio;
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
#define KDSARADC_CLOCK_SRC(x)       x

#define _KDSARADC_HOST(x)              __kdsaradc_instance_##x
#define _KDSARADC_HOST_VA(x)           __kdsaradc_instance_va_##x

#define _KDSARADC_INAME(_x)         __kdsaradc_##_x
#define _KDSARADC_IVA(_x)           __kdsaradc_va_##_x

#define _KDSARADC_INSTANCE_BUFFER(_x)       __kdsaradc_instanceBuffer_##_x
#define _KDSARADC_INSTANCE_CHANNELS(_x)     __kdsaradc_instanceChannels_##_x

#define KDSARADC_MODULE(x)      x
#define _KDSARADC_MODULE(x)     ADC##x

#define KDSARADC_PORT(_port)   _port
#define _KDSARADC_GPIO(_port)  GPIO##_port

#define KDSARADC_PIN(_pin)     _pin
#define _KDSARADC_PIN(_pin)    GPIO_PIN_##_pin

#define KDSARADC_CHANNEL(_c)        _c
#define KDSARADC_CHANNEL_MAP(_c)    (1 << KDSARADC_CHANNEL_##_c)
#define _KDSARADC_CHANNEL(_c)       KDSARADC_CHANNEL_##_c

#define KDSARADC_DMA_MODULE(x)  x
#define _KDSARADC_DMA_MODULE(x)  DMA##x
#define KDSARADC_DMA_CHANNEL(x)  x

#define KDSARADC_BUFFER_INEDX(x)  x

#define _KDSARADC_FUNC_ENABLE(x)     __kdsaradc_enableFunc_##x
#define _KDSARADC_FUNC_DISABLE(x)    __kdsaradc_disableFunc_##x
    
#define KDSARADC_INSTANCE_CONTINUE_DEFINE(_instance, _refVolt, \
        _channels, _channelNumber, \
        _dmaModule, _dmaChannel, \
        _enableFunc, _disableFunc) \
    static void _KDSARADC_FUNC_ENABLE(_instance)(void) _enableFunc \
    static void _KDSARADC_FUNC_DISABLE(_instance)(void) _disableFunc \
    static kdsaradc_HostVa_t _KDSARADC_HOST_VA(_instance) = {0}; \
    uint16_t _KDSARADC_INSTANCE_BUFFER(_instance)[_channelNumber]; \
    static const uint32_t _KDSARADC_INSTANCE_CHANNELS(_instance) = _channels; \
    static const kdsaradc_Host_t _KDSARADC_HOST(_instance) = { \
        ._va = &_KDSARADC_HOST_VA(_instance), \
        .init = { \
            .adcBase = _KDSARADC_MODULE(_instance), \
            .buffer = { \
                .data = _KDSARADC_INSTANCE_BUFFER(_instance), \
            }, \
            .dma = { \
                .module = _KDSARADC_DMA_MODULE(_dmaModule), \
                .channel = _dmaChannel, \
                .init = { \
                    .periph_addr = (uint32_t) (&(_KDSARADC_MODULE(_instance)->DR)), \
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
            }, \
        }, \
        .refVolt = _refVolt, \
        .enableFunc = _KDSARADC_FUNC_ENABLE(_instance), \
        .disableFunc = _KDSARADC_FUNC_DISABLE(_instance), \
    };  \
    
    
#define KDSARADC_NOBLOCK_DEFINE(_instance, _chn, _name, \
        _bufferIndex, _sampleSpeed, \
        _gpio) \
    static kdsaradc_Va_t _KDSARADC_IVA(_name) = {0}; \
    const kdsaradc_t _KDSARADC_INAME(_name) = { \
        ._va = (kdsaradc_Va_t *) &_KDSARADC_IVA(_name), \
        ._config = { \
            .host = (kdsaradc_Host_t *) &_KDSARADC_HOST(_instance), \
            .channel = _KDSARADC_CHANNEL(_chn), \
            .bufferIndex = _bufferIndex, \
            .sampleSpeed = _sampleSpeed, \
            .gpio = ((kdgpio_t *) _gpio), \
        }, \
    };

/*@}*/

#endif
