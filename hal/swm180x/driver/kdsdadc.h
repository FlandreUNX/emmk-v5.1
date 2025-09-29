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

#ifndef _KADC_SDADC_H_
#define _KADC_SDADC_H_

#include <stdint.h>

#include "../SWM180_StdPeriph_Driver/SWM180.h"
#include "SWM180_sdadc.h"                 // Synwit::Device:Driver:ADC
#include "SWM180_gpio.h"                // Synwit::Device:Driver:GPIO
#include "SWM180_port.h"                // Synwit::Device:Driver:Port

/**
 * @addtogroup Typedef
 * @note none
 */
 
/*@{*/

typedef struct kdsdadc kdsdadc_t;

typedef enum {
    KDSDADC_CHANNEL_0 = SDADC_CH0,
    KDSDADC_CHANNEL_1 = SDADC_CH1,
    KDSDADC_CHANNEL_2 = SDADC_CH2,
    KDSDADC_CHANNEL_3 = SDADC_CH3,
    KDSDADC_CHANNEL_4 = SDADC_CH4,
    KDSDADC_CHANNEL_5 = SDADC_CH5,
} kdsdadc_Channel_t;

typedef enum {
    KDSDADC_SDMODE_SINGLE = 1,
    KDSDADC_SDMODE_DIFF = 0,
} kdsdadc_SdMode_t;

typedef enum {
    KDSDADC_CONTINUE_MODE_1 = 1,
    KDSDADC_CONTINUE_MODE_0 = 0,
} kdsdadc_ContinueMode_t;

typedef enum {
    KDSDADC_IRQ_ENABLE_1 = 1,
    KDSDADC_IRQ_ENABLE_0 = 0,
} kdsdadc_IrqEnable_t;

typedef enum {
    KDSDADC_CFG_ENABLE,
    KDSDADC_CFG_DISABLE,
} kdsdadc_CfgEnable_t;

typedef enum {
    KDSDADC_CFG_REFM_TO_N_GND = 0,
    KDSDADC_CFG_REFM_TO_INTERNAL_GND = 1,
} kdsdadc_cfgRefM_t;

typedef struct {
    int8_t initRefs;
} kdsdadc_Va_t;

struct kdsdadc {
    struct {
        struct {
            SDADC_InitStructure init;
            uint16_t refVolt;
            
            struct {
                kdsdadc_SdMode_t sdMode;
                uint32_t gain;
                uint32_t refM;
                uint32_t channels;
                float convertOffset;
            } cfg[3];
            
            void (*enableFunc)(kdsdadc_t *kd);
            void (*disableFunc)(kdsdadc_t *kd);
            uint8_t (*isInit)(kdsdadc_t *kd);
        } instance;
    } _config;
    
    struct {
        int32_t *result;
        uint16_t resultBufferLength;
    } _buffer;
    
    kdsdadc_Va_t *_va;
};

/*@}*/

/**
 * @addtogroup Instance define
 * @note none
 */
 
/*@{*/

#define _KDSDADC_GET(x)     __kdsdadc_instance_##x
#define _KDSDADC_EXTERN(x)  extern const kdsdadc_t __kdsdadc_instance_##x
#define KDSDADC_GET(x)     _KDSDADC_GET(x)
#define KDSDADC_EXTERN(x)  _KDSDADC_EXTERN(x)

#define KDSDADC_DMA_BUFFER_LENGTH(x) x
#define KDSDADC_GAIN(x) x

#define KDSDADC_MODULE(x) x

extern int32_t kdsdadc_init(kdsdadc_t *kd); \
extern int32_t kdsdadc_finalize(kdsdadc_t *kd); \
extern int32_t kdsdadc_powerUp(kdsdadc_t *kd); \
extern int32_t kdsdadc_powerDown(kdsdadc_t *kd); \
extern void kdsdadc_convertStart(kdsdadc_t *kd); \
extern void kdsdadc_convertStop(kdsdadc_t *kd); \
extern uint8_t kdsdadc_isConvertCompleted(kdsdadc_t *kd, uint32_t wait); \
extern int32_t kdsdadc_getRaw(kdsdadc_t *kd, kdsdadc_Channel_t chn, int16_t *raw); \
extern float kdsdadc_convertRawToVolt(kdsdadc_t *kd, kdsdadc_Channel_t chn, int16_t raw); 
    
#define KDSDADC_DMA_DEFINE(_clockSrc, _refpSelect, _refVolt, _triggerSrc, _continueMode, \
    _bufferLength, \
    _channels, \
    _sdModeA, _gainA, _refMA, channelsA, _convertGaintA, \
    _sdModeB, _gainB, _refMB, channelsB, _convertGaintB, \
    _sdModeC, _gainC, _refMC, channelsC, _convertGaintC, \
    _irqEOCEnable, _irqOVFEnable, _irqHFullEnable, _irqFullEnable, \
    _enableFunc, _disableFunc, _isInit) \
    static void __kdsdadc_enableFunc(kdsdadc_t *kd) _enableFunc \
    static void __kdsdadc_disableFunc(kdsdadc_t *kd) _disableFunc \
    static uint8_t __kdsdadc_isInit(kdsdadc_t *kd) _isInit \
    kdsdadc_Va_t __kdsdadc_va = { \
    }; \
    static int32_t AT_NONCACHEABLE_SECTION_ALIGN_INIT(__kdsdadc_buffer[_bufferLength], 4); \
    const kdsdadc_t __kdsdadc_instance_0 = { \
        ._va = &__kdsdadc_va, \
        ._buffer = { \
            .result = __kdsdadc_buffer, \
            .resultBufferLength = _bufferLength, \
        }, \
        ._config = { \
            .instance = { \
                .init = { \
                    .clk_src = _clockSrc, \
                    .channels = _channels, \
                    .out_cali = SDADC_OUT_CALIED, \
                    .refp_sel = _refpSelect, \
                    .trig_src = _triggerSrc, \
                    .Continue = _continueMode, \
                    .EOC_IEn = _irqEOCEnable, \
                    .OVF_IEn = _irqOVFEnable, \
                    .HFULL_IEn = _irqHFullEnable, \
                    .FULL_IEn = _irqFullEnable, \
                }, \
                .refVolt = _refVolt, \
                .cfg = { \
                    [0] = { \
                        _sdModeA, _gainA, _refMA, channelsA, \
                        (((((float) _refVolt)) / (2.0f * _convertGaintA)) / 32767.0f) \
                    }, \
                    [1] = { \
                        _sdModeB, _gainB, _refMB, channelsB, \
                        (((((float) _refVolt)) / (2.0f * _convertGaintB)) / 32767.0f) \
                    }, \
                    [2] = { \
                        _sdModeC, _gainC, _refMC, channelsC, \
                        (((((float) _refVolt)) / (2.0f * _convertGaintC)) / 32767.0f) \
                    }, \
                }, \
                .enableFunc = __kdsdadc_enableFunc, \
                .disableFunc = __kdsdadc_disableFunc, \
                .isInit = __kdsdadc_isInit, \
            }, \
        }, \
    };

/*@}*/

#endif
