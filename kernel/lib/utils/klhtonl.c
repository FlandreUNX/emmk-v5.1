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

/**
 * @addtogroup Define
 * @note none
 */
 
/*@{*/

#define Swap16(A) \
    ((((uint16_t) (A) & 0xff00) >> 8) | (((uint16_t) (A) & 0x00ff) << 8))
                              
#define Swap32(A) \
    ((((uint32_t) (A) & 0xff000000) >> 24) | \
    (((uint32_t) (A) & 0x00ff0000) >> 8) | \
    (((uint32_t) (A) & 0x0000ff00) << 8) | \
    (((uint32_t) (A) & 0x000000ff) << 24))
    
#define ENDIANNESS ((char) ENDIAN_TEST.mylong)  

/*@}*/

/**
 * @addtogroup Private constant
 * @note none
 */
 
/*@{*/

static union {   
    char c[4];   
    unsigned long mylong;   
} const ENDIAN_TEST = {{ 'l', '?', '?', 'b' }}; 

/*@}*/

/**
 * @addtogroup Public func
 * @note none
 */
 
/*@{*/

uint16_t klhtons(uint16_t hs) {
	return (ENDIANNESS=='l') ? Swap16(hs): hs;
}
 

uint32_t klhtonl(uint32_t hl) {
	return (ENDIANNESS=='l') ? Swap32(hl): hl;
}
 

uint16_t klntohs(uint16_t ns) {
	return (ENDIANNESS=='l') ? Swap16(ns): ns;	
}
 

uint32_t klntohl(uint32_t nl) {
	return (ENDIANNESS=='l') ? Swap32(nl): nl;	
}

/*@}*/
