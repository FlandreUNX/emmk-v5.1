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

#ifndef _XFILTER_RCLPF_H_
#define _XFILTER_RCLPF_H_

#include <stdint.h>

/**
 * @addtogroup Typedef
 * @note none
 */
 
/*@{*/

typedef struct {
    float dcdiff_th;          // Data change difference threshold
    uint32_t sct_th;           // Stationary number threshold
    
    float dkx;              // Default Filter coefficient
    float kx;               // Filter coefficient
    float kx_step;          // Coefficient growth step
    
    uint8_t pre_va_dir:1;   // The direction of this data change. 1:up, 0:down
    float pre_va;           // Last processed result
    
    uint32_t fc;            // Filter counter

    uint8_t is_init:1;      // Whether the filter has been initialized
} xfilter_rclpf_dynamic_t;

typedef struct {
    float kx;               // Filter coefficient
    float pre_va;           // Last processed result
    uint8_t is_init:1;      // Whether the filter has been initialized
} xfilter_rclpf_fix_t;

/*@}*/

/**
 * @addtogroup Dynamic RC-LPF fucntions
 * @note none
 */
 
/*@{*/

extern void xfilter_rclpf_dynamic_init(xfilter_rclpf_dynamic_t *f, 
        float diff_th, uint32_t sct_th, 
        float kx, float kx_step);
        
extern float xfilter_rclpf_dynamic(xfilter_rclpf_dynamic_t *f, float new_va);

/*@}*/
        
/**
 * @addtogroup Fix RC-LPF fucntions
 * @note none
 */
 
/*@{*/

extern void xfilter_rclpf_fix_init(xfilter_rclpf_fix_t *filter, float kx);
        
extern float xfilter_rclpf_fix(xfilter_rclpf_fix_t *filter, float new_va);

/*@}*/

#endif
