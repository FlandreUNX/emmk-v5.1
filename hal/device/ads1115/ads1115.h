//
// Created by Fland on 24-10-23.
//

#ifndef ADS1115_H
#define ADS1115_H

#include <stdint.h>

/**
 * @addtogroup Typedef
 * @note none
 */

/*@{*/

typedef enum {
    ADS1115_MUX_P0_N1 = 0b000,
    ADS1115_MUX_P0_N3 = 0b001,
    ADS1115_MUX_P1_N3 = 0b010,
    ADS1115_MUX_P2_N3 = 0b011,
    ADS1115_MUX_P0_NG = 0b100,
    ADS1115_MUX_P1_NG = 0b101,
    ADS1115_MUX_P2_NG = 0b110,
    ADS1115_MUX_P3_NG = 0b111,
} ads1115_Mux_t;
#define ADS1115_MUX_OFFSET 12

typedef enum {
    ADS1115_PGA_6144 = 0b000,
    ADS1115_PGA_4096 = 0b001,
    ADS1115_PGA_2048 = 0b010,
    ADS1115_PGA_1024 = 0b011,
    ADS1115_PGA_512 = 0b100,
    ADS1115_PGA_256 = 0b101,
} ads1115_Pga_t;
#define ADS1115_PGA_OFFSET 9

typedef enum {
    ADS1115_MODE_CONTINUOUS = 0,
    ADS1115_MODE_SHOT = 1,
} ads1115_Mode_t;
#define ADS1115_MODE_OFFSET 8

typedef enum {
    ADS1115_DR_8_SPS = 0b000,
    ADS1115_DR_16_SPS = 0b001,
    ADS1115_DR_32_SPS = 0b010,
    ADS1115_DR_64_SPS = 0b011,
    ADS1115_DR_128_SPS = 0b100,
    ADS1115_DR_250_SPS = 0b101,
    ADS1115_DR_475_SPS = 0b110,
    ADS1115_DR_860_SPS = 0b111,
} ads1115_Dr_t;
#define ADS1115_DR_OFFSET 5

typedef enum {
    ADS1115_ADDRESS_GND = 0b01001000,
    ADS1115_ADDRESS_VDD = 0b01001001,
    ADS1115_ADDRESS_SDA = 0b01001010,
    ADS1115_ADDRESS_SCL = 0b01001011,
} ads1115_Address_t;

typedef union {
    uint16_t u16;
    struct {
        uint16_t compQue: 2;
        uint16_t compLat: 1;
        uint16_t compPol: 1;
        uint16_t compMode: 1;
        uint16_t dr: 3;

        uint16_t mode: 1;
        uint16_t pga: 3;
        uint16_t mux: 3;
        uint16_t os: 1;
    };
} ads1115_Config_t;

typedef struct {
    void *i2cIf;
    ads1115_Address_t address;
    struct {
        uint16_t data;
        ads1115_Config_t config;
    } reg;
} ads1115_Instance_t;

/*@}*/

/**
 * @addtogroup Export funcs
 * @note none
 */

/*@{*/

extern void ads1115_initBus(ads1115_Instance_t *ins, void *kdi2c);
extern int32_t ads1115_initSoft(ads1115_Instance_t *ins);
extern void ads1115_finalizeBus(ads1115_Instance_t *ins);
extern void ads1115_finalizeSoft(ads1115_Instance_t *ins);

extern int32_t ads1115_updateConfig(ads1115_Instance_t *ins);
extern int32_t ads1115_reloadConfig(ads1115_Instance_t *ins);
extern void ads1115_getConfig(ads1115_Instance_t *ins, ads1115_Config_t *config);
extern void ads1115_setConfig(ads1115_Instance_t *ins, ads1115_Config_t *config);

extern int32_t ads1115_convertOnceShot(ads1115_Instance_t *ins);

extern int32_t ads1115_reloadData(ads1115_Instance_t *ins);

extern int16_t ads1115_getRaw(ads1115_Instance_t *ins);
extern float ads1115_getVolt(ads1115_Instance_t *ins);

/*@}*/

#endif // ADS1115_H
