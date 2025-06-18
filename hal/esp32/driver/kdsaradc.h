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

#ifndef ESP32C3_KDSARADC_H
#define ESP32C3_KDSARADC_H

#include <stdint.h>

#include "esp_adc/adc_continuous.h"

/**
 * @addtogroup Typedef
 * @note none
 */

/*@{*/

typedef struct kdsaradc kdsaradc_t;

typedef enum {
    KDSARADC_ADC1_CHANNEL_0 = 0,
    KDSARADC_ADC1_CHANNEL_1 = 1,
    KDSARADC_ADC1_CHANNEL_2 = 2,
    KDSARADC_ADC1_CHANNEL_3 = 3,
    KDSARADC_ADC1_CHANNEL_4 = 4,

    KDSARADC_ADC2_CHANNEL_0 = 5,

    KDSARADC_ADC_CHANNEL_RESERVE = 6,
} kdsaradc_Channel_t;

typedef struct {
    int8_t initRefs;
    int8_t startRefs;
    uint16_t buffer[KDSARADC_ADC_CHANNEL_RESERVE];
    adc_continuous_handle_t handle;
} kdsaradc_InstanceVa_t;

typedef struct {
    kdsaradc_InstanceVa_t *_va;
    adc_continuous_handle_cfg_t handleCfg;
    adc_continuous_config_t digCfg;

    uint16_t refVolt;

    void (*enableFunc)(kdsaradc_t *kd);
    void (*disableFunc)(kdsaradc_t *kd);
} kdsaradc_Instance_t;

typedef struct {
    int8_t initRefs;
} kdsaradc_Va_t;

struct kdsaradc {
    struct {
        kdsaradc_Channel_t channel;
        kdsaradc_Instance_t *instance;
    } _config;

    kdsaradc_Va_t *_va;
};

/*@}*/

/**
 * @addtogroup Instance define
 * @note none
 */

/*@{*/

#define _KDSARADC_NEXTERN(_x)       extern const kdsaradc_t __kdsaradc_##_x
#define _KDSARADC_NGET(_x)          __kdsaradc_##_x
#define KDSARADC_NEXTERN(_x)        _KDSARADC_NEXTERN(_x)
#define KDSARADC_NGET(_x)           _KDSARADC_NGET(_x)

#define _KDSARADC_INSTANCE          __kdsaradc_instance
#define _KDSARADC_INSTANCE_VA       __kdsaradc_instance_va

#define _KDSARADC_INAME(_x)         __kdsaradc_##_x
#define _KDSARADC(_adc, _chn)       __kdsaradc_##_adc##_##_chn
#define _KDSARADC_VA(_adc, _chn)    __kdsaradc_va_##_adc##_##_chn

extern int32_t kdsaradc_init(kdsaradc_t *kd);
extern int32_t kdsaradc_finalize(kdsaradc_t *kd);
extern int32_t kdsaradc_powerUp(kdsaradc_t *kd);
extern int32_t kdsaradc_powerDown(kdsaradc_t *kd);
extern uint16_t kdsaradc_getRaw(kdsaradc_t *kd);
extern float kdsaradc_convertRawToVolt(kdsaradc_t *kd, uint16_t raw);
extern uint8_t kdsaradc_isConvertCompleted(kdsaradc_t *kd, uint32_t wait);
extern void kdsaradc_convertStart(kdsaradc_t *kd);
extern void kdsaradc_convertStop(kdsaradc_t *kd);

#define KDSARADC_CHANNEL(_unit, _channel, _atten) \
        {.channel = _channel, .unit = _unit - 1, .atten = _atten, .bit_width = SOC_ADC_DIGI_MAX_BITWIDTH},

#define KDSARADC_REF_VOLT(x)  x
#define KDSARADC_BUFFER_SIZE(x)       x
#define KDSARADC_READ_LEN(x)       x

#define KDSARADC_NUM_EACH(x)       x
#define KDSARADC_CHN_INDEX(x)       BIT(x)
#define KDSARADC_ADC1_CHN_MASK(x)       x
#define KDSARADC_ADC2_CHN_MASK(x)       x
#define KDSARADC_LIMIT_EN(x)  x
#define KDSARADC_LIMIT_NUM(x)  x
#define KDSARADC_FREQ(x)  x
#define KDSARADC_CHANNEL_NUMBER(x)  x
#define KDSARADC_ADC_NUMBER(_adc)  _adc
#define KDSARADC_ADC_CHANNEL(_adc, _chn)  KDSARADC_ADC##_adc##_CHANNEL_##_chn

#define KDSARADC_INSTANCE_DEFINE(_refVolt, _bufferSize, _readLen, \
        _freq, _convMode, _format, \
        _channelNumber, _channelTable,   \
        _enableFunc, _disableFunc) \
    static void __kdsaradc_enableFunc(kdsaradc_t *kd) _enableFunc \
    static void __kdsaradc_disableFunc(kdsaradc_t *kd) _disableFunc \
    static kdsaradc_InstanceVa_t _KDSARADC_INSTANCE_VA = {0};                                                            \
    static adc_digi_pattern_config_t __kdasradc_pattern[SOC_ADC_PATT_LEN_MAX] = _channelTable;    \
    static const kdsaradc_Instance_t _KDSARADC_INSTANCE = { \
        ._va = (kdsaradc_InstanceVa_t *) &_KDSARADC_INSTANCE_VA, \
        .refVolt = _refVolt, \
        .enableFunc = __kdsaradc_enableFunc, \
        .disableFunc = __kdsaradc_disableFunc,                    \
        .handleCfg = {                                            \
            .max_store_buf_size = _bufferSize,                           \
            .conv_frame_size = _readLen, \
        },                                                        \
        .digCfg = {                                               \
            .sample_freq_hz = _freq,                              \
            .conv_mode = _convMode,                               \
            .format = _format,                                    \
            .adc_pattern = __kdasradc_pattern,                    \
            .pattern_num = _channelNumber,                                                          \
        }                                                              \
    };

#define KDSARADC_NOBLOCK_DEFINE(_adc, _chn, _name) \
    static kdsaradc_Va_t _KDSARADC_VA(_adc, _chn) = {0};                                                   \
    const kdsaradc_t _KDSARADC_INAME(_name) = { \
        ._va = &_KDSARADC_VA(_adc, _chn),                           \
        ._config = {                  \
            .channel = KDSARADC_ADC_CHANNEL(_adc, _chn),                          \
            .instance = (kdsaradc_Instance_t *) &_KDSARADC_INSTANCE,       \
        },                                               \
    };

/*@}*/

#endif //ESP32C3_KDSARADC_H
