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

#ifndef _XFILTER_WA_H_
#define _XFILTER_WA_H_

#include <stdint.h>

/**
 * @addtogroup Typedef
 * @note none
 */
 
/*@{*/

typedef struct {
    float last_va;     // Last process value
    uint32_t num_s;    // Number of samples
    uint8_t is_init:1;
} xfilter_wa_t;

/*@}*/

/**
 * @addtogroup moving-average filter fucntions
 * @note none
 */
 
/*@{*/

extern void xfilter_wa_init(xfilter_wa_t *filter, uint32_t sample_num);

extern float xfilter_wa(xfilter_wa_t *filter, float new_va);

/*@}*/
 
#endif
