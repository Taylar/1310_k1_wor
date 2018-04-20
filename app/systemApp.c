#include "../general.h"
#include "../app/systemApp.h"
#include "../app/nodeApp.h"
#include "../app/concenterApp.h"

/***** Defines *****/
#define         SYSTEM_APP_STACK_SIZE        1024




// **************************************************************************
/***** Variable declarations *****/
// task
uint8_t         systemAppTaskStack[SYSTEM_APP_STACK_SIZE];
Task_Struct     systemAppTaskStruct;


// event 
Event_Struct systemAppEvtStruct;
Event_Handle systemAppEvtHandle;

// system application extern variable
uint8_t deviceMode;

// test result info
uint8_t testResultInfo[32];
uint8_t testResultInfoLen;

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

void SystemKey1EventPostIsr(void)
{
    Event_post(systemAppEvtHandle, SYSTEMAPP_EVT_KEY1);
}

void SystemLongKey1EventPostIsr(void)
{
    Event_post(systemAppEvtHandle, SYSTEMAPP_EVT_KEY1_LONG);
}

void SystemUsbIntEventPostIsr(void)
{
    Event_post(systemAppEvtHandle, SYSTEMAPP_EVT_USBINT);
}


void SysTimerCb(UArg arg0)
{
    Event_post(systemAppEvtHandle, SYSTEMAPP_EVT_TIMER);
}


void WdtResetCb(uintptr_t handle)
{

	// call this function will reset immediately, otherwise will waite another wdt timeout to reset
	SysCtrlSystemReset();
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


void SystemAppTaskFxn(void)
{
    uint32_t    eventId;
    // uint32_t	voltageTemp;

    Event_Params eventParam;
    Event_Params_init(&eventParam);
	/* Construct system process Event */
	Event_construct(&systemAppEvtStruct, &eventParam);
	/* Obtain event instance handle */
	systemAppEvtHandle = Event_handle(&systemAppEvtStruct);

    deviceMode = DEVICES_OFF_MODE;

	RtcInit(RtcEventSet);

#ifdef  BOARD_S6_6
	S6HwInit();
#endif

#ifdef BOARD_S2_2
    S2HwInit();
#endif

#ifdef BOARD_S1_2
   	S1HwInit();
#endif

	Sensor_init();

	RtcStart();

#ifdef		SUPPORT_WATCHDOG
	WdtInit(WdtResetCb);
#endif

#ifdef BOARD_CONFIG_DECEIVE
	ConcenterConfigDeceiveInit();
#endif


#if (defined BOARD_S6_6) || (defined BOARD_S2_2)
#ifndef  BOARD_CONFIG_DECEIVE
	ConcenterWakeup();
#endif
#endif



	for(;;)
	{
        eventId = Event_pend(systemAppEvtHandle, 0, SYSTEMAPP_EVT_ALL, BIOS_WAIT_FOREVER);
	

        if(eventId &SYSTEMAPP_EVT_RADIO_NODE)
		{

		}

		if(eventId &SYSTEMAPP_EVT_RADIO_CONCENTER)
		{

		}

// the config deceive key is disable
#ifndef   BOARD_CONFIG_DECEIVE

		if(eventId &SYSTEMAPP_EVT_KEY0)
		{
#ifdef BOARD_S2_2
			S2ShortKeyApp();
#endif


#ifdef BOARD_S1_2
			S1ShortKeyApp();
#endif

#ifdef BOARD_S6_6
			S6ShortKeyApp();
#endif
		}


		if(eventId & SYSTEMAPP_EVT_KEY0_LONG)
		{

#ifdef BOARD_S2_2
			S2LongKeyApp();
#endif


#ifdef BOARD_S1_2
			S1LongKeyApp();
#endif

#ifdef BOARD_S6_6
			// S6ConcenterLongKeyApp();
#endif

		}


#ifdef BOARD_S6_6
		if(eventId & SYSTEMAPP_EVT_KEY1)
		{
			S6ShortKey1App();
		}

		if(eventId & SYSTEMAPP_EVT_KEY1_LONG)
		{
			S6LongKey1App();
		}
#endif

#endif


		if(eventId &SYSTEMAPP_EVT_INTERFACE)
		{

		}

		

		if(eventId &SYSTEMAPP_EVT_SENSOR)
		{

		}

		if(eventId &SYSTEMAPP_EVT_RTC)
		{
#ifdef		SUPPORT_WATCHDOG
			WdtClear();
#endif	


#ifdef BOARD_S2_2
			ConcenterRtcProcess();
#endif

#ifdef BOARD_S6_6
			ConcenterRtcProcess();
			S6AppRtcProcess();
#endif

#ifdef BOARD_S1_2
			S1AppRtcProcess();
			NodeRtcProcess();
#endif
			// Led_toggle(LED_R);
			// Led_toggle(LED_B);
			// Led_toggle(LED_G);
			// voltageTemp = AONBatMonBatteryVoltageGet();
			// voltageTemp = ((voltageTemp&0xff00)>>8)*1000 +1000*(voltageTemp&0xff)/256;
			// System_printf("voltage: %d mV\n", voltageTemp);
			// SHT2X_FxnTable.measureFxn(SHT2X_I2C_CH0);

			// DeepTemp_FxnTable.measureFxn(MAX31855_SPI_CH0);
			// System_printf("the temperature : %d\n", DeepTemp_FxnTable.getValueFxn(MAX31855_SPI_CH0, SENSOR_DEEP_TEMP)/256);

			// Battery_voltage_measure();
			// System_printf("the voltage : %dmV\n",Battery_get_voltage());

			// NTC_FxnTable.measureFxn(NTC_CH0);
			// System_printf("the Ntc Temp : %dm C\n",NTC_FxnTable.getValueFxn(NTC_CH0, SENSOR_TEMP));
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

		}

		if(eventId & SYSTEMAPP_EVT_STORE_CONCENTER)
		{
			ConcenterSensorDataSave();
		}

		if(eventId & SYSTEMAPP_EVT_USBINT)
		{
			UsbIntProcess();
		}

		if(eventId & SYSTEMAPP_EVT_TIMER)
		{
		}

	}
}

