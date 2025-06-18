//
// Created by Fland on 24-10-23.
//

#include "./p4io08.h"

#include "emmk-config.h"
#include "emmk-driver.h"

/**
 * @addtogroup DebugSupport
 * @note none
 */

/*@{*/

#undef TAG
static const char *tag = "p4io08";
#define TAG     tag

/*@}*/

/**
 * @addtogroup Private funcs
 * @note none
 */

/*@{*/

static inline int32_t readReg(p4io08_Instance_t *ins, uint8_t regAddr, uint8_t *data, uint8_t len) {
    return kdi2c_regRead(ins->i2cIf,
                         ins->address,
                         regAddr, 1,
                         data, len);
}


static inline int32_t writeReg(p4io08_Instance_t *ins, uint8_t regAddr, uint8_t *data, uint8_t len) {
    uint8_t d[2] = {
      regAddr, data[0]
    };
    return kdi2c_write(ins->i2cIf,
                          ins->address,
                          d, 2);
}

/*@}*/

/**
 * @addtogroup Export funcs
 * @note none
 */

/*@{*/

void p4io08_initBus(p4io08_Instance_t *ins, void *kdi2c) {
    ASSERT(ins != NULL);
    ASSERT(kdi2c != NULL);

    if (ins->i2cIf != NULL) {
        return;
    }
    ins->i2cIf = kdi2c;
    kdi2c_init(ins->i2cIf);
    kdi2c_powerUp(ins->i2cIf);
}

int32_t p4io08_initSoft(p4io08_Instance_t *ins) {
    ASSERT(ins != NULL);

    int32_t rc;
    uint8_t data[8] = {0};

    if (readReg(ins, 0x01, data, 8) != 0) {
        rc = -1;
        goto l_errorExit;
    }
    if ((data[0] & 0b11111100) != 0b10100000) {
        rc = -9;
        goto l_errorExit;
    }

    data[0] = ins->reg.out.u8;
    if (writeReg(ins, 0x05, data, 1) != 0) {
        rc = -2;
        goto l_errorExit;
    }
    data[0] = ins->reg.od.u8;
    if (writeReg(ins, 0x07, data, 1) != 0) {
        rc = -3;
        goto l_errorExit;
    }
    data[0] = ins->reg.inDefault.u8;
    if (writeReg(ins, 0x09, data, 1) != 0) {
        rc = -4;
        goto l_errorExit;
    }
    data[0] = ins->reg.pullUpDownEnable.u8;
    if (writeReg(ins, 0x0B, data, 1) != 0) {
        rc = -5;
        goto l_errorExit;
    }
    data[0] = ins->reg.pullUpDownSelect.u8;
    if (writeReg(ins, 0x0D, data, 1) != 0) {
        rc = -6;
        goto l_errorExit;
    }
    data[0] = ins->reg.dir.u8;
    if (writeReg(ins, 0x03, data, 1) != 0) {
        rc = -7;
        goto l_errorExit;
    }

    if (readReg(ins, 0x0F, data, 1) != 0) {
        rc = -8;
        goto l_errorExit;
    }
    ins->reg.in.u8 = data[0];

    LOG_I("softInit, Success, addr=%02X", ins->address);
    return 0;

    l_errorExit:
    LOG_E("softInit, Failed, addr=%02X, rc=%ld", ins->address, rc);
    return rc;
}

void p4io08_finalizeBus(p4io08_Instance_t *ins) {
    ASSERT(ins != NULL);

    if (ins->i2cIf == NULL) {
        return;
    }
    kdi2c_powerDown(((kdI2C_t *) ins->i2cIf));
    kdi2c_finalize(((kdI2C_t *) ins->i2cIf));
    ins->i2cIf = NULL;
}

void p4io08_finalizeSoft(p4io08_Instance_t *ins) {
}

int32_t p4io08_readAllReg(p4io08_Instance_t *ins) {
    ASSERT(ins != NULL);
    int32_t rc = 0;
    if (readReg(ins, 0x03, &ins->reg.dir.u8, 1) != 0) {
        rc = -1;
        goto l_errorExit;
    }
    if (readReg(ins, 0x05, &ins->reg.out.u8, 1) != 0) {
        rc = -2;
        goto l_errorExit;
    }
    if (readReg(ins, 0x07, &ins->reg.od.u8, 1) != 0) {
        rc = -3;
        goto l_errorExit;
    }
    if (readReg(ins, 0x09, &ins->reg.inDefault.u8, 1) != 0) {
        rc = -4;
        goto l_errorExit;
    }
    if (readReg(ins, 0x0B, &ins->reg.pullUpDownEnable.u8, 1) != 0) {
        rc = -5;
        goto l_errorExit;
    }
    if (readReg(ins, 0x0D, &ins->reg.pullUpDownSelect.u8, 1) != 0) {
        rc = -6;
        goto l_errorExit;
    }
    if (readReg(ins, 0x0F, &ins->reg.in.u8, 1) != 0) {
        rc = -7;
        goto l_errorExit;
    }

    LOG_I("readAllReg, addr=%02X, %02X,%02X,%02X,%02X,%02X,%02X,%02X",
          ins->address,
          ins->reg.dir.u8,
          ins->reg.out.u8,
          ins->reg.od.u8,
          ins->reg.inDefault.u8,
          ins->reg.pullUpDownEnable.u8,
          ins->reg.pullUpDownSelect.u8,
          ins->reg.in.u8
    );
    return 0;

    l_errorExit:
    LOG_E("readAllReg, Failed, addr=%02X, rc=%ld", ins->address, rc);
    return rc;
}

int32_t p4io08_softReset(p4io08_Instance_t *ins) {
    ASSERT(ins != NULL);

    uint8_t data = 0x01;

    if (writeReg(ins, 0x01, &data, 1) != 0) {
        LOG_W("softReset, Failed, addr=%02X", ins->address);
        return -1;
    }
    LOG_I("softReset, Success, addr=%02X", ins->address);
    return 0;
}

int32_t p4io08_setPinDir(p4io08_Instance_t *ins, p4io08_Pin_t pin, p4io08_Dir_t dir) {
    ASSERT(ins != NULL);

    uint8_t data = ins->reg.dir.u8;

    if (dir) {
        data |= pin;
    } else {
        data &= ~pin;
    }

    if (writeReg(ins, 0x03, &data, 1) != 0) {
        LOG_W("setPinDir, Failed, addr=%02X", ins->address);
        return -1;
    }
    ins->reg.dir.u8 = data;

    return 0;
}

int32_t p4io08_updateInput(p4io08_Instance_t *ins) {
    ASSERT(ins->i2cIf != NULL);

    uint8_t data = 0;

    if (readReg(ins, 0x0F, &data, 1) != 0) {
        LOG_W("updateInput, Failed, addr=%02X", ins->address);
        return -1;
    }
    ins->reg.in.u8 = data;

    return 0;
}

int32_t p4io08_updateOutput(p4io08_Instance_t *ins) {
    ASSERT(ins->i2cIf != NULL);

    if (!ins->outputUpdateRequest) {
        return 0;
    }

    uint8_t data = ins->reg.out.u8;

    if (writeReg(ins, 0x05, &data, 1) != 0) {
        LOG_W("updateOutput, Failed, addr=%02X", ins->address);
        return -1;
    }

    ins->outputUpdateRequest = false;

    return 0;
}

uint8_t p4io08_readPin(p4io08_Instance_t *ins, p4io08_Pin_t pin) {
    ASSERT(ins->i2cIf != NULL);

    return ins->reg.in.u8 & pin;
}


uint8_t p4io08_readPinPort(p4io08_Instance_t *ins) {
    ASSERT(ins->i2cIf != NULL);

    return ins->reg.in.u8;
}


void p4io08_setPin(p4io08_Instance_t *ins, p4io08_Pin_t pin, uint8_t vl, bool force) {
    ASSERT(ins->i2cIf != NULL);

    uint8_t current = ins->reg.out.u8;

    if (vl) {
        ins->reg.out.u8 |= pin;
    } else {
        ins->reg.out.u8 &= ~pin;
    }

    if (current != ins->reg.out.u8 || force) {
        ins->outputUpdateRequest = true;
    }
}


void p4io08_setPinPort(p4io08_Instance_t *ins, uint16_t pinPort, bool force) {
    ASSERT(ins->i2cIf != NULL);

    uint8_t current = ins->reg.out.u8;
    ins->reg.out.u8 = pinPort;

    if (current != ins->reg.out.u8 || force) {
        ins->outputUpdateRequest = true;
    }
}

/*@}*/

/**
 * @addtogroup DebugSupport
 * @note none
 */

/*@{*/

#undef TAG

/*@}*/

