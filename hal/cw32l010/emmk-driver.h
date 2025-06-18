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

#include <stdint.h>

/**
 * @addtogroup Verison
 * @note none
 */
 
/*@{*/

#define DEVICE_VERSION            "CW32L010"
#define DEVICE_MAJOR_VERSION      "0"
#define DEVICE_SECONDARY_VERSION  "1"

#define EOS_DRIVER_VERSION    "EMMK-"DEVICE_VERSION"-"DEVICE_MAJOR_VERSION"."DEVICE_SECONDARY_VERSION

/*@}*/

/**
 * @addtogroup NVIC define
 * @note none
 */
 
/*@{*/

/*@}*/

/**
 * @addtogroup RAM define
 * @note none
 */
 
/*@{*/

#define SIZEALIGN(var, alignbytes) \
    ((unsigned int) ((var) + ((alignbytes) - 1)) & (unsigned int) (~(unsigned int) ((alignbytes) - 1)))

#define AT_NONCACHEABLE_SECTION(var) __attribute__((section("noncacheable"), zero_init)) var
#define AT_NONCACHEABLE_SECTION_ALIGN(var, alignbytes) \
    __attribute__((section("noncacheable"))) __attribute__((aligned(alignbytes))) var
#define AT_NONCACHEABLE_SECTION_INIT(var) __attribute__((section("noncacheable.init"))) var
#define AT_NONCACHEABLE_SECTION_ALIGN_INIT(var, alignbytes) \
    __attribute__((section("noncacheable.init"))) __attribute__((aligned(alignbytes))) var

extern int Image$$RW_IRAM1$$ZI$$Limit;
#define EMMK_KMEM_START_ADDRESS  ((void *) &Image$$RW_IRAM1$$ZI$$Limit)
#define EMMK_KMEM_END_ADDRESS    ((void *) (0x20000000 + 0x1000))

/*@}*/

/**
 * @addtogroup Extern var
 * @note none
 */
 
/*@{*/

extern uint32_t SystemCoreClock;

/*@}*/

////
//// Base
////
#include "cw32l010.h"
#include "cw32l010_rtc.h"

#include "RTE_Components.h"
#ifdef RTE_Compiler_IO_STDOUT_EVR
#include "EventRecorder.h"
#endif

////
//// driver
////
#include "./driver/kdmisc.h"
#include "./driver/kdgpio.h"
#include "./driver/kdpwm.h"
#include "./driver/kdspi.h"
#include "./driver/kduart.h"
#include "./driver/kdwdog.h"
#include "./driver/kdrtc.h"
#include "./driver/kdi2c.h"
#include "./driver/kdimtd.h"
#include "./driver/kdsaradc.h"
#include "./driver/kdstty.h"

/**
 * @addtogroup BaseFunc
 * @note none
 */
 
/*@{*/

extern void kdpwr_reboot(void);
extern void kdpwr_shutDown(void);
extern void kdpwr_deepSleep(void);
extern void kdpwr_sysInit(void);

extern void kdpwr_setRtcToLCR(void);
extern void kdpwr_setRtcToLXT(void);
extern uint8_t kdpwr_getLxtReady(void);

extern int32_t kdpwr_enableCLKTRIM(uint32_t val);
extern void kdpwr_disableCLKTRIM(void);

extern int32_t emmkDriver_initRefsCountUp(int8_t *initRefs);
extern int32_t emmkDriver_initRefsCountDown(int8_t *initRefs);

/*@}*/

#endif
