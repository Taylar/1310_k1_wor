//***********************************************************************************
// Copyright 2017, Zksiot Development Ltd.
// Created by Linjie, 2017.08.08
// MCU:	MSP430F5529
// OS: TI-RTOS
// Project:
// File name: i2c_drv.h
// Description: i2c process routine head.
//***********************************************************************************

#ifndef __ZKSIOT_I2C_DRV_H__
#define __ZKSIOT_I2C_DRV_H__

/*!
 *  @def    CC1310_LAUNCHXL_I2CName
 *  @brief  Enum of I2C names
 */
typedef enum CC1310_LAUNCHXL_I2CName {
    CC1310_LAUNCHXL_I2C0 = 0,

    CC1310_LAUNCHXL_I2CCOUNT
} CC1310_LAUNCHXL_I2CName;


extern ErrorStatus I2c_init(void);
extern ErrorStatus I2c_write(uint8_t devAddr, uint8_t *pBuff, uint8_t n);
extern ErrorStatus I2c_read(uint8_t devAddr, uint8_t *pBuff, uint8_t n);
extern ErrorStatus I2c_regRead(uint8_t devAddr, uint8_t regAddr, uint8_t *pBuff, uint8_t n);

#endif	/* __ZKSIOT_I2C_DRV_H__ */

