#include "../general.h"

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/aon_batmon.h)
#include DeviceFamily_constructPath(driverlib/trng.h)

/***** Defines *****/
#define         SYSTEM_APP_STACK_SIZE        1500

// **************************************************************************
/***** Variable declarations *****/
// task
uint8_t         systemAppTaskStack[SYSTEM_APP_STACK_SIZE];
Task_Struct     systemAppTaskStruct;


// event 
Event_Struct systemAppEvtStruct;
Event_Handle systemAppEvtHandle;

// radom Semaphore
Semaphore_Struct radomFuncSem;  /* not static so you can see in ROV */
Semaphore_Handle radomFuncSemHandle;


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

void SystemLongKey0EventPostIsr(void)
{
    Event_post(systemAppEvtHandle, SYSTEMAPP_EVT_KEY0_LONG);
}

void SystemUsbIntEventPostIsr(void)
{
    Event_post(systemAppEvtHandle, SYSTEMAPP_EVT_USBINT);
}

void WdtResetCb(uintptr_t handle)
{
	// UInt hwiKey;
	// // hwiKey = Hwi_disable();	

 //    hwiKey = Hwi_enable();
 //    Flash_log("WDT\n");
	// // call this function will reset immediately, otherwise will waite another wdt timeout to reset
	// g_rSysConfigInfo.sysState.err_restarts ++;
 //    Flash_store_config();
 //    Hwi_restore(hwiKey);
    while(1)
        HWREGBITW( PRCM_BASE + PRCM_O_WARMRESET, PRCM_WARMRESET_WR_TO_PINRESET_BITN ) = 1;
}

void RtcEventSet(void)
{

#ifdef SUPPORT_SENSOR
    Sensor_collect_time_isr();
#endif // SUPPORT_SENSOR

	Nwk_upload_time_isr();

#if defined(SUPPORT_BOARD_OLD_S1) || defined(SUPPORT_BOARD_OLD_S2S_1)
	OldS1NodeApp_RtcIProcess();
#endif

#ifdef BOARD_S3
	S1AppRtcProcess();
#endif

#ifdef S_C//节点
	NodeRtcProcess();
#endif // S_C//节点

#ifdef S_G//网关
	ConcenterRtcProcess();
#endif // S_G//网关

#ifdef  SUPPORT_CHARGE_DECT_ALARM
    Sys_chagre_alarm_timer_isr();
#endif
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

//***********************************************************************************
// brief:   generate the random num, this maybe waste almost 1 sec
// 
// parameter: 
// ret: the randdom Num
//***********************************************************************************
uint32_t RandomDataGenerate(void)
{
    uint32_t randomNum;

    // gennerate a ramdom num maybe waste almost 1sec
    /* Use the True Random Number Generator to generate sensor node address randomly */
    Semaphore_pend(radomFuncSemHandle, BIOS_WAIT_FOREVER);
    Power_setDependency(PowerCC26XX_PERIPH_TRNG);
    TRNGEnable();
    while (!(TRNGStatusGet() & TRNG_NUMBER_READY))
    {
        //wiat for randum number generator
    }
    randomNum = TRNGNumberGet(TRNG_LOW_WORD);

    while (!(TRNGStatusGet() & TRNG_NUMBER_READY))
    {
        //wiat for randum number generator
    }

    randomNum |= ((uint32_t)TRNGNumberGet(TRNG_HI_WORD)) << 16;

    TRNGDisable();
    Power_releaseDependency(PowerCC26XX_PERIPH_TRNG);
    Semaphore_post(radomFuncSemHandle);
    return randomNum;
}

uint32_t RandomDataGenerate_Software(void)
{
	return rand();
}



//#define BOARD_S6_6
void SystemAppTaskFxn(void)
{
    uint32_t    eventId;
#ifdef SUPPORT_ENGMODE
    uint8_t     engmodeFlag = 0;
#endif // SUPPORT_ENGMODE
    // uint32_t	voltageTemp;
    Semaphore_Params semParam;
    Semaphore_Params_init(&semParam);
    Semaphore_construct(&radomFuncSem, 1, &semParam);
    radomFuncSemHandle = Semaphore_handle(&radomFuncSem); 

    Event_Params eventParam;
    Event_Params_init(&eventParam);
	/* Construct system process Event */
	Event_construct(&systemAppEvtStruct, &eventParam);
	/* Obtain event instance handle */
	systemAppEvtHandle = Event_handle(&systemAppEvtStruct);

    deviceMode = DEVICES_OFF_MODE;

	// RtcInit(RtcEventSet);

#ifdef  BOARD_S6_6
	S6HwInit();
#endif

#ifdef BOARD_B2S
    S2HwInit();
    Led_ctrl2(LED_B, 1, 200 * CLOCK_UNIT_MS, 800 * CLOCK_UNIT_MS, 3);
#endif

#ifdef BOARD_S3
   	S1HwInit();
#endif
   	RtcInit(RtcEventSet);

#ifdef SUPPORT_SENSOR
	Sensor_init();
#endif // SUPPORT_SENSOR

    Task_sleep(10 * CLOCK_UNIT_MS);

#if defined(BOARD_CONFIG_DECEIVE) || defined(SUPPORT_BOARD_OLD_S1)|| defined(SUPPORT_BOARD_OLD_S2S_1)
    RtcStart();
#endif

#ifdef BOARD_CONFIG_DECEIVE
	ConcenterConfigDeceiveInit();
#endif

#ifdef SUPPORT_SENSOR
	// Sensor_measure(0);
#endif


#if (defined BOARD_S6_6 || defined BOARD_B2S)

#ifdef SUPPORT_ENGMODE
    if (GetEngModeFlag())
    {
 		EngMode();
 		engmodeFlag = 1;
    }
#endif  // SUPPORT_ENGMODE

#ifndef  BOARD_CONFIG_DECEIVE

#ifdef BOARD_S6_6		
	if((Battery_get_voltage() > BAT_VOLTAGE_LOW) && (engmodeFlag == 0))
		S6Wakeup();
	else
		S6Sleep();
#endif // BOARD_S6_6

#ifdef BOARD_B2S
	if(Battery_get_voltage() > BAT_VOLTAGE_LOW)
		S2Wakeup();
	else
		S2Sleep();
#endif // BOARD_B2S

#endif // BOARD_CONFIG_DECEIVE

#endif // defined BOARD_S6_6 || defined BOARD_B2S

#ifdef  SUPPORT_DEVICED_STATE_UPLOAD
			Flash_store_devices_state(TYPE_POWER_ON);
#endif // SUPPORT_DEVICED_STATE_UPLOAD

#ifdef		SUPPORT_WATCHDOG
    #if (defined(BOARD_S6_6) ||  defined(BOARD_B2S))
	    WdtInit(WdtResetCb);
    #endif
#endif //SUPPORT_WATCHDOG

#ifdef BOARD_S3
	if(g_rSysConfigInfo.sysState.wtd_restarts & STATUS_POWERON)
	{
	    Task_sleep(100 * CLOCK_UNIT_MS);
		S1Wakeup();
#ifndef SUPPORT_BOARD_OLD_S1
		Sys_event_post(SYS_EVT_SENSOR);
#endif
	}
#endif // (defined BOARD_S6_6 || defined BOARD_B2S)


	for(;;)
	{
        eventId = Event_pend(systemAppEvtHandle, 0, SYSTEMAPP_EVT_ALL, BIOS_WAIT_FOREVER);
	

#ifdef		SUPPORT_WATCHDOG
        if(eventId & SYS_FEED_WATCHDOG)
        {
			WdtClear();
        }
#endif	

// the config deceive key is disable
#ifdef   BOARD_CONFIG_DECEIVE
        if(eventId & (SYSTEMAPP_EVT_KEY0_LONG | SYSTEMAPP_EVT_KEY1_LONG))
        {
        	Led_ctrl(LED_R, 1, 500 * CLOCK_UNIT_MS, 1);
	        Task_sleep(750 * CLOCK_UNIT_MS);
	        while(1)
	        	SysCtrlSystemReset();
        }

        if(eventId &(SYSTEMAPP_EVT_KEY0 | SYSTEMAPP_EVT_KEY1))
        {
        	Led_ctrl(LED_B, 1, 500 * CLOCK_UNIT_MS, 1);
        }

#else
		if(eventId &SYSTEMAPP_EVT_KEY0)
		{

#ifdef BOARD_S3
			S1ShortKeyApp();
#endif

#ifdef BOARD_S6_6
			S6ShortKeyApp();
#endif

#ifdef BOARD_B2S
            S2ShortKeyApp();
#endif
		}


		if(eventId & SYSTEMAPP_EVT_KEY0_LONG)
		{

#ifdef BOARD_S3
			S1LongKeyApp();
#endif

#ifdef BOARD_S6_6
			// S6ConcenterLongKeyApp();
#endif

#ifdef BOARD_B2S
			S2LongKeyApp();
#endif
		}

		if(eventId & SYSTEMAPP_EVT_KEY0_DOUBLE)
		{

#ifdef BOARD_S3
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

		if(eventId & SYSTEMAPP_EVT_KEY0_LONG)
		{
			S6LongKey0App();
		}
#endif

#endif

		if(eventId & SYSTEMAPP_EVT_CONCENTER_MONITER)
		{
			ConcenterResetRadioState();
		}

		if(eventId & SYS_EVT_CONFIG_MODE_EXIT)
		{
			S1AppConfigModeExit();
		}

		if(eventId & SYSTEMAPP_EVT_STORE_CONCENTER)
		{
			ConcenterSensorDataSave();
		}
#ifdef SUPPORT_USB
		if(eventId & SYSTEMAPP_EVT_USBINT)
		{
			UsbIntProcess();
		}
#endif // SUPPORT_USB

#ifdef BOARD_S6_6
		S6AppBatProcess();
#ifndef BOARD_CONFIG_DECEIVE
		if(eventId & SYS_EVT_ALARM)
		{
			if (!(g_rSysConfigInfo.status & STATUS_ALARM_OFF)) {
                buzzerAlarmCnt = 2;
                Sys_buzzer_enable();
                Clock_setPeriod(sysAlarmClkHandle, 500*CLOCK_UNIT_MS);//500MS
                Clock_start(sysAlarmClkHandle);
                Disp_info_close();
                Disp_poweron();
            }
		}
#endif  // BOARD_CONFIG_DECEIVE

#ifdef SUPPORT_ALARM_RECORD_QURERY
      	if(eventId & SYS_EVT_ALARM_SAVE)
      	{
          	Flash_store_alarm_record((uint8_t*)(&g_AlarmSensor),sizeof(Alarmdata_t));
        }

#endif  // SUPPORT_ALARM_RECORD_QURERY

#endif  // BOARD_S6_6

		if(eventId & SYSTEMAPP_EVT_STORE_SYS_CONFIG)
		{
			// Flash_store_config();
		}


//#if defined(SUPPORT_BOARD_OLD_S1) || defined(SUPPORT_BOARD_OLD_S2S_1)
//		if(eventId &   SYS_EVT_EVT_OLD_S1_UPLOAD_NODE) {
//		    OldS1NodeAPP_Mode2NodeUploadProcess();
//		}
//#endif

		if(eventId & SYS_EVT_STRATEGY) 
		{
			if(GetStrategyRegisterStatus() == false)
			{
				NodeStrategyTimeoutProcess();
				RadioSend();
			}
		}

		if(eventId & SYSTEMAPP_EVT_RADIO_ABORT) 
		{
			if(ExtflashRingQueueIsEmpty((&extflashWriteQ)))
			{
				Flash_log("TX Tout R\n");
		        SystemResetAndSaveRtc();
			}
			else
			{
				Sys_event_post(SYSTEMAPP_EVT_RADIO_ABORT);
				Sys_event_post(SYSTEMAPP_EVT_STORE_CONCENTER);
			}
		}

#ifdef SUPPORT_SENSOR
        if (eventId & SYS_EVT_SENSOR) {
#ifdef SUPPORT_SENSOR_ADJUST
            Sensor_process();
#else
#ifdef 	BOARD_S3
      //       if(radioAccessSemHandle)
      //       {
	     //        if(Semaphore_pend(radioAccessSemHandle, 4000 * CLOCK_UNIT_MS) == FALSE)
	     //        {
		    //         Sys_event_post(SYS_EVT_SENSOR);
	     //        }
	     //        else
	     //        {
	     //        	Sensor_measure(1);
	     //        	Semaphore_post(radioAccessSemHandle);
	     //        }
      //       }
	    	// else
	    	// {
	     //        Sensor_measure(1);
	    	// }
#else    
            Sensor_measure(1);
#endif  //BOARD_S3

#endif
            Battery_porcess();

#ifdef      SUPPORT_UPLOADTIME_LIMIT
            if((g_rSysConfigInfo.uploadPeriod/60) < Flash_get_unupload_items()){
                Nwk_upload_set();
            }
#endif      // SUPPORT_UPLOADTIME_LIMIT

#ifdef  S_C
            if ((deviceMode != DEVICES_OFF_MODE) && (deviceMode != DEVICES_CONFIG_MODE))
	        {
#if !defined(SUPPORT_BOARD_OLD_S1) && !defined(SUPPORT_BOARD_OLD_S2S_1)
	            RadioSensorDataPack();
#endif //!defined(SUPPORT_BOARD_OLD_S1) && !defined(SUPPORT_BOARD_OLD_S2S_1)
	        }
#endif // S_C

        }
#endif // SUPPORT_SENSOR


         //=======================================
#ifdef SUPPORT_BLUETOOTH_PRINT
        if (eventId & SYS_EVT_PRINT_CONTINU){
            Btp_print_record();
        }
#endif

#ifdef SUPPORT_DISP_SCREEN
		if(eventId & SYSTEMAPP_EVT_DISP)
		{
        	Disp_proc();
		}
#endif

#if defined(SUPPORT_BOARD_OLD_S1) || defined(SUPPORT_BOARD_OLD_S2S_1)
        if(eventId & SYS_EVT_S1_SENSOR)
        {
            Battery_porcess();
            OldS1NodeAPP_scheduledUploadData();
        }
#endif
	}
}


//***********************************************************************************
// brief:   reset the system and save the rtc data
// 
// parameter:   none 
//***********************************************************************************
void SystemResetAndSaveRtc(void)
{
	g_rSysConfigInfo.rtc = Rtc_get_calendar();
#ifdef  SUPPORT_DEVICED_STATE_UPLOAD
    Flash_store_devices_state(TYPE_POWER_RESTART);
#endif
    Flash_store_config();
	SysCtrlSystemReset();
}


