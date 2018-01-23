#include "../general.h"
#include "../app/systemApp.h"
#include "../app/nodeApp.h"
#include "../app/concenterApp.h"

/***** Defines *****/
#define         SYSTEM_APP_STACK_SIZE        768

// **************************************************************************


// /* Clock for the fast report timeout */

// static Clock_Struct sysTimerClock;     /* not static so you can see in ROV */

// static Clock_Handle sysTimerClockHandle;


// **************************************************************************
/***** Variable declarations *****/
// task
uint8_t         systemAppTaskStack[SYSTEM_APP_STACK_SIZE];
Task_Struct     systemAppTaskStruct;


// event 
Event_Struct systemAppEvtStruct;
Event_Handle systemAppEvtHandle;

// system application extern variable
uint8_t powerMode;



/***** Prototypes *****/
void SystemAppTaskFxn(void);


/***** Function definitions *****/
void SystemKeyEventPostIsr(void)
{
    Event_post(systemAppEvtHandle, SYSTEMAPP_EVT_KEY0);
}

void SystemLongKeyEventPostIsr(void)
{
    Event_post(systemAppEvtHandle, SYSTEMAPP_EVT_KEY0_LONG);
}

void SysTimerCb(UArg arg0)
{
    Event_post(systemAppEvtHandle, SYSTEMAPP_EVT_TIMER);
}



void RtcEventSet(void)
{
    Event_post(systemAppEvtHandle, SYSTEMAPP_EVT_RTC);
}

void SysAppTaskCreate(void)
{
    Error_Block eb;

    Error_init(&eb);

	/* Construct system application Task threads */
    Task_Params taskParams;
    Task_Params_init(&taskParams);
    taskParams.stackSize = SYSTEM_APP_STACK_SIZE;
    taskParams.stack = &systemAppTaskStack;
    taskParams.priority = 1;
    Task_construct(&systemAppTaskStruct, (Task_FuncPtr)SystemAppTaskFxn, &taskParams, &eb);

}


void Disp_info(void);
void SystemAppTaskFxn(void)
{
    uint32_t    eventId;
    // uint32_t	voltageTemp;
	/* Construct system process Event */
	Event_construct(&systemAppEvtStruct, NULL);
	/* Obtain event instance handle */
	systemAppEvtHandle = Event_handle(&systemAppEvtStruct);

#ifdef  BOARD_S6_6
    Disp_init();
    Disp_poweron();
    Disp_info();
#endif

#ifdef BOARD_S2_2
	ConcenterAppHwInit();
#endif

#ifdef BOARD_S1_2
   	// NodeAppHwInit();
#endif


    powerMode = DEVICES_POWER_OFF;

    KeyInit();
    KeyRegister(SystemKeyEventPostIsr, KEY_0_SHORT_PRESS);

    KeyRegister(SystemLongKeyEventPostIsr, KEY_0_LONG_PRESS);

	RtcInit(RtcEventSet);

	RtcStart();

    
	// voltageTemp = Clock_getTicks();
	// Clock_setTimeout(sysTimerClockHandle, 0);
	// Clock_start(sysTimerClockHandle);


	for(;;)
	{
        eventId = Event_pend(systemAppEvtHandle, 0, SYSTEMAPP_EVT_ALL, BIOS_WAIT_FOREVER);
	

        if(eventId &SYSTEMAPP_EVT_RADIO_NODE)
		{

		}

		if(eventId &SYSTEMAPP_EVT_RADIO_CONCENTER)
		{

		}



		if(eventId &SYSTEMAPP_EVT_KEY0)
		{
#ifdef BOARD_S2_2
			ConcenterShortKeyApp();
#endif


#ifdef BOARD_S1_2
			NodeShortKeyApp();
#endif
		}


		if(eventId & SYSTEMAPP_EVT_KEY0_LONG)
		{

#ifdef BOARD_S2_2
			ConcenterLongKeyApp();
#endif


#ifdef BOARD_S1_2
			NodeLongKeyApp();
#endif

		}



		if(eventId &SYSTEMAPP_EVT_INTERFACE)
		{

		}

		

		if(eventId &SYSTEMAPP_EVT_SENSOR)
		{

		}

		if(eventId &SYSTEMAPP_EVT_RTC)
		{

			// Led_toggle(LED_R);
			// Led_toggle(LED_B);
			// Led_toggle(LED_G);
			// voltageTemp = AONBatMonBatteryVoltageGet();
			// voltageTemp = ((voltageTemp&0xff00)>>8)*1000 +1000*(voltageTemp&0xff)/256;
			// System_printf("voltage: %d mV\n", voltageTemp);
			// SHT2X_FxnTable.measureFxn(SHT2X_I2C_CH0);

			// DeepTemp_FxnTable.measureFxn(MAX31855_SPI_CH0);
			// System_printf("the temperature : %d\n", DeepTemp_FxnTable.getValueFxn(MAX31855_SPI_CH0, SENSOR_DEEP_TEMP)/256);
		}

		if(eventId & SYSTEMAPP_EVT_UPLOAD_NODE)
		{
			NodeUploadProcess();
		}


		if(eventId & SYSTEMAPP_EVT_COLLECT_NODE)
		{
			NodeCollectProcess();
		}


		if(eventId & SYSTEMAPP_EVT_NET_UPLOAD)
		{
			Nwk_upload_data();
		}

		if(eventId & SYSTEMAPP_EVT_STORE_CONCENTER)
		{
			ConcenterSensorDataSave();
		}


		if(eventId & SYSTEMAPP_EVT_TIMER)
		{
			// System_printf("start time: %d \n", voltageTemp);
			// System_printf("period time: %d \n", b);
		}

	}
}

