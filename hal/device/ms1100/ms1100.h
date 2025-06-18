//
// Created by Fland on 2024/6/22.
//

#ifndef RM_MS1100_H
#define RM_MS1100_H

#include <stdint.h>

/**
 * @addtogroup Typedef
 * @note none
 */

/*@{*/

typedef enum {
    MS1100_PGA_1 = 0,
    MS1100_PGA_2 = 1,
    MS1100_PGA_4 = 2,
    MS1100_PGA_8 = 3,
} MS1100_Pga_t;

typedef enum {
    MS1100_DR_240SPS_12B = 0,
    MS1100_DR_60SPS_14B = 1,
    MS1100_DR_30SPS_15B = 2,
    MS1100_DR_15SPS_16B = 3,
} MS1100_Dr_t;

typedef struct MS1100_Instance MS1100_Instance_t;

struct MS1100_Instance {
    void *iicIf;
    uint8_t addr;
    union {
        uint8_t d8[3];
        struct {
            uint8_t RESERVED1;
            uint8_t RESERVED2;
            
            MS1100_Pga_t pga: 2;
            MS1100_Dr_t dr: 2;
            bool onceMode: 1;
            uint8_t RESERVED3: 2;
            uint8_t stDrdy: 1;
        };
    } config;
};

/*@}*/

/**
 * @addtogroup ExportFunc
 * @note none
 */

/*@{*/

extern int32_t ms1100_init(MS1100_Instance_t *ins, void *iicIf, uint8_t addr);
extern int32_t ms1100_finalize(MS1100_Instance_t *ins);

extern void ms1100_setPga(MS1100_Instance_t *ins, MS1100_Pga_t pga);
extern void ms1100_setDr(MS1100_Instance_t *ins, MS1100_Dr_t dr);
extern void ms1100_setOnceMode(MS1100_Instance_t *ins, bool s);
extern int32_t ms1100_updateConfig(MS1100_Instance_t *ins);

extern uint8_t ms1100_isDataReady(MS1100_Instance_t *ins);
extern int32_t ms1100_read(MS1100_Instance_t *ins);

extern int16_t ms1100_getAd(MS1100_Instance_t *ins);
extern float ms1100_getVolt(MS1100_Instance_t *ins, int16_t ad);

/*@}*/

#endif //RM_MS1100_H
