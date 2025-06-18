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
 
#ifndef _KSPI_SOC_H_
#define _KSPI_SOC_H_

#include <stdint.h>
#include <driver/spi_master.h>

#include "emmk-config.h"

#include "kdgpio.h"

/**
 * @addtogroup Define
 * @note none
 */
 
/*@{*/

#define KSPI_FLAG_TRANSMIT_COMPLETE     0x00000001u

/*@}*/

/**
 * @addtogroup type defin
 * @note none
 */
 
/*@{*/

typedef struct kdspi kdspi_t;

typedef struct {
    int8_t initRefs;
    int8_t initRefsPower;
    
    void *mutex;
    
    spi_device_handle_t handle;
} kdspi_Va_t;

struct kdspi {
    kdspi_Va_t *_va;
    
    struct {
        spi_host_device_t host;
        spi_bus_config_t *busConfig;
        spi_device_interface_config_t *ifConfig;
    } _config;
};

/*@}*/

/**
 * @addtogroup Instance define
 * @note none
 */
 
/*@{*/

#define _KDSPI_NEXTERN(x)     extern const kdspi_t __kdspi_##x
#define _KDSPI_NGET(x)        __kdspi_##x
#define KDSPI_NEXTERN(x)     _KDSPI_NEXTERN(x)
#define KDSPI_NGET(x)        _KDSPI_NGET(x)

#define _KDSPI(x)                   __kdspi_##x
#define _KDSPI_INAME(x)             __kdspi_##x
#define _KDSPI_IVA(x)               __kdspi_va_##x
#define _KDSPI_CONFIG_BUS_NAME(x)   __kdspi_config_bus_##x
#define _KDSPI_CONFIG_IF_NAME(x)    __kdspi_config_if_##x

extern int32_t kdspi_init(kdspi_t *kd); \
extern int32_t kdspi_finalize(kdspi_t *kd); \
extern int32_t kdspi_powerUp(kdspi_t *kd); \
extern int32_t kdspi_powerDown(kdspi_t *kd); \
extern void kdspi_select(kdspi_t *kd); \
extern void kdspi_unselect(kdspi_t *kd); \
extern int32_t kdspi_transmit(kdspi_t *kd, uint8_t *wbuf, uint8_t *rbuf, uint16_t len, uint32_t timeout); \
extern int32_t kdspi_sendData(kdspi_t *kd, uint8_t *data, uint16_t len, uint32_t timeout); \
extern int32_t kdspi_recvData(kdspi_t *kd, uint8_t *data, uint16_t len, uint32_t timeout); \
extern void kdspi_setBaudRate(kdspi_t *kd, uint32_t br);

#define KDSPI_CONFIG_BUS(_moduleNumber) static const spi_bus_config_t _KDSPI_CONFIG_BUS_NAME(_moduleNumber)

#define KDSPI_CONFIG_IF(_moduleNumber) static const spi_device_interface_config_t _KDSPI_CONFIG_IF_NAME(_moduleNumber)

#define KDSPI_DEFINE(_moduleNumber, _name) \
    static kdspi_VA_t _KDSPI_VA(_moduleNumber) = { \
    }; \
    __KDSPI_SECTION const kdspi_t _KDSPI_INAME(_name) = { \
        ._va = &_KDSPI_IVA(_moduleNumber), \
        ._config = {                       \
            .host = _moduleNumber,                               \
            .busConfig = (spi_bus_config_t *) &_KDSPI_CONFIG_BUS_NAME(_moduleNumber), \
            .ifConfig = (spi_device_interface_config_t *) &_KDSPI_CONFIG_IF_NAME(_moduleNumber), \
        }, \
    };
    
#define KDSPI_TRANSMIT_TIMEOUT_BLOCK   ((uint32_t) -1)
#define KDSPI_TRANSMIT_TIMEOUT_NOBLOCK ((uint32_t) 0)
#define KDSPI_TRANSMIT_TIMEOUT_WAIT(x) ((uint32_t) x)
    
/*@}*/

/**
 * @addtogroup Instance support functions
 * @note none
 */
 
/*@{*/

extern void *kdspi_getInstance(const char *path);

/*@}*/

/**
 * @addtogroup Public functions 
 * @note none
 */
 
/*@{*/

/*@}*/

#endif
