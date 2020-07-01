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


void SystemUsbIntEventPostIsr(void)
{
    Event_post(systemAppEvtHandle, SYSTEMAPP_EVT_USBINT);
}

void WdtResetCb(uintptr_t handle)
{
    while(1)
        HWREGBITW( PRCM_BASE + PRCM_O_WARMRESET, PRCM_WARMRESET_WR_TO_PINRESET_BITN ) = 1;
}

void RtcEventSet(void)
{

#ifdef BOARD_S3
	Sys_event_post(SYS_EVT_RTC);
#endif

#ifdef S_C//节点
	if(g_rSysConfigInfo.rfStatus&STATUS_1310_MASTER){
	   NodeRtcProcess();
	}
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
static uint8_t g_firstStartFlag = 0;
static uint8_t lcd_power_state = 0;
void SystemAppTaskFxn(void)
{
    uint32_t    eventId;

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


#ifdef BOARD_S3
   	S1HwInit();
#endif
   	RtcInit(RtcEventSet);


    Task_sleep(10 * CLOCK_UNIT_MS);

#ifdef BOARD_S3
    RtcStart();
#endif




#ifdef SUPPORT_ENGMODE
    if (GetEngModeFlag())
    {
 		EngMode();
 		engmodeFlag = 1;
    }
#endif  // SUPPORT_ENGMODE


#ifdef BOARD_S6_6		
		S6Wakeup();

    WdtInit(WdtResetCb);
#endif // BOARD_S6_6


#ifdef BOARD_S3
	Task_sleep(100 * CLOCK_UNIT_MS);
	S1Wakeup();
#endif //

	// uint32_t RestStatus;
 //    uint8_t logtest[6] = {0};
 //    Flash_log("PON\n");
 //    RestStatus = SysCtrlResetSourceGet();
 //    sprintf((char*)logtest, "R%2ld\n",RestStatus);
 //    Flash_log((uint8_t*)logtest);
    lcd_power_state = 1;
	for(;;)
	{
        eventId = Event_pend(systemAppEvtHandle, 0, SYSTEMAPP_EVT_ALL, BIOS_WAIT_FOREVER);
	

        if(eventId & SYS_FEED_WATCHDOG)
        {
			WdtClear();
        }

#ifdef BOARD_S6_6
        if(eventId & SYS_EVT_KEY_SCAN){
        	KeyScanFxn();
        }
#endif // BOARD_S6_6

// the config deceive key is disable
		if(eventId &SYSTEMAPP_EVT_KEY)
		{

#ifdef BOARD_S6_6
			if(lcd_power_state== 1)
			    S6KeyApp();
#endif
		}



		if((eventId & SYSTEMAPP_EVT_RTC_READ))
		{
			ConcenterRtcRead();
		}

		if((eventId & SYSTEMAPP_EVT_STORE_CONCENTER))
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

#ifdef SUPPORT_ALARM_RECORD_QURERY
    	if(eventId & SYS_EVT_ALARM_SAVE)
      	{
          	Flash_store_alarm_record((uint8_t*)(&g_AlarmSensor),sizeof(Alarmdata_t));
        }

#endif  // SUPPORT_ALARM_RECORD_QURERY

#endif  // BOARD_S6_6

		if(eventId & SYSTEMAPP_EVT_STORE_SYS_CONFIG)
		{
			Flash_store_config();
		}


		if((eventId & SYSTEMAPP_EVT_RADIO_ABORT))
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

		if(SYS_EVT_ELE_SHOCK_DESTROY & eventId){
			destroyEleShock = 1;
			SoundEventSet(SOUND_TYPE_DESTROYED);
		}


		
		if(eventId & SYS_EVT_RTC){
			S1AppRtcProcess();
		}
#ifdef S_G
        if(eventId & SYS_EVT_KEY){
            //S1AppRtcProcess();
            //Menu_low_power_display(1234);
            if(lcd_power_state)
            {
                Lcd_poweroff();
                lcd_power_state = 0;
            }
            else
            {
                lcd_power_state = 1;
                Disp_init();
                Disp_poweron();
                Sys_event_post(SYSTEMAPP_EVT_DISP);
                g_firstStartFlag = 0;
            }

        }
#endif //S_G
		if(eventId & SYS_EVT_MOTO_INT_REC){
			eleShock_set(ELE_MOTO_ENABLE, 1);
			Task_sleep(1000 * CLOCK_UNIT_MS);
			eleShock_set(ELE_MOTO_ENABLE, 0);
		}


		if(eventId & SYS_EVT_SOUND_PLAY){
			SoundDriverSet(soundEventType);
		}		


#ifdef SUPPORT_DISP_SCREEN
		if(eventId & SYS_EVT_ALARM && lcd_power_state == 1)
		{
		    set_meun_alarmOrSetting(1);
		    menuc_alarm_main(_VK_DISPLAY);
		}
        
		if(eventId & SYSTEMAPP_EVT_DISP && lcd_power_state == 1)
		{
        	Disp_proc();
        	// KeyIcInit();
        	if(!g_firstStartFlag)
        	{
        	   g_firstStartFlag=!g_firstStartFlag;
        	   //gpio_power_en_config();
        	   set_meun_alarmOrSetting(0);
        	   power_on_init_key_code();
        	   menuc_main(_VK_COMMAND);
        	   lcd_power_state = 1;
        	}
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
#ifdef  SUPPORT_DEVICED_STATE_UPLOAD
    Flash_store_devices_state(TYPE_POWER_RESTART);
#endif
    Flash_store_config();
	SysCtrlSystemReset();
}


