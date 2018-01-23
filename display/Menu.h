//***********************************************************************************
// Copyright 2017, Zksiot Development Ltd.
// Created by Linjie, 2017.08.08
// MCU:	MSP430F5529
// OS: TI-RTOS
// Project:
// File name: 	Menu.h
// Description:	Menu process head.
//***********************************************************************************

#ifndef __ZKSIOT_MENU_H__
#define __ZKSIOT_MENU_H__

#ifdef SUPPORT_MENU

typedef enum {
    MENU_AC_ENTER = 0,
    MENU_AC_UP,
    MENU_AC_DOWN,
    MENU_AC_RETURN,
    MENU_AC_ESC
} MENU_ACTION;

typedef struct _MenuItem { 
    uint8_t count;                  //当前层节点数
    const uint8_t *string;          //菜单标题
    void (*func)();                 //节点函数
    struct _MenuItem *children;     //子节点
    struct _MenuItem *parent;       //父节点
} MenuItem_t; 

typedef struct {
	const MenuItem_t *menu;
	uint8_t index;
	uint8_t startItem;
	uint8_t startRecord;
} MenuObject_t;

#define MENU_PAGE_ITEM          1


extern void Menu_init(void);
extern void Menu_action_proc(MENU_ACTION action);
extern void Menu_show(void);
extern void Menu_exit(void);
extern uint8_t Menu_is_process(void);
extern uint8_t Menu_is_record(void);
extern void PoweroffMenu_init(void);


#endif  /* SUPPORT_MENU */

#endif  /* __ZKSIOT_MENU_H__ */

