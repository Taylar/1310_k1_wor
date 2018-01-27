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

// #define     BATTERY_REF_VDDS
#define     BATTERY_REF_FIX

#ifdef      BATTERY_REF_FIX
#define     BATTERY_REF_VOL             4300
#else
#define     BATTERY_REF_VOL             3326
#endif


#define     BATTERY_DETECT_TIME         8

#define     BATTERY_DETECT_PIN          IOID_30

ADCCC26XX_Object adcCC26xxObjects[1];



const ADCCC26XX_HWAttrs adcCC26xxHWAttrs[1] = {
    {
        .adcDIO              = BATTERY_DETECT_PIN,
        .adcCompBInput       = ADC_COMPB_IN_AUXIO0,
#ifdef      BATTERY_REF_FIX
        .refSource           = ADCCC26XX_FIXED_REFERENCE,
#else
        .refSource           = ADCCC26XX_VDDS_REFERENCE,
#endif
        .samplingDuration    = ADCCC26XX_SAMPLING_DURATION_2P7_US,
        .inputScalingEnabled = true,
        .triggerSource       = ADCCC26XX_TRIGGER_MANUAL,
        .returnAdjustedVal   = false
    }
};

const ADC_Config ADC_config[1] = {
    {&ADCCC26XX_fxnTable, &adcCC26xxObjects[CC1310_LAUNCHXL_ADC0], &adcCC26xxHWAttrs[CC1310_LAUNCHXL_ADC0]},
};

const uint_least8_t ADC_count = 1;


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

    ADC_init();

    ADC_Params_init(&params);
    batteryHandle = ADC_open(Board_ADC0, &params);
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

