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
    
    kd->_config.host->enableFunc(kd);
    
    ADC->CR1 = (0x01 << 10);
	ADC->CR2 = (0x01 << 16);
	ADC->CR0 = (0x01 << 11)
        | (kd->_config.host->init.clockDiv << 4);
    
    ADC->CR0 |= (0x01 << 0);
    ADC->CR0 |= (0x01 << 1);

    return 0;
}

int32_t kdsaradc_finalize(kdsaradc_t *kd) {
    ASSERT(kd != NULL);
    
    if (emmkDriver_initRefsCountDown(&kd->_config.host->_va->initRefs) != 0) {
        return -1;
    }
    kd->_config.host->_va->startRefs = 0;
    
    ADC->CR0 = 0;
    ADC->CR2 = 0;
    ADC->CR1 = 0;
    
    kd->_config.host->disableFunc(kd);
    
    return 0;
}

int32_t kdsaradc_powerUp(kdsaradc_t *kd) {
    ASSERT(kd != NULL);
    
    if (kd->_config.channel >= KDSARADC_CHANNEL_FAKE_BASE) {
        return 0;
    }
    
    if (emmkDriver_initRefsCountUp(&kd->_va->initRefs) != 0) {
        return -1;
    }
    
    ADC->CR0 &= ~KLBIT(0);
    ADC->CR0 &= ~KLBIT(1);
    ADC->CR2 |= (1u << kd->_config.channel);
    ADC->CR0 |= KLBIT(0);
    ADC->CR0 |= KLBIT(1);
    
    if (kd->_config.pin.gpio != NULL) {
        _gpio_modeConfig(kd->_config.pin.gpio->_config.base.port,
            kd->_config.pin.gpio->_config.base.number,
            kd->_config.pin.gpio->_config.base.pin,
            KDGPIO_MODE_AIN);
        _gpio_pullConfig(kd->_config.pin.gpio->_config.base.port,
            kd->_config.pin.gpio->_config.base.number,
            kd->_config.pin.gpio->_config.base.pin,
            KDGPIO_PULL_NONE);
    }

    return 0;
}

int32_t kdsaradc_powerDown(kdsaradc_t *kd) {
    ASSERT(kd != NULL);
    
    if (kd->_config.channel >= KDSARADC_CHANNEL_FAKE_BASE) {
        return 0;
    }
    
    if (emmkDriver_initRefsCountDown(&kd->_va->initRefs) != 0) {
        return -1;
    }
    
    ADC->CR0 &= ~(0x01 << 0);
    ADC->CR2 &= ~(1u << kd->_config.channel);
    ADC->CR0 |= (0x01 << 0);
    
    if (kd->_config.pin.gpio != NULL) {
        kdgpio_powerDown(kd->_config.pin.gpio);
    }

    return 0;
}

void kdsaradc_convertStart(kdsaradc_t *kd) {
    if (kd->_config.channel >= KDSARADC_CHANNEL_FAKE_BASE) {
        return;
    }
    if (emmkDriver_initRefsCountUp(&kd->_config.host->_va->startRefs) != 0) {
        return;
    }
}

void kdsaradc_convertStop(kdsaradc_t *kd) {
    if (kd->_config.channel >= KDSARADC_CHANNEL_FAKE_BASE) {
        return;
    }
    if (emmkDriver_initRefsCountDown(&kd->_config.host->_va->startRefs) != 0) {
        return;
    }
}

uint8_t kdsaradc_isConvertCompleted(kdsaradc_t *kd, uint32_t wait) {
    return 1;
}

uint16_t kdsaradc_getRaw(kdsaradc_t *kd) {
    if (kd->_config.channel >= KDSARADC_CHANNEL_FAKE_BASE) {
        return (*(__IO uint32_t *) (((uint32_t) &ADC->RESULT0) + (4 * (kd->_config.channel - KDSARADC_CHANNEL_FAKE_BASE)))) & 0x0FFF; 
    } else {
        return (*(__IO uint32_t *) (((uint32_t) &ADC->RESULT0) + (4 * kd->_config.channel))) & 0x0FFF;
    }
}

float kdsaradc_convertRawToVolt(kdsaradc_t *kd, uint16_t raw) {
    return raw * ((float) kd->_config.host->init.refV / 4096.0f);
}

/*@}*/

#pragma GCC pop_options
