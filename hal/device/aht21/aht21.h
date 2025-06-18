/*
 * Copyright (C) 2018 Flandreunx@outlook.com
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
 
#ifndef _AHT21_H_
#define _AHT21_H_

#include <stdint.h>

/**
 * @addtogroup Typedef
 * @note none
 */
 
/*@{*/


/*@}*/

/** 
 * @addtogroup Export func
 * @note none
 */
 
/*@{*/

extern void aht21_initBus(void *devIf);
extern int32_t aht21_initSoft(void);
extern void aht21_finalizeBus(void);
extern void aht21_finalizeSoft(void);

extern int32_t aht21_measure(void);

/*@}*/

/**
 * @addtogroup Get/Set
 * @note none
 */
 
/*@{*/

extern float aht21_getTempC(void);
extern float aht21_getHumi(void);

/*@}*/

#endif
