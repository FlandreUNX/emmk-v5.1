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

#include "./klattoken.h"
#include "klstr.h"

/**
 * @addtogroup Define & Typedef
 * @note none
 */

/*@{*/



/*@}*/

/**
 * @addtogroup Private constants
 * @note none
 */

/*@{*/



/*@}*/

/**
 * @addtogroup Private vars
 * @note none
 */

/*@{*/



/*@}*/

/**
 * @addtogroup Private funcs
 * @note none
 */

/*@{*/

#ifndef __BSD_VISIBLE

static char *__strsep(char **stringp, const char *delim) {
    char *begin, *end;
    
    begin = *stringp;
    if (begin == NULL)
        return NULL;
    
    if (delim[0] == '\0' || delim[1] == '\0') {
        char ch = delim[0];
        
        if (ch == '\0')
            end = NULL;
        else {
            if (*begin == ch)
                end = begin;
            else if (*begin == '\0')
                end = NULL;
            else
                end = strchr (begin + 1, ch);
        }
    } else
        end = strpbrk(begin, delim);
    
    if (end) {
        *end++ = '\0';
        *stringp = end;
    } else
        *stringp = NULL;
    
    return begin;
}
#define strsep __strsep
#endif

#define skip_white_space klib_str_skipWhiteSpace


static void skip_next_command(char **p_cur, char sep) {
    if (*p_cur == NULL) {
        return;
    }
    
    while (**p_cur != '\0' && **p_cur != sep) {
        (*p_cur)++;
    }
    
    if (**p_cur == sep) {
        (*p_cur)++;
    }
}


static char *next_token(char **p_cur, char sep) {
    char *ret = NULL;
    
    skip_white_space(p_cur);
    
    if (*p_cur == NULL) {
        ret = NULL;
    } else if (**p_cur == '"') {
        (*p_cur)++;
        ret = strsep(p_cur, "\"");
        skip_next_command(p_cur, sep);
    } else {
        char seps[2] = {sep, '\0'};
        ret = strsep(p_cur, seps);
    }
    
    return ret;
}


////
//// Parses the next integer in the AT response line and places it in *p_out
//// returns 0 on success and -1 on fail
//// updates *p_cur
//// "base" is the same as the base param in strtol
////
static int32_t _getNextInt_base(char **p_cur, char sep, void *p_out, int base, uint32_t uns) {
    char *ret;
    
    if (*p_cur == NULL) {
        return -1;
    }
    
    ret = next_token(p_cur, sep);
    
    if (ret == NULL) {
        return -1;
    } else {
        long l;
        char *end;
        
        if (uns) {
            l = (long) strtoul(ret, &end, base);
            *((uint32_t *) p_out) = l;
        } else {
            l = strtol(ret, &end, base);
            *((int32_t *) p_out) = l;
        }
        
        if (end == ret) {
            return -1;
        }
        
        if (*end == 0x00) {
            *end = sep;
        }
    }
    
    return 0;
}

/*@}*/

/**
 * @addtogroup Export funcs
 * @note none
 */

/*@{*/

//// 
//// Starts tokenizing an AT response string
//// returns -1 if this is not a valid response string, 0 on success.
//// updates *p_cur with current position
//// 
int32_t klAttoken_start(char **p_cur) {
    if (*p_cur == NULL) {
        return -1;
    }
    
    // skip prefix
    // consume "^[^:]:"
    
    *p_cur = strchr(*p_cur, ':');
    
    if (*p_cur == NULL) {
        return -1;
    }
    
    (*p_cur)++;
    
    return 0;
}


////
//// Starts tokenizing an AT response string
//// returns -1 if this is not a valid response string, 0 on success.
//// updates *p_cur with current position
////
int32_t klAttoken_startWith(char **p_cur, char consume) {
    if (*p_cur == NULL) {
        return -1;
    }
    
    // skip prefix
    // consume "^[^:]:"
    
    *p_cur = strchr(*p_cur, consume);
    
    if (*p_cur == NULL) {
        return -1;
    }
    
    (*p_cur)++;
    
    return 0;
}


inline void klAttoken_skipSep(char **p_cur, char sep) {
    skip_next_command(p_cur, sep);
}


inline void klAttoken_skip(char **p_cur) {
    skip_next_command(p_cur, ',');
}


//// 
//// Parses the next base 10 integer in the AT response line
//// and places it in *p_out
//// returns 0 on success and -1 on fail
//// updates *p_cur
//// 
inline int32_t klAttoken_getNextIntSep(char **p_cur, char sep, int32_t *p_out) {
    return _getNextInt_base(p_cur, sep, p_out, 10, 0);
}


inline int32_t klAttoken_getNextInt(char **p_cur, int32_t *p_out) {
    return _getNextInt_base(p_cur, ',', p_out, 10, 0);
}


//// 
//// Parses the next base 16 integer in the AT response line
//// and places it in *p_out
//// returns 0 on success and -1 on fail
//// updates *p_cur
//// 
inline int32_t klAttoken_getNextUIntSep(char **p_cur, char sep, uint32_t *p_out) {
    return _getNextInt_base(p_cur, sep, p_out, 10, 1);
}


inline int32_t klAttoken_getNextUInt(char **p_cur, uint32_t *p_out) {
    return _getNextInt_base(p_cur, ',', p_out, 10, 1);
}


int32_t klAttoken_getNextBoolSep(char **p_cur, char sep, uint8_t *p_out) {
    uint8_t ret;
    int32_t result;
    
    ret = klAttoken_getNextIntSep(p_cur, sep, &result);
    
    if (ret > 0) {
        return -1;
    }
    
    // booleans should be 0 or 1
    if (!(result == 0 || result == 1)) {
        return -1;
    }
    
    if (p_out != NULL) {
        *p_out = (char) result;
    }
    
    return ret;
}


inline int32_t klAttoken_getNextBool(char **p_cur, uint8_t *p_out) {
    return klAttoken_getNextBoolSep(p_cur, ',', p_out);
}


int32_t klAttoken_getNextStringSep(char **p_cur, char sep, char **p_out, int32_t *length) {
    if (*p_cur == NULL) {
        return -1;
    }
    
    uint32_t plen = 0;
    *p_out = next_token(p_cur, sep);
    
    plen = strlen(*p_out);
    
    if (*(*p_out + plen) == 0x00) {
        *(*p_out + plen) = '"';
    }
    
    if (length != NULL) {
        *length = (int) plen;
    }
    
    return 0;
}


inline int32_t klAttoken_getNextString(char **p_cur, char **p_out, int32_t *length) {
    return klAttoken_getNextStringSep(p_cur, ',', p_out, length);
}


// returns 1 on "has more tokens" and 0 if no
uint8_t klAttoken_hasmore(char **p_cur) {
    return !(*p_cur == NULL || **p_cur == '\0');
}

/*@}*/
