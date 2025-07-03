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

void RTC_WKUP_IRQHandler(void) {
    rtc_flag_clear(RTC_INT_WAKEUP);
    rtc_interrupt_disable(RTC_INT_WAKEUP);

    exti_interrupt_flag_clear(EXTI_20);
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
    rtc_register_sync_wait();

    if (v == 0) {
        rtc_wakeup_timer_set(0);
        rtc_wakeup_disable();
        rtc_interrupt_disable(RTC_INT_WAKEUP);

        NVIC_DisableIRQ(RTC_WKUP_IRQn);
        exti_interrupt_disable(EXTI_20);
    } else {
        rtc_interrupt_disable(RTC_INT_WAKEUP);
        rtc_flag_clear(RTC_INT_WAKEUP);
        rtc_wakeup_disable();

        rtc_wakeup_clock_set(WAKEUP_CKSPRE);
        rtc_wakeup_timer_set(v);

        rtc_interrupt_enable(RTC_INT_WAKEUP);
        rtc_wakeup_enable();

        exti_init(EXTI_20, EXTI_INTERRUPT, EXTI_TRIG_BOTH);
        exti_interrupt_enable(EXTI_20);

        NVIC_EnableIRQ(RTC_WKUP_IRQn);
    }
}


void kdrtc_clearPeriodFLag(void) {
    rtc_flag_clear(RTC_INT_WAKEUP);
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

    int32_t regTime = 0U, regDate = 0U;
    regDate = (DATE_YR(bin2bcd(tm.year - 2000))
               | DATE_DOW(bin2bcd(tm.wday))
               | DATE_MON(bin2bcd(tm.mon))
               | DATE_DAY(bin2bcd(tm.day)));
    regTime = (0 //tt.am_pm |
               | TIME_HR(bin2bcd(tm.hour))
               | TIME_MN(bin2bcd(tm.min))
               | TIME_SC(bin2bcd(tm.sec)));

    RTC_WPK = RTC_UNLOCK_KEY1;
    RTC_WPK = RTC_UNLOCK_KEY2;
    if (ERROR != rtc_init_mode_enter()) {
        RTC_TIME = (uint32_t) regTime;
        RTC_DATE = (uint32_t) regDate;

        RTC_CTL &= (uint32_t) (~RTC_CTL_CS);
        // RTC_CTL |= tt.display_format;
        rtc_init_mode_exit();
        rtc_register_sync_wait();
    }

    RTC_WPK = RTC_LOCK_KEY;
}


void kdrtc_getTm(klDateTime_SampleTm_t *tm) {
    rtc_parameter_struct tt;
    rtc_current_time_get(&tt);
    tm->min = bcd2bin(tt.minute);
    tm->hour = bcd2bin(tt.hour);
    tm->sec = bcd2bin(tt.second);
    tm->day = bcd2bin(tt.day_of_week);
    tm->mon = bcd2bin(tt.month);
    tm->year = bcd2bin(tt.year) + 2000;
}


void kdrtc_setBkp(uint32_t index, uint32_t value) {
    RTC_BKP1 = value;
    REG32((RTC) + 0x00000054U + index * 4) = value;
}


uint32_t kdrtc_getBkp(uint32_t index) {
    return REG32((RTC) + 0x00000054U + index * 4);
}

/*@}*/
