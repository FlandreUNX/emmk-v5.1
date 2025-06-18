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

    if (kd->_config.spi.spi->CR2 & SPI_CR2_EN_Msk) {
        kd->_config.spi.spi->CR2 &= ~SPI_CR2_EN_Msk;
        
        kd->_config.spi.spi->CR1 &= ~SPI_CR1_BR_Msk;
        kd->_config.spi.spi->CR1 |= val << SPI_CR1_BR_Pos;
        
        kd->_config.spi.spi->CR2 |= SPI_CR2_EN_Msk;
    } else {
        kd->_config.spi.spi->CR1 &= ~SPI_CR1_BR_Msk;
        kd->_config.spi.spi->CR1 |= val << SPI_CR1_BR_Pos;
    }
}

/*@}*/

/**
 * @addtogroup Public Functions
 * @note none
 */
 
/*@{*/

int32_t kdspi_init(kdspi_t *kd) {
    if (emmkDriver_initRefsCountUp(&kd->_va->initRefs) != 0) {
        return -1;
    }
    
    kd->_instance.enableFunc(kd);
    
    REGBITS_MODIFY(kd->_config.spi.spi->CR1, 
            SPI_CR1_MODE_Msk 
            | SPI_CR1_WIDTH_Msk 
            | SPI_CR1_SSM_Msk 
            | SPI_CR1_SMP_Msk 
            | SPI_CR1_LSBF_Msk 
            | SPI_CR1_BR_Msk 
            | SPI_CR1_MSTR_Msk 
            | SPI_CR1_CPOL_Msk 
            | SPI_CR1_CPHA_Msk,
        kd->_config.spi.init.SPI_Direction 
        | kd->_config.spi.init.SPI_DataSize 
        | kd->_config.spi.init.SPI_NSS 
        | kd->_config.spi.init.SPI_Speed 
        | kd->_config.spi.init.SPI_FirstBit 
        | kd->_config.spi.init.SPI_BaudRatePrescaler 
        | kd->_config.spi.init.SPI_Mode 
        | kd->_config.spi.init.SPI_CPOL 
        | kd->_config.spi.init.SPI_CPHA);
    
    if (kd->_config.spi.defualtSpeed != 0) {
        updateFreq(kd, kd->_config.spi.defualtSpeed);
    }

    _mutexNew(kd);
    
    return 0;
}


int32_t kdspi_finalize(kdspi_t *kd) {
    if (emmkDriver_initRefsCountDown(&kd->_va->initRefs) != 0) {
        return -1;
    }
    if (kd->_va->initRefsPower != 0) {
        kd->_va->initRefsPower = 1;
        kdspi_powerDown(kd);
    }
    
    kd->_config.spi.spi->CR2 = 0;
    kd->_config.spi.spi->CR1 = 0;
    
    kd->_instance.disableFunc(kd);
    
    _mutexDelete(kd);

    return 0;
}


int32_t kdspi_powerUp(kdspi_t *kd) {
    if (emmkDriver_initRefsCountUp(&kd->_va->initRefsPower) != 0) {
        return -1;
    }

    if (kd->_config.pin.clk.gpio != NULL) {
        _gpio_modeConfig(kd->_config.pin.clk.gpio->_config.base.port, 
            kd->_config.pin.clk.gpio->_config.base.pin, 
            KDGPIO_MODE_OUTPUT_PP);
        _gpio_pullConfig(kd->_config.pin.clk.gpio->_config.base.port, 
            kd->_config.pin.clk.gpio->_config.base.pin, 
            KDGPIO_PULL_NONE);
        _gpio_afConfig(kd->_config.pin.clk.gpio->_config.base.port, 
            kd->_config.pin.clk.gpio->_config.base.number, 
            kd->_config.pin.clk.af);
    }

    if (kd->_config.pin.mosi.gpio != NULL) {
        _gpio_modeConfig(kd->_config.pin.mosi.gpio->_config.base.port, 
            kd->_config.pin.mosi.gpio->_config.base.pin, 
            KDGPIO_MODE_OUTPUT_PP);
        _gpio_pullConfig(kd->_config.pin.mosi.gpio->_config.base.port, 
            kd->_config.pin.mosi.gpio->_config.base.pin, 
            KDGPIO_PULL_NONE);
        _gpio_afConfig(kd->_config.pin.mosi.gpio->_config.base.port, 
            kd->_config.pin.mosi.gpio->_config.base.number, 
            kd->_config.pin.mosi.af);
    }
    
    if (kd->_config.pin.miso.gpio != NULL) {
        _gpio_modeConfig(kd->_config.pin.miso.gpio->_config.base.port, 
            kd->_config.pin.miso.gpio->_config.base.pin, 
            KDGPIO_MODE_INPUT);
        _gpio_pullConfig(kd->_config.pin.miso.gpio->_config.base.port, 
            kd->_config.pin.miso.gpio->_config.base.pin, 
            KDGPIO_PULL_NONE);
        _gpio_afConfig(kd->_config.pin.miso.gpio->_config.base.port, 
            kd->_config.pin.miso.gpio->_config.base.number, 
            kd->_config.pin.miso.af);
    }

    kd->_config.spi.spi->CR2 |= SPI_CR2_EN_Msk;

    return 0;
}


int32_t kdspi_powerDown(kdspi_t *kd) {
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
    
    kd->_config.spi.spi->CR2 &= ~SPI_CR2_EN_Msk;
    
    return 0;
}


void kdspi_select(kdspi_t *kd) {
    _mutexLock(kd);
}


void kdspi_unselect(kdspi_t *kd) {
    _mutexUnlock(kd);
}


int32_t kdspi_transmit(kdspi_t *kd, uint8_t *wbuf, uint8_t *rbuf, uint16_t len, uint32_t timeout) {
    (void) kd->_config.spi.spi->DR;
    
    for (register uint32_t i = 0; i < len; i++) {
        while (!(kd->_config.spi.spi->ISR & SPI_FLAG_TXE)) {
            threadYield();
        }
        kd->_config.spi.spi->DR = (uint32_t) wbuf[i];

        while (!(kd->_config.spi.spi->ISR & SPI_FLAG_RXNE)) {
            threadYield();
        }
        rbuf[i] = kd->_config.spi.spi->DR;
    }
    
    return len;
}


int32_t kdspi_sendData(kdspi_t *kd, uint8_t *data, uint16_t len, uint32_t timeout) {
    for (register uint32_t i = 0; i < len; i++) {
        while (!(kd->_config.spi.spi->ISR & SPI_FLAG_TXE)) {
            threadYield();
        }
        kd->_config.spi.spi->DR = (uint32_t) data[i];
        
        while ((kd->_config.spi.spi->ISR & SPI_FLAG_BUSY)) {
            threadYield();
        }
        (void) kd->_config.spi.spi->DR;
    }

    return len;
}


int32_t kdspi_recvData(kdspi_t *kd, uint8_t *data, uint16_t len, uint32_t timeout) {
    (void) kd->_config.spi.spi->DR;
    
    for (register uint32_t i = 0; i < len; i++) {
        while (!(kd->_config.spi.spi->ISR & SPI_FLAG_TXE)) {
            threadYield();
        }
        kd->_config.spi.spi->DR = 0xFF;
        
        while (!(kd->_config.spi.spi->ISR & SPI_FLAG_RXNE)) {
            threadYield();
        }
        data[i] = kd->_config.spi.spi->DR;
    }

    return 0;
}


void kdspi_setBaudRate(kdspi_t *kd, uint32_t br) {
    _mutexLock(kd);
    updateFreq(kd, br);
    _mutexUnlock(kd);
}

/*@}*/

