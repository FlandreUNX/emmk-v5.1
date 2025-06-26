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
    if (emmkDriver_initRefsCountUp(&kd->_va->initRefs) != 0) {
        return -1;
    }
    
    if (kd->_host->isAtim) {
        if (kd->_host->tim.atim->PSC != 0 && kd->_host->tim.atim->ARR != 0) {
            return 0;
        }
    } else {
        if (kd->_host->tim.gtim->PSC != 0 && kd->_host->tim.gtim->ARR != 0) {
            return 0;
        }
    }
    kd->_host->enableFunc(kd);

    if (kd->_host->isAtim) {
        ATIM_InitTypeDef init = {
            .BufferState = kd->_host->autoReload,
            .CounterAlignedMode = ATIM_COUNT_ALIGN_MODE_EDGE,
            .CounterDirection = ATIM_COUNTING_UP,
            .Prescaler = kd->_host->psc,
            .ReloadValue = kd->_host->arr,
            .RepetitionCounter = 0
        };
        ATIM_Init(&init);
        CW_ATIM->BDTR_f.MOE = 0b1;
    } else {
        kd->_host->tim.gtim->CR1 = 0;
        kd->_host->tim.gtim->CR1_f.ARPE = kd->_host->autoReload;
        kd->_host->tim.gtim->PSC = kd->_host->psc;
        kd->_host->tim.gtim->ARR = kd->_host->arr;
    }
    return 0;
}

int32_t kdpwm_finalize(kdpwm_t *kd) {
    if (emmkDriver_initRefsCountDown(&kd->_va->initRefs) != 0) {
        return -1;
    }
    
    if (kd->_host->isAtim) {
        ATIM_Cmd(DISABLE);
        ATIM_DeInit();
    } else {
        GTIM_Cmd(kd->_host->tim.gtim, DISABLE);
    }

    kd->_host->disableFunc(kd);
 
    return 0;
}

int32_t kdpwm_powerUp(kdpwm_t *kd) {
    __IO uint32_t *ccmr = NULL;
    if (kd->_host->isAtim) {
        if (kd->_config.channel.index <= 3) {
            ccmr = (uint32_t *) ((uint32_t) (&kd->_host->tim.atim->CCMR1CMP)
            + ((kd->_config.channel.index / 2) * 4));
            uint32_t genmask = GENMASK(7, 0) << ((kd->_config.channel.index % 2) * 8);
            *ccmr &= ~genmask;
            *ccmr |= FIELD_PREP(genmask, 
                (((uint32_t) kd->_config.channel.init.mode) << 4)
                | (((uint32_t) kd->_config.channel.init.preLoad) << 3));
        } else {
            ccmr = &kd->_host->tim.atim->CCMR3CMP;
            uint32_t genmask = GENMASK(7, 0) << (((kd->_config.channel.index - 4) % 2) * 8);
            *ccmr &= ~genmask;
            *ccmr |= FIELD_PREP(genmask, 
                (((uint32_t) kd->_config.channel.init.mode) << 4)
                | (((uint32_t) kd->_config.channel.init.preLoad) << 3));
        }
    } else {
        if (kd->_config.channel.index <= 1) {
            ccmr = (uint32_t *) ((uint32_t) (&kd->_host->tim.gtim->CCMR1CMP)
            + ((kd->_config.channel.index) * 4));
            uint32_t genmask = GENMASK(7, 0) << ((kd->_config.channel.index % 2) * 8);
            *ccmr &= ~genmask;
            *ccmr |= FIELD_PREP(genmask, 
                (((uint32_t) kd->_config.channel.init.mode) << 4)
                | (((uint32_t) kd->_config.channel.init.preLoad) << 3));
        } else {
            ccmr = &kd->_host->tim.gtim->CCMR2CMP;
            uint32_t genmask = GENMASK(7, 0) << (((kd->_config.channel.index - 2) % 2) * 8);
            *ccmr &= ~genmask;
            *ccmr |= FIELD_PREP(genmask, 
                (((uint32_t) kd->_config.channel.init.mode) << 4)
                | (((uint32_t) kd->_config.channel.init.preLoad) << 3));
        }
    }

    kdpwm_setDuty(kd, 0);
    
    if (kd->_config.pin.gpio != NULL) {
        _gpio_modeConfig(kd->_config.pin.gpio->_config.base.port, 
            kd->_config.pin.gpio->_config.base.pin, 
            KDGPIO_MODE_AF_PP);
        _gpio_pullConfig(kd->_config.pin.gpio->_config.base.port,
            kd->_config.pin.gpio->_config.base.pin, 
            KDGPIO_PULL_NONE);
        _gpio_afConfig(kd->_config.pin.gpio->_config.base.port, 
            kd->_config.pin.gpio->_config.base.number, 
            kd->_config.pin.af);

        if (kd->_host->isAtim) {
            if (kd->_config.channel.init.pol) {
                kd->_host->tim.atim->CCER |= KLBIT(4 * (kd->_config.channel.index) + 1);
            } else {
                kd->_host->tim.atim->CCER &= ~KLBIT(4 * (kd->_config.channel.index) + 1);
            }
            kd->_host->tim.atim->CCER |= KLBIT(4 * (kd->_config.channel.index));
        } else {
            if (kd->_config.channel.init.pol) {
                kd->_host->tim.gtim->CCER |= KLBIT(4 * (kd->_config.channel.index) + 1);
            } else {
                kd->_host->tim.gtim->CCER &= ~KLBIT(4 * (kd->_config.channel.index) + 1);
            }
            kd->_host->tim.gtim->CCER |= KLBIT(4 * (kd->_config.channel.index));
        }
    }
    
    if (kd->_config.pinN.gpio != NULL) {
        _gpio_modeConfig(kd->_config.pinN.gpio->_config.base.port, 
            kd->_config.pinN.gpio->_config.base.pin, 
            KDGPIO_MODE_AF_PP);
        _gpio_pullConfig(kd->_config.pinN.gpio->_config.base.port,
            kd->_config.pinN.gpio->_config.base.pin, 
            KDGPIO_PULL_NONE);
        _gpio_afConfig(kd->_config.pinN.gpio->_config.base.port, 
            kd->_config.pinN.gpio->_config.base.number, 
            kd->_config.pinN.af);
        
        if (kd->_host->isAtim) {
            if (kd->_config.channel.init.polN) {
                kd->_host->tim.atim->CCER |= KLBIT(4 * (kd->_config.channel.index) + 3);
            } else {
                kd->_host->tim.atim->CCER &= ~KLBIT(4 * (kd->_config.channel.index) + 3);
            }
            kd->_host->tim.atim->CCER |= KLBIT(4 * (kd->_config.channel.index) + 2);
        } else {
            if (kd->_config.channel.init.polN) {
                kd->_host->tim.gtim->CCER |= KLBIT(4 * (kd->_config.channel.index) + 3);
            } else {
                kd->_host->tim.gtim->CCER &= ~KLBIT(4 * (kd->_config.channel.index) + 3);
            }
            kd->_host->tim.gtim->CCER |= KLBIT(4 * (kd->_config.channel.index) + 2);
        }
    }
    
    return 0;
}

int32_t kdpwm_powerDown(kdpwm_t *kd) {
    if (kd->_config.pin.gpio != NULL) {
        _gpio_afConfig(kd->_config.pin.gpio->_config.base.port, 
            kd->_config.pin.gpio->_config.base.number, 
            0);
        kdgpio_powerDown(kd->_config.pin.gpio);
    
        if (kd->_host->isAtim) {
            kd->_host->tim.atim->CCER &= ~KLBIT(4 * (kd->_config.channel.index));
        } else {
            kd->_host->tim.gtim->CCER &= ~KLBIT(4 * (kd->_config.channel.index));
        }
    }
    
    if (kd->_config.pinN.gpio != NULL) {
        _gpio_afConfig(kd->_config.pinN.gpio->_config.base.port, 
            kd->_config.pinN.gpio->_config.base.number, 
            0);
        kdgpio_powerDown(kd->_config.pinN.gpio);
        
        if (kd->_host->isAtim) {
            kd->_host->tim.atim->CCER &= ~KLBIT(6 * (kd->_config.channel.index));
        } else {
            kd->_host->tim.gtim->CCER &= ~KLBIT(6 * (kd->_config.channel.index));
        }
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
    
    if (kd->_host->isAtim) {
        kd->_host->tim.atim->PSC = psc;
        kd->_host->tim.atim->ARR = arr;
    } else {
        kd->_host->tim.gtim->PSC = psc;
        kd->_host->tim.gtim->ARR = arr;
    }
}

void kdpwm_setPrescaler(kdpwm_t *kd, uint32_t prescaler) {
    if (kd->_host->isAtim) {
        kd->_host->tim.atim->PSC = prescaler;
    } else {
        kd->_host->tim.gtim->PSC = prescaler;
    }
}

void kdpwm_setAutoReload(kdpwm_t *kd, uint32_t autoReload) {
    if (kd->_host->isAtim) {
        kd->_host->tim.atim->ARR = autoReload;
    } else {
        kd->_host->tim.gtim->ARR = autoReload;
    }
}

uint32_t kdpwm_getAutoReload(kdpwm_t *kd) {
    if (kd->_host->isAtim) {
        return kd->_host->tim.atim->ARR;
    } else {
        return kd->_host->tim.gtim->ARR;
    }
}

void kdpwm_setDuty(kdpwm_t *kd, uint32_t duty) {
    if (kd->_host->isAtim) {
        uint32_t os = kd->_config.channel.index * 4 + (kd->_config.channel.index >= 4 ? 4 : 0);
        *(__IO uint32_t *)(((uint32_t) &kd->_host->tim.atim->CCR1) + os) = duty;
    } else {
        uint32_t os = kd->_config.channel.index * 4;
        *(__IO uint32_t *)(((uint32_t) &kd->_host->tim.gtim->CCR1) + os) = duty;
    }
}

uint32_t kdpwm_getDuty(kdpwm_t *kd) {
    if (kd->_host->isAtim) {
        uint32_t os = kd->_config.channel.index * 4 + (kd->_config.channel.index >= 4 ? 4 : 0);
        return *(__IO uint32_t *)(((uint32_t) &kd->_host->tim.atim->CCR1) + os);
    } else {
        uint32_t os = kd->_config.channel.index * 4;
        return *(__IO uint32_t *)(((uint32_t) &kd->_host->tim.gtim->CCR1) + os);
    }
}

void kdpwm_counter(kdpwm_t *kd, uint8_t enable) {
    if (kd->_host->isAtim) {
        ATIM_Cmd(enable);
    } else {
        GTIM_Cmd(kd->_host->tim.gtim, enable);
    }
}

void kdpwm_counterReset(kdpwm_t *kd) {
    if (kd->_host->isAtim) {
        ATIM_SetCounterValue(0);
    } else {
        GTIM_SetCounterValue(kd->_host->tim.gtim, 0);
    }
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
    
    if (kd->_host->isAtim) {
        kd->_host->tim.atim->ICR = 0;
        if (evt == KDPWM_EVENT_NONE) {
            kd->_host->tim.atim->IER = 0;
            return;
        }
        if (evt & KDPWM_EVENT_UPDATE) {
            if (enable) {
                kd->_host->tim.atim->IER |= KLBIT(ATIM_IER_UIE_Pos);
            } else {
                kd->_host->tim.atim->IER &= ~KLBIT(ATIM_IER_UIE_Pos);
            }
        }
        kd->_host->tim.atim->ICR = 0;
    } else {
         kd->_host->tim.gtim->ICR = 0;
        if (evt == KDPWM_EVENT_NONE) {
            kd->_host->tim.gtim->IER = 0;
            return;
        }
        if (evt & KDPWM_EVENT_UPDATE) {
            if (enable) {
                kd->_host->tim.gtim->IER |= KLBIT(ATIM_IER_UIE_Pos);
            } else {
                kd->_host->tim.gtim->IER &= ~KLBIT(ATIM_IER_UIE_Pos);
            }
        }
        kd->_host->tim.gtim->ICR = 0;
    }
}

/*@}*/
