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

#ifndef _KLIB_CRC_H_
#define _KLIB_CRC_H_

#include <stdint.h>

/**
 * @addtogroup Public functions
 * @note none
 */
 
/*@{*/

extern uint8_t klib_crc8(uint8_t *msg, uint32_t len);
extern uint8_t klib_crc8_inc(uint8_t crc8_in, uint8_t *msg, uint32_t len);
extern uint8_t kcrc8_at(uint8_t *message, uint32_t length);
extern uint8_t kcrc8_inc(uint8_t _crc8, uint8_t *message, uint32_t length);

extern uint16_t klib_crc16_f(const uint8_t *msg, uint16_t len);
extern uint16_t klib_crc16(uint8_t *msg, uint32_t len);
extern uint16_t klCRC16_modbus(const uint8_t *msg, uint16_t len);

extern uint32_t klib_crc32(uint32_t crc32, uint8_t *msg, uint32_t len);

/*@}*/

#endif
