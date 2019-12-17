/*
* @Author: zxt
* @Date:   2018-03-09 11:13:28
* @Last Modified by:   zxt
* @Last Modified time: 2019-12-13 10:37:49
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
    LedInit();
    
    KeyInit();
    KeyRegister(SystemKeyEventPostIsr, KEY_0_SHORT_PRESS);
    KeyRegister(SystemLongKeyEventPostIsr, KEY_0_LONG_PRESS);
    KeyRegister(SystemDoubleKeyEventPostIsr, KEY_0_DOUBLE_PRESS);

    Spi_init();

#ifdef SUPPORT_SHT3X
    //SHT3X Reset Pin initial
    SHT3x_ResetIoInitial();
#endif

    I2c_init();

    Flash_init();

    configModeTimeCnt = 0;
#ifdef SUPPORT_UPLOAD_ASSET_INFO
    g_rSysConfigInfo.sensorModule[0] = SEN_TYPE_NONE;

    #ifdef SUPPORT_LIGHT
    g_rSysConfigInfo.sensorModule[1] = SEN_TYPE_OPT3001;
    #else
    g_rSysConfigInfo.sensorModule[1] = SEN_TYPE_NONE;
    #endif //HAIER_Z1_C
#endif // SUPPORT_UPLOAD_ASSET_INFO
    
    
#if defined(KINGBOSS_S3_C_SHT3X) || defined(ZKS_S3_C_SHT3X) || defined(ZKS_S3_C_SHT2X)

    g_rSysConfigInfo.sensorModule[0] = SEN_TYPE_SHT2X;
    g_rSysConfigInfo.sensorModule[1] = SEN_TYPE_NONE;

    g_rSysConfigInfo.rfStatus       |= STATUS_1310_MASTER;

#endif //KINGBOSS_PROJECT
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
        case DEVICES_WAKEUP_MODE:
        if(g_rSysConfigInfo.rfStatus & STATUS_LORA_TEST)
        {
            RadioTestEnable();
        }
        Led_ctrl(LED_B, 1, 200 * CLOCK_UNIT_MS, 1);
        break;

        case DEVICES_OFF_MODE:
        Led_ctrl(LED_R, 1, 200 * CLOCK_UNIT_MS, 1);
        break;

        case DEVICES_CONFIG_MODE:
        // if(g_rSysConfigInfo.rfStatus & STATUS_LORA_TEST)
        // {
        //     Sys_event_post(SYS_EVT_CONFIG_MODE_EXIT);
        // }
        Led_ctrl(LED_G, 1, 200 * CLOCK_UNIT_MS, 1);
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
        case DEVICES_WAKEUP_MODE:
        case DEVICES_CONFIG_MODE:
        if(!(g_rSysConfigInfo.status & STATUS_HIDE_PWOF_MENU))
        {
            g_rSysConfigInfo.sysState.wtd_restarts &= (0xFFFF^STATUS_POWERON);
        }
        S1Sleep();
        Led_ctrl2(LED_R, 1, 200 * CLOCK_UNIT_MS, 800 * CLOCK_UNIT_MS, 3);
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
            Led_ctrl(LED_R, 1, 200 * CLOCK_UNIT_MS, 1);
        }
        else
        {
            Led_ctrl2(LED_B, 1, 200 * CLOCK_UNIT_MS, 800 * CLOCK_UNIT_MS, 3);
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
        case DEVICES_WAKEUP_MODE:
        case DEVICES_CONFIG_MODE:
        case DEVICES_OFF_MODE:
        if(DEVICES_CONFIG_MODE != deviceMode)
            deviceModeTemp = deviceMode;

#if   defined(SUPPORT_BOARD_OLD_S1) || defined(SUPPORT_BOARD_OLD_S2S_1)
            OldS1NodeApp_stopSendSensorData();
#endif
        // enter DEVICES_CONFIG_MODE, clear radio tx buf and send the config parameter to config deceive
        // if(RadioStatueRead() == RADIOSTATUS_TRANSMITTING)
        NodeStrategyReset();
        NodeResetAPC();
        deviceMode                      = DEVICES_CONFIG_MODE;
        configModeTimeCnt = 0;
        NodeUploadOffectClear();
        //RadioModeSet(RADIOMODE_RECEIVEPORT);
        SetRadioDstAddr(CONFIG_DECEIVE_ID_DEFAULT);
#if   defined(SUPPORT_BOARD_OLD_S1) || defined(SUPPORT_BOARD_OLD_S2S_1) || defined(SUPPORT_RARIO_SPEED_SET)
        RadioSwitchingUserRate();
#endif

#if  !defined(SUPPORT_BOARD_OLD_S1) && !defined(SUPPORT_BOARD_OLD_S2S_1)
        NodeStrategyStop();
        RadioAbort();
        EasyLink_setRfPower(7);
        RadioSetRxMode();
#endif

        RadioEventPost(RADIO_EVT_SEND_CONFIG);
        Led_ctrl2(LED_G, 1, 200 * CLOCK_UNIT_MS, 800 * CLOCK_UNIT_MS, 3);
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

    if( !(g_rSysConfigInfo.rfStatus&STATUS_1310_MASTER)){
       deviceMode = DEVICES_WAKEUP_MODE;
    }
    else{
        deviceMode = DEVICES_ON_MODE;
    }
    RtcStart();

#ifdef  SUPPORT_WATCHDOG
    WdtInit(WdtResetCb);
#endif

#if !defined (SUPPORT_BOARD_OLD_S1)
    NodeWakeup();
#endif
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
