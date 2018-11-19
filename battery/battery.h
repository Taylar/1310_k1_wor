//***********************************************************************************
// Copyright 2017, Zksiot Development Ltd.
// Created by Linjie, 2017.08.08
// MCU:	MSP430F5529
// OS: TI-RTOS
// Project:
// File name: battery.h
// Description: battery process routine head.
//***********************************************************************************

#ifndef __ZKSIOT_BATTERY_H__
#define __ZKSIOT_BATTERY_H__

#ifdef SUPPORT_BATTERY

//Battery voltage (mV)
#ifdef  BOARD_S3

#define BAT_VOLTAGE_LOW             2150
#define BAT_VOLTAGE_L1              2500
#define BAT_VOLTAGE_L2              2800
#define BAT_VOLTAGE_L3              3050
#define BAT_VOLTAGE_FULL            3100

#else

#define BAT_VOLTAGE_LOW             3600
#define BAT_VOLTAGE_L1              3750
#define BAT_VOLTAGE_L2              3900
#define BAT_VOLTAGE_L3              4050
#define BAT_VOLTAGE_FULL            4200

#endif

extern void Battery_init(void);
extern void Battery_voltage_measure(void);
extern uint16_t Battery_get_voltage(void);
extern void Battery_porcess(void);

#endif  /* SUPPORT_BATTERY */

#endif	/* __ZKSIOT_BATTERY_H__ */

