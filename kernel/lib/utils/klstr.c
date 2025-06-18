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

static const char HEX_DIGITS[] = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};

/*@}*/

/**
 * @addtogroup PrivateFunc
 * @note none
 */

/*@{*/

static uint32_t __atol_ipv4(const char *nptr) {
    uint32_t total = 0;
    char sign = '+';
    while (isspace((int) *nptr)) {
        ++nptr;
    }

    if (*nptr == '-' || *nptr == '+') {
        sign = *nptr++;
    }

    while (isdigit((int) *nptr)) {
        total = 10 * total + ((*nptr++) - '0');
    }

    return (sign == '-') ? -total : total;
}


static uint8_t a2x(const char c) {
    switch (c) {
        case '0' ... '9':
            return (uint8_t) atoi(&c);
        case 'a' ... 'f':
            return 0xA + (c - 'a');
        case 'A' ... 'F':
            return 0xA + (c - 'A');
        default:
            return 0;
    }
}

/*@}*/

/**
 * @addtogroup Public func
 * @note none
 */

/*@{*/

void klib_str_skipWhiteSpace(char **p_cur) {
    if (*p_cur == NULL) {
        return;
    }

    while (**p_cur != '\0' && isspace((int) (**p_cur))) {
        (*p_cur)++;
    }
}

int32_t klib_str_u32s(uint32_t u32, char *dst) {
    uint8_t dst_len = 0;

    if (dst == NULL) {
        return 0;
    }

    for (int32_t i = 4; i > 0; i--) {
        uint8_t c = u32 >> (8 * (i - 1)) & 0x000000FF;
        dst[dst_len++] = HEX_DIGITS[(c & 0xF0) >> 4];
        dst[dst_len++] = HEX_DIGITS[(c & 0x0F)];
    }

    return dst_len;
}


char *klib_str_strdup(const char *s) {
#if EMMK_CFG_KMEM_MANAGER >= 0
    if (s == NULL) {
        return NULL;
    }

    uint32_t len = strlen(s) + 1;
    char *tmp = (char *) malloc(len);

    if (!tmp) {
        return NULL;
    }

    memcpy(tmp, s, len);

    return tmp;
#else
    return NULL;
#endif
}

/*@}*/

/**
 * @addtogroup emmk-NewLib-1
 * @note none
 */

/*@{*/

void klStr_convertMacStr2Hex(const char *str, uint8_t *hex) {
    char macStr[12] = {
            str[0], str[1],
            str[3], str[4],
            str[6], str[7],
            str[9], str[10],
            str[12], str[13],
            str[15], str[16],
    };
    klStr_hexStr2hex(macStr, 12, (char *) hex);
}


int32_t klStr_hex2str(uint8_t *src, uint16_t srcLen, char *dst) {
    if (src == NULL || srcLen == 0 || dst == NULL) {
        return 0;
    }

    uint16_t dst_len = 0;
    for (uint16_t i = 0; i < srcLen; i++) {
        dst[dst_len++] = HEX_DIGITS[(src[i] & 0xF0) >> 4];
        dst[dst_len++] = HEX_DIGITS[(src[i] & 0x0F)];
    }

    return dst_len;
}


uint8_t klStr_str2bcd(char str[2]) {
    return (uint8_t) ((str[0] - '0') << 4) + (str[1] - '0');
}


void klStr_bcd2str(char *bcd, char *str, uint16_t bcdLength) {
    for (uint8_t i = 0; i < bcdLength; i++) {
        str[i * 2] = bcd2numc_h(bcd[i]);
        str[i * 2 + 1] = bcd2numc_l(bcd[i]);
    }
}


uint16_t klStr_hexStr2hex(char *str, uint16_t strLength, char *out) {
    char *p = str;
    char high = 0, low = 0;
    uint16_t tmplen = strLength, cnt = 0;

    while (cnt < (tmplen / 2)) {
        high = ((*p > '9') && ((*p <= 'F') || (*p <= 'f'))) ? *p - 48 - 7 : *p - 48;
        low = (*(++p) > '9' && ((*p <= 'F') || (*p <= 'f'))) ? *(p) - 48 - 7 : *(p) - 48;
        out[cnt] = ((high & 0x0f) << 4 | (low & 0x0f));
        p++;
        cnt++;
    }

    if (tmplen % 2 != 0) {
        out[cnt] = ((*p > '9') && ((*p <= 'F') || (*p <= 'f'))) ? *p - 48 - 7 : *p - 48;
    }

    return tmplen / 2 + tmplen % 2;
}


uint32_t klStr_inetAddr(char *cp) {
    __attribute__((aligned(4))) char ipBytes[4] = {0};
    uint32_t i;

    for (i = 0; i < 4; i++, cp++) {
        ipBytes[i] = (char) __atol_ipv4(cp);
        if (!(cp = strchr(cp, '.'))) {
            break;
        }
    }

    return *(uint32_t *) ipBytes;
}


void klStr_dec2bcd_nl(uint64_t dec, uint8_t *buf, uint8_t *num) {
    uint8_t flag = 1; 
    uint8_t _num = 0;
    
    if (num == NULL || *num == 0) {
        uint64_t temp = dec;
        do {
            _num++;
            temp /= 10;
        } while (temp);
    }
    if (num != NULL) {
        *num = _num;
    }

    if (buf != NULL) {
        for (int8_t i = _num; i > 0; i--) {
            if (flag) {
                buf[(i - 1) / 2] = (dec % 10) & 0x0F;
                flag = 0;
            } else {
                buf[(i - 1) / 2] |= ((dec % 10) & 0x0F) << 4;
                flag = 1;
            }
            dec /= 10;
        }
    }
}

/*@}*/
