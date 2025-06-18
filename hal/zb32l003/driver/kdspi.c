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

__STATIC_FORCEINLINE void _mutexNew(kdspi_t *kd) {
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    const osMutexAttr_t mutex_attr = {
        .attr_bits = osMutexRobust | osMutexPrioInherit
    };
    
    kd->_va->mutex = osMutexNew(&mutex_attr);
    ASSERT(kd->_va->mutex != NULL);
#endif
}


__STATIC_FORCEINLINE void _mutexDelete(kdspi_t *kd) {
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    osMutexDelete(kd->_va->mutex);
    kd->_va->mutex = NULL;
#endif
}


__STATIC_FORCEINLINE void _mutexLock(kdspi_t *kd) {
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    osMutexAcquire(kd->_va->mutex, osWaitForever);
#endif
}


__STATIC_FORCEINLINE void _mutexUnlock(kdspi_t *kd) {
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    osMutexRelease(kd->_va->mutex);
#endif
}

__STATIC_FORCEINLINE void threadYield(void) {
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    osThreadYield();
#endif
}


static void updateFreq(kdspi_t *kd, uint32_t speed) {
    uint32_t pclk = SystemCoreClock;
    uint32_t val;
    
    for (val = 0U; val < 8U; val++) {
        if (speed >= (pclk >> (val + 1U))) { 
            break; 
        }
    }
    
    if ((val == 8U) || (speed < (pclk >> (val + 1U)))) {
        return;
    }

    if (kd->_config.spi.spi->CR & (uint32_t) (1u << 6u)) {
        kd->_config.spi.spi->CR &= ~(1u << 6u);
        
        kd->_config.spi.spi->CR &= ~(0x03 | (0x80));
        kd->_config.spi.spi->CR |= (val & 0x04) << 5;
        kd->_config.spi.spi->CR |= (val & 0x03);

        kd->_config.spi.spi->CR |= (1u << 6u);
    } else {
        kd->_config.spi.spi->CR &= ~(0x03 | (0x80));
        kd->_config.spi.spi->CR |= (val & 0x04) << 5;
        kd->_config.spi.spi->CR |= (val & 0x03);
    }
}

/*@}*/

/**
 * @addtogroup Public Functions
 * @note none
 */
 
/*@{*/

int32_t kdspi_init(kdspi_t *kd) {
    ASSERT(kd != NULL);
    
    if (emmkDriver_initRefsCountUp(&kd->_va->initRefs) != 0) {
        return -1;
    }
    
    kd->_instance.enableFunc(kd);
    
    kd->_config.spi.spi->CR &= ~0xFFu;
    kd->_config.spi.spi->CR = kd->_config.spi.init.clksel
        | kd->_config.spi.init.mstr
        | kd->_config.spi.init.cpha
        | kd->_config.spi.init.cpol;
    
    if (kd->_config.spi.defualtSpeed != 0) {
        updateFreq(kd, kd->_config.spi.defualtSpeed);
    }

    _mutexNew(kd);
    
    return 0;
}


int32_t kdspi_finalize(kdspi_t *kd) {
    ASSERT(kd != NULL);
    
    if (emmkDriver_initRefsCountDown(&kd->_va->initRefs) != 0) {
        return -1;
    }
    if (kd->_va->initRefsPower != 0) {
        kd->_va->initRefsPower = 1;
        kdspi_powerDown(kd);
    }

    kd->_config.spi.spi->CR &= ~0xFFu;
    
    kd->_instance.disableFunc(kd);
    
    _mutexDelete(kd);

    return 0;
}


int32_t kdspi_powerUp(kdspi_t *kd) {
    ASSERT(kd != NULL);
    
    if (emmkDriver_initRefsCountUp(&kd->_va->initRefsPower) != 0) {
        return -1;
    }

    if (kd->_config.pin.clk.gpio != NULL) {
        _gpio_modeConfig(kd->_config.pin.clk.gpio->_config.base.port, 
            kd->_config.pin.clk.gpio->_config.base.number, 
            kd->_config.pin.clk.gpio->_config.base.pin, 
            KDGPIO_MODE_OUTPUT_PP);
        _gpio_pullConfig(kd->_config.pin.clk.gpio->_config.base.port, 
            kd->_config.pin.clk.gpio->_config.base.number, 
            kd->_config.pin.clk.gpio->_config.base.pin, 
            KDGPIO_PULL_NONE);
        _gpio_afConfig(kd->_config.pin.clk.gpio->_config.base.port, 
            kd->_config.pin.clk.gpio->_config.base.number, 
            kd->_config.pin.clk.af);
    }

    if (kd->_config.pin.mosi.gpio != NULL) {
        _gpio_modeConfig(kd->_config.pin.mosi.gpio->_config.base.port, 
            kd->_config.pin.mosi.gpio->_config.base.number, 
            kd->_config.pin.mosi.gpio->_config.base.pin, 
            KDGPIO_MODE_OUTPUT_PP);
        _gpio_pullConfig(kd->_config.pin.mosi.gpio->_config.base.port, 
            kd->_config.pin.mosi.gpio->_config.base.number, 
            kd->_config.pin.mosi.gpio->_config.base.pin, 
            KDGPIO_PULL_NONE);
        _gpio_afConfig(kd->_config.pin.mosi.gpio->_config.base.port, 
            kd->_config.pin.mosi.gpio->_config.base.number, 
            kd->_config.pin.mosi.af);
    }
    
    if (kd->_config.pin.miso.gpio != NULL) {
        _gpio_modeConfig(kd->_config.pin.miso.gpio->_config.base.port, 
            kd->_config.pin.miso.gpio->_config.base.number, 
            kd->_config.pin.miso.gpio->_config.base.pin, 
            KDGPIO_MODE_INPUT);
        _gpio_pullConfig(kd->_config.pin.miso.gpio->_config.base.port, 
            kd->_config.pin.miso.gpio->_config.base.number, 
            kd->_config.pin.miso.gpio->_config.base.pin, 
            KDGPIO_PULL_NONE);
        _gpio_afConfig(kd->_config.pin.miso.gpio->_config.base.port, 
            kd->_config.pin.miso.gpio->_config.base.number, 
            kd->_config.pin.miso.af);
    }

    kd->_config.spi.spi->CR |= (1u << 6u);

    return 0;
}


int32_t kdspi_powerDown(kdspi_t *kd) {
    ASSERT(kd != NULL);
    
    if (emmkDriver_initRefsCountDown(&kd->_va->initRefsPower) != 0) {
        return -1;
    }
   
    if (kd->_config.pin.clk.gpio != NULL) {
        _gpio_afConfig(kd->_config.pin.clk.gpio->_config.base.port, 
            kd->_config.pin.clk.gpio->_config.base.number, 
            0);
        kdgpio_powerDown(kd->_config.pin.clk.gpio);
    }

    if (kd->_config.pin.mosi.gpio != NULL) {
        _gpio_afConfig(kd->_config.pin.mosi.gpio->_config.base.port, 
            kd->_config.pin.mosi.gpio->_config.base.number, 
            0);
        kdgpio_powerDown(kd->_config.pin.mosi.gpio);
    }
    
    if (kd->_config.pin.miso.gpio != NULL) {
        _gpio_afConfig(kd->_config.pin.miso.gpio->_config.base.port, 
            kd->_config.pin.miso.gpio->_config.base.number, 
            0);
        kdgpio_powerDown(kd->_config.pin.miso.gpio);
    }
    
    kd->_config.spi.spi->CR &= ~(1u << 6u);
    
    return 0;
}


void kdspi_select(kdspi_t *kd) {
    _mutexLock(kd);
}


void kdspi_unselect(kdspi_t *kd) {
    _mutexUnlock(kd);
}


int32_t kdspi_transmit(kdspi_t *kd, uint8_t *wbuf, uint8_t *rbuf, uint16_t len, uint32_t timeout) {
    for (uint16_t i = 0; i < len; i++) {
        SPI->DATA = wbuf[i];
        while (!SPI->SR);
        rbuf[i] = SPI->DATA & 0x000000FF;
    }
    
    return len;
}


int32_t kdspi_sendData(kdspi_t *kd, uint8_t *data, uint16_t len, uint32_t timeout) {
    for (uint16_t i = 0; i < len; i++) {
        SPI->DATA = data[i];
        while (!SPI->SR);
        (void) SPI->DATA;
    }

    return len;
}


int32_t kdspi_recvData(kdspi_t *kd, uint8_t *data, uint16_t len, uint32_t timeout) {
    for (uint16_t i = 0; i < len; i++) {
        SPI->DATA = 0xFF;
        while (!SPI->SR);
        data[i] = SPI->DATA & 0x000000FF;
    }

    return 0;
}


void kdspi_setBaudRate(kdspi_t *kd, uint32_t br) {
    _mutexLock(kd);
    updateFreq(kd, br);
    _mutexUnlock(kd);
}

/*@}*/

