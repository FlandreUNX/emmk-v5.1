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

int32_t kdpwm_init(kdpwm_t *kd) {
    ASSERT(kd != NULL);
    
    if (emmkDriver_initRefsCountUp(&kd->_va->initRefs) != 0) {
        return -1;
    }
    
//    if (kd->_host->timer->PSC != 0 && kd->_host->timer->ARR != 0) {
//        return 0;
//    }
    kd->_host->enableFunc(kd);
    
    kd->_host->timer->PSC = kd->_host->psc;
    kd->_host->timer->ARR = kd->_host->arr;
    if (kd->_host->autoReload) {
        kd->_host->timer->CR1 |= KLBIT(7);
    } else {
        kd->_host->timer->CR1 &= ~KLBIT(7);
    }
    if ((uint32_t) kd->_host->timer == (uint32_t) TIM1) {
        kd->_host->timer->BDTR |= KLBIT(15);
    }
    return 0;
}

int32_t kdpwm_finalize(kdpwm_t *kd) {
    ASSERT(kd != NULL);
    
    if (emmkDriver_initRefsCountDown(&kd->_va->initRefs) != 0) {
        return -1;
    }
    
    kd->_host->timer->CR1 &= ~KLBIT(0);
    kd->_host->timer->PSC = 0;
    kd->_host->timer->ARR = 0;
    kd->_host->disableFunc(kd);
 
    return 0;
}

int32_t kdpwm_powerUp(kdpwm_t *kd) {
    ASSERT(kd != NULL);
    
    kd->_host->timer->CR2 |= (kd->_config.channel.init.idle << (8u + (kd->_config.channel.index) * 2))
        | (kd->_config.channel.init.idleN << (9u + (kd->_config.channel.index) * 2));
    if (kd->_config.channel.index <= 1) {
        kd->_host->timer->CCMR1_OUT |= (kd->_config.channel.init.mode << ((kd->_config.channel.index) * 8u + 4u))
            | (0x01u << ((kd->_config.channel.index) * 8u + 3u));
    } else {
        kd->_host->timer->CCMR2_OUT |= (kd->_config.channel.init.mode << ((kd->_config.channel.index - 2) * 8u + 4))
            | (0x01u << ((kd->_config.channel.index - 2) * 8u + 3u));
    }
    kdpwm_setDuty(kd, kd->_config.channel.init.preLoad);

    if (kd->_config.pin.gpio != NULL) {
        _gpio_modeConfig(kd->_config.pin.gpio->_config.base.port, 
            kd->_config.pin.gpio->_config.base.number, 
            kd->_config.pin.gpio->_config.base.pin, 
            KDGPIO_MODE_AF_PP);
        _gpio_pullConfig(kd->_config.pin.gpio->_config.base.port,
            kd->_config.pin.gpio->_config.base.number, 
            kd->_config.pin.gpio->_config.base.pin, 
            KDGPIO_PULL_NONE);
        _gpio_afConfig(kd->_config.pin.gpio->_config.base.port, 
            kd->_config.pin.gpio->_config.base.number, 
            kd->_config.pin.af);
        
        kd->_host->timer->CCER |= 0x01u << ((kd->_config.channel.index) * 4u + 0u);
        kd->_host->timer->CCER |= kd->_config.channel.init.pol<< ((kd->_config.channel.index) * 4u + 1u);
    }
    
    if (kd->_config.pinN.gpio != NULL) {
        _gpio_modeConfig(kd->_config.pinN.gpio->_config.base.port, 
            kd->_config.pinN.gpio->_config.base.number, 
            kd->_config.pinN.gpio->_config.base.pin, 
            KDGPIO_MODE_AF_PP);
        _gpio_pullConfig(kd->_config.pinN.gpio->_config.base.port,
            kd->_config.pinN.gpio->_config.base.number, 
            kd->_config.pinN.gpio->_config.base.pin, 
            KDGPIO_PULL_NONE);
        _gpio_afConfig(kd->_config.pinN.gpio->_config.base.port, 
            kd->_config.pinN.gpio->_config.base.number, 
            kd->_config.pinN.af);
        
        kd->_host->timer->CCER |= 0x01u << ((kd->_config.channel.index) * 4u + 2u);
        kd->_host->timer->CCER |= kd->_config.channel.init.polN << ((kd->_config.channel.index) * 4u + 3u);
    }
    
    return 0;
}

int32_t kdpwm_powerDown(kdpwm_t *kd) {
    ASSERT(kd != NULL);
    
    if (kd->_config.pin.gpio != NULL) {
        _gpio_afConfig(kd->_config.pin.gpio->_config.base.port, 
            kd->_config.pin.gpio->_config.base.number, 
            0);
        kdgpio_powerDown(kd->_config.pin.gpio);
    
        kd->_host->timer->CCER &= ~(0x01u << ((kd->_config.channel.index) * 4u + 0u));
    }
    
    if (kd->_config.pinN.gpio != NULL) {
        _gpio_afConfig(kd->_config.pinN.gpio->_config.base.port, 
            kd->_config.pinN.gpio->_config.base.number, 
            0);
        kdgpio_powerDown(kd->_config.pinN.gpio);
        
        kd->_host->timer->CCER &= ~(0x01u << ((kd->_config.channel.index) * 4u + 2u));
    }
    
    return 0;
}

void kdpwm_setFreq(kdpwm_t *kd, uint32_t freq) {
    uint32_t psc = 0, arr = 0;
	float mid;
	long clkInt;
	long midInt;
    
	float clkFloat = SystemCoreClock / freq;
	if (clkFloat - (long) clkFloat >= 0.5f) {
        clkInt = clkFloat + 1;
    } else {
        clkInt = (long) clkFloat;
    }
	
	mid = klMath_carmSqrt(clkFloat);
	if (mid - (long) mid >= 0.5f) {
        midInt = (long) mid + 1;
    } else {
        midInt = (long) mid;
    }

	for (int i = midInt; i >= 1; i--) {
		if (clkInt % i == 0) {
			psc = i;
			arr = clkInt / i;
			break;
		}
	}
    
    kd->_host->timer->PSC = psc;
    kd->_host->timer->ARR = arr;
}

void kdpwm_setPrescaler(kdpwm_t *kd, uint32_t prescaler) {
    kd->_host->timer->PSC = prescaler;
}

void kdpwm_setAutoReload(kdpwm_t *kd, uint32_t autoReload) {
    kd->_host->timer->ARR = autoReload;
}

uint32_t kdpwm_getAutoReload(kdpwm_t *kd) {
    return kd->_host->timer->ARR;
}

void kdpwm_setDuty(kdpwm_t *kd, uint32_t duty) {
     switch (kd->_config.channel.index) {
        case 0:
            kd->_host->timer->CCR1 = duty;
            break;
        case 1:
            kd->_host->timer->CCR2 = duty;
            break;
        case 2:
            kd->_host->timer->CCR3 = duty;
            break;
        case 3:
            kd->_host->timer->CCR4 = duty;
            break;
    }
}

uint32_t kdpwm_getDuty(kdpwm_t *kd) {
    switch (kd->_config.channel.index) {
        case 0:
            return kd->_host->timer->CCR1;
        case 1:
            return kd->_host->timer->CCR2;
        case 2:
            return kd->_host->timer->CCR3;
        case 3:
            return kd->_host->timer->CCR4;
    }
    return 0;
}

void kdpwm_counter(kdpwm_t *kd, uint8_t enable) {
    if (enable) {
        kd->_host->timer->CR1 |= 0x01u;
    } else {
        kd->_host->timer->CR1 &= ~0x01u;
    }
}

void kdpwm_counterReset(kdpwm_t *kd) {
    kd->_host->timer->CNT = 0;
}

void kdpwm_irqEnable(kdpwm_t *kd, kdpwm_Event_t evt, bool enable, kdpwm_SignalEvent_t cbEvent) {
    if (kd->_host->va->cbEventRegistedCount == 0) {
        kd->_host->va->cbEventRegistedCount++;
        kd->_host->va->cbEvent = cbEvent;
    } else {
        if (enable) {
            kd->_host->va->cbEventRegistedCount++;
        } else if (kd->_host->va->cbEventRegistedCount != 0) {
            kd->_host->va->cbEventRegistedCount--;
        }
    }
    
    kd->_host->timer->SR = 0;
    if (evt == KDPWM_EVENT_NONE) {
        kd->_host->timer->DIER &= ~0x01u;
        return;
    }
    if (evt & KDPWM_EVENT_UPDATE) {
        if (enable) {
            kd->_host->timer->DIER |= 0x01u;
        } else {
            kd->_host->timer->DIER &= ~0x01u;
        }
    }
    kd->_host->timer->SR = 0;
}

/*@}*/
