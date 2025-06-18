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
    RTC->WPR = 0xCA;
	RTC->WPR = 0x53;
    
    RTC->ISR &= ~0x04;
    
    RTC->WPR = 0;
}


void kdrtc_setupPeriod(uint32_t v) {
    RTC->WPR = 0xCA;
    RTC->WPR = 0x53;
    
    if (v) {
        RTC->INTCLR = (1 << 5u);
        RTC->ALM2PRD |= v;
        RTC->CR |= (0x01u << 5u);
        NVIC_EnableIRQ(RTC_IRQn);
    } else {
        NVIC_DisableIRQ(RTC_IRQn);
        RTC->INTCLR = (1 << 5u);
        RTC->CR &= ~(0x01u << 5u);
        RTC->ALM2PRD = 0u;
    }
    RTC->WPR = 0;
}


void kdrtc_clearPeriodFLag(void) {
    RTC->WPR = 0xCA;
    RTC->WPR = 0x53;
    
    RTC->INTCLR = (1 << 5u);
    
    RTC->WPR = 0;
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
    
	RTC->WPR = 0xCA;
	RTC->WPR = 0x53;
    
    RTC->ISR |= 0x01;
    qSTimer_t wait;
    qSTimer_Set(&wait, 1000);
	while (!(RTC->ISR & (0x02)) && !qSTimer_Expired(&wait)) {
        __WFI();
    }
    
	RTC->TIME = bin2bcd(tm.sec) | (bin2bcd(tm.min) << 8) | (bin2bcd(tm.hour) << 16) | (bin2bcd(tm.wday) << 24);
	RTC->DATE = bin2bcd(tm.day) | (bin2bcd(tm.mon) << 8) | (bin2bcd((tm.year - 2000)) << 16);
	RTC->ISR &= ~(0x01 | 0x04);
    qSTimer_Set(&wait, 1000);
	while (!(RTC->ISR & (0x02)) && !qSTimer_Expired(&wait)) {
        __WFI();
    }
    
    RTC->WPR = 0;
}


void kdrtc_getTm(klDateTime_SampleTm_t *tm) {
    qSTimer_t wait;
    qSTimer_Set(&wait, 1000);
	while (!(RTC->ISR & (0x04)) && !qSTimer_Expired(&wait)) {
        __WFI();
    }
    
    tm->hour = bcd2bin((RTC->TIME >> 16) & 0xFF);
	tm->min = bcd2bin((RTC->TIME >> 8) & 0xFF);
	tm->sec = bcd2bin((RTC->TIME >> 0) & 0xFF);

	tm->year = bcd2bin(RTC->DATE >> 16 & 0xFF) + 2000;
	tm->mon = bcd2bin(RTC->DATE >> 8 & 0xFF);
	tm->day = bcd2bin(RTC->DATE & 0xFF);
}


void kdrtc_setBkp(uint8_t index, uint8_t value) {
    RTC->WPR = 0xCA;
    RTC->WPR = 0x53;
    
    if (index <= 2) {
        uint32_t rv = RTC->ALM1TIME;
        if (index == 0) {
            rv &= ~GENMASK(20, 16);
            rv |= FIELD_PREP(GENMASK(20, 16), value);
        } else if (index == 1) {
            rv &= ~GENMASK(14, 8);
            rv |= FIELD_PREP(GENMASK(14, 8), value);
        } else if (index == 2) {
            rv &= ~GENMASK(6, 0);
            rv |= FIELD_PREP(GENMASK(6, 0), value);
        }
        RTC->ALM1TIME = rv;
    } else if (index <= 5) {
        uint32_t rv = RTC->ALM1DATE;
        if (index == 3) {
            rv &= ~GENMASK(23, 16);
            rv |= FIELD_PREP(GENMASK(23, 16), value);
        } else if (index == 4) {
            rv &= ~GENMASK(12, 8);
            rv |= FIELD_PREP(GENMASK(12, 8), value);
        } else if (index == 5) {
            rv &= ~GENMASK(5, 0);
            rv |= FIELD_PREP(GENMASK(5, 0), value);
        } 
        RTC->ALM1DATE = rv;
    }
    
    RTC->WPR = 0;
}


uint32_t kdrtc_getBkp(uint8_t index) {
    if (index <= 2) {
        uint32_t rv = RTC->ALM1TIME;
        if (index == 0) {
            return FIELD_GET(GENMASK(20, 16), rv);
        } else if (index == 1) {
            return FIELD_GET(GENMASK(14, 8), rv);
        } else if (index == 2) {
            return FIELD_GET(GENMASK(6, 0), rv);
        }
    } else if (index <= 5) {
        uint32_t rv = RTC->ALM1DATE;
        if (index == 3) {
            return FIELD_GET(GENMASK(23, 16), rv);
        } else if (index == 4) {
            return FIELD_GET(GENMASK(12, 8), rv);
        } else if (index == 5) {
            return FIELD_GET(GENMASK(5, 0), rv);
        }
    }
    return 0;
}

/*@}*/
