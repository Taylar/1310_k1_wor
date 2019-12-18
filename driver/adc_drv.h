#ifndef			ADC_DRV_H
#define			ADC_DRV_H



// #define     BATTERY_REF_VDDS
#define     BATTERY_REF_FIX

#ifdef      BATTERY_REF_FIX
#define     BATTERY_REF_VOL             4300
#else
#define     BATTERY_REF_VOL             3326
#endif




typedef enum ZKS_ADCName
{
	ZKS_BATTERY_ADC,
	ZKS_MAX_ADC,
} ZKS_ADCName;


void AdcDriverInit(void);


#endif			// ADC_DRV_H
