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
 * @addtogroup Static hal func
 * @note none
 */
 
/*@{*/

void _gpio_modeConfig(PORT_TypeDef *port, GPIO_TypeDef *gpio, uint32_t pin, kdgpio_Mode_t mode) {
    switch (mode) {
        case KDGPIO_MODE_INPUT: {
            gpio->DIR &= ~pin;
            break;
        }
        case KDGPIO_MODE_AF_PP:
        case KDGPIO_MODE_OUTPUT_PP: {
            gpio->DIR |= pin;
            port->OPEND &= ~pin;
            break;
        }
        case KDGPIO_MODE_AF_OD:
        case KDGPIO_MODE_OUTPUT_OD: {
            gpio->DIR |= pin;
            port->OPEND |= pin;
            break;
        }
        case KDGPIO_MODE_AIN: {
            break;
        }
    }
}

void _gpio_pullConfig(PORT_TypeDef *port, uint32_t pin, kdgpio_PullResistor_t pull) {
    if (pull == KDGPIO_PULL_UP) {
        port->PULLU |= pin;
    } else if (pull == KDGPIO_PULL_DOWN) {
        port->PULLD |= pin;
    } else {
        port->PULLU &= ~pin;
        port->PULLD &= ~pin;
    }
}

/*@}*/

/**
 * @addtogroup Public functions
 * @note none
 */
 
/*@{*/

int32_t kdgpio_init(kdgpio_t *kd) {
    return 0;
}

int32_t kdgpio_finalize(kdgpio_t *kd) {
    return 0;
}

int32_t kdgpio_powerUp(kdgpio_t *kd, kdgpio_Mode_t mode, kdgpio_PullResistor_t pull) {
    PORT_Init(kd->_config.base.port, kd->_config.base.number, 0, 1);
    _gpio_modeConfig(kd->_config.base.port, kd->_config.base.gpio, kd->_config.base.pin, mode);
    _gpio_pullConfig(kd->_config.base.port, kd->_config.base.pin, pull);
    return 0;
}

int32_t kdgpio_powerDown(kdgpio_t *kd) {
    PORT_Init(kd->_config.base.port, kd->_config.base.number, 0, 0);
    _gpio_modeConfig(kd->_config.base.port, kd->_config.base.gpio, kd->_config.base.pin, kd->_config.downCfg.mode);
    _gpio_pullConfig(kd->_config.base.port, kd->_config.base.pin, kd->_config.downCfg.pull);
    if (kd->_config.downCfg.outputLevel == KDGPIO_DOWN_LEVEL_LOW) {
        kdgpio_output(kd, 0);
    } else {
        kdgpio_output(kd, 1);
    }
    return 0;
}

void kdgpio_setPull(kdgpio_t *kd, kdgpio_PullResistor_t pull) {
   _gpio_pullConfig(kd->_config.base.port, kd->_config.base.pin, pull);
}

void kdgpio_setMode(kdgpio_t *kd, kdgpio_Mode_t mode) {
    _gpio_modeConfig(kd->_config.base.port, kd->_config.base.gpio, kd->_config.base.pin, mode);
}

void kdgpio_output(kdgpio_t *kd, uint8_t v) {
    if (v) {
        kd->_config.base.gpio->DATA |= kd->_config.base.pin;
    } else {
        kd->_config.base.gpio->DATA &= ~kd->_config.base.pin;
    }
}


void kdgpio_brr(kdgpio_t *kd) {
    kd->_config.base.gpio->DATA |= kd->_config.base.pin;
}

void kdgpio_bsrr(kdgpio_t *kd) {
    kd->_config.base.gpio->DATA &= ~kd->_config.base.pin;
}

uint32_t kdgpio_input(kdgpio_t *kd) {
    return kd->_config.base.gpio->DATA & kd->_config.base.pin;
}

void kdgpio_toggle(kdgpio_t *kd) {
    kd->_config.base.gpio->DATA = kd->_config.base.gpio->DATA ^ kd->_config.base.pin;
}

void kdgpio_irqEnable(kdgpio_t *kd, kdgpio_EventTrigger_t t, kdgpio_SignalEvent_t cbEvent) {
    if (kd->_va != NULL) {
        kd->_va->cbEvt = cbEvent;
    }
    if (t == KDGPIO_TRIGGER_NONE) {
        EXTI_Close(kd->_config.base.gpio, kd->_config.base.number);
        if (kd->_va != NULL) {
            kd->_va->cbEvt = NULL;
        }
        return;
    } 
    
    EXTI_Init(kd->_config.base.gpio, kd->_config.base.number, t & 0x7F);
    EXTI_Open(kd->_config.base.gpio, kd->_config.base.number);
}

kdgpio_Event_t kdgpio_irqStatusSelect(kdgpio_t *kd) {
    return 0;
}

void kdgpio_irqEventClean(kdgpio_t *kd, kdgpio_Event_t e) {
}

/*@}*/

#pragma GCC pop_options
