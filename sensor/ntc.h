//***********************************************************************************
// Copyright 2017, Zksiot Development Ltd.
// Created by Linjie, 2017.08.08
// MCU:	MSP430F5529
// OS: TI-RTOS
// Project:
// File name: ntc.h
// Description: ntc process routine head.
//***********************************************************************************

#ifndef __ZKSIOT_NTC_H__
#define __ZKSIOT_NTC_H__

#ifdef SUPPORT_NTC

#ifdef NTC_ELIWELL_10K
#define START_TEMPERATURE           -35
#define CTRL_RES_NUM                4
#define ADC_COMPENSATION            0

#elif  defined(NTC_TIANYOU_10K)

#define START_TEMPERATURE           -40
#define CTRL_RES_NUM                4
#define ADC_COMPENSATION            0

#elif  defined(NTC_XINXIANG_10K)

#define START_TEMPERATURE           -40
#define CTRL_RES_NUM                4
#define ADC_COMPENSATION            0

#elif defined(NTC_KEMIT_10K)

#define START_TEMPERATURE           -40
#define CTRL_RES_NUM                4
#define ADC_COMPENSATION            0

#elif defined(NTC_KEMIT_100K)

#define START_TEMPERATURE           -50
#define CTRL_RES_NUM                4
#define ADC_COMPENSATION            0

#elif defined(NTC_KEMIT_PT1000)

#define START_TEMPERATURE           70
#define CTRL_RES_NUM                1
#define ADC_COMPENSATION            13

#elif defined(NTC_KEMIT_PT1000_2ADC)

#define START_TEMPERATURE           -200
#define CTRL_RES_NUM                1
#define ADC_COMPENSATION            0

#define ADC_BASEON_ADC

#endif

typedef struct {
    //NTC ADC Series Resistor control pin.
    uint16_t pin;
    //NTC ADC Series Resistor value x100(K).
    float res;
    //NTC ADC Series Resistor select level.
    //uint16_t level;
} ResHWAtts_t;


#endif  /* SUPPORT_NTC */

#endif	/* __ZKSIOT_NTC_H__ */

