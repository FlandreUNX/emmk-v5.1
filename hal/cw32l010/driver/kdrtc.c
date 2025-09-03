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
 * @addtogroup Interface define
 * @note none
 */
 
/*@{*/

static void rtcReset(void) {
    RTC_Cmd(DISABLE);
    RTC_SetClockSource(CW_SYSCTRL->LSE_f.STABLE ? RTC_RTCCLK_FROM_LSE : RTC_RTCCLK_FROM_LSI);
    RTC_UNLOCK();
    CW_RTC->CR0_f.H24 = 1;
    RTC_LOCK();
    RTC_Cmd(ENABLE);
}

/*@}*/

/**
 * @addtogroup System IRQn
 * @note none
 */
 
/*@{*/

void RTC_IRQHandler(void) {
    kdrtc_clearPeriodFLag();
}

/*@}*/

/**
 * @addtogroup hal functions
 * @note none
 */
 
/*@{*/

void kdrtc_syncRequest(void) {
}


void kdrtc_setupPeriod(uint32_t v) {
    if (v) {
        RTC_UNLOCK();
        if (IS_RTC_START()) {
            uint32_t timeout = 100000;
            CW_RTC->CR1_f.ACCESS = 1;
            while (!CW_RTC->CR1_f.WINDOW) {
                timeout--;
                if (timeout == 0) {
                    break;
                }
            }
        }
        CW_RTC->CR2_f.AWTSRC = 0b1;
        CW_RTC->CR2_f.AWTPRS = 0b00;
        CW_RTC->AWTARR = v;
        CW_RTC->IER_f.AWTIMER = 0b1;
        CW_RTC->CR2_f.AWTEN = 0b1;
        CW_RTC->CR1_f.ACCESS = 0b0;
        RTC_LOCK();
        NVIC_EnableIRQ(RTC_IRQn);
    } else {
        RTC_UNLOCK();
        if (IS_RTC_START()) {
            uint32_t timeout = 100000;
            CW_RTC->CR1_f.ACCESS = 1;
            while (!CW_RTC->CR1_f.WINDOW) {
                timeout--;
                if (timeout == 0) {
                    break;
                }
            }
        }
        CW_RTC->CR2_f.AWTEN = 0b0;
        CW_RTC->IER_f.AWTIMER = 0b0;
        CW_RTC->CR1_f.ACCESS = 0b0;
        RTC_LOCK();
        NVIC_DisableIRQ(RTC_IRQn);
    }
}


void kdrtc_clearPeriodFLag(void) {
    RTC_ClearITPendingBit(RTC_IT_AWTIMER);
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
    time_t t;
    t = *ts;
    klDateTime_bktime(t, &tm);
    
	RTC_UNLOCK();
    
    if (IS_RTC_START()) {
        qSTimer_t wait;
        qSTimer_Set(&wait, 1100);
        RTC_ACCESS_SET();
        while (!qSTimer_Expired(&wait)) {
            if (RTC_WINDOW_GETVALUE()) {
                break;
            }
        }
        if (qSTimer_Expired(&wait)) {
            rtcReset();

            qSTimer_Set(&wait, 1100);
            RTC_ACCESS_SET();
            while (!qSTimer_Expired(&wait)) {
                if (RTC_WINDOW_GETVALUE()) {
                    break;
                }
            }
        }
    }
    
    CW_RTC->TIME = bin2bcd(tm.sec) | (bin2bcd(tm.min) << 8) | (bin2bcd(tm.hour) << 16);
    CW_RTC->DATE = bin2bcd(tm.day) | (bin2bcd(tm.mon) << 8) | (bin2bcd((tm.year - 2000)) << 16) | (bin2bcd(tm.wday) << 24);
    
    RTC_ACCESS_RESET();
    
    RTC_LOCK();
}


void kdrtc_getTm(klDateTime_SampleTm_t *tm) {
    qSTimer_t wait;
    qSTimer_Set(&wait, 1100);

    if (IS_RTC_START()) {
        while (!qSTimer_Expired(&wait)) {
            if (CW_RTC->CR1_f.WAIT == 0) {
                break;
            }
        }

        if (qSTimer_Expired(&wait)) {
            rtcReset();

            qSTimer_Set(&wait, 1100);
            while (!qSTimer_Expired(&wait)) {
                if (CW_RTC->CR1_f.WAIT == 0) {
                    break;
                }
            }
        }
    }
    
    uint32_t regTemp;
    qSTimer_Set(&wait, 1100);
    do {
        regTemp = CW_RTC->TIME;
        if (qSTimer_Expired(&wait)) {
            break;
        }
    }
    while (regTemp != CW_RTC->TIME);

	tm->hour = bcd2bin((uint8_t) ((regTemp & RTC_TIME_HOUR_Msk) >> 16));
	tm->min = bcd2bin((uint8_t) ((regTemp & RTC_TIME_MINUTE_Msk) >> 8));
	tm->sec = bcd2bin((uint8_t) (regTemp & RTC_TIME_SECOND_Msk));
    if (CW_RTC->CR0_f.H24 == RTC_HOUR12) {
        tm->hour = bcd2bin(((uint8_t) (regTemp & RTC_TAMPTIME_HOUR_Msk)) & 0x1F);
    }

    qSTimer_Set(&wait, 1100);
    do {
        regTemp = CW_RTC->DATE;
        if (qSTimer_Expired(&wait)) {
            break;
        }
    }
    while (regTemp != CW_RTC->DATE);

	tm->year = 2000 + bcd2bin((uint8_t) ((regTemp & RTC_DATE_YEAR_Msk) >> 16));
	tm->mon = bcd2bin((uint8_t) ((regTemp & RTC_DATE_MONTH_Msk) >> 8));
	tm->day = bcd2bin((uint8_t) (regTemp & RTC_DATE_DAY_Msk));
    tm->wday = bcd2bin((uint8_t) ((regTemp & RTC_DATE_WEEK_Msk) >> 24));
}


void kdrtc_setBkp(uint8_t index, uint8_t value) {
}


uint32_t kdrtc_getBkp(uint8_t index) {
    return 0;
}

/*@}*/
