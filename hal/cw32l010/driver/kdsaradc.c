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
    if (emmkDriver_initRefsCountUp(&kd->_config.host->_va->initRefs) != 0) {
        return -1; 
    }
    
    kd->_config.host->enableFunc(kd);
    
    REGBITS_MODIFY(CW_ADC->CR,
        ADC_CR_CLK_Msk | ADC_CR_CONT_Msk,
        (((uint32_t) kd->_config.host->init.clockDiv) << 4)
            | ADC_ConvertMode_Continuous);
    CW_ADC->CR |= ADC_CR_BGREN_Msk | ADC_CR_TSEN_Msk;
    
    REGBITS_CLR(CW_ADC->START, ADC_START_START_Msk);
    REGBITS_SET(CW_ADC->CR, ADC_CR_EN_Msk);

    return 0;
}

int32_t kdsaradc_finalize(kdsaradc_t *kd) {
    if (emmkDriver_initRefsCountDown(&kd->_config.host->_va->initRefs) != 0) {
        return -1;
    }
    kd->_config.host->_va->startRefs = 0;
    
    REGBITS_CLR(CW_ADC->START, ADC_START_START_Msk);
    CW_ADC->CR = 0;
    
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
            KDGPIO_MODE_INPUT);
        _gpio_pullConfig(kd->_config.pin.gpio->_config.base.port,
            kd->_config.pin.gpio->_config.base.pin,
            KDGPIO_PULL_NONE);
        kd->_config.pin.gpio->_config.base.port->ANALOG |= kd->_config.pin.gpio->_config.base.pin;
    }
    
    if (kd->_config.host->_va->startRefs != 0) {
        REGBITS_CLR(CW_ADC->START, ADC_START_START_Msk);
    }  
    
    uint32_t treg = CW_ADC->SAMPLE;
    treg &= ~(GENMASK(3, 0) << (4 * kd->_config.seqIndex));
    treg |= (((uint32_t) kd->_config.sampleSelect) << (4 * kd->_config.seqIndex));
    CW_ADC->SAMPLE = treg;
    
    treg = CW_ADC->SQRCFR;
    treg &= ~(GENMASK(3, 0) << (4 * kd->_config.seqIndex));
    treg |= (((uint32_t) kd->_config.channel) << (4 * kd->_config.seqIndex));
    CW_ADC->SQRCFR = treg;
    
    treg = (CW_ADC->CR & ADC_CR_ENS_Msk) >> ADC_CR_ENS_Pos;
    if (kd->_config.host->_va->startRefs != 0) {
        treg += 1;
    }
    if (treg > 7) {
        treg = 7;
    }
    CW_ADC->CR = ((CW_ADC->CR & (~(ADC_CR_ENS_Msk))) 
        | (treg << ADC_CR_ENS_Pos));
    
    if (kd->_config.host->_va->startRefs != 0) {
        REGBITS_SET(CW_ADC->START, ADC_START_START_Msk);
    }
    
    return 0;
}

int32_t kdsaradc_powerDown(kdsaradc_t *kd) {
    if (emmkDriver_initRefsCountDown(&kd->_va->initRefs) != 0) {
        return -1;
    }
    
    if (kd->_config.host->_va->startRefs != 0) {
        REGBITS_CLR(CW_ADC->START, ADC_START_START_Msk);
    }
    
    uint32_t treg = CW_ADC->SAMPLE;
    treg &= ~(GENMASK(3, 0) << (4 * kd->_config.seqIndex));
    CW_ADC->SAMPLE = treg;
    
    treg = CW_ADC->SQRCFR;
    treg &= ~(GENMASK(3, 0) << (4 * kd->_config.seqIndex));
    CW_ADC->SQRCFR = treg;
    
    if (kd->_config.pin.gpio != NULL) {
        kd->_config.pin.gpio->_config.base.port->ANALOG &= ~kd->_config.pin.gpio->_config.base.pin;
        kdgpio_powerDown(kd->_config.pin.gpio);
    }
    
    if (kd->_config.host->_va->startRefs != 0) {
        REGBITS_SET(CW_ADC->START, ADC_START_START_Msk);
    }
        
    return 0;
}

void kdsaradc_convertStart(kdsaradc_t *kd) {
    if (emmkDriver_initRefsCountUp(&kd->_config.host->_va->startRefs) != 0) {
        return;
    }
    REGBITS_CLR(CW_ADC->START, ADC_START_START_Msk);
    REGBITS_SET(CW_ADC->START, ADC_START_START_Msk);
}

void kdsaradc_convertStop(kdsaradc_t *kd) {
    if (emmkDriver_initRefsCountDown(&kd->_config.host->_va->startRefs) != 0) {
        return;
    }
    
    REGBITS_CLR(CW_ADC->START, ADC_START_START_Msk);
}

uint8_t kdsaradc_isConvertCompleted(kdsaradc_t *kd, uint32_t wait) {
    return 1;
}

uint16_t kdsaradc_getRaw(kdsaradc_t *kd) {
    uint32_t *pAdcResult = (uint32_t *) &CW_ADC->RESULT0;
    
    return pAdcResult[kd->_config.seqIndex];
}

float kdsaradc_convertRawToVolt(kdsaradc_t *kd, uint16_t raw) {
    uint16_t refv = 0;
    if (kd->_config.host->init.refV == 0) {
        uint16_t bgrAd = ((uint32_t *) &CW_ADC->RESULT0)[kd->_config.host->init.brgIndex];
        refv = (*(uint16_t *) 0x001007D2) * 4095 / bgrAd;
    } else {
        refv = kd->_config.host->init.refV;
    }
    return raw * ((float) refv / 4095.0f);
}

/*@}*/

#pragma GCC pop_options
