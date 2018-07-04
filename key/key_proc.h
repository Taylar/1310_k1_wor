//***********************************************************************************
// Copyright 2017, Zksiot Development Ltd.
// Created by Linjie, 2017.08.08
// MCU:	MSP430F5529
// OS: TI-RTOS
// Project:
// File name: key_proc.h
// Description: key process routine head.
//***********************************************************************************

#ifndef __ZKSIOT_KEY_PROC_H__
#define __ZKSIOT_KEY_PROC_H__

#ifdef BOARD_S1_2
#define Board_BUTTON0                            IOID_4
#endif

#ifdef BOARD_S2_2
#define Board_BUTTON0                    IOID_1
#endif

#ifdef BOARD_S6_6
#define Board_BUTTON1                    IOID_1
#define Board_BUTTON0                    IOID_18
#endif


//  define key num
typedef enum {
    KEY0_NUM,

#ifdef BOARD_S6_6
    KEY1_NUM,
#endif

    KEY_MAX_NUM,
}KEY_NUM_E;



// Define key action
#define KEY_ACTION_TYPE_MAX    3
typedef enum {
    KEY_0_SHORT_PRESS,
    KEY_0_LONG_PRESS,
    KEY_0_DOUBLE_PRESS,


#ifdef BOARD_S6_6
    KEY_1_SHORT_PRESS,
    KEY_1_LONG_PRESS,
    KEY_1_DOUBLE_PRESS,
#endif

    KEY_ACTION_MAX
} KEY_ACTION;


// Define key pressed time, unit is 10ms.
#define TIME_KEY_NEW            3
#define TIME_KEY_DOUBLE         40
#define TIME_KEY0_LONG          300
#define TIME_KEY1_LONG          200

// Key state define.
#define KEY_PRESSED             0
#define KEY_RELEASE             1

typedef struct {
    // Key hold on time count.
    uint16_t holdTime;
    // Key double press time count.
    uint16_t doublePressTime;
    // Key Hold flag
    uint8_t holdPress;
    // Key short press flag.
    uint8_t shortPress;
    // Key double press flag.
    uint8_t doublePress;
    // key num
    uint8_t keyNum;
} KeyTask_t;



void KeyInit(void);

void KeyRegister(void (*Cb)(void), KEY_ACTION action);

uint8_t KeyReadState(KEY_NUM_E key);


#endif	/* __ZKSIOT_KEY_PROC_H__ */

