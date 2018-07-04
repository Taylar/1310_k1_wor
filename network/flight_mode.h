//***********************************************************************************
// Copyright 2017-2018, Zksiot Development Ltd.
// Created by yuanGuo, 2018.05.21
// MCU:	MSP430F5529
// OS: TI-RTOS
// Project:
// File name: flight_mode.h
// Description: function for flight mode.
//***********************************************************************************

#ifndef __ZKSIOT_FLIGHT_MODE_H__
#define __ZKSIOT_FLIGHT_MODE_H__
#include "../general.h"

#ifdef SUPPORT_FLIGHT_MODE

#define FLIGHT_ACK_BUFFER_SIZE 5 //

typedef struct _flight_mode{
    char isFlightMode; // 1 yes ; 0 no
    uint16_t minFlightTimesSetting; // how many minus setting for flight mode
}Flight_mode_t;

extern void Flight_mode_cancel(void);
extern void Flight_mode_local_setting(void);
extern void Flight_mode_entry(uint16_t minutes);
extern void Flight_mode_setting(uint16_t minutes);
extern uint16_t Flight_mode_get_timesetting(void);

extern char Flight_mode_isFlightMode(void);
extern char Nwk_is_Active(void);
extern void Nwk_send_upload_event(void);


#endif // SUPPORT_FLIGHT_MODE

#endif
