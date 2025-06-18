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
 
/*
 * Copyright (C) 2003 Bernardo Innocenti <bernie@develer.com>
 * Based on former asm-ppc/div64.h and asm-m68knommu/div64.h
 *
 * Optimization for constant divisors on 32-bit machines:
 * Copyright (C) 2006-2015 Nicolas Pitre
 *
 * The semantics of do_div() are:
 *
 * uint32_t do_div(uint64_t *n, uint32_t base)
 * {
 * 	uint32_t remainder = *n % base;
 * 	*n = *n / base;
 * 	return remainder;
 * }
 *
 * NOTE: macro parameter n is evaluated multiple times,
 *       beware of side effects!
 */
 
#ifndef _EMMK_LIB_GENERIC_DIV64_H
#define _EMMK_LIB_GENERIC_DIV64_H

#include <stdint.h>

static inline uint32_t do_div(uint64_t *n, uint32_t base) {
	uint32_t remainder = *n % base;
	*n = *n / base;
	return remainder;
}

#endif
