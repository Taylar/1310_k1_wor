/*
* @Author: justfortest
* @Date:   2018-03-09 11:13:28
* @Last Modified by:   zxt
* @Last Modified time: 2020-06-29 15:10:56
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
    //-----KeyInit();

    Spi_init();

#ifdef SUPPORT_SHT3X
    //SHT3X Reset Pin initial
    SHT3x_ResetIoInitial();
#endif

    AdcDriverInit();

    I2c_init();

    PwmDriverInit();

    Flash_init();

    ElectricShockInit();

    SoundDriverInit();

    Battery_init();

    Battery_voltage_measure();
    
    configModeTimeCnt = 0;

    g_rSysConfigInfo.rfStatus       |= STATUS_1310_MASTER;
    g_rSysConfigInfo.rfBW            = FREQ_434_50;

    SoundEventSet(SOUND_TYPE_VOLUME_MAX);
}


uint32_t lowBatCnt = 0;
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

    if(g_rSysConfigInfo.electricFunc & ELE_FUNC_ENABLE_PREVENT_INSERT){
        ElecPreventInsertMeasure();
        if(ElecPreventInsertState()){
            EletricPulseSetTime_S(1);
            // RadioCmdSetWithNoResponBrocast(RADIO_CMD_INSERT_TYPE, RADIO_CONTROLER_ADDRESS);
            RadioCmdSetWithNoRes(RADIO_CMD_INSERT_TYPE, RADIO_CONTROLER_ADDRESS);
            SoundEventSet(SOUND_TYPE_INSERT);
        }
    }

    if(g_rSysConfigInfo.electricFunc & ELE_FUNC_ENABLE_PREVENT_ESCAPE){
        escapeTimeCnt++;
        if(escapeTimeCnt == 10){
            SoundEventSet(SOUND_TYPE_DI_DI_DI);
        }
        if(escapeTimeCnt == 20){
            EletricPulseSetTime_S(2);
            SoundEventSet(SOUND_TYPE_DI_DI_DI);
        }
        if(escapeTimeCnt == 30){
            EletricPulseSetTime_S(6);
            SoundEventSet(SOUND_TYPE_DI_DI_DI);
        }
        if(escapeTimeCnt > 36){
            SoundEventSet(SOUND_TYPE_DI_DI_DI);
        }
    }


    Battery_porcess();
    if(Battery_get_voltage() < 3600){
        if((lowBatCnt == 0) || (lowBatCnt >= 30)){
            SoundEventSet(SOUND_TYPE_LOW_BAT);
            lowBatCnt = 1;
            // RadioCmdSetWithNoResponBrocast(RADIO_CMD_LOW_VOL_TYPE, RADIO_CONTROLER_ADDRESS);
            RadioCmdSetWithNoRes(RADIO_CMD_LOW_VOL_TYPE, RADIO_CONTROLER_ADDRESS);
        }
        lowBatCnt++;
    }else{
        lowBatCnt = 0;
    }


    destroyEleShock = DestroyPinRead();
    if(destroyEleShock){
        if(destroyEleShock){
            EletricPulseSetTime_S(1);
            // RadioCmdSetWithNoResponBrocast(RADIO_CMD_DESTROY_TYPE, RADIO_CONTROLER_ADDRESS);
            RadioCmdSetWithNoRes(RADIO_CMD_DESTROY_TYPE, RADIO_CONTROLER_ADDRESS);
            SoundEventSet(SOUND_TYPE_DESTROYED);
        } 
    }

    // for test
    // RadioCmdSetWithNoRes(RADIO_PRO_CMD_ALL_RESP, RADIO_CONTROLER_ADDRESS);
    // ElectricShockLevelSet(0);
    // ElectricShockLevelSet(1);
    // ElectricShockLevelSet(2);
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
