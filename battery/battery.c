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
#ifndef BOARD_S3
    ADC_Params   params;

    ADC_Params_init(&params);
    batteryHandle = ADC_open(ZKS_BATTERY_ADC, &params);
#endif

#ifdef SUPPORT_SOFT_CHARGE_DECT
    minBat = INVALID_BAT;
    maxBat = INVALID_BAT;
    chargeState = 0;
#endif //SUPPORT_SOFT_CHARGE_DECT
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
    //static uint8_t init = 0, batLowCnt = 0;
    static uint16_t batVoltageBuf;
    uint16_t    voltage;
    voltage = AONBatMonBatteryVoltageGet();
    voltage = ((voltage&0xff00)>>8)*1000 +1000*(voltage&0xff)/256;
    batVoltageBuf = voltage;
    
    // if(init == 0)
    // {
    //     batVoltageBuf = voltage;
    //     init          = 1;
    // }

    // if(batVoltageBuf > voltage)
    // {
    //     if(batLowCnt > 2)
    //     {
    //         batVoltageBuf -= 5;
    //     }
    //     else
    //     {
    //         batLowCnt++;
    //     }
    // }
    // else
    // {
    //     batLowCnt = 0;
    // }
    
    if(batVoltageBuf > BAT_VOLTAGE_FULL)
       batVoltageBuf = BAT_VOLTAGE_FULL;
    return batVoltageBuf;
#else
    if(bBatVoltage > BAT_VOLTAGE_FULL)
       bBatVoltage = BAT_VOLTAGE_FULL;
	
    return bBatVoltage;
#endif
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
#ifdef  BOARD_S3
                S1Sleep();
#else

#ifdef BOARD_S6_6
#ifdef      SUPPORT_DISP_SCREEN
            Disp_poweroff();
#endif      //SUPPORT_DISP_SCREEN

                S6Sleep();
#endif//BOARD_S6_6

#ifdef BOARD_B2S
        S2Sleep();
#endif // BOARD_B2S

#endif//BOARD_S3
            }
        }
        else
        {
            batCount = 0;
        }

}
#endif  /* SUPPORT_BATTERY */

