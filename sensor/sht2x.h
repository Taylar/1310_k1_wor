//***********************************************************************************
// Copyright 2017, Zksiot Development Ltd.
// Created by Linjie, 2017.08.08
// MCU:	MSP430F5529
// OS: TI-RTOS
// Project:
// File name: sht2x.h
// Description: SHT2x humiture sensor process routine head.
//***********************************************************************************

#ifndef __ZKSIOT_SHT2X_H__
#define __ZKSIOT_SHT2X_H__

#ifdef SUPPORT_SHT2X

#define Board_SHT2x_ADDR            (0x80 >> 1)

#define TEMPERATURE_OVERLOAD        0x7fff
#define HUMIDTY_OVERLOAD            0xffff
#define DEEP_TEMP_OVERLOAD          0x7fffffff

// sensor command
typedef enum{
    TRIG_T_MEASURE_HM = 0xE3, // command trig. temp meas. hold master
    TRIG_H_MEASURE_HM = 0xE5, // command trig. humidity meas. hold master
    TRIG_T_MEASURE_POLL = 0xF3, // command trig. temp meas. no hold master
    TRIG_H_MEASURE_POLL = 0xF5, // command trig. humidity meas. no hold master
    USER_REG_W = 0xE6, // command writing user register
    USER_REG_R = 0xE7, // command reading user register
    SOFT_RESET = 0xFE // command soft reset
} SHT2X_CMD;

typedef enum {
    SHT2x_RES_12_14BIT = 0x00, // RH=12bit, T=14bit
    SHT2x_RES_8_12BIT = 0x01, // RH= 8bit, T=12bit
    SHT2x_RES_10_13BIT = 0x80, // RH=10bit, T=13bit
    SHT2x_RES_11_11BIT = 0x81, // RH=11bit, T=11bit
    SHT2x_RES_MASK = 0x81 // Mask for res. bits (7,0) in user reg.
} SHT2X_RESOLUTION;

typedef enum {
    SHT2x_EOB_ON = 0x40, // end of battery
    SHT2x_EOB_MASK = 0x40, // Mask for EOB bit(6) in user reg.
} SHT2X_EOB;

typedef enum {
    SHT2x_HEATER_ON = 0x04, // heater on
    SHT2x_HEATER_OFF = 0x00, // heater off
    SHT2x_HEATER_MASK = 0x04, // Mask for Heater bit(2) in user reg.
} SHT2X_HEATER;


// CRC algorithm
#define POLYNOMIAL                  0x131 //P(x)=x^8+x^5+x^4+1 = 100110001


#endif  /* SUPPORT_SHT2X */

#endif	/* __ZKSIOT_SHT2X_H__ */

