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

void  _gpio_modeConfig(GPIO_TypeDef *port, uint32_t pin, kdgpio_Mode_t mode) {
    REGBITS_CLR(port->ANALOG, pin); 
    
    switch (mode) {
        case KDGPIO_MODE_INPUT: {
            port->BRR = pin;
            REGBITS_SET(port->DIR, pin);
            break;
        }
        case KDGPIO_MODE_AF_PP:
        case KDGPIO_MODE_OUTPUT_PP: {
            port->BRR = pin;
            REGBITS_CLR(port->OPENDRAIN, pin);
            REGBITS_CLR(port->DIR, pin);
            break;
        }
        case KDGPIO_MODE_AF_OD:
        case KDGPIO_MODE_OUTPUT_OD: {
            port->BRR = pin;
            REGBITS_SET(port->OPENDRAIN, pin);
            REGBITS_CLR(port->DIR, pin);
            break;
        }
        case KDGPIO_MODE_AIN: {
            port->BRR = pin;
            REGBITS_SET(port->DIR, pin);
            REGBITS_CLR(port->OPENDRAIN, pin);
            REGBITS_SET(port->ANALOG, pin);
            break;
        }
    }
}

void _gpio_pullConfig(GPIO_TypeDef *port, uint32_t pin, kdgpio_PullResistor_t pull) {
    if (pull == KDGPIO_PULL_UP) {
        REGBITS_SET(port->PUR, pin);   
    } else {
        REGBITS_CLR(port->PUR, pin);
    }
}

void _gpio_output(GPIO_TypeDef *port, uint32_t pin, bool set) {
    if (set) {
        port->BSRR = pin;
    } else {
        port->BRR = pin;
    }
}

void _gpio_afConfig(GPIO_TypeDef *port, uint32_t number, uint32_t af) {
    if (number / 8) {
        port->AFRH &= ~(0x0F << (4 * (number % 8)));
        port->AFRH |= (af << (4 * (number % 8)));
    } else {
        port->AFRL &= ~(0x0F << (4 * (number % 8)));
        port->AFRL |= (af << (4 * (number % 8)));
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
    _gpio_modeConfig(kd->_config.base.port, kd->_config.base.pin, mode);
    _gpio_pullConfig(kd->_config.base.port, kd->_config.base.pin, pull);
    _gpio_afConfig(kd->_config.base.port, kd->_config.base.number, 0);
    return 0;
}

int32_t kdgpio_powerDown(kdgpio_t *kd) {
    _gpio_modeConfig(kd->_config.base.port, kd->_config.base.pin, kd->_config.downCfg.mode);
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
     _gpio_modeConfig(kd->_config.base.port, kd->_config.base.pin, mode);
}

void kdgpio_output(kdgpio_t *kd, uint8_t v) {
    if (v) {
        kd->_config.base.port->BSRR = kd->_config.base.pin;
    } else {
        kd->_config.base.port->BRR = kd->_config.base.pin;
    }
}

void kdgpio_brr(kdgpio_t *kd) {
    kd->_config.base.port->BRR = kd->_config.base.pin;
}

void kdgpio_bsrr(kdgpio_t *kd) {
    kd->_config.base.port->BSRR = kd->_config.base.pin;
}

uint32_t kdgpio_input(kdgpio_t *kd) {
    return kd->_config.base.port->IDR & kd->_config.base.pin;
}

void kdgpio_toggle(kdgpio_t *kd) {
    kd->_config.base.port->TOG = kd->_config.base.pin;
}

void kdgpio_irqEnable(kdgpio_t *kd, kdgpio_EventTrigger_t t, kdgpio_SignalEvent_t cbEvent) {
    kd->_config.base.port->ICR &= ~kd->_config.base.pin;
    if (kd->_va != NULL) {
        kd->_va->cbEvt = cbEvent;
    }
    if (t == KDGPIO_TRIGGER_NONE) {
        kd->_config.base.port->RISEIE &= ~kd->_config.base.pin;
        kd->_config.base.port->FALLIE &= ~kd->_config.base.pin;
        if (kd->_va != NULL) {
            kd->_va->cbEvt = NULL;
        }
        return;
    } else if (t == KDGPIO_TRIGGER_RISING) {
        kd->_config.base.port->RISEIE |= kd->_config.base.pin;
    } else if (t == KDGPIO_TRIGGER_FALLING) {
        kd->_config.base.port->FALLIE |= kd->_config.base.pin;
    } else if (t == KDGPIO_TRIGGER_RISING_FALLING) {
        kd->_config.base.port->RISEIE |= kd->_config.base.pin;
        kd->_config.base.port->FALLIE |= kd->_config.base.pin;
    }
    kd->_config.base.port->FILTER |= kd->_config.base.pin;
    kd->_config.base.port->ICR &= ~kd->_config.base.pin;
}

kdgpio_Event_t kdgpio_irqStatusSelect(kdgpio_t *kd) {
    if (kd->_config.base.port->ISR & kd->_config.base.pin) {
        return KDGPIO_EVENT_IRQ;
    }
    return 0;
}

void kdgpio_irqEventClean(kdgpio_t *kd, kdgpio_Event_t e) {
    kd->_config.base.port->ICR &= ~kd->_config.base.pin;
}

/*@}*/
