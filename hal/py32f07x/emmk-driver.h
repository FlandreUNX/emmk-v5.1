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
 
#ifndef _EOS_KDRIVER_H_
#define _EOS_KDRIVER_H_

/**
 * @addtogroup Verison
 * @note none
 */
 
/*@{*/

#define DEVICE_VERSION            "PY32F04X"
#define DEVICE_MAJOR_VERSION      "0"
#define DEVICE_SECONDARY_VERSION  "1"

#define EOS_DRIVER_VERSION    "EMMK-"DEVICE_VERSION"-"DEVICE_MAJOR_VERSION"."DEVICE_SECONDARY_VERSION

/*@}*/

/**
 * @addtogroup Macros
 * @note none
 */
 
/*@{*/

#define REG32       KLREG32
#define REG16       KLREG16
#define REG8        KLREG8
#define BIT         KLBIT
#define BITS        KLBITS
#define GET_BITS    KLGET_BITS

// typedef enum {DISABLE = 0, ENABLE = !DISABLE} _EventStatus, ControlStatus;
// typedef enum {RESET = 0, SET = !RESET} FlagStatus;
// typedef enum {ERROR = 0, SUCCESS = !ERROR} ErrStatus;

/*@}*/

/**
 * @addtogroup Complier
 * @note none
 */
 
/*@{*/

#define USEC_TO_COUNT(us, clockFreqInHz)   (uint64_t) ((uint64_t) us * clockFreqInHz / 1000000U)
#define MSEC_TO_COUNT(ms, clockFreqInHz) (uint64_t)((uint64_t)(ms) * (clockFreqInHz) / 1000U)

#define SIZEALIGN(var, alignbytes) \
    ((unsigned int) ((var) + ((alignbytes) - 1)) & (unsigned int) (~(unsigned int) ((alignbytes) - 1)))
        
#define AT_NONCACHEABLE_SECTION(var) __attribute__((section("noncacheable"), zero_init)) var
#define AT_NONCACHEABLE_SECTION_ALIGN(var, alignbytes) \
    __attribute__((section("noncacheable"))) __attribute__((aligned(alignbytes))) var
#define AT_NONCACHEABLE_SECTION_INIT(var) __attribute__((section("noncacheable.init"))) var
#define AT_NONCACHEABLE_SECTION_ALIGN_INIT(var, alignbytes) \
    __attribute__((section("noncacheable.init"))) __attribute__((aligned(alignbytes))) var

/*@}*/
    
/**
 * @addtogroup Heap
 * @note none
 */
 
/*@{*/

extern int Image$$RW_IRAM1$$ZI$$Limit;
#define EMMK_KMEM_START_ADDRESS  ((void *) &Image$$RW_IRAM1$$ZI$$Limit)
#define EMMK_KMEM_END_ADDRESS    ((void *) (0x20000000 + 0x2000))

/*@}*/

////
//// Base
////    
#include "py32f0xx.h"
#include "py32f040_hal_flash.h"

/// GD32 Support
#include "gdhal/gd32e10x_dma.h"
#include "gdhal/gd32e10x_i2c.h"
#include "gdhal/gd32e10x_timer.h"
#include "gdhal/gd32e10x_rtc.h"
#include "gdhal/gd32e10x_spi.h"
#include "gdhal/gd32e10x_usart.h"

////
//// driver
////
#include "./kdgpio.h"
#include "./kduart.h"
#include "./kdrtc.h"
#include "./kdwdog.h"
#include "./kdsaradc.h"
#include "./kdi2c.h"
#include "./kdspi.h"
#include "./kdmisc.h"
#include "./kdpwm.h"
#include "./kdimtd.h"

/**
 * @addtogroup BaseFunc
 * @note none
 */
 
/*@{*/

extern void kdpwr_reboot(void);
extern void kdpwr_shutDown(void);
extern void kdpwr_deepSleep(void);
extern void kdpwr_sysInit(void);

extern int32_t emmkDriver_initRefsCountUp(int8_t *initRefs);
extern int32_t emmkDriver_initRefsCountDown(int8_t *initRefs);

/*@}*/

#endif
