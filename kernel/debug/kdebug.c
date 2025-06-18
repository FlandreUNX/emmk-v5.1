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

/**
 * @addtogroup Locate variable
 * @note none
 */
 
/*@{*/

#if EMMK_CFG_DEBUG_ENABLE == 1

#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5
static osMutexId_t mLogMutex;
#elif EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_FREERTOS

#endif

volatile uint8_t g_kdebug_skip_log = false;

/*@}*/

/**
 * @addtogroup kdbg inline func
 * @note none
 */
 
/*@{*/

inline void __kdebug_lock(void) {
#if EMMK_CFG_DEBUG_ENABLE == 1
#ifdef CMSIS_OS2_H_
    osMutexAcquire(mLogMutex, (uint32_t) -1);
#endif
#endif
}


inline void __kdebug_unlock(void) {
#if EMMK_CFG_DEBUG_ENABLE == 1
#ifdef CMSIS_OS2_H_
    osMutexRelease(mLogMutex);
#endif
#endif
}


#if EMMK_CFG_DEBUG_OVER_RTT == 1
void ptf_putchar(char c) {
    SEGGER_RTT_PutChar(0, c);
}
#endif

/*@}*/


/**
 * @addtogroup Public func
 * @note none
 */
 
/*@{*/

int32_t kdebug_init(void) {
#if EMMK_CFG_DEBUG_ENABLE == 1
#ifdef CMSIS_OS2_H_
    const osMutexAttr_t mutexAttr = {
        .attr_bits = osMutexRobust,
    };
    mLogMutex = osMutexNew(&mutexAttr);
    ASSERT(mLogMutex != NULL);
#endif
#endif
    
#if EMMK_CFG_DEBUG_OVER_RTT == 1
    SEGGER_RTT_Init();
#endif
    
    g_kdebug_skip_log = false;
    
    return 0;
}


void kdebug_skip(bool set) {
    g_kdebug_skip_log = set;
}

/*@}*/

#endif
