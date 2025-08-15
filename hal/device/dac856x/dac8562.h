//
// Created by Fland on 2024/6/22.
//

#ifndef RM_DAC8562_H
#define RM_DAC8562_H

#include <stdint.h>

/**
 * @addtogroup Typedef
 * @note none
 */

/*@{*/

typedef struct dac8562_Instance dac8562_Instance_t;

struct dac8562_Instance {
    void *kdspi;
    struct {
        uint16_t refMv;
    } config;
    union {
        float f;
        uint32_t u;
    } output[2];
};

typedef enum {
    DAC8562_POWER_CTL_IGNORE = 0,
    DAC8562_POWER_CTL_UP_A = 0x0001,
    DAC8562_POWER_CTL_UP_B = 0x0002,
    DAC8562_POWER_CTL_UP_AB = 0x0003,
    DAC8562_POWER_CTL_DOWN_A_1K = 0x0010 | 0x0001,
    DAC8562_POWER_CTL_DOWN_B_1K = 0x0010 | 0x0002,
    DAC8562_POWER_CTL_DOWN_AB_1K = 0x0010 | 0x0003,
    DAC8562_POWER_CTL_DOWN_A_100K = 0x0020 | 0x0001,
    DAC8562_POWER_CTL_DOWN_B_100K = 0x0020 | 0x0002,
    DAC8562_POWER_CTL_DOWN_AB_100K = 0x0020 | 0x0003,
    DAC8562_POWER_CTL_DOWN_A_HIZ = 0x0030 | 0x0001,
    DAC8562_POWER_CTL_DOWN_B_HIZ = 0x0030 | 0x0002,
    DAC8562_POWER_CTL_DOWN_AB_HIZ = 0x0030 | 0x0003,
} dac8562_PowerCtl_t;

typedef enum {
    DAC8562_CHN_A = 0,
    DAC8562_CHN_B = 1,
    DAC8562_CHN_AB = 3,
} dac8562_Channel_t;

typedef enum {
    DAC8562_GAIN_B2_A2 = 0,
    DAC8562_GAIN_B2_A1 = 1,
    DAC8562_GAIN_B1_A2 = 2,
    DAC8562_GAIN_B1_A1 = 3,
} dac8562_Gain_t;

/*@}*/

/**
 * @addtogroup ExportAuxFunc
 * @note none
 */

/*@{*/

#define DAC8562_GET_RAW(ref, mv)    ((uint16_t) ((float) mv / (float) ref * (float) 0xFFFF))

/*@}*/

/**
 * @addtogroup ExportFunc
 * @note none
 */

/*@{*/

extern int32_t dac8562_hardInit(dac8562_Instance_t *ins, void *iicIf);
extern int32_t dac8562_hardFinalize(dac8562_Instance_t *ins);

extern int32_t dac8562_softInit(dac8562_Instance_t *ins, bool iVrefEnable, dac8562_Gain_t gain, uint16_t vref);
extern void dac8562_softFinalize(dac8562_Instance_t *ins, dac8562_PowerCtl_t ctl);

extern void dac8562_powerControl(dac8562_Instance_t *ins, dac8562_PowerCtl_t ctl);

extern int32_t dac8562_setOutVolt(dac8562_Instance_t *ins, dac8562_Channel_t ch, float mv);
extern int32_t dac8562_setOutVolt2(dac8562_Instance_t *ins, float mv);

extern uint32_t dac8562_caliRawByVolt(dac8562_Instance_t *ins, float mv);
extern int32_t dac8562_setOutRaw(dac8562_Instance_t *ins, dac8562_Channel_t chn, uint16_t u);
extern int32_t dac8562_setOutRaw2(dac8562_Instance_t *ins, uint16_t u);

/*@}*/

#endif
