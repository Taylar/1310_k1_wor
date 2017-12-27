#include "../general.h"
#include "systemApp.h"

/***** Defines *****/
#define         SYSTEM_APP_STACK_SIZE        1024

// **************************************************************************


/* Clock for the fast report timeout */

static Clock_Struct sysTimerClock;     /* not static so you can see in ROV */

static Clock_Handle sysTimerClockHandle;


// **************************************************************************
/***** Variable declarations *****/
// task
uint8_t         systemAppTaskStack[SYSTEM_APP_STACK_SIZE];
Task_Struct     systemAppTaskStruct;


// event 
Event_Struct systemAppEvtStruct;
Event_Handle systemAppEvtHandle;



/***** Prototypes *****/
void SystemAppTaskFxn(void);


/***** Function definitions *****/
void SystemKeyEventPostIsr(void)
{
    Event_post(systemAppEvtHandle, SYSTEMAPP_EVT_KEY0);
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

	/* Create clock object which is used for fast report timeout */
    Clock_Params clkParams;
    clkParams.period = 1000;
    clkParams.startFlag = FALSE;
    Clock_construct(&sysTimerClock, SysTimerCb, 1, &clkParams);
    sysTimerClockHandle = Clock_handle(&sysTimerClock);

	/* Construct system application Task threads */
    Task_Params taskParams;
    Task_Params_init(&taskParams);
    taskParams.stackSize = SYSTEM_APP_STACK_SIZE;
    taskParams.stack = &systemAppTaskStack;
    taskParams.priority = 2;
    Task_construct(&systemAppTaskStruct, (Task_FuncPtr)SystemAppTaskFxn, &taskParams, &eb);

}



void SystemAppTaskFxn(void)
{
    uint32_t    eventId;

    /* Construct system process Event */
    Event_construct(&systemAppEvtStruct, NULL);
    /* Obtain event instance handle */
    systemAppEvtHandle = Event_handle(&systemAppEvtStruct);


	Spi_init();

	I2c_init();

	Flash_init();

	SHT2X_FxnTable.initFxn(SEN_I2C_CH0);

	Key_init(SystemKeyEventPostIsr);

	Led_init();

	RtcInit(RtcEventSet);

	RtcStart();	

	// Clock_start(sysTimerClockHandle);

	for(;;)
	{
        eventId = Event_pend(systemAppEvtHandle, 0, SYSTEMAPP_EVT_ALL, BIOS_WAIT_FOREVER);
	
		if(eventId &SYSTEMAPP_EVT_KEY0)
		{
			Led_toggle(LED_R);
			Led_toggle(LED_G);
			SHT2X_FxnTable.measureFxn(SEN_I2C_CH0);
			System_printf("the temperature : %d\n", SHT2X_FxnTable.getValueFxn(SEN_I2C_CH0, SENSOR_TEMP));
			System_printf("the humi : %d\n", SHT2X_FxnTable.getValueFxn(SEN_I2C_CH0, SENSOR_HUMI));
		}

		if(eventId &SYSTEMAPP_EVT_INTERFACE)
		{

		}

		if(eventId &SYSTEMAPP_EVT_RADIO_NODE)
		{

		}

		if(eventId &SYSTEMAPP_EVT_RADIO_CONCENTER)
		{

		}

		if(eventId &SYSTEMAPP_EVT_SENSOR)
		{

		}

		if(eventId &SYSTEMAPP_EVT_RTC)
		{

		}

		if(eventId &SYSTEMAPP_EVT_ALL)
		{

		}

		if(eventId & SYSTEMAPP_EVT_TIMER)
		{
			
		}

	}
}

