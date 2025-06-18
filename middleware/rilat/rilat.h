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

#ifndef RILAT_H
#define RILAT_H

#include <stdint.h>
#include "opts_rilat.h"

/**
 * @addtogroup Configure
 * @note none
 */

/*@{*/

#ifndef RILAT_TX_LOG_SUPPORT
#define RILAT_TX_LOG_SUPPORT                    (1)
#endif

#ifndef RILAT_RX_LOG_SUPPORT
#define RILAT_RX_LOG_SUPPORT                    (1)
#endif

#ifndef RILAT_DIRECT_WRITE_LOG_SUPPORT
#define RILAT_DIRECT_WRITE_LOG_SUPPORT          (1)
#endif

#ifndef RILAT_STREAM_SUPPORT
#define RILAT_STREAM_SUPPORT                    (0)
#endif

// AT指令匹配处理
#ifndef RILAT_MATCHER_ENABLE
#define RILAT_MATCHER_ENABLE                        1
#endif

// AT指令行结尾补丁处理
#ifndef RILAT_PARTIAL_LINE_PATCH_ENABLE
#define RILAT_PARTIAL_LINE_PATCH_ENABLE             1
#endif

// AT指令URC推送前的补丁处理,
//  例如:
//      +QIURC: "recv",0,3<CR><CL>ABC<CR><CL> -> +QIURC: "recv",0,3, ABC<CR><CL>
#ifndef RILAT_PARTIAL_READ_LINE_LINK_PATCH_ENABLE
#define RILAT_PARTIAL_READ_LINE_LINK_PATCH_ENABLE   1
#endif

/*@}*/

/**
 * @addtogroup Define & Typedef
 * @note none
 */

/*@{*/

#if RILAT_MATCHER_ENABLE == 1

typedef struct {
    const char *matcher;
    int32_t (*onMatch)(const char *line, uint16_t len);
} rilat_Matcher_t;

#if defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6100100)
#define __RILAT_MATCHER_SECTION      KCOMPILER_SECTION("_RILAT_MT")
#define __RILAT_MATCHER_START       ((uint32_t) &KCOMPILER_SECTION_START(_RILAT_MT))
#define __RILAT_MATCHER_END         ((uint32_t) &KCOMPILER_SECTION_END(_RILAT_MT))
#define _RILAT_MATCHER_EXTEND \
extern uint32_t KCOMPILER_SECTION_START(_RILAT_MT); \
extern uint32_t KCOMPILER_SECTION_END(_RILAT_MT);
#elif defined(__GNUC__RV32_CHX)
#define __RILAT_MATCHER_SECTION      KCOMPILER_SECTION("RILAT_MT")
#define __RILAT_MATCHER_START       ((uint32_t) &KCOMPILER_SECTION_START(RILAT_MT))
#define __RILAT_MATCHER_END         ((uint32_t) &KCOMPILER_SECTION_END(RILAT_MT))
#define _RILAT_MATCHER_EXTEND \
    extern uint32_t KCOMPILER_SECTION_START(RILAT_MT); \
    extern uint32_t KCOMPILER_SECTION_END(RILAT_MT);
#elif defined (__GNUC__)
#define __RILAT_MATCHER_SECTION      KCOMPILER_SECTION(".RILAT_MT")
#define __RILAT_MATCHER_START       ((uint32_t) &KCOMPILER_SECTION_START(_RILAT_MT))
#define __RILAT_MATCHER_END         ((uint32_t) &KCOMPILER_SECTION_END(_RILAT_MT))
#define _RILAT_MATCHER_EXTEND \
extern uint32_t KCOMPILER_SECTION_START(_RILAT_MT); \
extern uint32_t KCOMPILER_SECTION_END(_RILAT_MT);
#endif




#define RILAT_COMMAND_MATCH_DEFINE(_name, _matcher, _varData, _varLen) \
    static int32_t serialOnMatchFunc_##_name(const char *_varData, uint16_t _varLen); \
    __RILAT_MATCHER_SECTION const rilat_Matcher_t __SERIAL_MATCH_##_name = { \
        .matcher = _matcher"\0",                        \
        .onMatch = serialOnMatchFunc_##_name                                \
    }; \
    static int32_t serialOnMatchFunc_##_name(const char *_varData, uint16_t _varLen)

#endif

#if RILAT_PARTIAL_LINE_PATCH_ENABLE == 1

typedef struct {
    const char *matcher;
    const char *patchString;
    uint8_t patchStringLength;
} Rilat_PartialLinePatch_t;

#if defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6100100)
#define __RILAT_PARTIAL_LINE_PATCH_SECTION     KCOMPILER_SECTION("_RILAT_LP")
#define __RILAT_PARTIAL_LINE_PATCH_START       ((uint32_t) &KCOMPILER_SECTION_START(_RILAT_LP))
#define __RILAT_PARTIAL_LINE_PATCH_END         ((uint32_t) &KCOMPILER_SECTION_END(_RILAT_LP))
#define __RILAT_PARTIAL_LINE_PATCH_EXTERN \
    extern uint32_t KCOMPILER_SECTION_START(_RILAT_LP); \
    extern uint32_t KCOMPILER_SECTION_END(_RILAT_LP);
#elif defined(__GNUC__RV32_CHX)
#define __RILAT_PARTIAL_LINE_PATCH_SECTION     KCOMPILER_SECTION("RILAT_LP")
#define __RILAT_PARTIAL_LINE_PATCH_START       ((uint32_t) &KCOMPILER_SECTION_START(RILAT_LP))
#define __RILAT_PARTIAL_LINE_PATCH_END         ((uint32_t) &KCOMPILER_SECTION_END(RILAT_LP))
#define __RILAT_PARTIAL_LINE_PATCH_EXTERN \
    extern uint32_t KCOMPILER_SECTION_START(RILAT_LP); \
    extern uint32_t KCOMPILER_SECTION_END(RILAT_LP);
#elif defined (__GNUC__)
#define __RILAT_PARTIAL_LINE_PATCH_SECTION     KCOMPILER_SECTION(".RILAT_LP")
#define __RILAT_PARTIAL_LINE_PATCH_START       ((uint32_t) &KCOMPILER_SECTION_START(_RILAT_LP))
#define __RILAT_PARTIAL_LINE_PATCH_END         ((uint32_t) &KCOMPILER_SECTION_END(_RILAT_LP))
#define __RILAT_PARTIAL_LINE_PATCH_EXTERN \
    extern uint32_t KCOMPILER_SECTION_START(_RILAT_LP); \
    extern uint32_t KCOMPILER_SECTION_END(_RILAT_LP);
#endif


#define RILAT_PARTIAL_LINE_PATCH_DEFINE(name, _mather, _patchString, _patchStringLength) \
    __RILAT_PARTIAL_LINE_PATCH_SECTION const Rilat_PartialLinePatch_t __rilat_chn_partialLinePatch_##name = \
        { _mather, _patchString, _patchStringLength}

#endif

#if RILAT_PARTIAL_READ_LINE_LINK_PATCH_ENABLE == 1
typedef struct {
    const char *matcher;
    char* (*onMatch)(const char *line, uint16_t len);
} Rilat_ReadlLineLinkPatch_t;

#if defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6100100)
#define __RILAT_READ_LINE_LINK_PATCH_SECTION     KCOMPILER_SECTION("_RILAT_RLLP")
#define __RILAT_READ_LINE_LINK_PATCH_START       ((uint32_t) &KCOMPILER_SECTION_START(_RILAT_RLLP))
#define __RILAT_READ_LINE_LINK_PATCH_END         ((uint32_t) &KCOMPILER_SECTION_END(_RILAT_RLLP))
#define __RILAT_READ_LINE_LINK_PATCH_EXTERN \
extern uint32_t KCOMPILER_SECTION_START(_RILAT_RLLP); \
extern uint32_t KCOMPILER_SECTION_END(_RILAT_RLLP);
#elif defined(__GNUC__RV32_CHX)
#define __RILAT_READ_LINE_LINK_PATCH_SECTION     KCOMPILER_SECTION("RILAT_RLLP")
#define __RILAT_READ_LINE_LINK_PATCH_START       ((uint32_t) &KCOMPILER_SECTION_START(RILAT_RLLP))
#define __RILAT_READ_LINE_LINK_PATCH_END         ((uint32_t) &KCOMPILER_SECTION_END(RILAT_RLLP))
#define __RILAT_READ_LINE_LINK_PATCH_EXTERN \
    extern uint32_t KCOMPILER_SECTION_START(RILAT_RLLP); \
    extern uint32_t KCOMPILER_SECTION_END(RILAT_RLLP);
#elif defined (__GNUC__)
#define __RILAT_READ_LINE_LINK_PATCH_SECTION     KCOMPILER_SECTION(".RILAT_RLLP")
#define __RILAT_READ_LINE_LINK_PATCH_START       ((uint32_t) &KCOMPILER_SECTION_START(_RILAT_RLLP))
#define __RILAT_READ_LINE_LINK_PATCH_END         ((uint32_t) &KCOMPILER_SECTION_END(_RILAT_RLLP))
#define __RILAT_READ_LINE_LINK_PATCH_EXTERN \
    extern uint32_t KCOMPILER_SECTION_START(_RILAT_RLLP); \
    extern uint32_t KCOMPILER_SECTION_END(_RILAT_RLLP);
#endif




#define RILAT_READ_LINE_LINK_PATCH_DEFINE(name, _mather, _varData, _varLen) \
    static char* serialOnReadLinePatchMatchFunc_##_name(const char *_varData, uint16_t _varLen); \
    __RILAT_READ_LINE_LINK_PATCH_SECTION const Rilat_ReadlLineLinkPatch_t __rilat_chn_readLineLinkPatch_##name = \
        { _mather,  serialOnReadLinePatchMatchFunc_##_name}; \
    static char* serialOnReadLinePatchMatchFunc_##_name(const char *_varData, uint16_t _varLen)

#endif

typedef struct Rilat_AtLine {
    struct Rilat_AtLine *next;
    char *line;
} Rilat_AtLine_t;

typedef struct {
    uint8_t success: 1;
    uint8_t stream: 1;
    char *finalResponse;
    Rilat_AtLine_t *intermediates;
} Rilat_AtResponse_t;

typedef struct Rilat_Instance Rilat_Instance_t;

typedef enum {
    RILAT_CALL_EVENT_ON_INIT,
    RILAT_CALL_EVENT_ON_FINALIZE,
    
    RILAT_CALL_EVENT_ON_WRITE,
    RILAT_CALL_EVENT_ON_READ,
    
    RILAT_CALL_EVENT_ON_RAW_DATA_RECV,
    
    RILAT_CALL_EVENT_ON_BLOCK_WAIT,
    RILAT_CALL_EVENT_ON_POLL_HOCK,
} Rilat_CallbackEvent_t;
typedef union {
    void *ptr;
    uint32_t u32;
} Rilat_CallbackVar_t;
typedef int32_t (*Rilat_OnCallback_t)(Rilat_Instance_t *instance, Rilat_CallbackEvent_t event, Rilat_CallbackVar_t var1, Rilat_CallbackVar_t var2, Rilat_CallbackVar_t var3);

typedef enum {
    RILAT_PDU_S_WITH_CTLZ = 0x01,
    RILAT_PDU_S_ONLY_MATCH_TAG = 0x02,
    RILAT_PDU_S_RESP_ONLY_MATCH = 0x04,
    RILAT_PDU_S_DIRECT_WRITE = 0x08,
} Rilat_PduFlag_t;

typedef void (*Rilat_PduDirectWrite_t)(Rilat_Instance_t *instance, uint32_t packMsgId);

struct Rilat_Instance {
    //// User init
    void *userData;
    void *optPhy;
    const char *name;
    
    struct {
        uint16_t size;
        char *buffer;
        
        char *_posRead;
        char *_posEol;
        uint16_t _activeRecvLength;
    } recvBuffer;
    
    struct {
        uint16_t size;
        char *buffer;
    } transmitBuffer;
    
    Rilat_OnCallback_t callback;
    
    //// Internal
    struct {
        uint8_t commandType;
        char *commandResponsePrefix;
        Rilat_AtResponse_t *commandResponse;
        
        char *pduStartPrefix;
        void *smsPDU;
        uint16_t smsPDULength;
        uint32_t pduDirectWriteMsgId;
    } _pending;
    
    union {
        uint8_t _;
        struct {
            uint8_t isBusy : 1;
            uint8_t sendPDUCtrlZ : 1;
            uint8_t onlyMathPduTag: 1;
            uint8_t pduDirectWrite: 1;
        };
    } _flag;
};

/*@}*/

/**
 * @addtogroup Base funcs
 * @note none
 */

/*@{*/

extern void rilat_init(Rilat_Instance_t *instance);
extern void rilat_finalize(Rilat_Instance_t *instance);
extern int32_t rilat_loop(Rilat_Instance_t *instance);

/*@}*/

/**
 * @addtogroup Direct func
 * @note none
 */

/*@{*/

extern void rilat_setUserData(Rilat_Instance_t *instance, void *data);
extern void *rilat_getUserData(Rilat_Instance_t *instance);

extern int32_t rilat_directWritePhy(Rilat_Instance_t *instance, uint8_t *data, uint16_t length);
extern int32_t rilat_directWrite(Rilat_Instance_t *instance, uint8_t *data, uint16_t length, uint8_t isStreamData);
extern int32_t rilat_directWriteWithEOF(Rilat_Instance_t *instance, uint8_t *data, uint16_t length);
extern int32_t rilat_directWriteWithCtlZ(Rilat_Instance_t *instance, uint8_t *data, uint16_t length);
extern int32_t rilat_directSetPdu(Rilat_Instance_t *instance, uint8_t *data, uint16_t length, uint8_t withCtrlZ);
extern void rilat_directResetReceiveBuffer(Rilat_Instance_t *instance);

/*@}*/

/**
 * @addtogroup Block command func
 * @note none
 */

/*@{*/

extern void rilat_freeResponse(Rilat_Instance_t *instance, Rilat_AtResponse_t *p_response);
extern int32_t rilat_writeLineWithStreamData(Rilat_Instance_t *instance,
                                             const char *command,
                                             const char *pduStartPerFix,
                                             const char *responsePrefix,
                                             Rilat_AtResponse_t **pp_outResponse,
                                             uint32_t timeout);
extern int32_t rilat_writeLine(Rilat_Instance_t *instance,
                               const char *command,
                               Rilat_AtResponse_t **pp_outResponse,
                               uint32_t timeout);
extern int32_t rilat_writeSingleline(Rilat_Instance_t *instance,
                                     const char *command,
                                     const char *responsePrefix,
                                     Rilat_AtResponse_t **pp_outResponse,
                                     uint32_t timeout);
extern int32_t rilat_writeSinglelineOnlyPrefixMatched(Rilat_Instance_t *instance,
                                                      const char *command,
                                                      const char *responsePrefix,
                                                      Rilat_AtResponse_t **pp_outResponse,
                                                      uint32_t timeout);
extern int32_t rilat_writeMultiline(Rilat_Instance_t *instance,
                                    const char *command,
                                    const char *responsePrefix,
                                    Rilat_AtResponse_t **pp_outResponse,
                                    uint32_t timeout);
extern int32_t rilat_writeLineWithPDU(Rilat_Instance_t *instance,
                                      const char *command,
                                      const char *pduStartPerFix,
                                      void *pdu,
                                      uint16_t pduLength,
                                      Rilat_PduFlag_t pduFlag,
                                      uint32_t packMsgId,
                                      Rilat_AtResponse_t **pp_outResponse,
                                      uint32_t timeout);
extern int32_t rilat_writeSinglelineWithPDU(Rilat_Instance_t *instance,
                                            const char *command,
                                            const char *pduStartPerFix,
                                            void *pdu,
                                            uint16_t pduLength,
                                            Rilat_PduFlag_t pduFlag,
                                            const char *responsePrefix,
                                            uint32_t packMsgId,
                                            Rilat_AtResponse_t **pp_outResponse,
                                            uint32_t timeout);
extern int32_t rilat_writeMumericLine(Rilat_Instance_t *instance,
                                      const char *command,
                                      Rilat_AtResponse_t **pp_outResponse,
                                      uint32_t timeout);

/*@}*/

/**
 * @addtogroup Memheap func
 * @note none
 */

/*@{*/

#define rilat_malloc(i, s)          malloc(s)
#define rilat_calloc(i, s1, s2)     calloc(s1, s2)
#define rilat_free(instance, ptr)   free(ptr)

/*@}*/

#endif
