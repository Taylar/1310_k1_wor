/*
* @Author: zxt
* @Date:   2018-03-09 11:15:03
* @Last Modified by:   zxt
* @Last Modified time: 2018-09-27 17:30:09
*/
#include "../general.h"

#ifdef BOARD_S6_6



/***** Defines *****/
#define     SCREEN_SLEEP_TIME           15

/***** Type declarations *****/


/***** Variable declarations *****/

Clock_Struct sysLcdShutClkStruct, sysAlarmClkStruct, BatAlarmClkStruct;
Clock_Handle sysLcdShutClkHandle, sysAlarmClkHandle, BatAlarmClkHandle;


PWM_Handle buzzerHandle = NULL;



uint8_t buzzerAlarmCnt;

/***** Prototypes *****/



/***** Function definitions *****/


//***********************************************************************************
//
// System buzzer init.
//
//***********************************************************************************
void Sys_buzzer_init(void)
{
    PWM_Params params;

    PWM_Params_init(&params);
    params.dutyUnits   = PWM_DUTY_US;
    params.dutyValue   = 1000000L/ PWM_BUZZER_FRQ / 2;
    params.periodUnits = PWM_PERIOD_US;
    params.periodValue = 1000000L/ PWM_BUZZER_FRQ;
    params.idleLevel   = PWM_IDLE_LOW;
    buzzerHandle       = PWM_open(Board_PWM0, &params);
}


//***********************************************************************************
//
// System buzzer enable.
//
//***********************************************************************************
void Sys_buzzer_enable(void)
{
    PWM_start(buzzerHandle);
    Led_ctrl(LED_R, 1, 0, 0);
}


//***********************************************************************************
//
// System buzzer disable.
//
//***********************************************************************************
void Sys_buzzer_disable(void)
{
    PWM_stop(buzzerHandle);
    Led_ctrl(LED_R, 0, 0, 0);
}


//***********************************************************************************
//
// System alarm timer.
//
//***********************************************************************************
void Sys_alarmFxn(UArg arg0)
{
    if(g_bAlarmSensorFlag == 0){
        Sys_buzzer_disable();
        Clock_stop(sysAlarmClkHandle);
        return;
    }    
    
    if(buzzerAlarmCnt == 0) {
        buzzerAlarmCnt++;
        return;
    }


    if(buzzerAlarmCnt == 1) {      
        Clock_stop(sysAlarmClkHandle);
        Sys_event_post(SYS_EVT_ALARM_SAVE);
        return;        
    }

    if (++buzzerAlarmCnt >= 21) {
        Sys_buzzer_disable();
        Clock_stop(sysAlarmClkHandle);
        Clock_setPeriod(sysAlarmClkHandle, 60*CLOCK_UNIT_S);
        buzzerAlarmCnt = 0;
        Clock_start(sysAlarmClkHandle);
    } else {
        if (buzzerAlarmCnt % 2)
            Sys_buzzer_disable();
        else
            Sys_buzzer_enable();
    }
}

//***********************************************************************************
//
// battery  alarm timer.
//
//***********************************************************************************
void Bat_alarmFxn(UArg arg0)
{
    Led_ctrl(LED_R, 1, 500* CLOCK_UNIT_MS, 1);
}



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

    /* Construct a 500ms periodic Clock Instance to temperature alarm */
    clkParams.period = 500 * CLOCK_UNIT_MS;
    clkParams.startFlag = FALSE;
    Clock_construct(&sysAlarmClkStruct, (Clock_FuncPtr)Sys_alarmFxn, 500 * CLOCK_UNIT_MS, &clkParams);
    /* Obtain clock instance handle */
    sysAlarmClkHandle = Clock_handle(&sysAlarmClkStruct);

    /* Construct a 5s periodic Clock Instance to battery alarm */
    clkParams.period = 5 * CLOCK_UNIT_S;
    clkParams.startFlag = FALSE;
    Clock_construct(&BatAlarmClkStruct, (Clock_FuncPtr)Bat_alarmFxn, 500 * CLOCK_UNIT_MS, &clkParams);
    /* Obtain clock instance handle */
    BatAlarmClkHandle = Clock_handle(&BatAlarmClkStruct);

    LedInit();

	KeyInit();
    KeyRegister(SystemKeyEventPostIsr, KEY_0_SHORT_PRESS);
    KeyRegister(SystemLongKeyEventPostIsr, KEY_0_LONG_PRESS);

    KeyRegister(SystemKey1EventPostIsr, KEY_1_SHORT_PRESS);
    KeyRegister(SystemLongKey1EventPostIsr, KEY_1_LONG_PRESS);

	AdcDriverInit();

    Disp_init();

    Spi_init();

    I2c_init();
    
    Flash_init();

    PwmDriverInit();
    Sys_buzzer_init();

    UsbIntInit(SystemUsbIntEventPostIsr);

    Battery_init();
    Battery_voltage_measure();



    if(!(g_rSysConfigInfo.module & MODULE_LCD)){//
        Clock_setPeriod(sysLcdShutClkHandle, 60*CLOCK_UNIT_S);
        Clock_setTimeout(sysLcdShutClkHandle,  60*CLOCK_UNIT_S);
        Clock_start(sysLcdShutClkHandle); 
    }


    if(Battery_get_voltage() > g_rSysConfigInfo.batLowVol)
    {
        Disp_poweron();
        Sys_event_post(SYSTEMAPP_EVT_DISP);
    }
}




//***********************************************************************************
// brief:the S6 Concenter short key application
// 
// parameter: 
//***********************************************************************************
void S6ShortKeyApp(void)
{
    switch(deviceMode)
    {
        case DEVICES_ON_MODE:
        Disp_poweron();
#ifdef SUPPORT_MENU
        if (Menu_is_process()) {
            Menu_action_proc(MENU_AC_DOWN);
        } else
#endif
        {
            Disp_info_switch();
        }
        Sys_event_post(SYSTEMAPP_EVT_DISP);
        break;

        case DEVICES_OFF_MODE:
        Led_ctrl(LED_R, 1, 500 * CLOCK_UNIT_MS, 1);
        // Sys_event_post(SYSTEMAPP_EVT_DISP);
        break;

        case DEVICES_SLEEP_MODE:
        Disp_poweron();
        deviceMode = DEVICES_ON_MODE;
        Sys_event_post(SYSTEMAPP_EVT_DISP);
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
    switch(deviceMode)
    {
        case DEVICES_ON_MODE:
        Disp_poweroff();
        S6Sleep();
        Led_ctrl(LED_R, 1, 250 * CLOCK_UNIT_MS, 6);
        Sys_event_post(SYSTEMAPP_EVT_DISP);
        break;

        case DEVICES_OFF_MODE:
        Battery_voltage_measure();
        if(Battery_get_voltage() > g_rSysConfigInfo.batLowVol)
        {
            Led_ctrl(LED_B, 1, 250 * CLOCK_UNIT_MS, 6);
            deviceMode = DEVICES_ON_MODE;
            S6Wakeup();
            Disp_poweron();
            Disp_info_close();
            Sys_event_post(SYSTEMAPP_EVT_DISP);
        }
        else
        {
            Led_ctrl(LED_R, 1, 500 * CLOCK_UNIT_MS, 1);
        }
        break;

        case DEVICES_SLEEP_MODE:
        Disp_poweron();
        deviceMode = DEVICES_ON_MODE;
        Sys_event_post(SYSTEMAPP_EVT_DISP);
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
#ifdef BOARD_S6_6
    switch(deviceMode)
    {
        case DEVICES_ON_MODE:
        Disp_poweron();
#ifdef SUPPORT_MENU
        if (Menu_is_process()) {
            Menu_action_proc(MENU_AC_ENTER);
        } else
#endif
        {
            Disp_sensor_switch();
        }
        Sys_event_post(SYSTEMAPP_EVT_DISP);
        break;

        case DEVICES_OFF_MODE:
        Led_ctrl(LED_R, 1, 500 * CLOCK_UNIT_MS, 1);
        // Sys_event_post(SYSTEMAPP_EVT_DISP);
        break;

        case DEVICES_SLEEP_MODE:
        Disp_poweron();
        deviceMode = DEVICES_ON_MODE;
        Sys_event_post(SYSTEMAPP_EVT_DISP);
        break;

    }
#endif
}



//***********************************************************************************
// brief:the Concenter long key application
// 
// parameter: 
//***********************************************************************************
void S6LongKey1App(void)
{
    switch(deviceMode)
    {
        case DEVICES_ON_MODE:
        Disp_poweron();
        PoweroffMenu_init();
        Sys_event_post(SYSTEMAPP_EVT_DISP);
        break;

        case DEVICES_OFF_MODE:
        Led_ctrl(LED_B, 1, 250 * CLOCK_UNIT_MS, 6);
        Battery_voltage_measure();
        if(Battery_get_voltage() > g_rSysConfigInfo.batLowVol)
        {
            deviceMode = DEVICES_ON_MODE;
            S6Wakeup();
            Disp_poweron();
            Disp_info_close();
            Sys_event_post(SYSTEMAPP_EVT_DISP);
        }
        break;


        case DEVICES_SLEEP_MODE:
        Disp_poweron();
        deviceMode = DEVICES_ON_MODE;
        Sys_event_post(SYSTEMAPP_EVT_DISP);
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
#ifdef S_G//网关
    Battery_porcess();
#endif //S_G//网关
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
            S6Sleep();

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
            S6Wakeup();
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

//***********************************************************************************
// brief:   S6 wakeup enable the rtc and the radio function
// 
// parameter: 
//***********************************************************************************
void S6Wakeup(void)
{
    deviceMode = DEVICES_ON_MODE;
    RtcStart();
    Flash_log("PON\n");
    if(GetUsbState() == USB_UNLINK_STATE)
    {
#ifdef  SUPPORT_NETWORK
        Nwk_poweron();
#endif
    }

#ifdef S_G//网关
    ConcenterWakeup();
    
#ifdef SUPPORT_FREQ_FIND
    AutoFreqInit();
#endif  // SUPPORT_FREQ_FIND

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
void S6Sleep(void)
{
    RtcStop();
    Flash_log("POF\n");
    // wait the nwk disable the uart
#ifdef  SUPPORT_NETWORK
    Nwk_poweroff();
    while(Nwk_is_Active())
        Task_sleep(100 * CLOCK_UNIT_MS);
#endif

#ifndef S_A
    ConcenterSleep();
#endif // S_A

    deviceMode = DEVICES_OFF_MODE;
}


#endif
