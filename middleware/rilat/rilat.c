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

/** @file rilat.c
 *  @brief RILAT (RIL AT) 嵌入式AT指令解析库实现文件
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

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "emmk-config.h"

#include "./rilat.h"

/**
 * @addtogroup DebugSupport
 * @note none
 */

/*@{*/

#undef TAG
#define TAG     "RILAT"
#undef DBG_SECTION_NAME
#define DBG_SECTION_NAME  TAG

/*@}*/

/**
 * @addtogroup Define & Typedef
 * @note none
 */

/*@{*/

#define NUM_ELEMS(x)      (sizeof(x) / sizeof(x[0]))

typedef enum {
    NO_RESULT, // no intermediate response expected
    NUMERIC, // a single intermediate response starting with a 0-9
    SINGLELINE, // a single intermediate response starting with a prefix
    MULTILINE, // multiple line intermediate response starting with a prefix
    SINGLELINE_ONLY_PREFIX_MATCH, // Response success only when prefix matched */
    STREAM_DATA,
} SendCommand_t;

/*@}*/

/**
 * @addtogroup Private constants
 * @note none
 */

/*@{*/

static const char *const _FINAL_RESPONSE_SUCCESS[] = {
    "OK",
    "SEND OK",
    "+ok", // M0L200PX
    "+OK", // CustomProtocol
};

static const char *const _FINAL_RESPONSE_ERROR[] = {
    "+CMDERR:",
    "+CME ERROR:",
    "+CTM2M ERROR:", // NB81_CTM2M
    "+CIS ERROR:", // HHM302_LWM2M
    "+SOCKET ERROR:", // HHM302_SOCKET
    "ERROR",
    "NO DIAL TONE",
    "NO CARRIER",
    "BUSY",
};

static void _handleUnsolicited(Rilat_Instance_t *instance, const char *line);

#if RILAT_PARTIAL_READ_LINE_LINK_PATCH_ENABLE == 1

RILAT_READ_LINE_LINK_PATCH_DEFINE(O_O, "VVVVBEDF1", data, len) {
    return 0;
}

#endif

#if RILAT_PARTIAL_LINE_PATCH_ENABLE == 1
RILAT_PARTIAL_LINE_PATCH_DEFINE(ooo, "AAAABEAF1", "\r\n", 2);
#endif

/*@}*/

/**
 * @addtogroup Private funcs
 * @note none
 */

/*@{*/

/**
 * @brief 添加中间响应行到响应结构体中
 * @param[in] instance RILAT实例指针
 * @param[in] line 要添加的响应行
 * @return 无
 * 
 * @details 该函数用于将AT命令的中间响应行添加到当前命令的响应结构体中，
 *          通常用于处理多行响应或需要进一步解析的数据行。函数会分配内存来存储行数据，
 *          并将其添加到响应结构体的链表头部。
 */
static void addIntermediate(Rilat_Instance_t *instance, const char *line) {
    Rilat_AtLine_t *p_new;

    p_new = rilat_malloc(instance, sizeof(Rilat_AtLine_t));
    ASSERT(p_new != NULL);
    memset(p_new, 0x00, sizeof(Rilat_AtLine_t));

    p_new->line = rilat_malloc(instance, strlen(line) + 1);
    ASSERT(p_new->line != NULL);
    memset(p_new->line, 0x00, strlen(line) + 1);
    strcpy(p_new->line, line);

    p_new->next = instance->_pending.commandResponse->intermediates;
    instance->_pending.commandResponse->intermediates = p_new;
}


/**
 * @brief 反转中间响应行的顺序
 * @param[in,out] p_response AT响应结构体指针
 * @return 无
 * 
 * @details 由于响应行是以链表形式存储的，且新的行总是添加到链表头部，
 *          所以行的顺序是反的。该函数将中间响应行链表反转，使其恢复正确的顺序，
 *          便于后续处理和解析。
 */
static void reverseIntermediates(Rilat_AtResponse_t *p_response) {
    Rilat_AtLine_t *pcur, *pnext;

    pcur = p_response->intermediates;
    p_response->intermediates = NULL;

    while (pcur != NULL) {
        pnext = pcur->next;
        pcur->next = p_response->intermediates;
        p_response->intermediates = pcur;
        pcur = pnext;
    }
}


/**
 * @brief 清除挂起的命令状态
 * @param[in] instance RILAT实例指针
 * @return 无
 * 
 * @details 该函数用于清理当前挂起的AT命令相关资源，包括释放响应结构体内存、
 *          清空相关指针和状态变量，为下一条AT命令的执行做准备。
 */
static void clearPendingCommand(Rilat_Instance_t *instance) {
    if (instance->_pending.commandResponse != NULL) {
        rilat_freeResponse(instance, instance->_pending.commandResponse);
    }

    instance->_pending.commandResponse = NULL;
    instance->_pending.commandResponsePrefix = NULL;
    instance->_pending.smsPDU = NULL;
    instance->_pending.pduStartPrefix = NULL;
}


/**
 * @brief 创建新的AT响应结构体
 * @param[in] instance RILAT实例指针
 * @return 新创建的AT响应结构体指针，失败返回NULL
 * 
 * @details 该函数用于为新的AT命令创建响应结构体，分配所需内存并初始化为零。
 *          这是处理AT命令响应的第一步，所有AT命令的响应处理都基于此结构体。
 */
static Rilat_AtResponse_t *atResponseNew(Rilat_Instance_t *instance) {
    Rilat_AtResponse_t *p = rilat_malloc(instance, sizeof(Rilat_AtResponse_t));
    if (p == NULL) {
        LOG_W("atResponseNew, AllocFailed");
        return NULL;
    }

    memset(p, 0x00, sizeof(Rilat_AtResponse_t));
    return p;
}


/**
 * @brief 在内存块中查找子字符串
 * @param[in] srcStr 源字符串指针
 * @param[in] srcStrLength 源字符串长度
 * @param[in] subStr 要查找的子字符串指针
 * @return 找到的子字符串指针，未找到返回NULL
 * 
 * @details 此函数在指定长度的内存块中查找子字符串，类似于strstr函数，
 *          但增加了长度限制，避免越界访问。通过先比较首字符再使用memcmp
 *          进行完整比较的方式提高效率。
 */
static char *xMemstr(char *srcStr, int32_t srcStrLength, char *subStr) {
    // 检查输入参数的有效性
    if (srcStr == NULL || srcStrLength <= 0 || subStr == NULL) {
        return NULL;
    }
    
    // 空字符串处理 - 按照设计要求，空字符串不匹配任何内容
    if (*subStr == '\0') {
        return NULL;
    }
    
    // 计算子字符串长度并检查有效性
    uint16_t sublen = strlen(subStr);
    if (sublen > srcStrLength || sublen == 0) {
        return NULL;
    }

    // 计算最后可能的匹配位置
    char *cur = srcStr;
    int16_t lastPossible = srcStrLength - sublen + 1;
    if (lastPossible <= 0) {
        return NULL;
    }

    // 逐位置查找匹配
    for (uint16_t i = 0; i < lastPossible; i++) {
        // 先比较第一个字符以提高效率
        if (*cur == *subStr) {
            // 首字符匹配后，使用memcmp进行完整比较
            if (memcmp(cur, subStr, sublen) == 0) {
                return cur;
            }
        }
        cur++;
    }

    return NULL;
}


#if RILAT_MATCHER_ENABLE == 1

static const rilat_Matcher_t *findCommand(const char *line) {
    _RILAT_MATCHER_EXTEND;

    uint32_t strLen = strlen(line);
    uint8_t nFound = 0;
    const rilat_Matcher_t *_match = NULL;
    int32_t tlen = ((rilat_Matcher_t *) __RILAT_MATCHER_END) - ((rilat_Matcher_t *) __RILAT_MATCHER_START);

    for (rilat_Matcher_t *match = ((rilat_Matcher_t *) __RILAT_MATCHER_START);
         match != ((rilat_Matcher_t *) __RILAT_MATCHER_START) + tlen;
         match++) {
        if (strstr(line, match->matcher)) {
            if (strLen == strlen(match->matcher)) {
                // full match
                return match;
            }

            _match = match;
            nFound++;
        }
    }

    if (nFound == 1) {
        return _match;
    }

    return NULL;
}

#endif


/**
 * @brief 查找下一个行结束符
 * @param[in] instance RILAT实例指针
 * @param[in] p_cursor 当前查找位置指针
 * @return 行结束符位置指针，未找到返回NULL
 * 
 * @details 该函数用于在数据缓冲区中查找下一个行结束符的位置，支持查找\r、\n字符，
 *          以及特殊字符>。这是解析AT命令响应行的关键函数，确保能正确分割每一行数据。
 */
static char *findNextEol(Rilat_Instance_t *instance, char *p_cursor) {
    if (p_cursor[0] == '>') {
        return p_cursor + 1;
    }

    while (*p_cursor != '\0' && *p_cursor != '\r' && *p_cursor != '\n') {
        p_cursor++;

        if (p_cursor >= instance->recvBuffer.buffer + instance->recvBuffer.size) {
            return NULL;
        }
    }

    return *p_cursor == '\0' ? NULL : p_cursor;
}


/**
 * @brief 从接收缓冲区读取一行数据
 * @param[in] instance RILAT实例指针
 * @return 读取到的行数据指针，无数据可读时返回NULL
 * 
 * @details 该函数从接收缓冲区中读取一行完整的数据，处理行结束符并记录日志。
 *          支持部分行数据的拼接处理，确保能正确解析AT命令响应。
 */
static const char *readLine(Rilat_Instance_t *instance) {
    char *ret;
    char *p_read = NULL;
    char *p_eol = NULL;

    uint32_t count = 0;

    if (*(instance->recvBuffer._posRead) == '\0') {
        instance->recvBuffer._posRead = instance->recvBuffer.buffer;
        *(instance->recvBuffer._posRead) = '\0';
        p_read = instance->recvBuffer.buffer;
    } else {
        while (*(instance->recvBuffer._posRead) == '\r' || *(instance->recvBuffer._posRead) == '\n') {
            instance->recvBuffer._posRead++;
        }

#if RILAT_PARTIAL_READ_LINE_LINK_PATCH_ENABLE == 1
        __RILAT_READ_LINE_LINK_PATCH_EXTERN;

        Rilat_ReadlLineLinkPatch_t *start = (Rilat_ReadlLineLinkPatch_t *) __RILAT_READ_LINE_LINK_PATCH_START;
        int32_t tlen = ((Rilat_ReadlLineLinkPatch_t *) __RILAT_READ_LINE_LINK_PATCH_END) - start;
        uint8_t isMatch = 0;

        for (Rilat_ReadlLineLinkPatch_t *patcher = start; patcher != start + tlen; patcher++) {
            char *s = xMemstr(instance->recvBuffer._posRead, instance->recvBuffer._activeRecvLength,
                              (char *) patcher->matcher);
            if (s == NULL) {
                continue;
            }

            char *findEol = findNextEol(instance, instance->recvBuffer._posRead);
            if (findEol != NULL && ((uint32_t) findEol < (uint32_t) s)) {
                p_eol = findEol;
                isMatch = 2;
                break;
            }

            char *newEOL = patcher->onMatch(s, instance->recvBuffer._activeRecvLength);
            if (newEOL != NULL) {
                p_eol = newEOL;
                isMatch = 1;
            }
            break;
        }

        if (!isMatch) {
            p_eol = findNextEol(instance, instance->recvBuffer._posRead);
        }
#else
        p_eol = findNextEol(instance, instance->recvBuffer._posRead);
#endif

        if (p_eol == NULL) {
#if RILAT_PARTIAL_LINE_PATCH_ENABLE == 1
            __RILAT_PARTIAL_LINE_PATCH_EXTERN;

            Rilat_PartialLinePatch_t *start = (Rilat_PartialLinePatch_t *) __RILAT_PARTIAL_LINE_PATCH_START;
            int32_t tlen = ((Rilat_PartialLinePatch_t *) __RILAT_PARTIAL_LINE_PATCH_END) - start;

            for (Rilat_PartialLinePatch_t *patcher = start; patcher != start + tlen; patcher++) {
                if (!strstr(instance->recvBuffer._posRead, patcher->matcher)) {
                    continue;
                }

                memcpy(instance->recvBuffer._posRead + strlen(instance->recvBuffer._posRead), patcher->patchString,
                       patcher->patchStringLength);

                p_eol = findNextEol(instance, instance->recvBuffer._posRead);

                if (p_eol != NULL) {
                    break;
                }
            }
#endif

            if (p_eol == NULL) {
                uint32_t len = strlen(instance->recvBuffer._posRead);

                memmove(instance->recvBuffer.buffer, instance->recvBuffer._posRead, len + 1);
                p_read = instance->recvBuffer.buffer + len;
                instance->recvBuffer._posRead = instance->recvBuffer.buffer;
            }
        }
    }

    while (p_eol == NULL) {
        if (0 == instance->recvBuffer.size - (p_read - (char *) instance->recvBuffer.buffer)) {
            instance->recvBuffer._posRead = instance->recvBuffer.buffer;
            *(instance->recvBuffer._posRead) = '\0';
            p_read = instance->recvBuffer.buffer;
        }

        if (instance->callback(instance, RILAT_CALL_EVENT_ON_READ,
                               (Rilat_CallbackVar_t){.ptr = (uint8_t *) p_read},
                               (Rilat_CallbackVar_t){
                                   .u32 = instance->recvBuffer.size -
                                          (p_read - (char *) instance->recvBuffer.buffer)
                               },
                               (Rilat_CallbackVar_t){.ptr = &count}) == 0
            && count > 0) {
            instance->recvBuffer._activeRecvLength = count;
            p_read[count] = '\0';

            while (*(instance->recvBuffer._posRead) == '\r' || *(instance->recvBuffer._posRead) == '\n') {
                instance->recvBuffer._posRead++;
                instance->recvBuffer._activeRecvLength--;
            }

#if RILAT_PARTIAL_READ_LINE_LINK_PATCH_ENABLE == 1
            __RILAT_READ_LINE_LINK_PATCH_EXTERN;

            Rilat_ReadlLineLinkPatch_t *start = (Rilat_ReadlLineLinkPatch_t *) __RILAT_READ_LINE_LINK_PATCH_START;
            int32_t tlen = ((Rilat_ReadlLineLinkPatch_t *) __RILAT_READ_LINE_LINK_PATCH_END) - start;
            uint8_t isMatch = 0;

            for (Rilat_ReadlLineLinkPatch_t *patcher = start; patcher != start + tlen; patcher++) {
                char *s = xMemstr(instance->recvBuffer._posRead, count, (char *) patcher->matcher);
                if (s == NULL) {
                    continue;
                }

                char *findEol = findNextEol(instance, instance->recvBuffer._posRead);
                if (findEol != NULL && ((uint32_t) findEol < (uint32_t) s)) {
                    instance->recvBuffer._activeRecvLength -= ((uint32_t) findEol -
                                                               (uint32_t) instance->recvBuffer._posRead);
                    p_eol = findEol;
                    isMatch = 2;
                    break;
                }

                char *newEOL = patcher->onMatch(s, count);
                if (newEOL != NULL) {
                    instance->recvBuffer._activeRecvLength = ((uint32_t) newEOL - (uint32_t) s);
                    p_eol = newEOL;
                    isMatch = 1;
                }
                break;
            }

            if (!isMatch) {
                p_eol = findNextEol(instance, instance->recvBuffer._posRead);

                if (p_eol != NULL) {
                    instance->recvBuffer._activeRecvLength = ((uint32_t) p_eol -
                                                              (uint32_t) instance->recvBuffer._posRead);
                }
            }
#else
            p_eol = findNextEol(instance, instance->recvBuffer._posRead);
#endif
            p_read += count;
        } else if (count <= 0) {
            return NULL;
        }
    }

    ret = instance->recvBuffer._posRead;

    *p_eol = '\0';
    instance->recvBuffer._posEol = p_eol;

    instance->recvBuffer._posRead = p_eol + 1;

#if RILAT_RX_LOG_SUPPORT == 1
    LOG_I("%s< %s", instance->name, ret);
#endif

    return ret;
}


/**
 * @brief 检查是否为最终错误响应
 * @param[in] line 待检查的行数据
 * @param[in] len 行数据长度，为0时使用strlen计算
 * @return 找到的错误标识指针，未找到返回NULL
 * 
 * @details 该函数检查给定的行数据是否包含预定义的错误响应标识，
 *          用于判断AT命令是否执行失败。
 */
static char *isFinalResponseError(const char *line, uint32_t len) {
    for (uint8_t i = 0; i < NUM_ELEMS(_FINAL_RESPONSE_ERROR); i++) {
        char *s = xMemstr((char *) line, (int32_t) (len == 0 ? strlen(line) : len), (char *) _FINAL_RESPONSE_ERROR[i]);
        if (s) {
            return s;
        }
    }
    return NULL;
}


/**
 * @brief 检查是否为最终成功响应
 * @param[in] line 待检查的行数据
 * @param[in] len 行数据长度，为0时使用strlen计算
 * @return 找到的成功标识指针，未找到返回NULL
 * 
 * @details 该函数检查给定的行数据是否包含预定义的成功响应标识，
 *          用于判断AT命令是否执行成功。
 */
static char *isFinalResponseSuccess(const char *line, uint32_t len) {
    for (uint8_t i = 0; i < NUM_ELEMS(_FINAL_RESPONSE_SUCCESS); i++) {
        char *s = xMemstr((char *) line, (int32_t) (len == 0 ? strlen(line) : len),
                          (char *) _FINAL_RESPONSE_SUCCESS[i]);
        if (s) {
            return s;
        }
    }
    return NULL;
}


/**
 * @brief 检查是否为响应行格式
 * @param[in] line 待检查的行数据
 * @return 1表示是响应行格式，0表示不是
 * 
 * @details 该函数检查给定行是否符合AT命令响应行的格式（包含+和:字符），
 *          用于区分普通响应行和带有参数的响应行。
 */
static uint8_t isResponseLine(const char *line) {
    uint8_t found = 0;
    uint32_t len = strlen(line);

    for (uint32_t i = 0; i < len; i++) {
        if (line[i] == '+') {
            found = 1;
        }

        if (found == 1) {
            if (line[i] == ':') {
                found = 2;

                break;
            }
        }
    }

    return found == 2;
}


/**
 * @brief 处理最终响应
 * @param[in] instance RILAT实例指针
 * @param[in] line 最终响应行数据
 * @return 无
 * 
 * @details 该函数将最终响应行数据保存到当前命令的响应结构体中，
 *          用于AT命令执行完成后的结果处理。
 */
static void handleFinalResponse(Rilat_Instance_t *instance, const char *line) {
    instance->_pending.commandResponse->finalResponse = rilat_malloc(instance, strlen(line) + 1);
    ASSERT(instance->_pending.commandResponse->finalResponse != NULL);

    memset(instance->_pending.commandResponse->finalResponse, 0x00, strlen(line) + 1);
    strcpy(instance->_pending.commandResponse->finalResponse, line);
}


/**
 * @brief 写入数据到物理接口
 * @param[in] instance RILAT实例指针
 * @param[in] s 要写入的数据指针
 * @param[in] len 要写入的数据长度
 * @param[in] suffix 要追加的后缀数据指针
 * @param[in] suffixLen 要追加的后缀数据长度
 * @param[in] isStreamData 是否为流数据
 * @param[in] skipFlag 是否跳过某些处理步骤
 * @return 0表示成功，-1表示失败
 * 
 * @details 该函数将数据写入物理接口，支持添加后缀数据。
 *          支持直接写入和通过传输缓冲区写入两种方式。
 */
static int32_t writeData(Rilat_Instance_t *instance, void *s, uint16_t len, 
                         const void *suffix, uint16_t suffixLen, 
                         uint8_t isStreamData, bool skipFlag) {
    uint32_t cur = 0;
    int32_t written;
    uint16_t totalLen = len + suffixLen;

    if (!skipFlag && instance->_flag.pduDirectWrite) {
        Rilat_PduDirectWrite_t call = s;
        call(instance, instance->_pending.pduDirectWriteMsgId);
        
        // 写入后缀数据
        if (suffix != NULL && suffixLen > 0) {
            written = instance->callback(instance, RILAT_CALL_EVENT_ON_WRITE,
                                         (Rilat_CallbackVar_t){.ptr = (uint8_t *) suffix},
                                         (Rilat_CallbackVar_t){.u32 = suffixLen},
                                         (Rilat_CallbackVar_t){});
            if (written < 0) {
                return -1;
            }
        }
    } else {
        if (s != NULL && len != 0) {
            if (instance->transmitBuffer.buffer != NULL && instance->transmitBuffer.size >= totalLen) {
                memcpy(instance->transmitBuffer.buffer, s, len);
                if (suffix != NULL && suffixLen > 0) {
                    memcpy(instance->transmitBuffer.buffer + len, suffix, suffixLen);
                }

                while (cur < totalLen) {
                    written = instance->callback(instance, RILAT_CALL_EVENT_ON_WRITE,
                                                 (Rilat_CallbackVar_t){
                                                     .ptr = (uint8_t *) instance->transmitBuffer.buffer + cur
                                                 },
                                                 (Rilat_CallbackVar_t){.u32 = totalLen - cur},
                                                 (Rilat_CallbackVar_t){});
                    if (written < 0) {
                        return -1;
                    }

                    cur += written;
                }
            } else {
                // 写入主要数据
                while (cur < len) {
                    written = instance->callback(instance, RILAT_CALL_EVENT_ON_WRITE,
                                                 (Rilat_CallbackVar_t){.ptr = (uint8_t *) s + cur},
                                                 (Rilat_CallbackVar_t){.u32 = len - cur},
                                                 (Rilat_CallbackVar_t){});
                    if (written < 0) {
                        return -1;
                    }

                    cur += written;
                }
                
                // 写入后缀数据
                if (suffix != NULL && suffixLen > 0) {
                    written = instance->callback(instance, RILAT_CALL_EVENT_ON_WRITE,
                                                 (Rilat_CallbackVar_t){.ptr = (uint8_t *) suffix},
                                                 (Rilat_CallbackVar_t){.u32 = suffixLen},
                                                 (Rilat_CallbackVar_t){});
                    if (written < 0) {
                        return -1;
                    }
                }
            }
        } else if (suffix != NULL && suffixLen > 0) {
            // 只写入后缀数据
            written = instance->callback(instance, RILAT_CALL_EVENT_ON_WRITE,
                                         (Rilat_CallbackVar_t){.ptr = (uint8_t *) suffix},
                                         (Rilat_CallbackVar_t){.u32 = suffixLen},
                                         (Rilat_CallbackVar_t){});
            if (written < 0) {
                return -1;
            }
        }

#if EMMK_CFG_DEBUG_ENABLE == 1 && (RILAT_TX_LOG_SUPPORT == 1 || RILAT_DIRECT_WRITE_LOG_SUPPORT == 1)
        if (!isStreamData && s != NULL) {
            LOG_I("%s> %s", instance->name, (char *) s);
        }
#endif
    }

    return 0;
}


/**
 * @brief 写入数据（无行结束符）
 * @param[in] instance RILAT实例指针
 * @param[in] s 要写入的数据指针
 * @param[in] len 要写入的数据长度
 * @param[in] isStreamData 是否为流数据
 * @param[in] skipFlag 是否跳过某些处理步骤
 * @return 0表示成功，-1表示失败
 * 
 * @details 该函数将数据写入物理接口，不添加行结束符。
 *          支持直接写入和通过传输缓冲区写入两种方式。
 */
static int32_t writeNoeof(Rilat_Instance_t *instance, void *s, uint16_t len, uint8_t isStreamData, bool skipFlag) {
    return writeData(instance, s, len, NULL, 0, isStreamData, skipFlag);
}


/**
 * @brief 写入数据（带行结束符）
 * @param[in] instance RILAT实例指针
 * @param[in] s 要写入的数据指针
 * @param[in] len 要写入的数据长度
 * @param[in] skipFlag 是否跳过某些处理步骤
 * @return 0表示成功，-1表示失败
 * 
 * @details 该函数将数据写入物理接口，并在末尾添加\r\n行结束符。
 *          支持直接写入和通过传输缓冲区写入两种方式。
 */
static int32_t writeEof(Rilat_Instance_t *instance, void *s, uint16_t len, bool skipFlag) {
    static const char SUFFIX[] = "\r\n";
    return writeData(instance, s, len, SUFFIX, 2, 0, skipFlag);
}


/**
 * @brief 写入数据（带Ctrl+Z结束符）
 * @param[in] instance RILAT实例指针
 * @param[in] s 要写入的数据指针
 * @param[in] len 要写入的数据长度
 * @param[in] skipFlag 是否跳过某些处理步骤
 * @return 0表示成功，-1表示失败
 * 
 * @details 该函数将数据写入物理接口，并在末尾添加Ctrl+Z(0x1A)结束符。
 *          主要用于短信PDU数据的发送。
 */
static int32_t writeCtlZ(Rilat_Instance_t *instance, void *s, uint16_t len, bool skipFlag) {
    static const char SUFFIX = 0x1A;
    return writeData(instance, s, len, &SUFFIX, 1, 0, skipFlag);
}


/**
 * @brief 处理读取到的一行数据
 * @param[in] instance RILAT实例指针
 * @param[in] line 读取到的行数据
 * @return 无
 * 
 * @details 根据当前命令状态和行数据内容，将数据分发到相应的处理函数。
 *          可能的处理包括：最终响应处理、中间响应处理、PDU数据处理或未识别命令处理。
 */
static void handleLine(Rilat_Instance_t *instance, const char *line) {
    if (instance->_pending.commandResponse == NULL) {
        _handleUnsolicited(instance, line);
    } else if (isFinalResponseSuccess(line, 0) != NULL
               && instance->_pending.commandType != STREAM_DATA
               && instance->_pending.commandType != SINGLELINE_ONLY_PREFIX_MATCH) {
        instance->_pending.commandResponse->success = 1;
        if (instance->_flag.onlyMathPduTag == 0) {
            handleFinalResponse(instance, line);
        }
    } else if (isFinalResponseError(line, 0) != NULL) {
        instance->_pending.commandResponse->success = 0;
        handleFinalResponse(instance, line);
    } else if (instance->_pending.smsPDU != NULL) {
        if ((instance->_pending.pduStartPrefix != NULL && (0 == strcmp(line, instance->_pending.pduStartPrefix)))
            || (instance->_pending.pduStartPrefix == NULL && (0 == strcmp(line, ">")))) {
            if (instance->_flag.sendPDUCtrlZ) {
                writeCtlZ(instance, instance->_pending.smsPDU, instance->_pending.smsPDULength, false);
            } else {
                writeNoeof(instance, instance->_pending.smsPDU, instance->_pending.smsPDULength, 0, false);
            }
            instance->_pending.smsPDU = NULL;
            instance->_pending.pduStartPrefix = NULL;
            instance->_pending.smsPDULength = 0;
            instance->_flag.onlyMathPduTag = 0;
            instance->_flag.pduDirectWrite = 0;
        }
    } else {
        switch (instance->_pending.commandType) {
            case NO_RESULT: {
                _handleUnsolicited(instance, line);
                break;
            }
            case SINGLELINE_ONLY_PREFIX_MATCH: {
                if (instance->_pending.commandResponse->intermediates == NULL
                    && instance->_pending.commandResponsePrefix != NULL
                    && strstr(line, instance->_pending.commandResponsePrefix)) {
                    addIntermediate(instance, line);
                    handleFinalResponse(instance, line);
                    instance->_pending.commandResponse->success = 1;
                } else {
                    _handleUnsolicited(instance, line);
                }
                break;
            }
            case SINGLELINE: {
                if (instance->_pending.commandResponse->intermediates == NULL
                    && instance->_pending.commandResponsePrefix != NULL
                    && strstr(line, instance->_pending.commandResponsePrefix)) {
                    addIntermediate(instance, line);
                } else {
                    _handleUnsolicited(instance, line);
                }
                break;
            }
            case MULTILINE: {
                if (instance->_pending.commandResponsePrefix != NULL
                    && strstr(line, instance->_pending.commandResponsePrefix)) {
                    addIntermediate(instance, line);
                } else {
                    if (isResponseLine(line)) {
                        _handleUnsolicited(instance, line);
                    } else {
                        addIntermediate(instance, line);
                    }
                }
                break;
            }
            case NUMERIC: {
                if (instance->_pending.commandResponse->intermediates == NULL
                    && isdigit((int) line[0])) {
                    addIntermediate(instance, line);
                } else {
                    _handleUnsolicited(instance, line);
                }
                break;
            }
        }
    }
}


/**
 * @brief 读取数据循环处理
 * @param[in] instance RILAT实例指针
 * @return 无
 * 
 * @details 根据当前命令类型，循环读取并处理数据。对于流数据类型命令，
 *          直接处理数据流；对于其他类型命令，按行读取并处理。
 */
static void readDataLoop(Rilat_Instance_t *instance) {
#if RILAT_STREAM_SUPPORT == 1
    if (instance->_pending.commandResponse != NULL && instance->_pending.commandType == STREAM_DATA) {
        uint32_t count = 0;
        if (instance->callback(instance, RILAT_CALL_EVENT_ON_READ,
                               (Rilat_CallbackVar_t){.ptr = instance->recvBuffer._posRead},
                               (Rilat_CallbackVar_t){
                                   .u32 = instance->recvBuffer.size - (
                                              (uint32_t) instance->recvBuffer._posRead - (uint32_t) instance->recvBuffer
                                              .buffer)
                               },
                               (Rilat_CallbackVar_t){.ptr = &count}) == 0 && count > 0) {
            LOG_I("#%s< %d, ...", instance->name, (int) count);

            instance->recvBuffer._posRead += count;
            instance->recvBuffer._activeRecvLength += count;

            char *ctlEol;
            uint32_t ctlCount;
            if (count >= 32) {
                ctlCount = 32;
                ctlEol = &instance->recvBuffer.buffer[instance->recvBuffer._activeRecvLength - ctlCount];
            } else {
                ctlCount = count;
                ctlEol = instance->recvBuffer.buffer;
            }

            if (xMemstr(instance->recvBuffer.buffer, (int32_t) instance->recvBuffer._activeRecvLength,
                        instance->_pending.pduStartPrefix) != NULL) {
                if (isFinalResponseSuccess(ctlEol, ctlCount) != NULL ||
                    (instance->_pending.commandResponsePrefix != NULL
                     && xMemstr(ctlEol, (int32_t) ctlCount, instance->_pending.commandResponsePrefix) != NULL)) {
                    instance->_pending.commandResponse->success = 1;
                    instance->_pending.commandResponse->finalResponse = instance->recvBuffer.buffer;
                    LOG_I("#%s< SUCCESS", instance->name);
                } else if (isFinalResponseError(ctlEol, ctlCount) != NULL) {
                    instance->_pending.commandResponse->success = 0;
                    instance->_pending.commandResponse->finalResponse = instance->recvBuffer.buffer;
                    LOG_W("#%s< FAILED", instance->name);
                } else {
                    LOG_I("#%s< CONTINUE", instance->name);
                }
            } else if (isFinalResponseError(ctlEol, ctlCount) != NULL) {
                instance->_pending.commandResponse->success = 0;
                instance->_pending.commandResponse->finalResponse = instance->recvBuffer.buffer;
                LOG_W("#%s< FAILED", instance->name);
            }
        }
    } else
#endif
    {
        const char *line = readLine(instance);
        if (line == NULL) {
            return;
        }
        handleLine(instance, line);
    }
}


/**
 * @brief 发送AT命令行并等待响应
 * @param[in] instance RILAT实例指针
 * @param[in] command 要发送的AT命令
 * @param[in] responsePrefix 期望的响应前缀
 * @param[in] pdu PDU数据指针
 * @param[out] pp_outResponse 返回的响应结构体指针
 * @param[in] sendCommand 命令类型
 * @param[in] timeout 超时时间（毫秒）
 * @return 0表示成功，-1表示失败
 * 
 * @details 该函数是AT命令处理的核心函数，负责发送命令、等待响应并处理超时。
 *          根据不同的命令类型采取不同的处理策略。
 */
static int32_t writeLine(Rilat_Instance_t *instance,
                         const char *command,
                         const char *responsePrefix,
                         void *pdu,
                         Rilat_AtResponse_t **pp_outResponse,
                         SendCommand_t sendCommand,
                         uint32_t timeout) {
    int32_t err = 0;
    size_t commandLen;

    // 检查实例是否忙碌
    if (instance->_flag.isBusy) {
        return -1;
    }

    // 检查是否有待处理的命令
    if (instance->_pending.commandResponse != NULL) {
        return -1;
    }

    // 设置忙碌标志
    instance->_flag.isBusy = 1;

    // 创建新的AT响应结构体
    instance->_pending.commandResponse = atResponseNew(instance);
    if (instance->_pending.commandResponse == NULL) {
        err = -1;
        instance->_flag.isBusy = 0;
        return err;
    }

    // 初始化命令参数
    instance->_pending.commandType = sendCommand;
    instance->_pending.commandResponsePrefix = (char *) responsePrefix;
    instance->_pending.smsPDU = pdu;

    // 流数据特殊处理
    if (instance->_pending.commandType == STREAM_DATA) {
        instance->_pending.commandResponse->stream = true;
        instance->recvBuffer._posEol = instance->recvBuffer.buffer;
        instance->recvBuffer._posRead = instance->recvBuffer.buffer;
        instance->recvBuffer._activeRecvLength = 0;
        memset(instance->recvBuffer.buffer, 0x00, instance->recvBuffer.size);
    }

    // 发送命令
    commandLen = strlen(command);
    writeEof(instance, (void *) command, commandLen, true);

    // 如果设置了超时，则等待响应
    if (timeout != 0) {
        qSTimer_t wait = QSTIMER_INITIALIZER;
        qSTimer_Set(&wait, timeout);

        while (instance->_pending.commandResponse->finalResponse == NULL) {
            if (qSTimer_Expired(&wait)) {
                err = -1;
                goto l_error;
            }

            if (instance->callback(instance, RILAT_CALL_EVENT_ON_BLOCK_WAIT,
                                   (Rilat_CallbackVar_t){},
                                   (Rilat_CallbackVar_t){},
                                   (Rilat_CallbackVar_t){}) != 0) {
                err = -1;
                goto l_error;
            }

            readDataLoop(instance);
        }
    } else {
        // 无超时情况直接设置成功
        instance->_pending.commandResponse->success = true;
    }

    // 处理响应
    if (pp_outResponse != NULL) {
        // 只有在需要返回响应时才反转中间响应行
        reverseIntermediates(instance->_pending.commandResponse);
        *pp_outResponse = instance->_pending.commandResponse;
    } else if (instance->_pending.commandResponse != NULL) {
        // 不需要返回响应时直接释放
        rilat_freeResponse(instance, instance->_pending.commandResponse);
    }

    // 清理并返回
    instance->_pending.commandResponse = NULL;
    instance->_flag.isBusy = 0;
    return 0;

l_error:
    // 错误处理
    if (instance->_pending.commandResponse != NULL) {
        rilat_freeResponse(instance, instance->_pending.commandResponse);
        instance->_pending.commandResponse = NULL;
    }
    instance->_flag.isBusy = 0;
    return err;
}


/**
 * @brief 处理未识别的命令行
 * @param[in] instance RILAT实例指针
 * @param[in] line 未识别的命令行
 * @return 无
 * 
 * @details 该函数处理未被当前命令识别的行数据，通常为被动接收的URC数据。
 *          根据配置的匹配器进行匹配并调用相应处理函数。
 */
static void _handleUnsolicited(Rilat_Instance_t *instance, const char *line) {
#if RILAT_MATCHER_ENABLE == 1
    const rilat_Matcher_t *matcher = findCommand(line);
    if (matcher == NULL) {
        return;
    }

    matcher->onMatch(line, instance->recvBuffer._posEol - line);
#endif
}

/*@}*/

/**
 * @addtogroup Base funcs
 * @note none
 */

/*@{*/

/**
 * @brief 初始化RILAT实例
 * @param[in] instance RILAT实例指针
 * @return 无
 * 
 * @details 该函数初始化RILAT实例的各项参数，包括接收缓冲区指针、
 *          命令状态和标志位，并调用用户定义的初始化回调函数。
 */
void rilat_init(Rilat_Instance_t *instance) {
    instance->recvBuffer._posRead = instance->recvBuffer.buffer;
    instance->recvBuffer._posEol = instance->recvBuffer.buffer;
    instance->_pending.commandResponsePrefix = NULL;
    instance->_pending.commandResponse = NULL;
    instance->_pending.commandType = 0;
    instance->_flag._ = 0;

    instance->callback(instance, RILAT_CALL_EVENT_ON_INIT,
                       (Rilat_CallbackVar_t){},
                       (Rilat_CallbackVar_t){},
                       (Rilat_CallbackVar_t){});
}


/**
 * @brief 终止RILAT实例
 * @param[in] instance RILAT实例指针
 * @return 无
 * 
 * @details 该函数终止RILAT实例，调用用户定义的终止回调函数，
 *          进行资源清理工作。
 */
void rilat_finalize(Rilat_Instance_t *instance) {
    instance->callback(instance, RILAT_CALL_EVENT_ON_FINALIZE,
                       (Rilat_CallbackVar_t){},
                       (Rilat_CallbackVar_t){},
                       (Rilat_CallbackVar_t){});
}


/**
 * @brief RILAT主循环处理函数
 * @param[in] instance RILAT实例指针
 * @return 0表示成功，-1表示失败
 * 
 * @details 该函数是RILAT的主处理循环，负责调用用户定义的轮询钩子函数
 *          并处理接收到的数据。应在主循环中定期调用此函数。
 */
int32_t rilat_loop(Rilat_Instance_t *instance) {
    int32_t rc = instance->callback(instance, RILAT_CALL_EVENT_ON_POLL_HOCK,
                                    (Rilat_CallbackVar_t){},
                                    (Rilat_CallbackVar_t){},
                                    (Rilat_CallbackVar_t){});
    if (rc < 0) {
        return -1;
    }
    readDataLoop(instance);
    return 0;
}

/*@}*/

/**
 * @addtogroup Direct func
 * @note none
 */

/*@{*/

/**
 * @brief 设置用户数据
 * @param[in] instance RILAT实例指针
 * @param[in] data 用户数据指针
 * @return 无
 * 
 * @details 该函数用于设置与RILAT实例关联的用户数据，
 *          可在回调函数中使用该数据。
 */
void rilat_setUserData(Rilat_Instance_t *instance, void *data) {
    instance->userData = data;
}


/**
 * @brief 获取用户数据
 * @param[in] instance RILAT实例指针
 * @return 用户数据指针
 * 
 * @details 该函数用于获取与RILAT实例关联的用户数据。
 */
void *rilat_getUserData(Rilat_Instance_t *instance) {
    return instance->userData;
}


/**
 * @brief 直接写入物理接口
 * @param[in] instance RILAT实例指针
 * @param[in] data 要写入的数据指针
 * @param[in] length 要写入的数据长度
 * @return 0表示成功，-1表示失败
 * 
 * @details 该函数直接将数据写入物理接口，不进行任何格式化处理。
 */
int32_t rilat_directWritePhy(Rilat_Instance_t *instance, uint8_t *data, uint16_t length) {
    uint32_t cur = 0;
    int32_t written;

    while (cur < length) {
        written = instance->callback(instance, RILAT_CALL_EVENT_ON_WRITE,
                                     (Rilat_CallbackVar_t){.ptr = (uint8_t *) data + cur},
                                     (Rilat_CallbackVar_t){.u32 = length - cur},
                                     (Rilat_CallbackVar_t){});
        if (written < 0) {
            return -1;
        }

        cur += written;
    }
    return 0;
}

/**
 * @brief 直接写入数据（无行结束符）
 * @param[in] instance RILAT实例指针
 * @param[in] data 要写入的数据指针
 * @param[in] length 要写入的数据长度
 * @param[in] isStreamData 是否为流数据
 * @return 0表示成功，-1表示失败
 * 
 * @details 该函数将数据写入物理接口，不添加行结束符。
 */
int32_t rilat_directWrite(Rilat_Instance_t *instance, uint8_t *data, uint16_t length, uint8_t isStreamData) {
    return writeNoeof(instance, (char *) data, length, isStreamData, true);
}


/**
 * @brief 直接写入数据（带行结束符）
 * @param[in] instance RILAT实例指针
 * @param[in] data 要写入的数据指针
 * @param[in] length 要写入的数据长度
 * @return 0表示成功，-1表示失败
 * 
 * @details 该函数将数据写入物理接口，并在末尾添加\r\n行结束符。
 */
int32_t rilat_directWriteWithEOF(Rilat_Instance_t *instance, uint8_t *data, uint16_t length) {
    return writeEof(instance, (char *) data, length, true);
}


/**
 * @brief 直接写入数据（带Ctrl+Z结束符）
 * @param[in] instance RILAT实例指针
 * @param[in] data 要写入的数据指针
 * @param[in] length 要写入的数据长度
 * @return 0表示成功，-1表示失败
 * 
 * @details 该函数将数据写入物理接口，并在末尾添加Ctrl+Z(0x1A)结束符。
 */
int32_t rilat_directWriteWithCtlZ(Rilat_Instance_t *instance, uint8_t *data, uint16_t length) {
    return writeCtlZ(instance, (char *) data, length, true);
}


/// @Deprecated
inline int32_t rilat_directSetPdu(Rilat_Instance_t *instance, uint8_t *data, uint16_t length, uint8_t withCtrlZ) {
    instance->_flag.sendPDUCtrlZ = withCtrlZ;
    instance->_pending.smsPDULength = length;
    instance->_pending.smsPDU = (char *) data;

    return 0;
}


/**
 * @brief 重置接收缓冲区
 * @param[in] instance RILAT实例指针
 * @return 无
 * 
 * @details 该函数重置接收缓冲区的读取指针和相关状态，
 *          清空缓冲区内容，为新的数据接收做准备。
 */
void rilat_directResetReceiveBuffer(Rilat_Instance_t *instance) {
    instance->recvBuffer._posRead = instance->recvBuffer.buffer;
    instance->recvBuffer._posEol = instance->recvBuffer.buffer;
    memset(instance->recvBuffer.buffer, 0x00, instance->recvBuffer.size);
    instance->recvBuffer._activeRecvLength = 0;
}


/**
 * @brief 检查是否接收到Ctrl+Z字符
 * @param[in] instance RILAT实例指针
 * @return true表示接收到Ctrl+Z字符，false表示未接收到
 * @details 用于检查接收缓冲区中是否包含Ctrl+Z(0x1A)字符，通常用于判断PDU数据传输是否完成
 */
bool rilat_readForCtlZ(Rilat_Instance_t *instance) {
    int32_t rc = instance->callback(instance, RILAT_CALL_EVENT_ON_POLL_HOCK,
                                (Rilat_CallbackVar_t){},
                                (Rilat_CallbackVar_t){},
                                (Rilat_CallbackVar_t){});
    if (rc < 0) {
        return false;
    }
    const char *line = readLine(instance);
    if (line == NULL) {
        return false;
    }
    if (0 == strcmp(line, ">")) {
        return true;
    }
    handleLine(instance, line);
    return false;
}


/*@}*/

/**
 * @addtogroup Block command func
 * @note none
 */

/*@{*/

/**
 * @brief 释放AT响应结构体
 * @param[in] instance RILAT实例指针
 * @param[in] p_response 要释放的AT响应结构体指针
 * @return 无
 * 
 * @details 该函数释放AT响应结构体及其包含的所有中间响应行和最终响应数据，
 *          防止内存泄漏。
 */
void rilat_freeResponse(Rilat_Instance_t *instance, Rilat_AtResponse_t *p_response) {
    if (p_response == NULL) {
        return;
    }

    Rilat_AtLine_t *p_line;
    p_line = p_response->intermediates;

    while (p_line != NULL) {
        Rilat_AtLine_t *p_toFree;

        p_toFree = p_line;
        p_line = p_line->next;

        rilat_free(instance, p_toFree->line);
        rilat_free(instance, p_toFree);
    }

    if (!p_response->stream) {
        rilat_free(instance, p_response->finalResponse);
    }
    rilat_free(instance, p_response);
}


/**
 * @brief 发送带流数据的AT命令
 * @param[in] instance RILAT实例指针
 * @param[in] command 要发送的AT命令
 * @param[in] pduStartPerFix PDU开始前缀
 * @param[in] responsePrefix 期望的响应前缀
 * @param[out] pp_outResponse 返回的响应结构体指针
 * @param[in] timeout 超时时间（毫秒）
 * @return 0表示成功，-1表示失败
 * 
 * @details 该函数用于发送需要处理流数据的AT命令，例如数据传输命令。
 */
int32_t rilat_writeLineWithStreamData(Rilat_Instance_t *instance,
                                      const char *command,
                                      const char *pduStartPerFix,
                                      const char *responsePrefix,
                                      Rilat_AtResponse_t **pp_outResponse,
                                      uint32_t timeout) {
    if (pduStartPerFix == NULL) {
        return -1;
    }

    instance->_flag.onlyMathPduTag = 0;
    instance->_pending.pduStartPrefix = (char *) pduStartPerFix;
    return writeLine(instance, command, responsePrefix, NULL, pp_outResponse, STREAM_DATA, timeout);
}


/**
 * @brief 发送AT命令
 * @param[in] instance RILAT实例指针
 * @param[in] command 要发送的AT命令
 * @param[out] pp_outResponse 返回的响应结构体指针
 * @param[in] timeout 超时时间（毫秒）
 * @return 0表示成功，-1表示失败
 * 
 * @details 该函数用于发送不需要中间响应的AT命令。
 */
int32_t rilat_writeLine(Rilat_Instance_t *instance,
                        const char *command,
                        Rilat_AtResponse_t **pp_outResponse,
                        uint32_t timeout) {
    instance->_flag.onlyMathPduTag = 0;
    return writeLine(instance, command, NULL, NULL, pp_outResponse, NO_RESULT, timeout);
}


/**
 * @brief 发送单行响应AT命令
 * @param[in] instance RILAT实例指针
 * @param[in] command 要发送的AT命令
 * @param[in] responsePrefix 期望的响应前缀
 * @param[out] pp_outResponse 返回的响应结构体指针
 * @param[in] timeout 超时时间（毫秒）
 * @return 0表示成功，-1表示失败
 * 
 * @details 该函数用于发送期望获得单行指定前缀响应的AT命令。
 *          如果未收到匹配的中间响应，函数将返回失败。
 */
int32_t rilat_writeSingleline(Rilat_Instance_t *instance,
                              const char *command,
                              const char *responsePrefix,
                              Rilat_AtResponse_t **pp_outResponse,
                              uint32_t timeout) {
    instance->_flag.onlyMathPduTag = 0;
    int32_t rc = writeLine(instance, command, responsePrefix, NULL, pp_outResponse, SINGLELINE, timeout);

    if (rc == 0
        && pp_outResponse != NULL
        && (*pp_outResponse)->success > 0
        && (*pp_outResponse)->intermediates == NULL) {
        rilat_freeResponse(instance, *pp_outResponse);
        *pp_outResponse = NULL;

        return -1;
    }

    return rc;
}


/**
 * @brief 发送单行前缀匹配AT命令
 * @param[in] instance RILAT实例指针
 * @param[in] command 要发送的AT命令
 * @param[in] responsePrefix 期望的响应前缀
 * @param[out] pp_outResponse 返回的响应结构体指针
 * @param[in] timeout 超时时间（毫秒）
 * @return 0表示成功，-1表示失败
 * 
 * @details 该函数用于发送仅当前缀匹配时即认为命令成功的AT命令。
 *          与普通单行响应命令不同，只要匹配到前缀就认为命令成功。
 */
int32_t rilat_writeSinglelineOnlyPrefixMatched(Rilat_Instance_t *instance,
                                               const char *command,
                                               const char *responsePrefix,
                                               Rilat_AtResponse_t **pp_outResponse,
                                               uint32_t timeout) {
    instance->_flag.onlyMathPduTag = 0;
    int32_t rc = writeLine(instance, command, responsePrefix, NULL, pp_outResponse, SINGLELINE_ONLY_PREFIX_MATCH,
                           timeout);
    if (rc == 0
        && pp_outResponse != NULL
        && (*pp_outResponse)->success > 0
        && (*pp_outResponse)->intermediates == NULL) {
        rilat_freeResponse(instance, *pp_outResponse);
        *pp_outResponse = NULL;
        return -1;
    }

    return rc;
}


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
 * 
 * @details 该函数用于发送需要附带PDU数据的AT命令，例如短信发送命令。
 *          支持多种PDU处理选项，如使用Ctrl+Z结束符、直接写入等。
 */
int32_t rilat_writeLineWithPDU(Rilat_Instance_t *instance,
                               const char *command,
                               const char *pduStartPerFix,
                               void *pdu,
                               uint16_t pduLength,
                               Rilat_PduFlag_t pduFlag,
                               uint32_t packMsgId,
                               Rilat_AtResponse_t **pp_outResponse,
                               uint32_t timeout) {
    instance->_flag.sendPDUCtrlZ = pduFlag & RILAT_PDU_S_WITH_CTLZ ? 1 : 0;
    instance->_flag.onlyMathPduTag = pduFlag & RILAT_PDU_S_ONLY_MATCH_TAG ? 1 : 0;
    instance->_flag.pduDirectWrite = pduFlag & RILAT_PDU_S_DIRECT_WRITE ? 1 : 0;
    instance->_pending.smsPDULength = pduLength;
    instance->_pending.pduStartPrefix = (char *) pduStartPerFix;
    instance->_pending.pduDirectWriteMsgId = packMsgId;
    int32_t rc = writeLine(instance, command, NULL, pdu, pp_outResponse,
                           pduFlag & RILAT_PDU_S_RESP_ONLY_MATCH ? SINGLELINE_ONLY_PREFIX_MATCH : NO_RESULT,
                           timeout);
    return rc;
}


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
 * 
 * @details 该函数用于发送需要附带PDU数据且期望获得单行指定前缀响应的AT命令。
 *          如果未收到匹配的中间响应，函数将返回失败。
 */
int32_t rilat_writeSinglelineWithPDU(Rilat_Instance_t *instance,
                                     const char *command,
                                     const char *pduStartPerFix,
                                     void *pdu,
                                     uint16_t pduLength,
                                     Rilat_PduFlag_t pduFlag,
                                     const char *responsePrefix,
                                     uint32_t packMsgId,
                                     Rilat_AtResponse_t **pp_outResponse,
                                     uint32_t timeout) {
    instance->_flag.sendPDUCtrlZ = (pduFlag & RILAT_PDU_S_WITH_CTLZ) ? 1 : 0;
    instance->_flag.onlyMathPduTag = (pduFlag & RILAT_PDU_S_ONLY_MATCH_TAG) ? 1 : 0;
    instance->_pending.smsPDULength = pduLength;
    instance->_flag.pduDirectWrite = pduFlag & RILAT_PDU_S_DIRECT_WRITE ? 1 : 0;
    instance->_pending.pduStartPrefix = (char *) pduStartPerFix;
    instance->_pending.pduDirectWriteMsgId = packMsgId;
    int32_t rc = writeLine(instance, command, responsePrefix, pdu, pp_outResponse,
                           pduFlag & RILAT_PDU_S_RESP_ONLY_MATCH ? SINGLELINE_ONLY_PREFIX_MATCH : SINGLELINE,
                           timeout);

    if (rc == 0
        && pp_outResponse != NULL
        && (*pp_outResponse)->success > 0
        && (*pp_outResponse)->intermediates == NULL) {
        rilat_freeResponse(instance, *pp_outResponse);
        *pp_outResponse = NULL;

        return -1;
    }

    return rc;
}


/**
 * @brief 发送多行响应AT命令
 * @param[in] instance RILAT实例指针
 * @param[in] command 要发送的AT命令
 * @param[in] responsePrefix 期望的响应前缀
 * @param[out] pp_outResponse 返回的响应结构体指针
 * @param[in] timeout 超时时间（毫秒）
 * @return 0表示成功，-1表示失败
 * 
 * @details 该函数用于发送期望获得多行指定前缀响应的AT命令。
 */
int32_t rilat_writeMultiline(Rilat_Instance_t *instance,
                             const char *command,
                             const char *responsePrefix,
                             Rilat_AtResponse_t **pp_outResponse,
                             uint32_t timeout) {
    instance->_flag.onlyMathPduTag = 0;
    return writeLine(instance, command, responsePrefix, NULL, pp_outResponse, MULTILINE, timeout);
}


/**
 * @brief 发送数值响应AT命令
 * @param[in] instance RILAT实例指针
 * @param[in] command 要发送的AT命令
 * @param[out] pp_outResponse 返回的响应结构体指针
 * @param[in] timeout 超时时间（毫秒）
 * @return 0表示成功，-1表示失败
 * 
 * @details 该函数用于发送期望获得以数字开头的单行响应的AT命令。
 *          如果未收到匹配的中间响应，函数将返回失败。
 */
int32_t rilat_writeNumericLine(Rilat_Instance_t *instance,
                               const char *command,
                               Rilat_AtResponse_t **pp_outResponse,
                               uint32_t timeout) {
    instance->_flag.onlyMathPduTag = 0;
    int32_t rc = writeLine(instance, command, NULL, NULL, pp_outResponse, NUMERIC, timeout);

    if (rc == 0
        && pp_outResponse != NULL
        && (*pp_outResponse)->success > 0
        && (*pp_outResponse)->intermediates == NULL) {
        rilat_freeResponse(instance, *pp_outResponse);
        *pp_outResponse = NULL;

        return -1;
    }

    return rc;
}

/*@}*/

/**
 * @addtogroup DebugSupport
 * @note none
 */

/*@{*/

#undef DBG_SECTION_NAME
#undef TAG

/*@}*/