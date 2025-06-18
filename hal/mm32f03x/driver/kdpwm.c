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
    
    if (kd->host->tim->PSC != 0 && kd->host->tim->ARR != 0) {
        return 0;
    }
    kd->host->enableFunc(kd);

    TIM_TimeBaseInitTypeDef init = {
        .TIM_ClockDivision = 0, 
        .TIM_CounterMode = TIM_CounterMode_Up,
        .TIM_Period = kd->host->arr,
        .TIM_Prescaler = kd->host->psc,
        .TIM_RepetitionCounter = 0,
    };
    TIM_TimeBaseInit(kd->host->tim, &init);
    TIM_ARRPreloadConfig(kd->host->tim, ENABLE);

    if ((uint32_t) kd->host->tim == (uint32_t) TIM1 
            || (uint32_t) kd->host->tim == (uint32_t) TIM16
            || (uint32_t) kd->host->tim == (uint32_t) TIM17) {
        TIM_BDTRInitTypeDef bdtr_init;

        bdtr_init.TIM_OSSRState = TIM_OSSRState_Disable;
        bdtr_init.TIM_OSSIState = TIM_OSSIState_Disable;
        bdtr_init.TIM_LOCKLevel = TIM_LOCKLevel_OFF;
        bdtr_init.TIM_DeadTime = 0;
        bdtr_init.TIM_Break = TIM_Break_Disable;
        bdtr_init.TIM_BreakPolarity = TIM_BreakPolarity_High;
        bdtr_init.TIM_AutomaticOutput = TIM_AutomaticOutput_Disable;
        TIM_BDTRConfig(kd->host->tim, &bdtr_init);
        
        TIM_CtrlPWMOutputs(kd->host->tim, ENABLE);
    }

    return 0;
}

int32_t kdpwm_finalize(kdpwm_t *kd) {
    if (emmkDriver_initRefsCountDown(&kd->_va->initRefs) != 0) {
        return -1;
    }
    
    TIM_Cmd(kd->host->tim, DISABLE);
    TIM_DeInit(kd->host->tim);
    
    kd->host->disableFunc(kd);
 
    return 0;
}

int32_t kdpwm_powerUp(kdpwm_t *kd) {
    TIM_OCInitTypeDef init = {
        .TIM_OCIdleState = kd->_config.channel.init.idle,
        .TIM_OCMode = kd->_config.channel.init.mode,
        .TIM_OCNIdleState = kd->_config.channel.init.idleN,
        .TIM_OCNPolarity = kd->_config.channel.init.polN,
        .TIM_OCPolarity = kd->_config.channel.init.pol,
        .TIM_Pulse = 0,
    };
    switch (kd->_config.channel.channel) {
        case TIM_Channel_1: 
            TIM_OC1Init(kd->host->tim, &init);
            TIM_OC1PreloadConfig(kd->host->tim, TIM_OCPreload_Enable);
            break;
        case TIM_Channel_2: 
            TIM_OC2Init(kd->host->tim, &init);
            TIM_OC2PreloadConfig(kd->host->tim, TIM_OCPreload_Enable);
            break;
        case TIM_Channel_3: 
            TIM_OC3Init(kd->host->tim, &init);
            TIM_OC3PreloadConfig(kd->host->tim, TIM_OCPreload_Enable);
            break;
        case TIM_Channel_4: 
            TIM_OC4Init(kd->host->tim, &init);
            TIM_OC4PreloadConfig(kd->host->tim, TIM_OCPreload_Enable);
            break;
    }
    
   if (kd->_config.pin.gpio != NULL) {
        _gpio_modeConfig(kd->_config.pin.gpio->_config.base.port, 
            kd->_config.pin.gpio->_config.base.pin, 
            KDGPIO_MODE_OUTPUT_PP);
        _gpio_pullConfig(kd->_config.pin.gpio->_config.base.port,
            kd->_config.pin.gpio->_config.base.pin, 
            KDGPIO_PULL_NONE);
        _gpio_afConfig(kd->_config.pin.gpio->_config.base.port, 
            kd->_config.pin.gpio->_config.base.number, 
            kd->_config.pin.af);

        TIM_CCxCmd(kd->host->tim, kd->_config.channel.channel, TIM_CCx_Enable);
    }
    
    if (kd->_config.pinN.gpio != NULL) {
        _gpio_modeConfig(kd->_config.pinN.gpio->_config.base.port, 
            kd->_config.pinN.gpio->_config.base.pin, 
            KDGPIO_MODE_OUTPUT_PP);
        _gpio_pullConfig(kd->_config.pinN.gpio->_config.base.port,
            kd->_config.pinN.gpio->_config.base.pin, 
            KDGPIO_PULL_NONE);
        _gpio_afConfig(kd->_config.pinN.gpio->_config.base.port, 
            kd->_config.pinN.gpio->_config.base.number, 
            kd->_config.pinN.af);
        
        TIM_CCxNCmd(kd->host->tim, kd->_config.channel.channel, TIM_CCxN_Enable);
    }
    
    return 0;
}

int32_t kdpwm_powerDown(kdpwm_t *kd) {
    if (kd->_config.pin.gpio != NULL) {
        _gpio_afConfig(kd->_config.pin.gpio->_config.base.port, 
            kd->_config.pin.gpio->_config.base.number, 
            0);
        kdgpio_powerDown(kd->_config.pin.gpio);
        
        TIM_CCxCmd(kd->host->tim, kd->_config.channel.channel, TIM_CCx_Disable);
    }
    
    if (kd->_config.pinN.gpio != NULL) {
        _gpio_afConfig(kd->_config.pinN.gpio->_config.base.port, 
            kd->_config.pinN.gpio->_config.base.number, 
            0);
        kdgpio_powerDown(kd->_config.pinN.gpio);
        
        TIM_CCxNCmd(kd->host->tim, kd->_config.channel.channel, TIM_CCxN_Disable);
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
	
	mid = sqrt(clkFloat);
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

    kd->host->tim->PSC = psc;
    kd->host->tim->ARR = arr;
}

void kdpwm_setPrescaler(kdpwm_t *kd, uint32_t prescaler) {
    kd->host->tim->PSC = prescaler;
}

void kdpwm_setAutoReload(kdpwm_t *kd, uint32_t autoReload) {
   kd->host->tim->ARR = autoReload;
}

uint32_t kdpwm_getAutoReload(kdpwm_t *kd) {
    return kd->host->tim->ARR;
}

void kdpwm_setDuty(kdpwm_t *kd, uint32_t duty) {
    switch (kd->_config.channel.channel) {
        case TIM_Channel_1:
            kd->host->tim->CCR1 = duty;
            break;
        case TIM_Channel_2:
            kd->host->tim->CCR2 = duty;
            break;
        case TIM_Channel_3:
            kd->host->tim->CCR3 = duty;
            break;
        case TIM_Channel_4:
            kd->host->tim->CCR4 = duty;
            break;
    }
}

uint32_t kdpwm_getDuty(kdpwm_t *kd) {
    switch (kd->_config.channel.channel) {
        case TIM_Channel_1:
            return kd->host->tim->CCR1;
        case TIM_Channel_2:
            return kd->host->tim->CCR2;
        case TIM_Channel_3:
            return kd->host->tim->CCR3;
        case TIM_Channel_4:
            return kd->host->tim->CCR4;
    }
    
    return 0;
}

void kdpwm_counter(kdpwm_t *kd, uint8_t enable) {
    if (enable) {
        TIM_Cmd(kd->host->tim, ENABLE);
    } else {
        TIM_Cmd(kd->host->tim, DISABLE);
    }
}

void kdpwm_counterReset(kdpwm_t *kd) {
    kd->host->tim->CNT = 0;
}

void kdpwm_irqEnable(kdpwm_t *kd, kdpwm_Event_t evt, bool enable, kdpwm_SignalEvent_t cbEvent) {
    if (kd->host->va->cbEventRegistedCount == 0) {
        kd->host->va->cbEventRegistedCount++;
        kd->host->va->cbEvent = cbEvent;
    } else {
        if (enable) {
            kd->host->va->cbEventRegistedCount++;
        } else if (kd->host->va->cbEventRegistedCount != 0) {
            kd->host->va->cbEventRegistedCount--;
        }
    }

    if (evt & KDPWM_EVENT_UPDATE) {
        if (enable) {
            kd->host->tim->DIER |= TIM_DIER_UIE;
        } else {
            kd->host->tim->DIER &= ~TIM_DIER_UIE;
        }
    }
}

/*@}*/
