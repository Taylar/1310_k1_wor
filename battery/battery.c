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



static uint16_t bBatVoltage;


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
    uint16_t value, max, min;
    uint32_t  batSum;
    uint8_t i;


    ADC_convert(batteryHandle, &max);
    min     = max;
    batSum  = max;

    for(i = 0; i < (BATTERY_DETECT_TIME - 1); i++)
    {
        ADC_convert(batteryHandle, &value);
        if(value > max)
            max = value;

        if(value < min)
            min = value;

        batSum += value;
    }

    batSum = (batSum - max - min) / (BATTERY_DETECT_TIME - 2);

        // Input (V) * 2^12 / VDDS (V)
    bBatVoltage = (uint16_t)((batSum * BATTERY_REF_VOL * 2) >> 12);
}

//***********************************************************************************
//
// Battery get voltage value.
//
//***********************************************************************************
uint16_t Battery_get_voltage(void)
{
    return bBatVoltage;
}


#endif  /* SUPPORT_BATTERY */

