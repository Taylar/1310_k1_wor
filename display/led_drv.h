//***********************************************************************************
// Copyright 2017, Zksiot Development Ltd.
// Created by Linjie, 2017.08.08
// MCU:	MSP430F5529
// OS: TI-RTOS
// Project:
// File name: led_drv.h
// Description: led process routine head.
//***********************************************************************************

#ifndef __ZKSIOT_LED_DRV_H__
#define __ZKSIOT_LED_DRV_H__

typedef enum {
    LED_R,
    LED_B,
    LED_G,
    LED_MAX
} LED_ID;

// variable typedef
typedef struct 
{
	uint8_t enable;
	uint8_t state;
	uint8_t times;
	uint8_t periodT1Set;
	uint8_t periodT2Set;
	uint8_t periodT1;
	uint8_t periodT2;
} singleport_drive_t;


extern void Led_ctrl(uint8_t ledId, uint8_t state, uint32_t period, uint8_t times);
extern void Led_set(uint8_t ledId, uint8_t status);
extern void Led_toggle(uint8_t ledId);
extern void LedInit(void);

#endif	/* __ZKSIOT_LED_DRV_H__ */

