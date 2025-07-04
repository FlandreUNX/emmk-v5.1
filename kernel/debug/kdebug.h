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

#ifndef _KDG_LOG_H__
#define _KDG_LOG_H__

#include <stdint.h>

#include "emmk-config.h"

#if EMMK_CFG_DEBUG_LOG_TARGET == 0 || EMMK_CFG_DEBUG_LOG_TARGET == 2

#include "./SEGGER_RTT.h"

/**
 * @addtogroup Debug config
 * @note none
 */
 
/*@{*/

#if EMMK_CFG_DEBUG_ENABLE == 1
extern int32_t kdebug_init(void);
#else
#define kdebug_init()
#endif
extern void kdebug_skip(bool set);

extern void __kdebug_lock(void);
extern void __kdebug_unlock(void);

extern void _klPtf_putchar(char c);
#if EMMK_CFG_DEBUG_OVER_EVR == 1 && EMMK_CFG_DEBUG_OVER_RTT == 1
#define PRINTF(fmt, ...) \
    printf(fmt, ##__VA_ARGS__); \
    klPtf_printf(fmt, ##__VA_ARGS__) 
#elif EMMK_CFG_DEBUG_OVER_EVR == 1
#define PRINTF(fmt, ...) \
    printf(fmt, ##__VA_ARGS__)
#elif EMMK_CFG_DEBUG_OVER_RTT == 1
#define PRINTF(fmt, ...) \
    klPtf_printf(fmt, ##__VA_ARGS__) 
#endif


// DEBUG level
#define DBG_ERROR           0
#define DBG_WARNING         1
#define DBG_INFO            2
#define DBG_LOG             3

#ifndef DBG_SECTION_NAME
#define DBG_SECTION_NAME    "SYS"
#endif

#ifndef DBG_LEVEL
#define DBG_LEVEL         DBG_LOG
#endif

/*@}*/

/**
 * @addtogroup Debug color
 * @note none
 */
 
/*@{*/

//// 
//// The color for terminal (foreground)
//// BLACK    30
//// RED      31
//// GREEN    32
//// YELLOW   33
//// BLUE     34
//// PURPLE   35
//// CYAN     36
//// WHITE    37
//// 
#if EMMK_CFG_DEBUG_COLOR == 1
#define _DBG_COLOR(n) \
    PRINTF("\033["#n"m")
    
#define _DBG_LOG_HDR(lvl_name, color_n) \
    PRINTF("\033["#color_n"m["lvl_name"/"DBG_SECTION_NAME"] ""\033[0m")
    
#define _DBG_LOG_X_END \
    PRINTF("\r\n")
#else
#define _DBG_COLOR(n)

#if EMMK_CFG_DEBUG_TAG == 0
#define _DBG_LOG_HDR(lvl_name, color_n)
#else
#define _DBG_LOG_HDR(lvl_name, color_n) \
    PRINTF("["lvl_name"/"DBG_SECTION_NAME"] ")
#endif
    
#define _DBG_LOG_X_END \
    PRINTF("\r\n")
#endif

/*@}*/

/**
 * @addtogroup Debug tick
 * @note none
 */
 
/*@{*/

#if EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_RTX5

#if EMMK_RTOS_TYPE == EMMK_RTOS_IS_RTX5

#if EMMK_CFG_DEBUG_TICK == 1
#define _DBG_LOG_TICK_HDR() \
    PRINTF("[%10d]", osKernelGetTickCount())
#else
#define _DBG_LOG_TICK_HDR()
#endif

#elif EMMK_FULL_RTOS_SUPPORT == EMMK_FULL_RTOS_FREERTOS

#if EMMK_CFG_DEBUG_TICK == 1
#define _DBG_LOG_TICK_HDR() \
    PRINTF("[%10d]", xTaskGetTickCount())
#else
#define _DBG_LOG_TICK_HDR()
#endif
#else
#endif

#else
#if EMMK_CFG_DEBUG_TICK == 1
#define _DBG_LOG_TICK_HDR() \
    PRINTF("[%10d] ", qClock_GetTick())
#else
#define _DBG_LOG_TICK_HDR()
#endif
#endif

/*@}*/

/**
 * @addtogroup Debug static func
 * @note none
 */
 
/*@{*/

#if EMMK_CFG_DEBUG_ENABLE == 1
    
//// 
//// static debug routine
//// NOTE: This is a NOT RECOMMENDED API. Please using LOG_X API.
////       It will be DISCARDED later. Because it will take up more resources.
//// 
#define dbg_log(level, fmt, ...) \
    if ((level) <= DBG_LEVEL) { \
        _DBG_LOG_TICK_HDR(); \
        switch(level) { \
            case DBG_ERROR:   _DBG_LOG_HDR("E", 31); break; \
            case DBG_WARNING: _DBG_LOG_HDR("W", 33); break; \
            case DBG_INFO:    _DBG_LOG_HDR("I", 32); break; \
            case DBG_LOG:     _DBG_LOG_HDR("D", 0); break; \
            default: break; \
        } \
        PRINTF(fmt, ##__VA_ARGS__); \
        _DBG_COLOR(0); \
    }

#define dbg_here \
    if ((DBG_LEVEL) <= DBG_LOG) { \
        _DBG_LOG_TICK_HDR(); \
        PRINTF(DBG_SECTION_NAME " Here %s:%d\n", \
            __FUNCTION__, __LINE__); \
    }

#define dbg_enter \
    if ((DBG_LEVEL) <= DBG_LOG) { \
        _DBG_LOG_TICK_HDR(); \
        _DBG_COLOR(32); \
        PRINTF(DBG_SECTION_NAME " Enter %s\n", \
            __FUNCTION__); \
        _DBG_COLOR(0); \
    }

#define dbg_exit \
    if ((DBG_LEVEL) <= DBG_LOG) { \
        _DBG_LOG_TICK_HDR(); \
        _DBG_COLOR(32); \
        PRINTF(DBG_SECTION_NAME " Exit  %s:%d\n", \
            __FUNCTION__); \
        _DBG_COLOR(0); \
    }
    

#define dbg_log_line(lvl, color_n, fmt, ...) \
    do { \
        extern volatile uint8_t g_kdebug_skip_log; \
        if (g_kdebug_skip_log) { \
            break; \
        } \
        _DBG_LOG_TICK_HDR(); \
        _DBG_LOG_HDR(lvl, color_n); \
        PRINTF(fmt, ##__VA_ARGS__); \
        _DBG_LOG_X_END; \
    } while (0)
    
#define dbg_raw(...) \
    PRINTF(__VA_ARGS__);

#else
#define dbg_log(level, fmt, ...)
#define dbg_here
#define dbg_enter
#define dbg_exit
#define dbg_log_line(lvl, color_n, fmt, ...)
#define dbg_raw(...)
#endif /* DBG_ENABLE */

/*@}*/

/**
 * @addtogroup Debug log func
 * @note none
 */
 
/*@{*/

#if (DBG_LEVEL >= DBG_LOG)
#define LOG_D(fmt, ...)      dbg_log_line("D", 0, fmt, ##__VA_ARGS__)
#define KLOG_D(fmt, ...)     dbg_log_line("Kernel-D", 0, fmt, ##__VA_ARGS__)
#else
#define LOG_D(...)
#define KLOG_D(...)
#endif

#if (DBG_LEVEL >= DBG_INFO)
#define LOG_I(fmt, ...)      dbg_log_line("I", 32, fmt, ##__VA_ARGS__)
#define KLOG_I(fmt, ...)     dbg_log_line("Kernel-I", 32, fmt, ##__VA_ARGS__)
#else
#define LOG_I(...)
#define KLOG_I(...)
#endif

#if (DBG_LEVEL >= DBG_WARNING)
#define LOG_W(fmt, ...)      dbg_log_line("W", 33, fmt, ##__VA_ARGS__)
#define KLOG_W(fmt, ...)     dbg_log_line("Kernel-W", 33, fmt, ##__VA_ARGS__)
#else
#define LOG_W(...)
#define KLOG_W(...)
#endif

#if (DBG_LEVEL >= DBG_ERROR)
#define LOG_E(fmt, ...)      dbg_log_line("E", 31, fmt, ##__VA_ARGS__)
#define KLOG_E(fmt, ...)     dbg_log_line("Kernel-E", 31, fmt, ##__VA_ARGS__)
#else
#define LOG_E(...)
#define KLOG_E(...)
#endif

#define LOG_RAW(...)         dbg_raw(__VA_ARGS__)

/*@}*/

#elif EMMK_CFG_DEBUG_LOG_TARGET == 1

#include "esp_log.h"
#define LOG_RAW(fmt, ...)     ESP_LOGD(TAG, fmt, ##__VA_ARGS__)
#define LOG_D(fmt, ...)       ESP_LOGD(TAG, fmt, ##__VA_ARGS__)
#define LOG_I(fmt, ...)       ESP_LOGI(TAG, fmt, ##__VA_ARGS__)
#define LOG_W(fmt, ...)       ESP_LOGW(TAG, fmt, ##__VA_ARGS__)
#define LOG_E(fmt, ...)       ESP_LOGE(TAG, fmt, ##__VA_ARGS__)

#endif

#endif
