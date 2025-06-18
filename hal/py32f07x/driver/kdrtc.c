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
 * @addtogroup Define
 * @note none
 */
 
/*@{*/

/* RTC register high / low bits mask */
#define RTC_HIGH_BITS_MASK         ((uint32_t) 0x000F0000U)  /* RTC high bits mask */
#define RTC_LOW_BITS_MASK          ((uint32_t) 0x0000FFFFU)  /* RTC low bits mask */

/* RTC register high bits offset */
#define RTC_HIGH_BITS_OFFSET       ((uint32_t) 16U)

/*@}*/

/**
 * @addtogroup System IRQn
 * @note none
 */
 
/*@{*/

void RTC_IRQHandler(void) {
    rtc_interrupt_flag_clear(RTC_INT_FLAG_ALARM);
    rtc_interrupt_disable(RTC_INT_ALARM);
    rtc_lwoff_wait();
}

/*@}*/

/**
 * @addtogroup hal functions
 * @note none
 */
 
/*@{*/

void kdrtc_syncRequest(void) {
    RTC_CTL &= ~RTC_CTL_RSYNF;
    while (RESET == (RTC_CTL & RTC_CTL_RSYNF)) {
        // __WFI();
    }
}

void kdrtc_setupPeriod(uint32_t v) {
    v = rtc_counter_get() + v;
    
    // rtc_register_sync_wait();
    while (RESET == (RTC_CTL & RTC_CTL_RSYNF)) {
        // __WFI();
    }
    
    rtc_lwoff_wait();
    
    if (v == 0) {
        rtc_alarm_config(0);
        rtc_lwoff_wait();
        
        rtc_interrupt_disable(RTC_INT_ALARM);
        rtc_lwoff_wait();
        rtc_interrupt_flag_clear(RTC_INT_FLAG_ALARM);
        rtc_lwoff_wait();
        
        NVIC_DisableIRQ(RTC_IRQn);
    } else {    
        rtc_interrupt_disable(RTC_INT_ALARM);
        rtc_lwoff_wait();
        rtc_interrupt_flag_clear(RTC_INT_FLAG_ALARM);
        rtc_lwoff_wait();
        
        rtc_alarm_config(v - 1);
        rtc_lwoff_wait();

        rtc_interrupt_enable(RTC_INT_ALARM);
        rtc_lwoff_wait();
        rtc_interrupt_flag_clear(RTC_INT_FLAG_ALARM);
        rtc_lwoff_wait();

        NVIC_EnableIRQ(RTC_IRQn);
    }
}

void kdrtc_clearPeriodFLag(void) {
    rtc_interrupt_flag_clear(RTC_INT_FLAG_ALARM);
}

uint32_t kdrtc_getTimestamp(uint32_t *ts) {
	time_t t;
    
	t = rtc_counter_get();
	
	if (ts == NULL) {
        return t;
	}
	
	*ts = t;
	return 0;
}

void kdrtc_setTimestamp(uint32_t *ts) {
    rtc_lwoff_wait();
    rtc_configuration_mode_enter();
    rtc_counter_set(*ts);
    rtc_configuration_mode_exit();
    rtc_lwoff_wait();
}

/*@}*/
