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

#ifndef _KSTR_H_
#define _KSTR_H_

#include <stdbool.h>
#include <stdint.h>

/**
 * @addtogroup Public func
 * @note none
 */
 
/*@{*/

extern void klib_str_skipWhiteSpace(char **p_cur);

extern int32_t klib_str_u32s(uint32_t u32, char *dst);

extern char *klib_str_strdup(const char *s);

/*@}*/

/**
 * @addtogroup emmk-NewLib-1
 * @note none
 */
 
/*@{*/

extern void klStr_convertMacStr2Hex(const char *str, uint8_t *hex);

extern int32_t klStr_hex2str(uint8_t *src, uint16_t srcLen, char *dst);
extern uint16_t klStr_hexStr2hex(char *str, uint16_t strLength, char *out);

extern uint8_t klStr_str2bcd(char str[2]);
extern void klStr_bcd2str(char *bcd, char *str, uint16_t bcdLength);

extern uint32_t klStr_inetAddr(char *cp);

extern void klStr_dec2bcd_nl(uint64_t dec, uint8_t *buf, uint8_t *num);

/*@}*/

#endif
