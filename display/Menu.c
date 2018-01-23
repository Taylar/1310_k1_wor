//***********************************************************************************
// Copyright 2017, Zksiot Development Ltd.
// Created by Linjie, 2017.08.08
// MCU:	MSP430F5529
// OS: TI-RTOS
// Project:
// File name: 	Menu.c
// Description:	Menu process.
//***********************************************************************************
#include "../general.h"

#ifdef SUPPORT_MENU
typedef enum {
    MENU_128X32_START_RECORD = 0,
    MENU_128X32_STOP_RECORD,
    MENU_128X32_BT_PRINT,
    MENU_128X32_EXIT,
    MENU_128X32_CONNECTING,
    MENU_128X32_CONNECT_FAIL,
    MENU_128X32_PRINTING,
} MENU_PICTURE;

#define MENU_128X32_OFS             512

const uint8_t menu128x32[]= {
    #include "font\menu128x32.txt"
};

const uint8_t menu128x64[]= {
    #include "font\menu128x64.txt"
};


void Menu_start_record(void);
void Menu_stop_record(void);
void Menu_print_proc(void);
void Menu_power_off(void);
void Menu_restart(void);
extern SysTask_t rSysTask;

//#define USE_ENGLISH_MENU

#ifndef  USE_ENGLISH_MENU
const MenuItem_t rMainMenu[4] = {
    {4, &menu128x32[MENU_128X32_START_RECORD * MENU_128X32_OFS],    Menu_start_record,   NULL,   NULL},
    {4, &menu128x32[MENU_128X32_STOP_RECORD * MENU_128X32_OFS],    Menu_stop_record,   NULL,   NULL},
    {4, &menu128x32[MENU_128X32_BT_PRINT * MENU_128X32_OFS],    Menu_print_proc,   NULL,   NULL},
    {4, &menu128x32[MENU_128X32_EXIT * MENU_128X32_OFS],    Menu_exit,   NULL,   NULL},
};
#else
const MenuItem_t rMainMenu[4] = {
    {4, "Start Monitoring",     Menu_start_record,  NULL,   NULL},
    {4, "End Monitoring",       Menu_stop_record,   NULL,   NULL},
    {4, "Bluetooth Print",      Menu_print_proc,    NULL,   NULL},
    {4, "Exit",                 Menu_exit,          NULL,   NULL},
};

#endif

#ifndef  USE_ENGLISH_MENU
const MenuItem_t PoweroffMenu[] = {
    {3, menu128x64,     Menu_power_off,      NULL,   NULL},
    {3, menu128x64,     Menu_restart,        NULL,   NULL},
    {3, menu128x64,     Menu_exit,           NULL,   NULL},
};

#else
const MenuItem_t PoweroffMenu[] = {
    {3, "Power Off",    Menu_power_off,      NULL,   NULL},
    {3, "Restart",     Menu_restart,        NULL,   NULL},
    {3, "Cancel",      Menu_exit,           NULL,   NULL},
};
#endif

static MenuObject_t rMenuObject;


void PoweroffMenu_init(void)
{
    rMenuObject.menu = PoweroffMenu;
    rMenuObject.index = 2;
    rMenuObject.startItem = 0;
    Disp_clear_all();
}

void Menu_power_off(void)
{    
    rSysTask.state = SYS_STATE_STANDBY;
    rSysTask.stateStep = 0;

    Menu_exit();
}

void Menu_restart(void)
{    
    //Sys_software_reset();
   // Menu_exit();
}

//***********************************************************************************
//
// Menu init function.
//
//***********************************************************************************
void Menu_init(void)
{
    rMenuObject.menu = rMainMenu;
    rMenuObject.index = 0;
    rMenuObject.startItem = 0;
    Disp_clear_all();
}

//***********************************************************************************
//
// Menu action process function.
//
//***********************************************************************************
void Menu_action_proc(MENU_ACTION action)
{
    switch (action) {
        case MENU_AC_ENTER:
            if (rMenuObject.menu[rMenuObject.index].func != NULL) {
                rMenuObject.menu[rMenuObject.index].func();
            } else if (rMenuObject.menu[rMenuObject.index].children != NULL) {
                rMenuObject.menu = rMenuObject.menu[rMenuObject.index].children;
                rMenuObject.index = 0;
                rMenuObject.startItem = 0;
            }
            break;

        case MENU_AC_UP:
            if (rMenuObject.index > 0)
                rMenuObject.index--;
            else
                rMenuObject.index = rMenuObject.menu[0].count - 1;
            break;

        case MENU_AC_DOWN:
            rMenuObject.index++;
            if (rMenuObject.index >= rMenuObject.menu[0].count)
                rMenuObject.index = 0;
            break;

        case MENU_AC_RETURN:
            if (rMenuObject.menu[rMenuObject.index].parent != NULL) {
                rMenuObject.menu = rMenuObject.menu[rMenuObject.index].parent;
            } else {
                rMenuObject.menu = NULL;
            }
            rMenuObject.index = 0;
            rMenuObject.startItem = 0;
            break;

        case MENU_AC_ESC:
            rMenuObject.menu = NULL;
            rMenuObject.index = 0;
            rMenuObject.startItem = 0;
            break;
    }
}

//***********************************************************************************
//
// Menu show function.
//
//***********************************************************************************
void Menu_show(void)
{
#ifdef  USE_ENGLISH_MENU
    uint8_t i;
#endif

    if (rMenuObject.menu == rMainMenu){

#ifndef  USE_ENGLISH_MENU
        Disp_picture(0, 2, 128, 32, rMenuObject.menu[rMenuObject.index].string);
#else
        Disp_clear_all();
        Disp_msg(0, 3, rMenuObject.menu[rMenuObject.index].string, FONT_8X16);
#endif
    }
    else if(rMenuObject.menu == PoweroffMenu){
        
#ifndef  USE_ENGLISH_MENU
        Disp_picture(0, 0, 128, 64, rMenuObject.menu[rMenuObject.index].string);
        Disp_msg(3, rMenuObject.index*2+1, "*", FONT_8X16);
#else
        for(i=0; i < PoweroffMenu[0].count; ++i){
            Disp_msg(1, i*3, " ", FONT_8X16);            
            Disp_msg(4, i*3, rMenuObject.menu[i].string, FONT_8X16);
        }        
        Disp_msg(1, rMenuObject.index*3, "*", FONT_8X16);
#endif
    }    
}

//***********************************************************************************
//
// Menu start record process function.
//
//***********************************************************************************
void Menu_start_record(void)
{
    rMenuObject.startRecord = 1;
   // g_bAlarmSensorFlag = 0;
#ifdef FLASH_EXTERNAL
    Flash_store_record_addr(1);
#endif
    Menu_exit();
}

//***********************************************************************************
//
// Menu stop record process function.
//
//***********************************************************************************
void Menu_stop_record(void)
{
    rMenuObject.startRecord = 0;
#ifdef FLASH_EXTERNAL
    Flash_store_record_addr(0);
#endif
    Menu_exit();
}

//***********************************************************************************
//
// Menu print process function.
//
//***********************************************************************************
void Menu_print_proc(void)
{
#ifdef SUPPORT_BLUETOOTH_PRINT
    uint8_t timeout;

    Sys_lcd_stop_timing();
    Disp_picture(0, 2, 128, 32, &menu128x32[MENU_128X32_CONNECTING * MENU_128X32_OFS]);
    Btp_poweron();
    timeout = 40;   // 40s
    while (timeout--) {
        if (Btp_is_connect()) {
            //Bluetooth connect success
            break;
        }
        Task_sleep(1 * CLOCK_UNIT_S);
    }

    if (Btp_is_connect()) {
        //Bluetooth connect success
        Disp_picture(0, 2, 128, 32, &menu128x32[MENU_128X32_PRINTING * MENU_128X32_OFS]);
        Btp_print_record();
    } else {
        //Bluetooth connect fail
        Disp_picture(0, 2, 128, 32, &menu128x32[MENU_128X32_CONNECT_FAIL * MENU_128X32_OFS]);
    }

    Task_sleep(3 * CLOCK_UNIT_S);
    Btp_poweroff();
    Menu_exit();
    Sys_lcd_start_timing();

#endif
}

//***********************************************************************************
//
// Menu exit function.
//
//***********************************************************************************
void Menu_exit(void)
{
    rMenuObject.menu = NULL;
    rMenuObject.index = 0;
    rMenuObject.startItem = 0;
    Disp_clear_all();
}

//***********************************************************************************
//
// Menu get menu process flag.
//
//***********************************************************************************
uint8_t Menu_is_process(void)
{
	if (rMenuObject.menu == NULL)
		return 0;
	return 1;
}

//***********************************************************************************
//
// Menu get menu process flag.
//
//***********************************************************************************
uint8_t Menu_is_record(void)
{
	return rMenuObject.startRecord;
}

#endif  /* SUPPORT_MENU */
