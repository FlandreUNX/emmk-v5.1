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

    if (kd->_host->isInitFunc(kd)) {
        return 0;
    }
    kd->_host->enableFunc(kd);

    if (kd->_host->isTimx) {
        R32_TMR_CNT_END(kd->_host->ins.timx) = kd->_host->cfg.timx.arr;
        R8_TMR_CTRL_MOD(kd->_host->ins.timx) = 0;
        R8_TMR_CTRL_MOD(kd->_host->ins.timx) |= RB_TMR_ALL_CLEAR;
        R8_TMR_CTRL_MOD(kd->_host->ins.timx) |= ((uint32_t) kd->_host->cfg.timx.outputPol) << 4;
    }
    return 0;
}


int32_t kdpwm_finalize(kdpwm_t *kd) {
    if (emmkDriver_initRefsCountDown(&kd->_va->initRefs) != 0) {
        return -1;
    }
    
    if (kd->_host->isTimx) {
        R8_TMR_CTRL_MOD(kd->_host->ins.timx) = RB_TMR_ALL_CLEAR;
    }

    kd->_host->disableFunc(kd);
 
    return 0;
}


int32_t kdpwm_powerUp(kdpwm_t *kd) {
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
            0);
    }

    if (kd->_host->isTimx) {
        R8_TMR_CTRL_MOD(kd->_host->ins.timx) |= RB_TMR_OUT_EN;
    }
    return 0;
}


int32_t kdpwm_powerDown(kdpwm_t *kd) {
    if (kd->_config.pin.gpio != NULL) {
        _gpio_afConfig(kd->_config.pin.gpio->_config.base.port, 
            kd->_config.pin.gpio->_config.base.number, 
            0);
        kdgpio_powerDown(kd->_config.pin.gpio);
    }

    if (kd->_host->isTimx) {
        R8_TMR_CTRL_MOD(kd->_host->ins.timx) &= ~RB_TMR_OUT_EN;
    }

    return 0;
}


void kdpwm_setFreq(kdpwm_t *kd, uint32_t freq) {
    if (kd->_host->isTimx) {
        R32_TMR_CNT_END(kd->_host->ins.timx) = kd->_host->sysFreq / freq;
    }
}


void kdpwm_setPrescaler(kdpwm_t *kd, uint32_t prescaler) {
    kdpwm_setAutoReload(kd, prescaler);
}


void kdpwm_setAutoReload(kdpwm_t *kd, uint32_t autoReload) {
    if (kd->_host->isTimx) {
        R32_TMR_CNT_END(kd->_host->ins.timx) = autoReload;
    }
}


uint32_t kdpwm_getAutoReload(kdpwm_t *kd) {
    if (kd->_host->isTimx) {
        return R32_TMR_CNT_END(kd->_host->ins.timx);
    }
    return 0;
}


void kdpwm_setDuty(kdpwm_t *kd, uint32_t duty) {
    if (kd->_host->isTimx) {
        R32_TMR_FIFO(kd->_host->ins.timx) = duty;
    }
}


uint32_t kdpwm_getDuty(kdpwm_t *kd) {
    if (kd->_host->isTimx) {
        return R32_TMR_FIFO(kd->_host->ins.timx);
    }
    return 0;
}


void kdpwm_counter(kdpwm_t *kd, uint8_t enable) {
    if (kd->_host->isTimx) {
        if (enable) {
            R8_TMR_CTRL_MOD(kd->_host->ins.timx) |= RB_TMR_COUNT_EN;
        } else {
            R8_TMR_CTRL_MOD(kd->_host->ins.timx) &= ~RB_TMR_COUNT_EN;
        }
    }
}


void kdpwm_counterReset(kdpwm_t *kd) {
    R8_TMR_CTRL_MOD(kd->_host->ins.timx) |= RB_TMR_ALL_CLEAR;
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

    if (kd->_host->isTimx) {
        R8_TMR_INT_FLAG(kd->_host->ins.timx) |= KDPWM_EVENT_UPDATE | KDPWM_EVENT_PWM_ACT_END;
        if (evt == KDPWM_EVENT_NONE) {
            R8_TMR_INTER_EN(kd->_host->ins.timx) &= ~(KDPWM_EVENT_UPDATE | KDPWM_EVENT_PWM_ACT_END);
            return;
        }
        if (evt & KDPWM_EVENT_UPDATE) {
            if (enable) {
                R8_TMR_INTER_EN(kd->_host->ins.timx) |= (KDPWM_EVENT_UPDATE);
            } else {
                R8_TMR_INTER_EN(kd->_host->ins.timx) &= ~(KDPWM_EVENT_UPDATE);
            }
        }
        if (evt & KDPWM_EVENT_PWM_ACT_END) {
            if (enable) {
                R8_TMR_INTER_EN(kd->_host->ins.timx) |= (KDPWM_EVENT_PWM_ACT_END);
            } else {
                R8_TMR_INTER_EN(kd->_host->ins.timx) &= ~(KDPWM_EVENT_PWM_ACT_END);
            }
        }
        R8_TMR_INT_FLAG(kd->_host->ins.timx) |= KDPWM_EVENT_UPDATE | KDPWM_EVENT_PWM_ACT_END;
    }
}

/*@}*/
