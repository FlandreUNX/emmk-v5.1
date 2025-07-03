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

static void gpioModeSet(uint32_t gpio_periph, uint32_t mode, uint32_t pin) {
    uint32_t ctl = GPIO_CTL(gpio_periph);
    for (uint16_t i = 0U; i < 16U; i++) {
        if ((1U << i) & pin) {
            ctl &= ~GPIO_MODE_MASK(i);
            ctl |= GPIO_MODE_SET(i, mode);
        }
    }
    GPIO_CTL(gpio_periph) = ctl;
}


void _gpio_modeConfig(uint32_t port, uint32_t pin, kdgpio_Mode_t mode) {
    switch (mode) {
        case KDGPIO_MODE_INPUT: {
            gpioModeSet(port, GPIO_MODE_INPUT, pin);
            break;
        }
        case KDGPIO_MODE_AF_PP:
        case KDGPIO_MODE_OUTPUT_PP: {
            gpioModeSet(port, mode == KDGPIO_MODE_AF_PP ? GPIO_MODE_AF : GPIO_MODE_OUTPUT, pin);
            gpio_output_options_set(port, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, pin);
            break;
        }
        case KDGPIO_MODE_AF_OD:
        case KDGPIO_MODE_OUTPUT_OD: {
            gpioModeSet(port, mode == KDGPIO_MODE_AF_OD ? GPIO_MODE_AF : GPIO_MODE_OUTPUT, pin);
            gpio_output_options_set(port, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, pin);
            break;
        }
        case KDGPIO_MODE_AIN: {
            gpioModeSet(port, GPIO_MODE_ANALOG, pin);
            break;
        }
    }
}


void _gpio_pullConfig(uint32_t port, uint32_t pin, kdgpio_PullResistor_t pull) {
    uint32_t pupd = GPIO_PUD(port);
    for (int i = 0U; i < 16U; i++) {
        if ((1U << i) & pin) {
            pupd &= ~GPIO_PUPD_MASK(i);
            pupd |= GPIO_PUPD_SET(i, pull);
        }
    }
    GPIO_PUD(port) = pupd;
}


void _gpio_output(uint32_t port, uint32_t pin, bool set) {
    if (set) {
        GPIO_BOP(port) = pin;
    } else {
        GPIO_BC(port) = pin;
    }
}


void _gpio_afConfig(uint32_t port, uint32_t number, uint32_t pin, uint32_t af) {
    (void) number;

    gpio_af_set(port, af, pin);
}

/*@}*/

/**
 * @addtogroup Public functions
 * @note none
 */

/*@{*/

int32_t kdgpio_init(kdgpio_t *kd) {
    ASSERT(kd != NULL);
    return 0;
}

int32_t kdgpio_finalize(kdgpio_t *kd) {
    ASSERT(kd != NULL);
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
    _gpio_afConfig(kd->_config.base.port, kd->_config.base.number, kd->_config.base.pin, 0);
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
        GPIO_BOP(kd->_config.base.port) = kd->_config.base.pin;
    } else {
        GPIO_BC(kd->_config.base.port) = kd->_config.base.pin;
    }
}

void kdgpio_brr(kdgpio_t *kd) {
    GPIO_BC(kd->_config.base.port) = kd->_config.base.pin;
}

void kdgpio_bsrr(kdgpio_t *kd) {
    GPIO_BOP(kd->_config.base.port) = kd->_config.base.pin;
}

uint32_t kdgpio_input(kdgpio_t *kd) {
    return GPIO_ISTAT(kd->_config.base.port) & kd->_config.base.pin;
}

void kdgpio_toggle(kdgpio_t *kd) {
    gpio_bit_toggle(kd->_config.base.port, kd->_config.base.pin);
}

void kdgpio_irqEnable(kdgpio_t *kd, kdgpio_EventTrigger_t t, kdgpio_SignalEvent_t cbEvent) {
    if (kd->_va != NULL) {
        kd->_va->cbEvt = cbEvent;
    }
    if (t == KDGPIO_TRIGGER_NONE) {
        if (kd->_va != NULL) {
            kd->_va->cbEvt = NULL;
        }

        exti_init(kd->_config.extiConfig.eLine, EXTI_INTERRUPT, EXTI_TRIG_NONE);
        exti_interrupt_disable(kd->_config.extiConfig.eLine);
        exti_flag_clear(kd->_config.extiConfig.eLine);
        return;
    }

    if (t == KDGPIO_TRIGGER_RISING) {
        exti_init(kd->_config.extiConfig.eLine, EXTI_INTERRUPT, EXTI_TRIG_RISING);
    } else if (t == KDGPIO_TRIGGER_FALLING) {
        exti_init(kd->_config.extiConfig.eLine, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
    } else if (t == KDGPIO_TRIGGER_RISING_FALLING) {
        exti_init(kd->_config.extiConfig.eLine, EXTI_INTERRUPT, EXTI_TRIG_BOTH);
    }

    syscfg_exti_line_config(kd->_config.extiConfig.portSource, kd->_config.extiConfig.pinSource);
    exti_interrupt_enable(kd->_config.extiConfig.eLine);
    exti_flag_clear(kd->_config.extiConfig.eLine);
}

kdgpio_Event_t kdgpio_irqStatusSelect(kdgpio_t *kd) {
    if (exti_interrupt_flag_get(kd->_config.extiConfig.eLine)) {
        return KDGPIO_EVENT_IRQ;
    }
    return 0;
}

void kdgpio_irqEventClean(kdgpio_t *kd, kdgpio_Event_t e) {
    (void) e;
    exti_interrupt_flag_clear(kd->_config.extiConfig.eLine);
}

/*@}*/
