//
// Created by unx on 2023/4/29.
//

#include "emmk-driver.h"
#include "emmk-config.h"

/**
 * @addtogroup Interfaace define
 * @note none
 */

/*@{*/

static const char *TAG = "kdgpio";

/*@}*/

/**
 * @addtogroup Instance support functions
 * @note none
 */

/*@{*/

static volatile int32_t mIsrInstallCount = 0;

void _gpio_modeConfig(gpio_config_t *ioConfig, kdgpio_Mode_t mode) {
    ioConfig->mode = mode;
}

void _gpio_pullConfig(gpio_config_t *ioConfig, kdgpio_PullResistor_t pull) {
    if (pull == KDGPIO_PULL_UP) {
        ioConfig->pull_up_en = GPIO_PULLUP_ENABLE;
        ioConfig->pull_down_en = GPIO_PULLDOWN_DISABLE;
    } else if (pull == KDGPIO_PULL_DOWN) {
        ioConfig->pull_up_en = GPIO_PULLUP_DISABLE;
        ioConfig->pull_down_en = GPIO_PULLDOWN_ENABLE;
    } else {
        ioConfig->pull_up_en = GPIO_PULLUP_DISABLE;
        ioConfig->pull_down_en = GPIO_PULLDOWN_DISABLE;
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

    gpio_config_t ioConfig = {0};
    _gpio_modeConfig(&ioConfig, mode);
    _gpio_pullConfig(&ioConfig, pull);
    ioConfig.pin_bit_mask = kd->_config.base.pin;
    gpio_config(&ioConfig);

    return 0;
}

int32_t kdgpio_powerDown(kdgpio_t *kd) {
    ASSERT(kd != NULL);

    gpio_config_t ioConfig = {0};
    _gpio_modeConfig(&ioConfig, kd->_config.downCfg.mode);
    _gpio_pullConfig(&ioConfig, kd->_config.downCfg.pull);
    ioConfig.pin_bit_mask = kd->_config.base.pin;
    gpio_config(&ioConfig);
    if (kd->_config.downCfg.outputLevel == KDGPIO_DOWN_LEVEL_LOW) {
        kdgpio_output(kd, 0);
    } else if (kd->_config.downCfg.outputLevel == KDGPIO_DOWN_LEVEL_HIGH) {
        kdgpio_output(kd, 1);
    }
    return 0;
}

void kdgpio_setPull(kdgpio_t *kd, kdgpio_PullResistor_t pull) {
    if (pull == KDGPIO_PULL_UP) {
        gpio_pullup_en(kd->_config.base.number);
        gpio_pulldown_dis(kd->_config.base.number);
    } else if (pull == KDGPIO_PULL_DOWN) {
        gpio_pullup_dis(kd->_config.base.number);
        gpio_pulldown_en(kd->_config.base.number);
    } else {
        gpio_pullup_dis(kd->_config.base.number);
        gpio_pulldown_dis(kd->_config.base.number);
    }
}

void kdgpio_setMode(kdgpio_t *kd, kdgpio_Mode_t mode) {
}

void kdgpio_output(kdgpio_t *kd, uint8_t v) {
    if (v) {
        gpio_set_level(kd->_config.base.number, 1);
    } else {
        gpio_set_level(kd->_config.base.number, 0);
    }
}

void kdgpio_brr(kdgpio_t *kd) {
    gpio_set_level(kd->_config.base.number, 0);
}

void kdgpio_bsrr(kdgpio_t *kd) {
    gpio_set_level(kd->_config.base.number, 1);
}

uint32_t kdgpio_input(kdgpio_t *kd) {
    return gpio_get_level(kd->_config.base.number);
}


void kdgpio_toggle(kdgpio_t *kd) {
    uint32_t level = gpio_get_level(kd->_config.base.number);
    if (level) {
        gpio_set_level(kd->_config.base.number, 0);
    } else {
        gpio_set_level(kd->_config.base.number, 1);
    }
}


void kdgpio_irqEnable(kdgpio_t *kd, kdgpio_EventTrigger_t t, kdgpio_SignalEvent_t cbEvent) {
    if (kd->_va != NULL) {
        kd->_va->cbEvt = cbEvent;
    }
    if (t == KDGPIO_TRIGGER_NONE) {
        gpio_set_intr_type(kd->_config.base.number, GPIO_INTR_DISABLE);
        gpio_intr_disable(kd->_config.base.number);

        if (mIsrInstallCount != 0) {
            mIsrInstallCount--;
        }
        if (mIsrInstallCount == 0) {
            gpio_uninstall_isr_service();
        }

        if (kd->_va != NULL) {
            kd->_va->cbEvt = NULL;
        }
        return;
    }

    gpio_set_intr_type(kd->_config.base.number, (gpio_int_type_t) t);
    gpio_intr_enable(kd->_config.base.number);

    if (!mIsrInstallCount) {
        gpio_install_isr_service(0);
    }
    mIsrInstallCount += 1;
}

kdgpio_Event_t kdgpio_irqStatusSelect(kdgpio_t *kd) {
    return 0;
}

void kdgpio_irqEventClean(kdgpio_t *kd, kdgpio_Event_t e) {
}

void esp32_kdgpio_irqHandleReg(kdgpio_t *kd, void (*gpio_isr_t)(void *arg), void *arg) {
    gpio_isr_handler_add(kd->_config.base.number, gpio_isr_t, arg);
}

/*@}*/
