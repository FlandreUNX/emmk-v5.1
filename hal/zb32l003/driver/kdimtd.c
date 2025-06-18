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
 * @addtogroup Define
 * @note none
 */
 
/*@{*/

#define FLASH_REGUNLOCK_KEY1    0x5A5A
#define FLASH_REGUNLOCK_KEY2    0xA5A5

#define FLASH_REGISTER_UNLOCK \
    FLASH->BYPASS = FLASH_REGUNLOCK_KEY1; \
    FLASH->BYPASS = FLASH_REGUNLOCK_KEY2;

#define FLASH_REGISTER_LOCK  \
    FLASH->BYPASS = FLASH_REGUNLOCK_KEY1; \
    FLASH->BYPASS = 0x00000000;

/*@}*/

/**
 * @addtogroup Private func
 * @note none
 */
 
/*@{*/

static int32_t sectorErase(kdimtd_t *kd, uint32_t addr) {
    while (FLASH->CR & (0x01 << 2)) {};
	 
	__disable_irq();
//    __DSB();
//    __ISB();
    
//    if (addr >= 0x8000) {
        FLASH_REGISTER_UNLOCK;
        FLASH->SLOCK1 = 0xFFFFFFFF;
//    } else {
//        FLASH_REGISTER_UNLOCK;
//        FLASH->SLOCK0 = 0xFFFFFFFF;
//    }
    while (FLASH->CR & (0x01 << 2));

	FLASH_REGISTER_UNLOCK;
    FLASH->CR &= ~0x03;
    
    FLASH_REGISTER_UNLOCK;
    FLASH->CR |= 0x02;
    
	*(__IO uint32_t *) (addr) = 0;	
	while (FLASH->CR & (0x01 << 2));
    
	FLASH_REGISTER_UNLOCK;
	FLASH->CR &= ~0x03;
    
//    if (addr >= 0x8000) {
        FLASH_REGISTER_UNLOCK;
        FLASH->SLOCK1 = 0;
//    } else {
//        FLASH_REGISTER_UNLOCK;
//        FLASH->SLOCK0 = 0;
//    }

//    __DSB();
//    __ISB();
	__enable_irq();
    
    return 0;
}


static int32_t pageProgram(kdimtd_t *kd, uint32_t addr, uint8_t *data, uint16_t size) {
    while (FLASH->CR & (0x01 << 2)) {};
    
    __disable_irq();
//    __DSB();
//    __ISB();
	
//	if (addr >= 0x8000) {
        FLASH_REGISTER_UNLOCK;
        FLASH->SLOCK1 = 0xFFFFFFFF;
//    } else {
//        FLASH_REGISTER_UNLOCK;
//        FLASH->SLOCK0 = 0xFFFFFFFF;
//    }
    while (FLASH->CR & (0x01 << 2));

	for (uint16_t i = 0; i < size; i++) {
        FLASH_REGISTER_UNLOCK;
        FLASH->CR &= ~0x03;

        FLASH_REGISTER_UNLOCK;
        FLASH->CR |= 0x01;
        
        *(volatile uint8_t *) (addr + i) = data[i];
		while (FLASH->CR & (0x01 << 2)){}
        __DSB();
        __ISB();
	}
    
    FLASH_REGISTER_UNLOCK;
    FLASH->CR &= ~0x03;
    
//    if (addr >= 0x8000) {
        FLASH_REGISTER_UNLOCK;
        FLASH->SLOCK1 = 0;
//    } else {
//        FLASH_REGISTER_UNLOCK;
//        FLASH->SLOCK0 = 0;
//    }
	
//    __DSB();
//    __ISB();
	__enable_irq();
    
    return 0;
}

/*@}*/

/**
 * @addtogroup Mutex support
 * @note none
 */
 
/*@{*/

__STATIC_FORCEINLINE void mutexNew(kdimtd_t *kd) {
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    const osMutexAttr_t mutex_attr = {
        .name = kd->_path,
        .attr_bits = osMutexRobust
    };
    
    kd->_va->mutex = osMutexNew(&mutex_attr);
    ASSERT(kd->_va->mutex != NULL);
#endif
}


__STATIC_FORCEINLINE void mutexDelete(kdimtd_t *kd) {
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    osMutexDelete(kd->_va->mutex);
    kd->_va->mutex = NULL;
#endif
}


__STATIC_FORCEINLINE void mutexLock(kdimtd_t *kd) {
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    osMutexAcquire(kd->_va->mutex, osWaitForever);
#endif
}


__STATIC_FORCEINLINE void mutexUnlock(kdimtd_t *kd) {
#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
    osMutexRelease(kd->_va->mutex);
#endif
}


__STATIC_FORCEINLINE void threadYield(void) {
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
//    if (emmkDriver_initRefsCountUp(&kd->_va->initRefs) != 0) {
//        return -1;
//    }
//
//    mutexNew(kd);
    
    return 0;
}


int32_t kdimtd_finalize(kdimtd_t *kd) {
//    if (emmkDriver_initRefsCountDown(&kd->_va->initRefs) != 0) {
//        return -1;
//    }
//
//    if (kd->_va->initRefsPower != 0) {
//        kd->_va->initRefsPower = 1;
//        kdimtd_powerDown(kd);
//    }
//
//    mutexDelete(kd);
    
    return 0;
}


int32_t kdimtd_powerUp(kdimtd_t *kd) {
//    if (emmkDriver_initRefsCountUp(&kd->_va->initRefsPower) != 0) {
//        return -1;
//    }
    
    return 0;
}


int32_t kdimtd_powerDown(kdimtd_t *kd) {
//    if (emmkDriver_initRefsCountDown(&kd->_va->initRefsPower) != 0) {
//        return -1;
//    }
    
    return 0;
}


int32_t kdimtd_reads(kdimtd_t *kd, uint32_t address, uint8_t *data, uint32_t readSize, uint32_t timeout) {
    memcpy(data, (uint32_t *) (address + kd->_instance.addressStart), readSize);
    
    return 0;
}


int32_t kdimtd_earse(kdimtd_t *kd, uint32_t sectorAddress, uint32_t size, uint32_t timeout) {
    mutexLock(kd);

    sectorErase(kd, sectorAddress + kd->_instance.addressStart);

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

