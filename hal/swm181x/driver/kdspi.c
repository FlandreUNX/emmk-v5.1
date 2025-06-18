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
 * @addtogroup Mutex support
 * @note none
 */
 
/*@{*/

__STATIC_FORCEINLINE void mutexNew(kdspi_t *kd) {
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    const osMutexAttr_t mutex_attr = {
        .attr_bits = osMutexRobust | osMutexPrioInherit
    };
    
    kd->_va->mutex = osMutexNew(&mutex_attr);
    ASSERT(kd->_va->mutex != NULL);
#endif
}


__STATIC_FORCEINLINE void mutexDelete(kdspi_t *kd) {
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    osMutexDelete(kd->_va->mutex);
    kd->_va->mutex = NULL;
#endif
}


__STATIC_FORCEINLINE void mutexLock(kdspi_t *kd) {
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    osMutexAcquire(kd->_va->mutex, osWaitForever);
#endif
}


__STATIC_FORCEINLINE void mutexUnlock(kdspi_t *kd) {
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    osMutexRelease(kd->_va->mutex);
#endif
}


__STATIC_FORCEINLINE void threadYield(void) {
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    osThreadYield();
#else
    // __WFI();
#endif
}

/*@}*/

/**
 * @addtogroup Static hal func
 * @note none
 */
 
/*@{*/

static void updateFreq(kdspi_t *kd, uint32_t speed) {
    uint32_t val;

    for (val = 0U; val < 8U; val++) {
        if (speed >= (SystemCoreClock >> (val + 1U))) { 
            break; 
        }
    }
    
    if ((val == 8U) || (speed < (SystemCoreClock >> (val + 1U)))) {
        return;
    }

    if (kd->_config.spiConfig.spi->CTRL & SPI_CTRL_EN_Msk) {
        kd->_config.spiConfig.spi->CTRL &= ~SPI_CTRL_EN_Msk;
        kd->_config.spiConfig.spi->CTRL &= ~SPI_CTRL_CLKDIV_Msk;
        kd->_config.spiConfig.spi->CTRL |= val << SPI_CTRL_CLKDIV_Pos;
        kd->_config.spiConfig.spi->CTRL |= SPI_CTRL_EN_Msk;
    } else {
        kd->_config.spiConfig.spi->CTRL &= ~SPI_CTRL_CLKDIV_Msk;
        kd->_config.spiConfig.spi->CTRL |= val << SPI_CTRL_CLKDIV_Pos;
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
    
    SPI_Init(SPI0, &kd->_config.spiConfig.init);
    if (kd->_config.spiConfig.defualtSpeed != 0) {
        updateFreq(kd, kd->_config.spiConfig.defualtSpeed);
    }
    
    mutexNew(kd);

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

    kd->_instance.disableFunc(kd);
    
    mutexDelete(kd);

    return 0;
}

int32_t kdspi_powerUp(kdspi_t *kd) {
    if (emmkDriver_initRefsCountUp(&kd->_va->initRefsPower) != 0) {
        return -1;
    }

    if (kd->_config.pinConfig.clk.gpio != NULL) {
        PORT_Init(kd->_config.pinConfig.clk.gpio->_config.base.port, 
            kd->_config.pinConfig.clk.gpio->_config.base.number, 
            kd->_config.pinConfig.clk.af, 1);
    }
    
    if (kd->_config.pinConfig.miso.gpio != NULL) {
        PORT_Init(kd->_config.pinConfig.miso.gpio->_config.base.port, 
            kd->_config.pinConfig.miso.gpio->_config.base.number, 
            kd->_config.pinConfig.miso.af, 1);
    }
    
    if (kd->_config.pinConfig.mosi.gpio != NULL) {
        PORT_Init(kd->_config.pinConfig.mosi.gpio->_config.base.port, 
            kd->_config.pinConfig.mosi.gpio->_config.base.number, 
            kd->_config.pinConfig.mosi.af, 1);
    }
    
    SPI_Open(kd->_config.spiConfig.spi);

    return 0;
}

int32_t kdspi_powerDown(kdspi_t *kd) {
    if (emmkDriver_initRefsCountDown(&kd->_va->initRefsPower) != 0) {
        return -1;
    }
    
    if (kd->_config.pinConfig.clk.gpio != NULL) {
        PORT_Init(kd->_config.pinConfig.clk.gpio->_config.base.port, 
            kd->_config.pinConfig.clk.gpio->_config.base.number, 
            0, 0);
        kdgpio_powerDown(kd->_config.pinConfig.clk.gpio);
    }
    
    if (kd->_config.pinConfig.miso.gpio != NULL) {
        PORT_Init(kd->_config.pinConfig.miso.gpio->_config.base.port, 
            kd->_config.pinConfig.miso.gpio->_config.base.number, 
            0, 0);
        kdgpio_powerDown(kd->_config.pinConfig.miso.gpio);
    }
    
    if (kd->_config.pinConfig.mosi.gpio != NULL) {
        PORT_Init(kd->_config.pinConfig.mosi.gpio->_config.base.port, 
            kd->_config.pinConfig.mosi.gpio->_config.base.number, 
            0, 0);
        kdgpio_powerDown(kd->_config.pinConfig.mosi.gpio);
    }

    SPI_Close(kd->_config.spiConfig.spi);
    
    return 0;
}

void kdspi_select(kdspi_t *kd) {
    mutexLock(kd);
}

void kdspi_unselect(kdspi_t *kd) {
    mutexUnlock(kd);
}

int32_t kdspi_transmit(kdspi_t *kd, uint8_t *wbuf, uint8_t *rbuf, uint16_t len, uint32_t timeout) {
    for (uint32_t i = 0; i < len; i++) {
        rbuf[i] = SPI_ReadWrite(kd->_config.spiConfig.spi, wbuf[i]);
    }
    
    return len;
}

int32_t kdspi_sendData(kdspi_t *kd, uint8_t *data, uint16_t len, uint32_t timeout) {
    for (uint32_t i = 0; i < len; i++) {
        while (SPI_IsTXFull(kd->_config.spiConfig.spi)) {
            threadYield();
        }
        
        SPI_Write(kd->_config.spiConfig.spi, ((uint8_t *) data)[i]);
    }
    
    while (!SPI_IsTXEmpty(kd->_config.spiConfig.spi)) {
        threadYield();
    }
    
    return len;
}

int32_t kdspi_recvData(kdspi_t *kd, uint8_t *data, uint16_t len, uint32_t timeout) {
    for (uint32_t i = 0; i < len; i++) {
        data[i] = SPI_ReadWrite(kd->_config.spiConfig.spi, 0xFF);
    }
    
    return 0;
}


void kdspi_setBaudRate(kdspi_t *kd, uint32_t br) {
    updateFreq(kd, br);
}

/*@}*/
