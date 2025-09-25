//
// Created by unx on 2023/4/29.
//

#include "emmk-config.h"
#include "emmk-driver.h"

/**
 * @addtogroup PortFunc
 * @note none
 */

/*@{*/

void _gpio_modeConfig(uint32_t port, uint32_t pin, kdgpio_Mode_t mode) {
    if (port == (uint32_t) BA_PB) {
        R32_PIN_CONFIG2 &= ~(pin << 16);
    } else {
        R32_PIN_CONFIG2 &= ~pin;
    }

    switch (mode) {
        case KDGPIO_MODE_INPUT: {
            R32_GPIO_DIR(port) &= ~pin;
            break;
        }
        case KDGPIO_MODE_AF_PP: {
            R32_GPIO_PD_DRV(port) &= ~pin;
            R32_GPIO_DIR(port) |= pin;
            break;
        }
        case KDGPIO_MODE_OUTPUT_PP: {
            R32_GPIO_PD_DRV(port) |= pin;
            R32_GPIO_DIR(port) |= pin;
            break;
        }
        case KDGPIO_MODE_AF_OD:
        case KDGPIO_MODE_OUTPUT_OD: {
            break;
        }
        case KDGPIO_MODE_AIN: {
            R32_GPIO_PD_DRV(port) &= ~pin;
            R32_GPIO_DIR(port) &= ~pin;
            R32_GPIO_CLR(port) |= pin;
            if (port == (uint32_t) BA_PB) {
                R32_PIN_CONFIG2 |= pin << 16;
            } else {
                R32_PIN_CONFIG2 |= pin;
            }
            break;
        }
    }
}


void _gpio_pullConfig(uint32_t port, uint32_t pin, kdgpio_PullResistor_t pull) {
    if (pull == KDGPIO_PULL_UP) {
        R32_GPIO_PD_DRV(port) &= ~(pin);
        R32_GPIO_PU(port) |= pin;
    } else if (pull == KDGPIO_PULL_DOWN) {
        R32_GPIO_PD_DRV(port) |= (pin);
        R32_GPIO_PU(port) &= ~pin;
    } else {
        R32_GPIO_PD_DRV(port) &= ~(pin);
        R32_GPIO_PU(port) &= ~pin;
    }
}


void _gpio_output(uint32_t port, uint32_t pin, bool set) {
    if (set) {
        *((__IO uint32_t *) (port + GPIO_OUT)) |= pin;
    } else {
        *((__IO uint32_t *) (port + GPIO_CLR)) |= pin;
    }
}


void _gpio_afConfig(uint32_t port, uint32_t number, uint32_t af) {
}


static void gpio_configIt(uint32_t b, uint32_t pin, GPIOITModeTpDef mode) {
    uint32_t out = b + GPIO_OUT;
    uint32_t intModeOffset = b == (uint32_t) BA_PB ? 16 : 0;

    if (mode == GPIO_ITMode_LowLevel) {
        R32_GPIO_INT_MODE &= ~(pin << intModeOffset);
        *((__IO uint32_t *) out) &= ~pin;
    } else if (mode == GPIO_ITMode_HighLevel) {
        R32_GPIO_INT_MODE &= ~(pin << intModeOffset);
        *((__IO uint32_t *) out) |= pin;
    } else if (mode == GPIO_ITMode_FallEdge) {
        R32_GPIO_INT_MODE |= (pin << intModeOffset);
        *((__IO uint32_t *) out) &= ~pin;
    } else if (mode == GPIO_ITMode_RiseEdge) {
        R32_GPIO_INT_MODE |= (pin << intModeOffset);
        *((__IO uint32_t *) out) |= pin;
    }
}

static void gpio_clearIt(uint32_t b, uint32_t pin) {
    uint32_t offset = b == BA_PB ? 16 : 0;

    R32_GPIO_INT_IF = (pin << offset);
}


static uint32_t gpio_getIt(uint32_t b, uint32_t pin) {
    uint32_t offset = b == BA_PB ? 16 : 0;

    return R32_GPIO_INT_IF & (pin << offset);
}


static void gpio_enableIt(uint32_t b, uint32_t pin, bool en) {
    uint32_t offset = b == BA_PB ? 16 : 0;

    if (en) {
        R32_GPIO_INT_IF = (pin << offset);
        R32_GPIO_INT_EN |= (pin << offset);
    } else {
        R32_GPIO_INT_EN &= ~(pin << offset);
        R32_GPIO_INT_IF = (pin << offset);
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
        *((__IO uint32_t *) (kd->_config.base.port + GPIO_OUT)) |= kd->_config.base.pin;
    } else {
        *((__IO uint32_t *) (kd->_config.base.port + GPIO_CLR)) |= kd->_config.base.pin;
    }
}

void kdgpio_brr(kdgpio_t *kd) {
    *((__IO uint32_t *) (kd->_config.base.port + GPIO_CLR)) |= kd->_config.base.pin;
}

void kdgpio_bsrr(kdgpio_t *kd) {
    *((__IO uint32_t *) (kd->_config.base.port + GPIO_OUT)) |= kd->_config.base.pin;
}

uint32_t kdgpio_input(kdgpio_t *kd) {
    return *((__IO uint32_t *) (kd->_config.base.port + GPIO_PIN)) & kd->_config.base.pin;
}

void kdgpio_toggle(kdgpio_t *kd) {
    *((__IO uint32_t *) (kd->_config.base.port + GPIO_OUT)) ^= kd->_config.base.pin;
}


void kdgpio_irqEnable(kdgpio_t *kd, kdgpio_EventTrigger_t t, kdgpio_SignalEvent_t cbEvent) {
    gpio_enableIt(kd->_config.base.port, kd->_config.base.pin, false);
    if (kd->_va != NULL) {
        kd->_va->cbEvt = cbEvent;
    }
    if (t == KDGPIO_TRIGGER_NONE || t == KDGPIO_TRIGGER_RISING_FALLING) {
        if (kd->_va != NULL) {
            kd->_va->cbEvt = NULL;
        }
        return;
    } else if (t == KDGPIO_TRIGGER_RISING) {
        gpio_configIt(kd->_config.base.port, kd->_config.base.pin, GPIO_ITMode_RiseEdge);
    } else if (t == KDGPIO_TRIGGER_FALLING) {
        gpio_configIt(kd->_config.base.port, kd->_config.base.pin, GPIO_ITMode_FallEdge);
    } else if (t == KDGPIO_TRIGGER_LOW) {
        gpio_configIt(kd->_config.base.port, kd->_config.base.pin, GPIO_ITMode_LowLevel);
    } else if (t == KDGPIO_TRIGGER_HIGH) {
        gpio_configIt(kd->_config.base.port, kd->_config.base.pin, GPIO_ITMode_HighLevel);
    }
    gpio_enableIt(kd->_config.base.port, kd->_config.base.pin, true);
}


kdgpio_Event_t kdgpio_irqStatusSelect(kdgpio_t *kd) {
    if (gpio_getIt(kd->_config.base.port, kd->_config.base.pin)) {
        return KDGPIO_EVENT_IRQ;
    }
    return 0;
}


void kdgpio_irqEventClean(kdgpio_t *kd, kdgpio_Event_t e) {
    gpio_clearIt(kd->_config.base.port, kd->_config.base.pin);
}

/*@}*/
