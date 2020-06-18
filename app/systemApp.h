#ifndef         __SYSTEMAPP_H__
#define         __SYSTEMAPP_H__

#include "../general.h"

/***** Defines *****/
#define         SYSTEMAPP_EVT_KEY                       Event_Id_00
#define         SYSTEMAPP_EVT_KEY_LONG                  Event_Id_01
#define         SYSTEMAPP_EVT_KEY_DOUBLE                Event_Id_02
#define         SYSTEMAPP_EVT_RTC_READ         			Event_Id_03
#define         SYSTEMAPP_EVT_STORE_CONCENTER           Event_Id_04
#define         SYSTEMAPP_EVT_USBINT                    Event_Id_05
#define         SYSTEMAPP_EVT_RADIO_ABORT				Event_Id_06
#define         SYS_EVT_ALARM_SAVE                      Event_Id_07
#define         SYSTEMAPP_EVT_STORE_SYS_CONFIG			Event_Id_08
#define         SYSTEMAPP_EVT_DISP                      Event_Id_09
#define         SYS_EVT_ALARM                      		Event_Id_10
#define         SYS_EVT_RTC                      		Event_Id_11
#define         SYS_FEED_WATCHDOG                      	Event_Id_12
#define         SYS_EVT_USB_CHARGE                      Event_Id_13
#define         SYS_EVT_CONFIG_MODE_EXIT                Event_Id_14
#define 		SYS_EVT_KEY_SCAN						Event_Id_15
#define 		SYS_EVT_ELE_SHOCK_DESTROY				Event_Id_16
#define 		SYS_EVT_MOTO_INT_REC					Event_Id_17
#define 		SYS_EVT_SOUND_PLAY						Event_Id_18

#define         SYSTEMAPP_EVT_ALL                       0xffffffff

#define         SYSTEMAPP_EVT_ALL_KEY                   (Event_Id_00)


// test result info
extern uint8_t testResultInfo[32];
extern uint8_t testResultInfoLen;



extern Event_Handle systemAppEvtHandle;


/***** Prototypes *****/
void Sys_event_post(UInt event);

void SysAppTaskCreate(void);

void SystemUsbIntEventPostIsr(void);

void SystemLongKeyEventPostIsr(void);

void SystemDoubleKeyEventPostIsr(void);

void SystemKey1EventPostIsr(void);

void SystemLongKey1EventPostIsr(void);

void SystemLongKey0EventPostIsr(void);

void SystemUsbIntEventPostIsr(void);

uint32_t RandomDataGenerate(void);

void SystemResetAndSaveRtc(void);

uint32_t RandomDataGenerate_Software(void);

#endif      // __SYSTEMAPP_H__
