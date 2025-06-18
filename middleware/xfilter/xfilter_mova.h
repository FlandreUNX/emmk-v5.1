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

#ifndef _XFILTER_MOVA_H_
#define _XFILTER_MOVA_H_

#include <stdint.h>

/**
 * @addtogroup Typedef
 * @note none
 */
 
/*@{*/

typedef struct {
    float *moving_buf;
    uint32_t buf_size;

    uint16_t pos;

    uint8_t is_init:1;
} xfilter_mova_t;

/*@}*/

/**
 * @addtogroup moving-average filter fucntions
 * @note none
 */
 
/*@{*/

extern void xfilter_mova_init(xfilter_mova_t *filter, float *moving_buf, uint32_t buf_size);

extern float xfilter_mova(xfilter_mova_t *filter, float new_va);

/*@}*/
 
#endif
