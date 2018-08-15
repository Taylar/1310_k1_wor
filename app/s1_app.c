/*
* @Author: zxt
* @Date:   2018-03-09 11:13:28
* @Last Modified by:   zxt
* @Last Modified time: 2018-08-15 19:23:33
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

    I2c_init();

    Flash_init();

    configModeTimeCnt = 0;
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
#ifdef SUPPORT_BOARD_OLD_S1
        case OLD_S1_DEVICES_RADIO_UPGRADE:
#endif

        Led_ctrl(LED_B, 1, 500 * CLOCK_UNIT_MS, 1);
        break;

        case DEVICES_OFF_MODE:
        Led_ctrl(LED_R, 1, 500 * CLOCK_UNIT_MS, 1);
        break;

        case DEVICES_CONFIG_MODE:
        Led_ctrl(LED_G, 1, 500 * CLOCK_UNIT_MS, 1);
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
        NodeSleep();
        g_rSysConfigInfo.sysState.reserve &= (0xFFFF^STATUS_POWERON);
        Led_ctrl(LED_R, 1, 250 * CLOCK_UNIT_MS, 6);
        g_rSysConfigInfo.rtc = Rtc_get_calendar();
        Flash_store_config();
        Task_sleep(3000 * CLOCK_UNIT_MS);
        SysCtrlSystemReset();
        break;

        case DEVICES_OFF_MODE:
        if(Battery_get_voltage() <= g_rSysConfigInfo.batLowVol)
        {
            Led_ctrl(LED_R, 1, 250 * CLOCK_UNIT_MS, 1);
        }
        else
        {
            Led_ctrl(LED_B, 1, 250 * CLOCK_UNIT_MS, 6);
            g_rSysConfigInfo.sysState.reserve |= STATUS_POWERON;
            NodeWakeup();
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
#ifdef SUPPORT_BOARD_OLD_S1
            OldS1NodeApp_stopSendSensorData();
#endif
        // enter DEVICES_CONFIG_MODE, clear radio tx buf and send the config parameter to config deceive
        // if(RadioStatueRead() == RADIOSTATUS_TRANSMITTING)
        {
            EasyLink_abort();
        }
        NodeStrategyReset();
        deviceMode                      = DEVICES_CONFIG_MODE;
        configModeTimeCnt = 0;
        NodeUploadOffectClear();
        NodeStrategyBusySet(false);
        RadioModeSet(RADIOMODE_RECEIVEPORT);
        SetRadioDstAddr(CONFIG_DECEIVE_ID_DEFAULT);
#ifdef SUPPORT_BOARD_OLD_S1
        RadioSwitchingUserRate();
#endif
        RadioEventPost(RADIO_EVT_SEND_CONFIG);
        Led_ctrl(LED_G, 1, 250 * CLOCK_UNIT_MS, 6);
        break;
    }
}



void S1AppRtcProcess(void)
{
	if(deviceMode == DEVICES_CONFIG_MODE && RADIOMODE_UPGRADE != RadioModeGet())
    {
        configModeTimeCnt++;
        if(configModeTimeCnt >= 120)
        {
            ClearRadioSendBuf();
            RadioModeSet(RADIOMODE_SENDPORT);
            NodeStartBroadcast();
            NodeStrategyBusySet(true);

            NodeBroadcasting();
            deviceMode = DEVICES_ON_MODE;
        }
    }
}
