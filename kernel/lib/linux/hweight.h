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
 
#ifndef _EMMK_LIB_HWEIGHT_H_
#define _EMMK_LIB_HWEIGHT_H_

#include <stdint.h>

/**
 * @addtogroup Public func
 * @note none
 */
 
/*@{*/

extern uint32_t __sw_hweight32(uint32_t w);
extern uint32_t __sw_hweight16(uint32_t w);
extern uint32_t __sw_hweight8(uint32_t w);
extern uint64_t __sw_hweight64(uint64_t w);

/*@}*/

#endif
