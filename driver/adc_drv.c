/*
* @Author: zxt
* @Date:   2018-01-27 15:49:40
* @Last Modified by:   zxt
* @Last Modified time: 2019-02-20 18:09:15
*/
#include "../general.h"



#define     BATTERY_DETECT_PIN          IOID_30

#define     NTC_DETECT_PIN	          	IOID_23


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

#ifdef BOARD_S6_6
    {
        .adcDIO              = NTC_DETECT_PIN,
        .adcCompBInput       = ADC_COMPB_IN_AUXIO7,
#ifdef      BATTERY_REF_FIX
        .refSource           = ADCCC26XX_FIXED_REFERENCE,
#else
        .refSource           = ADCCC26XX_VDDS_REFERENCE,
#endif
        .samplingDuration    = ADCCC26XX_SAMPLING_DURATION_5P46_MS,
        .inputScalingEnabled = true,
        .triggerSource       = ADCCC26XX_TRIGGER_MANUAL,
        .returnAdjustedVal   = false
    }
#endif

};

const ADC_Config ADC_config[ZKS_MAX_ADC] = {
    {&ADCCC26XX_fxnTable, &adcCC26xxObjects[ZKS_BATTERY_ADC], &adcCC26xxHWAttrs[ZKS_BATTERY_ADC]},

#ifdef BOARD_S6_6
    {&ADCCC26XX_fxnTable, &adcCC26xxObjects[ZKS_NTC_ADC], &adcCC26xxHWAttrs[ZKS_NTC_ADC]},
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



