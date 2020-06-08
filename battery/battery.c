//***********************************************************************************
// Copyright 2017, Zksiot Development Ltd.
// Created by Linjie, 2017.08.08
// MCU:	MSP430F5529
// OS: TI-RTOS
// Project:
// File name: battery.c
// Description: battery process routine.
//***********************************************************************************
#include "../general.h"

#ifdef SUPPORT_BATTERY



#define     BATTERY_DETECT_TIME         8

#define     INVALID_BAT     0XFFFFFFFF

uint16_t bBatVoltage;


ADC_Handle   batteryHandle;
//***********************************************************************************
//
// Battery init.
//
//***********************************************************************************
void Battery_init(void)
{
    ADC_Params   params;

    ADC_Params_init(&params);
    batteryHandle = ADC_open(ZKS_BATTERY_ADC, &params);
}

//***********************************************************************************
//
// Battery voltage measure.
//
//***********************************************************************************
void Battery_voltage_measure(void)
{
    uint16_t temp;
    uint32_t value, max, min;
    uint32_t  batSum;
    uint8_t i;


    ADC_convert(batteryHandle, &temp);
    max    = ADC_convertToMicroVolts(batteryHandle, temp);
    min    = max;
    batSum = max;

    for(i = 0; i < (BATTERY_DETECT_TIME - 1); i++)
    {
        ADC_convert(batteryHandle, &temp);
        value    = ADC_convertToMicroVolts(batteryHandle, temp); // uv
        if(value > max)
            max = value;

        if(value < min)
            min = value;

        batSum += value;
    }

    batSum = (batSum - max - min) / (BATTERY_DETECT_TIME - 2);

        // Input (V) * 2^12 / VDDS (V)
    bBatVoltage = (uint16_t)((batSum * 2)/1000);

#ifdef SUPPORT_SOFT_CHARGE_DECT
    if(minBat == INVALID_BAT)
        minBat = bBatVoltage;
    if(minBat == INVALID_BAT)
        maxBat = bBatVoltage;

    if(chargeState == 0){
        if((bBatVoltage > BAT_VOLTAGE_L3) || (bBatVoltage > (minBat+200))){
            chargeState = 1;
            minBat = bBatVoltage;
            maxBat = bBatVoltage;
        }
    }else{
        if((bBatVoltage < BAT_VOLTAGE_L1) || (maxBat > (bBatVoltage+200))){
            chargeState = 0;
            minBat = bBatVoltage;
            maxBat = bBatVoltage;
        }
    }
    
    if(minBat > bBatVoltage)
        minBat = bBatVoltage;

    if(maxBat < bBatVoltage)
        maxBat = bBatVoltage;
#endif //SUPPORT_SOFT_CHARGE_DECT
}

//***********************************************************************************
//
// Battery get voltage value.
//
//***********************************************************************************
uint16_t Battery_get_voltage(void)
{

    if(bBatVoltage > BAT_VOLTAGE_FULL)
       bBatVoltage = BAT_VOLTAGE_FULL;
	
    return bBatVoltage;
}

static uint8_t  batCount = 0;
//***********************************************************************************
//
// measure the batter and process
//
//***********************************************************************************
void Battery_porcess(void)
{ 
    Battery_voltage_measure();

    if(Battery_get_voltage() <= g_rSysConfigInfo.batLowVol)
    {
        batCount++;
        if (batCount > 5 && (deviceMode != DEVICES_OFF_MODE))
        {
            batCount = 6;
            // S1Sleep();

#ifdef      SUPPORT_DISP_SCREEN
            //Disp_poweroff();
#endif      //SUPPORT_DISP_SCREEN

#ifdef BOARD_S6_6
            // S6Sleep();
#endif
        }
    }
    else
    {
        batCount = 0;
    }

}
#endif  /* SUPPORT_BATTERY */

