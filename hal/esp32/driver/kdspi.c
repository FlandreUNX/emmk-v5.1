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

static const char *TAG = "kdspi";

/*@}*/

/**
 * @addtogroup Static hal func
 * @note none
 */
 
/*@{*/

static void mutexNew(kdspi_t *kd) {
    kd->_va->mutex = xSemaphoreCreateMutex();
    ASSERT(kd->_va->mutex != NULL);
}


static void mutexDelete(kdspi_t *kd) {
    vSemaphoreDelete(kd->_va->mutex);
    kd->_va->mutex = NULL;
}


static void mutexLock(kdspi_t *kd) {
    xSemaphoreTake(kd->_va->mutex, portMAX_DELAY);
}


static void mutexUnlock(kdspi_t *kd) {
    xSemaphoreGive(kd->_va->mutex);
}


static void updateFreq(kdspi_t *kd, uint32_t speed) {
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
    
    ESP_ERROR_CHECK(spi_bus_initialize(kd->_config.host, kd->_config.busConfig, SPI_DMA_CH_AUTO));
    
    mutexNew(kd);

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
    
    ESP_ERROR_CHECK(spi_bus_free(kd->_config.host));

    mutexDelete(kd);

    return 0;
}

int32_t kdspi_powerUp(kdspi_t *kd) {
    ASSERT(kd != NULL);
    
    if (emmkDriver_initRefsCountUp(&kd->_va->initRefsPower) != 0) {
        return -1;
    }
    
    ESP_ERROR_CHECK(spi_bus_add_device(kd->_config.host, kd->_config.ifConfig, &kd->_va->handle));
    
    kdspi_unselect(kd);

    return 0;
}

int32_t kdspi_powerDown(kdspi_t *kd) {
    ASSERT(kd != NULL);
    
    if (emmkDriver_initRefsCountDown(&kd->_va->initRefsPower) != 0) {
        return -1;
    }
    
    kdspi_unselect(kd);
    
    ESP_ERROR_CHECK(spi_bus_remove_device(kd->_va->handle));

    return 0;
}

void kdspi_select(kdspi_t *kd) {
    mutexLock(kd);
}

void kdspi_unselect(kdspi_t *kd) {
    mutexUnlock(kd);
}

int32_t kdspi_transmit(kdspi_t *kd, uint8_t *wbuf, uint8_t *rbuf, uint16_t len, uint32_t timeout) {
    mutexLock(kd);
    
    spi_transaction_t t = {0};
    t.length = len * 8;
    t.tx_buffer = wbuf;
    t.rx_buffer = rbuf;
    ESP_ERROR_CHECK(spi_device_polling_transmit(kd->_va->handle, &t));
    
    mutexUnlock(kd);
    
    return len;
}

int32_t kdspi_sendData(kdspi_t *kd, uint8_t *data, uint16_t len, uint32_t timeout) {
    mutexLock(kd);
    
    spi_transaction_t t = {0};
    t.length = len * 8;
    t.tx_buffer = data;
    if (timeout == UINT32_MAX) {
        ESP_ERROR_CHECK(spi_device_polling_transmit(kd->_va->handle, &t));
    }
    
    mutexUnlock(kd);
    
    return len;
}

int32_t kdspi_recvData(kdspi_t *kd, uint8_t *data, uint16_t len, uint32_t timeout) {
    mutexLock(kd);
    
    spi_transaction_t t = {0};
    t.length = len * 8;
    t.rx_buffer = data;
    t.tx_buffer = data;
    ESP_ERROR_CHECK(spi_device_polling_transmit(kd->_va->handle, &t));
    
    mutexUnlock(kd);
    
    return 0;
}

void kdspi_setBaudRate(kdspi_t *kd, uint32_t br) {
    mutexLock(kd);
    updateFreq(kd, br);
    mutexUnlock(kd);
}

/*@}*/
