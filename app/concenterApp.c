/*
* @Author: zxt
* @Date:   2017-12-28 10:09:45
* @Last Modified by:   zxt
* @Last Modified time: 2018-01-31 15:35:46
*/
#include "../general.h"

#include "../radio_app/radio_app.h"
#include "../APP/concenterApp.h"
#include "../APP/systemApp.h"
#include "../APP/radio_protocal.h"
#include "../interface_app/interface.h"





#define     NODE_SETTING_CMD_LENGTH    10

#define     CONCENTER_RADIO_MONITOR_CNT_MAX     10


#define     SCREEN_SLEEP_TIME           15
/***** Type declarations *****/
typedef struct 
{
    uint32_t channelDispath;
    uint8_t  monitorCnt;
    uint8_t  screenSleepMonitorCnt;
    
    bool  configFlag;    // 0: unload the config; 1: has load the config
    bool  synTimeFlag;    // 0: unsyntime; 1: synchron time

}concenter_para_t;





/***** Variable declarations *****/

concenter_para_t concenterParameter;


/* Clock for node period collect */
static Clock_Struct concenterUploadClock;     /* not static so you can see in ROV */
static Clock_Handle concenterUploadClockHandle;


extflash_queue_s extflashWriteQ;
/***** Prototypes *****/




/***** Function definitions *****/

//***********************************************************************************
// brief:   set the concenter upload event
// 
// parameter: 
//***********************************************************************************
static void ConcenterUploadTimerCb(UArg arg0)
{
    Event_post(systemAppEvtHandle, SYSTEMAPP_EVT_NET_UPLOAD);
}


//***********************************************************************************
// brief:   init the concenter upload timer
// 
// parameter: 
//***********************************************************************************
void ConcenterAppInit(void)
{
    Clock_Params clkParams;
    Clock_Params_init(&clkParams);
    clkParams.period    = 0;
    clkParams.startFlag = FALSE;
    Clock_construct(&concenterUploadClock, ConcenterUploadTimerCb, 1, &clkParams);
    concenterUploadClockHandle = Clock_handle(&concenterUploadClock);

    concenterParameter.channelDispath = 0;
    concenterParameter.synTimeFlag    = 0;
    concenterParameter.monitorCnt     = 0;
    concenterParameter.configFlag     = InternalFlashLoadConfig();

    InternalFlashInit();

    ExtflashRingQueueInit(&extflashWriteQ);

    SetRadioSrcAddr(DEFAULT_DST_ADDR);

    ConcenterSleep();
}

//***********************************************************************************
// brief:   
// 
// parameter: 
//***********************************************************************************
void ConcenterAppHwInit(void)
{

    Spi_init();

    Flash_init();


#ifdef BOARD_S2_2
    DeepTemp_FxnTable.initFxn(MAX31855_SPI_CH0);
#endif

#ifdef BOARD_S6_6
    NTC_FxnTable.initFxn(NTC_CH0);
#endif


}



//***********************************************************************************
// brief:   start the upload timer
// 
// parameter: 
//***********************************************************************************
void ConcenterUploadStart(void)
{
    if(Clock_isActive(concenterUploadClockHandle) == false)
        Clock_start(concenterUploadClockHandle);
}


//***********************************************************************************
// brief:   stop the upload timer
// 
// parameter: 
//***********************************************************************************
void ConcenterUploadStop(void)
{
    if(Clock_isActive(concenterUploadClockHandle))
        Clock_stop(concenterUploadClockHandle);
}

//***********************************************************************************
// brief:   set the upload timer period
// 
// parameter
// period:  the uint is ms
//***********************************************************************************
void ConcenterUploadPeriodSet(uint32_t period)
{
    Clock_setPeriod(concenterUploadClockHandle, period * CLOCK_UNIT_MS);
}


//***********************************************************************************
// brief:  store the sensor data to queue
// 
// parameter: 
//***********************************************************************************
bool ConcenterSensorDataSaveToQueue(uint8_t *dataP, uint8_t length)
{
    if(ExtflashRingQueuePush(&extflashWriteQ, dataP) == true)
    {
        Event_post(systemAppEvtHandle, SYSTEMAPP_EVT_STORE_CONCENTER);
        return true;
    }
    else
    {
        Event_post(systemAppEvtHandle, SYSTEMAPP_EVT_STORE_CONCENTER);
        return false;
    }
}

//***********************************************************************************
// brief:  save the sensor data to extflash
// 
// parameter: 
//***********************************************************************************
void ConcenterSensorDataSave(void)
{
    uint8_t dataP[SENSOR_DATA_LENGTH_MAX];

    if(ExtflashRingQueuePoll(&extflashWriteQ, dataP) == true)
    {
        Flash_store_sensor_data(dataP, SENSOR_DATA_LENGTH_MAX);
        Event_post(systemAppEvtHandle, SYSTEMAPP_EVT_STORE_CONCENTER);
        Event_post(systemAppEvtHandle, SYSTEMAPP_EVT_NET_UPLOAD);
    }
}


//***********************************************************************************
// brief:   concenter upload the data to the internet
// 
// parameter: 
//***********************************************************************************
void ConcenterSensorDataUpload(void)
{
    uint8_t     data[24];
    uint32_t    dataItems;
    uint8_t     offsetUnit;
    //reverse the buf to other command
    offsetUnit = 0;
    dataItems  = Flash_get_unupload_items();
    
    while(dataItems)
    {
        Flash_load_sensor_data(data, 22, dataItems);

        // upload the data to network 
        
        InterfaceSend(data, 32);
        dataItems--;
        offsetUnit++;
        
    }
}       


//***********************************************************************************
// brief: seach the Node parameter setting table to updata the specify node parasetting
// 
// parameter: 
//***********************************************************************************
void ConcenterUpdataNodeSetting(uint32_t srcAddr, uint32_t dstAddr)
{
    // search the table to updata the parameter setting


    // 
    // ConcenterRadioSendParaSet(srcAddr, dstAddr, NODE_SETTING_CMD, NODE_SETTING_CMD_LENGTH);
}

//***********************************************************************************
// brief: seach the Node parameter setting table to update the table updata flag
// 
// parameter: 
//***********************************************************************************
void ConcenterNodeSettingSuccess(uint32_t srcAddr, uint32_t dstAddr)
{
    // search the table to clear the special node parameter seeting 
}


//***********************************************************************************
// brief: Set the concenter upload event to upload the sensor data to internet
// 
// parameter: 
//***********************************************************************************
void ConcenterUploadEventSet(void)
{
    Event_post(systemAppEvtHandle, SYSTEMAPP_EVT_NET_UPLOAD);
}


//***********************************************************************************
// brief:   make the concenter board into sleep mode
// 
// parameter: 
//***********************************************************************************
void ConcenterSleep(void)
{
    if(concenterParameter.configFlag)
    {
        Nwk_poweroff();
        EasyLink_abort();
        RadioFrontDisable();
    }
    deviceMode = DEVICES_OFF_MODE;
}

//***********************************************************************************
// brief:   make the Concenter board into work mode
// 
// parameter: 
//***********************************************************************************
void ConcenterWakeup(void)
{
    deviceMode = DEVICES_ON_MODE;
    if(concenterParameter.configFlag)
    {
        RadioFrontRxEnable();
        Nwk_poweron();
        RadioFrontRxEnable();
        EasyLink_setCtrl(EasyLink_Ctrl_AsyncRx_TimeOut, 0);
        RadioModeSet(RADIOMODE_RECEIVEPORT);
    }
}


//***********************************************************************************
// brief:   save the node addr and channel to the internal flash
// 
// parameter: 
//***********************************************************************************
void ConcenterSaveChannel(uint32_t nodeAddr)
{
    if(InternalFlashSaveNodeAddr(nodeAddr, &concenterParameter.channelDispath))
        concenterParameter.channelDispath++;
}

//***********************************************************************************
// brief:   read the node channel from the internal flash according to the node addr
// 
// parameter: 
//***********************************************************************************
uint32_t ConcenterReadChannel(uint32_t nodeAddr)
{
    return InternalFlashReadNodeAddr(nodeAddr);
}



//***********************************************************************************
// brief:the Concenter short key application
// 
// parameter: 
//***********************************************************************************
void ConcenterShortKeyApp(void)
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
void ConcenterLongKeyApp(void)
{
    switch(deviceMode)
    {
        case DEVICES_ON_MODE:
        ConcenterSleep();
        Led_ctrl(LED_R, 1, 250 * CLOCK_UNIT_MS, 6);
        break;

        case DEVICES_OFF_MODE:
        Led_ctrl(LED_B, 1, 250 * CLOCK_UNIT_MS, 6);
        ConcenterWakeup();
        break;
    }
}

//***********************************************************************************
// brief:the S6 Concenter short key application
// 
// parameter: 
//***********************************************************************************
void S6ConcenterShortKeyApp(void)
{
    concenterParameter.screenSleepMonitorCnt = 0;
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
    concenterParameter.screenSleepMonitorCnt = 0;
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
void S6ConcenterShortKey1App(void)
{
    concenterParameter.screenSleepMonitorCnt = 0;
    switch(deviceMode)
    {
        case DEVICES_ON_MODE:
        Disp_info_close();
        Disp_proc();
        break;

        case DEVICES_OFF_MODE:
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
void S6ConcenterLongKey1App(void)
{
    concenterParameter.screenSleepMonitorCnt = 0;
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
// brief:save the config to internal flash
// 
// parameter: 
//***********************************************************************************
void ConcenterStoreConfig(void)
{
    InternalFlashStoreConfig();
    concenterParameter.configFlag   = 1;
}

//***********************************************************************************
// brief:
// 
// parameter: 
//***********************************************************************************
void ConcenterTimeSychronization(Calendar *currentTime)
{
    Rtc_set_calendar(currentTime);
    concenterParameter.synTimeFlag  = 1;
}

//***********************************************************************************
// brief:
// 
// parameter: 
//***********************************************************************************
uint8_t ConcenterReadSynTimeFlag(void)
{
    return concenterParameter.synTimeFlag;
}


//***********************************************************************************
// brief:
// 
// parameter: 
//***********************************************************************************
void ConcenterRadioMonitor(void)
{
    concenterParameter.monitorCnt++;
    if(concenterParameter.monitorCnt >= CONCENTER_RADIO_MONITOR_CNT_MAX)
    {
        if(concenterParameter.configFlag)
        {
            EasyLink_abort();
            RadioFrontDisable();
            RadioFrontRxEnable();
            EasyLink_setCtrl(EasyLink_Ctrl_AsyncRx_TimeOut, 0);
            RadioModeSet(RADIOMODE_RECEIVEPORT);
        }
    }
}


//***********************************************************************************
// brief:
// 
// parameter: 
//***********************************************************************************
void ConcenterRadioMonitorClear(void)
{
    concenterParameter.monitorCnt = 0;
}


//***********************************************************************************
// brief:
// 
// parameter: 
//***********************************************************************************
void ScreenSleepMonitor(void)
{
    if(deviceMode == DEVICES_SLEEP_MODE)
        return;
    concenterParameter.screenSleepMonitorCnt ++;
    if(concenterParameter.screenSleepMonitorCnt >= SCREEN_SLEEP_TIME)
    {
        Disp_poweroff();
        deviceMode = DEVICES_SLEEP_MODE;
    }
}



