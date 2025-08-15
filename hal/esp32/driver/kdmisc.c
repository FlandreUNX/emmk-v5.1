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

#include "freertos/task.h"

/**
 * @name IdFunc
 * @{
 */

void kdmisc_getCPUID(uint8_t out[12]) {
}

/** @} */

/**
 * @name DelayFunc
 * @{
 */

void kdmisc_delayInit(void) {

}


void kdmisc_delayFinalize(void) {

}


void kdmisc_delayUs(uint64_t v) {

}


void inline kdmisc_delayMs(uint32_t v) {
    vTaskDelay(pdMS_TO_TICKS(v));
}

/** @} */

/**
 * @name Clone Func
 * @{
 */

void kdmisc_cloneEncrypt(void) {
#if CONFIG_USING_CLONE_ENCRYPT == 1
    _encryptCheck();
#endif
}

/** @} */

/**
 * @name Systick function
 * @{
 */

void kdmisc_systickDisable(void) {

}


void kdmisc_systickEnable(void) {

}


int32_t kdmisc_systickInit(uint32_t freq) {
    return 0;
}

/** @} */
