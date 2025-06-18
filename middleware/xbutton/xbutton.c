//
// Created by Fland on 24-11-26.
//

#include "xbutton.h"

#include "emmk-config.h"
#include "emmk-driver.h"

#include "opts_xbutton.h"

/**
 * @addtogroup DebugSupport
 * @note none
 */

/*@{*/

#undef TAG
#define TAG     "xbutton"

/*@}*/

/**
 * @addtogroup Typedef
 * @note none
 */

/*@{*/

/*@}*/

/**
 * @addtogroup Variables
 * @note none
 */

/*@{*/

extern bool xbutton_cb_access(xbutton_Obj_t *obj);
extern bool xbutton_cb_vLevel(xbutton_Obj_t *obj);
extern void xbutton_cb_attach(xbutton_Obj_t *obj, xbutton_Event_t evt);

static xbutton_Obj_t *mBtnObj = NULL;
static uint8_t mBtnCount = 0;
#if CONFIG_XBUTTON_ROTENC_ENABLE != 0
static xbutton_RotEncObj_t *mRotEnc = NULL;
static uint8_t mRotEncCount = 0;
static qSTimer_t mRotEncScanTimer = QSTIMER_INITIALIZER;
#endif
static qSTimer_t mLoopTimer = QSTIMER_INITIALIZER;

/*@}*/

/**
 * @addtogroup PrivateFunc
 * @note none
 */

/*@{*/

#if CONFIG_XBUTTON_ROTENC_ENABLE != 0

static void enc_irqHandler(uint32_t i) {
    uint32_t pinALevel = kdgpio_input(mRotEnc[i].hal.pinA) ? 1 : 0;
    uint32_t pinBLevel = kdgpio_input(mRotEnc[i].hal.pinB) ? 1 : 0;

    if (mRotEnc[i].aux.mode == XBUTTON_ROTENC_MODE_1N1P) {
        if (pinALevel != mRotEnc[i].aux.cwxA) {
            if (!pinALevel) {
                if (pinBLevel) {
                    mRotEnc[i].aux.attachA = 1;
                    mRotEnc[i].aux.attachB = 0;
                } else {
                    mRotEnc[i].aux.attachA = 0;
                    mRotEnc[i].aux.attachB = 1;
                }
            }
            mRotEnc[i].aux.cwxA = pinALevel;
            mRotEnc[i].aux.cwxB = pinBLevel;
        }
    } else {
        if (pinALevel != mRotEnc[i].aux.cwxA) {
            if (pinALevel) {
                if (!pinBLevel && mRotEnc[i].aux.cwxB) {
                    mRotEnc[i].aux.attachA = 1;
                    mRotEnc[i].aux.attachB = 0;
                }
                if (pinBLevel && !mRotEnc[i].aux.cwxB) {
                    mRotEnc[i].aux.attachA = 0;
                    mRotEnc[i].aux.attachB = 1;
                }
                if (pinBLevel == mRotEnc[i].aux.cwxB) {
                    if (!pinBLevel) {
                        mRotEnc[i].aux.attachA = 1;
                        mRotEnc[i].aux.attachB = 0;
                    } else {
                        mRotEnc[i].aux.attachA = 0;
                        mRotEnc[i].aux.attachB = 1;
                    }
                }
            } else {
                if (!pinBLevel && mRotEnc[i].aux.cwxB) {
                    mRotEnc[i].aux.attachA = 0;
                    mRotEnc[i].aux.attachB = 1;
                }
                if (pinBLevel && !mRotEnc[i].aux.cwxB) {
                    mRotEnc[i].aux.attachA = 1;
                    mRotEnc[i].aux.attachB = 0;
                }
                if (pinBLevel == mRotEnc[i].aux.cwxB) {
                    if (!pinBLevel) {
                        mRotEnc[i].aux.attachA = 0;
                        mRotEnc[i].aux.attachB = 1;
                    } else {
                        mRotEnc[i].aux.attachA = 1;
                        mRotEnc[i].aux.attachB = 0;
                    }
                }
            }
            mRotEnc[i].aux.cwxA = pinALevel;
            mRotEnc[i].aux.cwxB = pinBLevel;
        }
    }
}

static void enc_irqHandler_alps(uint32_t i) {
    uint32_t pinALevel = kdgpio_input(mRotEnc[i].hal.pinA) ? 1 : 0;
    uint32_t pinBLevel = kdgpio_input(mRotEnc[i].hal.pinB) ? 1 : 0;

    if (mRotEnc[i].aux.alps.alpsIdleA && mRotEnc[i].aux.alps.alpsIdleB) {
        if (!pinALevel && pinBLevel) {
            mRotEnc[i].aux.attachA = 1;
        } else if (pinALevel && !pinBLevel) {
            mRotEnc[i].aux.attachB = 1;
        }

        mRotEnc[i].aux.alps.alpsIdleA = kdgpio_input(mRotEnc[i].hal.pinA) ? 1 : 0;
        mRotEnc[i].aux.alps.alpsIdleB = kdgpio_input(mRotEnc[i].hal.pinB) ? 1 : 0;
    } else {
        if (pinALevel && !pinBLevel) {
            mRotEnc[i].aux.attachA = 1;
        } else if (!pinALevel && pinBLevel) {
            mRotEnc[i].aux.attachB = 1;
        }

        mRotEnc[i].aux.alps.alpsIdleA = kdgpio_input(mRotEnc[i].hal.pinA) ? 1 : 0;
        mRotEnc[i].aux.alps.alpsIdleB = kdgpio_input(mRotEnc[i].hal.pinB) ? 1 : 0;
    }
}


void xbutton_enc_pinAIrqHandler(uint32_t rotIndex) {
    if (mRotEnc[(uint32_t) rotIndex].aux.mode == XBUTTON_ROTENC_MODE_ALPS) {
        enc_irqHandler_alps((uint32_t) rotIndex);
    } else {
        enc_irqHandler((uint32_t) rotIndex);
    }
}


static void enc_scan(void) {
#if CONFIG_XBUTTON_ROTENC_FILTER_MS != 0
    if (qSTimer_Status(&mRotEncScanTimer)) {
        if (!qSTimer_Expired(&mRotEncScanTimer)) {
            return;
        }
    }
    qSTimer_Set(&mRotEncScanTimer, CONFIG_XBUTTON_ROTENC_FILTER_MS);
#endif
    for (uint8_t i = 0; i < mRotEncCount; i++) {
        if (mRotEnc[i].aux.mode != XBUTTON_ROTENC_MODE_ALPS) {
            continue;
        }

        uint32_t pinALevel = kdgpio_input(mRotEnc[i].hal.pinA) ? 1 : 0;
        uint32_t pinBLevel = kdgpio_input(mRotEnc[i].hal.pinB) ? 1 : 0;

        if (mRotEnc[i].aux.alps.alpsIdleA != pinALevel) {
            if (pinALevel) {
                if (mRotEnc[i].aux.alps.alpsIdleB && !pinBLevel) {
                    mRotEnc[i].aux.attachA = 1;
                }
                if (!mRotEnc[i].aux.alps.alpsIdleB && pinBLevel) {
                    mRotEnc[i].aux.attachB = 1;
                }
                if ((mRotEnc[i].aux.alps.alpsIdleB == pinBLevel) && !pinBLevel) {
                    mRotEnc[i].aux.attachA = 1;
                }
                if ((mRotEnc[i].aux.alps.alpsIdleB == pinBLevel) && pinBLevel) {
                    mRotEnc[i].aux.attachB = 1;
                }
            } else {
                if (mRotEnc[i].aux.alps.alpsIdleB && !pinBLevel) {
                    mRotEnc[i].aux.attachB = 1;
                }
                if (!mRotEnc[i].aux.alps.alpsIdleB && pinBLevel) {
                    mRotEnc[i].aux.attachA = 1;
                }
                if ((mRotEnc[i].aux.alps.alpsIdleB == pinBLevel) && !pinBLevel) {
                    mRotEnc[i].aux.attachB = 1;
                }
                if ((mRotEnc[i].aux.alps.alpsIdleB == pinBLevel) && pinBLevel) {
                    mRotEnc[i].aux.attachA = 1;
                }
            }
            mRotEnc[i].aux.alps.alpsIdleA = kdgpio_input(mRotEnc[i].hal.pinA) ? 1 : 0;
            mRotEnc[i].aux.alps.alpsIdleB = kdgpio_input(mRotEnc[i].hal.pinB) ? 1 : 0;
        }
    }
}

#endif

static void button_event(uint8_t index, xbutton_Event_t evt) {
    xbutton_cb_attach(&mBtnObj[index], evt);
}

/*@}*/

/**
 * @addtogroup ExportFunc
 * @note none
 */

/*@{*/

void xbutton_init(xbutton_Obj_t *objs, uint8_t btnCount) {
    ASSERT(objs != NULL);
    ASSERT(btnCount > 0);

    qSTimer_Set(&mLoopTimer, CONFIG_XBUTTON_FILTER_MS);
    mBtnObj = objs;
    mBtnCount = btnCount;

    for (int i = 0; i < mBtnCount; i++) {
        mBtnObj[i].index = i;
        mBtnObj[i].flag.access = xbutton_cb_access(&mBtnObj[i]);
        if (mBtnObj[i].flag.access) {
            if (mBtnObj[i].hal.opts.accessCountTh1 == 0) {
                mBtnObj[i].hal.opts.accessCountTh1 = CONFIG_XBUTTON_DEFAULT_ACCESS_TH_COUNT;
            }
            if (mBtnObj[i].hal.opts.accessCountTh2 == 0) {
                mBtnObj[i].hal.opts.accessCountTh2 = CONFIG_XBUTTON_DEFAULT_ACCESS_2_TH_COUNT;
            }
            if (mBtnObj[i].hal.opts.accessCountThEmit == 0) {
                mBtnObj[i].hal.opts.accessCountThEmit = CONFIG_XBUTTON_DEFAULT_ACCESS_2_EMIT_TH_COUNT;
            }

            if (mBtnObj[i].hal.gpio != NULL) {
                kdgpio_init(mBtnObj[i].hal.gpio);
                kdgpio_powerUp(mBtnObj[i].hal.gpio, mBtnObj[i].hal.gpioMode, mBtnObj[i].hal.gpioPullResistor);
            }
        }
    }
}


void xbutton_finalize(void) {
    for (int i = 0; i < mBtnCount; i++) {
        if (xbutton_cb_access(&mBtnObj[i])) {
            if (mBtnObj[i].hal.gpio != NULL) {
                kdgpio_powerDown(mBtnObj[i].hal.gpio);
                kdgpio_finalize(mBtnObj[i].hal.gpio);
            }
        }
    }
}

void xbutton_rotEnc_init(xbutton_RotEncObj_t *objs, uint8_t encCount) {
    ASSERT(objs != NULL);
    ASSERT(encCount > 0);

#if CONFIG_XBUTTON_ROTENC_FILTER_MS != 0
    qSTimer_Set(&mRotEncScanTimer, CONFIG_XBUTTON_ROTENC_FILTER_MS);
#endif
    mRotEnc = objs;
    mRotEncCount = encCount;

#if CONFIG_XBUTTON_ROTENC_ENABLE > 0
    for (uint8_t i = 0; i < mRotEncCount; i++) {
        kdgpio_init(mRotEnc[i].hal.pinA);
        kdgpio_powerUp(mRotEnc[i].hal.pinA, mRotEnc[i].hal.pinAGpioMode, mRotEnc[i].hal.pinAGpioPullResistor);
        kdgpio_irqEnable(mRotEnc[i].hal.pinA, KDGPIO_TRIGGER_RISING_FALLING, NULL);

        kdmisc_delayMs(10);
        mRotEnc[i].aux.alps.alpsIdleA = kdgpio_input(mRotEnc[i].hal.pinA) ? 1 : 0;
        mRotEnc[i].aux.alps.alpsIdleB = kdgpio_input(mRotEnc[i].hal.pinB) ? 1 : 0;

        mBtnObj[mRotEnc[i].aux.btnObjIndexA].enc.setStep = 1;
        mBtnObj[mRotEnc[i].aux.btnObjIndexB].enc.setStep = 1;
    }
#endif
}


void xbutton_rotEnc_finalize(void) {
#if CONFIG_XBUTTON_ROTENC_ENABLE > 0
    for (uint8_t i = 0; i < mRotEncCount; i++) {
        kdgpio_irqEnable(mRotEnc[i].hal.pinA, KDGPIO_TRIGGER_NONE, NULL);
        kdgpio_irqEventClean(mRotEnc[i].hal.pinA, KDGPIO_EVENT_RISING_FALLING);
        kdgpio_powerDown(mRotEnc[i].hal.pinA);
        kdgpio_finalize(mRotEnc[i].hal.pinA);
    }
#endif
}


void xbutton_refresh(void) {
    qSTimer_Disarm(&mLoopTimer);
#if CONFIG_XBUTTON_ROTENC_ENABLE >= 1
    qSTimer_Disarm(&mRotEncScanTimer);
#endif
}

int32_t xbutton_sync(void) {
#if CONFIG_XBUTTON_ROTENC_ENABLE >= 1
    enc_scan();
#endif

    if (qSTimer_Status(&mLoopTimer)) {
        if (!qSTimer_Expired(&mLoopTimer)) {
            return -1;
        }
    }
    qSTimer_Set(&mLoopTimer, CONFIG_XBUTTON_FILTER_MS);

#if CONFIG_XBUTTON_ROTENC_ENABLE >= 1
    for (uint8_t i = 0; i < mRotEncCount; i++) {
        if (!mRotEnc[i].aux.outputB) {
            if (mRotEnc[i].aux.attachA) {
                mRotEnc[i].aux.outputA = 1;
                mRotEnc[i].aux.attachA = 0;
                mRotEnc[i].aux.releaseCount[0] = CONFIG_XBUTTON_ROTENC_RELEASE_COUNT;

                mRotEnc[i].aux.outputB = 0;
                mRotEnc[i].aux.attachB = 0;
            }
        }
        if (mRotEnc[i].aux.outputA && mRotEnc[i].aux.releaseCount[0]) {
            mRotEnc[i].aux.releaseCount[0]--;
        }
        if (mRotEnc[i].aux.releaseCount[0] == 0) {
            mRotEnc[i].aux.outputA = 0;
        }
        if (mRotEnc[i].aux.outputA) {
            mBtnObj[mRotEnc[i].aux.btnObjIndexA].enc.setStep = 2;
        } else {
            mBtnObj[mRotEnc[i].aux.btnObjIndexA].enc.setStep = 1;
        }

        if (!mRotEnc[i].aux.outputA) {
            if (mRotEnc[i].aux.attachB) {
                mRotEnc[i].aux.outputB = 1;
                mRotEnc[i].aux.attachB = 0;
                mRotEnc[i].aux.releaseCount[1] = CONFIG_XBUTTON_ROTENC_RELEASE_COUNT;

                mRotEnc[i].aux.outputA = 0;
                mRotEnc[i].aux.attachA = 0;
            }
        }
        if (mRotEnc[i].aux.outputB && mRotEnc[i].aux.releaseCount[1]) {
            mRotEnc[i].aux.releaseCount[1]--;
        }
        if (mRotEnc[i].aux.releaseCount[1] == 0) {
            mRotEnc[i].aux.outputB = 0;
        }
        if (mRotEnc[i].aux.outputB) {
            mBtnObj[mRotEnc[i].aux.btnObjIndexB].enc.setStep = 2;
        } else {
            mBtnObj[mRotEnc[i].aux.btnObjIndexB].enc.setStep = 1;
        }
    }
#endif

    for (uint8_t i = 0; i < mBtnCount; i++) {
        if (!mBtnObj[i].flag.access) {
            continue;
        }
        uint32_t pressing;
        if (mBtnObj[i].enc.setStep) {
            pressing = mBtnObj[i].enc.setStep - 1;
//            if (pressing) {
//                mBtnObj[i].flag.pressingPre = 1;
//                mBtnObj[i].flag.pressingAccess = 1;
//            }
        } else if (mBtnObj[i].hal.gpio == NULL) {
            pressing = xbutton_cb_vLevel(&mBtnObj[i]);
        } else {
            if ((kdgpio_input(mBtnObj[i].hal.gpio) ? 1 : 0) == mBtnObj[i].hal.opts.pressingLevel) {
                pressing = true;
            } else {
                pressing = false;
            }
        }

        if (pressing) {
            button_event(i, XBUTTON_EVT_LEVEL_ACTING);
            if (!mBtnObj[i].flag.pressingPre && !mBtnObj[i].flag.pressingAccess) {
                mBtnObj[i].flag.pressingPre = 1;
            } else {
                if (++mBtnObj[i].pressingCount >= UINT16_MAX) {
                    mBtnObj[i].pressingCount = UINT16_MAX;
                }
                if (!mBtnObj[i].flag.pressingAccess && mBtnObj[i].pressingCount >= mBtnObj[i].hal.opts.accessCountTh1) {
                    mBtnObj[i].flag.pressingPre = 0;
                    mBtnObj[i].flag.pressingAccess = 1;
                    mBtnObj[i].flag.releaseAccess = 0;
                }
                if (mBtnObj[i].flag.pressingAccess && mBtnObj[i].pressingCount >= mBtnObj[i].hal.opts.accessCountTh2) {
                    if (mBtnObj[i].hal.opts.isLongPressBtn && !mBtnObj[i].flag.isLongBtnSet) {
                        mBtnObj[i].flag.isLongBtnSet = 1;
                        button_event(i, XBUTTON_EVT_LONG_PRESS);
                    } else if (!mBtnObj[i].hal.opts.isLongPressBtn) {
                        if (mBtnObj[i].pressingCount >= mBtnObj[i].hal.opts.accessCountThEmit) {
                            mBtnObj[i].pressingCount = mBtnObj[i].hal.opts.accessCountTh2;
                            button_event(i, XBUTTON_EVT_SHORT_PRESS);
                        }
                    }
                    button_event(i, XBUTTON_EVT_PRESSING);
                }
            }
        } else {
            if (mBtnObj[i].flag.pressingPre) {
                mBtnObj[i].flag.pressingPre = 0;
                mBtnObj[i].flag.isLongBtnSet = 0;
            } else if (mBtnObj[i].flag.pressingAccess) {
                if (!mBtnObj[i].flag.releaseAccess) {
                    mBtnObj[i].flag.releaseAccess = 1;
                } else {
                    mBtnObj[i].flag.pressingPre = 0;
                    mBtnObj[i].flag.pressingAccess = 0;
                    mBtnObj[i].flag.releaseAccess = 0;
                    if (!mBtnObj[i].flag.isLongBtnSet) {
                        button_event(i, XBUTTON_EVT_SHORT_PRESS);
                    }
                    button_event(i, XBUTTON_EVT_RELEASE);
                    mBtnObj[i].pressingCount = 0;
                    mBtnObj[i].flag.isLongBtnSet = 0;
                }
            }
        }
    }

    return 0;
}

/*@}*/

/**
 * @addtogroup DebugSupport
 * @note none
 */

/*@{*/

#undef TAG

/*@}*/
