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

#include "./xfilter_mova.h"

/**
 * @addtogroup DebugSupport
 * @note none
 */

/*@{*/

#undef TAG
#define TAG     "xfilter-mova"

/*@}*/

/**
 * @addtogroup Private func
 * @note none
 */
 
/*@{*/

static void min_max(float *buf, uint16_t start, uint16_t end, float *vmax, float *vmin) {
    if (end <= start + 1) {
        if (buf[start] >= buf[end]) {
            *vmax = buf[start];
            *vmin = buf[end];
        } else {
            *vmax = buf[end];
            *vmin = buf[start];
        }
        
        return;
    }
    
    float lmin, lmax, rmax, rmin;
    
    min_max(buf, start, start + (end - start) / 2, &lmax, &lmin);
    min_max(buf, start + (end - start) / 2 + 1, end, &rmax, &rmin);
    
    if (lmin > rmin) {
        *vmin = rmin;
    } else {
        *vmin = lmin;
    }
    
    if (lmax > rmax) {
        *vmax = lmax;
    } else {
        *vmax = rmax;
    }
}

/*@}*/

/**
 * @addtogroup moving-average filter fucntions
 * @note none
 */
 
/*@{*/

void xfilter_mova_init(xfilter_mova_t *filter, float *moving_buf, uint32_t buf_size) {
    ASSERT(filter != NULL);
    ASSERT(moving_buf != NULL);
    
    memset(filter, 0x00, sizeof(xfilter_mova_t));
    filter->moving_buf = moving_buf;
    filter->buf_size = buf_size;
}


float xfilter_mova(xfilter_mova_t *filter, float new_va) {
    float av;
    float vmax, vmin;
    
    ASSERT(filter != NULL);
    
    if (!filter->is_init) {
        for (uint32_t i = 0; i < filter->buf_size; i++) {
            filter->moving_buf[i] = new_va;
        }
        filter->pos = 0;
        filter->is_init = 1;
    }
    
    filter->pos = (filter->pos + 1) % filter->buf_size;
    filter->moving_buf[filter->pos] = new_va;
    
    min_max(filter->moving_buf, 0, filter->buf_size - 1, &vmax, &vmin);
    
    av = 0;
    for (uint32_t i = 0; i < filter->buf_size; i++) {
        av += filter->moving_buf[i];
    }
    av -= (vmax + vmin);
    av /= filter->buf_size - 2;
    
    return av;
}

/*@}*/

/**
 * @addtogroup DebugSupport
 * @note none
 */

/*@{*/

#undef TAG

/*@}*/
