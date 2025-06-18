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
 * @addtogroup Bitmap
 * @note Chen, Xanne
 */
 
/*@{*/

void xbitmap_bufferBitSet(uint8_t *bitset, size_t i, bool flag) {
    size_t shiftedi = i / 8;
    
    uint8_t mask = ((uint8_t) 1) << (7 - i % 8);
    uint8_t dynmask = ((uint8_t) flag) << (7 - i % 8);
    
    uint8_t w = bitset[shiftedi];
    w &= ~mask;
    w |= dynmask;
    bitset[shiftedi] = w;
}

/*@}*/
