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
   RCC->PCLKEN |= (1 << 17);
    IWDG->UNLOCK = 0x55AA6699;
    IWDG->CFGR &= ~0x01;
    IWDG->RLOAD = 0xFFFFF;
    IWDG->UNLOCK = 0;
    IWDG->CMDCR = 0x55;
    IWDG->CMDCR = 0xAA;
}


void kdwdog_reload(void) {
    IWDG->CMDCR = 0xAA;
}

/*@}*/
