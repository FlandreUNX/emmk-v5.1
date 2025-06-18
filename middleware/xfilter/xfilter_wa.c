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

#include "./xfilter_wa.h"

/**
 * @addtogroup DebugSupport
 * @note none
 */

/*@{*/

#undef TAG
#define TAG     "xfilter-wa"

/*@}*/

/**
 * @addtogroup moving-average filter fucntions
 * @note none
 */
 
/*@{*/

void xfilter_wa_init(xfilter_wa_t *filter, uint32_t sample_num) {
    ASSERT(filter != NULL);
    ASSERT(sample_num > 0);
    
    memset(filter, 0x00, sizeof(xfilter_wa_t));
    filter->num_s = sample_num;
}


float xfilter_wa(xfilter_wa_t *filter, float new_va) {
    ASSERT(filter != NULL);
    
    if (!filter->is_init) {
        filter->is_init = 1;
        filter->last_va = new_va;
    }
    
    filter->last_va = (filter->last_va * (filter->num_s - 1) + new_va) / filter->num_s;
    
    return filter->last_va;
}

/*@}*/


/**
 * @addtogroup DebugSupport
 * @note none
 */

/*@{*/

#undef TAG

/*@}*/