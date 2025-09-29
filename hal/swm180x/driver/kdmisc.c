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

#if CONFIG_USING_CLONE_ENCRYPT == 1
__USED static volatile const __attribute__((section(".ARM.__at_0x00000300"))) uint32_t __CLONE_ENCRYPT[1] = {0xFFFFFFFF};
#endif

/*@}*/

/**
 * @addtogroup Private encrypt func
 * @note none
 */
 
/*@{*/

#if CONFIG_USING_CLONE_ENCRYPT == 1

__USED static void _encryptWrite(uint32_t uid) {

}


static uint32_t _encryptRead(void) {

}


__USED static void _encryptCheck(void) {
    uint32_t t[3];
    uint32_t uid32;

    kdmisc_getCPUID((uint8_t *) t);
    
    uid32 = t[0] + t[1] + t[2];

    if (uid32 == 0) {
        for (;;);
    }
    
    uint32_t addr = (uint32_t) __CLONE_ENCRYPT;
    if (*(__IO uint32_t *) addr == 0xFFFFFFFF) {
        _encryptWrite(uid32);
    }
    
    if (((__IO uint32_t *) addr)[0] != uid32) {
        for (;;);
    }
}

#endif

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


void kdmisc_delayUs(uint32_t v) {
#if __CORTEX_M > 0
    volatile uint32_t tickStart = __DWT_CYCCNT;
    volatile uint32_t tickStop = (uint32_t) ((SystemCoreClock / 1000000u) * v) + tickStart;
    
    if (tickStop >= tickStart) {
        while ((__DWT_CYCCNT > tickStart) && (__DWT_CYCCNT < tickStop));
    } else {
        while (!((__DWT_CYCCNT > tickStop) && (__DWT_CYCCNT < tickStart)));
    }
#else
    v = v * SystemCoreClock / 1000000u;
    while (v--) {
        asm volatile ("");
    }
#endif
}


void inline kdmisc_delayMs(uint32_t v) {
    kdmisc_delayUs(v * 1000u);
}

/*@}*/

/**
 * @addtogroup Public func
 * @note none
 */
 
/*@{*/

void kdmisc_getCPUID(uint8_t out[12]) {
    memcpy((void *) (0x180000F0), out, 12);
}

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

void IRQ0_Handler(void) {
    extern void kdsdadc_DMAIrqHandler(void);
    kdsdadc_DMAIrqHandler();
}

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

    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk;
    SysTick->LOAD = load;
    SysTick->VAL = 0U;
    
    return 0;
}

/*@}*/
