/*
* @Author: zxt
* @Date:   2018-03-09 11:13:28
* @Last Modified by:   zxt
* @Last Modified time: 2020-01-10 16:35:36
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
    

    KeyInit();


    Spi_init();

#ifdef SUPPORT_SHT3X
    //SHT3X Reset Pin initial
    SHT3x_ResetIoInitial();
#endif

    I2c_init();

    Flash_init();

    configModeTimeCnt = 0;
    


    g_rSysConfigInfo.rfStatus       |= STATUS_1310_MASTER;

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
        if(g_rSysConfigInfo.rfStatus & STATUS_LORA_TEST)
        {
            RadioTestEnable();
        }

        break;

        case DEVICES_OFF_MODE:

        break;

        case DEVICES_CONFIG_MODE:
        // if(g_rSysConfigInfo.rfStatus & STATUS_LORA_TEST)
        // {
        //     Sys_event_post(SYS_EVT_CONFIG_MODE_EXIT);
        // }

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
#if  defined(SUPPORT_RARIO_SPEED_SET)
        RadioSwitchingUserRate();
#endif

        NodeStrategyStop();
        RadioAbort();
        EasyLink_setRfPower(7);
        RadioSetRxMode();

        RadioEventPost(RADIO_EVT_SEND_CONFIG);

        break;
    }
}


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
}

void NodeAppConfigModeExit(void)
{
    deviceMode = deviceModeTemp;
    RadioSwitchingSettingRate();
    NodeStrategyBuffClear();
    RadioModeSet(RADIOMODE_SENDPORT);
    if(deviceMode != DEVICES_OFF_MODE)
    {
        NodeStartBroadcast();
        NodeBroadcasting();
       if( g_rSysConfigInfo.rfStatus&STATUS_1310_MASTER){
          NodeStrategyStart();
        }
    }

    NodeWakeup();
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
