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

#include "./mb_misc.h"

/**
 * @addtogroup DebugSupport
 * @note none
 */

/*@{*/

#undef TAG
#define TAG     "modbus-misc"

/*@}*/

/**
 * http://www.freemodbus.org/api/group__modbus__utils.html#ga1
 */
 
/**
 * @addtogroup Define
 * @note none
 */

/*@{*/

#define BITS_UINT8    8

/*@}*/

/**
 * @addtogroup Private func
 * @note none
 */
 
/*@{*/

static inline uint16_t _bswap16(uint16_t x) {
    return (x >> 8) | (x << 8);
}


static inline uint32_t _bswap32(uint32_t x) {
    return (_bswap16(x & 0xffff) << 16) | (_bswap16(x >> 16));
}

/*@}*/

/**
 * @addtogroup Bit utils
 */

/*@{*/

/**
 * 设置一个字节缓冲的位
 * @note 这个函数可以实现有效处理位域的功能;存储位域的数值必须是两个字节;一次操作最多可处理8位
 *
 * @param *dataArea, 位存储的缓冲区: 必须是2个字节的倍数
 * @param bitOffset, 位设置的起始地址: 第一个位的偏移为0
 * @param nBits, 需要修改的位的数量: 该值必须小于8 串口方法集合
 * @param value, 位的新值: 在bitOffset中的第一位的值是newValue的最低有效位
 *
 * @return none 
 */
void mbMisc_SetBits(uint8_t *data_area, uint16_t bit_offset, uint8_t n_bits, uint8_t value) {
    uint16_t word_buf;
    uint16_t mask;
    uint16_t byte_offset;
    uint16_t n_pre_bits;
    uint16_t new_value = value;

    /**
     * Calculate byte offset for first byte containing the bit values starting at usBitOffset.
     */
    byte_offset = (uint16_t) ((bit_offset) / BITS_UINT8);

    // How many bits precede our bits to set.
    n_pre_bits = (uint16_t) (bit_offset - byte_offset * BITS_UINT8);

    // Move bit field into position over bits to set 
    new_value <<= n_pre_bits;

    // Prepare a mask for setting the new bits.
    mask = (uint16_t) ((1 << (uint16_t) n_bits) - 1);
    mask <<= bit_offset - byte_offset * BITS_UINT8;

    // Copy bits into temporary storage.
    word_buf = data_area[byte_offset];
    word_buf |= data_area[byte_offset + 1] << BITS_UINT8;

    // Zero out bit field bits and then or value bits into them.
    word_buf = (uint16_t) ((word_buf & (~mask)) | new_value);

    // Move bits back into storage
    data_area[byte_offset] = (uint8_t) (word_buf & 0xFF);
    data_area[byte_offset + 1] = (uint8_t) (word_buf >> BITS_UINT8);
}


/**
 * 在字节缓冲中读取位
 * @note 这个函数从一个字节中来获取一个位的值;一步最多可以获取8个位的值
 *
 * @param *dataArea, 位存储的缓冲区: 必须是2个字节的倍数
 * @param bitOffset, 位设置的起始地址: 第一个位的偏移为0
 * @param nBits, 需要读取的位的数量: 该值必须小于8
 *
 * @return uint8_t, 该位状态
 */
uint8_t mbMisc_getBits(uint8_t *data_area, uint16_t bit_offset, uint8_t n_bits) {
    uint16_t word_buf;
    uint16_t mask;
    uint16_t byte_offset;
    uint16_t n_pre_bits;

    // Calculate byte offset for first byte containing the bit values starting at usBitOffset 
    byte_offset = (uint16_t) ((bit_offset) / BITS_UINT8);

    // How many bits precede our bits to set 
    n_pre_bits = (uint16_t) (bit_offset - byte_offset * BITS_UINT8);

    // Prepare a mask for setting the new bits 
    mask = (uint16_t) ((1 << (uint16_t) n_bits) - 1);

    // Copy bits into temporary storage 
    word_buf = data_area[byte_offset];
    word_buf |= data_area[byte_offset + 1] << BITS_UINT8;

    // Throw away unneeded bits 
    word_buf >>= n_pre_bits;

    // Mask away bits above the requested bitfield 
    word_buf &= mask;

    return (uint8_t) word_buf;
}

/*@}*/

/**
 * @addtogroup Byte <--> Bits
 * @note none
 */
 
/*@{*/

void mbMisc_setBitsFromByte(uint8_t *dest, int32_t idx, const uint8_t value) {
    for (int32_t i = 0; i < 8; i++) {
        dest[idx+i] = (value & (1 << i)) ? 1 : 0;
    }
}


void mbMisc_setBitsFromBytes(uint8_t *dest, int32_t idx, uint32_t bitsCount, const uint8_t *tabByte) {
    int32_t shift = 0;

    for (int32_t i = idx; i < idx + bitsCount; i++) {
        dest[i] = tabByte[(i - idx) / 8] & (1 << shift) ? 1 : 0;
        //// gcc doesn't like: shift = (++shift) % 8;
        shift++;
        shift %= 8;
    }
}


uint8_t mbMisc_getBitsFromBytes(const uint8_t *src, int32_t idx, uint32_t bitsCount) {
    uint8_t value = 0;

    if (bitsCount > 8) {
        ASSERT(bitsCount < 8);
        bitsCount = 8;
    }

    for (uint32_t i = 0; i < bitsCount; i++) {
        value |= (src[idx+i] << i);
    }

    return value;
}

/*@}*/

/**
 * @addtogroup Float <--> uint16
 * @note none
 */
 
/*@{*/

void mbMisc_setFloatABCD(float f, uint16_t *dest) {
    uint32_t i;

    memcpy(&i, &f, sizeof(uint32_t));
    i = klhtonl(i);
    dest[0] = (uint16_t) (i >> 16);
    dest[1] = (uint16_t) i;
}


void mbMisc_setFloatDCBA(float f, uint16_t *dest) {
    uint32_t i;

    memcpy(&i, &f, sizeof(uint32_t));
    i = _bswap32(klhtonl(i));
    dest[0] = (uint16_t) (i >> 16);
    dest[1] = (uint16_t) i;
}


void mbMisc_setFloatBADC(float f, uint16_t *dest) {
    uint32_t i;

    memcpy(&i, &f, sizeof(uint32_t));
    i = klhtonl(i);
    dest[0] = (uint16_t) _bswap16(i >> 16);
    dest[1] = (uint16_t) _bswap16(i & 0xFFFF);
}


void mbMisc_setFloatCDAB(float f, uint16_t *dest) {
    uint32_t i;

    memcpy(&i, &f, sizeof(uint32_t));
    i = klhtonl(i);
    dest[0] = (uint16_t) i;
    dest[1] = (uint16_t) (i >> 16);
}


float mbMisc_getFloatABCD(const uint16_t *src) {
    float f;
    uint32_t i;

    i = klntohl(((uint32_t) src[0] << 16) + src[1]);
    memcpy(&f, &i, sizeof(float));

    return f;
}


float mbMisc_getFloatDCBA(const uint16_t *src) {
    float f;
    uint32_t i;

    i = klntohl(_bswap32((((uint32_t) src[0]) << 16) + src[1]));
    memcpy(&f, &i, sizeof(float));

    return f;
}


float mbMisc_getFloatBADC(const uint16_t *src) {
    float f;
    uint32_t i;

    i = klntohl((uint32_t) (_bswap16(src[0]) << 16) + _bswap16(src[1]));
    memcpy(&f, &i, sizeof(float));

    return f;
}


float mbMisc_getFloatCDAB(const uint16_t *src) {
    float f;
    uint32_t i;

    i = klntohl((((uint32_t) src[1]) << 16) + src[0]);
    memcpy(&f, &i, sizeof(float));

    return f;
}

/*@}*/

/**
 * @addtogroup DebugSupport
 * @note none
 */

/*@{*/

#undef TAG

/*@}*/