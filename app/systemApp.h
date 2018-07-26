#ifndef         __SYSTEMAPP_H__
#define         __SYSTEMAPP_H__

#include "../general.h"

/***** Defines *****/
#define         SYSTEMAPP_EVT_KEY0                      Event_Id_00
#define         SYSTEMAPP_EVT_KEY0_LONG                 Event_Id_01
#define         SYSTEMAPP_EVT_KEY0_DOUBLE               Event_Id_02
#define         SYSTEMAPP_EVT_KEY1                      Event_Id_03
#define         SYSTEMAPP_EVT_KEY1_LONG                 Event_Id_04
#define         SYSTEMAPP_EVT_KEY1_DOUBLE               Event_Id_05
#define         SYSTEMAPP_EVT_RTC                       Event_Id_06
#define         SYSTEMAPP_EVT_UPLOAD_NODE				Event_Id_07
#define         SYSTEMAPP_EVT_STORE_CONCENTER           Event_Id_08
#define         SYSTEMAPP_EVT_USBINT                    Event_Id_09
#define         SYSTEMAPP_EVT_RADIO_ABORT				Event_Id_10
#define         SYSTEMAPP_EVT_ALARM                     Event_Id_11
#define         SYSTEMAPP_EVT_STORE_SYS_CONFIG			Event_Id_12
#define         SYSTEMAPP_EVT_DISP                      Event_Id_13
#define         SYS_EVT_ALARM                      		Event_Id_14
#define         SYS_EVT_SENSOR                      	Event_Id_15
#define         SYS_FEED_WATCHDOG                      	Event_Id_16
#define         SYS_EVT_STRATEGY                      	Event_Id_17
#define         SYSTEMAPP_EVT_ALL                       0xffffffff

#ifdef SUPPORT_BOARD_OLD_S1
#define         SYS_EVT_EVT_OLD_S1_UPLOAD_NODE          Event_Id_16
#endif

#define         SYSTEMAPP_EVT_ALL_KEY                   (Event_Id_00 | Event_Id_01 | Event_Id_02 | Event_Id_03)


// test result info
extern uint8_t testResultInfo[32];
extern uint8_t testResultInfoLen;



extern Event_Handle systemAppEvtHandle;


/***** Prototypes *****/
void Sys_event_post(UInt event);

void SysAppTaskCreate(void);

void SystemUsbIntEventPostIsr(void);

void SystemKeyEventPostIsr(void);

void SystemLongKeyEventPostIsr(void);

void SystemDoubleKeyEventPostIsr(void);

void SystemKey1EventPostIsr(void);

void SystemLongKey1EventPostIsr(void);

void SystemUsbIntEventPostIsr(void);

#endif      // __SYSTEMAPP_H__
