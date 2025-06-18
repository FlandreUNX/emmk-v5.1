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
 
#ifndef _KDSTTY_H_
#define _KDSTTY_H_

#include <stdint.h>

#include "cw32l010.h"

#include "kdgpio.h"
#include "emmk-config.h"

/**
 * @addtogroup Typedef
 * @note none
 */
 
/*@{*/

typedef struct kdstty_Host kdstty_Host_t;
typedef struct kdSTTY kdstty_t;

typedef enum {
    KDSTTY_RXPIN_IRQ_DISABLE = 0,
    KDSTTY_RXPIN_IRQ_ENABLE = 1
} kdstty_RxPinIrqEnable_t;

typedef enum {
    KDSTTY_PIN_REVERSE_DISABLE = 0,
    KDSTTY_PIN_REVERSE_ENABLE = 1
} kdstty_PinReverse_t;

typedef enum {
    KDSTTY_PARITY_NONE = 0,
    KDSTTY_PARITY_ODD = 1,
    KDSTTY_PARITY_EVEN = 2,
} kdstty_Parity_t;

typedef struct {
    kdgpio_t *gpio;
    kdstty_PinReverse_t reverse: 1;
} kdstty_Pin_t;

typedef struct {
    struct {
        struct {
            uint8_t bitCount;
            uint8_t bitShift;
            
            uint8_t parityCount;
            uint8_t launchBit;
            
            uint16_t bufferIndex;
            uint16_t bufferDataLen;
            
            uint8_t isBusy: 1;
            uint8_t isEnable: 1;
        } tx;
        struct {
            uint8_t bitCount;
            uint8_t bitShift;
            
            uint8_t launchData;
            
            uint8_t launchBitMiddleTiming;
            uint8_t isLaunchBitMiddleTimingSet;
            
            uint8_t isBusy: 1;
            uint8_t isWaitIdle: 1;
            
            uint8_t isCompleted: 1;
        } rx;
    } state;

    int8_t initRefs;
    int8_t initRefsPower;
} kdstty_Va_t;

typedef struct {
    int8_t initRefs;
    int8_t initRefsPower;
    __IO uint8_t timingDiv;
} kdstty_HostVa_t;

struct kdstty_Host {
    kdstty_HostVa_t *_va;

    struct {
        struct {
            BTIM_TypeDef *tim;
        } tim;
        
        uint32_t baudrate;
    } _config;
    
    struct {
        void (*baseEnable)(kdstty_Host_t *kd);
        void (*baseDisable)(kdstty_Host_t *kd);
    } _base;
};

struct kdSTTY {
    kdstty_Va_t *_va;
    kdstty_Host_t *_host;
    
    struct {
        struct {
            kdstty_Pin_t tx;
            kdstty_Pin_t rx;
        } pin;
        
        struct {
            uint8_t dataLen;
            uint8_t stopBit;
            kdstty_Parity_t parity;
            uint8_t idefC1Len;
            uint8_t idefC2Len;
            
            uint16_t rxIdleCount;
            
            kdstty_RxPinIrqEnable_t rxPinIrqMode;
        } uart;
    } _config;

    struct {
        struct {
            uint8_t *pool;
            uint16_t poolSize;
        } tx;
        struct {
            uint8_t *pool;
            uint16_t poolSize;
            qBSBuffer_t *buffer;
        } rx;
    } _buffer;
};

/*@}*/

/**
 * @addtogroup Instance define
 * @note none
 */
 
/*@{*/

#define _KDSTTY_NGET(x)         __kdstty_##x
#define _KDSTTY_NEXTERN(x)      extern const kdstty_t __kdstty_##x
#define KDSTTY_NGET(x)          _KDSTTY_NGET(x)
#define KDSTTY_NEXTERN(x)       _KDSTTY_NEXTERN(x)

extern int32_t kdstty_init(kdstty_t *kd); \
extern int32_t kdstty_finalize(kdstty_t *kd); \
extern int32_t kdstty_powerUp(kdstty_t *kd); \
extern int32_t kdstty_powerDown(kdstty_t *kd); \
extern int32_t kdstty_sends(kdstty_t *kd, const void *data, uint32_t size, uint32_t timeout); \
extern int32_t kdstty_recvs(kdstty_t *kd, void *data, uint32_t expect_size, uint32_t *recv_size, uint32_t timeout); \
extern int32_t kdstty_flush(kdstty_t *kd); \
extern int32_t kdstty_hasRecvData(kdstty_t *kd); \
extern int32_t kdstty_isSendIdle(kdstty_t *kd); \
extern void kdstty_updateBaudRate(kdstty_t *kd, uint32_t bd); \
    
#define _KDSTTY_HOST_VA(x)              __kdstty_host_va_##x
#define _KDSTTY_HOST_FUNC_ENABLE(x)     __kdstty_host_enable_##x
#define _KDSTTY_HOST_FUNC_DISABLE(x)    __kdstty_host_disable_##x
#define _KDSTTY_HOST_INAME(x)           __kdstty_host_##x
    
#define KDSTTY_TIM_MODULE(_number)      _number
#define _KDSTTY_TIM_MODULE(_number)     CW_BTIM##_number

#define KDSTTY_BAUDRATE(x)      x
#define KDSTTY_TX_BUFFER(x)     x
#define KDSTTY_RX_BUFFER(x)     x
#define KDSTTY_TIMEOUT_CHAR(x)  x
#define KDSTTY_DATALEN(x)       x
#define KDSTTY_STOPBIT(x)       x
    
#define _KDSTTY_INAME(x)        __kdstty_##x
#define _KDSTTY_VA(x)           __kdstty_va_##x
#define _KDSTTY_TX_BUFFER(x)    __kdstty_tx_buffer_##x
#define _KDSTTY_RX_BUFFER(x)    __kdstty_rx_buffer_##x
#define _KDSTTY_RX_FIFO(x)      __kdstty_rx_lwrb_##x
    
#define KDSTTY_HOST_DEV_LIST(_name)         const kdstty_t * const __kdstty_hostDevList_##_name[]
    
#define KDSTTY_PINRX_IRQ_NGET(_name)        __kdstty_pinRxIrq_##_name
#define KDSTTY_PINRX_IRQ_NEXTERN(_name)     extern void __kdstty_pinRxIrq_##_name(void)

#define KDSTTY_HOST_TIM_DEFINE(_name, _tim, _baudrate, _enableFunc, _disableFunc, _irqITM) \
    static void _KDSTTY_HOST_FUNC_ENABLE(_name)(kdstty_Host_t *kd) _enableFunc \
    static void _KDSTTY_HOST_FUNC_DISABLE(_name)(kdstty_Host_t *kd) _disableFunc \
    static kdstty_HostVa_t _KDSTTY_HOST_VA(_name) = {0}; \
    const kdstty_Host_t _KDSTTY_HOST_INAME(_name) = { \
        ._va = &_KDSTTY_HOST_VA(_name), \
        ._config = { \
            .baudrate = _baudrate, \
            .tim = { \
                .tim = _KDSTTY_TIM_MODULE(_tim), \
            }, \
        }, \
        ._base = { \
            .baseEnable = _KDSTTY_HOST_FUNC_ENABLE(_name), \
            .baseDisable = _KDSTTY_HOST_FUNC_DISABLE(_name), \
        }, \
    }; \
    void _irqITM(void) { \
        _KDSTTY_TIM_MODULE(_tim)->ICR &= ~0x01; \
        extern const kdstty_t * const __kdstty_hostDevList_##_name[]; \
        extern void kdstty_timingHandler(kdstty_Host_t *host, void *devList); \
        kdstty_timingHandler((kdstty_Host_t *) &_KDSTTY_HOST_INAME(_name), (void *) __kdstty_hostDevList_##_name); \
    }
    
#define KDSTTY_TX_DEFINE(_gpio, _reverse) \
    .tx = { \
        .gpio = ((kdgpio_t *) _gpio), \
        .reverse = _reverse, \
    }
    
#define KDSTTY_RX_DEFINE(_gpio, _reverse) \
    .rx = { \
        .gpio = ((kdgpio_t *) _gpio), \
        .reverse = _reverse, \
    }
    
#define KDSTTY_IF_DEFINE(_name, _hostName, \
        _tx, _rx, \
        _timeoutTime, \
        _txBufferSize, _rxBufferSize, \
        _dataLen, _parity, _stopBit, \
        _rxPinIrqMode \
        ) \
    static uint8_t AT_NONCACHEABLE_SECTION_ALIGN(_KDSTTY_TX_BUFFER(_name)[_txBufferSize], 4); \
    static uint8_t AT_NONCACHEABLE_SECTION_ALIGN(_KDSTTY_RX_BUFFER(_name)[_rxBufferSize], 4); \
    static qBSBuffer_t _KDSTTY_RX_FIFO(_name); \
    static kdstty_Va_t _KDSTTY_VA(_name) = {0}; \
    const kdstty_t _KDSTTY_INAME(_name) = { \
        ._va = &_KDSTTY_VA(_name), \
        ._host = (kdstty_Host_t *) &_KDSTTY_HOST_INAME(_hostName), \
        ._config = { \
            .uart = { \
                .dataLen = _dataLen, \
                .stopBit = _stopBit, \
                .parity = _parity, \
                .idefC1Len = _parity != 0 ? _dataLen + 2 : _dataLen + 1, \
                .idefC2Len = _stopBit + (_parity != 0 ? _dataLen + 2 : _dataLen + 1), \
                .rxIdleCount = _timeoutTime, \
                .rxPinIrqMode = _rxPinIrqMode, \
            }, \
            .pin = { \
                _tx, _rx, \
            }, \
        }, \
        ._buffer = { \
            .rx = { \
                .pool = _KDSTTY_RX_BUFFER(_name), \
                .poolSize = _rxBufferSize, \
                .buffer = &_KDSTTY_RX_FIFO(_name), \
            }, \
            .tx = { \
                .pool = _KDSTTY_TX_BUFFER(_name), \
                .poolSize = _txBufferSize, \
            }, \
        }, \
    }; \
    void KDSTTY_PINRX_IRQ_NGET(_name)(void) { \
        if (_KDSTTY_INAME(_name)._config.pin.rx.gpio->MSKINTSR & _KDSTTY_INAME(_name)._config.pin.rx.pinSet) { \
            _KDSTTY_INAME(_name)._config.pin.rx.gpio->INTCLR |= _KDSTTY_INAME(_name)._config.pin.rx.pinSet; \
            extern void kdstty_pinRxIrqHandler(kdstty_t *kd); \
            kdstty_pinRxIrqHandler((kdstty_t *) &_KDSTTY_INAME(_name)); \
        } \
    } \
    
    
#define KDSTTY_IF_ONLYTX_DEFINE(_name, _hostName, \
        _tx, _rx, \
        _timeoutTime, \
        _txBufferSize, _rxBufferSize, \
        _dataLen, _parity, _stopBit, \
        _rxPinIrqMode \
        ) \
    static uint8_t AT_NONCACHEABLE_SECTION_ALIGN(_KDSTTY_TX_BUFFER(_name)[_txBufferSize], 4); \
    static kdstty_Va_t _KDSTTY_VA(_name) = {0}; \
    const kdstty_t _KDSTTY_INAME(_name) = { \
        ._va = &_KDSTTY_VA(_name), \
        ._host = (kdstty_Host_t *) &_KDSTTY_HOST_INAME(_hostName), \
        ._config = { \
            .uart = { \
                .dataLen = _dataLen, \
                .stopBit = _stopBit, \
                .parity = _parity, \
                .idefC1Len = _parity != 0 ? _dataLen + 2 : _dataLen + 1, \
                .idefC2Len = _stopBit + (_parity != 0 ? _dataLen + 2 : _dataLen + 1), \
                .rxIdleCount = _timeoutTime, \
                .rxPinIrqMode = 0, \
            }, \
            .pin = { \
                _tx, _rx, \
            }, \
        }, \
        ._buffer = { \
            .rx = { \
                .pool = NULL, \
                .poolSize = 0, \
                .buffer = NULL, \
            }, \
            .tx = { \
                .pool = _KDSTTY_TX_BUFFER(_name), \
                .poolSize = _txBufferSize, \
            }, \
        }, \
    };
    
/*@}*/

#endif
