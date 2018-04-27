#ifndef         __SYSTEMAPP_H__
#define         __SYSTEMAPP_H__

#include "../general.h"

/***** Defines *****/
#define         SYSTEMAPP_EVT_KEY0                      Event_Id_00
#define         SYSTEMAPP_EVT_KEY0_LONG                 Event_Id_01
#define         SYSTEMAPP_EVT_KEY1                      Event_Id_02
#define         SYSTEMAPP_EVT_KEY1_LONG                 Event_Id_03
#define         SYSTEMAPP_EVT_RTC                       Event_Id_04
#define         SYSTEMAPP_EVT_UPLOAD_NODE				Event_Id_05
#define         SYSTEMAPP_EVT_STORE_CONCENTER           Event_Id_06
#define         SYSTEMAPP_EVT_USBINT                    Event_Id_07
#define         SYSTEMAPP_EVT_DISP                      Event_Id_08
#define         SYSTEMAPP_EVT_ALL                       0xffffffff



// test result info
extern uint8_t testResultInfo[32];
extern uint8_t testResultInfoLen;



extern Event_Handle systemAppEvtHandle;


/***** Prototypes *****/
void SystemEventSet(UInt event);

void SysAppTaskCreate(void);

void SystemUsbIntEventPostIsr(void);

void SystemKeyEventPostIsr(void);

void SystemLongKeyEventPostIsr(void);

void SystemKey1EventPostIsr(void);

void SystemLongKey1EventPostIsr(void);

void SystemUsbIntEventPostIsr(void);

#endif      // __SYSTEMAPP_H__
