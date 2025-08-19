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

#define AT_IN_RAM       __attribute__((section("PlaceInRAM")))

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
#include "gd32f1x0.h"                   // Device header
#include "gd32f1x0_exti.h"              // GigaDevice::Device:GD32F1x0_StdPeripherals:EXTI
#include "gd32f1x0_fmc.h"               // GigaDevice::Device:GD32F1x0_StdPeripherals:FMC
#include "gd32f1x0_gpio.h"              // GigaDevice::Device:GD32F1x0_StdPeripherals:GPIO
#include "gd32f1x0_misc.h"              // GigaDevice::Device:GD32F1x0_StdPeripherals:MISC
#include "gd32f1x0_pmu.h"               // GigaDevice::Device:GD32F1x0_StdPeripherals:PMU
#include "gd32f1x0_rcu.h"               // GigaDevice::Device:GD32F1x0_StdPeripherals:RCU
#include "gd32f1x0_rtc.h"               // GigaDevice::Device:GD32F1x0_StdPeripherals:RTC
#include "gd32f1x0_timer.h"             // GigaDevice::Device:GD32F1x0_StdPeripherals:TIMER
#include "gd32f1x0_usart.h"             // GigaDevice::Device:GD32F1x0_StdPeripherals:USART
#include "gd32f1x0_libopt.h"            // GigaDevice::Device:GD32F1x0_libopt
#include "gd32f1x0_wwdgt.h"             // GigaDevice::Device:GD32F1x0_StdPeripherals:WWDGT
#include "gd32f1x0_fwdgt.h"             // GigaDevice::Device:GD32F1x0_StdPeripherals:FWDGT
#include "gd32f1x0_spi.h"               // GigaDevice::Device:GD32F1x0_StdPeripherals:SPI_I2S
#include "gd32f1x0_dma.h"
#include "gd32f1x0_syscfg.h"
#include "gd32f1x0_i2c.h"

#include "RTE_Components.h"

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
