#include "../general.h"


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

//***********************************************************************************
//
// System event post.
//
//***********************************************************************************
void Sys_event_post(UInt event)
{
    Event_post(systemAppEvtHandle, event);
}


void SystemKeyEventPostIsr(void)
{
    Event_post(systemAppEvtHandle, SYSTEMAPP_EVT_KEY0);
}

void SystemLongKeyEventPostIsr(void)
{
    Event_post(systemAppEvtHandle, SYSTEMAPP_EVT_KEY0_LONG);
}

void SystemDoubleKeyEventPostIsr(void)
{
    Event_post(systemAppEvtHandle, SYSTEMAPP_EVT_KEY0_DOUBLE);
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


void WdtResetCb(uintptr_t handle)
{

#ifdef SUPPORT_BOARD_OLD_S1
        g_rSysConfigInfo.rtc =Rtc_get_calendar();
        Flash_store_config();
#endif
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

//#define BOARD_S6_6
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

//	RtcInit(RtcEventSet);

#ifdef  BOARD_S6_6
	S6HwInit();
#endif

#ifdef BOARD_S2_2
    S2HwInit();
#endif

#ifdef BOARD_S1_2
   	S1HwInit();
#endif
   	RtcInit(RtcEventSet);
	Sensor_init();

	RtcStart();

    Task_sleep(10 * CLOCK_UNIT_MS);

#ifdef BOARD_CONFIG_DECEIVE
	ConcenterConfigDeceiveInit();
#endif

	Sensor_measure(0);



#ifdef 		BOARD_S6_6

#ifdef SUPPORT_ENGMODE
    if (GetEngModeFlag())
    {
        SetEngModeConfig();
 		EngMode();
    }
#endif  // SUPPORT_ENGMODE

#ifndef  BOARD_CONFIG_DECEIVE

	if(Battery_get_voltage() > BAT_VOLTAGE_LOW)
		ConcenterWakeup();
	else
		ConcenterSleep();

#endif // BOARD_CONFIG_DECEIVE

#endif // BOARD_S6_6

#ifdef  SUPPORT_DEVICED_STATE_UPLOAD
			Flash_store_devices_state(TYPE_POWER_ON);
#endif // SUPPORT_DEVICED_STATE_UPLOAD

#ifdef		SUPPORT_WATCHDOG
	WdtInit(WdtResetCb);
#endif

	for(;;)
	{
        eventId = Event_pend(systemAppEvtHandle, 0, SYSTEMAPP_EVT_ALL, BIOS_WAIT_FOREVER);
	

// the config deceive key is disable
#ifndef   BOARD_CONFIG_DECEIVE

		if(eventId &SYSTEMAPP_EVT_KEY0)
		{

#ifdef BOARD_S1_2
			S1ShortKeyApp();
#endif

#ifdef BOARD_S6_6
			S6ShortKeyApp();
#endif
		}


		if(eventId & SYSTEMAPP_EVT_KEY0_LONG)
		{

#ifdef BOARD_S1_2
			S1LongKeyApp();
#endif

#ifdef BOARD_S6_6
			// S6ConcenterLongKeyApp();
#endif

		}

		if(eventId & SYSTEMAPP_EVT_KEY0_DOUBLE)
		{

#ifdef BOARD_S1_2
			S1DoubleKeyApp();
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


		


		if(eventId &SYSTEMAPP_EVT_RTC)
		{
#ifdef		SUPPORT_WATCHDOG
			WdtClear();
#endif	


#ifdef BOARD_S6_6
			if(!(g_rSysConfigInfo.rfStatus & STATUS_1310_MASTER))
			{
				NodeRtcProcess();
			}
			else
			{
				ConcenterRtcProcess();
			}
			// S6AppRtcProcess();
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


		if(eventId & SYSTEMAPP_EVT_STORE_CONCENTER)
		{
			ConcenterSensorDataSave();
		}

		if(eventId & SYSTEMAPP_EVT_USBINT)
		{
			UsbIntProcess();
		}

#ifdef BOARD_S6_6
		if(eventId & SYSTEMAPP_EVT_ALARM)
		{
			buzzerAlarmCnt = 2;
			Sys_buzzer_enable();
            Clock_setPeriod(sysAlarmClkHandle, 500*CLOCK_UNIT_MS);//500MS
            Clock_start(sysAlarmClkHandle);
            Disp_info_close();
            Disp_poweron();
		}
#endif


		if(eventId & SYSTEMAPP_EVT_STORE_SYS_CONFIG)
		{

		}

		if(eventId & SYSTEMAPP_EVT_ALARM)
		{

		}


#ifdef SUPPORT_DISP_SCREEN
		if(eventId & SYSTEMAPP_EVT_DISP)
		{
        	Disp_proc();
		}
#endif

#ifdef SUPPORT_BOARD_OLD_S1
		if(eventId &   SYS_EVT_EVT_OLD_S1_UPLOAD_NODE) {
		    OldS1NodeAPP_Mode2NodeUploadProcess();
		}
#endif
	}
}

