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
 * @addtogroup Interface define
 * @note none
 */
 
/*@{*/

KDPWM_G_DEFINE(PWM_CLKDIV_8);
    
/*@}*/

/**
 * @addtogroup Public func
 * @note none
 */
 
/*@{*/

int32_t kdpwm_init(kdpwm_t *kd) {
    if (emmkDriver_initRefsCountUp(&kd->_va->initRefs) != 0) {
        return -1;
    }
    
    if (!(SYS->CLKEN & (0x01 << SYS_CLKEN_PWM_Pos))) {
        SYS->CLKEN |= (0x01 << SYS_CLKEN_PWM_Pos);
        PWM0->INIOUT &= ~(PWM_INIOUT_PWMA_Pos | PWM_INIOUT_PWMB_Pos);
        PWM1->INIOUT &= ~(PWM_INIOUT_PWMA_Pos | PWM_INIOUT_PWMB_Pos);
        PWM2->INIOUT &= ~(PWM_INIOUT_PWMA_Pos | PWM_INIOUT_PWMB_Pos);
        PWM3->INIOUT &= ~(PWM_INIOUT_PWMA_Pos | PWM_INIOUT_PWMB_Pos);
        PWMG->CLKDIV = gPWMDivClk;
        PWMG->IMSK = 0x00000000;
    }
    
    if (PWMG->CHEN & kd->_config.host->mask) {
        return 0;
    } else {
        PORT_Init(kd->_config.pin.gpio->_config.base.port, 
            kd->_config.pin.gpio->_config.base.number, 
            0, 1);
        _gpio_modeConfig(kd->_config.pin.gpio->_config.base.port,
            kd->_config.pin.gpio->_config.base.gpio,
            kd->_config.pin.gpio->_config.base.pin,
            KDGPIO_MODE_OUTPUT_PP);
        _gpio_pullConfig(kd->_config.pin.gpio->_config.base.port,
            kd->_config.pin.gpio->_config.base.pin,
            KDGPIO_PULL_NONE);
        GPIO_ClrBit(kd->_config.pin.gpio->_config.base.gpio, kd->_config.pin.gpio->_config.base.number);

        kd->_config.host->pwmx->MODE = kd->_config.host->mode;
    }
    
    if (kd->_config.channel.numberX == 0) {
        kd->_config.host->pwmx->PERA = kd->_config.channel.cycle;
        kd->_config.host->pwmx->HIGHA = kd->_config.channel.duty;
        kd->_config.host->pwmx->DZA = 0;
    }
    
    if (kd->_config.channel.numberX == 1) {
        kd->_config.host->pwmx->PERB = kd->_config.channel.cycle;
        kd->_config.host->pwmx->HIGHB = kd->_config.channel.duty;
        kd->_config.host->pwmx->DZB = 0;
    }
    
    return 0;
}

int32_t kdpwm_finalize(kdpwm_t *kd) {
    ASSERT(kd != NULL);
    
    if (emmkDriver_initRefsCountDown(&kd->_va->initRefs) != 0) {
        return -1;
    }
    
    if (kd->_config.channel.numberX == 0) {
        kd->_config.host->pwmx->PERA = 0;
        kd->_config.host->pwmx->HIGHA = 0;
        kd->_config.host->pwmx->DZA = 0;
    }
    
    if (kd->_config.channel.numberX == 1) {
        kd->_config.host->pwmx->PERB = 0;
        kd->_config.host->pwmx->HIGHB = 0;
        kd->_config.host->pwmx->DZB = 0;
    }
    
    return 0;
}

int32_t kdpwm_powerUp(kdpwm_t *kd) {
    ASSERT(kd != NULL);
    
    PORT_Init(kd->_config.pin.gpio->_config.base.port, 
        kd->_config.pin.gpio->_config.base.number, 
        kd->_config.pin.af, 1);
    _gpio_modeConfig(kd->_config.pin.gpio->_config.base.port,
        kd->_config.pin.gpio->_config.base.gpio,
        kd->_config.pin.gpio->_config.base.pin,
        KDGPIO_MODE_OUTPUT_PP);
    _gpio_pullConfig(kd->_config.pin.gpio->_config.base.port,
        kd->_config.pin.gpio->_config.base.pin,
        KDGPIO_PULL_NONE);
    PWMG->CHEN |= 1 << (kd->_config.host->pwmNumber * 2 + kd->_config.channel.numberX);
    
    return 0;
}

int32_t kdpwm_powerDown(kdpwm_t *kd) {
    ASSERT(kd != NULL);
 
    kdgpio_powerDown(kd->_config.pin.gpio);
    PWMG->CHEN &= ~(1 << (kd->_config.host->pwmNumber * 2 + kd->_config.channel.numberX));
    
    return 0;
}

void kdpwm_setFreq(kdpwm_t *kd, uint32_t freq) {
}

void kdpwm_setPrescaler(kdpwm_t *kd, uint32_t prescaler) {
    uint8_t isEnable = PWMG->CHEN & (1 << (kd->_config.host->pwmNumber * 2 + kd->_config.channel.numberX)) ? 1 : 0;
    
    PWMG->CHEN &= ~(1 << (kd->_config.host->pwmNumber * 2 + kd->_config.channel.numberX));
    
    if (kd->_config.channel.numberX == 0) {
        kd->_config.host->pwmx->PERA = prescaler;
    } else if (kd->_config.channel.numberX == 1) {
        kd->_config.host->pwmx->PERB = prescaler;
    }
    
    if (isEnable) {
        PWMG->CHEN |= 1 << (kd->_config.host->pwmNumber * 2 + kd->_config.channel.numberX);
    }
}

void kdpwm_setAutoReload(kdpwm_t *kd, uint32_t autoReload) {
    kdpwm_setPrescaler(kd, autoReload);
}

uint32_t kdpwm_getAutoReload(kdpwm_t *kd) {
    if (kd->_config.channel.numberX == 0) {
        return kd->_config.host->pwmx->PERA;
    } else {
        return kd->_config.host->pwmx->PERB;
    }
}

void kdpwm_setDuty(kdpwm_t *kd, uint32_t duty) {
    if (kd->_config.channel.numberX == 0) {
        kd->_config.host->pwmx->HIGHA = duty;
    } else if (kd->_config.channel.numberX == 1) {
        kd->_config.host->pwmx->HIGHB = duty;
    }
}

uint32_t kdpwm_getDuty(kdpwm_t *kd) {
    if (kd->_config.channel.numberX == 0) {
        return kd->_config.host->pwmx->HIGHA;
    } else {
        return kd->_config.host->pwmx->HIGHB;
    }
}

void kdpwm_counter(kdpwm_t *kd, uint8_t enable) {

}

void kdpwm_counterReset(kdpwm_t *kd) {

}

void kdpwm_irqEnable(kdpwm_t *kd, kdpwm_Event_t evt, bool enable, kdpwm_SignalEvent_t cbEvent) {
}

/*@}*/
