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

#include "./xfilter_rclpf.h"

/**
 * @addtogroup DebugSupport
 * @note none
 */

/*@{*/

#undef TAG
#define TAG     "xfilter-lpf"

/*@}*/

/**
 * @addtogroup Dynamic RC-LPF fucntions
 * @note none
 */
 
/*@{*/

// diff_th 差值阈值
// sct_th 累加数阈值
// kx 基础系数
// kx_step 步进系数
void xfilter_rclpf_dynamic_init(xfilter_rclpf_dynamic_t *filter, 
        float diff_th, uint32_t sct_th, 
        float kx, float kx_step) {
            
    ASSERT(filter != NULL);
    
    memset(filter, 0x00, sizeof(xfilter_rclpf_dynamic_t));
    
    filter->dcdiff_th = diff_th;
    filter->sct_th = sct_th;
    filter->dkx = filter->kx = kx;
    filter->kx_step = kx_step;
}


float xfilter_rclpf_dynamic(xfilter_rclpf_dynamic_t *filter, float new_va) {
    uint8_t new_dir = 0;
    
    ASSERT(filter != NULL);
    
    if (!filter->is_init) {
        filter->is_init = 1;
        filter->pre_va = new_va;
        return new_va;
    }
    
    if ((new_va - filter->pre_va) > 0) {
        new_dir = 1;
    } else if ((new_va - filter->pre_va) < 0) {
        new_dir = 0;
    }
    
    if (new_dir == filter->pre_va_dir) {
        filter->fc += 1;
        
        if (fabs(new_va - filter->pre_va) > filter->dcdiff_th) {
            filter->fc += 2;
        }
        
        if (filter->fc > filter->sct_th) {
            filter->fc = 0;
            filter->kx += filter->kx_step;
        }
    } else {
        filter->kx = filter->dkx;
        filter->fc = 0;
    }
    
    filter->pre_va = (1.0f - filter->kx) * filter->pre_va + filter->kx * new_va;
    
    return filter->pre_va;
}

/*@}*/

/**
 * @addtogroup Fix RC-LPF fucntions
 * @note none
 */
 
/*@{*/

void xfilter_rclpf_fix_init(xfilter_rclpf_fix_t *filter, float kx) {  
    ASSERT(filter != NULL);
    
    memset(filter, 0x00, sizeof(xfilter_rclpf_fix_t));
    
    filter->kx = kx;
}


float xfilter_rclpf_fix(xfilter_rclpf_fix_t *filter, float new_va) {
    ASSERT(filter != NULL);
    
    if (!filter->is_init) {
        filter->is_init = 1;
        filter->pre_va = new_va;
        return new_va;
    }
    
    filter->pre_va = (1.0f - filter->kx) * filter->pre_va + filter->kx * new_va;
    
    return filter->pre_va;
}

/*@}*/

/**
 * @addtogroup DebugSupport
 * @note none
 */

/*@{*/

#undef TAG

/*@}*/

