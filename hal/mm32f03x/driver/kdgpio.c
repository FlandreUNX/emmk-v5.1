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

void _gpio_modeConfig(GPIO_TypeDef *port, uint32_t pin, kdgpio_Mode_t mode) {
    GPIO_InitTypeDef init = {
        .GPIO_Pin = pin,
        .GPIO_Speed = GPIO_Speed_50MHz,
    };
    switch (mode) {
        case KDGPIO_MODE_INPUT: {
            init.GPIO_Mode = GPIO_Mode_IN_FLOATING;
            break;
        }
        case KDGPIO_MODE_OUTPUT_PP: {
            init.GPIO_Mode = GPIO_Mode_Out_PP;
            break;
        }
        case KDGPIO_MODE_OUTPUT_OD: {
            init.GPIO_Mode = GPIO_Mode_Out_OD;
            break;
        }
        case KDGPIO_MODE_AF_OD: {
            init.GPIO_Mode = GPIO_Mode_AF_OD;
            break;
        }
        case KDGPIO_MODE_AF_PP: {
            init.GPIO_Mode = GPIO_Mode_AF_PP;
            break;
        }
        case KDGPIO_MODE_AIN: {
            break;
        }
    }
    GPIO_Init(port, &init);
}

void _gpio_pullConfig(GPIO_TypeDef *port, uint32_t pin, kdgpio_PullResistor_t pull) {
    GPIO_InitTypeDef init = {
        .GPIO_Pin = pin,
        .GPIO_Speed = GPIO_Speed_50MHz,
    };
    if (pull == KDGPIO_PULL_UP) {
        init.GPIO_Mode = GPIO_Mode_IPU;
    } else {
        init.GPIO_Mode = GPIO_Mode_IPD;
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
    if (kd->_config.base.port->ODR & kd->_config.base.pin) {
        kdgpio_brr(kd);
    } else {
        kdgpio_bsrr(kd);
    }
}

void kdgpio_irqEnable(kdgpio_t *kd, kdgpio_EventTrigger_t t, kdgpio_SignalEvent_t cbEvent) {
    if (kd->_va != NULL) {
        kd->_va->cbEvt = cbEvent;
    }
    
    EXTI_InitTypeDef extiInit;
    extiInit.EXTI_Line = kd->_config.exti.extiLinePin;
    extiInit.EXTI_LineCmd = ENABLE;
    extiInit.EXTI_Mode = EXTI_Mode_Interrupt;
    
    if (t == KDGPIO_TRIGGER_NONE) {
        EXTI->IMR &= ~(kd->_config.exti.extiLinePin);
        if (kd->_va != NULL) {
            kd->_va->cbEvt = NULL;
        }
        return;
    } else if (t == KDGPIO_TRIGGER_RISING) {
        extiInit.EXTI_Trigger = EXTI_Trigger_Rising;
    } else if (t == KDGPIO_TRIGGER_FALLING) {
        extiInit.EXTI_Trigger = EXTI_Trigger_Falling;
    } else if (t == KDGPIO_TRIGGER_RISING_FALLING) {
        extiInit.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    }
    EXTI_Init(&extiInit);
}

kdgpio_Event_t kdgpio_irqStatusSelect(kdgpio_t *kd) {
    return 0;
}

void kdgpio_irqEventClean(kdgpio_t *kd, kdgpio_Event_t e) {

}

/*@}*/
