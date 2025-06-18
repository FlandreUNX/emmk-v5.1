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

#ifndef _MODBUS_SALVE_H_
#define _MODBUS_SALVE_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "./mb_def.h"
#include "./mb_misc.h"

/**
 * @addtogroup Configure
 * @note none
 */
 
/*@{*/

#define MBS_ENABLE                    1
#define MBS_ENABLE_FUNC_COILS_REG     0
#define MBS_ENABLE_FUNC_HOLDING_REG   1
#define MBS_ENABLE_FUNC_INPUT_REG     0

/*@}*/

/**
 * @addtogroup Protect func
 * @note none
 */
 
/*@{*/

extern MBS_Register_t* _mbs_findHandlerByTypeAndAddress(MBS_RegisterType_t type, uint16_t address);

/*@}*/

/**
 * @addtogroup Register define macro
 * @note none
 */

/*@{*/

////
//// Instance
////
#define MBS_INSTANCE_DEFINE(aname, \
    _txBuffer, _txBufferSize, \
    _rxBuffer, _rxBufferSize) \
    MBS_t gMbsInstance_##aname = { \
        .txBuffer = _txBuffer, \
        .txBufferSize = _txBufferSize, \
        .rxBufferSize = _rxBufferSize, \
        .rxBuffer = _rxBuffer, \
    };
    
#define MBS_INSTANCE_EXTERN(aname) \
    extern MBS_t gMbsInstance_##aname;
    
#define MBS_INSTANCE_LOCAL_GET(aname) &gMbsInstance_##aname

#define MBS_INSTANCE_LOCAL_GET2(aname) gMbsInstance_##aname

////
//// Modbus slave functions
////
#if defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6100100)
#define MBS_FUNC_SECTION             __attribute__((used, section("SECTION_MBS_FUNC")))
#define MBS_FUNC_SECTION_START       ((uint32_t) &SECTION_MBS_FUNC$$Base)
#define MBS_FUNC_SECTION_END         ((uint32_t) &SECTION_MBS_FUNC$$Limit)
#define MBS_FUNC_SECTION_EXTEND \
extern uint32_t SECTION_MBS_FUNC$$Base; \
extern uint32_t SECTION_MBS_FUNC$$Limit;
#elif defined(__GNUC__RV32_CHX)
#define MBS_FUNC_SECTION     KCOMPILER_SECTION("SECTION_MBS_FUNC")
#define MBS_FUNC_SECTION_START       ((uint32_t) &KCOMPILER_SECTION_START(SECTION_MBS_FUNC))
#define MBS_FUNC_SECTION_END         ((uint32_t) &KCOMPILER_SECTION_END(SECTION_MBS_FUNC))
#define MBS_FUNC_SECTION_EXTEND \
extern uint32_t KCOMPILER_SECTION_START(SECTION_MBS_FUNC); \
extern uint32_t KCOMPILER_SECTION_END(SECTION_MBS_FUNC);
#elif defined (__GNUC__)
#define MBS_FUNC_SECTION     KCOMPILER_SECTION(".SECTION_MBS_FUNC")
#define MBS_FUNC_SECTION_START       ((uint32_t) &KCOMPILER_SECTION_START(_SECTION_MBS_FUNC))
#define MBS_FUNC_SECTION_END         ((uint32_t) &KCOMPILER_SECTION_END(_SECTION_MBS_FUNC))
#define MBS_FUNC_SECTION_EXTEND \
extern uint32_t KCOMPILER_SECTION_START(_SECTION_MBS_FUNC); \
extern uint32_t KCOMPILER_SECTION_END(_SECTION_MBS_FUNC);
#endif

    
#define MBS_FUNC_DEFINE(_name, _funcCode, _handler) \
    MBS_FUNC_SECTION const MBS_FuncHandler_t _MBS_FUNC_HANDLER_##_name = { \
        .funcCode = _funcCode, \
        .onHandle = _handler \
    }

////
//// Modbus slave registers
////
#if defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6100100)
#define MBS_REGISTER_SECTION             __attribute__((used, section("SECTION_MBS_REGISTER")))
#define MBS_REGISTER_SECTION_START       ((uint32_t) &SECTION_MBS_REGISTER$$Base)
#define MBS_REGISTER_SECTION_END         ((uint32_t) &SECTION_MBS_REGISTER$$Limit)
#define MBS_REGISTER_SECTION_EXTEND \
extern uint32_t SECTION_MBS_REGISTER$$Base; \
extern uint32_t SECTION_MBS_REGISTER$$Limit;
#elif defined(__GNUC__RV32_CHX)
#define MBS_REGISTER_SECTION     KCOMPILER_SECTION("SECTION_MBS_REGISTER")
#define MBS_REGISTER_SECTION_START       ((uint32_t) &KCOMPILER_SECTION_START(SECTION_MBS_REGISTER))
#define MBS_REGISTER_SECTION_END         ((uint32_t) &KCOMPILER_SECTION_END(SECTION_MBS_REGISTER))
#define MBS_REGISTER_SECTION_EXTEND \
extern uint32_t KCOMPILER_SECTION_START(SECTION_MBS_REGISTER); \
extern uint32_t KCOMPILER_SECTION_END(SECTION_MBS_REGISTER);
#elif defined (__GNUC__)
#define MBS_REGISTER_SECTION     KCOMPILER_SECTION(".SECTION_MBS_REGISTER")
#define MBS_REGISTER_SECTION_START       ((uint32_t) &KCOMPILER_SECTION_START(_SECTION_MBS_REGISTER))
#define MBS_REGISTER_SECTION_END         ((uint32_t) &KCOMPILER_SECTION_END(_SECTION_MBS_REGISTER))
#define MBS_REGISTER_SECTION_EXTEND \
extern uint32_t KCOMPILER_SECTION_START(_SECTION_MBS_REGISTER); \
extern uint32_t KCOMPILER_SECTION_END(_SECTION_MBS_REGISTER);
#endif


    
#define MBS_REGSITERS_MAP_DEFINE(_name, _size) \
    static uint16_t AT_NONCACHEABLE_SECTION_ALIGN_INIT(_MBS_REGISTER_##_name##_MAP[_size], 4) = {0}; \
    
#define MBS_REGSITERS_MAP(_name) \
    _MBS_REGISTER_##_name##_MAP
    
#define MBS_REGISTER_HANDLER_NONE   MBS_REGISTER_SECTION const uint8_t _MBS_REGISTER_NONE;
 
#define MBS_REGISTER_HANDLER_DEFINE(_name, _type, \
        _mapName, _address, \
        _userData) \
    static MBS_RegisterHandlerVa_t _MBS_REGISTER_##_name##_va = { \
        .onWriteAuth = NULL, \
        .onRequested = NULL, \
    }; \
    MBS_REGISTER_SECTION const MBS_Register_t _MBS_REGISTER_##_name = { \
        .type = _type, \
        .address = _address, \
        .data = &_mapName[_address], \
        .userData = _userData, \
        .va = &_MBS_REGISTER_##_name##_va, \
    }
    
/*@}*/
    
/**
 * @addtogroup Public func
 * @note none
 */
 
/*@{*/
    
extern void mbs_init(MBS_t *instance, 
        MB_Interface_t *interface, 
        MB_Callback_t *callback,
        uint16_t *holdRegMap, uint16_t holdRegMapSize,
        uint16_t *inputRegMap, uint16_t inputRegMapSize,
        uint8_t devAddr, uint32_t baudrat);
extern void mbs_finalize(MBS_t *instance);
extern void mbs_setMode(MBS_t *instance, MBS_Mode_t mode);
extern int32_t mbs_updateDeviceAddress(MBS_t *instance, uint8_t newAddr);
extern int32_t mbs_updateBaudRate(MBS_t *instance, uint32_t baudRate);

extern int32_t mbs_inputOutput(MBS_t *instance, uint8_t *pduData, uint16_t *pduLength, bool skipCRC, bool skipWrite);
extern void mbs_loop(MBS_t *instance);
    
extern int16_t mbs_setRegisterData(MBS_t *instance, uint16_t startAddress, uint16_t *data, uint16_t length);
extern int16_t mbs_getRegisterData(MBS_t *instance, uint16_t startAddress, uint16_t *data, uint16_t length);

extern void mbs_setRegisterOnRequested(uint16_t address, MBS_ResigsterOnRequested_t onRequested);
extern void mbs_setRegisterOnWriteAuth(uint16_t address, MBS_ResigsterOnAuth_t onWriteAuth);
extern void mbs_setRegisterOnValueCheckAuth(uint16_t address, MBS_ResigsterOnAuth_t onValueCheckAuth);

/*@}*/
      
#endif
