/*
* @Author: zxt
* @Date:   2018-03-09 11:15:03
* @Last Modified by:   zxt
* @Last Modified time: 2018-04-25 14:45:08
*/
#include "../general.h"

#ifdef BOARD_S6_6



/***** Defines *****/
#define     SCREEN_SLEEP_TIME           15

/***** Type declarations *****/


/***** Variable declarations *****/

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
    if((!(g_rSysConfigInfo.module & MODULE_LCD)) || (deviceMode == DEVICES_OFF_MODE)){//
        return;
    }

    if (!(g_rSysConfigInfo.status & STATUS_LCD_ALWAYS_ON))
    {
        deviceMode = DEVICES_SLEEP_MODE;
        Disp_poweroff();
    }
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


    UsbIntInit(SystemUsbIntEventPostIsr);

    Battery_init();
    Battery_voltage_measure();

    Disp_poweron();
    SystemEventSet(SYSTEMAPP_EVT_DISP);

    
    

}




//***********************************************************************************
// brief:the S6 Concenter short key application
// 
// parameter: 
//***********************************************************************************
void S6ShortKeyApp(void)
{
    Disp_poweron();
    switch(deviceMode)
    {
        case DEVICES_ON_MODE:
#ifdef SUPPORT_MENU
        if (Menu_is_process()) {
            Menu_action_proc(MENU_AC_DOWN);
        } else
#endif
        {
            Disp_info_switch();
        }
        break;

        case DEVICES_OFF_MODE:
        Led_ctrl(LED_R, 1, 500 * CLOCK_UNIT_MS, 1);
        break;

        case DEVICES_SLEEP_MODE:
        Disp_poweron();
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
    Disp_poweron();
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
        break;

        case DEVICES_SLEEP_MODE:
        Disp_poweron();
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
    Disp_poweron();
    switch(deviceMode)
    {
        case DEVICES_ON_MODE:
#ifdef SUPPORT_MENU
        if (Menu_is_process()) {
            Menu_action_proc(MENU_AC_ENTER);
        } else
#endif
        {
#ifdef SUPPORT_SENSOR
            Disp_sensor_switch();
#endif
        }
        break;

        case DEVICES_OFF_MODE:
        Led_ctrl(LED_R, 1, 500 * CLOCK_UNIT_MS, 1);
        break;

        case DEVICES_SLEEP_MODE:
        Disp_poweron();
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
    Disp_poweron();
    switch(deviceMode)
    {
        case DEVICES_ON_MODE:
        PoweroffMenu_init();
        break;

        case DEVICES_OFF_MODE:
        Led_ctrl(LED_B, 1, 250 * CLOCK_UNIT_MS, 6);
        ConcenterWakeup();
        Disp_poweron();
        Disp_info_close();
        break;


        case DEVICES_SLEEP_MODE:
        Disp_poweron();
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

#endif
