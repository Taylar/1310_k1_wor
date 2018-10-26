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
#ifndef BOARD_S3
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
#ifndef BOARD_S3
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
#ifdef  BOARD_S3
    static uint8_t init = 0, batLowCnt = 0;
    static uint16_t batVoltageBuf;
    uint16_t    voltage;
    voltage = AONBatMonBatteryVoltageGet();
    voltage = ((voltage&0xff00)>>8)*1000 +1000*(voltage&0xff)/256;
    
    if(init == 0)
    {
        batVoltageBuf = voltage;
        init          = 1;
    }

    if(batVoltageBuf > voltage)
    {
        if(batLowCnt > 2)
        {
            batVoltageBuf -= 5;
        }
        else
        {
            batLowCnt++;
        }
    }
    else
    {
        batLowCnt = 0;
    }
    return batVoltageBuf;
#else
    return bBatVoltage;
#endif
}

#ifdef S_G//网关
static uint32_t batMeasureCnt;
#endif //S_G//网关
static uint8_t  batCount = 0;
//***********************************************************************************
//
// measure the batter and process
//
//***********************************************************************************
void Battery_porcess(void)
{
#ifdef S_G//网关
    batMeasureCnt++;
    if(batMeasureCnt < g_rSysConfigInfo.uploadPeriod)
    {
        return;
    }
    batMeasureCnt = 0;
#endif //S_G//网关
    
    Battery_voltage_measure();

    if(Battery_get_voltage() <= g_rSysConfigInfo.batLowVol)
        {
            batCount++;
            if (batCount > 5)
            {

#ifdef  BOARD_S3
                S1Sleep();
#else

#ifdef BOARD_S6_6
#ifdef      SUPPORT_DISP_SCREEN
            Disp_poweroff();
#endif      //SUPPORT_DISP_SCREEN

                S6Sleep();
#endif//BOARD_S6_6
#endif//BOARD_S3
            }
        }
        else
        {
            batCount = 0;
        }

}
#endif  /* SUPPORT_BATTERY */

