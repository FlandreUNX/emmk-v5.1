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
    SINGLELINE_ONLY_PREFIX_MATCH, // Resoponse success only when prefex matched */
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


static void clearPendingCommand(Rilat_Instance_t *instance) {
    if (instance->_pending.commandResponse != NULL) {
        rilat_freeResponse(instance, instance->_pending.commandResponse);
    }

    instance->_pending.commandResponse = NULL;
    instance->_pending.commandResponsePrefix = NULL;
    instance->_pending.smsPDU = NULL;
    instance->_pending.pduStartPrefix = NULL;
}


static Rilat_AtResponse_t *atResponseNew(Rilat_Instance_t *instance) {
    Rilat_AtResponse_t *p = rilat_malloc(instance, sizeof(Rilat_AtResponse_t));
    if (p == NULL) {
        LOG_W("atResponseNew, AllocFailed");
        return NULL;
    }

    memset(p, 0x00, sizeof(Rilat_AtResponse_t));
    return p;
}


//static uint8_t xStrswith(const char *line, const char *prefix) {
//    for (; *line != '\0' && *prefix != '\0'; line++, prefix++) {
//        if (*line != *prefix) {
//            return 0;
//        }
//    }
//
//    return *prefix == '\0';
//}


static char *xMemstr(char *srcStr, int32_t srcStrLength, char *subStr) {
    if (srcStr == NULL || srcStrLength <= 0 || subStr == NULL) {
        return NULL;
    }
    if (*subStr == '\0') {
        return NULL;
    }
    uint16_t sublen = strlen(subStr);
    if (sublen > srcStrLength || sublen == 0) {
        return NULL;
    }

    char *cur = srcStr;
    int16_t lastPossible = srcStrLength - sublen + 1;
    if (lastPossible <= 0) {
        return NULL;
    }

    for (uint16_t i = 0; i < lastPossible; i++) {
        if (*cur == *subStr) {
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
        p_eol = _findNextEOL(instance, instance->recvBuffer._posRead);
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
            p_eol = _findNextEOL(instance, instance->recvBuffer._posRead);
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


static char *isFinalResponseError(const char *line, uint32_t len) {
    for (uint8_t i = 0; i < NUM_ELEMS(_FINAL_RESPONSE_ERROR); i++) {
        char *s = xMemstr((char *) line, (int32_t) (len == 0 ? strlen(line) : len), (char *) _FINAL_RESPONSE_ERROR[i]);
        if (s) {
            return s;
        }
    }
    return NULL;
}


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


static void handleFinalResponse(Rilat_Instance_t *instance, const char *line) {
    instance->_pending.commandResponse->finalResponse = rilat_malloc(instance, strlen(line) + 1);
    ASSERT(instance->_pending.commandResponse->finalResponse != NULL);

    memset(instance->_pending.commandResponse->finalResponse, 0x00, strlen(line) + 1);
    strcpy(instance->_pending.commandResponse->finalResponse, line);
}


static int32_t writeNoeof(Rilat_Instance_t *instance, void *s, uint16_t len, uint8_t isStreamData, bool skipFlag) {
    uint32_t cur = 0;
    int32_t written;

    if (!skipFlag && instance->_flag.pduDirectWrite) {
        Rilat_PduDirectWrite_t call = s;
        call(instance, instance->_pending.pduDirectWriteMsgId);
    } else {
        if (instance->transmitBuffer.buffer != NULL && instance->transmitBuffer.size >= len) {
            memcpy(instance->transmitBuffer.buffer, s, len);

            while (cur < len) {
                written = instance->callback(instance, RILAT_CALL_EVENT_ON_WRITE,
                                             (Rilat_CallbackVar_t){
                                                 .ptr = (uint8_t *) instance->transmitBuffer.buffer +
                                                        cur
                                             },
                                             (Rilat_CallbackVar_t){.u32 = len - cur},
                                             (Rilat_CallbackVar_t){});
                if (written < 0) {
                    return -1;
                }

                cur += written;
            }
        } else {
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
        }

#if EMMK_CFG_DEBUG_ENABLE == 1 && RILAT_DIRECT_WRITE_LOG_SUPPORT == 1
        if (!isStreamData) {
            LOG_I("%s> %s", instance->name, (char *) s);
        }
#endif
    }

    return 0;
}


static int32_t writeEof(Rilat_Instance_t *instance, void *s, uint16_t len, bool skipFlag) {
    uint32_t cur = 0;
    int32_t written;

    if (!skipFlag && instance->_flag.pduDirectWrite) {
        Rilat_PduDirectWrite_t call = s;
        call(instance, instance->_pending.pduDirectWriteMsgId);
        written = instance->callback(instance, RILAT_CALL_EVENT_ON_WRITE,
                                     (Rilat_CallbackVar_t){.ptr = (uint8_t *) "\r\n"},
                                     (Rilat_CallbackVar_t){.u32 = 2},
                                     (Rilat_CallbackVar_t){});
        if (written < 0) {
            return -1;
        }
#if EMMK_CFG_DEBUG_ENABLE == 1 && RILAT_DIRECT_WRITE_LOG_SUPPORT == 1
        LOG_I("%s> ", instance->name);
#endif
    } else {
        if (s != NULL && len != 0) {
            if (instance->transmitBuffer.buffer != NULL && instance->transmitBuffer.size >= (len + 2)) {
                memcpy(instance->transmitBuffer.buffer, s, len);
                memcpy(instance->transmitBuffer.buffer + len, "\r\n", 2);
                len += 2;

                while (cur < len) {
                    written = instance->callback(instance, RILAT_CALL_EVENT_ON_WRITE,
                                                 (Rilat_CallbackVar_t){
                                                     .ptr =
                                                     (uint8_t *) instance->transmitBuffer.buffer + cur
                                                 },
                                                 (Rilat_CallbackVar_t){.u32 = len - cur},
                                                 (Rilat_CallbackVar_t){});
                    if (written < 0) {
                        return -1;
                    }

                    cur += written;
                }
            } else {
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

                written = instance->callback(instance, RILAT_CALL_EVENT_ON_WRITE,
                                             (Rilat_CallbackVar_t){.ptr = (uint8_t *) "\r\n"},
                                             (Rilat_CallbackVar_t){.u32 = 2},
                                             (Rilat_CallbackVar_t){});
                if (written < 0) {
                    return -1;
                }
            }
#if EMMK_CFG_DEBUG_ENABLE == 1 && RILAT_TX_LOG_SUPPORT == 1
            LOG_I("%s> %s", instance->name, (char *) s);
#endif
        } else {
            written = instance->callback(instance, RILAT_CALL_EVENT_ON_WRITE,
                                         (Rilat_CallbackVar_t){.ptr = (uint8_t *) "\r\n"},
                                         (Rilat_CallbackVar_t){.u32 = 2},
                                         (Rilat_CallbackVar_t){});
            if (written < 0) {
                return -1;
            }
#if EMMK_CFG_DEBUG_ENABLE == 1 && RILAT_TX_LOG_SUPPORT == 1
            LOG_I("%s> ", instance->name);
#endif
        }
    }

    return 0;
}


static int32_t writeCtlZ(Rilat_Instance_t *instance, void *s, uint16_t len, bool skipFlag) {
    static const char CTRL_Z = 0x1A;

    uint32_t cur = 0;
    int32_t written;

    if (!skipFlag && instance->_flag.pduDirectWrite) {
        Rilat_PduDirectWrite_t call = s;
        call(instance, instance->_pending.pduDirectWriteMsgId);
        written = instance->callback(instance, RILAT_CALL_EVENT_ON_WRITE,
                                     (Rilat_CallbackVar_t){.ptr = (uint8_t *) &CTRL_Z},
                                     (Rilat_CallbackVar_t){.u32 = 1},
                                     (Rilat_CallbackVar_t){});
        if (written < 0) {
            return -1;
        }
    } else {
        if (s != NULL && len != 0) {
            if (instance->transmitBuffer.buffer != NULL && instance->transmitBuffer.size >= (len + 1)) {
                memcpy(instance->transmitBuffer.buffer, s, len);
                memcpy(instance->transmitBuffer.buffer + len, &CTRL_Z, 1);
                len += 1;

                while (cur < len) {
                    written = instance->callback(instance, RILAT_CALL_EVENT_ON_WRITE,
                                                 (Rilat_CallbackVar_t){
                                                     .ptr =
                                                     (uint8_t *) instance->transmitBuffer.buffer + cur
                                                 },
                                                 (Rilat_CallbackVar_t){.u32 = len - cur},
                                                 (Rilat_CallbackVar_t){});
                    if (written < 0) {
                        return -1;
                    }

                    cur += written;
                }
            } else {
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

                written = instance->callback(instance, RILAT_CALL_EVENT_ON_WRITE,
                                             (Rilat_CallbackVar_t){.ptr = (uint8_t *) &CTRL_Z},
                                             (Rilat_CallbackVar_t){.u32 = 1},
                                             (Rilat_CallbackVar_t){});
                if (written < 0) {
                    return -1;
                }
            }
        } else {
            written = instance->callback(instance, RILAT_CALL_EVENT_ON_WRITE,
                                         (Rilat_CallbackVar_t){.ptr = (uint8_t *) &CTRL_Z},
                                         (Rilat_CallbackVar_t){.u32 = 1},
                                         (Rilat_CallbackVar_t){});
            if (written < 0) {
                return -1;
            }
        }
    }
#if EMMK_CFG_DEBUG_ENABLE == 1 && RILAT_TX_LOG_SUPPORT == 1
    LOG_I("%s> %d,Z", instance->name, len);
#endif
    return 0;
}


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


static int32_t writeLine(Rilat_Instance_t *instance,
                         const char *command,
                         const char *responsePrefix,
                         void *pdu,
                         Rilat_AtResponse_t **pp_outResponse,
                         SendCommand_t sendCommand,
                         uint32_t timeout) {
    int32_t err = 0;

    if (instance->_flag.isBusy) {
        err = -1;
        goto l_error;
    }

    if (instance->_pending.commandResponse != NULL) {
        err = -1;
        goto l_error;
    } else {
        instance->_flag.isBusy = 1;

        instance->_pending.commandResponse = atResponseNew(instance);
        if (instance->_pending.commandResponse == NULL) {
            err = -1;
            goto l_error;
        }

        instance->_pending.commandType = sendCommand;
        instance->_pending.commandResponsePrefix = (char *) responsePrefix;
        instance->_pending.smsPDU = pdu;

        if (instance->_pending.commandType == STREAM_DATA) {
            instance->_pending.commandResponse->stream = true;
            instance->recvBuffer._posEol = instance->recvBuffer.buffer;
            instance->recvBuffer._posRead = instance->recvBuffer.buffer;
            instance->recvBuffer._activeRecvLength = 0;
            memset(instance->recvBuffer.buffer, 0x00, instance->recvBuffer.size);
        }

        writeEof(instance, (void *) command, strlen(command), true);

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
            instance->_pending.commandResponse->success = true;
        }
    }

    if (pp_outResponse == NULL) {
        rilat_freeResponse(instance, instance->_pending.commandResponse);
    } else {
        reverseIntermediates(instance->_pending.commandResponse);
        *pp_outResponse = instance->_pending.commandResponse;
    }

    instance->_pending.commandResponse = NULL;
    err = 0;

l_error:
    clearPendingCommand(instance);
    instance->_flag.isBusy = 0;

    return err;
}


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


void rilat_finalize(Rilat_Instance_t *instance) {
    instance->callback(instance, RILAT_CALL_EVENT_ON_FINALIZE,
                       (Rilat_CallbackVar_t){},
                       (Rilat_CallbackVar_t){},
                       (Rilat_CallbackVar_t){});
}


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

void rilat_setUserData(Rilat_Instance_t *instance, void *data) {
    instance->userData = data;
}


void *rilat_getUserData(Rilat_Instance_t *instance) {
    return instance->userData;
}


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

int32_t rilat_directWrite(Rilat_Instance_t *instance, uint8_t *data, uint16_t length, uint8_t isStreamData) {
    return writeNoeof(instance, (char *) data, length, isStreamData, true);
}


int32_t rilat_directWriteWithEOF(Rilat_Instance_t *instance, uint8_t *data, uint16_t length) {
    return writeEof(instance, (char *) data, length, true);
}


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


void rilat_directResetReceiveBuffer(Rilat_Instance_t *instance) {
    instance->recvBuffer._posRead = instance->recvBuffer.buffer;
    instance->recvBuffer._posEol = instance->recvBuffer.buffer;
    memset(instance->recvBuffer.buffer, 0x00, instance->recvBuffer.size);
    instance->recvBuffer._activeRecvLength = 0;
}

/*@}*/

/**
 * @addtogroup Block command func
 * @note none
 */

/*@{*/

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


int32_t rilat_writeLine(Rilat_Instance_t *instance,
                        const char *command,
                        Rilat_AtResponse_t **pp_outResponse,
                        uint32_t timeout) {
    instance->_flag.onlyMathPduTag = 0;
    return writeLine(instance, command, NULL, NULL, pp_outResponse, NO_RESULT, timeout);
}


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


int32_t rilat_writeMultiline(Rilat_Instance_t *instance,
                             const char *command,
                             const char *responsePrefix,
                             Rilat_AtResponse_t **pp_outResponse,
                             uint32_t timeout) {
    instance->_flag.onlyMathPduTag = 0;
    return writeLine(instance, command, responsePrefix, NULL, pp_outResponse, MULTILINE, timeout);
}


int32_t rilat_writeMumericLine(Rilat_Instance_t *instance,
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
