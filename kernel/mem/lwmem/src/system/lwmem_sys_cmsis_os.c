/**
 * \file            lwmem_sys_cmsis_os.c
 * \brief           System functions for CMSIS-OS based operating system
 */

/*
 * Copyright (c) 2023 Tilen MAJERLE
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * This file is part of LwMEM - Lightweight dynamic memory manager library.
 *
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 * Version:         v2.1.0
 */
#include "../include/system/lwmem_sys.h"

#if LWMEM_CFG_OS && !__DOXYGEN__

#include "emmk-config.h"

uint8_t lwmem_sys_mutex_create(LWMEM_CFG_OS_MUTEX_HANDLE* m) {
#ifdef CMSIS_OS2_H_
    m = osMutexNew(NULL);
    return m != NULL ? 1 : 0;
#else
    return 1;
#endif
}

uint8_t lwmem_sys_mutex_isvalid(LWMEM_CFG_OS_MUTEX_HANDLE* m) {
    return 0;
}


uint8_t lwmem_sys_mutex_wait(LWMEM_CFG_OS_MUTEX_HANDLE* m) {
#ifdef CMSIS_OS2_H_
    return osMutexAcquire(m, osWaitForever) == osOK;
#else
    return true;
#endif
}

uint8_t lwmem_sys_mutex_release(LWMEM_CFG_OS_MUTEX_HANDLE* m) {
#ifdef CMSIS_OS2_H_
    return osMutexRelease(m) == osOK;
#else
    return true;
#endif
}

#endif /* LWMEM_CFG_OS && !__DOXYGEN__ */

/**
 * @addtogroup kmem func
 * @note none
 */
 
/*@{*/

int32_t kmem_init(void *begin_addr, void *end_addr) {
    lwmem_region_t regions[] = {
        { begin_addr, (uint32_t) end_addr - (uint32_t) begin_addr },
        /* Add more regions if needed */
        { NULL, 0 }
    };
    lwmem_assignmem(regions);
    return 0;
}

/*@}*/

/**
 * @addtogroup C-Memory hacking
 * @note none
 */
 
/*@{*/

#if defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6100100)

__attribute__((used)) void *$sub$$malloc(size_t s) {
    return lwmem_malloc(s);
}


__attribute__((used)) void *$sub$$calloc(size_t s1, size_t s2) {
    return lwmem_calloc(s1, s2);
}


__attribute__((used)) void $sub$$free(void *p) {
    lwmem_free(p);
}

#if EEMK_CFG_MEM_REALLOC_OVERRIDE == 1
__attribute__((used)) void *$sub$$realloc(void *p, size_t s)  {
    return lwmem_realloc(p, s);
}
#endif

#endif


#if defined (__GNUC__)

__attribute__((used)) void *__wrap_malloc(size_t s) {
    return lwmem_malloc(s);
}


__attribute__((used)) void *__wrap_calloc(size_t s1, size_t s2) {
    return lwmem_calloc(s1, s2);
}


__attribute__((used)) void __wrap_free(void *p) {
    lwmem_free(p);
}

#if EEMK_CFG_MEM_REALLOC_OVERRIDE == 1
__attribute__((used)) void *__wrap_realloc(void *p, size_t s)  {
    return kmem_realloc(p, s);
}
#endif

#endif

/*@}*/
