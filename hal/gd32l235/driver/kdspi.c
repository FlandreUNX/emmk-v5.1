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
    uint32_t pclk;
    uint32_t val;

    switch ((uint32_t) kd->_config.spi.spi) {
        case (uint32_t) SPI0:
            pclk = rcu_clock_freq_get(CK_APB2);
            break;
#ifdef SPI1
         case (uint32_t) SPI1:
            pclk = rcu_clock_freq_get(CK_APB1);
            break;
#endif
#ifdef SPI2
         case (uint32_t) SPI2:
            pclk = rcu_clock_freq_get(CK_APB1);
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

    if (SPI_CTL0(kd->_config.spi.spi) & (uint32_t) SPI_CTL0_SPIEN) {
        spi_disable(kd->_config.spi.spi);
        SPI_CTL0(kd->_config.spi.spi) &= ~SPI_PSC_256;
        SPI_CTL0(kd->_config.spi.spi) |= CTL0_PSC((val));
        spi_enable(kd->_config.spi.spi);
    } else {
        SPI_CTL0(kd->_config.spi.spi) &= ~SPI_PSC_256;
        SPI_CTL0(kd->_config.spi.spi) |= CTL0_PSC((val));
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
    
    spi_init(kd->_config.spi.spi, &kd->_config.spi.init);
    if (kd->_config.spi.defualtSpeed != 0) {
        updateFreq(kd, kd->_config.spi.defualtSpeed);
    }
    
    if (kd->_config.dma.channelTx != 0xFF) {
        spi_dma_enable(kd->_config.spi.spi, SPI_DMA_TRANSMIT);
        dma_init(kd->_config.dma.channelTx, &kd->_config.dma.initTx);
        dma_circulation_disable(kd->_config.dma.channelTx);
        dma_memory_to_memory_disable( kd->_config.dma.channelTx);
        dma_interrupt_enable(kd->_config.dma.channelTx, DMA_INT_FTF);
    }
    
    if (kd->_config.dma.channelRx != 0xFF) {
        spi_dma_enable(kd->_config.spi.spi, SPI_DMA_RECEIVE);
        dma_init(kd->_config.dma.channelRx, &kd->_config.dma.initRx);
        dma_circulation_disable(kd->_config.dma.channelRx);
        dma_memory_to_memory_disable(kd->_config.dma.channelRx);
    }

    _mutexNew(kd);
    
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    kd->_va->flag.ptr = osEventFlagsNew(NULL);
    ASSERT(kd->_va->flag.ptr != NULL);
#else
    kd->_va->flag._ = 0;
    kd->_va->flag.isSendCompleted = 1;
#endif
    
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
    
    if (kd->_config.dma.channelTx != 0xFF) {
        spi_dma_disable(kd->_config.spi.spi, SPI_DMA_TRANSMIT);
        dma_interrupt_disable(kd->_config.dma.channelTx, DMA_INT_FTF);
        dma_deinit(kd->_config.dma.channelTx);
    }
    
    if (kd->_config.dma.channelRx != 0xFF) {
        spi_dma_disable(kd->_config.spi.spi, SPI_DMA_RECEIVE);
        dma_deinit(kd->_config.dma.channelRx);
    }
            
    spi_i2s_deinit(kd->_config.spi.spi);
    
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
            kd->_config.pin.clk.gpio->_config.base.pin,
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
            kd->_config.pin.mosi.gpio->_config.base.pin,
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
            kd->_config.pin.miso.gpio->_config.base.pin,
            kd->_config.pin.miso.af);
    }

    spi_enable(kd->_config.spi.spi);
    
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    osEventFlagsSet(kd->_va->flag.ptr, KDSPI_FLAG_TRANSMIT_COMPLETE);
#else
    kd->_va->flag._ = 0;
    kd->_va->flag.isSendCompleted = 1;
#endif
    
    return 0;
}


int32_t kdspi_powerDown(kdspi_t *kd) {
    if (emmkDriver_initRefsCountDown(&kd->_va->initRefsPower) != 0) {
        return -1;
    }
   
    if (kd->_config.pin.clk.gpio != NULL) {
        _gpio_afConfig(kd->_config.pin.clk.gpio->_config.base.port, 
            kd->_config.pin.clk.gpio->_config.base.number, 
            kd->_config.pin.clk.gpio->_config.base.pin,
            0);
        kdgpio_powerDown(kd->_config.pin.clk.gpio);
    }

    if (kd->_config.pin.mosi.gpio != NULL) {
        _gpio_afConfig(kd->_config.pin.mosi.gpio->_config.base.port, 
            kd->_config.pin.mosi.gpio->_config.base.number, 
            kd->_config.pin.mosi.gpio->_config.base.pin,
            0);
        kdgpio_powerDown(kd->_config.pin.mosi.gpio);
    }
    
    if (kd->_config.pin.miso.gpio != NULL) {
        _gpio_afConfig(kd->_config.pin.miso.gpio->_config.base.port, 
            kd->_config.pin.miso.gpio->_config.base.number, 
            kd->_config.pin.miso.gpio->_config.base.pin,
            0);
        kdgpio_powerDown(kd->_config.pin.miso.gpio);
    }
    
    spi_disable(kd->_config.spi.spi);
    
    if (kd->_config.dma.channelTx != 0xFF) {
        dma_interrupt_disable(kd->_config.dma.channelTx, DMA_INT_FTF);
        dma_channel_disable(kd->_config.dma.channelTx);
    }
    
    if (kd->_config.dma.channelRx != 0xFF) {
        dma_channel_disable(kd->_config.dma.channelRx);
    }
    
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    osEventFlagsClear(kd->_va->flag.ptr, KDSPI_FLAG_TRANSMIT_COMPLETE);
#else
    kd->_va->flag._ = 0;
#endif
    
    return 0;
}


void kdspi_select(kdspi_t *kd) {
    _mutexLock(kd);
}


void kdspi_unselect(kdspi_t *kd) {
    _mutexUnlock(kd);
}


int32_t kdspi_transmit(kdspi_t *kd, uint8_t *wbuf, uint8_t *rbuf, uint16_t len, uint32_t timeout) {
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    if (timeout < 0xFFFFFFFF
            && kd->_config.dma.channelTx != 0xFF 
            && kd->_config.dma.channelRx != 0xFF) {
        osEventFlagsClear(kd->_va->flag.ptr, KDSPI_FLAG_TRANSMIT_COMPLETE);

        dma_channel_disable(kd->_config.dma.channelRx);
        dma_transfer_number_config(kd->_config.dma.channelRx, len);
        dma_memory_address_config(kd->_config.dma.channelRx, (uint32_t) rbuf);
        dma_channel_enable(kd->_config.dma.channelRx);
                
        dma_channel_disable(kd->_config.dma.channelTx);
        dma_transfer_number_config(kd->_config.dma.channelTx, len);
        dma_memory_address_config(kd->_config.dma.channelTx, (uint32_t) wbuf);
        dma_channel_enable(kd->_config.dma.channelTx);

        if (timeout == 0) {
            return len;
        }
        
        int32_t flag = osEventFlagsWait(kd->_va->flag.ptr, KDSPI_FLAG_TRANSMIT_COMPLETE, osFlagsWaitAny, timeout);
        if ((flag < 0) || ((flag & KDSPI_FLAG_TRANSMIT_COMPLETE) != KDSPI_FLAG_TRANSMIT_COMPLETE)) {
            return -1;
        } else {
            return len;
        }
    } else {
        for (register uint32_t i = 0; i < len; i++) {
            while (!(SPI_STAT(kd->_config.spi.spi) & SPI_FLAG_TBE));
            SPI_DATA(kd->_config.spi.spi) = (uint32_t) wbuf[i];

            while (!(SPI_STAT(kd->_config.spi.spi) & SPI_FLAG_RBNE));
            rbuf[i] = SPI_DATA(kd->_config.spi.spi);
        }
        
        while ((SPI_STAT(kd->_config.spi.spi) & SPI_FLAG_TRANS));
        
        return len;
    }
#else
    for (register uint32_t i = 0; i < len; i++) {
        while (!(SPI_STAT(kd->_config.spi.spi) & SPI_FLAG_TBE));
        SPI_DATA(kd->_config.spi.spi) = (uint32_t) wbuf[i];

        while (!(SPI_STAT(kd->_config.spi.spi) & SPI_FLAG_RBNE));
        rbuf[i] = SPI_DATA(kd->_config.spi.spi);
    }
    
    while ((SPI_STAT(kd->_config.spi.spi) & SPI_FLAG_TRANS));
    
    return len;
#endif
}


int32_t kdspi_sendData(kdspi_t *kd, uint8_t *data, uint16_t len, uint32_t timeout) {
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    if (timeout < 0xFFFFFFFF
            && kd->_config.dma.channelTx != 0xFF) {
        if (timeout == 0 && !(osEventFlagsGet(kd->_va->flag.ptr) & KDSPI_FLAG_TRANSMIT_COMPLETE)) {
            return -1;
        }
        osEventFlagsClear(kd->_va->flag.ptr, KDSPI_FLAG_TRANSMIT_COMPLETE);

        dma_channel_disable(kd->_config.dma.channelTx);
        dma_transfer_number_config(kd->_config.dma.channelTx, len);
        dma_memory_address_config(kd->_config.dma.channelTx, (uint32_t) data);
        dma_channel_enable(kd->_config.dma.channelTx);
       
        if (timeout == 0) {
            return len;
        }
        
        int32_t flag = osEventFlagsWait(kd->_va->flag.ptr, KDSPI_FLAG_TRANSMIT_COMPLETE, osFlagsWaitAny, timeout);
        if ((flag < 0) || ((flag & KDSPI_FLAG_TRANSMIT_COMPLETE) != KDSPI_FLAG_TRANSMIT_COMPLETE)) {
            return -1;
        } else {
            return len;
        }
    } else {
        for (register uint32_t i = 0; i < len; i++) {
            while (!(SPI_STAT(kd->_config.spi.spi) & SPI_FLAG_TBE));
            SPI_DATA(kd->_config.spi.spi) = (uint32_t) data[i];
            
            while (!(SPI_STAT(kd->_config.spi.spi) & SPI_FLAG_RBNE));
            SPI_DATA(kd->_config.spi.spi);
        }
        
        while ((SPI_STAT(kd->_config.spi.spi) & SPI_FLAG_TRANS));
        
        return len;
    }
#else
    for (register uint32_t i = 0; i < len; i++) {
        while (!(SPI_STAT(kd->_config.spi.spi) & SPI_FLAG_TBE));
        SPI_DATA(kd->_config.spi.spi) = (uint32_t) data[i];
        
        while (!(SPI_STAT(kd->_config.spi.spi) & SPI_FLAG_RBNE));
        SPI_DATA(kd->_config.spi.spi);
    }
    
    while ((SPI_STAT(kd->_config.spi.spi) & SPI_FLAG_TRANS));
    
    return len;
#endif
}


int32_t kdspi_recvData(kdspi_t *kd, uint8_t *data, uint16_t len, uint32_t timeout) {
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    if (timeout < 0xFFFFFFFF
            && kd->_config.dma.channelRx != 0xFF) {
        osEventFlagsClear(kd->_va->flag.ptr, KDSPI_FLAG_TRANSMIT_COMPLETE);

        dma_channel_disable(kd->_config.dma.channelRx);
        dma_transfer_number_config(kd->_config.dma.channelRx, len);
        dma_memory_address_config(kd->_config.dma.channelRx, (uint32_t) data);
        dma_channel_enable(kd->_config.dma.channelRx);
        
        if (kd->_config.dma.channelTx != 0xFF) {
            dma_channel_disable(kd->_config.dma.channelTx);
            dma_transfer_number_config(kd->_config.dma.channelTx, len);
            dma_memory_address_config(kd->_config.dma.channelTx, (uint32_t) data);
            dma_channel_enable(kd->_config.dma.channelTx);
        } else {
            for (register uint32_t i = 0; i < len; i++) {
                while (!(SPI_STAT(kd->_config.spi.spi) & SPI_FLAG_TBE));
                SPI_DATA(kd->_config.spi.spi) = (uint32_t) 0xFF;
            }
        }
       
        if (timeout == 0) {
            return len;
        }
        
        int32_t flag = osEventFlagsWait(kd->_va->flag.ptr, KDSPI_FLAG_TRANSMIT_COMPLETE, osFlagsWaitAny, timeout);
        if ((flag < 0) || ((flag & KDSPI_FLAG_TRANSMIT_COMPLETE) != KDSPI_FLAG_TRANSMIT_COMPLETE)) {
            return -1;
        } else {
            return len;
        }
    } else {
        for (register uint32_t i = 0; i < len; i++) {
            while (!(SPI_STAT(kd->_config.spi.spi) & SPI_FLAG_TBE));
            SPI_DATA(kd->_config.spi.spi) = (uint32_t) 0xFF;
            
            while (!(SPI_STAT(kd->_config.spi.spi) & SPI_FLAG_RBNE));
            data[i] = SPI_DATA(kd->_config.spi.spi);
        }
        
        while ((SPI_STAT(kd->_config.spi.spi) & SPI_FLAG_TRANS));
        
        return 0;
    }
#else
    for (register uint32_t i = 0; i < len; i++) {
        while (!(SPI_STAT(kd->_config.spi.spi) & SPI_FLAG_TBE));
        SPI_DATA(kd->_config.spi.spi) = (uint32_t) 0xFF;
        
        while (!(SPI_STAT(kd->_config.spi.spi) & SPI_FLAG_RBNE));
        data[i] = SPI_DATA(kd->_config.spi.spi);
    }
    
    while ((SPI_STAT(kd->_config.spi.spi) & SPI_FLAG_TRANS));
    
    return 0;
#endif
}


void kdspi_setBaudRate(kdspi_t *kd, uint32_t br) {
    _mutexLock(kd);
    updateFreq(kd, br);
    _mutexUnlock(kd);
}

/*@}*/

