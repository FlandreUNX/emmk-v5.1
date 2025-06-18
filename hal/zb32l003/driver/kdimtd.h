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
 
#ifndef _KIMTD_SOC_H_
#define _KIMTD_SOC_H_

#include <stdint.h>

#ifdef CONFIG_USE_ZB32L003
#include "zb32l003.h"
#endif
#ifdef CONFIG_USE_ZB32L030
#include "zb32l030.h"
#endif

/**
 * @addtogroup Define
 * @note none
 */
 
/*@{*/

/*@}*/

/**
 * @addtogroup type defin
 * @note none
 */
 
/*@{*/

typedef struct kdimtd kdimtd_t;

typedef struct {
    void *mutex;
    
    int8_t initRefs;
    int8_t initRefsPower;
} kdimtd_VA_t;

struct kdimtd {
//    kdimtd_VA_t *_va;
    
    struct {
        uint32_t addressStart;
        uint32_t size;
        uint16_t blockSize;
        uint16_t pageSize;
    } _instance;
};

/*@}*/

/**
 * @addtogroup Instance define
 * @note none
 */
 
/*@{*/

#define _KDIMTD_NGET(x)       __kdimtd_##x
#define _KDIMTD_NEXTERN(x)    extern const kdimtd_t __kdimtd_##x
#define KDIMTD_NGET(x)        _KDIMTD_NGET(x)
#define KDIMTD_NEXTERN(x)     _KDIMTD_NEXTERN(x)

#define KDIMTD_ADDR_START(x) x
#define KDIMTD_ADDR_OFFSET(x) x
#define KDIMTD_SIZE(x) x
#define KDIMTD_INTERFACE(x) x
#define KDIMTD_BLOCK_SIZE(x) x
#define KDIMTD_PAGE_SIZE(x) x

#define _KDIMTD(x)           __kdimtd_##x
#define _KDIMTD_INAME(x)     __kdimtd_##x
#define _KDIMTD_IVA(x)       __kdimtd_va_##x

extern int32_t kdimtd_init(kdimtd_t *kd);
extern int32_t kdimtd_finalize(kdimtd_t *kd);
extern int32_t kdimtd_powerUp(kdimtd_t *kd);
extern int32_t kdimtd_powerDown(kdimtd_t *kd);
extern int32_t kdimtd_reads(kdimtd_t *kd, uint32_t address, uint8_t *data, uint32_t readSize, uint32_t timeout);
extern int32_t kdimtd_earse(kdimtd_t *kd, uint32_t sectorAddress, uint32_t size, uint32_t timeout);
extern int32_t kdimtd_writes(kdimtd_t *kd, uint32_t address, uint8_t *data, uint32_t size, uint32_t timeout);
extern uint32_t kdimtd_getAbsAddr(kdimtd_t *kd, uint32_t offset);

#define KDIMTD_DEFINE(_name, \
        _blockSize, _pageSize, \
        _start, _size) \
    const kdimtd_t _KDIMTD_INAME(_name) = { \
        ._instance = { \
            .addressStart = _start, \
            .size = _size, \
            .blockSize = _blockSize, \
            .pageSize = _pageSize, \
        }, \
    };

/*@}*/

/**
 * @addtogroup Instance support functions
 * @note none
 */
 
/*@{*/

extern void *kdimtd_getInstance(const char *path);

/*@}*/

/**
 * @addtogroup Public functions 
 * @note none
 */
 
/*@{*/

/*@}*/

#endif
