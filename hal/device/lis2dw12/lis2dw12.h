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
 
#ifndef _LIS2DW12_H_
#define _LIS2DW12_H_
 
#include <stdint.h>

#include "./lis2dw12_reg.h"

/**
 * @addtogroup Define
 * @note none
 */
 
/*@{*/

/*@}*/

/**
 * @addtogroup VFS iotcl support
 * @note nones
 */
 
/*@{*/

/*@}*/

/**
 * @addtogroup Public func
 * @note none
 */
 
/*@{*/

extern void lis2dw12_initBus(void *devIf);
extern void lis2dw12_finalizeBus(void);

extern int32_t lis2dw12_initSoft(void);
extern void lis2dw12_finalizeSoft(void);

extern stmdev_ctx_t* lis2dw12_getCtx(void);

/*@}*/

#endif
