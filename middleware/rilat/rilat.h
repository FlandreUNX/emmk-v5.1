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

/** @file rilat.h
 *  @brief RILAT (RIL AT) 嵌入式AT指令解析库
 * 
 *  RILAT是一个轻量级的嵌入式AT指令解析库，专门用于处理各种通信模块的AT指令交互，
 *  支持命令发送、响应解析、PDU数据处理等功能。该库具有良好的可移植性和扩展性，
 *  适用于各种嵌入式平台和通信模块（如GSM、LTE、NB-IoT等）。
 * 
 *  @version 1.2.0
 *  @date 2025-07-26
 *  @author Flandreunx@outlook.com
 *  @copyright Apache License, Version 2.0
 */

#ifndef RILAT_H
#define RILAT_H

/* RILAT版本信息 */
#define RILAT_VERSION_MAJOR       1U          ///< 主版本号
#define RILAT_VERSION_MINOR       2U          ///< 次版本号
#define RILAT_VERSION_PATCH       0U          ///< 修订版本号
#define RILAT_VERSION             "1.2.0"     ///< 版本字符串

#include <stdint.h>
#include "opts_rilat.h"

/**
 * @addtogroup Configure
 * @brief RILAT配置选项
 * @note none
 */

/*@{*/

#ifndef RILAT_TX_LOG_SUPPORT
/**
 * @brief 是否支持发送日志
 * @details 当定义为1时，启用AT命令发送日志功能
 */
#define RILAT_TX_LOG_SUPPORT                    (1)
#endif

#ifndef RILAT_RX_LOG_SUPPORT
/**
 * @brief 是否支持接收日志
 * @details 当定义为1时，启用AT命令接收日志功能
 */
#define RILAT_RX_LOG_SUPPORT                    (1)
#endif

#ifndef RILAT_DIRECT_WRITE_LOG_SUPPORT
/**
 * @brief 是否支持直写日志
 * @details 当定义为1时，启用直接写入日志功能
 */
#define RILAT_DIRECT_WRITE_LOG_SUPPORT          (1)
#endif

#ifndef RILAT_STREAM_SUPPORT
/**
 * @brief 是否支持流数据处理
 * @details 当定义为1时，启用流数据处理功能
 */
#define RILAT_STREAM_SUPPORT                    (0)
#endif

// AT指令匹配处理
#ifndef RILAT_MATCHER_ENABLE
/**
 * @brief 是否启用AT指令匹配处理功能
 * @details 当定义为1时，启用AT指令匹配处理功能
 */
#define RILAT_MATCHER_ENABLE                        1
#endif

// AT指令行结尾补丁处理
#ifndef RILAT_PARTIAL_LINE_PATCH_ENABLE
/**
 * @brief 是否启用AT指令行结尾补丁处理功能
 * @details 当定义为1时，启用AT指令行结尾补丁处理功能
 */
#define RILAT_PARTIAL_LINE_PATCH_ENABLE             1
#endif

// AT指令URC推送前的补丁处理,
//  例如:
//      +QIURC: "recv",0,3<CR><CL>ABC<CR><CL> -> +QIURC: "recv",0,3, ABC<CR><CL>
#ifndef RILAT_PARTIAL_READ_LINE_LINK_PATCH_ENABLE
/**
 * @brief 是否启用AT指令URC推送前的补丁处理功能
 * @details 当定义为1时，启用AT指令URC推送前的补丁处理功能
 * @note 用于处理类似+QIURC: "recv",0,3<CR><CL>ABC<CR><CL>这样的情况，将其转换为+QIURC: "recv",0,3, ABC<CR><CL>
 */
#define RILAT_PARTIAL_READ_LINE_LINK_PATCH_ENABLE   1
#endif

/*@}*/

/**
 * @addtogroup Define & Typedef
 * @brief RILAT数据结构和类型定义
 * @note none
 */

/*@{*/

#if RILAT_MATCHER_ENABLE == 1

/**
 * @brief AT指令匹配器结构体
 * @details 用于定义AT指令匹配规则和对应的处理函数
 */
typedef struct {
    const char *matcher;                                    ///< 匹配字符串
    int32_t (*onMatch)(const char *line, uint16_t len);     ///< 匹配成功时的回调函数
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




/**
 * @brief 定义AT指令匹配处理函数
 * @param[in] _name 匹配器名称
 * @param[in] _matcher 匹配字符串
 * @param[in] _varData 数据变量名
 * @param[in] _varLen 长度变量名
 * @details 用于定义一个AT指令匹配处理函数，当接收到匹配的AT指令时会调用该函数
 */
#define RILAT_COMMAND_MATCH_DEFINE(_name, _matcher, _varData, _varLen) \
    static int32_t serialOnMatchFunc_##_name(const char *_varData, uint16_t _varLen); \
    __RILAT_MATCHER_SECTION const rilat_Matcher_t __SERIAL_MATCH_##_name = { \
        .matcher = _matcher"\0",                        \
        .onMatch = serialOnMatchFunc_##_name                                \
    }; \
    static int32_t serialOnMatchFunc_##_name(const char *_varData, uint16_t _varLen)

#endif

#if RILAT_PARTIAL_LINE_PATCH_ENABLE == 1

/**
 * @brief AT指令行补丁结构体
 * @details 用于定义AT指令行补丁规则
 */
typedef struct {
    const char *matcher;            ///< 匹配字符串
    const char *patchString;        ///< 补丁字符串
    uint8_t patchStringLength;      ///< 补丁字符串长度
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


/**
 * @brief 定义AT指令行补丁
 * @param[in] name 补丁名称
 * @param[in] _mather 匹配字符串
 * @param[in] _patchString 补丁字符串
 * @param[in] _patchStringLength 补丁字符串长度
 * @details 用于定义一个AT指令行补丁规则
 */
#define RILAT_PARTIAL_LINE_PATCH_DEFINE(name, _mather, _patchString, _patchStringLength) \
    __RILAT_PARTIAL_LINE_PATCH_SECTION const Rilat_PartialLinePatch_t __rilat_chn_partialLinePatch_##name = \
        { _mather, _patchString, _patchStringLength}

#endif

#if RILAT_PARTIAL_READ_LINE_LINK_PATCH_ENABLE == 1
/**
 * @brief AT指令读取行链接补丁结构体
 * @details 用于定义AT指令读取行链接补丁规则
 */
typedef struct {
    const char *matcher;                                    ///< 匹配字符串
    char* (*onMatch)(const char *line, uint16_t len);       ///< 匹配成功时的回调函数
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




/**
 * @brief 定义AT指令读取行链接补丁处理函数
 * @param[in] name 补丁名称
 * @param[in] _mather 匹配字符串
 * @param[in] _varData 数据变量名
 * @param[in] _varLen 长度变量名
 * @details 用于定义一个AT指令读取行链接补丁处理函数
 */
#define RILAT_READ_LINE_LINK_PATCH_DEFINE(name, _mather, _varData, _varLen) \
    static char* serialOnReadLinePatchMatchFunc_##_name(const char *_varData, uint16_t _varLen); \
    __RILAT_READ_LINE_LINK_PATCH_SECTION const Rilat_ReadlLineLinkPatch_t __rilat_chn_readLineLinkPatch_##name = \
        { _mather,  serialOnReadLinePatchMatchFunc_##_name}; \
    static char* serialOnReadLinePatchMatchFunc_##_name(const char *_varData, uint16_t _varLen)

#endif

/**
 * @brief AT响应行结构体
 * @details 用于存储AT命令的单行响应数据
 */
typedef struct Rilat_AtLine {
    struct Rilat_AtLine *next;      ///< 下一行指针
    char *line;                     ///< 行数据
} Rilat_AtLine_t;

/**
 * @brief AT响应结构体
 * @details 用于存储AT命令的完整响应数据
 */
typedef struct {
    uint8_t success: 1;             ///< 命令执行是否成功
    uint8_t stream: 1;              ///< 是否为流数据
    char *finalResponse;            ///< 最终响应数据
    Rilat_AtLine_t *intermediates;  ///< 中间响应行链表
} Rilat_AtResponse_t;

/**
 * @brief RILAT实例结构体前置声明
 * @details 用于声明RILAT实例结构体
 */
typedef struct Rilat_Instance Rilat_Instance_t;

/**
 * @brief RILAT回调事件枚举
 * @details 定义了RILAT实例可能触发的各种回调事件
 */
typedef enum {
    RILAT_CALL_EVENT_ON_INIT,           ///< 初始化事件
    RILAT_CALL_EVENT_ON_FINALIZE,       ///< 终止事件
    
    RILAT_CALL_EVENT_ON_WRITE,          ///< 写入事件
    RILAT_CALL_EVENT_ON_READ,           ///< 读取事件
    
    RILAT_CALL_EVENT_ON_RAW_DATA_RECV,  ///< 原始数据接收事件
    
    RILAT_CALL_EVENT_ON_BLOCK_WAIT,     ///< 阻塞等待事件
    RILAT_CALL_EVENT_ON_POLL_HOCK,      ///< 轮询钩子事件
} Rilat_CallbackEvent_t;

/**
 * @brief RILAT回调变量联合体
 * @details 用于在回调函数中传递不同类型的数据
 */
typedef union {
    void *ptr;      ///< 指针类型数据
    uint32_t u32;   ///< 32位无符号整数类型数据
} Rilat_CallbackVar_t;

/**
 * @brief RILAT回调函数指针类型
 * @param[in] instance RILAT实例指针
 * @param[in] event 回调事件类型
 * @param[in] var1 回调变量1
 * @param[in] var2 回调变量2
 * @param[in] var3 回调变量3
 * @return 0表示成功，-1表示失败
 * @details 定义了RILAT实例的回调函数原型
 */
typedef int32_t (*Rilat_OnCallback_t)(Rilat_Instance_t *instance, Rilat_CallbackEvent_t event, Rilat_CallbackVar_t var1, Rilat_CallbackVar_t var2, Rilat_CallbackVar_t var3);

/**
 * @brief RILAT PDU标志枚举
 * @details 定义了PDU数据处理的各种标志
 */
typedef enum {
    RILAT_PDU_S_WITH_CTLZ = 0x01,           ///< 使用Ctrl+Z结束符
    RILAT_PDU_S_ONLY_MATCH_TAG = 0x02,      ///< 仅匹配标签
    RILAT_PDU_S_RESP_ONLY_MATCH = 0x04,     ///< 响应仅匹配
    RILAT_PDU_S_DIRECT_WRITE = 0x08,        ///< 直接写入
} Rilat_PduFlag_t;

/**
 * @brief RILAT PDU直接写入函数指针类型
 * @param[in] instance RILAT实例指针
 * @param[in] packMsgId 包消息ID
 * @details 定义了PDU直接写入函数的原型
 */
typedef void (*Rilat_PduDirectWrite_t)(Rilat_Instance_t *instance, uint32_t packMsgId);

/**
 * @brief RILAT实例结构体
 * @details 包含了RILAT实例的所有数据和状态信息
 */
struct Rilat_Instance {
    //// User init
    void *userData;                 ///< 用户数据指针
    void *optPhy;                   ///< 物理接口选项
    const char *name;               ///< 实例名称
    
    /**
     * @brief 接收缓冲区结构体
     * @details 用于存储接收到的数据
     */
    struct {
        uint16_t size;              ///< 缓冲区大小
        char *buffer;               ///< 缓冲区指针
        
        char *_posRead;             ///< 读取位置指针
        char *_posEol;              ///< 行结束位置指针
        uint16_t _activeRecvLength; ///< 有效接收数据长度
    } recvBuffer;
    
    /**
     * @brief 发送缓冲区结构体
     * @details 用于存储待发送的数据
     */
    struct {
        uint16_t size;              ///< 缓冲区大小
        char *buffer;               ///< 缓冲区指针
    } transmitBuffer;
    
    Rilat_OnCallback_t callback;    ///< 回调函数指针
    
    //// Internal
    /**
     * @brief 待处理命令结构体
     * @details 用于存储当前正在处理的命令信息
     */
    struct {
        uint8_t commandType;                ///< 命令类型
        char *commandResponsePrefix;        ///< 命令响应前缀
        Rilat_AtResponse_t *commandResponse;///< 命令响应结构体指针
        
        char *pduStartPrefix;               ///< PDU开始前缀
        void *smsPDU;                       ///< SMS PDU数据指针
        uint16_t smsPDULength;              ///< SMS PDU数据长度
        uint32_t pduDirectWriteMsgId;       ///< PDU直接写入消息ID
    } _pending;
    
    /**
     * @brief 标志位联合体
     * @details 用于存储各种标志位
     */
    union {
        uint8_t _;                          ///< 标志位组合值
        struct {
            uint8_t isBusy : 1;             ///< 是否忙碌
            uint8_t sendPDUCtrlZ : 1;       ///< 是否发送PDU Ctrl+Z
            uint8_t onlyMathPduTag: 1;      ///< 仅匹配PDU标签
            uint8_t pduDirectWrite: 1;      ///< PDU直接写入
        };
    } _flag;
};

/*@}*/

/**
 * @addtogroup Base funcs
 * @brief RILAT基础功能函数
 * @note none
 */

/*@{*/

/**
 * @brief 初始化RILAT实例
 * @param[in] instance RILAT实例指针
 * @return 无
 * @details 初始化指定的RILAT实例，设置初始状态和回调函数
 */
extern void rilat_init(Rilat_Instance_t *instance);

/**
 * @brief 终止RILAT实例
 * @param[in] instance RILAT实例指针
 * @return 无
 * @details 终止指定的RILAT实例，释放相关资源
 */
extern void rilat_finalize(Rilat_Instance_t *instance);

/**
 * @brief RILAT主循环处理函数
 * @param[in] instance RILAT实例指针
 * @return 0表示成功，-1表示失败
 * @details 处理RILAT实例的主循环逻辑，包括数据读取和命令处理
 */
extern int32_t rilat_loop(Rilat_Instance_t *instance);

/*@}*/

/**
 * @addtogroup Direct func
 * @brief RILAT直接操作函数
 * @note none
 */

/*@{*/

/**
 * @brief 设置用户数据
 * @param[in] instance RILAT实例指针
 * @param[in] data 用户数据指针
 * @return 无
 * @details 设置与RILAT实例关联的用户数据
 */
extern void rilat_setUserData(Rilat_Instance_t *instance, void *data);

/**
 * @brief 获取用户数据
 * @param[in] instance RILAT实例指针
 * @return 用户数据指针
 * @details 获取与RILAT实例关联的用户数据
 */
extern void *rilat_getUserData(Rilat_Instance_t *instance);

/**
 * @brief 直接写入物理接口
 * @param[in] instance RILAT实例指针
 * @param[in] data 要写入的数据指针
 * @param[in] length 要写入的数据长度
 * @return 0表示成功，-1表示失败
 * @details 直接将数据写入物理接口，不进行任何格式化处理
 */
extern int32_t rilat_directWritePhy(Rilat_Instance_t *instance, uint8_t *data, uint16_t length);

/**
 * @brief 直接写入数据（无行结束符）
 * @param[in] instance RILAT实例指针
 * @param[in] data 要写入的数据指针
 * @param[in] length 要写入的数据长度
 * @param[in] isStreamData 是否为流数据
 * @return 0表示成功，-1表示失败
 * @details 将数据写入物理接口，不添加行结束符
 */
extern int32_t rilat_directWrite(Rilat_Instance_t *instance, uint8_t *data, uint16_t length, uint8_t isStreamData);

/**
 * @brief 直接写入数据（带行结束符）
 * @param[in] instance RILAT实例指针
 * @param[in] data 要写入的数据指针
 * @param[in] length 要写入的数据长度
 * @return 0表示成功，-1表示失败
 * @details 将数据写入物理接口，并在末尾添加\r\n行结束符
 */
extern int32_t rilat_directWriteWithEOF(Rilat_Instance_t *instance, uint8_t *data, uint16_t length);

/**
 * @brief 直接写入数据（带Ctrl+Z结束符）
 * @param[in] instance RILAT实例指针
 * @param[in] data 要写入的数据指针
 * @param[in] length 要写入的数据长度
 * @return 0表示成功，-1表示失败
 * @details 将数据写入物理接口，并在末尾添加Ctrl+Z(0x1A)结束符
 */
extern int32_t rilat_directWriteWithCtlZ(Rilat_Instance_t *instance, uint8_t *data, uint16_t length);

/**
 * @brief 直接设置PDU数据
 * @param[in] instance RILAT实例指针
 * @param[in] data PDU数据指针
 * @param[in] length PDU数据长度
 * @param[in] withCtrlZ 是否使用Ctrl+Z结束符
 * @return 0表示成功，-1表示失败
 * @details 直接设置PDU数据，用于后续发送
 */
extern int32_t rilat_directSetPdu(Rilat_Instance_t *instance, uint8_t *data, uint16_t length, uint8_t withCtrlZ);

/**
 * @brief 重置接收缓冲区
 * @param[in] instance RILAT实例指针
 * @return 无
 * @details 重置接收缓冲区的读取指针和相关状态，清空缓冲区内容
 */
extern void rilat_directResetReceiveBuffer(Rilat_Instance_t *instance);


/**
 * @brief 检查是否接收到Ctrl+Z字符
 * @param[in] instance RILAT实例指针
 * @return true表示接收到Ctrl+Z字符，false表示未接收到
 * @details 用于检查接收缓冲区中是否包含Ctrl+Z(0x1A)字符，通常用于判断PDU数据传输是否完成
 */
extern bool rilat_readForCtlZ(Rilat_Instance_t *instance);

/*@}*/

/**
 * @addtogroup Block command func
 * @brief RILAT阻塞命令函数
 * @note none
 */

/*@{*/

/**
 * @brief 释放AT响应结构体
 * @param[in] instance RILAT实例指针
 * @param[in] p_response 要释放的AT响应结构体指针
 * @return 无
 * @details 释放AT响应结构体及其包含的所有中间响应行和最终响应数据，防止内存泄漏
 */
extern void rilat_freeResponse(Rilat_Instance_t *instance, Rilat_AtResponse_t *p_response);

/**
 * @brief 发送带流数据的AT命令
 * @param[in] instance RILAT实例指针
 * @param[in] command 要发送的AT命令
 * @param[in] pduStartPerFix PDU开始前缀
 * @param[in] responsePrefix 期望的响应前缀
 * @param[out] pp_outResponse 返回的响应结构体指针
 * @param[in] timeout 超时时间（毫秒）
 * @return 0表示成功，-1表示失败
 * @details 用于发送需要处理流数据的AT命令，例如数据传输命令
 */
extern int32_t rilat_writeLineWithStreamData(Rilat_Instance_t *instance,
                                             const char *command,
                                             const char *pduStartPerFix,
                                             const char *responsePrefix,
                                             Rilat_AtResponse_t **pp_outResponse,
                                             uint32_t timeout);

/**
 * @brief 发送AT命令
 * @param[in] instance RILAT实例指针
 * @param[in] command 要发送的AT命令
 * @param[out] pp_outResponse 返回的响应结构体指针
 * @param[in] timeout 超时时间（毫秒）
 * @return 0表示成功，-1表示失败
 * @details 用于发送不需要中间响应的AT命令
 */
extern int32_t rilat_writeLine(Rilat_Instance_t *instance,
                               const char *command,
                               Rilat_AtResponse_t **pp_outResponse,
                               uint32_t timeout);

/**
 * @brief 发送单行响应AT命令
 * @param[in] instance RILAT实例指针
 * @param[in] command 要发送的AT命令
 * @param[in] responsePrefix 期望的响应前缀
 * @param[out] pp_outResponse 返回的响应结构体指针
 * @param[in] timeout 超时时间（毫秒）
 * @return 0表示成功，-1表示失败
 * @details 用于发送期望获得单行指定前缀响应的AT命令。如果未收到匹配的中间响应，函数将返回失败
 */
extern int32_t rilat_writeSingleline(Rilat_Instance_t *instance,
                                     const char *command,
                                     const char *responsePrefix,
                                     Rilat_AtResponse_t **pp_outResponse,
                                     uint32_t timeout);

/**
 * @brief 发送单行前缀匹配AT命令
 * @param[in] instance RILAT实例指针
 * @param[in] command 要发送的AT命令
 * @param[in] responsePrefix 期望的响应前缀
 * @param[out] pp_outResponse 返回的响应结构体指针
 * @param[in] timeout 超时时间（毫秒）
 * @return 0表示成功，-1表示失败
 * @details 用于发送仅当前缀匹配时即认为命令成功的AT命令。与普通单行响应命令不同，只要匹配到前缀就认为命令成功
 */
extern int32_t rilat_writeSinglelineOnlyPrefixMatched(Rilat_Instance_t *instance,
                                                      const char *command,
                                                      const char *responsePrefix,
                                                      Rilat_AtResponse_t **pp_outResponse,
                                                      uint32_t timeout);

/**
 * @brief 发送多行响应AT命令
 * @param[in] instance RILAT实例指针
 * @param[in] command 要发送的AT命令
 * @param[in] responsePrefix 期望的响应前缀
 * @param[out] pp_outResponse 返回的响应结构体指针
 * @param[in] timeout 超时时间（毫秒）
 * @return 0表示成功，-1表示失败
 * @details 用于发送期望获得多行指定前缀响应的AT命令
 */
extern int32_t rilat_writeMultiline(Rilat_Instance_t *instance,
                                    const char *command,
                                    const char *responsePrefix,
                                    Rilat_AtResponse_t **pp_outResponse,
                                    uint32_t timeout);

/**
 * @brief 发送带PDU数据的AT命令
 * @param[in] instance RILAT实例指针
 * @param[in] command 要发送的AT命令
 * @param[in] pduStartPerFix PDU开始前缀
 * @param[in] pdu PDU数据指针
 * @param[in] pduLength PDU数据长度
 * @param[in] pduFlag PDU标志位
 * @param[in] packMsgId 包消息ID
 * @param[out] pp_outResponse 返回的响应结构体指针
 * @param[in] timeout 超时时间（毫秒）
 * @return 0表示成功，-1表示失败
 * @details 用于发送需要附带PDU数据的AT命令，例如短信发送命令。支持多种PDU处理选项，如使用Ctrl+Z结束符、直接写入等
 */
extern int32_t rilat_writeLineWithPDU(Rilat_Instance_t *instance,
                                      const char *command,
                                      const char *pduStartPerFix,
                                      void *pdu,
                                      uint16_t pduLength,
                                      Rilat_PduFlag_t pduFlag,
                                      uint32_t packMsgId,
                                      Rilat_AtResponse_t **pp_outResponse,
                                      uint32_t timeout);

/**
 * @brief 发送带PDU数据的单行响应AT命令
 * @param[in] instance RILAT实例指针
 * @param[in] command 要发送的AT命令
 * @param[in] pduStartPerFix PDU开始前缀
 * @param[in] pdu PDU数据指针
 * @param[in] pduLength PDU数据长度
 * @param[in] pduFlag PDU标志位
 * @param[in] responsePrefix 期望的响应前缀
 * @param[in] packMsgId 包消息ID
 * @param[out] pp_outResponse 返回的响应结构体指针
 * @param[in] timeout 超时时间（毫秒）
 * @return 0表示成功，-1表示失败
 * @details 用于发送需要附带PDU数据且期望获得单行指定前缀响应的AT命令。如果未收到匹配的中间响应，函数将返回失败
 */
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

/**
 * @brief 发送数值响应AT命令
 * @param[in] instance RILAT实例指针
 * @param[in] command 要发送的AT命令
 * @param[out] pp_outResponse 返回的响应结构体指针
 * @param[in] timeout 超时时间（毫秒）
 * @return 0表示成功，-1表示失败
 * @details 用于发送期望获得以数字开头的单行响应的AT命令。如果未收到匹配的中间响应，函数将返回失败
 */
extern int32_t rilat_writeNumericLine(Rilat_Instance_t *instance,
                                      const char *command,
                                      Rilat_AtResponse_t **pp_outResponse,
                                      uint32_t timeout);

/*@}*/

/**
 * @addtogroup Memheap func
 * @brief RILAT内存管理函数
 * @note none
 */

/*@{*/

/**
 * @brief RILAT内存分配函数
 * @param[in] i RILAT实例指针
 * @param[in] s 要分配的内存大小
 * @return 分配的内存指针，失败返回NULL
 * @details 用于分配指定大小的内存
 */
#define rilat_malloc(i, s)          malloc(s)

/**
 * @brief RILAT内存清零分配函数
 * @param[in] i RILAT实例指针
 * @param[in] s1 要分配的内存块数量
 * @param[in] s2 每个内存块的大小
 * @return 分配并清零的内存指针，失败返回NULL
 * @details 用于分配指定数量和大小的内存块，并将内存清零
 */
#define rilat_calloc(i, s1, s2)     calloc(s1, s2)

/**
 * @brief RILAT内存释放函数
 * @param[in] instance RILAT实例指针
 * @param[in] ptr 要释放的内存指针
 * @return 无
 * @details 用于释放之前分配的内存
 */
#define rilat_free(instance, ptr)   free(ptr)

/*@}*/

#endif
