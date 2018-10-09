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
#ifdef SUPPORT_ALARM_RECORD_QURERY
    MENU_128X32_ALRM_QUERY,
#endif
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
#ifdef SUPPORT_FLIGHT_MODE
    #include "font\menu128x16.txt" // 128*16dot bitmap picture
#else
    #include "font\menu128x64.txt"
#endif
};
#ifndef SUPPORT_FLIGHT_MODE
const uint8_t menu128x16[]= {
    #include "font\menu128x16.txt" // 128*16dot bitmap picture
};
#endif

void Menu_start_record(void);
void Menu_stop_record(void);
void Menu_print_proc(void);
void Menu_power_off(void);
void Menu_restart(void);

void Menu_flight_mode_entry(void);
void Menu_flight_mode_exit(void);

void Menu_alrm_query_proc(void);
void Menu_alarm_recode_proc(void);
extern SysTask_t rSysTask;




#ifdef SUPPORT_BLE_PRINT_LIST_MODE

#define LIST_PRINT_MODE_SINGLE  0 // One Select trigger once print
#define LIST_PRINT_MODE_MULTIPLE 1 //

static void PrintList_ItemSelect(void);
static void PrintList_Cancel(void);
static void PrintList_Print(void);
static void PrintList_Select_sign_show(uint8_t index);
static void PrintList_Cancel_sign_show(uint8_t index);

void Menu_printList_proc(void);
#define PRINT_LIST_PAGE_MAX 4 // Each page ECHO max line data
MenuItem_t rPrintMenu[] = {
    {PRINT_LIST_PAGE_MAX, &menu128x32[MENU_128X32_BT_PRINT * MENU_128X32_OFS], Menu_printList_proc,    NULL,   NULL},
    {PRINT_LIST_PAGE_MAX, &menu128x32[MENU_128X32_BT_PRINT * MENU_128X32_OFS], Menu_printList_proc,    NULL,   NULL},
    {PRINT_LIST_PAGE_MAX, &menu128x32[MENU_128X32_BT_PRINT * MENU_128X32_OFS], Menu_printList_proc,    NULL,   NULL},
    {PRINT_LIST_PAGE_MAX, &menu128x32[MENU_128X32_BT_PRINT * MENU_128X32_OFS], Menu_printList_proc,    NULL,   NULL},
};

static void PrintList_init_Page(void);
static void PrintList_getNext_Page(void);
#define R_PRINT_MENU_CHILDRED rPrintMenu
#endif

#ifndef R_PRINT_MENU_CHILDRED
#define R_PRINT_MENU_CHILDRED NULL
#endif

#ifdef SUPPORT_ALARM_RECORD_QURERY
#define RECORD_ALARM_PAGE_MAX 1
typedef struct {
   uint16_t alrmCount;
   uint16_t alrmindex;
}alrmRecord_t;

alrmRecord_t alrmRecord;
MenuItem_t rRecordArlmMenu[] = {

    {RECORD_ALARM_PAGE_MAX, &menu128x32[MENU_128X32_BT_PRINT * MENU_128X32_OFS], Menu_alarm_recode_proc,    NULL,   NULL},

};
#endif

#ifndef  USE_ENGLISH_MENU

#ifdef SUPPORT_ALARM_RECORD_QURERY

const MenuItem_t rMainMenu[5] = {
    {5, &menu128x32[MENU_128X32_START_RECORD * MENU_128X32_OFS],    Menu_start_record,     NULL,   NULL},
    {5, &menu128x32[MENU_128X32_STOP_RECORD * MENU_128X32_OFS],     Menu_stop_record,      NULL,   NULL},
    {5, &menu128x32[MENU_128X32_BT_PRINT * MENU_128X32_OFS],        Menu_print_proc,       R_PRINT_MENU_CHILDRED,   NULL},
    {5, &menu128x32[MENU_128X32_ALRM_QUERY * MENU_128X32_OFS],      Menu_alrm_query_proc,  rRecordArlmMenu,   NULL},
    {5, &menu128x32[MENU_128X32_EXIT * MENU_128X32_OFS],            Menu_exit,             NULL,   NULL},
};
#else
const MenuItem_t rMainMenu[4] = {
    {4, &menu128x32[MENU_128X32_START_RECORD * MENU_128X32_OFS],    Menu_start_record,  NULL,   NULL},
    {4, &menu128x32[MENU_128X32_STOP_RECORD * MENU_128X32_OFS],     Menu_stop_record,   NULL,   NULL},
    {4, &menu128x32[MENU_128X32_BT_PRINT * MENU_128X32_OFS],        Menu_print_proc,    R_PRINT_MENU_CHILDRED,   NULL},
    {4, &menu128x32[MENU_128X32_EXIT * MENU_128X32_OFS],            Menu_exit,          NULL,   NULL},
};
#endif


#else

#ifdef SUPPORT_ALARM_RECORD_QURERY
const MenuItem_t rMainMenu[5] = {
    {5, "Start Monitoring",     Menu_start_record,  NULL,   NULL},
    {5, "End Monitoring",       Menu_stop_record,   NULL,   NULL},
    {5, "Bluetooth Print",      Menu_print_proc,    R_PRINT_MENU_CHILDRED,   NULL},
    {5, "Alarm Query",          Menu_alrm_query_proc,    rRecordArlmMenu,   NULL},
    {5, "Exit",                 Menu_exit,          NULL,   NULL},
};
#else
const MenuItem_t rMainMenu[4] = {
      {4, "Start Monitoring",     Menu_start_record,  NULL,   NULL},
      {4, "End Monitoring",       Menu_stop_record,   NULL,   NULL},
      {4, "Bluetooth Print",      Menu_print_proc,    R_PRINT_MENU_CHILDRED,   NULL},
      {4, "Exit",                 Menu_exit,          NULL,   NULL},
};
#endif
#endif

#ifdef SUPPORT_FLIGHT_MODE
typedef enum{
    POWEROFFMENU_POWEROFF = 0,
    POWEROFFMENU_RESTART = 1,
    POWEROFFMENU_FLIGHT_ENTRY = 2,
    POWEROFFMENU_FLIGHT_EXIT = 3,
    POWEROFFMENU_CANCEL = 4,
}POWER_MENU_T;

#define POWEROFF_OFFSET 16*16 // 128bit*16line pic
#define PrintList_PrintImageStr     &menu128x64[5*POWEROFF_OFFSET]
#define PrintList_CancelImageStr    &menu128x64[2*POWEROFF_OFFSET]
#else
typedef enum{
    PRINTLIST_CANCEL = 0,
    PRINTLIST_PRINT = 1,
}POWER_MENU_T;

#define POWEROFF_OFFSET 16*16 // 128bit*16line pic
#define PrintList_PrintImageStr     &menu128x16[PRINTLIST_PRINT*POWEROFF_OFFSET]
#define PrintList_CancelImageStr    &menu128x16[PRINTLIST_CANCEL*POWEROFF_OFFSET]
#endif

#ifndef  USE_ENGLISH_MENU
const MenuItem_t PoweroffMenu[] = {
#ifdef SUPPORT_FLIGHT_MODE

    {5, &menu128x64[POWEROFFMENU_POWEROFF*POWEROFF_OFFSET],     Menu_power_off,             NULL,   NULL},
    {5, &menu128x64[POWEROFFMENU_RESTART*POWEROFF_OFFSET] ,     Menu_restart,               NULL,   NULL},
    {5, &menu128x64[3*POWEROFF_OFFSET] ,                        Menu_flight_mode_entry,     NULL,   NULL},
    {5, &menu128x64[4*POWEROFF_OFFSET],                         Menu_flight_mode_exit,      NULL,   NULL},
    {5, &menu128x64[2*POWEROFF_OFFSET],                         Menu_exit,                  NULL,   NULL},
#else
#ifdef SUPPORT_G7_PROTOCOL
    {2, menu128x64 + 16*24,     Menu_restart,        NULL,   NULL},
    {2, menu128x64 + 16*24,     Menu_exit,           NULL,   NULL},
#else
    {3, menu128x64,             Menu_power_off,      NULL,   NULL},
    {3, menu128x64,             Menu_restart,        NULL,   NULL},
    {3, menu128x64,             Menu_exit,           NULL,   NULL},
#endif
#endif
};

#define POWEROFFMENU_LIST_MAX (sizeof(PoweroffMenu)/sizeof(MenuItem_t)) //5

#else
#ifndef SUPPORT_FLIGHT_MODE
const MenuItem_t PoweroffMenu[] = {
    {3, "Power Off",   Menu_power_off,      NULL,   NULL},
    {3, "Restart",     Menu_restart,        NULL,   NULL},
    {3, "Cancel",      Menu_exit,           NULL,   NULL},
};
#else
const MenuItem_t PoweroffMenu[] = {
    {5, "Power Off",    Menu_power_off,         NULL,   NULL},
    {5, "Restart",      Menu_restart,           NULL,   NULL},
    {5, "Entry Flight ",Menu_flight_mode_entry, NULL,   NULL},
    {5, "Exit Flight ", Menu_flight_mode_exit,  NULL,   NULL},
    {5, "Cancel",       Menu_exit,              NULL,   NULL},
};
#endif
#endif

static MenuObject_t rMenuObject;


void PoweroffMenu_init(void)
{
    rMenuObject.menu = PoweroffMenu;
    rMenuObject.index = 2;
#ifdef  SUPPORT_FLIGHT_MODE
    rMenuObject.index = 4;
#endif
    rMenuObject.startItem = 0;
    Disp_clear_all();
}

void Menu_power_off(void)
{   
#ifdef BOARD_S6_6
    S6Sleep();
#endif
    Menu_exit();
    Disp_poweroff();
    deviceMode = DEVICES_OFF_MODE;
}

void Menu_restart(void)
{    
    // ConcenterSleep();
    g_rSysConfigInfo.rtc =Rtc_get_calendar();
    Flash_store_config();
    while(1)
        SysCtrlSystemReset();
   // Menu_exit();
}

void Menu_init_byflash(void)
{
    FlashPrintRecordAddr_t recordAddr;

    recordAddr = Flash_get_record_addr();

    if ((recordAddr.start == recordAddr.end) &&
		recordAddr.start != 0xffffffff){//start record
            rMenuObject.startRecord = 1;
    }    
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


            if (rMenuObject.menu[rMenuObject.index].children != NULL) {
                rMenuObject.menu = rMenuObject.menu[rMenuObject.index].children;
#ifdef SUPPORT_ALARM_RECORD_QURERY
                if (rMenuObject.menu == rRecordArlmMenu){
                    alrmRecord.alrmCount = Flash_get_alarm_record_items();
                    alrmRecord.alrmindex = 1;
                }
#endif
#ifdef SUPPORT_BLE_PRINT_LIST_MODE
                if (rMenuObject.menu == rPrintMenu){
                    PrintList_init_Page();
                }
#endif
                rMenuObject.index = 0;
                rMenuObject.startItem = 0;
                break;
            }

            if (rMenuObject.menu[rMenuObject.index].func != NULL) {
#ifdef SUPPORT_BLE_PRINT_LIST_MODE
                if (rMenuObject.menu == rPrintMenu){
                    rMenuObject.menu[rMenuObject.index].func();
                    break;
                }
#endif

#ifdef SUPPORT_FLIGHT_MODE
                // When Power On set index to 3, first operation , it will not work
                if (rMenuObject.menu == PoweroffMenu)
                if(rMenuObject.index == POWEROFFMENU_FLIGHT_EXIT){
                    if(!Flight_mode_isFlightMode()){
                        rMenuObject.menu[POWEROFFMENU_FLIGHT_ENTRY].func();
                    }else{
                        rMenuObject.menu[POWEROFFMENU_FLIGHT_EXIT].func();
                    }
                    break;
                }
#endif
#if 1
                // When Power On set index to 3, first operation , it will not work
                if (rMenuObject.menu == rMainMenu){
                    if(rMenuObject.index == MENU_128X32_STOP_RECORD){
                        if(rMenuObject.startRecord==0){
                            rMenuObject.menu[MENU_128X32_START_RECORD].func();
                        }else{
                            rMenuObject.menu[MENU_128X32_STOP_RECORD].func();
                        }
                        break;
                    }
                }
#endif
                {
                rMenuObject.menu[rMenuObject.index].func();
                }
            }

            break;

        case MENU_AC_UP:
            if (rMenuObject.index > 0){
                rMenuObject.index--;
                if (rMenuObject.menu == rMainMenu){//normal menu , not poweroff menu
                    if (!(g_rSysConfigInfo.module & MODULE_BTP) &&  rMenuObject.index == 2 )//没有蓝牙模块不显示打印
                        rMenuObject.index = 1;
                }
            }
            else
                rMenuObject.index = rMenuObject.menu[0].count - 1;
            break;

        case MENU_AC_DOWN:
            rMenuObject.index++;
            if (rMenuObject.menu == rMainMenu){//normal menu , not poweroff menu
                if (!(g_rSysConfigInfo.module & MODULE_BTP) &&  rMenuObject.index == 2)//没有蓝牙模块不显示打印
                    rMenuObject.index = 3;
            }
            
            #ifdef SUPPORT_FLIGHT_MODE
            if (rMenuObject.menu == PoweroffMenu)
            if(rMenuObject.index == 3){
                if(!Flight_mode_isFlightMode()){
                    rMenuObject.index = 4;
                }
            }
            #endif
#ifdef SUPPORT_ALARM_RECORD_QURERY
            if(rMenuObject.index && (rMenuObject.menu == rRecordArlmMenu) ){
                alrmRecord.alrmindex = alrmRecord.alrmindex + 1;
                if(alrmRecord.alrmindex > ALARM_RECORD_QURERY_MAX_ITEM  || alrmRecord.alrmindex > alrmRecord.alrmCount )
                   alrmRecord.alrmindex = 1;
            }
#endif

#ifdef SUPPORT_BLE_PRINT_LIST_MODE
            if(rMenuObject.index && (rMenuObject.menu == rPrintMenu) ){
                if(rMenuObject.menu[rMenuObject.index -1].func == PrintList_Cancel){
                    rMenuObject.index = rMenuObject.menu[0].count;
                }
            }
            if ((rMenuObject.index >= rMenuObject.menu[0].count) && (rMenuObject.menu == rPrintMenu)){
                PrintList_getNext_Page();
            }
#endif

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
#ifdef SUPPORT_FLIGHT_MODE
// Flight Mode display
void Flight_Mode_Menu_show(void)
{
    Disp_clear_all();
#if 1
    //if(rMenuObject.index == 0)
    {
    // PowerOff
        if(g_rSysConfigInfo.status & STATUS_HIDE_PWOF_MENU){
            if(rMenuObject.index == 0)
                rMenuObject.index = 1;
        }else
            Disp_picture(0, 0, 128, 16, rMenuObject.menu[0].string);
    }
#else
    // PowerOff
    Disp_picture(0, 0, 128, 16, rMenuObject.menu[0].string);
#endif

    //if(rMenuObject.index == 1)
    // Restart
    Disp_picture(0, 2, 128, 16, rMenuObject.menu[1].string);

    // Entry Flight/Exit Flight
    if(Flight_mode_isFlightMode()){
        Disp_picture(0, 4, 128, 16, rMenuObject.menu[3].string);
    }
    else{
        Disp_picture(0, 4, 128, 16, rMenuObject.menu[2].string);
    }

    if(rMenuObject.index == 2){
        if(Flight_mode_isFlightMode()){
            //Disp_picture(0, 6, 128, 16, rMenuObject.menu[3].string);
            rMenuObject.index = 3;
        }
    }
    // Cancel
    Disp_picture(0, 6, 128, 16, rMenuObject.menu[4].string);

    if(rMenuObject.index == 0)
        Disp_msg(3, 0, "*", FONT_8X16);
    else if(rMenuObject.index == 1)
            Disp_msg(3, 2, "*", FONT_8X16);
    else if(rMenuObject.index == 2 || rMenuObject.index == 3)
        Disp_msg(2, 4, "*", FONT_8X16);
    else
        Disp_msg(3, 6, "*", FONT_8X16);

}

// english Mode// Page-turning function
void Flight_Mode_Menu_EN_show(void)
{

    uint8_t i=0;
    uint8_t index_menu = rMenuObject.index;
    uint8_t index;
    Disp_clear_all();
    //uint8_t star_pos = 0;

    if(index_menu <3){ // 0 1 2/3
        i = 0;
        index = 0;
        Disp_msg(1, i*3, " ", FONT_8X16);
        Disp_msg(4, i*3, rMenuObject.menu[index].string, FONT_8X16);

        i = 1;
        index = 1;
        Disp_msg(1, i*3, " ", FONT_8X16);
        Disp_msg(4, i*3, rMenuObject.menu[index].string, FONT_8X16);

        i = 2;
        Disp_msg(1, i*3, " ", FONT_8X16);
        if(Flight_mode_isFlightMode()){
            Disp_msg(4, i*3, rMenuObject.menu[POWEROFFMENU_FLIGHT_EXIT].string, FONT_8X16);
        }
        else{
            Disp_msg(4, i*3, rMenuObject.menu[POWEROFFMENU_FLIGHT_ENTRY].string, FONT_8X16);
        }
    }else{ // 1 2/3 4
        i = 0;
        index = 1;
        Disp_msg(1, i*3, " ", FONT_8X16);
        Disp_msg(4, i*3, rMenuObject.menu[index].string, FONT_8X16);

        i = 1;
        Disp_msg(1, i*3, " ", FONT_8X16);
        if(Flight_mode_isFlightMode()){
            Disp_msg(4, i*3, rMenuObject.menu[POWEROFFMENU_FLIGHT_EXIT].string, FONT_8X16);
            //rMenuObject.index = POWEROFFMENU_FLIGHT_EXIT;
        }
        else{
            Disp_msg(4, i*3, rMenuObject.menu[POWEROFFMENU_FLIGHT_ENTRY].string, FONT_8X16);
            //rMenuObject.index = POWEROFFMENU_FLIGHT_ENTRY;
        }

        i = 2;
        index = 4;
        Disp_msg(1, i*3, " ", FONT_8X16);
        Disp_msg(4, i*3, rMenuObject.menu[index].string, FONT_8X16);
    }

    if(rMenuObject.index == 2){
        if(Flight_mode_isFlightMode()){
            rMenuObject.index = 3;
        }
    }

    switch(index_menu){

        case 0: // 0 1 2/3
            i = 0;
            Disp_msg(1, i*3, "*", FONT_8X16);
            break;
        case 1: // 0 1 2/3
            i = 1;
            Disp_msg(1, i*3, "*", FONT_8X16);
            break;
        case 2: // 0 1 2/3
            i = 2;
            Disp_msg(1, i*3, "*", FONT_8X16);
            break;
        // next page
        case 3: // 1 2/3 4
            i = 1;
            Disp_msg(1, i*3, "*", FONT_8X16);
            break;
        case 4: // 1 2/3 4
            i = 2;
            Disp_msg(1, i*3, "*", FONT_8X16);
            break;
    }
}

void Menu_flight_mode_entry(void)
{

#ifdef  SUPPORT_DEVICED_STATE_UPLOAD
    Flash_store_devices_state(TYPE_FLIGHT_MODE_START);
#endif

    Flight_mode_local_setting();

    Menu_exit();
}
void Menu_flight_mode_exit(void)
{
    Flight_mode_cancel();

    Menu_exit();
}

#endif

#ifdef SUPPORT_BLE_PRINT_LIST_MODE

#if (LIST_PRINT_MODE_SINGLE)&&(LIST_PRINT_MODE_MULTIPLE)
#error "Please Single Choice LIST_PRINT_MODE_SINGLE or LIST_PRINT_MODE_MULTIPLE"
#endif

#if DEUBG_BLE_PRINT
// Page-turning function
static uint32_t ListTest[NETGATE_BIND_NODE_MAX] = {0x1234,56,0,789,0,0,0,0,1000,999,0,0x22222,0x123456,0x12345678,0x83331234};
#endif

static uint32_t PrintListPage[PRINT_LIST_PAGE_MAX] = {0};
static uint32_t PrintListIndex = 0;
static uint32_t PrintListSelect[LSIT_PRINT_SELECT_MAX] = {0};

char PrintList_getSelectedDID_NUM(uint32_t *DIDBuffer, uint8_t *DID_Num)
{
    int i,ret=0;
    for(i=0; i < LSIT_PRINT_SELECT_MAX; i++){
        if(0 != PrintListSelect[i]){
            *(DIDBuffer+ret) = PrintListSelect[i];
            ret++;
        }
    }
    *DID_Num = ret;
    return ret;
}

char PrintList_getSelectedNum(void)
{
    int i,ret=0;
    for(i=0; i < LSIT_PRINT_SELECT_MAX; i++){
        if(0 != PrintListSelect[i])
            ret++;
    }
    return ret;
}
static char PrintList_isSelected(uint32_t DeID)
{
    int i,bSelect=0;
    for(i=0; i < LSIT_PRINT_SELECT_MAX; i++){
        if(DeID== PrintListSelect[i])
            bSelect = 1;
    }
    return bSelect;
}
static char PrintList_Select_insert(uint32_t DeID)
{
    int i,bInsert=0;
    for(i=0; i < LSIT_PRINT_SELECT_MAX; i++){
        if(0== PrintListSelect[i]){
            PrintListSelect[i] = DeID;
            bInsert = 1;
            break;
        }
    }
    return bInsert;
}

static char PrintList_Select_delete(uint32_t DeID)
{
    int i,bDel=0;
    for(i=0; i < LSIT_PRINT_SELECT_MAX; i++){
        if(DeID== PrintListSelect[i]){
            PrintListSelect[i] = 0;
            bDel = 1;
            break;
        }
    }
    return bDel;
}
static void PrintList_Select_ClearAll(void)
{
    int i;
    for(i=0; i < LSIT_PRINT_SELECT_MAX; i++){
        PrintListSelect[i] = 0;
    }
}

char PrintList_Select_insert_toggle(uint32_t DeID)
{
    // find
    int ret;
    if(PrintList_isSelected(DeID))
        ret = PrintList_Select_delete(DeID);
    else
        ret = PrintList_Select_insert(DeID);
    return ret;
}

static void PrintList_Page_ClearAll(void)
{
    int i;
    for(i=0; i < PRINT_LIST_PAGE_MAX; i++){
        PrintListPage[i] = 0;
    }
}

static void PrintList_getNext_Page(void);
// Fist time Into Print Mode
static void PrintList_init_Page(void)
{
    PrintListIndex = 0;
    //PrintList_Select_ClearAll();
    PrintList_Page_ClearAll();

    PrintList_getNext_Page();
}

uint32_t System_get_bindnode_DID_index(uint8_t index)
{

#ifdef SUPPORT_NETGATE_BIND_NODE
    if(index < NETGATE_BIND_NODE_MAX)
    if((0 != g_rSysConfigInfo.bindnode[index].Deviceid) &&(0xffffffff != g_rSysConfigInfo.bindnode[index].Deviceid))
        return g_rSysConfigInfo.bindnode[index].Deviceid;
    else
        return 0;

#endif

//#ifdef SUPPORT_NETGATE_BIND_NODE
//    if(index < NETGATE_BIND_NODE_MAX)
//        return g_rSysConfigInfo.bindnode[index].Deviceid;
//#endif
    return 0;
}
static void PrintList_getNext_Page(void)
{
    // clear page
    int r_num=0;
    int i;

    if((PrintListIndex >= NETGATE_BIND_NODE_MAX)){
        if(0 == PrintListPage[PRINT_LIST_PAGE_MAX-2])
            PrintListIndex = 0;
    }

    for( i = 0; (i <PRINT_LIST_PAGE_MAX) /*&&(PrintListIndex < NETGATE_BIND_NODE_MAX)*/ ; i++)
    {
        for(; PrintListIndex < NETGATE_BIND_NODE_MAX; ){

#if DEUBG_BLE_PRINT
            if(ListTest[PrintListIndex] != 0){
                PrintListPage[i] = ListTest[PrintListIndex%NETGATE_BIND_NODE_MAX];
                PrintListIndex++;
                r_num++;
                break;
            }
#else
            if(System_get_bindnode_DID_index(PrintListIndex) != 0){
                PrintListPage[i] = System_get_bindnode_DID_index(PrintListIndex%NETGATE_BIND_NODE_MAX);
                PrintListIndex++;
                r_num++;
                break;
            }
#endif
            PrintListIndex++;
        }
    }
    for(i = r_num; i < PRINT_LIST_PAGE_MAX; i++)
        PrintListPage[i] = 0;

#if 0 // LIST_PRINT_MODE_SINGLE
    //if(PrintListIndex == NETGATE_BIND_NODE_MAX){
        if(i <PRINT_LIST_PAGE_MAX -1)
            PrintListPage[i] = 0;
    //}
    if(PrintListIndex >= NETGATE_BIND_NODE_MAX)
        PrintListIndex = 0;
#else
    if((PrintListIndex >= NETGATE_BIND_NODE_MAX) && (r_num==0)){

        // Data forward
        for( i = 0; (i <PRINT_LIST_PAGE_MAX-1); i++)
        {
            PrintListPage[i] = PrintListPage[i+1];
        }
        PrintListPage[PRINT_LIST_PAGE_MAX-1] = 0;
        if(0 == PrintListPage[PRINT_LIST_PAGE_MAX-2])
            PrintListIndex = 0;
    }
    // Callback Function hook
    for( i = 0; (i <PRINT_LIST_PAGE_MAX); i++)
    {
        // Select Item callback
        if(PrintListPage[i] == 0){

            //rMenuObject.menu[i].func = PrintList_Print;
            rPrintMenu[i].func = PrintList_Print;
            if(i < PRINT_LIST_PAGE_MAX -1 ){
                //rMenuObject.menu[i+1].func = PrintList_Cancel;
                rPrintMenu[i+1].func = PrintList_Cancel;
                break;
            }
        }else{
            #if 0 //LIST_PRINT_MODE_SINGLE
            rPrintMenu[i].func = Menu_printList_proc;
            #else
            //rMenuObject.menu[i].func = PrintList_ItemSelect;
            rPrintMenu[i].func = PrintList_ItemSelect;
            #endif
        }
    }

#endif
}
void PrintList_Menu_show(void)
{
    uint16_t i=0;
    char strBuffer[30];

    uint8_t currIndex = rMenuObject.index;

    // Clear display
    Disp_clear_all();

    // display four list
    for(i = 0 ; i < PRINT_LIST_PAGE_MAX ; i++){


        if(rMenuObject.menu[i].func == PrintList_Print){
            //USE_ENGLISH_MENU
    #ifndef USE_ENGLISH_MENU
            Disp_picture(0, 2*i, 128, 16, PrintList_PrintImageStr);
    #else
            Disp_msg(2, 2*i, "Print", FONT_8X16);
    #endif
        }else if(rMenuObject.menu[i].func == PrintList_Cancel){
    #ifndef USE_ENGLISH_MENU
            Disp_picture(0, 2*i, 128, 16, PrintList_CancelImageStr);
    #else
            Disp_msg(2, 2*i, "End/Exit", FONT_8X16);
    #endif
        }

        else
        {
            memset(strBuffer, 0, sizeof(strBuffer));
            //MAC ID
            uint32_t CurrPrintDID = PrintListPage[i];

            if(0 == CurrPrintDID){

#if 0
                if(rMenuObject.index)
                    if(rMenuObject.menu[rMenuObject.index -1].func == PrintList_Cancel){
                        PrintList_getNext_Page();
                        rMenuObject.index = PRINT_LIST_PAGE_MAX+1;
                    }
#endif
                break;
            }
            //sprintf((char *)strBuffer, "DEID:%02x%02x%02x%02x", LOBYTE_ZKS(LOWORD_ZKS(CurrPrintDID)), HIBYTE_ZKS(LOWORD_ZKS(CurrPrintDID)),
              //                                                      LOBYTE_ZKS(HIWORD_ZKS(CurrPrintDID)), HIBYTE_ZKS(HIWORD_ZKS(CurrPrintDID)));
            sprintf((char *)strBuffer, "DEID:%02x%02x%02x%02x", HIBYTE_ZKS(HIWORD_ZKS(CurrPrintDID)),
                    LOBYTE_ZKS(HIWORD_ZKS(CurrPrintDID)), HIBYTE_ZKS(LOWORD_ZKS(CurrPrintDID)),LOBYTE_ZKS(LOWORD_ZKS(CurrPrintDID)));


            Disp_msg(1, 2*i, " ", FONT_8X16);
            Disp_msg(2, 2*i, (const uint8_t*)strBuffer, FONT_8X16);

            if(PrintList_isSelected(CurrPrintDID))
                PrintList_Select_sign_show(i);
            else
                PrintList_Cancel_sign_show(i);

        }
    }

    // display Select
    //Disp_msg(1, rMenuObject.index*2, "*", FONT_8X16);

    Disp_msg(1, currIndex*2, "*", FONT_8X16);
}

static void PrintList_Select_sign_show(uint8_t index)
{
    Disp_msg(0, 2*index, "+", FONT_8X16);
}
static void PrintList_Cancel_sign_show(uint8_t index)
{
    Disp_msg(0, 2*index, " ", FONT_8X16);
}
#endif // SUPPORT_BLE_PRINT_LIST_MODE

#ifdef SUPPORT_ALARM_RECORD_QURERY
//
//***********************************************************************************
void Alarm_record_Menu_show(void)
{
    char strBuffer[30];
    uint8_t buff[16]={0};
    int32_t value;

    // Clear display
    Disp_clear_all();
    if( alrmRecord.alrmCount == 0){
        sprintf((char *)strBuffer, "No Alarm !");
                  Disp_msg(3, 4, (uint8_t*)strBuffer, FONT_8X16);
                  return;
    }

    if( Flash_load_alarm_record_by_offset(buff,FLASH_ALARM_RECODRD_SIZE,alrmRecord.alrmindex ) == ES_SUCCESS ){

       sprintf((char *)strBuffer, "%02x-%02x-%02x %02x:%02x",buff[10],buff[11],buff[12],buff[13],buff[14]);
       Disp_msg(1, 0, (uint8_t*)strBuffer, FONT_8X16);
       sprintf((char *)strBuffer, "%02x%02x%02x%02x ",buff[3],buff[2],buff[1],buff[0]);
       Disp_msg(2, 2, (uint8_t*)strBuffer, FONT_8X16);

       HIBYTE_ZKS(HIWORD_ZKS(value)) = buff[9];
       LOBYTE_ZKS(HIWORD_ZKS(value)) = buff[8];
       HIBYTE_ZKS(LOWORD_ZKS(value)) = buff[7];
       LOBYTE_ZKS(LOWORD_ZKS(value)) = buff[6];

       switch(buff[5]){

       case SENSOR_DATA_TEMP:
           TempToDisplayBuff(value,strBuffer,buff[4]);
           Disp_msg(3, 4, (uint8_t*)strBuffer, FONT_8X16);
           break;
       case SENSOR_DATA_HUMI:
           sprintf((char*)strBuffer, "%d %02d%%",buff[4], (uint16_t)value/100);
           Disp_msg(3, 4, (uint8_t*)strBuffer, FONT_8X16);
           break;
       }




       sprintf((char *)strBuffer, "%d/%d",alrmRecord.alrmindex,alrmRecord.alrmCount);
       Disp_msg(10, 6, (uint8_t*)strBuffer, FONT_8X16);
    }
}
#endif

//***********************************************************************************
//
// Menu show function.
//
//***********************************************************************************
void Menu_show(void)
{
    if (rMenuObject.menu == rMainMenu){

#ifndef  USE_ENGLISH_MENU
    #if 0
        Disp_picture(0, 2, 128, 32, rMenuObject.menu[rMenuObject.index].string);
    #else
        if((rMenuObject.index == 0) || (rMenuObject.index == 1)){
            if(rMenuObject.startRecord==0 )
                Disp_picture(0, 2, 128, 32, rMenuObject.menu[MENU_128X32_START_RECORD].string);
            else
                Disp_picture(0, 2, 128, 32, rMenuObject.menu[MENU_128X32_STOP_RECORD].string);
            if(rMenuObject.index == 0)
                rMenuObject.index = MENU_128X32_STOP_RECORD;
        }else{
            Disp_picture(0, 2, 128, 32, rMenuObject.menu[rMenuObject.index].string);
        }
    #endif
#else
        Disp_clear_all();
    #if 0
        Disp_msg(0, 3, rMenuObject.menu[rMenuObject.index].string, FONT_8X16);
    #else
        if((rMenuObject.index == 0) || (rMenuObject.index == 1)){
            if(rMenuObject.startRecord==0 )
                //Disp_picture(0, 2, 128, 32, rMenuObject.menu[MENU_128X32_START_RECORD].string);
                Disp_msg(0, 3, rMenuObject.menu[MENU_128X32_START_RECORD].string, FONT_8X16);
            else
                //Disp_picture(0, 2, 128, 32, rMenuObject.menu[MENU_128X32_STOP_RECORD].string);
                Disp_msg(0, 3, rMenuObject.menu[MENU_128X32_STOP_RECORD].string, FONT_8X16);
            if(rMenuObject.index == 0)
                rMenuObject.index = MENU_128X32_STOP_RECORD;
        }else{
            //Disp_picture(0, 2, 128, 32, rMenuObject.menu[rMenuObject.index].string);
            Disp_msg(0, 3, rMenuObject.menu[rMenuObject.index].string, FONT_8X16);
        }
    #endif
#endif
    }
    else if(rMenuObject.menu == PoweroffMenu){
        
#ifndef  USE_ENGLISH_MENU

    #ifdef SUPPORT_FLIGHT_MODE
        Flight_Mode_Menu_show();
    #else
        #ifdef  SUPPORT_G7_PROTOCOL
        Disp_picture(0, 1, 128, 40, rMenuObject.menu[rMenuObject.index].string);
        Disp_msg(3, rMenuObject.index*2+1, "*", FONT_8X16);
        #else
        Disp_picture(0, 0, 128, 64, rMenuObject.menu[rMenuObject.index].string);
        Disp_msg(3, rMenuObject.index*2+1, "*", FONT_8X16);
        #endif
    #endif
#else
    #ifndef SUPPORT_FLIGHT_MODE
        uint8_t i=0;
        for(i=0; i < PoweroffMenu[0].count; ++i){
            Disp_msg(1, i*3, " ", FONT_8X16);            
            Disp_msg(4, i*3, rMenuObject.menu[i].string, FONT_8X16);
        }        
        Disp_msg(1, rMenuObject.index*3, "*", FONT_8X16);
    #else
        Flight_Mode_Menu_EN_show();
    #endif
#endif
    }
#ifdef SUPPORT_ALARM_RECORD_QURERY
    else if(rMenuObject.menu == rRecordArlmMenu){
            Alarm_record_Menu_show();
       }
#endif
#ifdef SUPPORT_BLE_PRINT_LIST_MODE
    else if(rMenuObject.menu == rPrintMenu){
        PrintList_Menu_show();
    }
#endif
}

//***********************************************************************************
//
// Menu start record process function.
//
//***********************************************************************************
void Menu_start_record(void)
{
    rMenuObject.startRecord = 1;
    g_bAlarmSensorFlag = 0;

#ifdef SUPPORT_G7_PROTOCOL
    BlePrintRecordStartNotify();
#endif  // SUPPORT_G7_PROTOCOL

#ifdef FLASH_EXTERNAL
    Flash_store_record_addr(1);
#endif

#ifdef  SUPPORT_DEVICED_STATE_UPLOAD
    Flash_store_devices_state(TYPE_RECORD_START);
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

    if(rMenuObject.startRecord)
    {
        rMenuObject.startRecord = 0;
#ifdef SUPPORT_G7_PROTOCOL
        BlePrintRecordStopNotify();
#endif  // SUPPORT_G7_PROTOCOL

#ifdef FLASH_EXTERNAL
        Flash_store_record_addr(0);

#endif

#ifdef  SUPPORT_DEVICED_STATE_UPLOAD
        Flash_store_devices_state(TYPE_RECORD_STOP);
#endif
    }
    Menu_exit();
}

#ifdef SUPPORT_ALARM_RECORD_QURERY
//***********************************************************************************
//
// Menu alrm query process function.
//
//***********************************************************************************
void Menu_alrm_query_proc(void){
    uint8_t buff[8];
    Flash_load_alarm_record_by_offset(buff,FLASH_ALARM_RECODRD_SIZE,alrmRecord.alrmindex);

}
#endif

//***********************************************************************************
//
// Menu alrm query process function display data.
//
//***********************************************************************************
void Menu_alarm_recode_proc(void)
{
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

    if (Btp_is_connect()) {
        //Bluetooth connect success
        Disp_picture(0, 2, 128, 32, &menu128x32[MENU_128X32_PRINTING * MENU_128X32_OFS]);
        Task_sleep(3 * CLOCK_UNIT_S);
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

#ifdef SUPPORT_BLE_PRINT_LIST_MODE

static void PrintList_ItemSelect(void)
{
    uint8_t currIndex = rMenuObject.index;
    // DECIMAL
    uint32_t DeID;
    //MAC ID
    DeID = PrintListPage[currIndex];

    PrintList_Select_insert_toggle( DeID);

#if LIST_PRINT_MODE_SINGLE
    PrintList_Print();
#endif

}
static void PrintList_Cancel(void)
{
    PrintList_Select_ClearAll();

    // Clear display
    Disp_clear_all();
    Menu_exit();
}

static void PrintList_Print(void)
{
    Menu_printList_proc();

    PrintList_Select_ClearAll();
}

static Menu_bluetooth_printList_proc(void)
{
#ifdef SUPPORT_BLUETOOTH_PRINT
    Btp_poweron();
#ifdef SUPPORT_G7_PROTOCOL
    if(Btp_scan_device_name() == ES_SUCCESS)
    {
        if (Btp_is_connect()) {
            //Bluetooth connect success
            Disp_picture(0, 2, 128, 32, &menu128x32[MENU_128X32_PRINTING * MENU_128X32_OFS]);
            Task_sleep(3 * CLOCK_UNIT_S);
            Btp_printList_record_bind_node();
        } else {
            //Bluetooth connect fail
            Disp_picture(0, 2, 128, 32, &menu128x32[MENU_128X32_CONNECT_FAIL * MENU_128X32_OFS]);
        }
    }
    else
    {
        //Bluetooth connect fail
        Disp_picture(0, 2, 128, 32, &menu128x32[MENU_128X32_CONNECT_FAIL * MENU_128X32_OFS]);
    }
#else

    if (Btp_is_connect()) {
        //Bluetooth connect success
        Disp_picture(0, 2, 128, 32, &menu128x32[MENU_128X32_PRINTING * MENU_128X32_OFS]);
        Task_sleep(3 * CLOCK_UNIT_S);
        Btp_printList_record_bind_node();
    } else {
        //Bluetooth connect fail
        Disp_picture(0, 2, 128, 32, &menu128x32[MENU_128X32_CONNECT_FAIL * MENU_128X32_OFS]);
    }
#endif

    Task_sleep(3 * CLOCK_UNIT_S);
    Btp_poweroff();
#endif
}

//***********************************************************************************
//
// Menu print List process function.
//
//***********************************************************************************
void Menu_printList_proc(void)
{
#ifdef SUPPORT_BLUETOOTH_PRINT

#ifdef SUPPORT_G7_PROTOCOL
    BlePrintingRecordNotify();
#endif  // SUPPORT_G7_PROTOCOL

    Sys_lcd_stop_timing();
    // Clear display
    Disp_clear_all();


#if 0
    CurrPrintDID = PrintListPage[rMenuObject.index];
    if(0 == PrintListPage[rMenuObject.index]){
        Disp_msg(4, 4, "Exit", FONT_8X16);
        // END/EXIT
        goto ExitPrint;
    }
    Disp_picture(0, 2, 128, 32, &menu128x32[MENU_128X32_CONNECTING * MENU_128X32_OFS]);
#else
    Disp_picture(0, 2, 128, 32, &menu128x32[MENU_128X32_CONNECTING * MENU_128X32_OFS]);
    // No Select special // Print all
    if(0 == PrintList_getSelectedNum()){
        //Disp_msg(1, 6, "All Print", FONT_8X16);
        Disp_msg(1, 6, "No Select", FONT_8X16);
        goto ExitPrint;
    }
    else
        Disp_msg(1, 6, "Select Print", FONT_8X16);

#endif

    // print
    Menu_bluetooth_printList_proc();

ExitPrint:
    Menu_exit();
    Sys_lcd_start_timing();
#endif
}
#endif //SUPPORT_BLE_PRINT_LIST_MODE

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


//***********************************************************************************
//
// Menu set menu store process flag.
//
//***********************************************************************************
void Menu_set_record(uint8_t flag)
{
    rMenuObject.startRecord = flag;
}



#endif  /* SUPPORT_MENU */
