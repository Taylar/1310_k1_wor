//***********************************************************************************
// Copyright 2017, Zksiot Development Ltd.
// Created by Linjie, 2017.08.08
// MCU:	MSP430F5529
// OS: TI-RTOS
// Project:
// File name: i2c_drv.c
// Description: i2c process routine.
//***********************************************************************************
#include "../general.h"



#define CC1310_LAUNCHXL_I2C0_SCL0             IOID_19
#define CC1310_LAUNCHXL_I2C0_SDA0             IOID_18

I2C_Handle i2cHandle;


#include <ti/drivers/I2C.h>
#include <ti/drivers/i2c/I2CCC26XX.h>
#include <ti/drivers/power/PowerCC26XX.h>

I2CCC26XX_Object i2cCC26xxObjects[CC1310_LAUNCHXL_I2CCOUNT];

const I2CCC26XX_HWAttrsV1 i2cCC26xxHWAttrs[CC1310_LAUNCHXL_I2CCOUNT] = {
    {
        .baseAddr    = I2C0_BASE,
        .powerMngrId = PowerCC26XX_PERIPH_I2C0,
        .intNum      = INT_I2C_IRQ,
        .intPriority = ~0,
        .swiPriority = 0,
        .sdaPin      = CC1310_LAUNCHXL_I2C0_SDA0,
        .sclPin      = CC1310_LAUNCHXL_I2C0_SCL0,
    }
};

const I2C_Config I2C_config[CC1310_LAUNCHXL_I2CCOUNT] = {
    {
        .fxnTablePtr = &I2CCC26XX_fxnTable,
        .object      = &i2cCC26xxObjects[CC1310_LAUNCHXL_I2C0],
        .hwAttrs     = &i2cCC26xxHWAttrs[CC1310_LAUNCHXL_I2C0]
    }
};

const uint_least8_t I2C_count = CC1310_LAUNCHXL_I2CCOUNT;

//***********************************************************************************
//
// I2C master init.
//
//***********************************************************************************
ErrorStatus I2c_init(void)
{
    I2C_Params i2cParams;

    I2C_init();

    I2C_Params_init(&i2cParams);
    i2cParams.bitRate = I2C_400kHz;
	i2cParams.transferMode = I2C_MODE_BLOCKING;
    i2cHandle = I2C_open(CC1310_LAUNCHXL_I2C0, &i2cParams);
    if (i2cHandle == NULL) {
        System_printf("I2C was not opened\n");
        return ES_ERROR;
    }

    return ES_SUCCESS;
}

//***********************************************************************************
//
// I2C master write n bytes.
//
//***********************************************************************************
ErrorStatus I2c_write(uint8_t devAddr, uint8_t *pBuff, uint8_t n)
{
    I2C_Transaction i2cTransaction;

    i2cTransaction.slaveAddress = devAddr;
    i2cTransaction.writeBuf = pBuff;
    i2cTransaction.writeCount = n;
    i2cTransaction.readBuf = NULL;
    i2cTransaction.readCount = 0;
    if (!I2C_transfer(i2cHandle, &i2cTransaction)) {
        System_printf("Bad I2C write transfer!\n");
        return ES_ERROR;
    }

    return ES_SUCCESS;
}

//***********************************************************************************
//
// I2C master read n bytes.
//
//***********************************************************************************
ErrorStatus I2c_read(uint8_t devAddr, uint8_t *pBuff, uint8_t n)
{
    I2C_Transaction i2cTransaction;

    i2cTransaction.slaveAddress = devAddr;
    i2cTransaction.writeBuf = NULL;
    i2cTransaction.writeCount = 0;
    i2cTransaction.readBuf = pBuff;
    i2cTransaction.readCount = n;
    if (!I2C_transfer(i2cHandle, &i2cTransaction)) {
        System_printf("Bad I2C read transfer!\n");
        return ES_ERROR;
    }

    return ES_SUCCESS;
}

//***********************************************************************************
//
// I2C master read n byte from a register.
//
//***********************************************************************************
ErrorStatus I2c_regRead(uint8_t devAddr, uint8_t regAddr, uint8_t *pBuff, uint8_t n)
{
    I2C_Transaction i2cTransaction;

    i2cTransaction.slaveAddress = devAddr;
    i2cTransaction.writeBuf = &regAddr;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = pBuff;
    i2cTransaction.readCount = n;
    if (!I2C_transfer(i2cHandle, &i2cTransaction)) {
        System_printf("Bad I2C reg read transfer!\n");
        return ES_ERROR;
    }

    return ES_SUCCESS;
}


