/*
* @Author: justfortest
* @Date:   2018-03-09 11:15:03
* @Last Modified by:   zxt
* @Last Modified time: 2020-06-19 10:37:09
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
}


//***********************************************************************************
//
// System buzzer disable.
//
//***********************************************************************************
void Sys_buzzer_disable(void)
{
    PWM_stop(buzzerHandle);
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
        Sys_event_post(SYS_EVT_ALARM);
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

}


#ifdef  SUPPORT_CHARGE_DECT_ALARM
void Sys_chagre_alarm_timer_isr(void)
{
    g_ChagerAlarmCnt ++;
    if(g_ChagerAlarmCnt > 20)
      {
        g_ChagerAlarmCnt = 0;
        if((Get_Charge_plug() == NO_CHARGE)&&
           ((STATUS_CHAGE_ALARM_SWITCH_ON&g_rSysConfigInfo.status)&&
           ( !(STATUS_ALARM_OFF&g_rSysConfigInfo.status))))
          {
            g_bAlarmSensorFlag |= ALARM_CHARGE_DECT_ALARM;
            Sys_event_post(SYS_EVT_ALARM);
          }
          else if(g_bAlarmSensorFlag & ALARM_CHARGE_DECT_ALARM){

                g_bAlarmSensorFlag ^= ALARM_CHARGE_DECT_ALARM;
          }
      }
}
#endif




//===================================
// sys_Node_Lose_Alarm
//=============================
void sys_Node_Lose_Alarm(void){
     Sys_event_post(SYS_EVT_ALARM);
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
        //Disp_poweroff();
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

    I2c_init();
    I2c_init_2();

	KeyInit();
	power_Key_init();
	AdcDriverInit();

    Disp_init();

    Spi_init();


    Flash_init();
    //Menu_init_byflash();
    PwmDriverInit();
    Sys_buzzer_init();

    UsbIntInit(SystemUsbIntEventPostIsr);

    Battery_init();
    Battery_voltage_measure();

    g_rSysConfigInfo.module |= MODULE_CC1310;
    g_rSysConfigInfo.rfBW   = FREQ_434_50;

    if(!(g_rSysConfigInfo.module & MODULE_LCD)){//
        Clock_setPeriod(sysLcdShutClkHandle, 60*CLOCK_UNIT_S);
        Clock_setTimeout(sysLcdShutClkHandle,  60*CLOCK_UNIT_S);
        Clock_start(sysLcdShutClkHandle); 
    }


    Disp_poweron();
    Sys_event_post(SYSTEMAPP_EVT_DISP);
}


uint8_t testRadio = 0;

//***********************************************************************************
// brief:S6 measure the bat
// 
// parameter: 
//***********************************************************************************
void S6AppBatProcess(void)
{
    Battery_porcess();

    if((Battery_get_voltage()<= BAT_VOLTAGE_L1) && (Clock_isActive(BatAlarmClkHandle) == FALSE) && 
        ((deviceMode == DEVICES_SLEEP_MODE) || (deviceMode == DEVICES_ON_MODE))) {
        Clock_start(BatAlarmClkHandle);
    }
    
    if((Battery_get_voltage()> BAT_VOLTAGE_L1) && (Clock_isActive(BatAlarmClkHandle) == TRUE)) {
        Clock_stop(BatAlarmClkHandle);
    }
    if(testRadio){
        // testRadio = 0;
        //RadioCmdSetWithNoRes_Groud(RADIO_CMD_GROUD_SHOCK_TYPE);
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
    //Flash_log("PON\n");
#ifdef SUPPORT_DEVICED_STATE_UPLOAD
    //Flash_store_devices_state(TYPE_POWER_ON);
#endif //SUPPORT_DEVICED_STATE_UPLOAD

    ConcenterWakeup();

}



//***********************************************************************************
// brief:   S6 wakeup enable the rtc and the radio function
// 
// parameter: 
//***********************************************************************************
void S6Sleep(void)
{
    RtcStop();
    //Flash_log("POF\n");
#ifdef SUPPORT_DEVICED_STATE_UPLOAD
    //Flash_store_devices_state(TYPE_POWER_DOWN);
#endif //SUPPORT_DEVICED_STATE_UPLOAD
    // wait the nwk disable the uart
    Disp_clear_all();
    Disp_msg(2, 3, "Power Off...", FONT_8X16);//display

    ConcenterSleep();

    deviceMode = DEVICES_OFF_MODE;
    if(Clock_isActive(BatAlarmClkHandle) == TRUE)
        Clock_stop(BatAlarmClkHandle);
}

void S6KeyApp(void)
{
    KEY_CODE_E keyCode;
    //RadioCmdSetWithNoRes_Groud(RADIO_CMD_GROUD_SHOCK_TYPE);
    keyCode = Key_get();

    //if(get_menu_alarmOrSetting())
    //menuc_alarm_main(keyCode);
   //else
    g_bAlarmSensorFlag = 0;
    menuc_main(keyCode);


}



#endif
