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
 * @addtogroup Private func
 * @note none
 */

/*@{*/


static int32_t sectorErase(kdimtd_t *kd, uint32_t addr, uint32_t size) {
    EEPROM_ERASE(addr, size);
    return 0;
}


static int32_t pageProgram(kdimtd_t *kd, uint32_t addr, uint8_t *data, uint16_t size) {
    EEPROM_WRITE(addr, data, size);
    return 0;
}


static int32_t pageRead(kdimtd_t *kd, uint32_t addr, uint8_t *data, uint16_t size) {
    EEPROM_READ(addr, data, size);
    return 0;
}

/*@}*/

/**
 * @addtogroup Mutex support
 * @note none
 */

/*@{*/

static inline void mutexNew(kdimtd_t *kd) {
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    const osMutexAttr_t mutex_attr = {
        .name = kd->_path,
        .attr_bits = osMutexRobust
    };

    kd->_va->mutex = osMutexNew(&mutex_attr);
    ASSERT(kd->_va->mutex != NULL);
#endif
}


static inline void mutexDelete(kdimtd_t *kd) {
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    osMutexDelete(kd->_va->mutex);
    kd->_va->mutex = NULL;
#endif
}


static inline void mutexLock(kdimtd_t *kd) {
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    osMutexAcquire(kd->_va->mutex, osWaitForever);
#endif
}


static inline void mutexUnlock(kdimtd_t *kd) {
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    osMutexRelease(kd->_va->mutex);
#endif
}


static inline void threadYield(void) {
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    osThreadYield();
#endif
}

/*@}*/

/**
 * @addtogroup Public functions
 * @note none
 */

/*@{*/

int32_t kdimtd_init(kdimtd_t *kd) {
    if (emmkDriver_initRefsCountUp(&kd->_va->initRefs) != 0) {
        return -1;
    }

    mutexNew(kd);

    return 0;
}


int32_t kdimtd_finalize(kdimtd_t *kd) {
    if (emmkDriver_initRefsCountDown(&kd->_va->initRefs) != 0) {
        return -1;
    }

    if (kd->_va->initRefsPower != 0) {
        kd->_va->initRefsPower = 1;
        kdimtd_powerDown(kd);
    }

    mutexDelete(kd);

    return 0;
}


int32_t kdimtd_powerUp(kdimtd_t *kd) {
    if (emmkDriver_initRefsCountUp(&kd->_va->initRefsPower) != 0) {
        return -1;
    }

    return 0;
}


int32_t kdimtd_powerDown(kdimtd_t *kd) {
    if (emmkDriver_initRefsCountDown(&kd->_va->initRefsPower) != 0) {
        return -1;
    }

    return 0;
}


int32_t kdimtd_reads(kdimtd_t *kd, uint32_t address, uint8_t *data, uint32_t readSize, uint32_t timeout) {
    ASSERT(kd != NULL);

    pageRead(kd, (address + kd->_instance.addressStart), data, readSize);

    return 0;
}


int32_t kdimtd_earse(kdimtd_t *kd, uint32_t sectorAddress, uint32_t size, uint32_t timeout) {
    mutexLock(kd);

    sectorErase(kd, sectorAddress + kd->_instance.addressStart, size);

    mutexUnlock(kd);
    return 0;
}


int32_t kdimtd_writes(kdimtd_t *kd, uint32_t address, uint8_t *data, uint32_t size, uint32_t timeout) {
    uint32_t pageOffset, pageRemain;
    int32_t rc = -1;

    mutexLock(kd);

    address += kd->_instance.addressStart;

    for (uint32_t i = 0; i < size;) {
        uint32_t written;
        uint32_t addr = address + i;

        if (hweight32(kd->_instance.pageSize) == 1) {
            pageOffset = addr & (kd->_instance.pageSize - 1);
        } else {
            uint64_t aux = addr;
            pageOffset = do_div(&aux, kd->_instance.pageSize);
        }

        pageRemain = min_t(uint32_t, kd->_instance.pageSize - pageOffset, size - i);

        if (pageProgram(kd, addr, data + i, pageRemain) != 0) {
            rc = -1;
            goto l_exit;
        }

        written = pageRemain;
        i += written;
    }

    rc = 0;

    l_exit:
    mutexUnlock(kd);
    return rc;
}


uint32_t kdimtd_getAbsAddr(kdimtd_t *kd, uint32_t offset) {
    return kd->_instance.addressStart + offset;
}

/*@}*/
