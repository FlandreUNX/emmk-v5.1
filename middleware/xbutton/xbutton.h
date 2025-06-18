//
// Created by Fland on 24-11-26.
//

#ifndef BASE_V1_0_0A_20241125_DEBUG_XBUTTON_H
#define BASE_V1_0_0A_20241125_DEBUG_XBUTTON_H

#include <stdint.h>

#include "kdgpio.h"

/**
 * @addtogroup Typedef & Define
 * @note none
 */

/*@{*/

typedef struct {
    void *gpio;
    kdgpio_Mode_t gpioMode;
    kdgpio_PullResistor_t gpioPullResistor;

    struct {
        uint8_t isLongPressBtn: 1;
        uint8_t pressingLevel: 1;

        uint16_t accessCountTh1;
        uint16_t accessCountTh2;
        uint16_t accessCountThEmit;
    } opts;
} xbutton_HalMapper_t;

typedef enum {
    XBUTTON_ROTENC_MODE_ALPS,
    XBUTTON_ROTENC_MODE_1N1P,
    XBUTTON_ROTENC_MODE_2N1P,
} xbutton_RotEncMode_t;

typedef struct {
    struct {
        void *pinA;
        kdgpio_Mode_t pinAGpioMode;
        kdgpio_PullResistor_t pinAGpioPullResistor;

        void *pinB;
        kdgpio_Mode_t pinBGpioMode;
        kdgpio_PullResistor_t pinBGpioPullResistor;
    } hal;
    struct {
        uint8_t attachA: 1;
        uint8_t attachB: 1;

        uint8_t outputA: 1;
        uint8_t outputB: 1;

        uint8_t cwxA: 1;
        uint8_t cwxB: 1;

        xbutton_RotEncMode_t mode;

        struct {
            uint8_t alpsIdleA: 1;
            uint8_t alpsIdleB: 1;
        } alps;

        uint16_t releaseCount[2];

        uint8_t btnObjIndexA;
        uint8_t btnObjIndexB;
    } aux;
} xbutton_RotEncObj_t;

typedef struct {
    xbutton_HalMapper_t hal;

    uint8_t index;
    uint32_t pressingCount;
    struct {
        uint8_t access: 1;

        uint8_t pressingPre: 1;
        uint8_t pressingAccess: 1;
        uint8_t releaseAccess: 1;
        uint8_t isLongBtnSet: 1;
    } flag;

    struct {
        uint8_t setStep;
    } enc;
} xbutton_Obj_t;

typedef enum {
    XBUTTON_EVT_LEVEL_ACTING,
    XBUTTON_EVT_PRESSING,
    XBUTTON_EVT_SHORT_PRESS,
    XBUTTON_EVT_LONG_PRESS,
    XBUTTON_EVT_RELEASE,
} xbutton_Event_t;

/*@}*/

/**
 * @addtogroup ExportFunc
 * @note none
 */

/*@{*/

extern void xbutton_init(xbutton_Obj_t *objs, uint8_t btnCount);
extern void xbutton_finalize(void);

extern void xbutton_rotEnc_init(xbutton_RotEncObj_t *objs, uint8_t encCount);
extern void xbutton_rotEnc_finalize(void);
extern void xbutton_enc_pinAIrqHandler(uint32_t rotIndex);

extern int32_t xbutton_sync(void);

extern void xbutton_refresh(void);

/*@}*/

#endif //BASE_V1_0_0A_20241125_DEBUG_XBUTTON_H
