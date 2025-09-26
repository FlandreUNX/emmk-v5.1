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

#include "CH59x_adc.h"

/**
 * @addtogroup Typedef
 * @note none
 */
 
/*@{*/

typedef struct kdsaradc kdsaradc_t;

typedef enum {
    KDSARADC_CHANNEL_0 = CH_EXTIN_0,
    KDSARADC_CHANNEL_1 = CH_EXTIN_1,
    KDSARADC_CHANNEL_2 = CH_EXTIN_2,
    KDSARADC_CHANNEL_3 = CH_EXTIN_3,
    KDSARADC_CHANNEL_4 = CH_EXTIN_4,
    KDSARADC_CHANNEL_5 = CH_EXTIN_5,
    KDSARADC_CHANNEL_6 = CH_EXTIN_6,
    KDSARADC_CHANNEL_7 = CH_EXTIN_7,
    KDSARADC_CHANNEL_8 = CH_EXTIN_8,
    KDSARADC_CHANNEL_9 = CH_EXTIN_9,
    KDSARADC_CHANNEL_10 = CH_EXTIN_10,
    KDSARADC_CHANNEL_11 = CH_EXTIN_11,
    KDSARADC_CHANNEL_12 = CH_EXTIN_12,
    KDSARADC_CHANNEL_13 = CH_EXTIN_13,
    KDSARADC_CHANNEL_14 = CH_INTE_VBAT,
    KDSARADC_CHANNEL_15 = CH_INTE_VTEMP,
    KDSARADC_CHANNEL_DIFF_C0_C2 = 16,
    KDSARADC_CHANNEL_DIFF_C1_C3 = 17,
    KDSARADC_CHANNEL_RESERVER = 18,
} kdsaradc_Channel_t;

typedef struct {
    int8_t initRefs;
    int8_t startRefs;

    int16_t calibratedValue;
    uint8_t convertIndex;
} kdsaradc_HostVa_t;

typedef struct {
    kdsaradc_HostVa_t *_va;
    
    struct {
        ADC_SampClkTypeDef clockDiv;
        ADC_SignalPGATypeDef pga;

        struct {
            uint8_t channelNumber;
            uint32_t channelMap;
            uint8_t *scanChannels;
        } map;

        struct {
            uint16_t *data;
        } buffer;
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

#define KDSARADC_SEQINDEX(_c)    _c
#define _KDSARADC_SEQINDEX(_c)   _c

#define _KDSARADC_HOST_BUFFER       __kdsaradc_instanceBuffer
#define _KDSARADC_HOST_CHANNELS_ARRAY       __kdsaradc_instanceChannels
#define _KDSARADC_HOST_CHANNELS(_x)     __kdsaradc_instanceChannels_##_x
#define KDSARADC_HOST_CHANNEL_MAP(_x)     (1 << KDSARADC_CHANNEL_##_x)

// (256-cycle)*16*Tsys
#define KDSARADC_AUTO_CYCLE(_x)     _x

#define KDSARADC_CHANNEL_COUNT(_c)    (_c)

#define KDSARADC_CHANNEL_BUFFER_IDX(_c)       _c

#define _KDSARADC_HOST      __kdsaradc_instance
#define _KDSARADC_HOST_VA   __kdsaradc_instance_va

#define _KDSARADC_INAME(_x)   __kdsaradc_##_x
#define _KDSARADC_IVA(_x)     __kdsaradc_va_##_x

#define KDSARADC_INSTANCE_CONTINUE_DEFINE(_clockSrc, \
        _channelNumber, _channelMap, \
        _pga, \
        _enableFunc, _disableFunc) \
    static void __kdsaradc_enableFunc(kdsaradc_t *kd) _enableFunc \
    static void __kdsaradc_disableFunc(kdsaradc_t *kd) _disableFunc \
    uint16_t _KDSARADC_HOST_BUFFER[_channelNumber]; \
    uint8_t _KDSARADC_HOST_CHANNELS_ARRAY[_channelNumber]; \
    static kdsaradc_HostVa_t _KDSARADC_HOST_VA = {0}; \
    const kdsaradc_Host_t _KDSARADC_HOST = { \
        ._va = &_KDSARADC_HOST_VA, \
        .init = { \
            .clockDiv = _clockSrc, \
            .pga = _pga, \
            .map = { \
                .channelNumber = _channelNumber, \
                .channelMap = _channelMap, \
                .scanChannels = _KDSARADC_HOST_CHANNELS_ARRAY, \
            }, \
            .buffer = { \
                .data = _KDSARADC_HOST_BUFFER, \
            }, \
        }, \
        .enableFunc = __kdsaradc_enableFunc, \
        .disableFunc = __kdsaradc_disableFunc, \
    };
    
#define KDSARADC_NOBLOCK_DEFINE(_chn, _name, \
        _seqIndex, \
        _gpio) \
    static kdsaradc_Va_t _KDSARADC_IVA(_name) = {0}; \
    const kdsaradc_t _KDSARADC_INAME(_name) = { \
        ._va = (kdsaradc_Va_t *) &_KDSARADC_IVA(_name), \
        ._config = { \
            .channel = _chn, \
            .bufferIndex = _KDSARADC_SEQINDEX(_seqIndex), \
            .host = (kdsaradc_Host_t *) &_KDSARADC_HOST, \
            .pin = { \
                .gpio = ((kdgpio_t *) _gpio), \
            }, \
        }, \
    }; 

/*@}*/

#endif
