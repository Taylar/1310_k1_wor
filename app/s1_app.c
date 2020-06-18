/*
* @Author: justfortest
* @Date:   2018-03-09 11:13:28
* @Last Modified by:   zxt
* @Last Modified time: 2020-06-14 15:47:50
*/
#include "../general.h"



/***** Defines *****/


/***** Type declarations *****/


/***** Variable declarations *****/
uint32_t configModeTimeCnt;          // the unit is sec



/***** Prototypes *****/



/***** Function definitions *****/



//***********************************************************************************
// brief:   
// 
// parameter: 
//***********************************************************************************
void S1HwInit(void)
{
    //-----KeyInit();

    Spi_init();

#ifdef SUPPORT_SHT3X
    //SHT3X Reset Pin initial
    SHT3x_ResetIoInitial();
#endif

    AdcDriverInit();

    I2c_init();

    PwmDriverInit();

    Flash_init();

    ElectricShockInit();

    SoundDriverInit();

    Battery_init();

    Battery_voltage_measure();
    
    configModeTimeCnt = 0;

    g_rSysConfigInfo.rfStatus       |= STATUS_1310_MASTER;

    SoundDriverSet(SOUND_TYPE_VOLUME_MAX);
}




//***********************************************************************************
// brief:the node short key application
// 
// parameter: 
//***********************************************************************************
void S1ShortKeyApp(void)
{
    switch(deviceMode)
    {
        case DEVICES_ON_MODE:
        break;

        case DEVICES_OFF_MODE:

        break;

        case DEVICES_CONFIG_MODE:

        break;

    }
}

//***********************************************************************************
// brief:the node long key application
// 
// parameter: 
//***********************************************************************************
void S1LongKeyApp(void)
{
    switch(deviceMode)
    {
        case DEVICES_ON_MODE:
        case DEVICES_CONFIG_MODE:
        if(!(g_rSysConfigInfo.status & STATUS_HIDE_PWOF_MENU))
        {
            g_rSysConfigInfo.sysState.wtd_restarts &= (0xFFFF^STATUS_POWERON);
        }
        S1Sleep();

        g_rSysConfigInfo.rtc = Rtc_get_calendar();
        Flash_store_config();
        Task_sleep(3000 * CLOCK_UNIT_MS);
        if(STATUS_POWERON_RESET_DATA & g_rSysConfigInfo.status)
            Flash_reset_all();
        SysCtrlSystemReset();
        break;

        case DEVICES_OFF_MODE:
        if(Battery_get_voltage() <= g_rSysConfigInfo.batLowVol)
        {

        }
        else
        {

            g_rSysConfigInfo.sysState.wtd_restarts |= STATUS_POWERON;
            S1Wakeup();
            Sys_event_post(SYSTEMAPP_EVT_STORE_SYS_CONFIG);
        }
        break;
    }
}


//***********************************************************************************
// brief:the node long key application
// 
// parameter: 
//***********************************************************************************
void S1DoubleKeyApp(void)
{
    switch(deviceMode)
    {
        case DEVICES_ON_MODE:
        case DEVICES_CONFIG_MODE:
        case DEVICES_OFF_MODE:
        if(DEVICES_CONFIG_MODE != deviceMode)
            deviceModeTemp = deviceMode;


        // enter DEVICES_CONFIG_MODE, clear radio tx buf and send the config parameter to config deceive
        // if(RadioStatueRead() == RADIOSTATUS_TRANSMITTING)
        NodeStrategyReset();
        NodeResetAPC();
        deviceMode                      = DEVICES_CONFIG_MODE;
        configModeTimeCnt = 0;
        NodeUploadOffectClear();
        //RadioModeSet(RADIOMODE_RECEIVEPORT);
        SetRadioDstAddr(CONFIG_DECEIVE_ID_DEFAULT);


        NodeStrategyStop();
        RadioAbort();
        EasyLink_setRfPower(7);
        RadioSetRxMode();

        RadioEventPost(RADIO_EVT_SEND_CONFIG);

        break;
    }
}

uint32_t lowBatCnt = 0;
void S1AppRtcProcess(void)
{
	if(deviceMode == DEVICES_CONFIG_MODE && RADIOMODE_UPGRADE != RadioModeGet())
    {
        configModeTimeCnt++;
        if(configModeTimeCnt >= S1_CONFIG_MODE_TIME)
        {
            Sys_event_post(SYS_EVT_CONFIG_MODE_EXIT);
        }
    }

    // ElecPreventInsertMeasure();
    // if(ElecPreventInsertState()){
    //     EletricPulseSetTime_S(1);
    //     RadioCmdSetWithNoResponBrocast(RADIO_CMD_INSERT_TYPE, (g_rSysConfigInfo.customId[0] << 8) | g_rSysConfigInfo.customId[1]);
    //     SoundEventSet(SOUND_TYPE_INSERT);
    // }


    Battery_porcess();
    if(Battery_get_voltage() < 3600){
        if((lowBatCnt == 0) || (lowBatCnt >= 30)){
            SoundEventSet(SOUND_TYPE_LOW_BAT);
            lowBatCnt = 1;
            RadioCmdSetWithNoResponBrocast(RADIO_CMD_LOW_VOL_TYPE, (g_rSysConfigInfo.customId[0] << 8) | g_rSysConfigInfo.customId[1]);
        }
        lowBatCnt++;
    }else{
        lowBatCnt = 0;
    }


    destroyEleShock = DestroyPinRead();
    if(destroyEleShock){
        if(destroyEleShock){
            EletricPulseSetTime_S(1);
            RadioCmdSetWithNoRes(RADIO_CMD_DESTROY_TYPE, NULL);
            SoundEventSet(SOUND_TYPE_DESTROYED);
        } 
    }

    // for test
    // RadioCmdSetWithNoRes(RADIO_PRO_CMD_ALL_RESP, RADIO_CONTROLER_ADDRESS);
}


extern void WdtResetCb(uintptr_t handle);



//***********************************************************************************
// brief:   S1 wakeup enable the rtc / wdt and the node function
// 
// parameter: 
//***********************************************************************************
void S1Wakeup(void)
{

    deviceMode = DEVICES_ON_MODE;
    RtcStart();

#ifdef  SUPPORT_WATCHDOG
    WdtInit(WdtResetCb);
#endif

    NodeWakeup();
}


//***********************************************************************************
// brief:   S1 sleep enable the rtc / wdt and the node function
// 
// parameter: 
//***********************************************************************************
void S1Sleep(void)
{
    deviceMode = DEVICES_OFF_MODE;
    NodeSleep();
}
