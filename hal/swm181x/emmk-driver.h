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

#define DEVICE_VERSION            "SWM181"
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
 * @addtogroup Hal define
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

#define AT_IN_RAM __attribute__((section("PlaceInRAM")))
#define BOOTLOADER_DATA_AREA   __attribute__((section("BOOTLOADER_DATA")))

extern int Image$$RW_IRAM1$$ZI$$Limit;
#define EMMK_KMEM_START_ADDRESS  ((void *) &Image$$RW_IRAM1$$ZI$$Limit)
#define EMMK_KMEM_END_ADDRESS    ((void *) (0x20000000 + 0x4000))

/*@}*/

////
//// Base
////
#include "SWM181.h"                     // Device header
#include "SWM181_adc.h"                 // Synwit::Device:Driver:ADC
#include "SWM181_cache.h"               // Synwit::Device:Driver:Cache
#include "SWM181_dma.h"                 // Synwit::Device:Driver:DMA
#include "SWM181_div.h"                 // Synwit::Device:Driver:Div
#include "SWM181_exti.h"                // Synwit::Device:Driver:EXIT
#include "SWM181_flash.h"               // Synwit::Device:Driver:Flash
#include "SWM181_gpio.h"                // Synwit::Device:Driver:GPIO
#include "SWM181_port.h"                // Synwit::Device:Driver:Port
#include "SWM181_pwm.h"                 // Synwit::Device:Driver:PWM
#include "SWM181_sdadc.h"               // Synwit::Device:Driver:SDADC
#include "SWM181_timr.h"                // Synwit::Device:Driver:Timer
#include "SWM181_uart.h"                // Synwit::Device:Driver:UART
#include "SWM181_wdt.h"                 // Synwit::Device:Driver:WDT
#include "SWM181_irqmux.h"              // Synwit::Device:Driver:IRQMux

////
//// driver
////
#include "./driver/kdsdadc.h"
#include "./driver/kdsaradc.h"
#include "./driver/kdgpio.h"
#include "./driver/kduart.h"
#include "./driver/kdwdog.h"
#include "./driver/kdmisc.h"
#include "./driver/kdi2c.h"
#include "./driver/kdspi.h"
#include "./driver/kdpwm.h"
#include "./driver/kdimtd.h"
#include "./driver/kdrtc.h"

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
