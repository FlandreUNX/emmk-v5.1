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
 * @addtogroup Config
 * @note none
 */
 
/*@{*/

#define CONFIG_USING_CLONE_ENCRYPT     0
 
/*@}*/

/**
 * @addtogroup Private vairables
 * @note none
 */
 
/*@{*/

#define USEC_TO_COUNT(us, clockFreqInHz)   (uint64_t) ((uint64_t) us * clockFreqInHz / 1000000U)

#if __CORTEX_M > 0

#define __DWT_CR         *(__IO uint32_t *) 0xE0001000
#define __DWT_CYCCNT     *(__IO uint32_t *) 0xE0001004
#define __DEM_CR         *(__IO uint32_t *) 0xE000EDFC

#define __DEM_CR_TRCENA       (1u << 24u)
#define __DWT_CR_CYCCNTENA    (1u << 0u)

#endif

/*@}*/

/**
 * @addtogroup Private vairables
 * @note none
 */
 
/*@{*/

#if CONFIG_USING_CLONE_ENCRYPT == 1
__USED static volatile const __attribute__((section(".ARM.0x08000300"))) uint32_t __CLONE_ENCRYPT[1] = {0xFFFFFFFF};
#endif

/*@}*/

/**
 * @addtogroup vfs define
 * @note none
 */
 
/*@{*/

/*@}*/

/**
 * @addtogroup Instance support functions
 * @note none
 */
 
/*@{*/

/*@}*/

/**
 * @addtogroup Private encrypt func
 * @note none
 */
 
/*@{*/

#if CONFIG_USING_CLONE_ENCRYPT == 1

static void _encryptWrite(uint32_t uid) {

}


static void _encryptCheck(void) {
    uint32_t t[3];
    uint32_t uid32;

    kdmisc_getCPUID((uint8_t *) t);
    
    uid32 = t[0] + t[1] + t[2];

    if (uid32 == 0) {
        ASSERT(0);
    }
    
    uint32_t addr = (uint32_t) __CLONE_ENCRYPT;
    if (*(__IO uint32_t *) addr == 0xFFFFFFFF) {
        _encryptWrite(uid32);
    }
    
    if (((__IO uint32_t *) addr)[0] != uid32) {
        ASSERT(0);
    }
}

#endif

/*@}*/

/**
 * @addtogroup Private delay func
 * @note none
 */
 
/*@{*/

#if __CORTEX_M > 0

void dwt_delayUs(uint32_t v) {
}

#endif

/*@}*/

/**
 * @addtogroup IdFunc
 * @note none
 */
 
/*@{*/

void kdmisc_getCPUID(uint8_t out[12]) {
    memcpy(out, (void *) (0x180000F0), 12);
}

/*@}*/

/**
 * @addtogroup DelayFunc
 * @note none
 */
 
/*@{*/

void kdmisc_delayInit(void) {
#if __CORTEX_M > 0
    if (__DWT_CR & __DWT_CR_CYCCNTENA) {
        return;
    }
    
    __DWT_CR |= (uint32_t) __DEM_CR_TRCENA;
    __DWT_CYCCNT = (uint32_t) 0u;
    __DWT_CR |= (uint32_t) __DWT_CR_CYCCNTENA;
#endif
}


void kdmisc_delayFinalize(void) {
#if __CORTEX_M > 0
    __DWT_CR &= ~((uint32_t) __DWT_CR_CYCCNTENA);
    __DWT_CYCCNT = (uint32_t) 0u;
    __DWT_CR &= ~((uint32_t) __DEM_CR_TRCENA);
#endif
}


__USED void kdmisc_delayUs(uint64_t v) {
#if __CORTEX_M > 0
    volatile uint32_t tickStart = __DWT_CYCCNT;
    volatile uint32_t tickStop = (uint32_t) ((SystemCoreClock / 1000000u) * v) + tickStart;
    
    if (tickStop >= tickStart) {
        while ((__DWT_CYCCNT > tickStart) && (__DWT_CYCCNT < tickStop));
    } else {
        while (!((__DWT_CYCCNT > tickStop) && (__DWT_CYCCNT < tickStart)));
    }
#else
    uint64_t d = v * (SystemCoreClock / 1000000) / 4;
    do {
        __NOP();
    }
    while (v --);
#endif
}


void inline kdmisc_delayMs(uint32_t v) {
    kdmisc_delayUs(v * 1000u);
}

/*@}*/

/**
 * @addtogroup CloneFunc
 * @note none
 */
 
/*@{*/

void kdmisc_cloneEncrypt(void) {
#if CONFIG_USING_CLONE_ENCRYPT == 1
    _encryptCheck();
#endif
}

/*@}*/

/**
 * @addtogroup Irq Handler
 * @note none
 */
 
/*@{*/

#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_QTS
void SysTick_Handler(void) {
    qClock_SysTick();
}
#endif

/*@}*/

/**
 * @addtogroup systick configure
 * @note none
 */
 
/*@{*/

void kdmisc_systickDisable(void) {
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}


void kdmisc_systickEnable(void) {
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
}


int32_t kdmisc_systickInit(uint32_t freq) {
    uint32_t load;
    
    if (freq == 0U) {
        return -1;
    }

    load = (SystemCoreClock / freq) - 1U;
    if (load > 0x00FFFFFFU) {
        return -1;
    }
    
    NVIC_SetPriority(SysTick_IRQn, 8);
    NVIC_EnableIRQ(SysTick_IRQn);

    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk;
    SysTick->LOAD = load;
    SysTick->VAL = 0U;
    
    return 0;
}

/*@}*/
