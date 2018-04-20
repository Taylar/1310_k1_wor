/*
* @Author: zxt
* @Date:   2018-03-09 11:15:03
* @Last Modified by:   zxt
* @Last Modified time: 2018-04-18 17:01:24
*/
#include "../general.h"





/***** Defines *****/
#define     SCREEN_SLEEP_TIME           15

/***** Type declarations *****/


/***** Variable declarations *****/
uint8_t  screenSleepMonitorCnt;



/***** Prototypes *****/



/***** Function definitions *****/


//***********************************************************************************
// brief:   
// 
// parameter: 
//***********************************************************************************
void S6HwInit(void)
{
	KeyInit();
    KeyRegister(SystemKeyEventPostIsr, KEY_0_SHORT_PRESS);

    KeyRegister(SystemLongKeyEventPostIsr, KEY_0_LONG_PRESS);

	AdcDriverInit();
    Disp_init();


    Spi_init();

    Flash_init();

    KeyRegister(SystemKey1EventPostIsr, KEY_1_SHORT_PRESS);

    KeyRegister(SystemLongKey1EventPostIsr, KEY_1_LONG_PRESS);
    
    screenSleepMonitorCnt = 0;

    UsbIntInit(SystemUsbIntEventPostIsr);

    Battery_init();
    Battery_voltage_measure();

    Disp_poweron();
    Disp_proc();
}



//***********************************************************************************
// brief:the S6 Concenter short key application
// 
// parameter: 
//***********************************************************************************
void S6ShortKeyApp(void)
{
    screenSleepMonitorCnt = 0;
    switch(deviceMode)
    {
        case DEVICES_ON_MODE:
        Disp_info_switch();
        Disp_proc();
        // Led_ctrl(LED_B, 1, 500 * CLOCK_UNIT_MS, 1);
        break;

        case DEVICES_OFF_MODE:
        // Disp_info_switch();
        // Disp_proc();
        Led_ctrl(LED_R, 1, 500 * CLOCK_UNIT_MS, 1);
        break;


        case DEVICES_MENU_MODE:
        Menu_action_proc(MENU_AC_DOWN);
        Disp_proc();
        break;

        case DEVICES_SLEEP_MODE:
        Disp_poweron();
        Disp_proc();
        deviceMode = DEVICES_ON_MODE;
        break;
    }
}

//***********************************************************************************
// brief:the Concenter long key application
// 
// parameter: 
//***********************************************************************************
void S6ConcenterLongKeyApp(void)
{
    screenSleepMonitorCnt = 0;
    switch(deviceMode)
    {
        case DEVICES_ON_MODE:
        Disp_poweroff();
        ConcenterSleep();
        Led_ctrl(LED_R, 1, 250 * CLOCK_UNIT_MS, 6);
        break;

        case DEVICES_OFF_MODE:
        Led_ctrl(LED_B, 1, 250 * CLOCK_UNIT_MS, 6);
        ConcenterWakeup();
        Disp_poweron();
        Disp_proc();
        break;

        case DEVICES_SLEEP_MODE:
        Disp_poweron();
        Disp_proc();
        deviceMode = DEVICES_ON_MODE;
        break;
    }
}

//***********************************************************************************
// brief:the S6 Concenter short key application
// 
// parameter: 
//***********************************************************************************
void S6ShortKey1App(void)
{
    screenSleepMonitorCnt = 0;
    switch(deviceMode)
    {
        case DEVICES_ON_MODE:
        Disp_info_close();
        Disp_proc();
        break;

        case DEVICES_OFF_MODE:
        Led_ctrl(LED_R, 1, 500 * CLOCK_UNIT_MS, 1);
        break;

        case DEVICES_MENU_MODE:
        Menu_action_proc(MENU_AC_ENTER);
        if(DEVICES_ON_MODE == deviceMode)
        {
            Disp_proc();
        }
        break;

        case DEVICES_SLEEP_MODE:
        Disp_poweron();
        Disp_proc();
        deviceMode = DEVICES_ON_MODE;
        break;

    }
}



//***********************************************************************************
// brief:the Concenter long key application
// 
// parameter: 
//***********************************************************************************
void S6LongKey1App(void)
{
    screenSleepMonitorCnt = 0;
    switch(deviceMode)
    {
        case DEVICES_ON_MODE:
        deviceMode = DEVICES_MENU_MODE;
        PoweroffMenu_init();
        Disp_proc();
        break;

        case DEVICES_OFF_MODE:
        Led_ctrl(LED_B, 1, 250 * CLOCK_UNIT_MS, 6);
        ConcenterWakeup();
        Disp_poweron();
        Disp_info_close();
        Disp_proc();
        break;

        case DEVICES_MENU_MODE:

        break;

        case DEVICES_SLEEP_MODE:
        Disp_poweron();
        Disp_proc();
        deviceMode = DEVICES_ON_MODE;
        break;
    }
}


//***********************************************************************************
// brief:the screen monitor
// 
// parameter: 
//***********************************************************************************
void S6AppRtcProcess(void)
{
    if(Disp_powerState())
    {
        screenSleepMonitorCnt ++;
        if(screenSleepMonitorCnt >= SCREEN_SLEEP_TIME)
        {
            Disp_poweroff();
            deviceMode = DEVICES_SLEEP_MODE;
        }
    }
}


