//
// Created by Fland on 2024/6/22.
//

#ifndef RM_GP8211_H
#define RM_GP8211_H

#include <stdint.h>

/**
 * @addtogroup Typedef
 * @note none
 */

/*@{*/

typedef enum {
    GP8211_RANGE_2V5 = 0,
    GP8211_RANGE_5V0 = 1,
    GP8211_RANGE_10V0 = 2,
    GP8211_RANGE_VCC = 3,
} GP8211_Range_t;

typedef struct GP8211_Instance GP8211_Instance_t;

struct GP8211_Instance {
    void *iicIf;
    struct {
        GP8211_Range_t range;
        uint16_t resolution;
    } config;
};

/*@}*/

/**
 * @addtogroup ExportAuxFunc
 * @note none
 */

/*@{*/

#define GP8211_GET_RAW(ref, mv)    (((uint16_t) ((float) mv / (float) ref * (float) 0x7FFF)))

/*@}*/

/**
 * @addtogroup ExportFunc
 * @note none
 */

/*@{*/

extern int32_t gp8211_hardInit(GP8211_Instance_t *ins, void *iicIf);
extern int32_t gp8211_hardFinalize(GP8211_Instance_t *ins);

extern int32_t gp8211_softInit(GP8211_Instance_t *ins);
extern void gp8211_softFinalize(GP8211_Instance_t *ins);

extern int32_t gp8211_setRange(GP8211_Instance_t *ins, GP8211_Range_t range);
extern int32_t gp8211_setResolution(GP8211_Instance_t *ins, uint16_t resolution);

extern uint32_t gp8211_caliRawByVolt(GP8211_Instance_t *ins, float mv);
extern float gp8211_getVoltByRaw(GP8211_Instance_t *ins, uint16_t value);

extern int32_t gp8211_setOutVolt(GP8211_Instance_t *ins, uint16_t voltage, uint8_t channel);
extern int32_t gp8211_setOutVolt_gp8512(GP8211_Instance_t *ins, uint16_t voltage, uint8_t channel);

/*@}*/

#endif //RM_MS1100_H
