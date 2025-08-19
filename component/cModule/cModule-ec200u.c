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

#include "./global.h"

/**
 * @addtogroup DebugSupport
 * @note none
 */

/*@{*/

#undef TAG
#define TAG     "ec200u"

/*@}*/

/**
 * @addtogroup ComponentSupport
 * @note none
 */

/*@{*/

#define _MODULE_REQUEST_TYPE(x)           CMODULE_REQ_##x
#define _MODULE_REQUEST_VARS              cModule_RequestVar_t
#define _MODULE_REQUEST_TYPED             cModule_RequestType_t
#define _MODULE_REQUEST_CALL              cModule_onRequestCallback
#define CREQUEST(type, var, ...)        _MODULE_REQUEST_CALL(_MODULE_REQUEST_TYPE(type), (_MODULE_REQUEST_VARS) var, ##__VA_ARGS__)

extern _MODULE_REQUEST_VARS _MODULE_REQUEST_CALL(_MODULE_REQUEST_TYPED type, _MODULE_REQUEST_VARS var, ...);

/*@}*/

/**
 * @addtogroup Config
 * @note none
 */

/*@{*/

/// @-1=Disable
#if CONFIG_CMODULE_INSTANCE_X_ENABLE == 55
#define CONFIG_CMODULE_INSTANCE_TAG_ID                    55
#else
#define CONFIG_CMODULE_INSTANCE_TAG_ID                    -1
#endif

/*@}*/

#if CONFIG_CMODULE_INSTANCE_TAG_ID >= 0

CMPMU_DEFINE();

/**
 * @addtogroup Protocol
 * @note none
 */

/*@{*/

struct {
    uint16_t mqttMessageId;
    int8_t fd;
} static mProtocol;

/*@}*/

/**
 * @addtogroup Private constants
 * @note none
 */

/*@{*/



/*@}*/

/**
 * @addtogroup Private vars
 * @note none
 */

/*@{*/

static void onInstanceInit(void);

static void onInstancePmu(uint8_t wakeup1sleep0);

static int32_t checkAT(void);

static int32_t AT(void);

static void onReboot(uint8_t isPowerUpRequest);

static void onLoop(void);

static void onInitStep(uint8_t step);

static void onResetStack(void);

static int32_t onPtPackIdReceived(cModule_TransmitPackageInfo_t *info);

static int32_t onPtPackIdTransmit(cModule_TransmitPackageInfo_t *info);

static int32_t onPtPackIdCustom(cModule_TransmitPackageInfo_t *info);

static int32_t onPtPackPayloadFree(cModule_TransmitPackageInfo_t *info, bool isForce);

static const cModule_Callback_t mCallback = {
        .onInstanceInit = onInstanceInit,
        .onInstancePmu = onInstancePmu,

        .onCommandAT = AT,
        .onCommandCheckAT = checkAT,

        .onReboot = onReboot,
        .onLoop = onLoop,
        .onInitStep = onInitStep,
        .onResetStack = onResetStack,

        .onPtPackIdReceived = onPtPackIdReceived,
        .onPtPackIdTransmit = onPtPackIdTransmit,
        .onPtPackIdCustom = onPtPackIdCustom,
        .onPtPackPayloadFree = onPtPackPayloadFree,
};

static uint8_t mModuleRxBuffer[CONFIG_CMODULE_RILAT_MEM_RECV_BUFFER_SIZE] = {0};
static cModule_Instance_t mModuleInstance = {
        .callback = &mCallback,
        .cmmpu = &CMPMU,
        .rilat = {
                .tagId = CONFIG_CMODULE_INSTANCE_TAG_ID,
                .recvBuffer = mModuleRxBuffer,
        },
};
const __CMODULE_X_SECTION cModule_InstanceConst_t mModuleInstanceConst = {
        .ins = &mModuleInstance,
};

/*@}*/

/**
 * @addtogroup AtCommand
 * @note none
 */

/*@{*/

static int32_t checkAT(void) {
    static const char AT_A[] = "AT";

    uint8_t isSuccess = 0;
    Rilat_AtResponse_t *response = NULL;

    CREQUEST(ON_SERIAL_FLUSH, {&mModuleInstance});

    rilat_writeLine(&mModuleInstance.rilat.instance, AT_A, NULL, 500);
    for (uint8_t i = 0; i < 2; i++) {
        rilat_writeLine(&mModuleInstance.rilat.instance, AT_A, &response, 500);
        if (response != NULL && response->success) {
            rilat_freeResponse(&mModuleInstance.rilat.instance, response);
            response = NULL;

            isSuccess = 1;
            break;
        }

        rilat_freeResponse(&mModuleInstance.rilat.instance, response);
        response = NULL;

        if (cModule_isBlocking(&mModuleInstance)) {
            isSuccess = 0;
            break;
        }
    }

    if (isSuccess) {
    } else {
        mModuleInstance.aux.initRetryCount++;
        return -1;
    }

    return 0;
}


static int32_t AT(void) {
    Rilat_AtResponse_t *response = NULL;

    rilat_writeLine(&mModuleInstance.rilat.instance, "AT", &response, 1000);
    if (response == NULL || response->success == 0) {
        rilat_freeResponse(&mModuleInstance.rilat.instance, response);
        return -1;
    }

    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return 0;
}


static inline int32_t ATE0(void) {
    static const char AT_ATE0[] = "ATE0";

    rilat_writeLine(&mModuleInstance.rilat.instance, AT_ATE0, NULL, 500);
    rilat_writeLine(&mModuleInstance.rilat.instance, AT_ATE0, NULL, 500);
    rilat_writeLine(&mModuleInstance.rilat.instance, AT_ATE0, NULL, 500);
    rilat_writeLine(&mModuleInstance.rilat.instance, "AT+QGMR", NULL, 500);

    return 0;
}


static int32_t checkCPIN(void) {
    static const char AT_CPIN_A[] = "AT+CPIN?";

    Rilat_AtResponse_t *response = NULL;

    if (rilat_writeSingleline(&mModuleInstance.rilat.instance, AT_CPIN_A, "+CPIN:", &response, 2000) != 0 ||
        response == NULL || response->success == 0) {
        goto _l_retryExit;
    }

    if (strstr(response->intermediates->line, "READY") == NULL) {
        goto _l_retryExit;
    }

    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return 0;

    _l_retryExit:
    mModuleInstance.aux.initRetryCount++;
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return 1;
}


static int32_t checkCGREG(void) {
    static const char AT_CGREG_A[] = "AT+CEREG?";

    char *line = NULL;
    int32_t numberResult = -1;
    Rilat_AtResponse_t *response = NULL;

    if (rilat_writeSingleline(&mModuleInstance.rilat.instance, AT_CGREG_A, "+CEREG:", &response, 600) != 0 ||
        response == NULL || response->success == 0) {
        LOG_E("1");
        goto _l_retryExit;
    }

    if (response->intermediates == NULL) {
        LOG_E("2");
        goto _l_retryExit;
    }
    line = response->intermediates->line;

    if (klAttoken_start(&line) != 0) {
        LOG_E("3");
        goto _l_retryExit;
    }

    LOG_I("%s", line);
    if (klAttoken_getNextInt(&line, &numberResult) != 0) {
        LOG_E("4");
        goto _l_retryExit;
    }

    if (numberResult != 0) {
        LOG_E("5");
        goto _l_retryExit;
    }

    if (klAttoken_getNextInt(&line, &numberResult) != 0) {
        LOG_E("6");
        goto _l_retryExit;
    }

    if (!(numberResult == 1 || numberResult == 5)) {
        LOG_E("78");
        goto _l_retryExit;
    }

    rilat_freeResponse(&mModuleInstance.rilat.instance, response);

    rilat_writeLine(&mModuleInstance.rilat.instance, "ATI", NULL, 500);
    rilat_writeLine(&mModuleInstance.rilat.instance, "AT+QENG=\"servingcell\"", NULL, 500);
    rilat_writeLine(&mModuleInstance.rilat.instance, "AT+COLP=1", NULL, 15000);
    rilat_writeLine(&mModuleInstance.rilat.instance, "AT^DSCI=1", NULL, 15000);
    rilat_writeLine(&mModuleInstance.rilat.instance, "AT+QURCCFG=\"URCPORT\",\"UART1\"", NULL, 15000);

    return 0;

    _l_retryExit:
    mModuleInstance.aux.initRetryCount++;
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return 1;
}


static int32_t queryIMEI(void) {
    static const char AT_CGSN[] = "AT+CGSN";

    Rilat_AtResponse_t *response = NULL;

    if (rilat_writeMultiline(&mModuleInstance.rilat.instance, AT_CGSN, "", &response, 1000) != 0 || response == NULL ||
        response->success == 0) {
        goto _l_retryExit;
    }

    if (response->intermediates == NULL) {
        goto _l_retryExit;
    }
    strcpy(mModuleInstance.modemInfo.imei, response->intermediates->line);

    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return 0;

    _l_retryExit:
    mModuleInstance.aux.initRetryCount++;
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return 1;
}


static int32_t queryIMSI(void) {
    static const char AT_CIMI[] = "AT+CIMI";

    Rilat_AtResponse_t *response = NULL;

    if (rilat_writeMultiline(&mModuleInstance.rilat.instance, AT_CIMI, "", &response, 1000) != 0 || response == NULL ||
        response->success == 0) {
        goto _l_retryExit;
    }

    if (response->intermediates == NULL) {
        goto _l_retryExit;
    }

    bool vailed = true;
    memset(mModuleInstance.modemInfo.imsi, 0x00, sizeof(mModuleInstance.modemInfo.imsi));
    strcpy(mModuleInstance.modemInfo.imsi, response->intermediates->line);
    for (uint32_t i = 0; i < strlen(mModuleInstance.modemInfo.imsi); i++) {
        if (!isdigit((int) mModuleInstance.modemInfo.imsi[i])) {
            vailed = false;
            break;
        }
    }
    if (!vailed) {
        goto _l_retryExit;
    }

    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return 0;

    _l_retryExit:
    mModuleInstance.aux.initRetryCount++;
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return 1;
}


static int32_t readIccid(void) {
    static const char AT_CSQ[] = "AT+QCCID";

    int8_t rc = -1;
    Rilat_AtResponse_t *response = NULL;

    if (rilat_writeSingleline(&mModuleInstance.rilat.instance, AT_CSQ, "+QCCID:", &response, 1000) != 0 ||
        response == NULL || response->success == 0) {
        goto _l_retryExit;
    }

    if (response->intermediates == NULL) {
        goto _l_retryExit;
    }
    char *line = response->intermediates->line;
    if (klAttoken_start(&line) != 0) {
        goto _l_retryExit;
    }
    while (isspace((unsigned char) *line) != 0) {
        line++;
    }
    memset(mModuleInstance.modemInfo.iccid, 0x00, sizeof(mModuleInstance.modemInfo.iccid));
    strncpy(mModuleInstance.modemInfo.iccid, line, 20);

    rc = 0;

    _l_retryExit:
    mModuleInstance.aux.initRetryCount++;
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return rc;
}


static int8_t readSignal(void) {
    static const char AT_CSQ[] = "AT+CSQ";

    int8_t dbm = -113;
    Rilat_AtResponse_t *response = NULL;

    if (rilat_writeSingleline(&mModuleInstance.rilat.instance, AT_CSQ, "+CSQ:", &response, 1000) != 0 ||
        response == NULL || response->success == 0) {
        goto _l_retryExit;
    }

    if (response->intermediates == NULL) {
        goto _l_retryExit;
    }
    char *line = response->intermediates->line;

    if (klAttoken_start(&line) != 0) {
        goto _l_retryExit;
    }

    int32_t rssi = 99;
    if (klAttoken_getNextInt(&line, &rssi) == 0) {
        dbm = -113 + (rssi * 2);
    } else {
        goto _l_retryExit;
    }

#if 1
    mModuleInstance.modemInfo.rssi = rssi;
#else
    mNetSignal = dbm;
#endif

    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return dbm;

    _l_retryExit:
    mModuleInstance.aux.initRetryCount++;
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return dbm;
}


static int32_t checkNTP(void) {
    char *line = NULL;
    char *dtStr = NULL;
    Rilat_AtResponse_t *response = NULL;
    klDateTime_SampleTm_t stm = {0};

    if (rilat_writeSingleline(&mModuleInstance.rilat.instance, "AT+CCLK?", "+CCLK:", &response, 1000) != 0) {
        // LOG_W("0");
        goto _l_retryExit;
    }

    if (response == NULL || response->success == 0 || response->intermediates == NULL) {
        // LOG_W("1");
        goto _l_retryExit;
    }

    line = response->intermediates->line;

    if (klAttoken_start(&line) != 0) {
        // LOG_W("2");
        goto _l_retryExit;
    }

    if (klAttoken_getNextString(&line, &dtStr, NULL) != 0) {
        // LOG_W("3");
        goto _l_retryExit;
    }

    {
        // static const char * const NTP_FORMAT = "%d/%d/%d,%d:%d:%d%*[+-]%d";
        int32_t i32 = 0;

        i32 = atoi(dtStr);
        while (*dtStr != '\0' && *dtStr != '/') {
            (dtStr)++;
        }
        if (*dtStr == '/') {
            dtStr++;
        }
        stm.year = i32 + 2000;

        i32 = atoi(dtStr);
        while (*dtStr != '\0' && *dtStr != '/') {
            (dtStr)++;
        }
        if (*dtStr == '/') {
            dtStr++;
        }
        stm.mon = i32;

        i32 = atoi(dtStr);
        while (*dtStr != '\0' && *dtStr != ',') {
            (dtStr)++;
        }
        if (*dtStr == ',') {
            dtStr++;
        }
        stm.day = i32;

        i32 = atoi(dtStr);
        while (*dtStr != '\0' && *dtStr != ':') {
            (dtStr)++;
        }
        if (*dtStr == ':') {
            dtStr++;
        }
        stm.hour = i32;

        i32 = atoi(dtStr);
        while (*dtStr != '\0' && *dtStr != ':') {
            (dtStr)++;
        }
        if (*dtStr == ':') {
            dtStr++;
        }
        stm.min = i32;

        stm.sec = (*dtStr - '0') * 10 + (*(dtStr + 1) - '0');
        dtStr += 2;

        if (*dtStr == '+') {
            dtStr++;
            i32 = atoi(dtStr);
        } else {
            dtStr++;
            i32 = 0 - atoi(dtStr);
        }

        uint32_t ts = klDateTime_mktime(&stm);

        if (strstr(line, "GMT")) {
            ts += i32 * 60 * 60;
        } else {
            ts += (i32 / 4) * 60 * 60;
        }

        // To UTC+0
        mModuleInstance.aux.timeStamp = ts - 28800;
    }

    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return 0;

    _l_retryExit:
    mModuleInstance.aux.initRetryCount++;
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return 1;
}


static void reboot(void) {
    static const char AT_RST[] = "AT+CFUN=0,1";
    rilat_writeLine(&mModuleInstance.rilat.instance, AT_RST, NULL, 500);
    _cModule_wait(&mModuleInstance, 10000, true);
}


static int32_t pdpact(void) {
    static const char AT_QIACT[] = "AT+QIACT=1";
    static const char AT_QIACT_A[] = "AT+QIACT?";

    Rilat_AtResponse_t *response = NULL;

    if (rilat_writeMultiline(&mModuleInstance.rilat.instance, AT_QIACT_A, "+QIACT", &response, 150000) != 0) {
        mModuleInstance.aux.initRetryCount++;
        rilat_freeResponse(&mModuleInstance.rilat.instance, response);
        return -1;
    }
    if (response != NULL && response->success == 1 && response->intermediates != NULL) {
        rilat_freeResponse(&mModuleInstance.rilat.instance, response);
        return 0;
    }

    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    if (rilat_writeLine(&mModuleInstance.rilat.instance, AT_QIACT, &response, 150000) != 0 || response == NULL ||
        response->success == 0) {
        mModuleInstance.aux.initRetryCount++;
        rilat_freeResponse(&mModuleInstance.rilat.instance, response);
        return -1;
    }
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return 0;
}


static int32_t mqttOpen(char *addr, uint16_t port) {
    static const char AT_QMTCFG_VERSION[] = "AT+QMTCFG=\"version\",0,4";
    static const char AT_QMTCFG_RECVMODE[] = "AT+QMTCFG=\"recv/mode\",0,0,1";
    static const char AT_QMTCFG_DATAFORMAT[] = "AT+QMTCFG=\"dataformat\",0,0,0";
    static const char AT_QMTCFG_TIMEOUT[] = "AT+QMTCFG=\"timeout\",0,30,3,0";
    static const char AT_QMTCFG_OPEN[] = "AT+QMTOPEN=0,\"";

    int32_t rc = -1;

    rilat_writeLine(&mModuleInstance.rilat.instance, AT_QMTCFG_VERSION, NULL, 500);
    rilat_writeLine(&mModuleInstance.rilat.instance, AT_QMTCFG_RECVMODE, NULL, 500);
    rilat_writeLine(&mModuleInstance.rilat.instance, AT_QMTCFG_TIMEOUT, NULL, 500);
    rilat_writeLine(&mModuleInstance.rilat.instance, AT_QMTCFG_DATAFORMAT, NULL, 500);

    Rilat_AtResponse_t *response = NULL;
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) AT_QMTCFG_OPEN, strlen(AT_QMTCFG_OPEN), 0);
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) addr, strlen(addr), 0);
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) "\",", 2, 0);
    char portBuffer[16] = {0};
    klPtf_sprintf(portBuffer, "%d", port);
    if (rilat_writeSinglelineOnlyPrefixMatched(&mModuleInstance.rilat.instance, portBuffer, "+QMTOPEN:", &response,
                                               120000) != 0 || response == NULL || response->success == 0) {
        rc = -1;
        goto _l_retryExit;
    }

    char *line = response->intermediates->line;
    int32_t result = 0;
    if (klAttoken_start(&line) != 0) {
        rc = -1;
        goto _l_retryExit;
    }
    if (klAttoken_getNextInt(&line, &result) != 0) {
        rc = -1;
        goto _l_retryExit;
    }
    if (klAttoken_getNextInt(&line, &result) != 0) {
        rc = -1;
        goto _l_retryExit;
    }
    if (result != 0) {
        rc = result;
        goto _l_retryExit;
    }

    rc = 0;
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return rc;

    _l_retryExit:
    mModuleInstance.aux.initRetryCount++;
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return -1;
}


static void mqttClose(void) {
    static const char AT_QMTCLOSE[] = "AT+QMTCLOSE=0";

    rilat_writeLine(&mModuleInstance.rilat.instance, AT_QMTCLOSE, NULL, 30000);
}


static int32_t mqttConnectCheck(void) {
    static const char AT_QMTCONN_A[] = "AT+QMTCONN?";

    Rilat_AtResponse_t *response = NULL;
    if (rilat_writeSinglelineOnlyPrefixMatched(&mModuleInstance.rilat.instance, AT_QMTCONN_A, "+QMTCONN:", &response,
                                               1000) != 0 || response == NULL || response->success == 0) {
        goto _l_retryExit;
    }

    char *line = response->intermediates->line;
    int32_t result = 0;
    if (klAttoken_start(&line) != 0) {
        goto _l_retryExit;
    }
    if (klAttoken_getNextInt(&line, &result) != 0) { // clientId
        goto _l_retryExit;
    }
    if (result != 0) {
        goto _l_retryExit;
    }
    if (klAttoken_getNextInt(&line, &result) != 0) { // state
        goto _l_retryExit;
    }
    if (result != 3) {
        goto _l_retryExit;
    }

    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return 0;

    _l_retryExit:
    mModuleInstance.aux.initRetryCount++;
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return -1;
}


static int32_t mqttConnect(char *clientId, char *userName, char *pwd) {
    static const char AT_QMTCONN[] = "AT+QMTCONN=0,\"";

    int32_t rc = -1;
    Rilat_AtResponse_t *response = NULL;

    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) AT_QMTCONN, strlen(AT_QMTCONN), false);
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) clientId, strlen(clientId), false);
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) "\",\"", 3, false);
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) userName, strlen(userName), false);
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) "\",\"", 3, false);
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) pwd, strlen(pwd), false);
    if (rilat_writeSinglelineOnlyPrefixMatched(&mModuleInstance.rilat.instance, "\"", "+QMTCONN:", &response,
                                               30000) != 0 || response == NULL || response->success == 0) {
        rc = -1;
        goto _l_retryExit;
    }

    char *line = response->intermediates->line;
    int32_t result = 0;
    if (klAttoken_start(&line) != 0) {
        rc = -1;
        goto _l_retryExit;
    }
    if (klAttoken_getNextInt(&line, &result) != 0) {
        rc = -1;
        goto _l_retryExit;
    }
    if (klAttoken_getNextInt(&line, &result) != 0) {
        rc = -1;
        goto _l_retryExit;
    }
    if (result != 0) {
        if (klAttoken_getNextInt(&line, &result) != 0) {
            rc = -1;
            goto _l_retryExit;
        }
        rc = result;
        goto _l_retryExit;
    }

    rc = 0;
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return rc;

    _l_retryExit:
    mModuleInstance.aux.initRetryCount++;
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return -1;
}


static void mqttDisconnect(void) {
    static const char AT_QMTDISC[] = "AT+QMTDISC=0";
    rilat_writeLine(&mModuleInstance.rilat.instance, AT_QMTDISC, NULL, 30000);
}


static int32_t mqttSubTopic(const char *topicStr, uint8_t qos) {
//    static const char AT_QMTSUB[] = "AT+QMTSUB=0,%d,\""
//                                    "mqtt/%s/%s/%s%s/dn/device_control"
//                                    "\",%d";
    static const char AT_QMTSUB1[] = "AT+QMTSUB=0,";

    int32_t rc = -1;
    Rilat_AtResponse_t *response = NULL;

    char *buffer = calloc(1, 8);
    if (buffer == NULL) {
        return -1;
    }

    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) AT_QMTSUB1, strlen(AT_QMTSUB1), false);
    klPtf_sprintf(buffer, "%d,\"", ++mProtocol.mqttMessageId);
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) buffer, strlen(buffer), false);
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) topicStr, strlen(topicStr), 0);
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) "\",", 2, 0);
    if (rilat_writeSinglelineOnlyPrefixMatched(&mModuleInstance.rilat.instance, "1", "+QMTSUB:", &response, 15000) !=
        0 || response == NULL || response->success == 0) {
        rc = -1;
        goto _l_retryExit;
    }
    free(buffer);
    buffer = NULL;

    char *line = response->intermediates->line;
    int32_t result = 0;
    if (klAttoken_start(&line) != 0) {
        rc = -1;
        goto _l_retryExit;
    }
    if (klAttoken_getNextInt(&line, &result) != 0) {    // clientId
        rc = -1;
        goto _l_retryExit;
    }
    if (klAttoken_getNextInt(&line, &result) != 0) {    // msgId
        rc = -1;
        goto _l_retryExit;
    }
    if (klAttoken_getNextInt(&line, &result) != 0) {    // result
        rc = -1;
        goto _l_retryExit;
    }
    if (result != 0) {
        rc = result;
        goto _l_retryExit;
    }

    rc = 0;
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return rc;

    _l_retryExit:
    free(buffer);
    mModuleInstance.aux.initRetryCount++;
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return -1;
}


static int32_t mqttPubTopic(cModule_ProtocolMqttMessage_t *msg, uint32_t msgId, char *xPayload, uint16_t xPayloadLen) {
    // static const char AT_QMTPUBEX[] = "AT+QMTPUBEX=0,%d,%d,0,\"%s\",%d";
    static const char AT_QMTPUBEX1[] = "AT+QMTPUBEX=0,";

    int32_t rc = -1;
    Rilat_AtResponse_t *response = NULL;

    char *buffer = calloc(1, 16);
    if (buffer == NULL) {
        return -1;
    }

    msg->qos = 1;

    mModuleInstance.state.urcResponseFlags &= ~(CMODULE_URC_FLAG_PUB_FAILED | CMODULE_URC_FLAG_PUB_SUCCESS);

    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) AT_QMTPUBEX1, strlen(AT_QMTPUBEX1), false);
    klPtf_sprintf(buffer, "%d,1,0,\"", ++mProtocol.mqttMessageId);
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) buffer, strlen(buffer), false);
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) msg->topic, strlen(msg->topic), 0);

    if (xPayload == NULL) {
        if (msg->len == 0) {
            msg->len = msg->writer.onDorectGetLength(&mModuleInstance.rilat.instance, msgId);
        }
    } else {
        msg->len = xPayloadLen;
    }
    klPtf_sprintf(buffer, "\",%d", msg->len);

    if (xPayload != NULL) {
        l_normalWrite:
        if (rilat_writeSinglelineWithPDU(&mModuleInstance.rilat.instance,
                                         buffer, NULL,
                                         xPayload, xPayloadLen, msg->qos >= 1 ? RILAT_PDU_S_RESP_ONLY_MATCH : 0,
                                         "+QMTPUBEX:",
                                         msgId,
                                         &response, 30000) != 0 || response == NULL ||
            response->success == 0) {
            rc = -1;
            goto _l_retryExit;
        }
    } else {
        if (msg->payload == NULL) {
            if (msg->writer.onDirectWrite != NULL) {
                if (rilat_writeSinglelineWithPDU(&mModuleInstance.rilat.instance,
                                                 buffer, NULL,
                                                 msg->writer.onDirectWrite, 0,
                                                 (msg->qos >= 1 ? RILAT_PDU_S_RESP_ONLY_MATCH : 0) |
                                                 RILAT_PDU_S_DIRECT_WRITE,
                                                 "+QMTPUBEX:",
                                                 msgId,
                                                 &response, 30000) != 0 || response == NULL ||
                    response->success == 0) {
                    rc = -1;
                    goto _l_retryExit;
                }
            }
        } else {
            xPayload = msg->payload;
            xPayloadLen = msg->len;
            goto l_normalWrite;
        }
    }

    free(buffer);
    buffer = NULL;

    char *line = response->intermediates->line;
    int32_t result = 0;
    if (klAttoken_start(&line) != 0) {
        rc = -1;
        goto _l_retryExit;
    }
    if (klAttoken_getNextInt(&line, &result) != 0) {
        rc = -1;
        goto _l_retryExit;
    }
    if (klAttoken_getNextInt(&line, &result) != 0) {
        rc = -1;
        goto _l_retryExit;
    }
    if (klAttoken_getNextInt(&line, &result) != 0) {
        rc = -1;
        goto _l_retryExit;
    }
    if (result != 0) {
        rc = result;
        goto _l_retryExit;
    }

    msg->writer.isWritenSuccess = true;

    rc = 0;
    free(buffer);
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return rc;

    _l_retryExit:
    msg->writer.isWritenSuccess = false;

    free(buffer);
    mModuleInstance.aux.initRetryCount++;
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return rc;
}


static int32_t callPhones(cModule_ReqId_Atd_t *atd) {
    Rilat_AtResponse_t *rsp = NULL;
    int32_t rc = -1;

    char *sendBuffer = rilat_calloc(&mModuleInstance.rilat.instance, 1, 16 + 16);
    if (sendBuffer == NULL) {
        goto _l_retryExit;
    }

    uint8_t index = 0;
    for (; index < atd->numberCount; index++) {
        klPtf_sprintf(sendBuffer, "ATD%s;", atd->number[index]);

        mModuleInstance.state.urcResponseFlags &= ~(CMODULE_URC_FLAG_CALL_START
                                                    | CMODULE_URC_FLAG_CALL_CONNECT
                                                    | CMODULE_URC_FLAG_CALL_END);

        if (rilat_writeLine(&mModuleInstance.rilat.instance,
                            sendBuffer, &rsp,
                            70000) != 0
            || rsp == NULL
            || !rsp->success) {
            rilat_freeResponse(&mModuleInstance.rilat.instance, rsp);
            rsp = NULL;
            continue;
        }
        rilat_freeResponse(&mModuleInstance.rilat.instance, rsp);
        rsp = NULL;

        LOG_I("CALL_START_WAIT...");
        qSTimer_t wait;
        qSTimer_Set(&wait, 30000);
        do {
            if (qSTimer_Expired(&wait)) {
                break;
            }
            if (mModuleInstance.state.urcResponseFlags & CMODULE_URC_FLAG_CALL_START) {
                break;
            }
            rilat_loop(&mModuleInstance.rilat.instance);
            _cModule_wait(&mModuleInstance, 1, false);
        } while (1);
        if (qSTimer_Expired(&wait)) {
            LOG_I("CALL_START_WAIT...TIMEOUT");
            continue;
        }

        LOG_I("CALL_CONNECT_WAIT...");
        qSTimer_Set(&wait, 120000);
        do {
            if (qSTimer_Expired(&wait)) {
                break;
            }
            if (mModuleInstance.state.urcResponseFlags & CMODULE_URC_FLAG_CALL_END
                || mModuleInstance.state.urcResponseFlags & CMODULE_URC_FLAG_CALL_CONNECT) {
                break;
            }
            rilat_loop(&mModuleInstance.rilat.instance);
            _cModule_wait(&mModuleInstance, 1, false);
        } while (1);

        rilat_writeLine(&mModuleInstance.rilat.instance, "+++", NULL, 1000);
        rilat_writeLine(&mModuleInstance.rilat.instance, "ATH", NULL, 1000);
        rilat_writeLine(&mModuleInstance.rilat.instance, "AT+CHUP", NULL, 1000);

        if (qSTimer_Expired(&wait)) {
            LOG_I("CALL_CONNECT_WAIT...TIMEOUT");
            continue;
        }
        if (mModuleInstance.state.urcResponseFlags & CMODULE_URC_FLAG_CALL_CONNECT) {
            LOG_I("CALL_SUCCESS");
            rc = 0;
            break;
        }
    }

    if (rc != 0) {
        if (index >= atd->numberCount) {
            rc = -3;
        }
    }

    _l_retryExit:
    rilat_free(&mModuleInstance.rilat.instance, sendBuffer);
    rilat_freeResponse(&mModuleInstance.rilat.instance, rsp);
    return rc;
}


static int32_t httpGet(uint32_t msgId, cModule_ReqId_HttpGetReq_t *httpGetReq) {
    ASSERT(httpGetReq->url != NULL);
    ASSERT(httpGetReq->readLength != 0);

    static const char QHTTPURL[] = "AT+QHTTPURL=%d,30";
    static const char QHTTPGETEX[] = "AT+QHTTPGETEX=30,%d,%d";
    static const char QHTTPREAD[] = "+QHTTPREAD:";
    static const char QCONNECT[] = "CONNECT\r\n";

    int32_t rc = -1;
    Rilat_AtResponse_t *rsp = NULL;

    char *buf = calloc(1, strlen(QHTTPGETEX) + 32);
    ASSERT(buf != NULL);

    rilat_writeLine(&mModuleInstance.rilat.instance, "AT+QHTTPCFG=\"requestheader\",0", NULL, 1000);
    rilat_writeLine(&mModuleInstance.rilat.instance, "AT+QHTTPCFG=\"responseheader\",1", NULL, 1000);

    klPtf_sprintf(buf, QHTTPURL, strlen(httpGetReq->url));
    if (rilat_writeLineWithPDU(&mModuleInstance.rilat.instance, buf, "CONNECT",
                               (void *) httpGetReq->url, strlen(httpGetReq->url),
                               0, msgId, &rsp, 30000) != 0 || rsp == NULL || !rsp->success) {
        goto l_exit;
    }
    rilat_freeResponse(&mModuleInstance.rilat.instance, rsp);
    rsp = NULL;

    uint8_t retryCount = 3;
    for (;;) {
        if (httpGetReq->totalSize != 0) {
            if (httpGetReq->readLength > httpGetReq->totalSize) {
                httpGetReq->readLength = httpGetReq->totalSize;
            }
        }
        klPtf_sprintf(buf, QHTTPGETEX, httpGetReq->startAt, httpGetReq->readLength);
        if (rilat_writeSinglelineOnlyPrefixMatched(&mModuleInstance.rilat.instance, buf, "+QHTTPGET:", &rsp, 30000) !=
            0 || rsp == NULL || !rsp->success) {
            goto l_retry;
        }
        char *lineStr = rsp->intermediates->line;
        int32_t result = 0;
        if (klAttoken_start(&lineStr) != 0) {
            goto l_retry;
        }
        if (klAttoken_getNextInt(&lineStr, &result) != 0 || result != 0) {
            goto l_retry;
        }
        if (klAttoken_getNextInt(&lineStr, &result) != 0) {
            goto l_retry;
        }
        if (result == 416) {
            LOG_I("Http-Get: Completed, result=416");
            break;
        }
        if (klAttoken_getNextInt(&lineStr, &result) != 0) {
            goto l_retry;
        }
        rilat_freeResponse(&mModuleInstance.rilat.instance, rsp);
        rsp = NULL;

        rilat_directResetReceiveBuffer(&mModuleInstance.rilat.instance);
        if (rilat_writeLineWithStreamData(&mModuleInstance.rilat.instance,
                                          "AT+QHTTPREAD=30", QCONNECT, QHTTPREAD, &rsp,
                                          30000) != 0 || rsp == NULL) {
            LOG_W("Http-Get: Failed writen");
            goto l_retry;
        }
        if (rsp->success == 0) {
            lineStr = strstr(rsp->finalResponse, "+CME ERROR:");
            if (lineStr != NULL) {
                if (klAttoken_start(&lineStr) != 0) {
                    LOG_W("Http-Get: Unknown error, 1");
                    goto l_retry;
                }
                if (klAttoken_getNextInt(&lineStr, &result) != 0) {
                    LOG_W("Http-Get: Unknown error, 3");
                    goto l_retry;
                }
                if (result == 705) {
                    LOG_I("Http-Get: Completed, dataSize==0");
                    break;
                } else {
                    LOG_W("Http-Get: Error=%d", (int) result);
                    goto l_retry;
                }
            } else {
                LOG_W("Http-Get: Unknown error, 2");
                goto l_retry;
            }
        }

        lineStr = strstr(rsp->finalResponse, QCONNECT);
        if (lineStr == NULL) {
            LOG_W("Http-Get: Can not find QCONNECT");
            goto l_retry;
        }
        lineStr += strlen(QCONNECT);

        cModule_ReqId_HttpGetPackage_t get;

        static const char *ContentRange = "Content-Range:";
        lineStr = strstr(lineStr, ContentRange);
        if (lineStr == NULL) {
            LOG_W("Http-Get: Can not find Content-Range");
            goto l_retry;
        }
        lineStr = strstr(lineStr, "bytes");
        if (lineStr == NULL) {
            LOG_W("Http-Get: Can not find Content-Range: bytes");
            goto l_retry;
        }
        lineStr += strlen("bytes");
        
        klib_str_skipWhiteSpace(&lineStr);

        char *strTotalSize = strstr(lineStr + 1, "/");
        if (strTotalSize == NULL) {
            LOG_W("Http-Get: Can not find total size");
            goto l_retry;
        }
        *strTotalSize = '\0';
        get.totalSize = atoi(strTotalSize + 1);

        char *strEndAt = lineStr;
        strEndAt = strstr(strEndAt, "-");
        if (strEndAt == NULL) {
            LOG_W("Http-Get: Can not find end at");
            goto l_retry;
        }
        *strEndAt = '\0';

        get.startAt = atoi(lineStr);
        uint32_t endAt = atoi(strEndAt + 1);
        get.dataSize = endAt - get.startAt + 1;

        if (httpGetReq->totalSize != 0) {
            if (get.totalSize != httpGetReq->totalSize) {
                LOG_W("Http-Get: fileGetSize!=httpGetReq->totalSize");
                goto l_retry;
            }
        } else {
            httpGetReq->totalSize = get.totalSize;
        }

        LOG_I("Http-Get: %s bytes %d-%d/%d", ContentRange, (int) get.startAt, (int) endAt, (int) get.totalSize);

        lineStr = strstr(strTotalSize + 1, "\r\n\r\n");
        if (lineStr == NULL) {
            LOG_W("Http-Get: Can not find body start");
            goto l_retry;
        }
        lineStr += 4;

        get.data = lineStr;

        cModule_ReqIdAck_t ack = {
                .id = TRANSMIT_PACK_REQ_ID_HTTP_GET,
                .messageId = msgId,
                .var.ptr = &get,
        };
        if (CREQUEST(ON_REQ_ID_ACK, { .ptr = &ack }).i32 != 0) {
            LOG_W("Http-Get: User cancel");
            goto l_exit;
        }

        rilat_directResetReceiveBuffer(&mModuleInstance.rilat.instance);

        if (get.startAt + get.dataSize >= get.totalSize) {
            LOG_I("Http-Get: Completed");
            break;
        }

        httpGetReq->startAt = get.startAt + httpGetReq->readLength;
        if (httpGetReq->startAt + httpGetReq->readLength > httpGetReq->totalSize) {
            httpGetReq->readLength = httpGetReq->totalSize - httpGetReq->startAt;
        }

        LOG_I("Http-Get: NextAt=%d, Read=%d", (int) httpGetReq->startAt, (int) httpGetReq->readLength);
        retryCount = 3;
        continue;
        l_retry:
        if (retryCount != 0) {
            retryCount--;
        } else if (retryCount == 0) {
            goto l_exit;
        }
    }

    rc = 0;

    l_exit:
    free(buf);
    rilat_freeResponse(&mModuleInstance.rilat.instance, rsp);
    rsp = NULL;
    return rc;
}


static int32_t socketTcpState(void) {
    Rilat_AtResponse_t *response = NULL;
    if (rilat_writeSinglelineOnlyPrefixMatched(&mModuleInstance.rilat.instance, "AT+QISTATE=0", "+MIPSTATE:", &response, 1000) != 0 || response == NULL || response->success == 0) {
        goto _l_retryExit;
    }
    char *line = response->intermediates->line;
    char *st = NULL;
    if (klAttoken_start(&line) != 0) {
        goto _l_retryExit;
    }
    klAttoken_skip(&line); // <connect_id>
    klAttoken_skip(&line); // <service_type>
    klAttoken_skip(&line); // <IP_address>
    klAttoken_skip(&line); // <remote_port>
    klAttoken_skip(&line); // <<local_port>
    if (klAttoken_getNextString(&line, &st, NULL) != 0) { // <socket_state>
        goto _l_retryExit;
    }
    if (strstr(st, "Connected")) {
        rilat_freeResponse(&mModuleInstance.rilat.instance, response);
        response = NULL;
        return 0;
    }
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    response = NULL;

    _l_retryExit:
    mModuleInstance.aux.initRetryCount++;
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return -1;
}


static int32_t socketTcpCreate(uint8_t tcp0udp1, char *ip, uint16_t port, uint32_t timeout) {
    static const char MIPOPEN_TCP[] = "AT+QIOPEN=1,0,\"TCP\",\"";
    static const char MIPOPEN_UDP[] = "AT+QIOPEN=1,0,\"UDP\",\"";

    Rilat_AtResponse_t *response = NULL;

    mModuleInstance.state.urcResponseFlags &= ~(CMODULE_URC_FLAG_SUB_SUCCESS | CMODULE_URC_FLAG_SUB_FAILED);

    rilat_writeLine(&mModuleInstance.rilat.instance, "AT+QICFG=\"dataformat\",1,1", NULL, 3000);

    char *buffer = calloc(1, 32);
    ASSERT(buffer != NULL);
    klPtf_sprintf(buffer,
        "%d,"   // <remote_port>
        "0,"    // <local_port>
        "1,"    // <access_mode>
        "11,"    // <tcp_client_maxnum>
        "%d",   // <connect_timeout>
        port, timeout / 1000);

    if (tcp0udp1) {
        rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) MIPOPEN_UDP, strlen(MIPOPEN_UDP), 0);
    } else {
        rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) MIPOPEN_TCP, strlen(MIPOPEN_TCP), 0);
    }
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) ip, strlen(ip), 0);
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) "\",", 2, 0);
    if (rilat_writeSinglelineOnlyPrefixMatched(&mModuleInstance.rilat.instance, buffer, "+QIOPEN:", &response, timeout + 3000) != 0) {
        goto l_retryExit;
    }
    free(buffer);
    buffer = NULL;

    char *line = response->intermediates->line;
    int32_t st = 0;
    if (klAttoken_start(&line) != 0) {
        goto l_retryExit;
    }
    klAttoken_skip(&line); // <connect_id>
    if (klAttoken_getNextInt(&line, &st) != 0) { // <state>
        goto l_retryExit;
    }
    if (st != 0) {
        goto l_retryExit;
    }

    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    response = NULL;

    return 0;

    l_retryExit:
    free(buffer);
    mModuleInstance.aux.initRetryCount++;
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return -1;
}

static int32_t socketTcpClose(void) {
    rilat_writeLine(&mModuleInstance.rilat.instance, "AT+QICLOSE=0", NULL, 3000);
    return 0;
}

static int32_t socketTcpSend(cModule_ProtocolTcpIpMessage_t *msg, uint32_t packMsgId) {
    static const char MIPSEND[] = "AT+QISEND=0,\"";  // HEX-Send

    int32_t rc = -1;

    if (msg->payloadLength == 0) {
        if (msg->writer.onDirectGetLength != NULL) {
            msg->payloadLength = msg->writer.onDirectGetLength(&mModuleInstance.rilat.instance, packMsgId);
        }
    }

    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) MIPSEND, strlen(MIPSEND), 0);

    if (msg->payload == NULL) {
        if (msg->writer.onDirectWrite != NULL) {
            msg->writer.onDirectWrite(&mModuleInstance.rilat.instance, packMsgId);
        }
    } else {
        uint32_t cur = 0;
        uint8_t *hexBuffer = calloc(1, 129);
        ASSERT(hexBuffer != NULL);
        while (cur < msg->payloadLength) {
            uint8_t writeLen = msg->payloadLength - cur;
            if (writeLen >= 64) {
                writeLen = 64;
            }
            uint8_t hexLen = klStr_hex2str((uint8_t *) msg->payload + cur, writeLen, (char *) hexBuffer);

            rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) hexBuffer, hexLen, 0);
            memset(hexBuffer, 0x00, 129);

            cur += writeLen;
        }
        free(hexBuffer);
    }

    Rilat_AtResponse_t *response = NULL;
    if (rilat_writeLine(&mModuleInstance.rilat.instance, "\"", &response, msg->timeout) != 0
            || response == NULL || response->success == 0) {
        rc = -1;
        goto _l_retryExit;
    }
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    response = NULL;

    msg->writer.isWritenSuccess = true;

    return 0;

_l_retryExit:
    mModuleInstance.aux.initRetryCount++;
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return -1;
}


static void powerSet(bool up) {
    void *pin = CREQUEST(ON_MODEM_POWER_PIN_ACCESS, {}).ptr;
    uint16_t delay = 0;

    if (up) {
        if (pin != NULL && (uint32_t) pin != UINT32_MAX) {
            delay = 750;
        }
    } else {
        if (pin != NULL && (uint32_t) pin != UINT32_MAX) {
            delay = 3200;
        } else if (pin == NULL) {
            rilat_writeLine(&mModuleInstance.rilat.instance, "AT+QPOWD=1", NULL, 1000);
            return;
        }
    }

    if (delay != 0) {
        kdgpio_t *pwrPin = pin;
        kdgpio_init((kdgpio_t *) pwrPin);
        kdgpio_powerUp((kdgpio_t *) pwrPin, KDGPIO_MODE_OUTPUT_PP, KDGPIO_PULL_NONE);
        kdgpio_output((kdgpio_t *) pwrPin, 1);
        _cModule_wait(&mModuleInstance, delay, 0);
        kdgpio_output((kdgpio_t *) pwrPin, 0);
        kdgpio_powerDown((kdgpio_t *) pwrPin);
        kdgpio_finalize((kdgpio_t *) pwrPin);
    }
}


static void __onHalSleep(void) {

}

static void __onHalWakeup(void) {

}

static void __onSoftWakeup(void) {

}

static void __onSoftSleep(void) {
    AT();
    AT();
    mqttDisconnect();
    mqttClose();

    powerSet(false);
}

static void onResetStack(void) {
    mqttDisconnect();
    mqttClose();
    onReboot(false);
}

static void onReboot(uint8_t isPowerUpRequest) {
    if (isPowerUpRequest) {
        powerSet(true);
    } else {
        void *pin = CREQUEST(ON_MODEM_RESET_PIN_ACCESS, {}).ptr;
        if (pin != NULL && (uint32_t) pin != UINT32_MAX) {
            kdgpio_t *rstPin = pin;
            kdgpio_init((kdgpio_t *) rstPin);
            kdgpio_powerUp((kdgpio_t *) rstPin, KDGPIO_MODE_OUTPUT_PP, KDGPIO_PULL_NONE);
            kdgpio_output((kdgpio_t *) rstPin, 1);
            _cModule_wait(&mModuleInstance, 100, 0);
            kdgpio_output((kdgpio_t *) rstPin, 0);
            kdgpio_powerDown((kdgpio_t *) rstPin);
            kdgpio_finalize((kdgpio_t *) rstPin);
        } else {
            reboot();
        }
    }
}


static void onLoop(void) {
    if (qSTimer_Expired(&mModuleInstance.aux.pollTimer)) {
        qSTimer_Set(&mModuleInstance.aux.pollTimer, 60 * 1000);

        AT();
        AT();
        if (checkNTP() != 0 || readSignal() <= -113) {
            if (++mModuleInstance.aux.initRetryCount > 2) {
                mModuleInstance.aux.initRetryCount = 0;
                mModuleInstance.state.urcResponseFlags |= CMODULE_URC_FLAG_NEED_RESET;
            }
        } else {
            CREQUEST(ON_NTP_UPDATE, (void *) &mModuleInstance,
                     mModuleInstance.aux.timeStamp);
        }
    }
}

/*@}*/

/**
 * @addtogroup EC800E-URC
 * @note none
 */

/*@{*/

RILAT_COMMAND_MATCH_DEFINE(_QMTSTAT, "+QMTSTAT:", data, len) {
    if (mModuleInstance.aux.flag.passiveRecvMode) {
        return -1;
    }
    qSTimer_Set(&CMPMU.aux.idleTimer, CONFIG_CMODULE_IDLE_TIMEOUT_MS);

    char *line = (char *) data;
    int32_t val = 0;

    if (klAttoken_start(&line) != 0) {
        return -1;
    }

    klAttoken_skip(&line); // clientId

    if (klAttoken_getNextInt(&line, &val) != 0) {
        return -1;
    }

    switch (val) {
        case 1:
        case 2:
        case 3:
        case 4:
        case 6:
        case 7:
            mModuleInstance.state.urcResponseFlags |= (CMODULE_URC_FLAG_NEED_RESET);
            break;
    }

    return 0;
}


RILAT_COMMAND_MATCH_DEFINE(_QMTRECV, "+QMTRECV:", data, len) {
    if (mModuleInstance.aux.flag.passiveRecvMode) {
        return -1;
    }
    qSTimer_Set(&CMPMU.aux.idleTimer, CONFIG_CMODULE_IDLE_TIMEOUT_MS);

    char *line = (char *) data;
    char *payloadStr = NULL;
    int32_t dataLength = 0;
    cModule_TransmitPackageInfo_t *pack = NULL;

    if (klAttoken_start(&line) != 0) {
        return -1;
    }
    klAttoken_skip(&line); // clientId
    klAttoken_skip(&line); // msgId

    char *topic = NULL;
    klAttoken_getNextString(&line, &topic, NULL);

    if (klAttoken_getNextInt(&line, &dataLength) != 0) {
        return -1;
    }

    // Skip " at end
    payloadStr = line + 1;
    uint32_t stLen = strlen(payloadStr) - 1;
    if (stLen != dataLength) {
        LOG_W("QMTRECV, stLen != dataLength");
        if (stLen < CONFIG_CMODULE_BUFFER_POS_LEN) {
            LOG_W("Using strlen");
            dataLength = stLen;
        }
    }

    if (!_cModule_isAllocRepPackAccess(&mModuleInstance)) {
        return -1;
    }

    _cModule_packMutexLock(&mModuleInstance, true);

    pack = calloc(1, sizeof(cModule_TransmitPackageInfo_t));
    if (pack == NULL) {
        goto l_errorExit;
    }
    pack->payloadLength = dataLength;
    pack->payload = calloc(1, pack->payloadLength + 1);
    if (pack->payload == NULL) {
        goto l_errorExit;
    }
    memcpy(pack->payload, payloadStr, dataLength);

    if (topic != NULL) {
        pack->var.ptr = calloc(1, strlen(topic) + 1);
        if (pack->var.ptr == NULL) {
            goto l_errorExit;
        }
        pack->varLength = strlen(topic);
        strncpy(pack->var.ptr, topic, strlen(topic));
        pack->flag.varIsLengthItem = 1;
        pack->flag.packHasVar = 1;
    }

    pack->flag.packIsDynData = 1;
    pack->flag.requestId = TRANSMIT_PACK_REQ_ID_RX;

    klist_init(&pack->list);
    klist_addTail(&mModuleInstance.transmit.packageRepList, &pack->list);
    mModuleInstance.transmit.queneRepCount++;

    _cModule_packMutexLock(&mModuleInstance, false);

    return 0;

    l_errorExit:
    _cModule_packMutexLock(&mModuleInstance, false);
    if (pack != NULL) {
        if (pack->payload != NULL) {
            free(pack->payload);
        }
        free(pack);
    }
    return -1;
}


RILAT_COMMAND_MATCH_DEFINE(__DSCI, "^DSCI:", data, len) {
    if (mModuleInstance.aux.flag.passiveRecvMode) {
        return -1;
    }
    qSTimer_Set(&CMPMU.aux.idleTimer, CONFIG_CMODULE_IDLE_TIMEOUT_MS);

    char *line = (char *) data;
    uint32_t u32 = 0;

    if (klAttoken_start(&line) != 0) {
        return -1;
    }
    klAttoken_skip(&line); // id
    if (klAttoken_getNextUInt(&line, &u32) != 0 || u32 != 0) {
        return -1;
    }
    if (klAttoken_getNextUInt(&line, &u32) != 0) {
        return -1;
    }
    if (u32 == 7) {
        LOG_I("CALL_START");
        mModuleInstance.state.urcResponseFlags |= (CMODULE_URC_FLAG_CALL_START);
    } else if (u32 == 3) {
        LOG_I("CALL_CONNECT");
        mModuleInstance.state.urcResponseFlags |= (CMODULE_URC_FLAG_CALL_CONNECT);
    } else if (u32 == 6) {
        LOG_I("CALL_END");
        mModuleInstance.state.urcResponseFlags |= (CMODULE_URC_FLAG_CALL_END);
    }
    return 0;
}



#if CONFIG_CMODULE_INSTANCE_INIT_MODE == CONFIG_CMODULE_INSTANCE_INIT_MODE_TCP
RILAT_COMMAND_MATCH_DEFINE(_MIPURC, "+QIURC:", data, len) {
    if (mModuleInstance.aux.flag.passiveRecvMode) {
        return -1;
    }

    char *line = (char *) data;
    char *statusStr = NULL;
    int32_t dataLength = 0;
    cModule_TransmitPackageInfo_t *pack = NULL;

    if (klAttoken_start(&line) != 0) {
        return -1;
    }

    // StatusString
    if (klAttoken_getNextString(&line, &statusStr, &dataLength) != 0) {
        return -1;
    }

    if (strstr(statusStr, "recv")) {
        uint32_t payloadLength = 0;
        char *payload = NULL;

        klAttoken_skip(&line);   // <connect_id>
        klAttoken_skip(&line);   // <recv_length>

        if (!_cModule_isAllocRepPackAccess(&mModuleInstance)) {
            return -1;
        }
        _cModule_packMutexLock(&mModuleInstance, true);

        while (*line != '\0' && isspace(*line)) {
            (line)++;
        }
        payloadLength = strlen(line);
        payload = calloc(1, payloadLength + 1);
        if (payload == NULL) {
            _cModule_packMutexLock(&mModuleInstance, false);
            return -1;
        }
        payloadLength = klStr_hexStr2hex(line, payloadLength, payload);

        cModule_TransmitPackageInfo_t *pack = calloc(1, sizeof(cModule_TransmitPackageInfo_t));
        if (pack == NULL) {
            goto l_errorExit;
        }
        pack->payloadLength = payloadLength;
        pack->payload = payload;
        pack->flag.packIsDynData = 1;
        pack->flag.requestId = TRANSMIT_PACK_REQ_ID_RX;
        klist_init(&pack->list);
        klist_addTail(&mModuleInstance.transmit.packageRepList, &pack->list);
        mModuleInstance.transmit.queneRepCount++;
        _cModule_packMutexLock(&mModuleInstance, false);
    } else if (strstr(statusStr, "closed") || strstr(statusStr, "incoming full")) {
        mModuleInstance.state.urcResponseFlags |= (CMODULE_URC_FLAG_NEED_RESET | CMODULE_URC_FLAG_CONN_FAILED);
    }

    l_errorExit:
    _cModule_packMutexLock(&mModuleInstance, false);
    if (pack != NULL) {
        if (pack->payload != NULL) {
            free(pack->payload);
        }
        free(pack);
    }
    return -1;
}
#endif

/*@}*/

/**
 * @addtogroup EC800E-Init
 * @note none
 */

/*@{*/

static void setSuccessStep(uint16_t nextLoopSec) {
    mModuleInstance.aux.initRetryCount = 0;
    mModuleInstance.state.processStep = CMODULE_PROCESS_STEP_INIT_COMPLETED;
    qSTimer_Set(&mModuleInstance.aux.pollTimer, 300);
    mModuleInstance.aux.nextPollSec = nextLoopSec;
}


static void setNextStep(void) {
    mModuleInstance.aux.initRetryCount = 0;
    mModuleInstance.state.processStep++;
    qSTimer_Set(&mModuleInstance.aux.pollTimer, 300);
}


static void setNextStepWithSkip(uint8_t skip) {
    mModuleInstance.aux.initRetryCount = 0;
    mModuleInstance.state.processStep += skip;
    qSTimer_Set(&mModuleInstance.aux.pollTimer, 300);
}


static void setErrorStep(uint16_t errorCount, cModule_State_t st, uint32_t nextTimeout) {
    if (mModuleInstance.aux.initRetryCount > errorCount || errorCount == 0) {
        mModuleInstance.state.currentState = st;
        CREQUEST(ON_STATE_CHANGED, (void *) &mModuleInstance, (uint32_t) mModuleInstance.state.currentState);
        mModuleInstance.aux.initRetryCount = CMODULE_ERR_COUNT;
        mModuleInstance.state.processStep = 0;
    }
    qSTimer_Set(&mModuleInstance.aux.pollTimer, nextTimeout);
}

static void onInitStep(uint8_t step) {
    if (!qSTimer_Expired(&mModuleInstance.aux.pollTimer)) {
        return;
    }

    if (step == 1) {
        ATE0();
        setNextStep();
    } else if (step == 2) {
        if (checkCPIN() == 0) {
            setNextStep();
        } else {
            setErrorStep(10, CMODULE_STATE_NO_COM_ID, 1000);
        }
    } else if (step == 3) {
        if (queryIMEI() == 0) {
            _cModule_wait(&mModuleInstance, 300, true);
            queryIMSI();
            _cModule_wait(&mModuleInstance, 300, true);
            readIccid();
            setNextStep();
        } else {
            setErrorStep(10, CMODULE_STATE_NO_COM_ID, 1000);
        }
    } else if (step == 4) {
        if (checkCGREG() == 0) {
            _cModule_wait(&mModuleInstance, 1000, true);
            readSignal();
            _cModule_wait(&mModuleInstance, 300, true);
            checkNTP();
            CREQUEST(ON_NTP_UPDATE, (void *) &mModuleInstance, mModuleInstance.aux.timeStamp);
            _cModule_wait(&mModuleInstance, 300, true);
            pdpact();
            setNextStep();
        } else {
            setErrorStep(120, CMODULE_STATE_FAILED_REG, 300);
        }
    }
#if CONFIG_CMODULE_INSTANCE_INIT_MODE == CONFIG_CMODULE_INSTANCE_INIT_MODE_TCP
    else if (step == 5) {
        if (socketTcpState() == 0) {
            setSuccessStep(120);
        } else {
            setNextStep();
        }
    } else if (step == 6) {
        int32_t rc;

        uint8_t socketMode = 0;
        char *ip = NULL;
        uint16_t port = 0;
        _cModule_onProtocolTcpConnectAlloc(&mModuleInstance, &ip);
        _cModule_onProtocolTcpConnect(&mModuleInstance, &socketMode, &ip, &port);
        rc = socketTcpCreate(socketMode, ip, port, 30000);
        _cModule_onProtocolTcpConnectFree(&mModuleInstance, &ip);

        if (rc == 0) {
            setSuccessStep(120);
        } else {
            setErrorStep(0, CMODULE_STATE_FAILED_CON, 1000);
        }
    }
#endif
#if CONFIG_CMODULE_INSTANCE_INIT_MODE == CONFIG_CMODULE_INSTANCE_INIT_MODE_MQTT
    else if (step == 5) {
        if (mqttConnectCheck() == 0) {
            setNextStepWithSkip(2);
        } else {
            setNextStep();
        }
    } else if (step == 6) {
        int32_t rc;

        char *ip = NULL;
        uint16_t port = 0;
        char *clientId = NULL;
        char *userName = NULL, *pwd = NULL;
        _cModule_onProtocolMqttConnectAlloc(&mModuleInstance, &ip, &clientId, &userName, &pwd);
        _cModule_onProtocolMqttConnect(&mModuleInstance, &ip, &port, &clientId, &userName, &pwd);
        rc = mqttOpen(ip, port);
        if (rc != 0) {
            _cModule_onProtocolMqttConnectFree(&mModuleInstance, &ip, &clientId, &userName, &pwd);
            goto l_mqttConnect_err;
        }
        rc = mqttConnect(clientId, userName, pwd);
        _cModule_onProtocolMqttConnectFree(&mModuleInstance, &ip, &clientId, &userName, &pwd);

        if (rc == 0) {
            setNextStep();
        } else {
            l_mqttConnect_err:
            setErrorStep(1, CMODULE_STATE_FAILED_CON, 1000);
        }
    } else if (step == 7) {
        int32_t rc = -1;

        uint8_t count = _cModule_onProtocolMqttSubCount(&mModuleInstance);
        for (uint8_t i = 0; i < count; i++) {
            char *topic = NULL;
            uint8_t qos = 1;
            _cModule_onProtocolMqttSubAlloc(&mModuleInstance, &topic, i);
            _cModule_onProtocolMqttSub(&mModuleInstance, &topic, &qos, i);
            rc = mqttSubTopic(topic, qos);
            _cModule_onProtocolMqttSubFree(&mModuleInstance, &topic, i);
            if (rc != 0) {
                break;
            }
        }

        if (rc == 0) {
            setSuccessStep(120);
        } else {
            setErrorStep(0, CMODULE_STATE_FAILED_CON, 1000);
        }
    }
#endif
}

/*@}*/

/**
 * @addtogroup Protocol-JW
 * @note none
 */

/*@{*/

int32_t onPtPackIdReceived(cModule_TransmitPackageInfo_t *info) {
    if (_cModule_onProtocolReceived(&mModuleInstance, info) != 0) {
        CREQUEST(ON_RECV_DATA, (void *) &mModuleInstance, info->payload, (uint32_t) info->payloadLength);
    }
    return 0;
}


static int32_t onPtPackIdTransmit(cModule_TransmitPackageInfo_t *info) {
    int32_t rc = -1;

    if (info->flag.packIsDynMqttMsg) {
        cModule_ProtocolMqttMessage_t *msg = info->payload;

        if (msg->isPayloadJson) {
#if CONFIG_CMODULE_CJSON_SUPPORT == 1
            if (_cModule_onProtocolTransmited(&mModuleInstance, info) != 0) {
                return rc;
            }

            cJSON *cjValue = (void *) msg->payload;
            char *payload = cJSON_PrintUnformatted(cjValue);
            if (payload != NULL) {
                rc = mqttPubTopic(msg, info->aux.gen.msgId, payload, strlen(payload));
            }
            cJSON_free(payload);
            if (rc != 0) {
                LOG_W("onPtPackIdTransmit, Failed!");
            }
            return rc;
#else
            ASSERT(0);
#endif
        } else if (msg->isPayloadString) {
            rc = mqttPubTopic(msg, info->aux.gen.msgId, NULL, 0);
        } else {
            LOG_E("cModule_ProtocolMqttMessage_t, Unknown msgType");
            return rc;
        }
    }

    return rc;
}


static int32_t onPtPackIdCustom(cModule_TransmitPackageInfo_t *info) {
    if (info->flag.requestId == TRANSMIT_PACK_REQ_ID_ATD) {
        cModule_ReqIdAck_t ack = {
                .id = TRANSMIT_PACK_REQ_ID_ATD,
                .messageId = info->aux.gen.msgId,
                .rc = callPhones(info->payload),
        };
        CREQUEST(ON_REQ_ID_ACK, { .ptr = &ack });
    } else if (info->flag.requestId == TRANSMIT_PACK_REQ_ID_HTTP_GET) {
        cModule_ReqIdAck_t ack = {
                .id = TRANSMIT_PACK_REQ_ID_HTTP_GET,
                .messageId = info->aux.gen.msgId,
                .rc = httpGet(info->aux.gen.msgId, info->payload),
                .var.ptr = NULL,
        };
        CREQUEST(ON_REQ_ID_ACK, { .ptr = &ack });
    }
    return 0;
}


static int32_t onPtPackPayloadFree(cModule_TransmitPackageInfo_t *info, bool isForce) {
    if (info->flag.requestId == TRANSMIT_PACK_REQ_ID_RX) {
        return 0;
    }
    
    if (info->flag.packIsDynMqttMsg) {
        cModule_ProtocolMqttMessage_t *msg = info->payload;
        if (msg->payload == NULL) {
            if (msg->writer.onDirectWriteResponse != NULL) {
                msg->writer.onDirectWriteResponse(&mModuleInstance.rilat.instance, info->aux.gen.msgId, msg->writer.isWritenSuccess, isForce);
            }
        } else {
            if (msg->isPayloadString) {
                free(msg->payload);
            } else if (msg->isPayloadJson) {
#if CONFIG_CMODULE_CJSON_SUPPORT == 1
                cJSON_Delete((void *) msg->payload);
#else
                ASSERT(0);
#endif
            } else {
                ASSERT(0);
            }
        }
        if (!msg->topicConstant) {
            free(msg->topic);
        }
    } else if (info->flag.packIsDynHttpMsg) {
        cModule_ProtocolHttpMessage_t *msg = info->payload;
        if (msg->onDirectWriteResponse != NULL) {
            msg->onDirectWriteResponse(&mModuleInstance.rilat.instance, info->aux.gen.msgId, msg->isWritenSuccess, isForce);
        }
        if (msg->host != NULL) {
            if (!msg->hostContant) {
                free(msg->host);
            }
        }
        if (msg->path != NULL) {
            if (!msg->pathContant) {
                free(msg->path);
            }
        }
        if (msg->payload != NULL) {
            if (msg->payloadLength != 0) {
                free(msg->payload);
            }
        }
    }
    return 0;
}

/*@}*/

/**
 * @addtogroup rilat callback
 * @note none
 */

/*@{*/

static int32_t rilat_onCallback(Rilat_Instance_t *instance, Rilat_CallbackEvent_t event, Rilat_CallbackVar_t var1,
                                Rilat_CallbackVar_t var2, Rilat_CallbackVar_t var3) {
    if (event == RILAT_CALL_EVENT_ON_INIT) {
        CREQUEST(ON_SERIAL_INIT, {&mModuleInstance});
    } else if (event == RILAT_CALL_EVENT_ON_FINALIZE) {
        CREQUEST(ON_SERIAL_FINALIZE, {&mModuleInstance});
    } else if (event == RILAT_CALL_EVENT_ON_READ) {
        *((uint32_t *) var3.ptr) = CREQUEST(ON_SERIAL_RECV, (void *) &mModuleInstance, (void *) var1.ptr,
                                            (uint32_t) var2.u32).u32;
    } else if (event == RILAT_CALL_EVENT_ON_WRITE) {
        return CREQUEST(ON_SERIAL_TRANSMIT, (void *) &mModuleInstance, (void *) var1.ptr, (uint32_t) var2.u32).i32;
    } else if (event == RILAT_CALL_EVENT_ON_BLOCK_WAIT) {
        return CREQUEST(ON_BLOCK_POLL, (void *) &mModuleInstance,
                        (uint32_t) (mModuleInstance.state.urcResponseFlags & CMODULE_URC_FLAG_STACK_NO_BLOCK_POLL) ? 1
                                                                                                                   : 0).i32;
    } else if (event == RILAT_CALL_EVENT_ON_POLL_HOCK) {
        CREQUEST(ON_BLOCK_POLL, (void *) &mModuleInstance,
                 (uint32_t) (mModuleInstance.state.urcResponseFlags & CMODULE_URC_FLAG_STACK_NO_BLOCK_POLL) ? 1 : 0);
    }
    return 0;
}

/*@}*/

/**
 * @addtogroup BaseFunc
 * @note none
 */

/*@{*/

static void onInstanceInit(void) {
    CMPMU.flag._ = 0;
    CMPMU.flag.isReady = 1;
    CMPMU.sleep.currentSleepLevel = COMPONENT_SLEEP_LV_RUN;
    qSTimer_Set(&CMPMU.aux.idleTimer, CONFIG_CMODULE_IDLE_TIMEOUT_MS);

    qSTimer_Set(&mModuleInstance.aux.pollTimer, 200);
    mModuleInstance.state.processStep = 0;
    mModuleInstance.aux.initRetryCount = 0;
    mModuleInstance.aux.flag.hasInit = 0;
    mModuleInstance.aux.flag.pmuSupport = 0;
    mModuleInstance.aux.flag.passiveRecvMode = CONFIG_CMODULE_INSTANCE_PASSIVE_RECV_MODE;
    mModuleInstance.aux.failedCount = 0;
    if (mModuleInstance.transmit.queneReqCount == 0) {
        klist_init(&mModuleInstance.transmit.packageReqList);
    }
    if (mModuleInstance.transmit.queneRepCount == 0) {
        klist_init(&mModuleInstance.transmit.packageRepList);
    }

    mModuleInstance.rilat.instance.callback = rilat_onCallback;
    mModuleInstance.rilat.instance.name = TAG;
    mModuleInstance.rilat.instance.transmitBuffer.buffer = NULL;
    mModuleInstance.rilat.instance.transmitBuffer.size = 0;
    mModuleInstance.rilat.instance.recvBuffer.buffer = (char *) mModuleInstance.rilat.recvBuffer;
    mModuleInstance.rilat.instance.recvBuffer.size = CONFIG_CMODULE_RILAT_MEM_RECV_BUFFER_SIZE;
    rilat_init(&mModuleInstance.rilat.instance);
}


static void onInstancePmu(uint8_t wakeup1sleep0) {
    if (wakeup1sleep0) {
        CREQUEST(ON_SERIAL_INIT, {&mModuleInstance});
        mModuleInstance.aux.initRetryCount = 0;
        mModuleInstance.state.processStep = 0;
        mModuleInstance.state.urcResponseFlags = 0;
        mModuleInstance.aux.flag.hasWakeupSuccessHappend = 1;
        qSTimer_Set(&mModuleInstance.aux.pollTimer, 200);

        __onHalWakeup();
        __onSoftWakeup();
    } else {
        __onSoftSleep();
        __onHalSleep();
        mModuleInstance.state.currentState = CMODULE_STATE_DISCON;
        CREQUEST(ON_STATE_CHANGED, (void *) &mModuleInstance,
                 (uint32_t) mModuleInstance.state.currentState);
        mModuleInstance.aux.flag.hasWakeupSuccessHappend = 0;
        CREQUEST(ON_SERIAL_FINALIZE, {&mModuleInstance});
    }
}

/*@}*/

/**
 * @addtogroup Instance
 * @note none
 */

/*@{*/

cModule_Instance_t *cModule_getInstance_ec200u(void) {
    return &mModuleInstance;
}

/*@}*/

/**
 * @addtogroup ProtocolFunc
 * @note none
 */

/*@{*/

/*@}*/

#endif

/**
 * @addtogroup DebugSupport
 * @note none
 */

/*@{*/

#undef TAG

/*@}*/