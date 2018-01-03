//***********************************************************************************
// Copyright 2017, Zksiot Development Ltd.
// Created by MIN.ZENG, 2017.10.24
// MCU: MSP430F5529
// OS: TI-RTOS
// Project:
// File name: max31855.h
// Description: ntc process routine head.
//***********************************************************************************

#ifndef __ZKSIOT_MAX31855_H__
#define __ZKSIOT_MAX31855_H__




typedef enum {
    MAX31855_SPI_CH0 = 0,
    MAX31855_SPI_MAX,
} MAX31855_CH_NUM;

typedef enum {
    SENSOR_DEEP_TEMP = 0x10,
} MAX31855_FUNCTION;


typedef void (*DeepTemp_initFxn) (uint8_t chNum);
typedef void (*DeepTemp_measureFxn)(uint8_t chNum);
typedef int32_t (*DeepTemp_getValueFxn)(uint8_t chNum, MAX31855_FUNCTION function);

typedef struct DeepTemp_FxnTable {
    DeepTemp_initFxn  initFxn;
    DeepTemp_measureFxn  measureFxn;  
    DeepTemp_getValueFxn getValueFxn;
} Max31855_FxnTable;




#ifdef SUPPORT_DEEPTEMP
typedef struct {
    uint16_t thermocouple;      // Àä¶ËÎÂ¶È
    uint16_t junction;          // Ð¾Æ¬ÎÂ¶È
} MAX31855_SensorData;

typedef struct {
    uint8_t index;
    MAX31855_SensorData sd;
    double temp;
} MAX31855_MeasureData;
extern void MAX31855_qsort(MAX31855_MeasureData md[], uint8_t nbrOfSample);
extern signed char Max31855_delay(uint8_t cnt);
extern double MAX31855_CalcTemp(double thermocouple, double junction);
#endif








#endif
