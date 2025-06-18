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

static void configExtiEnable(kdgpio_t *kd, uint8_t type) {
    uint32_t linepos = (kd->_config.extiConfig.eLine & EXTI_PIN_MASK);
    if (type == KDGPIO_TRIGGER_NONE) {
        EXTI->FTSR &= ~(0x01 << kd->_config.base.number);
        EXTI->RTSR &= ~(0x01 << kd->_config.base.number);
        return;
    }
    
    if (type == KDGPIO_TRIGGER_FALLING) {
        EXTI->FTSR |= FIELD_PREP(kd->_config.base.pin, 1);
    } else if (type == KDGPIO_TRIGGER_RISING) {
        EXTI->RTSR |= FIELD_PREP(kd->_config.base.pin, 1);
    } else if (type == KDGPIO_TRIGGER_RISING_FALLING) {
        EXTI->FTSR |= FIELD_PREP(kd->_config.base.pin, 1);
        EXTI->RTSR |= FIELD_PREP(kd->_config.base.pin, 1);
    }
    if ((kd->_config.extiConfig.eLine & EXTI_GPIO) == EXTI_GPIO) {
        uint32_t regval = EXTI->EXTICR[linepos >> 2u];
        regval &= ~(EXTI_EXTICR1_EXTI0 << (EXTI_EXTICR1_EXTI1_Pos * (linepos & 0x03u)));
        regval |= ((uint32_t) kd->_config.extiConfig.ePort << (EXTI_EXTICR1_EXTI1_Pos * (linepos & 0x03u)));
        EXTI->EXTICR[linepos >> 2u] = regval;
    }
    EXTI->IMR |= FIELD_PREP(kd->_config.base.pin, 1);
}

/*@}*/

/**
 * @addtogroup Static hal func
 * @note none
 */
 
/*@{*/

void _gpio_modeConfig(GPIO_TypeDef *port, uint32_t number, uint32_t pin, kdgpio_Mode_t mode) {
    port->MODER &= ~(0b11 << 2 * number);
    port->OTYPER &= ~pin;
    
    switch (mode) {
        case KDGPIO_MODE_INPUT: {
            break;
        }
        case KDGPIO_MODE_AF_PP: {
            port->MODER |= (0b10 << 2 * number);
            break;
        }
        case KDGPIO_MODE_OUTPUT_PP: {
            port->MODER |= (0b01 << 2 * number);
            break;
        }
        case KDGPIO_MODE_AF_OD: {
            port->MODER |= (0b10 << 2 * number);
            port->OTYPER |= pin;
            break;
        }
        case KDGPIO_MODE_OUTPUT_OD: {
            port->MODER |= (0b01 << 2 * number);
            port->OTYPER |= pin;
            break;
        }
        case KDGPIO_MODE_AIN: {
            port->MODER |= (0b11 << 2 * number);
            break;
        }
    }
}

void _gpio_pullConfig(GPIO_TypeDef *port, uint32_t number, kdgpio_PullResistor_t pull) {
    port->PUPDR &= ~(GPIO_PUPDR_PUPD0_Msk << 2 * number);
    if (pull == KDGPIO_PULL_UP) {
        port->PUPDR |= (GPIO_PUPDR_PUPD0_0 << 2 * number);
    } else if (pull == KDGPIO_PULL_DOWN) {
        port->PUPDR |= (GPIO_PUPDR_PUPD0_1 << 2 * number);
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
        port->AFR[1] &= ~(0x0F << (4 * (number % 8)));
        port->AFR[1] |= (af << (4 * (number % 8)));
    } else {
        port->AFR[0] &= ~(0x0F << (4 * (number % 8)));
        port->AFR[0] |= (af << (4 * (number % 8)));
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
    ASSERT(kd != NULL);
    
    _gpio_modeConfig(kd->_config.base.port, kd->_config.base.number, kd->_config.base.pin, mode);
    _gpio_pullConfig(kd->_config.base.port, kd->_config.base.pin, pull);
    _gpio_afConfig(kd->_config.base.port, kd->_config.base.number, 0);

    return 0;
}

int32_t kdgpio_powerDown(kdgpio_t *kd) {
    ASSERT(kd != NULL);
    _gpio_afConfig(kd->_config.base.port, kd->_config.base.number, 0);
    _gpio_modeConfig(kd->_config.base.port, kd->_config.base.number, kd->_config.base.pin, kd->_config.downCfg.mode);
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
     _gpio_modeConfig(kd->_config.base.port, kd->_config.base.number, kd->_config.base.pin, mode);
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
    kd->_config.base.port->ODR = kd->_config.base.port->ODR ^ kd->_config.base.pin;
}

void kdgpio_irqEnable(kdgpio_t *kd, kdgpio_EventTrigger_t t, kdgpio_SignalEvent_t cbEvent) {
    EXTI->PR = kd->_config.base.pin;
    if (kd->_va != NULL) {
        kd->_va->cbEvt = cbEvent;
    }
    if (t == KDGPIO_TRIGGER_NONE) {
        configExtiEnable(kd, KDGPIO_TRIGGER_NONE);
        if (kd->_va != NULL) {
            kd->_va->cbEvt = NULL;
        }
        return;
    } else if (t == KDGPIO_TRIGGER_RISING) {
         configExtiEnable(kd, KDGPIO_TRIGGER_RISING);
    } else if (t == KDGPIO_TRIGGER_FALLING) {
        configExtiEnable(kd, KDGPIO_TRIGGER_FALLING);
    } else if (t == KDGPIO_TRIGGER_RISING_FALLING) {
        configExtiEnable(kd, KDGPIO_TRIGGER_RISING_FALLING);
    }
}

kdgpio_Event_t kdgpio_irqStatusSelect(kdgpio_t *kd) {
    if (EXTI->PR & kd->_config.base.pin) {
        return KDGPIO_EVENT_IRQ;
    }
    return 0;
}

void kdgpio_irqEventClean(kdgpio_t *kd, kdgpio_Event_t e) {
    EXTI->PR = kd->_config.base.pin;
}

/*@}*/