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
 * @addtogroup Interface define
 * @note none
 */

/*@{*/

static const char *TAG = "kdi2c";

/*@}*/

/**
 * @addtogroup Mutex support
 * @note none
 */

/*@{*/

void threadYield(void) {
    vPortYield();
}

/*@}*/

/**
 * @addtogroup Public Functions
 * @note none
 */

/*@{*/

int32_t kdi2c_init(kdi2c_t *kd) {
    ASSERT(kd != NULL);

    if (emmkDriver_initRefsCountUp(&kd->_va->initRefs) != 0) {
        return -1;
    }

    i2c_config_t i2cConfig = {
            .mode = I2C_MODE_MASTER,
            .master = {
                    .clk_speed = kd->_instance.clockSpeed,
            },
            .scl_io_num = (int) kd->_config.pinConfig.scl.gpio->_config.base.number,
            .sda_io_num = (int) kd->_config.pinConfig.sda.gpio->_config.base.number,
            .scl_pullup_en = 0,
            .sda_pullup_en = 0,
    };
    ESP_ERROR_CHECK(i2c_param_config(kd->_instance.i2c, &i2cConfig));

    kd->_va->mutex = xSemaphoreCreateMutex();
    ASSERT(kd->_va->mutex != NULL);

    return 0;
}


int32_t kdi2c_finalize(kdi2c_t *kd) {
    ASSERT(kd != NULL);

    if (emmkDriver_initRefsCountDown(&kd->_va->initRefs) != 0) {
        return -1;
    }

    if (kd->_va->initRefsPower != 0) {
        kd->_va->initRefsPower = 1;
        kdi2c_powerDown(kd);
    }

    vSemaphoreDelete(kd->_va->mutex);
    kd->_va->mutex = NULL;

    return 0;
}


int32_t kdi2c_powerUp(kdi2c_t *kd) {
    if (emmkDriver_initRefsCountUp(&kd->_va->initRefsPower) != 0) {
        return -1;
    }

    ESP_ERROR_CHECK(i2c_driver_install(kd->_instance.i2c, I2C_MODE_MASTER,
                                       0, 0, 0));

    return 0;
}


 int32_t kdi2c_powerDown(kdi2c_t *kd) {
    if (emmkDriver_initRefsCountDown(&kd->_va->initRefsPower) != 0) {
        return -1;
    }

    i2c_driver_delete(kd->_instance.i2c);

    kdgpio_powerDown(kd->_config.pinConfig.sda.gpio);
    kdgpio_powerDown(kd->_config.pinConfig.scl.gpio);

    return 0;
}


void kdi2c_reset(kdi2c_t *kd) {
//    kdi2c_powerDown(kd);
//    kdi2c_finalize(kd);
//
//    gpio_config_t ioConfig = {0};
//    ioConfig.mode = GPIO_MODE_OUTPUT;
//    ioConfig.pin_bit_mask = BIT64(kd->_config.pinConfig.sda.pin);
//    gpio_config(&ioConfig);
//    ioConfig.mode = GPIO_MODE_OUTPUT;
//    ioConfig.pin_bit_mask = BIT64(kd->_config.pinConfig.scl.pin);
//    gpio_config(&ioConfig);
//
//    gpio_set_level(kd->_config.pinConfig.sda.pin, 1);
//    gpio_set_level(kd->_config.pinConfig.scl.pin, 1);
//
//    for (register uint32_t i = 0; i < 9; i++) {
//        gpio_set_level(kd->_config.pinConfig.scl.pin, 0);
//        vTaskDelay(1);
//        gpio_set_level(kd->_config.pinConfig.scl.pin, 1);
//        vTaskDelay(1);
//    }
//
//    gpio_set_level(kd->_config.pinConfig.sda.pin, 1);
//    gpio_set_level(kd->_config.pinConfig.scl.pin, 1);
//
//    ioConfig.mode = GPIO_MODE_DISABLE;
//    ioConfig.pin_bit_mask = BIT64(kd->_config.pinConfig.scl.pin);
//    gpio_config(&ioConfig);
//    ioConfig.mode = GPIO_MODE_DISABLE;
//    ioConfig.pin_bit_mask = BIT64(kd->_config.pinConfig.sda.pin);
//    gpio_config(&ioConfig);
//
//    kdi2c_init(kd);
//    kdi2c_powerUp(kd);
}


void kdi2c_setSpeed(kdi2c_t *kd, uint32_t speed) {
}


int32_t kdi2c_transmit(kdi2c_t *kd, uint8_t addr,
                          uint8_t *wbuf, uint16_t wsize,
                          uint8_t *rbuf, uint16_t rsize) {
    int32_t rc = -1;

    xSemaphoreTake(kd->_va->mutex, portMAX_DELAY);

    if (i2c_master_write_read_device(kd->_instance.i2c, addr, wbuf, wsize, rbuf, rsize, 100) == ESP_OK) {
        rc = 0;
    }

    if (rc < 0) {
        kdi2c_reset(kd);
        xSemaphoreGive(kd->_va->mutex);
        return -1;
    }
    xSemaphoreGive(kd->_va->mutex);
    return 0;
}


int32_t kdi2c_write(kdi2c_t *kd, uint8_t addr,
                       uint8_t *wbuf, uint16_t wsize) {
    int32_t rc = -1;

    xSemaphoreTake(kd->_va->mutex, portMAX_DELAY);

    if (i2c_master_write_to_device(kd->_instance.i2c, addr, wbuf, wsize, 100) == ESP_OK) {
        rc = 0;
    }

    if (rc < 0) {
        kdi2c_reset(kd);
        xSemaphoreGive(kd->_va->mutex);
        return -1;
    }
    xSemaphoreGive(kd->_va->mutex);
    return 0;
}


int32_t kdi2c_read(kdi2c_t *kd, uint8_t addr,
                      uint8_t *rbuf, uint16_t rsize) {
    int32_t rc = -1;

    xSemaphoreTake(kd->_va->mutex, portMAX_DELAY);

    if (i2c_master_read_from_device(kd->_instance.i2c, addr, rbuf, rsize, 100) == ESP_OK) {
        rc = 0;
    }

    if (rc < 0) {
        kdi2c_reset(kd);
        xSemaphoreGive(kd->_va->mutex);
        return -1;
    }
    xSemaphoreGive(kd->_va->mutex);

    return 0;
}


int32_t kdi2c_regWrite(kdi2c_t *kd, uint8_t addr,
                          uint32_t reg, uint8_t regLen,
                          uint8_t *data, uint16_t len) {
    int32_t rc = -1;

    xSemaphoreTake(kd->_va->mutex, portMAX_DELAY);

    uint8_t regs[4] = {0};
    switch (regLen) {
        case 1:
            regs[0] = reg;
            break;
        case 2:
            regs[0] = reg >> 8 & 0x00FF;
            regs[1] = reg & 0x00FF;
            break;
        case 3:
            regs[0] = reg >> 16 & 0x0000FF;
            regs[1] = reg >> 8 & 0x0000FF;
            regs[2] = reg & 0x0000FF;
            break;
        case 4:
            regs[0] = reg >> 24 & 0x000000FF;
            regs[1] = reg >> 16 & 0x000000FF;
            regs[2] = reg >> 8 & 0x000000FF;
            regs[3] = reg & 0x000000FF;
            break;
    }

    if (i2c_master_write_to_device(kd->_instance.i2c, addr, regs, regLen, 100) != ESP_OK) {
        goto l_exit;
    }
    if (i2c_master_write_to_device(kd->_instance.i2c, addr, data, len, 100) != ESP_OK) {
        goto l_exit;
    }
    rc = 0;

    l_exit:
    if (rc < 0) {
        kdi2c_reset(kd);
        xSemaphoreGive(kd->_va->mutex);
        return -1;
    }
    xSemaphoreGive(kd->_va->mutex);

    return 0;
}


int32_t kdi2c_regRead(kdi2c_t *kd, uint8_t addr,
                         uint32_t reg, uint8_t regLen,
                         uint8_t *data, uint16_t len) {
    uint8_t regs[4] = {0};
    switch (regLen) {
        case 1:
            regs[0] = reg;
            break;
        case 2:
            regs[0] = reg >> 8 & 0x00FF;
            regs[1] = reg & 0x00FF;
            break;
        case 3:
            regs[0] = reg >> 16 & 0x0000FF;
            regs[1] = reg >> 8 & 0x0000FF;
            regs[2] = reg & 0x0000FF;
            break;
        case 4:
            regs[0] = reg >> 24 & 0x000000FF;
            regs[1] = reg >> 16 & 0x000000FF;
            regs[2] = reg >> 8 & 0x000000FF;
            regs[3] = reg & 0x000000FF;
            break;
    }

    return kdi2c_transmit(kd, addr, regs, regLen, data, len);
}


 int32_t kdi2c_start(kdi2c_t *kd, uint8_t addr8RW) {
    return -1;
}


int32_t kdi2c_sendAndWaitAck(kdi2c_t *kd, uint8_t *wbuf, uint16_t wsize) {
    return -1;
}


int32_t kdi2c_stop(kdi2c_t *kd) {
    return -1;
}

/*@}*/
