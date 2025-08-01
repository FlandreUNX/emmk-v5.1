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
 * @addtogroup CpuUsage
 * @note none
 */

/*@{*/

#if EMMK_CFG_DEBUG_CPU_USAGE_ENABLE == 1

static klist_t mCpuUsage_list;

static uint32_t mCpuUsage_idleTick = 0;
static uint8_t mCpuUsage_idleUsage = 0;

#endif

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

#if EMMK_CFG_DEBUG_CPU_USAGE_ENABLE == 1
    klist_init(&mCpuUsage_list);
    mCpuUsage_idleTick = 0;
    mCpuUsage_idleUsage = 0;
#endif
    
    return 0;
}


void kdebug_skip(bool set) {
    g_kdebug_skip_log = set;
}

/*@}*/

/**
 * @addtogroup CpuUsage
 * @note none
 */

/*@{*/

#if EMMK_CFG_DEBUG_CPU_USAGE_ENABLE == 1

void kdebug_cpuUsage_calculate1ms(void) {
    static uint16_t tick = 0;
    if (++tick >= 1000) {
        tick = 0;
        static klist_t *pos = NULL;
        static kdebug_CpuUsage_t *usage = NULL;
        static uint32_t totalTick = 0;
        klist_forEach(pos, &mCpuUsage_list) {
            usage = klist_entry(pos, kdebug_CpuUsage_t, list);
            totalTick += usage->counter;
        }
        totalTick += mCpuUsage_idleTick;

        pos = NULL;
        usage = NULL;
        klist_forEach(pos, &mCpuUsage_list) {
            usage = klist_entry(pos, kdebug_CpuUsage_t, list);
            usage->usage =  (usage->counter * 100) / totalTick;
            usage->counter = 0;
        }

        mCpuUsage_idleUsage =  (mCpuUsage_idleTick * 100) / totalTick;
        mCpuUsage_idleTick = 0;

        totalTick = 0;
    }
}


void kdebug_cpuUsage_idleCount(void) {
    mCpuUsage_idleTick++;
}


uint8_t kdebug_cpuUsage_getIdleUsage(void) {
    return mCpuUsage_idleUsage;
}


void kdebug_cpuUsage_counter(kdebug_CpuUsage_t *usage) {
    ASSERT(usage != NULL);
    usage->counter++;
}


void kdebug_cpuUsage_register(kdebug_CpuUsage_t *usage) {
    ASSERT(usage != NULL);
    usage->usage = 0;
    usage->counter = 0;
    klist_init(&usage->list);
    klist_addTail(&mCpuUsage_list, &usage->list);
}

#endif

/*@}*/


#endif
