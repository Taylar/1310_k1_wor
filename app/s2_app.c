/*
* @Author: zxt
* @Date:   2018-03-09 11:14:22
* @Last Modified by:   zxt
* @Last Modified time: 2018-09-12 10:36:47
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

    I2c_init();

    Spi_init();

    Flash_init();

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
        Led_ctrl(LED_B, 1, 500 * CLOCK_UNIT_MS, 1);
        break;

        case DEVICES_OFF_MODE:
        Led_ctrl(LED_R, 1, 500 * CLOCK_UNIT_MS, 1);
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
        S2Sleep();
        Led_ctrl(LED_R, 1, 250 * CLOCK_UNIT_MS, 6);
        break;

        case DEVICES_OFF_MODE:
        Led_ctrl(LED_B, 1, 250 * CLOCK_UNIT_MS, 6);
        SysCtrlSystemReset();
        break;
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
    NodeWakeup();
#endif // S_C //节点
}



//***********************************************************************************
// brief:   S6 wakeup enable the rtc and the radio function
// 
// parameter: 
//***********************************************************************************
void S2Sleep(void)
{
    RtcStop();
    // wait the nwk disable the uart
#ifdef  SUPPORT_NETWORK
    Nwk_poweroff();
    while(Nwk_is_Active())
        Task_sleep(100 * CLOCK_UNIT_MS);
#endif

    InterfaceEnable();

#ifndef S_A
    ConcenterSleep();
#endif // S_A

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
