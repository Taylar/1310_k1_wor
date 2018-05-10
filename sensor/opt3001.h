//***********************************************************************************
// Copyright 2017, Zksiot Development Ltd.
// Created by MIN.ZENG, 2017.10.24
// MCU: MSP430F5529
// OS: TI-RTOS
// Project:
// File name: OPT3001.h
// Description: OPT3001 process routine head.
//***********************************************************************************

#ifndef __ZKSIOT_OPT3001_H__
#define __ZKSIOT_OPT3001_H__

#ifdef SUPPORT_OPT3001

#define Board_OPT3001_ADDR          (0x88 >> 1)

#define SET_CONFIGURATION_VALUE     0xc410
#define INTER_DEVICE_DI             0x3001
// sensor command
typedef enum{
    CMD_RESULT_REG                 = 0x00,
    CMD_SET_CONFIGURATION_REG      = 0x01,// Configuration Register 0x01
    CMD_LOW_LIMIT_REG              = 0x02,
    CMD_HIG_LIMIT_REG              = 0x03,
    CMD_DEVICE_ID_REG              = 0x7F,// command read device id
    CMD_MANUFACTURER_ID_REG        = 0x7E, // command  read manufactured_id
} OPT3001_CMD;
#define have_hig_limit_event 0x40
#define have_low_limit_event 0x20
typedef struct {
    uint16_t  OPT_MANUFACTURER_ID;
    uint16_t  OPT_DEVICE_ID;
}opt3001_ID;

typedef enum{
    CMD_CTRL_LUX_RANG_1 = 0x0000,//0.01-40.95
    CMD_CTRL_LUX_RANG_2 = 0x1000,//0.02-81.90
    CMD_CTRL_LUX_RANG_3 = 0x2000,//0.04-163.80
    CMD_CTRL_LUX_RANG_4 = 0x3000,//0.08-327.60
    CMD_CTRL_LUX_RANG_5 = 0x4000,//0.16-655.20
    CMD_CTRL_CT_R_W_800 = 0x0800,//1-800ms 0-100ns
    CMD_CTRL_FULL_SCANMODE = 0xC000,//auto scanmode
    CMD_CTRL_RESET = 0xc418,
}config_conctrl_reg_value;

typedef enum{
    RANG_0= 0,
    RANG_1 ,
    RANG_2 ,
    RANG_3,
    RANG_4 ,
    RANG_5 ,
    RANG_6 ,
    RANG_7 ,
    RANG_8 ,
    RANG_9 ,
    RANG_A ,
    RANG_B ,
}value;
#endif

#endif
