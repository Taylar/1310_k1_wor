//***********************************************************************************
// Copyright 2017, Zksiot Development Ltd.
// Created by Linjie, 2017.08.08
// MCU:	MSP430F5529
// OS: TI-RTOS
// Project:
// File name: battery.h
// Description: battery process routine head.
//***********************************************************************************

#ifndef __ZKSIOT_CHARGE_DETECT_H__
#define __ZKSIOT_CHARGE_DETECT_H__

#ifdef SUPPORT_CHARGE_DECT

#ifdef  BOARD_S6_6

#define CHARGE_DETECT_PIN             IOID_11

#endif // S6_6


extern void Charge_detect_init(void);
extern ChargeStateType Get_Charge_plug(void);

#endif  /* SUPPORT_BATTERY */

#endif	/* __ZKSIOT_BATTERY_H__ */

