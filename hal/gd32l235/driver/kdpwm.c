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

    if (RCU_REG_VAL(kd->_host->timerRcu) & BIT(RCU_BIT_POS(kd->_host->timerRcu))) {
        return 0;
    } 
    kd->_host->enableFunc(kd);
    
    timer_init(kd->_host->timer, (timer_parameter_struct *) &kd->_host->init);
    timer_internal_clock_config(kd->_host->timer);
    if (kd->_host->autoReload) {
        timer_auto_reload_shadow_enable(kd->_host->timer);
    } else {
        timer_auto_reload_shadow_disable(kd->_host->timer);
    }
#if defined(GD32L235)
    if (kd->_host->timer == TIMER0) {
        timer_primary_output_config(kd->_host->timer, ENABLE);
    }
#endif
    return 0;
}

int32_t kdpwm_finalize(kdpwm_t *kd) {
    ASSERT(kd != NULL);
    
    if (emmkDriver_initRefsCountDown(&kd->_va->initRefs) != 0) {
        return -1;
    }
    
    timer_disable(kd->_host->timer);

    kd->_host->disableFunc(kd);
 
    return 0;
}

int32_t kdpwm_powerUp(kdpwm_t *kd) {
    timer_channel_output_config(kd->_host->timer, kd->_config.channel.index, &kd->_config.channel.oc);
    timer_channel_output_mode_config(kd->_host->timer, kd->_config.channel.index, kd->_config.channel.mode);
    if (kd->_config.channel.shadowRelaod) {
        timer_channel_output_shadow_config(kd->_host->timer, kd->_config.channel.index, TIMER_OC_SHADOW_ENABLE);
    } else {
        timer_channel_output_shadow_config(kd->_host->timer, kd->_config.channel.index, TIMER_OC_SHADOW_DISABLE);
    }

    if (kd->_config.pin.gpio != NULL) {
        timer_channel_output_state_config(kd->_host->timer, kd->_config.channel.index, TIMER_CCX_ENABLE);
        
        _gpio_modeConfig(kd->_config.pin.gpio->_config.base.port, 
            kd->_config.pin.gpio->_config.base.pin, 
            KDGPIO_MODE_AF_PP);
        _gpio_pullConfig(kd->_config.pin.gpio->_config.base.port,
            kd->_config.pin.gpio->_config.base.pin, 
            KDGPIO_PULL_NONE);
    }
    
    if (kd->_config.pinN.gpio != NULL) {
#if defined(GD32L235)
        timer_channel_complementary_output_state_config(kd->_host->timer, kd->_config.channel.index, TIMER_CCXN_ENABLE);
#endif
        _gpio_modeConfig(kd->_config.pinN.gpio->_config.base.port, 
            kd->_config.pinN.gpio->_config.base.pin, 
            KDGPIO_MODE_AF_PP);
        _gpio_pullConfig(kd->_config.pinN.gpio->_config.base.port,
            kd->_config.pinN.gpio->_config.base.pin, 
            KDGPIO_PULL_NONE);
    }
    
    return 0;
}

int32_t kdpwm_powerDown(kdpwm_t *kd) {
    if (kd->_config.pin.gpio != NULL) {
        _gpio_afConfig(kd->_config.pin.gpio->_config.base.port, 
            kd->_config.pin.gpio->_config.base.number, 
            kd->_config.pin.gpio->_config.base.pin,
            0);
        kdgpio_powerDown(kd->_config.pin.gpio);
    
        timer_channel_output_state_config(kd->_host->timer, kd->_config.channel.index, TIMER_CCX_DISABLE);
    }
    
    if (kd->_config.pinN.gpio != NULL) {
        _gpio_afConfig(kd->_config.pinN.gpio->_config.base.port, 
            kd->_config.pinN.gpio->_config.base.number, 
            kd->_config.pinN.gpio->_config.base.pin,
            0);
        kdgpio_powerDown(kd->_config.pinN.gpio);
#if defined(GD32L235)
        timer_channel_complementary_output_state_config(kd->_host->timer, kd->_config.channel.index, TIMER_CCXN_DISABLE);
#endif
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
    
    timer_prescaler_config(kd->_host->timer, psc, TIMER_PSC_RELOAD_NOW);
    timer_autoreload_value_config(kd->_host->timer, arr);
}

void kdpwm_setPrescaler(kdpwm_t *kd, uint32_t prescaler) {
    timer_prescaler_config(kd->_host->timer, prescaler, TIMER_PSC_RELOAD_NOW);
}

void kdpwm_setAutoReload(kdpwm_t *kd, uint32_t autoReload) {
    timer_autoreload_value_config(kd->_host->timer, autoReload);
}

uint32_t kdpwm_getAutoReload(kdpwm_t *kd) {
    return TIMER_CAR(kd->_host->timer);
}

void kdpwm_setDuty(kdpwm_t *kd, uint32_t duty) {
    timer_channel_output_pulse_value_config(kd->_host->timer, kd->_config.channel.index, duty);
}

uint32_t kdpwm_getDuty(kdpwm_t *kd) {
    return timer_channel_capture_value_register_read(kd->_host->timer, kd->_config.channel.index);
}

void kdpwm_counter(kdpwm_t *kd, uint8_t enable) {
    if (enable) {
        timer_enable(kd->_host->timer);
    } else {
        timer_disable(kd->_host->timer);
    }
}

void kdpwm_counterReset(kdpwm_t *kd) {
    TIMER_CNT(kd->_host->timer) = 0;
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
    
    TIMER_INTF(kd->_host->timer) = 0;
    if (evt == KDPWM_EVENT_NONE) {
        TIMER_DMAINTEN(kd->_host->timer) &= ~GENMASK(6, 0);
        return;
    }
    if (evt & KDPWM_EVENT_UPDATE) {
        if (enable) {
            TIMER_DMAINTEN(kd->_host->timer) |= TIMER_DMAINTEN_UPIE;
        } else {
            TIMER_DMAINTEN(kd->_host->timer) &= ~TIMER_DMAINTEN_UPIE;
        }
    }
    TIMER_INTF(kd->_host->timer) = 0;
}

/*@}*/
