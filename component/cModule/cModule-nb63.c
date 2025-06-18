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
 
#include "global.h"
 
/**
 * @addtogroup Debug support
 * @note none
 */
 
/*@{*/

#undef TAG
#define TAG     "NB63"

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
    for (uint8_t i = 0; i < 5; i++) {
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
        if (mModuleInstance.aux.flag.pmuSupport) {
            static const char AT_CSCON[] = "AT+CSCON=1";
            static const char AT_ECPCFG[] = "AT+ECPCFG=\"slpWaitTime\",60000";
            static const char AT_ECPSMR[] = "AT+ECPSMR=1";
            static const char AT_CPSMS[] = "AT+CPSMS=1";
            static const char AT_PMU[] = "AT+ECPMUCFG=1,4";
            static const char AT_CEDRXS[] = "AT+CEDRXS=0";
            
            rilat_writeLine(&mModuleInstance.rilat.instance, AT_CEDRXS, NULL, 1000);
            rilat_writeLine(&mModuleInstance.rilat.instance, AT_ECPSMR, NULL, 1000);
            rilat_writeLine(&mModuleInstance.rilat.instance, AT_CPSMS, NULL, 1000);
            rilat_writeLine(&mModuleInstance.rilat.instance, AT_CSCON, NULL, 1000);
            rilat_writeLine(&mModuleInstance.rilat.instance, AT_PMU, NULL, 1000);
            rilat_writeLine(&mModuleInstance.rilat.instance, AT_ECPCFG, NULL, 1000);
        } else {
            static const char AT_CSCON[] = "AT+CSCON=0";
            static const char AT_ECPSMR[] = "AT+ECPSMR=0";
            static const char AT_CPSMS[] = "AT+CPSMS=0";
            static const char AT_PMU[] = "AT+ECPMUCFG=0,4";
            static const char AT_CEDRXS[] = "AT+CEDRXS=0";
            
            rilat_writeLine(&mModuleInstance.rilat.instance, AT_CEDRXS, NULL, 1000);
            rilat_writeLine(&mModuleInstance.rilat.instance, AT_ECPSMR, NULL, 1000);
            rilat_writeLine(&mModuleInstance.rilat.instance, AT_CPSMS, NULL, 1000);
            rilat_writeLine(&mModuleInstance.rilat.instance, AT_CSCON, NULL, 1000);
            rilat_writeLine(&mModuleInstance.rilat.instance, AT_PMU, NULL, 1000);
        }
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
    
    return 0;
}


static int32_t checkCPIN(void) {
    static const char AT_CPIN_A[] = "AT+CPIN?";
    
    Rilat_AtResponse_t *response = NULL;

    if (rilat_writeSingleline(&mModuleInstance.rilat.instance, AT_CPIN_A, "+CPIN:", &response, 2000) != 0 || response == NULL || response->success == 0) {
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

    if (rilat_writeSingleline(&mModuleInstance.rilat.instance, AT_CGREG_A, "+CEREG:", &response, 1000) != 0 || response == NULL || response->success == 0) {
        goto _l_retryExit;
    }
    
    if (response->intermediates == NULL) {
        goto _l_retryExit;
    }
    line = response->intermediates->line;

    if (klAttoken_start(&line) != 0) {
        goto _l_retryExit;
    }
    
    if (klAttoken_getNextInt(&line, &numberResult) != 0) {
        goto _l_retryExit;
    }
    
    if (numberResult != 0) {
        goto _l_retryExit;
    }
    
    if (klAttoken_getNextInt(&line, &numberResult) != 0) {
        goto _l_retryExit;
    }
    
    if (!(numberResult == 1 || numberResult == 5)) {
        goto _l_retryExit;
    }

    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return 0;
    
_l_retryExit:
    mModuleInstance.aux.initRetryCount++;
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return 1;
}


static int32_t queryIMEI(void) {
    static const char AT_CGSN[] = "AT+CGSN";
    
    char *line = NULL;
    int32_t numberResult = -1;
    Rilat_AtResponse_t *response = NULL;

    if (rilat_writeMultiline(&mModuleInstance.rilat.instance, AT_CGSN, "", &response, 1000) != 0 || response == NULL || response->success == 0) {
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
    
    char *line = NULL;
    int32_t numberResult = -1;
    Rilat_AtResponse_t *response = NULL;

    if (rilat_writeMultiline(&mModuleInstance.rilat.instance, AT_CIMI, "", &response, 1000) != 0 || response == NULL || response->success == 0) {
        goto _l_retryExit;
    }
    
    if (response->intermediates == NULL) {
        goto _l_retryExit;
    }

    strcpy(mModuleInstance.modemInfo.imsi, response->intermediates->line);

    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return 0;
    
_l_retryExit:
    mModuleInstance.aux.initRetryCount++;
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return 1;
}


static int8_t readSignal(void) {
    static const char AT_CSQ[] = "AT+CSQ";
    
    int8_t dbm = -113;
    Rilat_AtResponse_t *response = NULL;

    if (rilat_writeSingleline(&mModuleInstance.rilat.instance, AT_CSQ, "+CSQ:", &response, 1000) != 0 || response == NULL || response->success == 0) {
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
        goto _l_retryExit;
    }
    
    if (response == NULL || response->success == 0 || response->intermediates == NULL) {
        goto _l_retryExit;
    }
    
    line = response->intermediates->line;

    if (klAttoken_start(&line) != 0) {
        goto _l_retryExit;
    }
    
    if (klAttoken_getNextString(&line, &dtStr, NULL) != 0) {
        goto _l_retryExit;
    }
    
    {
        // static const char * const NTP_FORMAT = "%d/%d/%d,%d:%d:%d%*[+-]%d";
        int32_t i32 = 0;
        
        if (klAttoken_getNextIntSep(&dtStr, '/', &i32) != 0) {
            goto _l_retryExit;
        }
        stm.year = i32;
        
        if (klAttoken_getNextIntSep(&dtStr, '/', &i32) != 0) {
            goto _l_retryExit;
        }
        stm.mon = i32;
        
        if (klAttoken_getNextIntSep(&dtStr, ',', &i32) != 0) {
            goto _l_retryExit;
        }
        stm.day = i32;
        
        if (klAttoken_getNextIntSep(&dtStr, ':', &i32) != 0) {
            goto _l_retryExit;
        }
        stm.hour = i32;
        
        if (klAttoken_getNextIntSep(&dtStr, ':', &i32) != 0) {
            goto _l_retryExit;
        }
        stm.min = i32;

        stm.sec = (*dtStr - '0') * 10 + (*(dtStr + 1) - '0');
        dtStr += 2;
        
        if (klAttoken_getNextInt(&dtStr, &i32) != 0) {
            goto _l_retryExit;
        }
        
        uint32_t ts = klDateTime_mktime(&stm);
        
        if (strstr(line, "GMT")) {
            ts += i32 * 60 * 60;
        } else {
            ts += (i32 / 4) * 60 * 60;
        }
        
        // To UTC+0
        mModuleInstance.aux.timeStamp = ts; // - 28800;
    }

    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return 0;

_l_retryExit:
    mModuleInstance.aux.initRetryCount++;
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return 1;
}


static inline int32_t reboot(void) {
    return rilat_writeLine(&mModuleInstance.rilat.instance, "AT+ECRST", NULL, 500);
}


static int32_t lwm2mInit(uint32_t commandTimeout) {
    static const char AT_CTM2MSETPM[] = "AT+CTM2MSETPM=221.229.214.202,5683,38400";
    static const char AT_CTM2MREG[] = "AT+CTM2MREG";
    
    char *line;
    Rilat_AtResponse_t *response = NULL;
    uint32_t status = 0;
    
    mModuleInstance.state.urcResponseFlags &= ~(
        CMODULE_URC_FLAG_CONN_SUCCESS | CMODULE_URC_FLAG_CONN_FAILED | CMODULE_URC_FLAG_CONN_RETRY | CMODULE_URC_FLAG_STACK_READY | CMODULE_URC_FLAG_NEED_RESET
    );
    
    if (rilat_writeLine(&mModuleInstance.rilat.instance, AT_CTM2MSETPM, &response, commandTimeout) != 0 || response == NULL || response->success == 0) {
        rilat_freeResponse(&mModuleInstance.rilat.instance, response);
        return -1;
    }
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    response = NULL;
    
    if (rilat_writeLine(&mModuleInstance.rilat.instance, AT_CTM2MREG, NULL, commandTimeout) != 0) {
        return -1;
    }

//    qSTimer_t wait;
//    qSTimer_Set(&wait, commandTimeout);
//    while (!qSTimer_Expired(&wait)) {
//        rilat_loop(&mModuleInstance.rilat.instance);
//        
//        if (mModuleInstance.state.urcResponseFlags & CMODULE_URC_FLAG_CONN_SUCCESS 
//                && mModuleInstance.state.urcResponseFlags & CMODULE_URC_FLAG_STACK_READY) {
//            break;
//        } else if (mModuleInstance.state.urcResponseFlags & CMODULE_URC_FLAG_CONN_FAILED) {
//            return -1;
//        }
//    }
//    if (qSTimer_Expired(&wait)) {
//        return -1;
//    }

    return 0;
}


static int32_t lwm2mDelete(uint32_t commandTimeout) {
    mModuleInstance.aux.initRetryCount = 4;
    
    return 0;
}


static int32_t lwm2mPost(cModule_ProtocolGenMessage_t *msg, char *xPayload, uint16_t xPayloadLen, uint8_t usingRAI, uint32_t packMsgId) {
    static const char AT_CTM2MSEND[] = "AT+CTM2MSEND=";
    
    ASSERT(msg != NULL);
    int32_t rc = -1;

    mModuleInstance.state.urcResponseFlags &= ~(CMODULE_URC_FLAG_PUB_SUCCESS | CMODULE_URC_FLAG_PUB_FAILED);
    
    if (xPayload == NULL) {
        if (msg->payloadLength == 0) {
            msg->payloadLength = msg->writer.onDorectGetLength(&mModuleInstance.rilat.instance, packMsgId);
        }
    } else {
        msg->payloadLength = xPayloadLen;
    }
    
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) AT_CTM2MSEND, strlen(AT_CTM2MSEND), 0);
    if (xPayload != NULL) {
        uint32_t cur = 0;
        uint8_t *hexBuffer = calloc(1, 129);
        ASSERT(hexBuffer != NULL);
        while (cur < msg->payloadLength) {
            uint8_t writeLen = msg->payloadLength - cur;
            if (writeLen >= 64) {
                writeLen = 64;
            }
            uint8_t hexLen = klStr_hex2str((uint8_t *) xPayload + cur, writeLen, (char *) hexBuffer);
            
            rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) hexBuffer, hexLen, 0);
            
            cur += writeLen;
        }
        free(hexBuffer);
    } else {
        if (msg->payload == NULL) {
            if (msg->writer.onDirectWrite != NULL) {
                msg->writer.onDirectWrite(&mModuleInstance.rilat.instance, packMsgId);
            }
        } else {
            rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) msg->payload, msg->payloadLength, 0);
        }
    }

    Rilat_AtResponse_t *response = NULL;
    if (rilat_writeLine(&mModuleInstance.rilat.instance, usingRAI ? ",3" : ",0", &response, 10000) != 0 || response == NULL || response->success == 0) {
        goto l_errorReturn;
    }
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    
//    qSTimer_t wait;
//    qSTimer_Set(&wait, 10000);
//    while (!qSTimer_Expired(&wait)) {
//        rilat_loop(&mModuleInstance.rilat.instance);
//        
//        if (mModuleInstance.state.urcResponseFlags & CMODULE_URC_FLAG_PUB_SUCCESS) {
//            break;
//        } else if (mModuleInstance.state.urcResponseFlags & CMODULE_URC_FLAG_PUB_FAILED) {
//            goto l_errorReturn;
//        }
//    }
//    if (qSTimer_Expired(&wait)) {
//        msg->writer.isWritenSuccess = false;
//    } else {
        rc = 0;
        msg->writer.isWritenSuccess = true;
//    }
    
    return rc;

l_errorReturn:
    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return rc;
}


static int32_t lwm2mResponse(uint32_t id) {
    return 0;
}


static int32_t lwm2mUpdate(uint32_t commandTimeout) {
//    static const char * const AT_CMD_MIPLUPDATE = "AT+CTM2MUPDATE";
//    
//    int32_t rc = -1;
//    int32_t waitCount;
//    Rilat_AtResponse_t *response = NULL;
//    
//    mModuleInstance.state.urcResponseFlags &= ~CMODULE_URC_FLAG_STACK_UPDATE_SUCCESS;

//    if (rilat_writeLine(&mModuleInstance.rilat.instance, AT_CMD_MIPLUPDATE, &response, commandTimeout) != 0 || response == NULL || response->success == 0) {
//        rilat_freeResponse(&mModuleInstance.rilat.instance, response);
//        return -1;
//    }
//    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
//    response = NULL;
//    
//    qSTimer_t wait;
//    qSTimer_Set(&wait, commandTimeout);
//    while (!qSTimer_Expired(&wait)) {
//        rilat_loop(&mModuleInstance.rilat.instance);
//        
//        if (mModuleInstance.state.urcResponseFlags & CMODULE_URC_FLAG_STACK_UPDATE_SUCCESS) {
//            break;
//        }
//    }
//    if (qSTimer_Expired(&wait)) {
//        return -1;
//    }
//    
//    return 0;
    return -1;
}

static void powerSet(bool up) {
    void *pin = CREQUEST(ON_MODEM_POWER_PIN_ACCESS, {}).ptr;
    uint16_t delay = 0;

    if (up) {
        if (pin != NULL && (uint32_t) pin != UINT32_MAX) {
            delay = 3100;
        }
    } else {
        if (pin != NULL && (uint32_t) pin != UINT32_MAX) {
            delay = 3600;
        } else if (pin == NULL) {
            // rilat_writeLine(&mModuleInstance.rilat.instance, "AT+MPOF=1", NULL, 1000);
            return;
        }
    }

    if (delay != 0) {
        uint8_t lv = CREQUEST(ON_MODEM_POWER_PIN_LEVEL, {}).u32 ? true : false;
        kdgpio_t *pwrPin = pin;
        kdgpio_init((kdgpio_t *) pwrPin);
        kdgpio_powerUp((kdgpio_t *) pwrPin, KDGPIO_MODE_OUTPUT_PP, KDGPIO_PULL_NONE);
        kdgpio_output((kdgpio_t *) pwrPin, lv);
        _cModule_wait(&mModuleInstance, delay, 0);
        kdgpio_output((kdgpio_t *) pwrPin, !lv);
        kdgpio_powerDown((kdgpio_t *) pwrPin);
        kdgpio_finalize((kdgpio_t *) pwrPin);
    }
}

static void __onHalSleep(void) {

}


static void __onHalWakeup(void) {

}


static void __onRilatInit(void) {

}


static void __onRilatFinalize(void) {

}


static void __onSoftWakeup(void) {

}


static void __onSoftSleep(void) {
    AT();
    AT();
#if CONFIG_CMODULE_INSTANCE_INIT_MODE == CONFIG_CMODULE_INSTANCE_INIT_MODE_LWM2M
    lwm2mDelete(10000);
#endif
    if (mModuleInstance.aux.flag.pmuSupport) {
        powerSet(false);
    }
}


static void onResetStack(void) {
#if CONFIG_CMODULE_INSTANCE_INIT_MODE == CONFIG_CMODULE_INSTANCE_INIT_MODE_LWM2M
    lwm2mDelete(10000);
#endif
    onReboot(false);
}


static void onReboot(uint8_t isPowerUpRequest) {
    if (isPowerUpRequest) {
        powerSet(true);
    } else {
        void *pin = CREQUEST(ON_MODEM_RESET_PIN_ACCESS, {.ptr = &mModuleInstance}).ptr;
        if (pin != NULL && (uint32_t) pin != UINT32_MAX) {
            uint8_t lv = CREQUEST(ON_MODEM_RESET_PIN_LEVEL, {}).u32 ? true : false;
            kdgpio_t *rstPin = pin;
            kdgpio_init((kdgpio_t *) rstPin);
            kdgpio_powerUp((kdgpio_t *) rstPin, KDGPIO_MODE_OUTPUT_PP, KDGPIO_PULL_NONE);
            kdgpio_output((kdgpio_t *) rstPin, lv);
            _cModule_wait(&mModuleInstance, 400, 0);
            kdgpio_output((kdgpio_t *) rstPin, !lv);
            kdgpio_powerDown((kdgpio_t *) rstPin);
            kdgpio_finalize((kdgpio_t *) rstPin);
        } else {
            reboot();
        }
    }
}


static void onLoop(void) {
    if (qSTimer_Expired(&mModuleInstance.aux.pollTimer)) {
        qSTimer_Set(&mModuleInstance.aux.pollTimer, 60000);

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

#if CONFIG_CMODULE_INSTANCE_INIT_MODE == CONFIG_CMODULE_INSTANCE_INIT_MODE_LWM2M

RILAT_COMMAND_MATCH_DEFINE(_CTM2MUPDATE, "+CTM2MUPDATE:", data, len) {
    if (mModuleInstance.aux.flag.passiveRecvMode) {
        return -1;
    }
    
    mModuleInstance.state.urcResponseFlags |= CMODULE_URC_FLAG_STACK_UPDATE_SUCCESS;
    return 0;
}


RILAT_COMMAND_MATCH_DEFINE(_MIPLEVENT, "+CTM2M:", data, len) {
    if (mModuleInstance.aux.flag.passiveRecvMode) {
        return -1;
    }
    
    char *line = (char *) data;
    char *typeStr = NULL;
    uint32_t rc;
    
    if (klAttoken_start(&line) != 0) {
        return 0;
    }
    
    if (strstr(line, "obsrv")) {
        klAttoken_skip(&line);
        
        if (klAttoken_getNextUInt(&line, &rc) != 0) {
            return 0;
        }
        
        if (rc == 0) {
            mModuleInstance.state.urcResponseFlags |= CMODULE_URC_FLAG_CONN_SUCCESS | CMODULE_URC_FLAG_STACK_READY;
        } else {
            mModuleInstance.state.urcResponseFlags |= CMODULE_URC_FLAG_CONN_FAILED | CMODULE_URC_FLAG_NEED_RESET;
            mModuleInstance.state.urcResponseFlags &= ~(CMODULE_URC_FLAG_STACK_READY);
        }
    } else if (strstr(line, "lwstatus")) {
        klAttoken_skip(&line);
        
        if (klAttoken_getNextUInt(&line, &rc) != 0) {
            return 0;
        }
        
        if (rc == 29) {
            mModuleInstance.state.urcResponseFlags |= CMODULE_URC_FLAG_CONN_SUCCESS | CMODULE_URC_FLAG_STACK_READY;
        } else {
            mModuleInstance.state.urcResponseFlags |= CMODULE_URC_FLAG_CONN_FAILED | CMODULE_URC_FLAG_NEED_RESET;
            mModuleInstance.state.urcResponseFlags &= ~(CMODULE_URC_FLAG_STACK_READY);
        }
    } else if (strstr(line, "reg")) {
        klAttoken_skip(&line);
        
        if (klAttoken_getNextUInt(&line, &rc) != 0) {
            return 0;
        }
        
        if (rc == 1 || rc == 2) {
            mModuleInstance.state.urcResponseFlags |= CMODULE_URC_FLAG_CONN_FAILED | CMODULE_URC_FLAG_NEED_RESET;
            mModuleInstance.state.urcResponseFlags &= ~(CMODULE_URC_FLAG_STACK_READY);
        }
    } else if (strstr(line, "update")) {
        klAttoken_skip(&line);
        
        if (klAttoken_getNextUInt(&line, &rc) != 0) {
            return 0;
        }
        
        if (rc == 0) {
            mModuleInstance.state.urcResponseFlags |= CMODULE_URC_FLAG_STACK_UPDATE_SUCCESS;
        } else {
            mModuleInstance.state.urcResponseFlags &= ~CMODULE_URC_FLAG_STACK_UPDATE_SUCCESS;
        }
    } else if (strstr(line, "send")) {
        klAttoken_skip(&line);
        
        if (klAttoken_getNextUInt(&line, &rc) != 0) {
            return 0;
        }
        
        if (rc == 0) {
            mModuleInstance.state.urcResponseFlags |= CMODULE_URC_FLAG_PUB_SUCCESS;
        } else if (rc == 1 || rc == 2 || rc == 9 || rc == 11 || rc == 32) {
            mModuleInstance.state.urcResponseFlags &= ~CMODULE_URC_FLAG_PUB_SUCCESS;
            mModuleInstance.state.urcResponseFlags |= CMODULE_URC_FLAG_PUB_FAILED;
        }
    }

    return 0;
}


RILAT_COMMAND_MATCH_DEFINE(_MIPLWRITE, "+CTM2MRECV:", data, len) {
    if (mModuleInstance.aux.flag.passiveRecvMode) {
        return -1;
    }
    
    char *line = (char *) data;
    uint16_t payloadLength = 0;
    char *payload = NULL;
    
    if (klAttoken_start(&line) != 0) {
        return 0;
    }
    
    while (*line != '\0' && isspace(*line)) {
        (line)++;
    }
    
    payloadLength = ALIGN_UP(strlen(line) + 1, 16);
    payload = calloc(1, payloadLength);
    if (payload == NULL) {
        return -1;
    }
    payloadLength = klStr_hexStr2hex(line, payloadLength, payload);

    if (!_cModule_isAllocRepPackAccess(&mModuleInstance)) {
        LOG_W("!_cModule_isAllocRepPackAccess");
        free(payload);
        return -1;
    }
    _cModule_packMutexLock(&mModuleInstance, true);
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
    
    return 0;
    
    l_errorExit:
    _cModule_packMutexLock(&mModuleInstance, false);
    if (pack != NULL) {
        free(pack);
    }
    if (payload != NULL) {
        free(payload);
    }
    return -1;
}

#endif

/*@}*/

/**
 * @addtogroup Init
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
        if (queryIMEI() == 0
                && queryIMSI() == 0) {
            setNextStep();
        } else {
            setErrorStep(10, CMODULE_STATE_NO_COM_ID, 1000);
        }
    } else if (step == 4) {
        if (checkCGREG() == 0) {
            readSignal();
            checkNTP();
            setNextStep();
        } else {
            setErrorStep(120, CMODULE_STATE_FAILED_REG, 300);
        }
    } 
#if CONFIG_CMODULE_INSTANCE_INIT_MODE == CONFIG_CMODULE_INSTANCE_INIT_MODE_LWM2M
    else if (step == 5) {
        int32_t rc = lwm2mUpdate(10000);
        if (rc == 0) {
            setSuccessStep(120);
        } else {
            setNextStep();
        }
    } else if (step == 6) {
        int32_t rc = lwm2mInit(10000);
        if (rc == 0) {
            setSuccessStep(120);
        } else {
            lwm2mDelete(1000);
            setErrorStep(0, CMODULE_STATE_FAILED_CON, 1000);
        }
    }
#endif
}

/*@}*/

/**
 * @addtogroup Protocol-CAAEP
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
#if CONFIG_CMODULE_INSTANCE_INIT_MODE == CONFIG_CMODULE_INSTANCE_INIT_MODE_MQTT
    if (info->flag.packIsDynMqttMsg) {
        cModule_ProtocolMqttMessage_t *msg = info->payload;

        if (msg->isPayloadJson) {
#if CONFIG_CMODULE_CJSON_SUPPORT == 1
            cModule_ProtocolMqttMessage_t *msg = info->payload;
            if (_cModule_onProtocolTransmited(&mModuleInstance, info) != 0) {
                return rc;
            }

            cJSON *cjValue = (void *) msg->payload;
            char *payload = cJSON_PrintUnformatted(cjValue);
            if (payload != NULL) {
                rc = mqttPubTopic(msg, payload, strlen(payload));
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
            cModule_ProtocolMqttMessage_t *msg = info->payload;
            rc = mqttPubTopic(msg, NULL, 0);
        } else {
            return rc;
        }
    }
#endif
#if CONFIG_CMODULE_INSTANCE_INIT_MODE == CONFIG_CMODULE_INSTANCE_INIT_MODE_NONE
    if (info->flag.packIsDynHttpMsg) {
        cModule_ProtocolHttpMessage_t *msg = info->payload;
        rc = httpPost(msg);
    }
#endif
#if CONFIG_CMODULE_INSTANCE_INIT_MODE == CONFIG_CMODULE_INSTANCE_INIT_MODE_LWM2M
    if (info->flag.packIsDynData) {
        cModule_ProtocolGenMessage_t *msg = info->payload;
        return lwm2mPost(msg, msg->payload, msg->payloadLength, 0, info->aux.gen.msgId);
    }
#endif
    return rc;
}


static int32_t onPtPackIdCustom(cModule_TransmitPackageInfo_t *info) {
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
    } else if (info->flag.packIsDynData) {
        cModule_ProtocolGenMessage_t *msg = info->payload;
        if (msg->writer.onDirectWriteResponse != NULL) {
            msg->writer.onDirectWriteResponse(&mModuleInstance.rilat.instance, info->aux.gen.msgId, msg->writer.isWritenSuccess, isForce);
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

static int32_t rilat_onCallback(Rilat_Instance_t *instance, Rilat_CallbackEvent_t event, Rilat_CallbackVar_t var1, Rilat_CallbackVar_t var2, Rilat_CallbackVar_t var3) {
    if (event == RILAT_CALL_EVENT_ON_INIT) {
        CREQUEST(ON_SERIAL_INIT, {});
    } else if (event == RILAT_CALL_EVENT_ON_FINALIZE) {
        CREQUEST(ON_SERIAL_FINALIZE, {});
    } else if (event == RILAT_CALL_EVENT_ON_READ) {
        *((uint32_t *) var3.ptr) = CREQUEST(ON_SERIAL_RECV, (void *) &mModuleInstance, (void *) var1.ptr, (uint32_t) var2.u32).u32;
    } else if (event == RILAT_CALL_EVENT_ON_WRITE) {
        return CREQUEST(ON_SERIAL_TRANSMIT, (void *) &mModuleInstance, (void *) var1.ptr, (uint32_t) var2.u32).i32;
    } else if (event == RILAT_CALL_EVENT_ON_BLOCK_WAIT) {
        return CREQUEST(ON_BLOCK_POLL, (void *) &mModuleInstance, (uint32_t) (mModuleInstance.state.urcResponseFlags & CMODULE_URC_FLAG_STACK_NO_BLOCK_POLL) ? 1 : 0).i32;
    } else if (event == RILAT_CALL_EVENT_ON_POLL_HOCK) {
        return CREQUEST(ON_BLOCK_POLL, (void *) &mModuleInstance,(uint32_t) (mModuleInstance.state.urcResponseFlags & CMODULE_URC_FLAG_STACK_NO_BLOCK_POLL) ? 1 : 0).i32;
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
    mModuleInstance.aux.flag.pmuSupport = false;
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
        CREQUEST(ON_SERIAL_INIT, {});
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
        CREQUEST(ON_SERIAL_FINALIZE, {});
    }
}

/*@}*/

/**
 * @addtogroup Instance
 * @note none
 */
 
/*@{*/

cModule_Instance_t *cModule_getInstance_nb63(void) {
    return &mModuleInstance;
}

/*@}*/

#endif

/**
 * @addtogroup Debug support
 * @note none
 */
 
/*@{*/

#undef TAG

/*@}*/
