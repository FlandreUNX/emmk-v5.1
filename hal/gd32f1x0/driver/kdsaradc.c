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
 * @addtogroup Public func
 * @note none
 */
 
/*@{*/

int32_t kdsaradc_init(kdsaradc_t *kd) {
    ASSERT(kd != NULL);
    
    if (emmkDriver_initRefsCountUp(&kd->_config.host->_va->initRefs) != 0) {
        return -1; 
    }
    
    kd->_config.host->enableFunc();
    
    dma_deinit(kd->_config.host->init.dma.channel);
    dma_init(kd->_config.host->init.dma.channel, &kd->_config.host->init.dma.init);
    dma_circulation_enable(kd->_config.host->init.dma.channel);

    adc_deinit();
    if (kd->_config.host->init.adc.isContinueMode) {
        adc_special_function_config(ADC_CONTINUOUS_MODE, ENABLE);
    } else {
        adc_discontinuous_mode_config(ADC_REGULAR_CHANNEL, kd->_config.host->init.adc.channelNumber);
    }
    adc_special_function_config(ADC_SCAN_MODE, ENABLE);
    adc_data_alignment_config(ADC_DATAALIGN_RIGHT);
    
    adc_channel_length_config(ADC_REGULAR_CHANNEL, kd->_config.host->init.adc.channelNumber);
    uint8_t index = 0;
    for (uint8_t i = 0; i < KDSARADC_CHANNEL_RESERVED; i++) {
        if (kd->_config.host->init.adc.enableChannels & (1u << i)) {
            adc_regular_channel_config(index, i, kd->_config.host->init.adc.channelSampleSpeed[index]);
            index++;
        }
    }

    adc_external_trigger_source_config(ADC_REGULAR_CHANNEL, ADC_EXTTRIG_REGULAR_NONE);
    adc_external_trigger_config(ADC_REGULAR_CHANNEL, ENABLE);
    
    adc_enable();
    adc_calibration_enable();
    adc_dma_mode_enable();
    
    return 0;
}

int32_t kdsaradc_finalize(kdsaradc_t *kd) {
    ASSERT(kd != NULL);
    
    if (emmkDriver_initRefsCountDown(&kd->_config.host->_va->initRefs) != 0) {
        return -1;
    }
    kd->_config.host->_va->startRefs = 0;
    
    dma_channel_disable(kd->_config.host->init.dma.channel);
    
    adc_disable();
    adc_deinit();
    dma_deinit(kd->_config.host->init.dma.channel);
    
    kd->_config.host->disableFunc();
    
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
    
    dma_channel_enable(kd->_config.host->init.dma.channel);
    adc_software_trigger_enable(ADC_REGULAR_CHANNEL);
}

void kdsaradc_convertStop(kdsaradc_t *kd) {
    if (emmkDriver_initRefsCountDown(&kd->_config.host->_va->startRefs) != 0) {
        return;
    }
    
    dma_channel_disable(kd->_config.host->init.dma.channel);
}

uint8_t kdsaradc_isConvertCompleted(kdsaradc_t *kd, uint32_t wait) {
    (void) kd;
    (void) wait;
    return 1;
}

uint16_t kdsaradc_getRaw(kdsaradc_t *kd) {
    return kd->_config.host->init.buffer.data[kd->_config.bufferIndex] & 0x0FFF;
}

float kdsaradc_convertRawToVolt(kdsaradc_t *kd, uint16_t raw) {
    return raw * ((float) kd->_config.host->refVolt / 4096.0f);
}

/*@}*/
