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
#define RTC_HIGH_BITS_MASK         ((uint32_t)0x000F0000U)  /* RTC high bits mask */
#define RTC_LOW_BITS_MASK          ((uint32_t)0x0000FFFFU)  /* RTC low bits mask */

/* RTC register high bits offset */
#define RTC_HIGH_BITS_OFFSET       ((uint32_t)16U)

/*@}*/

/**
 * @addtogroup System IRQn
 * @note none
 */

/*@{*/

void RTC_IRQHandler(void) {
    rtc_flag_clear(RTC_FLAG_ALARM0);
    rtc_interrupt_disable(RTC_INT_ALARM);
    rtc_register_sync_wait();
}


void RTC_Alarm_IRQHandler(void) {
    rtc_flag_clear(RTC_FLAG_ALARM0);
    rtc_interrupt_disable(RTC_INT_ALARM);
    rtc_register_sync_wait();
    exti_interrupt_flag_clear(EXTI_17);
}

/*@}*/

/**
 * @addtogroup hal functions
 * @note none
 */

/*@{*/

void kdrtc_syncRequest(void) {
    rtc_register_sync_wait();
}


void kdrtc_setupPeriod(uint32_t v) {
    // v = rtc_counter_get() + v;
    //
    // rtc_register_sync_wait();
    // rtc_register_sync_wait();
    //
    // if (v == 0) {
    //     rtc_alarm_config(0);
    //     rtc_register_sync_wait();
    //
    //     rtc_interrupt_disable(RTC_INT_ALARM);
    //     rtc_register_sync_wait();
    //     rtc_interrupt_flag_clear(RTC_FLAG_ALARM0);
    //     rtc_register_sync_wait();
    //
    //     NVIC_DisableIRQ(RTC_IRQn);
    //     NVIC_DisableIRQ(RTC_ALARM_IRQn);
    //     exti_interrupt_disable(EXTI_17);
    // } else {
    //     rtc_interrupt_disable(RTC_INT_ALARM);
    //     rtc_register_sync_wait();
    //     rtc_interrupt_flag_clear(RTC_FLAG_ALARM0);
    //     rtc_register_sync_wait();
    //
    //     rtc_alarm_config(v - 1);
    //     rtc_register_sync_wait();
    //
    //     rtc_interrupt_enable(RTC_INT_ALARM);
    //     rtc_register_sync_wait();
    //     rtc_interrupt_flag_clear(RTC_FLAG_ALARM0);
    //     rtc_register_sync_wait();
    //
    //     exti_init(EXTI_17, EXTI_INTERRUPT, EXTI_TRIG_BOTH);
    //     exti_interrupt_enable(EXTI_17);
    //
    //     NVIC_EnableIRQ(RTC_IRQn);
    //     NVIC_EnableIRQ(RTC_ALARM_IRQn);
    // }
}


void kdrtc_clearPeriodFLag(void) {
    rtc_flag_clear(RTC_FLAG_ALARM0);
}


uint32_t kdrtc_getTimestamp(uint32_t *ts) {
    klDateTime_SampleTm_t tm;
    time_t t;

    kdrtc_getTm(&tm);
    t = klDateTime_mktime(&tm);

    if (ts == NULL) {
        return t;
    }

    *ts = t;
    return 0;
}


void kdrtc_setTimestamp(uint32_t *ts) {
    klDateTime_SampleTm_t tm;
    klDateTime_bktime(*ts, &tm);

    rtc_parameter_struct tmt = {
        .rtc_year = bin2bcd(tm.year - 2000),
        .rtc_month = bin2bcd(tm.mon),
        .rtc_date = bin2bcd(tm.day),
        .rtc_hour = bin2bcd(tm.hour),
        .rtc_minute = bin2bcd(tm.min),
        .rtc_second = bin2bcd(tm.sec),
    };
    rtc_init(&tmt);
}


void kdrtc_getTm(klDateTime_SampleTm_t *tm) {
    uint32_t temp_tr = 0U, temp_dr = 0U;

    temp_tr = (uint32_t) RTC_TIME;
    temp_dr = (uint32_t) RTC_DATE;

    tm->year = bcd2bin(GET_DATE_YR(temp_dr)) + 2000;
    tm->mon = bcd2bin(GET_DATE_MON(temp_dr));
    tm->day = bcd2bin(GET_DATE_DAY(temp_dr));
    tm->wday = bcd2bin(GET_DATE_DOW(temp_dr));
    tm->hour = bcd2bin(GET_TIME_HR(temp_tr));
    tm->min = bcd2bin(GET_TIME_MN(temp_tr));
    tm->sec = bcd2bin(GET_TIME_SC(temp_tr));
}


void kdrtc_setBkp(uint8_t index, uint8_t value) {
}


uint32_t kdrtc_getBkp(uint8_t index) {
}

/*@}*/
