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
    
    if (emmkDriver_initRefsCountUp(&kd->_config.instance->_va->initRefs) != 0) {
        return -1; 
    }
    
    kd->_config.instance->enableFunc(kd);

    ADC_Init(ADC, &kd->_config.instance->init.cfg);
    ADC_Open(ADC);

    return 0;
}

int32_t kdsaradc_finalize(kdsaradc_t *kd) {
    ASSERT(kd != NULL);
    
    if (emmkDriver_initRefsCountDown(&kd->_config.instance->_va->initRefs) != 0) {
        return -1;
    }
    
    kd->_config.instance->_va->startRefs = 0;
    
    ADC_Stop(ADC);
    ADC_Close(ADC);
    
    kd->_config.instance->disableFunc(kd);
    
    return 0;
}

int32_t kdsaradc_powerUp(kdsaradc_t *kd) {
    ASSERT(kd != NULL);
    
    if (emmkDriver_initRefsCountUp(&kd->_va->initRefs) != 0) {
        return -1;
    }
    
    PORT_Init(kd->_config.pin.gpio->_config.base.port, 
        kd->_config.pin.gpio->_config.base.number, 
        kd->_config.pin.af, 0);
    _gpio_modeConfig(kd->_config.pin.gpio->_config.base.port,
        kd->_config.pin.gpio->_config.base.gpio,
        kd->_config.pin.gpio->_config.base.pin,
        KDGPIO_MODE_INPUT);
    _gpio_pullConfig(kd->_config.pin.gpio->_config.base.port,
        kd->_config.pin.gpio->_config.base.pin,
        KDGPIO_PULL_NONE);

    ADC->CTRL |= (kd->_config.channel << ADC_CTRL_CH0_Pos);
    
    return 0;
}

int32_t kdsaradc_powerDown(kdsaradc_t *kd) {
    ASSERT(kd != NULL);
    
    if (emmkDriver_initRefsCountDown(&kd->_va->initRefs) != 0) {
        return -1;
    }
    
    ADC->CTRL &= ~(kd->_config.channel << ADC_CTRL_CH0_Pos);

    PORT_Init(kd->_config.pin.gpio->_config.base.port, 
        kd->_config.pin.gpio->_config.base.number, 
        0, 0);
    kdgpio_powerDown(kd->_config.pin.gpio);
        
    return 0;
}

void kdsaradc_convertStart(kdsaradc_t *kd) {
    if (emmkDriver_initRefsCountUp(&kd->_config.instance->_va->startRefs) != 0) {
        return;
    }
    
    ADC_Start(ADC);
}

void kdsaradc_convertStop(kdsaradc_t *kd) {
    if (emmkDriver_initRefsCountDown(&kd->_config.instance->_va->startRefs) != 0) {
        return;
    }
    
    ADC_Stop(ADC);
}

uint8_t kdsaradc_isConvertCompleted(kdsaradc_t *kd, uint32_t wait) {
    return 1;
}

uint16_t kdsaradc_getRaw(kdsaradc_t *kd) {
    return ADC_Read(ADC, kd->_config.channel);
}

float kdsaradc_convertRawToVolt(kdsaradc_t *kd, uint16_t raw) {
    return raw * ((float) kd->_config.instance->refVolt / 4096.0f);
}

/*@}*/

#pragma GCC pop_options
