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

#define CHARGE_DETECT_PIN             IOID_29
#define CHARGE_STATUS_PIN             IOID_7

#define GET_PLUG_IN_STATUS()          PIN_getInputValue(CHARGE_DETECT_PIN)
#define GET_CHARGE_STATUS()           PIN_getInputValue(CHARGE_STATUS_PIN)


typedef enum {
   NO_CHARGE = 0,
   CHARGE_FULL,
   CHARGEING
}ChargeStateType;

extern void Charge_detect_init(void);
extern ChargeStateType Get_Charge_plug(void);

#endif  /* SUPPORT_BATTERY */

#endif	/* __ZKSIOT_BATTERY_H__ */

