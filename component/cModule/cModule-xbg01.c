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
 * @addtogroup DebugSupport
 * @note none
 */

/*@{*/

#undef TAG
#define TAG     "xbg01"

#define CASSERT(err) ASSERT(err)

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
#define CREQUEST(type, var, ...)        _MODULE_REQUEST_CALL((uint32_t) _MODULE_REQUEST_TYPE(type), (_MODULE_REQUEST_VARS) var, ##__VA_ARGS__)

extern _MODULE_REQUEST_VARS _MODULE_REQUEST_CALL(_MODULE_REQUEST_TYPED type, _MODULE_REQUEST_VARS var, ...);

/*@}*/

/**
 * @addtogroup Config
 * @note none
 */

/*@{*/

/// @-1=Disable
#if CONFIG_CMODULE_INSTANCE_X_ENABLE == 81
#define CONFIG_CMODULE_INSTANCE_TAG_ID                    81
#else
#define CONFIG_CMODULE_INSTANCE_TAG_ID                    -1
#endif

#define CONFIG_SERIAL_BAURRATE                             (115200)

/*@}*/

#if CONFIG_CMODULE_INSTANCE_TAG_ID >= 0

#if CONFIG_CMODULE_INSTANCE_PMU_SUPPORT != 0
CMPMU_DEFINE();
#endif

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
#if CONFIG_CMODULE_INSTANCE_PMU_SUPPORT != 0
        .cmmpu = &CMPMU,
#else
        .cmmpu = NULL,
#endif
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
    static const char AT_A[] = "AX+AT=0";

    uint8_t isSuccess = 0;
    Rilat_AtResponse_t *response = NULL;

#if CONFIG_SERIAL_PHY_ENABLE == 1
    kduart_flush(((kduart_t *) mModuleInstance.rilat.instance.userData));
#endif
    rilat_writeLine(&mModuleInstance.rilat.instance, AT_A, NULL, 500);
    for (uint8_t i = 0; i < 10; i++) {
        rilat_writeLine(&mModuleInstance.rilat.instance, AT_A, &response, 500);
        if (response != NULL && response->success) {
            rilat_freeResponse(&mModuleInstance.rilat.instance, response);
            response = NULL;

            isSuccess = 1;
            break;
        }

        rilat_freeResponse(&mModuleInstance.rilat.instance, response);
        response = NULL;

        if (mModuleInstance.state.urcResponseFlags & CMODULE_URC_FLAG_STACK_BLOCK) {
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

    rilat_writeLine(&mModuleInstance.rilat.instance, "AX+AT=0", &response, 1000);
    if (response == NULL || response->success == 0) {
        rilat_freeResponse(&mModuleInstance.rilat.instance, response);
        return -1;
    }

    rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return 0;
}


static void reboot(void) {
    static const char AT_RST[] = "AX+REBOOT=1";
    rilat_writeLine(&mModuleInstance.rilat.instance, AT_RST, NULL, 500);
}


static int32_t reportRda1(char *payload, uint32_t payloadLength) {
    static const char AT_RDA1[] = "AX+RDA1=";
    // int32_t rc = -1;
    // Rilat_AtResponse_t *response = NULL;
    
    mModuleInstance.state.urcResponseFlags &= ~(CMODULE_URC_FLAG_PUB_SUCCESS | CMODULE_URC_FLAG_PUB_FAILED);
    
    rilat_directWrite(&mModuleInstance.rilat.instance, (uint8_t *) AT_RDA1, strlen(AT_RDA1), 0);
    rilat_writeLine(&mModuleInstance.rilat.instance, payload, NULL, 10);
    // if (rilat_writeLine(&mModuleInstance.rilat.instance, payload, &response, 1) != 0 || response == NULL || response->success == 0) {
    //     rc = -1;
    //     goto _l_retryExit;
    // }
    //
    // rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    
    // qSTimer_t wait;
    // qSTimer_Set(&wait, 15000);
    // while (!qSTimer_Expired(&wait)) {
    //     if (mModuleInstance.state.urcResponseFlags & CMODULE_URC_FLAG_PUB_SUCCESS
    //             || mModuleInstance.state.urcResponseFlags & CMODULE_URC_FLAG_PUB_FAILED) {
    //         break;
    //     }
    //     if (_cModule_wait(&mModuleInstance, 1, true) < 0) {
    //         wait = 0;
    //         break;
    //     }
    // }
    // if (qSTimer_Expired(&wait) || mModuleInstance.state.urcResponseFlags & CMODULE_URC_FLAG_PUB_FAILED) {
    //     return -1;
    // }
    
    return 0;
    
    _l_retryExit:
    mModuleInstance.aux.initRetryCount++;
    // rilat_freeResponse(&mModuleInstance.rilat.instance, response);
    return -1;
}


static void powerDown(void) {
#ifdef CONFIG_PWR_PIN_PATH
    kdGPIO_t *pwrPin = KDINSTANCE(GPIO, CONFIG_PWR_PIN_PATH);
    ASSERT(pwrPin != NULL);
    kdgpio_init((kdGPIO_t *) pwrPin);
    kdgpio_powerUp((kdGPIO_t *) pwrPin, 1);
    kdgpio_output((kdGPIO_t *) pwrPin, 1);
    _cModule_wait(&mModuleInstance, 3600, 0);
    kdgpio_output((kdGPIO_t *) pwrPin, 0);
    kdgpio_powerDown((kdGPIO_t *) pwrPin);
    kdgpio_finalize((kdGPIO_t *) pwrPin);
#else
#endif
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
#if CONFIG_CMODULE_INSTANCE_PMU_SUPPORT != 0
    powerDown();
#endif
}


static void onResetStack(void) {
    onReboot(false);
}


static void onReboot(uint8_t isPowerUpRequest) {
    if (isPowerUpRequest) {
        LOG_D("%s, PowerUpRequest", TAG);
#ifdef CONFIG_PWR_PIN_PATH
        kdGPIO_t *pwrPin = KDINSTANCE(GPIO, CONFIG_PWR_PIN_PATH);
        ASSERT(pwrPin != NULL);
        kdgpio_init((kdGPIO_t *) pwrPin);
        kdgpio_powerUp((kdGPIO_t *) pwrPin, 1);
        kdgpio_output((kdGPIO_t *) pwrPin, 1);
        _cModule_wait(&mModuleInstance, 3100, 0);
        kdgpio_output((kdGPIO_t *) pwrPin, 0);
        kdgpio_powerDown((kdGPIO_t *) pwrPin);
        kdgpio_finalize((kdGPIO_t *) pwrPin);
#endif
    } else {
        LOG_D("%s, RestReqeust", TAG);
#ifdef CONFIG_RST_PIN_PATH
        kdGPIO_t *rstPin = KDINSTANCE(GPIO, CONFIG_RST_PIN_PATH);
        ASSERT(rstPin != NULL);
        kdgpio_init((kdGPIO_t *) rstPin);
        kdgpio_powerUp((kdGPIO_t *) rstPin, 1);
        kdgpio_output((kdGPIO_t *) rstPin, 1);
        _cModule_wait(&mModuleInstance, 400, 0);
        kdgpio_output((kdGPIO_t *) rstPin, 0);
        kdgpio_powerDown((kdGPIO_t *) rstPin);
        kdgpio_finalize((kdGPIO_t *) rstPin);
#else
        reboot();
#endif
    }
}


static void onLoop(void) {
    if (qSTimer_Expired(&mModuleInstance.aux.pollTimer)) {
        qSTimer_Set(&mModuleInstance.aux.pollTimer, 60 * 1000);

        AT();
        if (AT() != 0) {
            if (++mModuleInstance.aux.initRetryCount > 2) {
                mModuleInstance.aux.initRetryCount = 0;
                mModuleInstance.state.urcResponseFlags |= CMODULE_URC_FLAG_NEED_RESET;
            }
        }
    }
}

/*@}*/

/**
 * @addtogroup EC800E-URC
 * @note none
 */

/*@{*/

RILAT_COMMAND_MATCH_DEFINE(_PAIRC, "+PAIRC:", data, len) {
    if (mModuleInstance.aux.flag.passiveRecvMode) {
        return -1;
    }

    char *line = (char *) data;
    uint32_t rc = 0;
    
    if (klAttoken_start(&line) != 0) {
        return -1;
    }
    if (klAttoken_getNextUInt(&line, &rc) != 0) {
        return -1;
    }
    CREQUEST(EXTMOD81_ON_PAIRC, {.u32 = rc});
    
    return 0;
}

RILAT_COMMAND_MATCH_DEFINE(_PAIRSS, "+PAIRSS:", data, len) {
    if (mModuleInstance.aux.flag.passiveRecvMode) {
        return -1;
    }

    char *line = (char *) data;
    uint32_t idx = 0;
    
    if (klAttoken_start(&line) != 0) {
        return -1;
    }
    if (klAttoken_getNextUInt(&line, &idx) != 0) {
        return -1;
    }
    CREQUEST(EXTMOD81_ON_PAIRSS, {.u32 = idx});
    
    return 0;
}

RILAT_COMMAND_MATCH_DEFINE(_NET, "+NET:", data, len) {
    if (mModuleInstance.aux.flag.passiveRecvMode) {
        return -1;
    }

    char *line = (char *) data;
    uint32_t rc = 0;
    
    if (klAttoken_start(&line) != 0) {
        return -1;
    }
    if (klAttoken_getNextUInt(&line, &rc) != 0) {
        return -1;
    }
    mModuleInstance.state.currentState = rc;
    
    return 0;
}

RILAT_COMMAND_MATCH_DEFINE(_RDA1R, "+RDA1R:", data, len) {
    if (mModuleInstance.aux.flag.passiveRecvMode) {
        return -1;
    }

    char *line = (char *) data;
    uint32_t rc = 0;
    
    if (klAttoken_start(&line) != 0) {
        return -1;
    }
    klAttoken_skip(&line);   // mid
    if (klAttoken_getNextUInt(&line, &rc) != 0) {
        return -1;
    }
    if (rc != 1) {
        mModuleInstance.state.urcResponseFlags |= CMODULE_URC_FLAG_PUB_FAILED;
    } else {
        mModuleInstance.state.urcResponseFlags |= CMODULE_URC_FLAG_PUB_SUCCESS;
    }
    
    return 0;
}


RILAT_COMMAND_MATCH_DEFINE(_RDA1Q, "+RDA1Q:", data, len) {
    if (mModuleInstance.aux.flag.passiveRecvMode) {
        return -1;
    }

    char *line = (char *) data;

    if (klAttoken_start(&line) != 0) {
        return -1;
    }
  
    char *payload = calloc(1, strlen(line) + 1);
    if (payload == NULL) {
        return -1;
    }
    memcpy(payload, line, strlen(line));
    
    _cModule_packMutexLock(&mModuleInstance, true);
    cModule_TransmitPackageInfo_t *pack = calloc(1, sizeof(cModule_TransmitPackageInfo_t));
    if (pack == NULL) {
        goto l_errorExit;
    }
    pack->payloadLength = strlen(payload) + 1;
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

/*@}*/

/**
 * @addtogroup EC800E-Init
 * @note none
 */

/*@{*/

static void onInitStep(uint8_t step) {
    mModuleInstance.aux.initRetryCount = 0;
    mModuleInstance.state.processStep = CMODULE_PROCESS_STEP_INIT_COMPLETED;
    qSTimer_Set(&mModuleInstance.aux.pollTimer, 300);
    mModuleInstance.aux.nextPollSec = 120;
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
    return -2;
}


static int32_t onPtPackIdCustom(cModule_TransmitPackageInfo_t *info) {
    if ((uint32_t) info->flag.requestId == (uint32_t) CMODULE_TPACK_RID_PT_AT_RDA1_REPORT) {
        return reportRda1(info->payload, info->payloadLength);
    }
    return 0;
}


static int32_t onPtPackPayloadFree(cModule_TransmitPackageInfo_t *info, bool isForce) {
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
    switch (event) {
        case RILAT_CALL_EVENT_ON_INIT: {
            CREQUEST(ON_SERIAL_INIT, {.u32 = CONFIG_SERIAL_BAURRATE});
            break;
        }
        case RILAT_CALL_EVENT_ON_FINALIZE: {
            CREQUEST(ON_SERIAL_FINALIZE, {});
            break;
        }
        case RILAT_CALL_EVENT_ON_READ: {
            *((uint32_t *) var3.ptr) = CREQUEST(ON_SERIAL_RECV, (void *) &mModuleInstance,
                                                (void *) var1.ptr, (uint32_t) var2.u32).u32;
            return 0;
        }
        case RILAT_CALL_EVENT_ON_WRITE: {
            return CREQUEST(ON_SERIAL_TRANSMIT, (void *) &mModuleInstance,
                            (void *) var1.ptr, (uint32_t) var2.u32).i32;
            break;
        }
        case RILAT_CALL_EVENT_ON_RAW_DATA_RECV: {
            break;
        }
        case RILAT_CALL_EVENT_ON_BLOCK_WAIT: {
            return CREQUEST(ON_BLOCK_POLL, (void *) &mModuleInstance,
                            (uint32_t) (mModuleInstance.state.urcResponseFlags & CMODULE_URC_FLAG_STACK_NO_BLOCK_POLL)
                            ? 1 : 0
            ).i32;
        }
        case RILAT_CALL_EVENT_ON_POLL_HOCK: {
            CREQUEST(ON_BLOCK_POLL, (void *) &mModuleInstance,
                     (uint32_t) (mModuleInstance.state.urcResponseFlags & CMODULE_URC_FLAG_STACK_NO_BLOCK_POLL) ? 1
                                                                                                                : 0
            );
            break;
        }
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
#if CONFIG_CMODULE_INSTANCE_PMU_SUPPORT == 1
    CMPMU.flag._ = 0;
    CMPMU.flag.isReady = 1;
    CMPMU.sleep.currentSleepLevel = COMPONENT_SLEEP_LV_RUN;
    qSTimer_Set(&CMPMU.aux.idleTimer, CONFIG_CMODULE_IDLE_TIMEOUT_MS);
#endif
    
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
        CREQUEST(ON_SERIAL_INIT, {.u32 = CONFIG_SERIAL_BAURRATE});
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

cModule_Instance_t *cModule_getInstance_xbg01(void) {
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