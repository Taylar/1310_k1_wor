#ifndef			__SYSTEMAPP_H__
#define			__SYSTEMAPP_H__

#include "../general.h"

/***** Defines *****/
#define			SYSTEMAPP_EVT_KEY0						Event_Id_00
#define			SYSTEMAPP_EVT_INTERFACE					Event_Id_01
#define			SYSTEMAPP_EVT_RADIO_NODE				Event_Id_02
#define			SYSTEMAPP_EVT_RADIO_CONCENTER			Event_Id_03
#define			SYSTEMAPP_EVT_SENSOR					Event_Id_04
#define			SYSTEMAPP_EVT_RTC						Event_Id_05
#define			SYSTEMAPP_EVT_UPLOAD_NODE				Event_Id_06
#define			SYSTEMAPP_EVT_COLLECT_NODE				Event_Id_07
#define			SYSTEMAPP_EVT_STORE_CONCENTER			Event_Id_08
#define			SYSTEMAPP_EVT_NET_UPLOAD				Event_Id_09
#define			SYSTEMAPP_EVT_TIMER						Event_Id_10
#define			SYSTEMAPP_EVT_ALL						0xffff






extern Event_Handle systemAppEvtHandle;


/***** Prototypes *****/
void SysAppTaskCreate(void);



#endif		// __SYSTEMAPP_H__
