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

#ifndef _KLIB_ATTOKEN_H
#define _KLIB_ATTOKEN_H

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/**
 * @addtogroup Export functions
 * @note none
 */

/*@{*/

extern int32_t klAttoken_start(char **p_cur);
extern int32_t klAttoken_startWith(char **p_cur, char consume);

extern void klAttoken_skip(char **p_cur);
extern int32_t klAttoken_getNextInt(char **p_cur, int32_t *p_out);
extern int32_t klAttoken_getNextUInt(char **p_cur, uint32_t *p_out);
extern int32_t klAttoken_getNextBool(char **p_cur, uint8_t *p_out);
extern int32_t klAttoken_getNextString(char **p_cur, char **p_out, int32_t *length);
extern uint8_t klAttoken_hasmore(char **p_cur);

extern void klAttoken_skipSep(char **p_cur, char sep);
extern int32_t klAttoken_getNextIntSep(char **p_cur, char sep, int32_t *p_out);
extern int32_t klAttoken_getNextUIntSep(char **p_cur, char sep, uint32_t *p_out);
extern int32_t klAttoken_getNextBoolSep(char **p_cur, char sep, uint8_t *p_out);
extern int32_t klAttoken_getNextStringSep(char **p_cur, char sep, char **p_out, int32_t *length);
extern uint8_t klAttoken_hasmoreSep(char **p_cur, char sep);

/*@}*/

#endif //ESP8266_KLIB_ATTOKEN_H
