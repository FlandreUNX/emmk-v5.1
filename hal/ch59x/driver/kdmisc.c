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
 * @addtogroup IdFunc
 * @note none
 */
 
/*@{*/

void kdmisc_getCPUID(uint8_t out[12]) {
    memcpy(out, (uint8_t *) 0x001007B0, 12);
}

/*@}*/

/**
 * @addtogroup DelayFunc
 * @note none
 */

/*@{*/

void kdmisc_delayInit(void) {

}


void kdmisc_delayFinalize(void) {

}


void kdmisc_delayUs(uint64_t v) {

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
}

/*@}*/

/**
 * @addtogroup Irq Handler
 * @note none
 */
 
/*@{*/

#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_QTS
__INTERRUPT void SysTick_Handler(void) {
    qClock_SysTick();
    SysTick->SR &= ~SysTick_SR_CNTIF;
}
#endif

/*@}*/

/**
 * @addtogroup systick configure
 * @note none
 */
 
/*@{*/

void kdmisc_systickDisable(void) {
    SysTick->CTLR &= ~SysTick_CTLR_STE;
    SysTick->SR &= ~SysTick_SR_CNTIF;
}


void kdmisc_systickEnable(void) {
    SysTick->SR &= ~SysTick_SR_CNTIF;
    SysTick->CTLR |= SysTick_CTLR_STE;
}


int32_t kdmisc_systickInit(uint32_t freq) {
    SysTick_Config(GetSysClock() / freq);
    PFIC_SetPriority(SysTick_IRQn, 0x14);
    return 0;
}

/*@}*/
