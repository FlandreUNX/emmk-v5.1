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

#include "emmk-config.h"
#include "emmk-driver.h"

/**
 * @name IrqHandler
 * @{
 */

__INTERRUPT
__HIGH_CODE
void ADC_IRQHandler(void) {
    extern const kdsaradc_Host_t _KDSARADC_HOST;

    if (ADC_GetITStatus()) {
        ADC_ClearITFlag();

        _KDSARADC_HOST.init.buffer.data[_KDSARADC_HOST._va->convertIndex] = ADC_ReadConverValue();
        _KDSARADC_HOST._va->convertIndex++;
        if (_KDSARADC_HOST._va->convertIndex >= _KDSARADC_HOST.init.map.channelNumber) {
            _KDSARADC_HOST._va->convertIndex = 0;
        }
        R8_ADC_CHANNEL = _KDSARADC_HOST.init.map.scanChannels[_KDSARADC_HOST._va->convertIndex];
        R8_ADC_CONVERT |= RB_ADC_START;
    }
}

/** @} */

/**
 * @addtogroup Public func
 * @note none
 */

/*@{*/

int32_t kdsaradc_init(kdsaradc_t *kd) {
    if (emmkDriver_initRefsCountUp(&kd->_config.host->_va->initRefs) != 0) {
        return -1;
    }

    kd->_config.host->enableFunc(kd);

    ADC_ExtSingleChSampInit(SampleFreq_3_2, ADC_PGA_0);
    kd->_config.host->_va->calibratedValue = 0;//ADC_DataCalib_Rough();

    R8_ADC_CFG &= ~RB_ADC_POWER_ON;
    R8_ADC_CFG = RB_ADC_BUF_EN
                 | (kd->_config.host->init.clockDiv << 6)
                 | ((kd->_config.host->init.pga & 0xF) << 4);
    if (kd->_config.host->init.pga & ADC_PGA_2_) {
        R8_ADC_CONVERT |= RB_ADC_PGA_GAIN2;
    } else {
        R8_ADC_CONVERT &= ~RB_ADC_PGA_GAIN2;
    }

    int scanIdx = 0;
    for (int channel = 0; channel < KDSARADC_CHANNEL_RESERVER; channel++) {
        if (kd->_config.host->init.map.channelMap & (1 << channel)) {
            kd->_config.host->init.map.scanChannels[scanIdx] = channel;
            scanIdx++;
        }
    }

    kd->_config.host->_va->convertIndex = 0;
    R8_ADC_CHANNEL = kd->_config.host->init.map.scanChannels[0];

    ADC_ClearITFlag();
    R8_ADC_CTRL_DMA |= RB_ADC_IE_EOC;
    PFIC_EnableIRQ(ADC_IRQn);

    return 0;
}


int32_t kdsaradc_finalize(kdsaradc_t *kd) {
    if (emmkDriver_initRefsCountDown(&kd->_config.host->_va->initRefs) != 0) {
        return -1;
    }
    kd->_config.host->_va->startRefs = 0;

    R8_ADC_CONVERT = R8_ADC_CONVERT;

    R8_ADC_CTRL_DMA = 0;
    R8_ADC_CFG = 0;

    kd->_config.host->disableFunc(kd);

    return 0;
}


int32_t kdsaradc_powerUp(kdsaradc_t *kd) {
    if (emmkDriver_initRefsCountUp(&kd->_va->initRefs) != 0) {
        return -1;
    }

    if (kd->_config.pin.gpio != NULL) {
        _gpio_modeConfig(kd->_config.pin.gpio->_config.base.port,
                         kd->_config.pin.gpio->_config.base.pin,
                         KDGPIO_MODE_AIN);
        _gpio_pullConfig(kd->_config.pin.gpio->_config.base.port,
                         kd->_config.pin.gpio->_config.base.pin,
                         KDGPIO_PULL_NONE);
    }

    return 0;
}


int32_t kdsaradc_powerDown(kdsaradc_t *kd) {
    if (emmkDriver_initRefsCountDown(&kd->_va->initRefs) != 0) {
        return -1;
    }

    if (kd->_config.pin.gpio != NULL) {
        kdgpio_powerDown(kd->_config.pin.gpio);
    }

    return 0;
}


void kdsaradc_convertStart(kdsaradc_t *kd) {
    if (emmkDriver_initRefsCountUp(&kd->_config.host->_va->startRefs) != 0) {
        return;
    }

    R8_ADC_CONVERT = R8_ADC_CONVERT;

    kd->_config.host->_va->convertIndex = 0;
    R8_ADC_CHANNEL = kd->_config.host->init.map.scanChannels[0];

    R8_ADC_CONVERT |= RB_ADC_START;
}


void kdsaradc_convertStop(kdsaradc_t *kd) {
    if (emmkDriver_initRefsCountDown(&kd->_config.host->_va->startRefs) != 0) {
        return;
    }

    R8_ADC_CONVERT &= ~RB_ADC_START;
    R8_ADC_CONVERT = R8_ADC_CONVERT;
    kd->_config.host->_va->convertIndex = 0;
}


uint8_t kdsaradc_isConvertCompleted(kdsaradc_t *kd, uint32_t wait) {
    return R8_ADC_DMA_IF & RB_ADC_IF_DMA_END ? 1 : 0;
}


uint16_t kdsaradc_getRaw(kdsaradc_t *kd) {
    int16_t iRaw = *((int16_t *) &kd->_config.host->init.buffer.data[kd->_config.bufferIndex]) + kd->_config.host->_va->calibratedValue;
    return *((uint16_t *) &iRaw);
}


float kdsaradc_convertRawToVolt(kdsaradc_t *kd, uint16_t raw) {
    float vi = 0;
    int16_t iRaw = (int16_t) raw;
    switch (kd->_config.host->init.pga) {
        case ADC_PGA_1_4: vi = ((float) iRaw / 512.0f - 3) * 1050.0f;
            break;
        case ADC_PGA_1_2: vi = ((float) iRaw / 1024.0f - 1) * 1050.0f;
            break;
        case ADC_PGA_0: vi = ((float) iRaw / 2048.0f) * 1050.0f;
            break;
        case ADC_PGA_2: vi = ((float) iRaw / 4096.0f + 0.5f) * 1050.0f;
            break;
        case ADC_PGA_4: vi = ((float) iRaw / 8192.0f + 0.75f) * 1050.0f;
            break;
        case ADC_PGA_8: vi = ((float) iRaw / 16384.0f + 0.875f) * 1050.0f;
            break;
        case ADC_PGA_16: vi = ((float) iRaw / 32768.0f + 0.9375f) * 1050.0f;
            break;
        default: break;
    }
    return vi;
}

/*@}*/
