//
// Created by Fland on 2024/6/22.
//

#ifndef RM_DAC8560_H
#define RM_DAC8560_H

#include <stdint.h>

/**
 * @addtogroup Typedef
 * @note none
 */

/*@{*/

typedef struct dac8560_Instance dac8560_Instance_t;

struct dac8560_Instance {
    void *kdspi;
    struct {
        uint16_t refMv;
    } config;
};

typedef enum {
    DAC8560_POWER_CTL_NORAMAL = 0,
    DAC8560_POWER_CTL_1K_GND = 1,
    DAC8560_POWER_CTL_100K_GND = 2,
    DAC8560_POWER_CTL_HIZ = 3,
} dac8560_PowerCtl_t;

/*@}*/

/**
 * @addtogroup ExportAuxFunc
 * @note none
 */

/*@{*/

#define DAC8560_GET_RAW(ref, mv)    ((uint16_t) ((float) mv / (float) ref * (float) 0xFFFF))

/*@}*/

/**
 * @addtogroup ExportFunc
 * @note none
 */

/*@{*/

extern int32_t dac8560_hardInit(dac8560_Instance_t *ins, void *iicIf);
extern int32_t dac8560_hardFinalize(dac8560_Instance_t *ins);

extern int32_t dac8560_softInit(dac8560_Instance_t *ins, bool iVrefEnable, dac8560_Gain_t gain, uint16_t vref);
extern void dac8560_softFinalize(dac8560_Instance_t *ins, dac8560_PowerCtl_t ctl);

extern uint32_t dac8560_caliRawByVolt(dac8560_Instance_t *ins, float mv);

extern int32_t dac8560_setOutVolt(dac8560_Instance_t *ins, dac8560_Channel_t ch, float mv);
extern int32_t dac8560_setOutRaw(dac8560_Instance_t *ins, dac8560_Channel_t chn, uint16_t u);

/*@}*/

#endif
