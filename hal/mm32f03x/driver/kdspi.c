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


static void _updateFreq(kdspi_t *kd, uint32_t speed) {
    uint32_t pclk;
    uint32_t val;
    RCC_ClocksTypeDef priClk;
    
    RCC_GetClocksFreq(&priClk);
    
    switch ((uint32_t) kd->_config.spi.spi) {
#ifdef SPI1
         case (uint32_t) SPI1:
            pclk = priClk.PCLK2_Frequency;
            break;
#endif
#ifdef SPI2
         case (uint32_t) SPI2:
            pclk = priClk.PCLK1_Frequency;
            break;
#endif
    }
    

    for (val = 0U; val < 8U; val++) {
        if (speed >= (pclk >> (val + 1U))) { 
            break; 
        }
    }
    
    if ((val == 8U) || (speed < (pclk >> (val + 1U)))) {
        return;
    }

    if (kd->_config.spi.spi->GCTL & SPI_GCTL_SPIEN) {
        kd->_config.spi.spi->GCTL &= ~SPI_GCTL_SPIEN;
        
        kd->_config.spi.spi->SPBRG &= ~0xFFFF;
        kd->_config.spi.spi->SPBRG |= val;
        
        kd->_config.spi.spi->GCTL |= SPI_GCTL_SPIEN;
    } else {
        kd->_config.spi.spi->SPBRG &= ~0xFFFF;
        kd->_config.spi.spi->SPBRG |= val;
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
    
    SPI_Init(kd->_config.spi.spi, &kd->_config.spi.init);
    if (kd->_config.spi.defualtSpeed != 0) {
        _updateFreq(kd, kd->_config.spi.defualtSpeed);
    }
    SPI_BiDirectionalLineConfig(kd->_config.spi.spi, SPI_Direction_Tx);
    SPI_BiDirectionalLineConfig(kd->_config.spi.spi, SPI_Direction_Rx);
    
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
    
    SPI_DeInit(kd->_config.spi.spi);

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
    
    SPI_Cmd(kd->_config.spi.spi, ENABLE);

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
    
    SPI_Cmd(kd->_config.spi.spi, DISABLE);
    
    return 0;
}

void kdspi_select(kdspi_t *kd) {
    _mutexLock(kd);
}

void kdspi_unselect(kdspi_t *kd) {
    _mutexUnlock(kd);
}

int32_t kdspi_transmit(kdspi_t *kd, uint8_t *wbuf, uint8_t *rbuf, uint16_t len, uint32_t timeout) {
    for (register uint32_t i = 0; i < len; i++) {
        while ((kd->_config.spi.spi->CSTAT & SPI_CSTAT_TXFULL)) {
            threadYield();
        }
        
        kd->_config.spi.spi->TXREG = (uint32_t) wbuf[i];

        while (!(kd->_config.spi.spi->CSTAT & SPI_CSTAT_RXAVL)) {
            threadYield();
        }
        rbuf[i] = kd->_config.spi.spi->RXREG;
    }
    
    return len;
}

int32_t kdspi_sendData(kdspi_t *kd, uint8_t *data, uint16_t len, uint32_t timeout) {
    for (register uint32_t i = 0; i < len; i++) {
        while ((kd->_config.spi.spi->CSTAT & SPI_CSTAT_TXFULL)) {
            threadYield();
        }
        
        kd->_config.spi.spi->TXREG = (uint32_t) data[i];
        
        while (!(kd->_config.spi.spi->CSTAT & SPI_CSTAT_RXAVL)) {
            threadYield();
        }
        (void) kd->_config.spi.spi->RXREG;
    }
    
    while (!(kd->_config.spi.spi->CSTAT & SPI_CSTAT_TXEPT)) {
        threadYield();
    }
    
    return len;
}

int32_t kdspi_recvData(kdspi_t *kd, uint8_t *data, uint16_t len, uint32_t timeout) {
    _mutexLock(kd);
    
    for (register uint32_t i = 0; i < len; i++) {
        while ((kd->_config.spi.spi->CSTAT & SPI_CSTAT_TXFULL)) {
            threadYield();
        }
        
        kd->_config.spi.spi->TXREG = 0xFF;
        
        while (!(kd->_config.spi.spi->CSTAT & SPI_CSTAT_RXAVL)) {
            threadYield();
        }
        data[i] = kd->_config.spi.spi->RXREG;
    }

    return 0;
}

void kdspi_setBaudRate(kdspi_t *kd, uint32_t br) {
    _mutexLock(kd);
    _updateFreq(kd, br);
    _mutexUnlock(kd);
}

/*@}*/

