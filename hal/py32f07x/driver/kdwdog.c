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
 * @addtogroup Interfaace define
 * @note none
 */
 
/*@{*/


/*@}*/

/**
 * @addtogroup Instance support functions
 * @note none
 */
 
/*@{*/


/*@}*/

/** 
 * @addtogroup Operate functions
 * @note none
 */
 
/*@{*/

void kdwdog_enable(void) {
    IWDG->KR = IWDG_KEY_ENABLE;
    IWDG->KR = IWDG_KEY_WRITE_ACCESS_ENABLE;
    IWDG->PR = 0b111;
    IWDG->RLR = 0xFFF;
    IWDG->KR = IWDG_KEY_RELOAD;
}


void kdwdog_reload(void) {
    IWDG->KR = IWDG_KEY_RELOAD;
}

/*@}*/
