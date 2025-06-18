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
 
#ifndef _MB_MISC_H_
#define _MB_MISC_H_

////
//// Base include
////
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/**
 * @addtogroup Modbus bit utils
 * @note none
 */

/*@{*/

/* ----------------------- uMBM_Util_SetBits -----------------------------*/
/**
 * ucBits[2] = {0, 0};
 *
 * // Set bit 4 to 1 (read: set 1 bit starting at bit offset 4 to value 1)
 * mb_misc_setBits(ucBits, 4, 1, 1);
 *
 * // Set bit 7 to 1 and bit 8 to 0.
 * mb_misc_setBits(ucBits, 7, 2, 0x01);
 *
 * // Set bits 8 - 11 to 0x05 and bits 12 - 15 to 0x0A;
 * mb_misc_setBits(ucBits, 8, 8, 0x5A);
 */
extern void mbMisc_SetBits(uint8_t *data_area, uint16_t bit_offset, uint8_t n_bits, uint8_t value);
 
/* ----------------------- uMBM_Util_GetBits -----------------------------*/
/**
 * uint8_t ucBits[2] = {0, 0};
 * uint8_t ucResult;
 *
 * // Extract the bits 3 - 10.
 * ucResult = mb_misc_getBits(ucBits, 3, 8);
 */
extern uint8_t mbMisc_getBits(uint8_t *data_area, uint16_t bit_offset, uint8_t n_bits);

/*@}*/

/**
 * @addtogroup Byte <--> Bits
 * @note none
 */
 
/*@{*/

extern void mbMisc_setBitsFromByte(uint8_t *dest, int32_t idx, const uint8_t value);
extern void mbMisc_setBitsFromBytes(uint8_t *dest, int32_t idx, uint32_t bitsCount, const uint8_t *tabByte);
extern uint8_t mbMisc_getBitsFromBytes(const uint8_t *src, int32_t idx, uint32_t bitsCount);

/*@}*/

/**
 * @addtogroup Float <--> uint16
 * @note none
 */
 
/*@{*/

extern void mbMisc_setFloatABCD(float f, uint16_t *dest);
extern void mbMisc_setFloatDCBA(float f, uint16_t *dest);
extern void mbMisc_setFloatBADC(float f, uint16_t *dest);
extern void mbMisc_setFloatCDAB(float f, uint16_t *dest);

extern float mbMisc_getFloatABCD(const uint16_t *src);
extern float mbMisc_getFloatDCBA(const uint16_t *src);
extern float mbMisc_getFloatBADC(const uint16_t *src);
extern float mbMisc_getFloatCDAB(const uint16_t *src);

/*@}*/

#endif
