//
// Created by Fland on 24-10-23.
//

#ifndef P4IO08_H
#define P4IO08_H

#include "stdint.h"
#include "stdbool.h"

/**
 * @addtogroup Typedef
 * @note none
 */

/*@{*/

typedef union {
    uint8_t u8;
    struct {
        uint8_t p00: 1;
        uint8_t p01: 1;
        uint8_t p02: 1;
        uint8_t p03: 1;
        uint8_t p04: 1;
        uint8_t p05: 1;
        uint8_t p06: 1;
        uint8_t p07: 1;
    };
} p4io08_BitData_t;

typedef enum {
    P4IO08_PIN_00 = 1 << 0,
    P4IO08_PIN_01 = 1 << 1,
    P4IO08_PIN_02 = 1 << 2,
    P4IO08_PIN_03 = 1 << 3,
    P4IO08_PIN_04 = 1 << 4,
    P4IO08_PIN_05 = 1 << 5,
    P4IO08_PIN_06 = 1 << 6,
    P4IO08_PIN_07 = 1 << 7,
    
    P4IO08_PIN_ALL = 0xFF
} p4io08_Pin_t;

typedef enum {
    P4IO08_DIR_INPUT = 0,
    P4IO08_DIR_OUTPUT = 1,
} p4io08_Dir_t;

typedef enum {
    P4IO08_ADDRESS_GND = 0x43,
    P4IO08_ADDRESS_VDD = 0x44,
} p4io08_Address_t;

typedef struct {
    void *i2cIf;
    p4io08_Address_t address;
    union {
        uint8_t u8[7];
        struct {
            p4io08_BitData_t dir;   // 03H
            p4io08_BitData_t out;   // 05H
            p4io08_BitData_t od;    // 07H
            p4io08_BitData_t inDefault;     // 09H
            p4io08_BitData_t pullUpDownEnable;  // 0BH
            p4io08_BitData_t pullUpDownSelect;  // 0DH
            p4io08_BitData_t in;    // 0FH
        };
    } reg;
    uint8_t outputUpdateRequest;
} p4io08_Instance_t;

/*@}*/

/**
 * @addtogroup Export funcs
 * @note none
 */

/*@{*/

extern void p4io08_initBus(p4io08_Instance_t *ins, void *kdi2c);
extern int32_t p4io08_initSoft(p4io08_Instance_t *ins);
extern void p4io08_finalizeBus(p4io08_Instance_t *ins);
extern void p4io08_finalizeSoft(p4io08_Instance_t *ins);

extern int32_t p4io08_softReset(p4io08_Instance_t *ins);

extern int32_t p4io08_readAllReg(p4io08_Instance_t *ins);

extern int32_t p4io08_setPinDir(p4io08_Instance_t *ins, p4io08_Pin_t pin, p4io08_Dir_t dir);

extern int32_t p4io08_updateInput(p4io08_Instance_t *ins);
extern int32_t p4io08_updateOutput(p4io08_Instance_t *ins);

extern uint8_t p4io08_readPin(p4io08_Instance_t *ins, p4io08_Pin_t pin);
extern uint8_t p4io08_readPinPort(p4io08_Instance_t *ins);

extern void p4io08_setPin(p4io08_Instance_t *ins, p4io08_Pin_t pin, uint8_t vl, bool force);
extern void p4io08_setPinPort(p4io08_Instance_t *ins, uint16_t pinPort, bool force);

/*@}*/

#endif //P4IO08_H
