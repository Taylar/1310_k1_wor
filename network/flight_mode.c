//***********************************************************************************
// Copyright 2017-2018, Zksiot Development Ltd.
// Created by yuanGuo, 2018.05.18
// MCU:	MSP430F5529
// OS: TI-RTOS
// Project:
// File name: flight_mode.c
// Description: function for flight mode.
//***********************************************************************************
#include "../general.h"

#ifdef SUPPORT_FLIGHT_MODE

/*
 * Principle
 * When Get the command to entry flight mode,
 * Judge the command , if PARAM EQ ZERO(0), do not Exit Auto Until Manually cancel
 * if PARAM > 0, Make a Clock , after some minutes to Exit Flight mode auto
 *
 * */

#define FLIGHT_MODE_NO_AUTO_CLOSE 0

static Flight_mode_t Flight_mode;

static Clock_Struct sysCancelFlightStruct;
static Clock_Handle sysCancelFlightHandle = NULL;
static char clockflag = 0;

void Flight_mode_ack_data_store(NwkMsgPacket_t *rNwkMsgPacket)
{
    int i;
    for(i=0; i < FLIGHT_ACK_BUFFER_SIZE; i++){
        Flight_mode.Ack_tempBuff[i] =  rNwkMsgPacket->buff[i];
    }
}

void Flight_mode_ack_data_readback(NwkMsgPacket_t *rNwkMsgPacket)
{
    int i;
    for(i=0; i < FLIGHT_ACK_BUFFER_SIZE; i++){
         rNwkMsgPacket->buff[i] = Flight_mode.Ack_tempBuff[i];
    }
}

void Flight_mode_cancel(void)
{
    Flight_mode.isFlightMode = 0;
    Flight_mode.minFlightTimesSetting = 0;
#ifdef SUPPORT_DEVICED_STATE_UPLOAD
    Sys_event_post(SYS_EVT_SYS_RUN_SAVE);
#endif
    Nwk_poweron();

    if(sysCancelFlightHandle != NULL)
        Clock_stop(sysCancelFlightHandle);
}

char Flight_mode_isFlightMode(void)
{
    return Flight_mode.isFlightMode;
}

static void sysTimer_CancelFlightFxn(UArg arg0)
{
    if(clockflag == 0){
        clockflag = 1;
    }else{
        clockflag = 0;
        Flight_mode_cancel();
    }
}

static void sysFlightMode_CreateClockFxn(uint16_t minutes)
{
    /* Construct a Clock Instance to Auto Cancel Flight Mode */
    /* Construct a minutes min periodic Clock Instance to battery alarm */
    if(NULL == sysCancelFlightHandle){
        Clock_Params clkParams;
        Clock_Params_init(&clkParams);
        clkParams.period = minutes * 60 * CLOCK_UNIT_S;
        clkParams.startFlag = FALSE;
        //
        Clock_construct(&sysCancelFlightStruct, (Clock_FuncPtr)sysTimer_CancelFlightFxn,3 * CLOCK_UNIT_S, &clkParams);
        /* Obtain clock instance handle */
        sysCancelFlightHandle = Clock_handle(&sysCancelFlightStruct);
    }

    if(sysCancelFlightHandle != NULL){// Create OK
        Clock_setPeriod(sysCancelFlightHandle, minutes * 60 * CLOCK_UNIT_S);
        Clock_setTimeout(sysCancelFlightHandle,  3 * CLOCK_UNIT_S);
        Clock_start(sysCancelFlightHandle);
    }
}

void Flight_mode_entry(uint16_t minutes)
{
    Flight_mode.isFlightMode = 1;

    // make clock
    if(minutes !=  FLIGHT_MODE_NO_AUTO_CLOSE){
        clockflag = 0;
        sysFlightMode_CreateClockFxn(minutes);
    }
}

void Flight_mode_setting(uint16_t minutes)
{
    Flight_mode.isFlightMode = 1;
    Flight_mode.minFlightTimesSetting = minutes;
}
uint16_t Flight_mode_get_timesetting(void)
{
    return Flight_mode.minFlightTimesSetting;
}

void Flight_mode_local_setting(void)
{
    Flight_mode_entry(FLIGHT_MODE_NO_AUTO_CLOSE);
    Nwk_send_upload_event();
}

#endif // SUPPORT_FLIGHT_MODE
