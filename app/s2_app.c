/*
* @Author: zxt
* @Date:   2018-03-09 11:14:22
* @Last Modified by:   zxt
* @Last Modified time: 2019-10-22 16:36:04
*/
#include "../general.h"

#ifdef BOARD_B2S

/***** Defines *****/


/***** Type declarations *****/


/***** Variable declarations *****/



/***** Prototypes *****/



/***** Function definitions *****/


//***********************************************************************************
// brief:   
// 
// parameter: 
//***********************************************************************************
void S2HwInit(void)
{
    LedInit();
    
	KeyInit();
    KeyRegister(SystemKeyEventPostIsr, KEY_0_SHORT_PRESS);
    KeyRegister(SystemLongKeyEventPostIsr, KEY_0_LONG_PRESS);
    KeyRegister(SystemDoubleKeyEventPostIsr, KEY_0_DOUBLE_PRESS);


    Led_ctrl2(LED_B, 1, 200 * CLOCK_UNIT_MS, 800 * CLOCK_UNIT_MS, 3);

    I2c_init();

    Spi_init();

    Flash_init();

#ifdef SUPPORT_SHT3X
    //SHT3X Reset Pin initial
    if(g_rSysConfigInfo.sensorModule[0] == SEN_TYPE_SHT2X)
        SHT3x_ResetIoInitial();
#endif

#ifdef SUPPORT_CHARGE_DECT
    Charge_detect_init();
#endif

    UsbIntInit(SystemUsbIntEventPostIsr);

    AdcDriverInit();
    Battery_init();
    Battery_voltage_measure();
}

//***********************************************************************************
// brief:the Concenter short key application
// 
// parameter: 
//***********************************************************************************
void S2ShortKeyApp(void)
{
    switch(deviceMode)
    {
        case DEVICES_ON_MODE:
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
// brief:the Concenter long key application
// 
// parameter: 
//***********************************************************************************
void S2LongKeyApp(void)
{
    switch(deviceMode)
    {
        case DEVICES_ON_MODE:
        case DEVICES_CONFIG_MODE:
        S2Sleep();
        Led_ctrl2(LED_R, 1, 200 * CLOCK_UNIT_MS, 800 * CLOCK_UNIT_MS, 3);
        break;

        case DEVICES_OFF_MODE:
        //Led_ctrl(LED_B, 1, 200 * CLOCK_UNIT_MS, 3);
        Flash_store_config();
        SysCtrlSystemReset();
        break;
    }
}


//***********************************************************************************
// brief:the node long key application
// 
// parameter: 
//***********************************************************************************
void S2DoubleKeyApp(void)
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
        RadioSwitchingUserRate();

        NodeStrategyStop();
        RadioAbort();
        EasyLink_setRfPower(7);
        RadioSetRxMode();

        RadioEventPost(RADIO_EVT_SEND_CONFIG);
        Led_ctrl2(LED_G, 1, 200 * CLOCK_UNIT_MS, 800 * CLOCK_UNIT_MS, 3);
        break;
    }
}

void S2AppRtcProcess(void)
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

//***********************************************************************************
// brief:   S2 wakeup enable the rtc and the radio function
// 
// parameter: 
//***********************************************************************************
void S2Wakeup(void)
{
    deviceMode = DEVICES_ON_MODE;
    RtcStart();
    
#ifdef  SUPPORT_NETWORK
    Nwk_poweron();
#endif

#ifdef S_G//网关
    ConcenterWakeup();
#endif // S_G//网关

#ifdef S_C //节点
#if !defined (SUPPORT_BOARD_OLD_S1)
    NodeWakeup();
    Sensor_colect_event_set();
#endif
#endif // S_C //节点
}



//***********************************************************************************
// brief:   S6 wakeup enable the rtc and the radio function
// 
// parameter: 
//***********************************************************************************
void S2Sleep(void)
{
#if !defined(SUPPORT_BOARD_OLD_S2S_1)
    RtcStop();
#endif
    // wait the nwk disable the uart
#ifdef  SUPPORT_NETWORK
    while(Nwk_is_Active()) {
        Nwk_poweroff();
        Task_sleep(100 * CLOCK_UNIT_MS);
    }
#endif

//    InterfaceEnable();

#ifdef S_G
    ConcenterSleep();
#endif // S_G

#ifdef S_C
    RadioAbort();
    NodeSleep();
#endif // S_C

    deviceMode = DEVICES_OFF_MODE;
}

//***********************************************************************************
// brief:
//
// parameter:
//***********************************************************************************
void UsbIntProcess(void)
{
    static uint8_t deviceModeTemp = DEVICES_SLEEP_MODE;

    if(GetUsbState() == USB_LINK_STATE)
    {
        switch(deviceMode)
        {
            case DEVICES_ON_MODE:
            case DEVICES_SLEEP_MODE:

#ifdef      SUPPORT_DISP_SCREEN
            Disp_poweroff();
#endif
            Task_sleep(100 * CLOCK_UNIT_MS);
            // wait for the gsm uart close
#ifdef  SUPPORT_NETWORK
            Nwk_poweroff();
            Task_sleep(100 * CLOCK_UNIT_MS);
            while(Nwk_is_Active())
                Task_sleep(100 * CLOCK_UNIT_MS);
#endif

            InterfaceEnable();
            RadioTestDisable();
            S2Sleep();

            deviceModeTemp = DEVICES_SLEEP_MODE;
            deviceMode = DEVICES_CONFIG_MODE;
            break;

            case DEVICES_OFF_MODE:
            InterfaceEnable();
            deviceModeTemp = DEVICES_OFF_MODE;
            deviceMode = DEVICES_CONFIG_MODE;
            break;

            case DEVICES_CONFIG_MODE:
            case DEVICES_TEST_MODE:
            break;

        }

    }
    else
    {
        // the usb has unlink
        if(deviceMode != DEVICES_CONFIG_MODE)
            return;

        deviceMode = deviceModeTemp;
        InterfaceDisable();
        SetRadioSrcAddr( (((uint32_t)(g_rSysConfigInfo.DeviceId[0])) << 24) |
                         (((uint32_t)(g_rSysConfigInfo.DeviceId[1])) << 16) |
                         (((uint32_t)(g_rSysConfigInfo.DeviceId[2])) << 8) |
                         g_rSysConfigInfo.DeviceId[3]);
        SetRadioSubSrcAddr(0xffff0000 | (g_rSysConfigInfo.customId[0] << 8) | g_rSysConfigInfo.customId[1]);
        switch(deviceMode)
        {
            case DEVICES_ON_MODE:
            case DEVICES_SLEEP_MODE:
            S2Wakeup();
            if(g_rSysConfigInfo.rfStatus & STATUS_LORA_TEST)
            {
                RadioTestEnable();
            }
            case DEVICES_OFF_MODE:
            break;

            case DEVICES_CONFIG_MODE:
            case DEVICES_TEST_MODE:
            break;

        }
    }
}
#endif
