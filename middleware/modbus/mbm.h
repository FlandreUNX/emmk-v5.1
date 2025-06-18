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

#ifndef _MODBUS_MASTER_H_
#define _MODBUS_MASTER_H_

#include <stdint.h>
#include <stdlib.h>

#include "./mb_def.h"
#include "./mb_misc.h"

/**
 * @addtogroup Configure
 * @note none
 */

/*@{*/

#define MBM_ENABLE                       1
#define MBM_MAX_WAIT_TIMEOUT             10000

#define MBM_ENABLE_FUNC_COILS_REG        0
#define MBM_ENABLE_FUNC_HOLDING_REG      1
#define MBM_ENABLE_FUNC_INPUT_REG        1

#define MBM_REQUEST_HANDLER_QUEUE_MAX    4

/*@}*/

/**
 * @addtogroup Define
 * @note none
 */

/*@{*/

#define MBM_REQUEST_BLOCK(timeout)    (0x80000000 | timeout)
#define MBM_REQUEST_NOBLOCK(timeout)  (timeout)

/*@}*/

/**
 * @addtogroup Modbus func - inter func
 * @note none
 */

/*@{*/

extern void __mbm_setDest(MBM_t *d, uint8_t dest);
extern uint8_t *__mbm_getTxPDU(MBM_t *d);
extern void __mbm_setTxPDULen(MBM_t *d, uint8_t len);
extern uint8_t *__mbm_getRxPDU(MBM_t *d);

/*@}*/

/**
 * @addtogroup preprocess func
 * @note none
 */

/*@{*/

////
//// Instance
////
#define MBM_INSTANCE_DEFINE(aname, \
    _txBuffer, _txBufferSize, \
    _rxBuffer, _rxBufferSize) \
    MBM_t gMbmInstance_##aname = { \
        .txBuf = _txBuffer, \
        .txBufSize = _txBufferSize, \
        .rxBufSize = _rxBufferSize, \
        .rxBuf = _rxBuffer, \
    };

#define MBM_INSTANCE_EXTERN(aname) \
    extern MBM_t gMbmInstance_##aname;

#define MBM_INSTANCE_LOCAL_GET(aname) \
    &gMbmInstance_##aname

#define MBM_INSTANCE_LOCAL_GET2(aname) gMbmInstance_##aname


////
//// Modbus master functions
////
#if defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6100100)
#define MBM_FUNC_SECTION             __attribute__((used, section("SECTION_MBM_FUNC")))
#define MBM_FUNC_SECTION_START       ((uint32_t) &SECTION_MBM_FUNC$$Base)
#define MBM_FUNC_SECTION_END         ((uint32_t) &SECTION_MBM_FUNC$$Limit)
#define MBM_FUNC_SECTION_EXTEND \
    extern uint32_t SECTION_MBM_FUNC$$Base; \
    extern uint32_t SECTION_MBM_FUNC$$Limit;
#elif defined(__GNUC__RV32_CHX)
#define MBM_FUNC_SECTION     KCOMPILER_SECTION("SECTION_MBM_FUNC")
#define MBM_FUNC_SECTION_START       ((uint32_t) &KCOMPILER_SECTION_START(SECTION_MBM_FUNC))
#define MBM_FUNC_SECTION_END         ((uint32_t) &KCOMPILER_SECTION_END(SECTION_MBM_FUNC))
#define MBM_FUNC_SECTION_EXTEND \
extern uint32_t KCOMPILER_SECTION_START(SECTION_MBM_FUNC); \
extern uint32_t KCOMPILER_SECTION_END(SECTION_MBM_FUNC);
#elif defined (__GNUC__)
#define MBM_FUNC_SECTION     KCOMPILER_SECTION(".SECTION_MBM_FUNC")
#define MBM_FUNC_SECTION_START       ((uint32_t) &KCOMPILER_SECTION_START(_SECTION_MBM_FUNC))
#define MBM_FUNC_SECTION_END         ((uint32_t) &KCOMPILER_SECTION_END(_SECTION_MBM_FUNC))
#define MBM_FUNC_SECTION_EXTEND \
extern uint32_t KCOMPILER_SECTION_START(_SECTION_MBM_FUNC); \
extern uint32_t KCOMPILER_SECTION_END(_SECTION_MBM_FUNC);
#endif

#define MBM_FUNC_DEFINE(name, code, pre, last) \
    MBM_FUNC_SECTION const MBM_FuncHandler_t _mbmFunc_X##name \
         = { \
            .funcCode = code, \
            .preHandler = pre, \
            .lastHandler = last \
        }
/*@}*/

/**
 * @addtogroup func
 * @note none
 */

/*@{*/

extern void mbm_init(MBM_t *instance, MB_Interface_t *interface, void *userData);
extern void mbm_finalize(MBM_t *instance);

extern MB_Exception_t mbm_input(MBM_t *instance, const uint8_t *data, uint16_t dataLength,
                             MBM_ResponseHandler_t *rep);
extern MBM_Error_t mbm_create(MBM_t *instance, MBM_RequestHandler_t *req, uint8_t *data, uint16_t *dataLength);
extern MBM_Error_t mbm_request(MBM_t *instance,
                               MBM_RequestHandler_t *req, MBM_ResponseHandler_t *rep,
                               uint32_t timeout,
                               uint8_t isSyncRecv);

extern uint8_t mbm_isBusy(MBM_t *instance);
extern uint8_t mbm_isOperateCompleted(MBM_t *instance);

/*@}*/

#endif
