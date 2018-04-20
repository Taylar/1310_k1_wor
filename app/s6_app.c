/*
* @Author: zxt
* @Date:   2018-03-09 11:15:03
* @Last Modified by:   zxt
* @Last Modified time: 2018-04-20 18:29:57
*/
#include "../general.h"





/***** Defines *****/
#define     SCREEN_SLEEP_TIME           15

/***** Type declarations *****/


/***** Variable declarations *****/
uint8_t  screenSleepMonitorCnt;

Clock_Struct sysLcdShutClkStruct;
Clock_Handle sysLcdShutClkHandle;

/***** Prototypes *****/



/***** Function definitions *****/



//***********************************************************************************
//
// System LCD shutdown callback function.
//
//***********************************************************************************
void Sys_lcdShutFxn(UArg arg0)
{
    if(!(g_rSysConfigInfo.module & MODULE_LCD)){//
        return;
    }

    if (!(g_rSysConfigInfo.status & STATUS_LCD_ALWAYS_ON))
        Disp_poweroff();
}


//***********************************************************************************
//
// System LCD auto shutdown start timing function.
//
//***********************************************************************************
void Sys_lcd_start_timing(void)
{
    if(!(g_rSysConfigInfo.module & MODULE_LCD)){//
        return;
    }

    if (!(g_rSysConfigInfo.status & STATUS_LCD_ALWAYS_ON))
        Clock_start(sysLcdShutClkHandle);
}

//***********************************************************************************
//
// System LCD auto shutdown stop timing function.
//
//***********************************************************************************
void Sys_lcd_stop_timing(void)
{
    if(!(g_rSysConfigInfo.module & MODULE_LCD)){//
        return;
    }

    if (!(g_rSysConfigInfo.status & STATUS_LCD_ALWAYS_ON))
        Clock_stop(sysLcdShutClkHandle);
}


//***********************************************************************************
// brief:   
// 
// parameter: 
//***********************************************************************************
void S6HwInit(void)
{
    LedInit();

	KeyInit();
    KeyRegister(SystemKeyEventPostIsr, KEY_0_SHORT_PRESS);
    KeyRegister(SystemLongKeyEventPostIsr, KEY_0_LONG_PRESS);

    KeyRegister(SystemKey1EventPostIsr, KEY_1_SHORT_PRESS);
    KeyRegister(SystemLongKey1EventPostIsr, KEY_1_LONG_PRESS);

	AdcDriverInit();

    Disp_init();

    Spi_init();

    Flash_init();

    screenSleepMonitorCnt = 0;

    UsbIntInit(SystemUsbIntEventPostIsr);

    Battery_init();
    Battery_voltage_measure();

    Disp_poweron();
    Disp_proc();



    /* Construct a one-shot Clock Instance to shutdown display */
    Clock_Params clkParams;
    Clock_Params_init(&clkParams);
    clkParams.period = 0;
    clkParams.startFlag = FALSE;
    Clock_construct(&sysLcdShutClkStruct, (Clock_FuncPtr)Sys_lcdShutFxn,
                        15 * CLOCK_UNIT_S, &clkParams);
    /* Obtain clock instance handle */
    sysLcdShutClkHandle = Clock_handle(&sysLcdShutClkStruct);
    
    if(!(g_rSysConfigInfo.module & MODULE_LCD)){//Ã»ÓÐlcd,sysLcdShutClkHandleÓÃÀ´×ö¹¤×÷Ö¸Ê¾µÆµÄ¶¨Ê±Æ÷¡£
        Clock_setPeriod(sysLcdShutClkHandle, 60*CLOCK_UNIT_S);
        Clock_setTimeout(sysLcdShutClkHandle,  60*CLOCK_UNIT_S);
        Clock_start(sysLcdShutClkHandle); 
    }

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

}


