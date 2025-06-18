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

/*@}*/

/**
 * @addtogroup System IRQn
 * @note none
 */
 
/*@{*/

/*@}*/

/**
 * @addtogroup hal functions
 * @note none
 */
 
/*@{*/

void kdrtc_syncRequest(void) {

}


void kdrtc_setupPeriod(uint32_t v) {

}


void kdrtc_clearPeriodFLag(void) {

}


uint32_t kdrtc_getTimestamp(uint32_t *ts) {
	time_t t;
    
	t = qClock_GetTick();
	
	if (ts == NULL) {
        return t;
	}
	
	*ts = t;
	return 0;
}


void kdrtc_setTimestamp(uint32_t *ts) {

}


void kdrtc_getTm(klDateTime_SampleTm_t *tm) {
    klDateTime_bktime(qClock_GetTick(), tm);
}


void kdrtc_setBkp(uint8_t index, uint8_t value) {

}


uint32_t kdrtc_getBkp(uint8_t index) {
    return 0;
}

/*@}*/

#pragma GCC pop_options
