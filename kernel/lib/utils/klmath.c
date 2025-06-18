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
 * @addtogroup Locate const
 * @note none
 */
 
/*@{*/


/*@}*/

/**
 * @addtogroup Public func
 * @note none
 */
 
/*@{*/

float klMath_carmSqrt(float x) {
    union{
        int intPart;
        float floatPart;
    } convertor;
    
    union{
        int intPart;
        float floatPart;
    } convertor2;
    
    convertor.floatPart = x;
    convertor2.floatPart = x;
    convertor.intPart = 0x1FBCF800 + (convertor.intPart >> 1);
    convertor2.intPart = 0x5f3759df - (convertor2.intPart >> 1);
    
    return 0.5f * (convertor.floatPart + (x * convertor2.floatPart));
}


int32_t klMatch_uFastAbs(int32_t v) {
    return (v ^ (v >> 31)) - (v >> 31);
}


/*@}*/



