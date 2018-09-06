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



uint16_t bBatVoltage;


ADC_Handle   batteryHandle;
//***********************************************************************************
//
// Battery init.
//
//***********************************************************************************
void Battery_init(void)
{
#ifndef BOARD_S3_2
    ADC_Params   params;

    ADC_Params_init(&params);
    batteryHandle = ADC_open(ZKS_BATTERY_ADC, &params);
#endif
}

//***********************************************************************************
//
// Battery voltage measure.
//
//***********************************************************************************
void Battery_voltage_measure(void)
{
#ifndef BOARD_S3_2
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

#endif
}

//***********************************************************************************
//
// Battery get voltage value.
//
//***********************************************************************************
uint16_t Battery_get_voltage(void)
{
#ifdef  BOARD_S3_2
    uint32_t voltageTemp[10];
    uint32_t count = 0;
    uint32_t max, min;
    uint32_t voltage = 0;
    uint8_t i = 0;

    for (; i < 10; i++) {
        voltageTemp[i] = AONBatMonBatteryVoltageGet();
        voltage = ((voltageTemp[i]&0xff00)>>8)*1000 +1000*(voltageTemp[i]&0xff)/256;
        if (0 == i) {
            max = voltage;
            min = voltage;
        }

        if (max < voltage) {
            max = voltage;
        }

        if (min > voltage) {
            min = voltage;
        }

        count += voltage;
    }
    return (uint16_t)((count - max - min) / 8);
#else
    return bBatVoltage;
#endif
}


#endif  /* SUPPORT_BATTERY */

