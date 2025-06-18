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
 * @addtogroup PrivateFunc
 * @note none
 */
 
/*@{*/

/* ADC regular channel macro */
#define ADC_REGULAR_CHANNEL_RANK_SIX               ((uint8_t) 6U)
#define ADC_REGULAR_CHANNEL_RANK_TWELVE            ((uint8_t) 12U)
#define ADC_REGULAR_CHANNEL_RANK_SIXTEEN           ((uint8_t) 16U)
#define ADC_REGULAR_CHANNEL_RANK_LENGTH            ((uint8_t) 5U)

/* ADC sampling time macro */
#define ADC_CHANNEL_SAMPLE_TEN                     ((uint8_t) 10U)
#define ADC_CHANNEL_SAMPLE_EIGHTEEN                ((uint8_t) 18U)
#define ADC_CHANNEL_SAMPLE_LENGTH                  ((uint8_t) 3U)

/* ADC_RSQx x=0..2 */
#define ADC_RSQX_RSQN                   KLBITS(0, 4)   /*!< nth conversion in regular sequence */
#define ADC_RSQ0_RL                     KLBITS(20, 23)  /*!< regular channel sequence length */

/* ADC_SAMPTx x=0..1 */
#define ADC_SAMPTX_SPTN                 KLBITS(0,2)                        /*!< channel n sample time selection */

void regularChannelConfig(ADC_TypeDef *adc, uint8_t rank, uint8_t adcChannel , uint32_t sampleTime) {
    uint32_t rsq, sampt;

    if (rank < ADC_REGULAR_CHANNEL_RANK_SIX) {
        rsq = adc->SQR3;
        rsq &= ~((uint32_t) (ADC_RSQX_RSQN << (ADC_REGULAR_CHANNEL_RANK_LENGTH * rank)));
        rsq |= ((uint32_t) adcChannel << (ADC_REGULAR_CHANNEL_RANK_LENGTH * rank));
        adc->SQR3 = rsq;
    } else if (rank < ADC_REGULAR_CHANNEL_RANK_TWELVE) {
        rsq = adc->SQR2;
        rsq &= ~((uint32_t) (ADC_RSQX_RSQN << (ADC_REGULAR_CHANNEL_RANK_LENGTH * (rank - ADC_REGULAR_CHANNEL_RANK_SIX))));
        rsq |= ((uint32_t) adcChannel << (ADC_REGULAR_CHANNEL_RANK_LENGTH * (rank - ADC_REGULAR_CHANNEL_RANK_SIX)));
        adc->SQR2 = rsq;
    } else if (rank < ADC_REGULAR_CHANNEL_RANK_SIXTEEN) {
        rsq = adc->SQR1;
        rsq &= ~((uint32_t) (ADC_RSQX_RSQN << (ADC_REGULAR_CHANNEL_RANK_LENGTH * (rank - ADC_REGULAR_CHANNEL_RANK_TWELVE))));
        rsq |= ((uint32_t) adcChannel << (ADC_REGULAR_CHANNEL_RANK_LENGTH * (rank - ADC_REGULAR_CHANNEL_RANK_TWELVE)));
        adc->SQR1 = rsq;
    } else {
    }
    
    if (adcChannel < ADC_CHANNEL_SAMPLE_TEN) {
        sampt = adc->SMPR3;
        sampt &= ~((uint32_t) (ADC_SAMPTX_SPTN << (ADC_CHANNEL_SAMPLE_LENGTH * adcChannel)));
        sampt |= (uint32_t) (sampleTime << (ADC_CHANNEL_SAMPLE_LENGTH * adcChannel));
        adc->SMPR3 = sampt;
    } else if (adcChannel < ADC_CHANNEL_SAMPLE_EIGHTEEN) {
        sampt = adc->SMPR2;
        sampt &= ~((uint32_t) (ADC_SAMPTX_SPTN << (ADC_CHANNEL_SAMPLE_LENGTH * (adcChannel - ADC_CHANNEL_SAMPLE_TEN))));
        sampt |= (uint32_t) (sampleTime << (ADC_CHANNEL_SAMPLE_LENGTH * (adcChannel - ADC_CHANNEL_SAMPLE_TEN)));
        adc->SMPR2 = sampt;
    } else {
        sampt = adc->SMPR1;
        sampt &= ~((uint32_t) (ADC_SAMPTX_SPTN << (ADC_CHANNEL_SAMPLE_LENGTH * (adcChannel - ADC_CHANNEL_SAMPLE_EIGHTEEN))));
        sampt |= (uint32_t) (sampleTime << (ADC_CHANNEL_SAMPLE_LENGTH * (adcChannel - ADC_CHANNEL_SAMPLE_EIGHTEEN)));
        adc->SMPR1 = sampt;
    }
}

/*@}*/

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
    
    // adc_deinit(kd->_config.host->init.adcBase);
    RCC->APBRSTR2 |= RCC_APBRSTR2_ADCRST;
    RCC->APBRSTR2 &= ~RCC_APBRSTR2_ADCRST;
  
    kd->_config.host->enableFunc();

    dma_deinit((uint32_t) kd->_config.host->init.dma.module, kd->_config.host->init.dma.channel);
    dma_init((uint32_t) kd->_config.host->init.dma.module, kd->_config.host->init.dma.channel, &kd->_config.host->init.dma.init);
    dma_circulation_enable((uint32_t) kd->_config.host->init.dma.module, kd->_config.host->init.dma.channel);

    if (kd->_config.host->init.adc.isContinueMode) {
        // adc_special_function_config(kd->_config.host->init.adcBase, ADC_CONTINUOUS_MODE, ENABLE);
        kd->_config.host->init.adcBase->CR2 |= ADC_CR2_CONT;
    } else { 
        // adc_discontinuous_mode_config(kd->_config.host->init.adcBase, ADC_REGULAR_CHANNEL, kd->_config.host->init.adc.channelNumber);
        kd->_config.host->init.adcBase->CR1 &= ~(
            ADC_CR1_DISCNUM_Msk
            | ADC_CR1_JDISCEN
            | ADC_CR1_DISCEN
        ); 
        kd->_config.host->init.adcBase->CR1 |= ADC_CR1_DISCEN
            | FIELD_PREP(ADC_CR1_DISCNUM_Msk, (kd->_config.host->init.adc.channelNumber - 1));
    }
    
    // adc_special_function_config(kd->_config.host->init.adcBase, ADC_SCAN_MODE, ENABLE);
    kd->_config.host->init.adcBase->CR1 |= ADC_CR1_SCAN;

    // adc_data_alignment_config(kd->_config.host->init.adcBase, ADC_DATAALIGN_RIGHT);
    kd->_config.host->init.adcBase->CR2 &= ~ADC_CR2_ALIGN_Msk;
    
    // adc_channel_length_config(kd->_config.host->init.adcBase, ADC_REGULAR_CHANNEL, kd->_config.host->init.adc.channelNumber);
    kd->_config.host->init.adcBase->SQR1 &= ~ADC_SQR1_L_Msk;
    kd->_config.host->init.adcBase->SQR1 |= FIELD_PREP(ADC_SQR1_L_Msk, kd->_config.host->init.adc.channelNumber - 1);
    
    uint8_t index = 0;
    for (uint8_t i = 0; i < KDSARADC_CHANNEL_RESERVED; i++) {
        if (kd->_config.host->init.adc.enableChannels & (1u << i)) {
            // adc_regular_channel_config(kd->_config.host->init.adcBase, index++, i, ADC_SAMPLETIME_55POINT5);
            regularChannelConfig(kd->_config.host->init.adcBase, index, i, kd->_config.sampleSpeed);
            index++;
        }
    }
    
    // adc_external_trigger_source_config(kd->_config.host->init.adcBase, ADC_REGULAR_CHANNEL, ADC0_1_EXTTRIG_REGULAR_NONE); 
    kd->_config.host->init.adcBase->CR2 &= ~ADC_CR2_EXTSEL_Msk;
    kd->_config.host->init.adcBase->CR2 |= FIELD_PREP(ADC_CR2_EXTSEL_Msk, 0b111);
    
    // adc_external_trigger_config(kd->_config.host->init.adcBase, ADC_REGULAR_CHANNEL, ENABLE);
    kd->_config.host->init.adcBase->CR2 |= ADC_CR2_EXTTRIG;
    
    // VREFBUFFER, 1.5V
    kd->_config.host->init.adcBase->CR2 &= ~ADC_CR2_VREFBUFFERE_Msk;
    if (kd->_config.host->refVolt == 2500) {
        kd->_config.host->init.adcBase->CR2 |= FIELD_PREP(ADC_CR2_VREFBUFFERE_Msk, 0b10);
        kd->_config.host->init.adcBase->CR2 |= ADC_CR2_VREFBUFFERE;
    } else if (kd->_config.host->refVolt == 2048) {
        kd->_config.host->init.adcBase->CR2 |= FIELD_PREP(ADC_CR2_VREFBUFFERE_Msk, 0b01);
        kd->_config.host->init.adcBase->CR2 |= ADC_CR2_VREFBUFFERE;
    } else if (kd->_config.host->refVolt == 1500) {
        kd->_config.host->init.adcBase->CR2 |= ADC_CR2_VREFBUFFERE;
    }        
    // kd->_config.host->init.adcBase->CR2 |= ADC_CR2_TSVREFE;
     
    // adc_calibration_enable(kd->_config.host->init.adcBase);
    kd->_config.host->init.adcBase->CR2 |= (uint32_t) ADC_CR2_RSTCAL; 
    while (RESET != (kd->_config.host->init.adcBase->CR2 & ADC_CR2_RSTCAL));
    kd->_config.host->init.adcBase->CR2 |= ADC_CR2_CAL;
    while (RESET != (kd->_config.host->init.adcBase->CR2 & ADC_CR2_CAL));
    
    // adc_dma_mode_enable(kd->_config.host->init.adcBase); 
    kd->_config.host->init.adcBase->CR2 |= ADC_CR2_DMA;
    
    // adc_enable(kd->_config.host->init.adcBase);
    kd->_config.host->init.adcBase->CR2 |= ADC_CR2_ADON;

    return 0;
}

int32_t kdsaradc_finalize(kdsaradc_t *kd) {
    ASSERT(kd != NULL);
    
    if (emmkDriver_initRefsCountDown(&kd->_config.host->_va->initRefs) != 0) {
        return -1;
    }
    kd->_config.host->_va->startRefs = 0;
    
    dma_channel_disable((uint32_t) kd->_config.host->init.dma.module, kd->_config.host->init.dma.channel);
    
    // adc_disable(kd->_config.host->init.adcBase);
    kd->_config.host->init.adcBase->CR2 &= ~ADC_CR2_ADON;

    dma_deinit((uint32_t) kd->_config.host->init.dma.module, kd->_config.host->init.dma.channel);
    
    kd->_config.host->disableFunc();
    
    // adc_deinit(kd->_config.host->init.adcBase);
    RCC->APBRSTR2 |= RCC_APBRSTR2_ADCRST;
    RCC->APBRSTR2 &= ~RCC_APBRSTR2_ADCRST;

    return 0;
}

int32_t kdsaradc_powerUp(kdsaradc_t *kd) {
    ASSERT(kd != NULL);
    
    if (emmkDriver_initRefsCountUp(&kd->_va->initRefs) != 0) {
        return -1;
    }
    
    if (kd->_config.gpio != NULL) {
        _gpio_modeConfig(kd->_config.gpio->_config.base.port,
            kd->_config.gpio->_config.base.number,
            kd->_config.gpio->_config.base.pin,
            KDGPIO_MODE_AIN);
    }
    
    return 0;
}

int32_t kdsaradc_powerDown(kdsaradc_t *kd) {
    ASSERT(kd != NULL);
    
    if (emmkDriver_initRefsCountDown(&kd->_va->initRefs) != 0) {
        return -1;
    }

    if (kd->_config.gpio != NULL) {
        kdgpio_powerDown(kd->_config.gpio);
    }
    
    return 0;
}

void kdsaradc_convertStart(kdsaradc_t *kd) {
    if (emmkDriver_initRefsCountUp(&kd->_config.host->_va->startRefs) != 0) {
        return;
    }

    dma_channel_enable((uint32_t) kd->_config.host->init.dma.module, kd->_config.host->init.dma.channel);
    
    // adc_software_trigger_enable(kd->_config.host->init.adcBase, ADC_REGULAR_CHANNEL);
    kd->_config.host->init.adcBase->CR2 |= ADC_CR2_SWSTART;
}

void kdsaradc_convertStop(kdsaradc_t *kd) {
    if (emmkDriver_initRefsCountDown(&kd->_config.host->_va->startRefs) != 0) {
        return;
    }
    
    kd->_config.host->init.adcBase->CR1 |= ADC_CR1_ADSTP;
    
    dma_channel_disable((uint32_t) kd->_config.host->init.dma.module, kd->_config.host->init.dma.channel);
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
