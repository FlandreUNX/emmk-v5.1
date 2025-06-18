//
// Created by Fland on 2024/6/22.
//

#ifndef RM_GP8503_H
#define RM_GP8503_H

#include <stdint.h>

/**
 * @addtogroup Typedef
 * @note none
 */

/*@{*/

typedef enum {
    GP8503_RANGE_2V5 = 0,
    GP8503_RANGE_VCC = 1,
} gp8503_Range_t;

typedef enum {
    GP8503_CHANNEL_0 = 0x02,
    GP8503_CHANNEL_1 = 0x04,
} gp8503_Channel_t;

typedef struct gp8503_Instance gp8503_Instance_t;

struct gp8503_Instance {
    void *kdi2c;
    struct {
        gp8503_Range_t range;
        float vcc;
    } config;
    union {
        float f;
        uint32_t u;
    } output[2];
};

/*@}*/

/**
 * @addtogroup ExportAuxFunc
 * @note none
 */

/*@{*/

#define GP8503_GET_RAW(ref, mv)    (((uint16_t) ((float) mv / (float) ref * (float) 0xFFF)) << 4)

/*@}*/

/**
 * @addtogroup ExportFunc
 * @note none
 */

/*@{*/

extern int32_t gp8503_hardInit(gp8503_Instance_t *ins, void *iicIf);
extern int32_t gp8503_hardFinalize(gp8503_Instance_t *ins);

extern int32_t gp8503_softInit(gp8503_Instance_t *ins, gp8503_Range_t range, float vcc);
extern void gp8503_softFinalize(gp8503_Instance_t *ins);

extern int32_t gp8503_setOutVolt(gp8503_Instance_t *ins, gp8503_Channel_t ch, float mv);
extern int32_t gp8503_setOutVolt2(gp8503_Instance_t *ins, float mv0, float mv1);

extern uint32_t gp8503_caliRawByVolt(gp8503_Instance_t *ins, float mv);
extern int32_t gp8503_setOutRaw2(gp8503_Instance_t *ins, uint16_t u1, uint16_t u2);

/*@}*/

#endif //RM_MS1100_H
