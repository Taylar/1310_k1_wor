//***********************************************************************************
// Copyright 2017, Zksiot Development Ltd.
// Created by zhengxuntai, 2017.12.18
// MCU: MSP430F5529
// OS: TI-RTOS
// Project:
// File name: led_drv.c
// Description: led process routine.
//***********************************************************************************
#include "../general.h"

static Clock_Struct keyClkStruct;
static Clock_Handle keyClkHandle;

static KeyTask_t rKeyTask;


#define Board_BUTTON0                       IOID_4




const PIN_Config keyPinTable[] = {
    Board_BUTTON0 | PIN_INPUT_EN | PIN_PULLUP | PIN_IRQ_NEGEDGE,       /* key isr enable          */
    PIN_TERMINATE
};


static PIN_State   keyState;
static PIN_Handle  keyHandle;

//***********************************************************************************
//
// Led_io_init.
//      
//
//***********************************************************************************
void Key_io_init(PIN_IntCb pCb)
{
    keyHandle = PIN_open(&keyState, keyPinTable);
    PIN_registerIntCb(keyHandle, pCb);
}


//***********************************************************************************
//
// Key stop scan.
//
//***********************************************************************************
static void Key_scan_stop(void)
{
    rKeyTask.holdTime = 0;
    rKeyTask.doublePressTime = 0;
    rKeyTask.shortPress = 0;
    rKeyTask.doublePress = 0;
    Clock_stop(keyClkHandle);
}

//***********************************************************************************
//
// Key scan callback function, use 10ms clock.
//
//***********************************************************************************
static void Key_scanFxn(UArg arg0)
{
    if (GPIO_read(Board_BUTTON0) == KEY_PRESSED) {
        // Power key pressed.
        if (rKeyTask.action != KEY_0_PRESS) {
            rKeyTask.holdTime = 0;
            rKeyTask.shortPress = 0;
            rKeyTask.doublePress = 0;
        }
        if (rKeyTask.holdTime <= TIME_KEY0_LONG)
            rKeyTask.holdTime++;
        if (rKeyTask.shortPress == 0 && rKeyTask.holdTime == TIME_KEY_NEW) {
            rKeyTask.holdTime = 0;
            rKeyTask.shortPress = 1;
        } else if (rKeyTask.shortPress == 1 && rKeyTask.holdTime == TIME_KEY0_LONG) {
            rKeyTask.shortPress = 0;
            rKeyTask.code = _VK_POWER;
//            Sys_event_post(SYS_EVT_KEY);
            Key_scan_stop();
        }
        rKeyTask.action = KEY_0_PRESS;
    } else {
        rKeyTask.holdTime = 0;
#ifdef SUPPORT_DOUBLE_CLICK
        rKeyTask.doublePressTime++;
        // key release.
        if (rKeyTask.shortPress == 1) {
            // short press, clear short press flag.
            rKeyTask.shortPress = 0;
            // judge double press
            if (rKeyTask.doublePress == 0) {
                rKeyTask.doublePressTime = 0;
                rKeyTask.doublePress = 1;
            } else {
                if (rKeyTask.doublePressTime < TIME_KEY_DOUBLE) {
                    rKeyTask.doublePress = 0;
                    if (rKeyTask.action == KEY_0_PRESS) {
                        rKeyTask.code = _VK_NULL;
                        Sys_event_post(SYS_EVT_KEY);
                        Key_scan_stop();
                    }
                }
            }
        } else if (rKeyTask.doublePress == 1) {
            // judge double press time
            if (rKeyTask.doublePressTime >= TIME_KEY_DOUBLE) {
                rKeyTask.doublePress = 0;
                if (rKeyTask.action == KEY_0_PRESS) {
                    rKeyTask.code = _VK_SELECT;
                    Sys_event_post(SYS_EVT_KEY);
                    Key_scan_stop();
                }
            }
        } else {
            Key_scan_stop();
        }
#else
        // key release.
        if (rKeyTask.shortPress == 1) {
            // short press, clear short press flag.
            rKeyTask.shortPress = 0;
            if (rKeyTask.action == KEY_0_PRESS) {
                rKeyTask.code = _VK_SELECT;
//                Sys_event_post(SYS_EVT_KEY);
                Key_scan_stop();
            }
        } else {
            Key_scan_stop();
        }
#endif
    }
}

//***********************************************************************************
//
// Key gpio hwi callback function.
//
//***********************************************************************************
static void Key_isrFxn(UInt index)
{
    if (Clock_isActive(keyClkHandle) == FALSE)
        Clock_start(keyClkHandle);
}

//***********************************************************************************
//
// Key init.
//
//***********************************************************************************
void Key_init(void)
{
    rKeyTask.holdTime = 0;
    rKeyTask.doublePressTime = 0;
    rKeyTask.shortPress = 0;
    rKeyTask.doublePress = 0;
    rKeyTask.action = KEY_NONE;
    rKeyTask.code = _VK_NULL;

    /* Construct a 10ms periodic Clock Instance to scan key */
    Clock_Params clkParams;
    Clock_Params_init(&clkParams);
    clkParams.period = 10 * CLOCK_UNIT_MS;
    clkParams.startFlag = FALSE;
    Clock_construct(&keyClkStruct, (Clock_FuncPtr)Key_scanFxn, 0, &clkParams);
    /* Obtain clock instance handle */
    keyClkHandle = Clock_handle(&keyClkStruct);

    /* install Button callback */
    Key_io_init((PIN_IntCb)Key_isrFxn);

}

//***********************************************************************************
//
// Key process task function.
//
//***********************************************************************************
uint8_t Key_get(void)
{
    return rKeyTask.code;
}

