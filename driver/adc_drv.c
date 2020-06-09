/*
* @Author: justfortest
* @Date:   2018-01-27 15:49:40
* @Last Modified by:   zxt
* @Last Modified time: 2020-06-08 16:52:43
*/
#include "../general.h"



#define     BATTERY_DETECT_PIN          IOID_30

#define     PREVENTIVE_INSERT_DETECT_PIN	          	IOID_29


ADCCC26XX_Object adcCC26xxObjects[ZKS_MAX_ADC];



const ADCCC26XX_HWAttrs adcCC26xxHWAttrs[ZKS_MAX_ADC] = {
    {
        .adcDIO              = BATTERY_DETECT_PIN,
        .adcCompBInput       = ADC_COMPB_IN_AUXIO0,
#ifdef      BATTERY_REF_FIX
        .refSource           = ADCCC26XX_FIXED_REFERENCE,
#else
        .refSource           = ADCCC26XX_VDDS_REFERENCE,
#endif
        .samplingDuration    = ADCCC26XX_SAMPLING_DURATION_5P46_MS,
        .inputScalingEnabled = true,
        .triggerSource       = ADCCC26XX_TRIGGER_MANUAL,
        .returnAdjustedVal   = false
    },

#ifdef BOARD_S3
    {
        .adcDIO              = PREVENTIVE_INSERT_DETECT_PIN,
        .adcCompBInput       = ADC_COMPB_IN_AUXIO1,
// #ifdef      BATTERY_REF_FIX
//         .refSource           = ADCCC26XX_FIXED_REFERENCE,
// #else
        .refSource           = ADCCC26XX_FIXED_REFERENCE,
// #endif
        .samplingDuration    = ADCCC26XX_SAMPLING_DURATION_5P46_MS,
        .inputScalingEnabled = true,
        .triggerSource       = ADCCC26XX_TRIGGER_MANUAL,
        .returnAdjustedVal   = false
    }
#endif

};

const ADC_Config ADC_config[ZKS_MAX_ADC] = {
    {&ADCCC26XX_fxnTable, &adcCC26xxObjects[ZKS_BATTERY_ADC], &adcCC26xxHWAttrs[ZKS_BATTERY_ADC]},

#if defined(BOARD_S3)
    {&ADCCC26XX_fxnTable, &adcCC26xxObjects[PREVENTIVE_INSERT_ADC], &adcCC26xxHWAttrs[PREVENTIVE_INSERT_ADC]},
#endif
};

const uint_least8_t ADC_count = ZKS_MAX_ADC;


//***********************************************************************************
// brief:Init the adc function, this function should be call before other adc api
// 
// parameter: 
//***********************************************************************************
void AdcDriverInit(void)
{
    ADC_init();
}



