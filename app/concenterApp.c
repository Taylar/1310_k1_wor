/*
* @Author: zxt
* @Date:   2017-12-28 10:09:45
* @Last Modified by:   zxt
* @Last Modified time: 2018-03-15 14:13:46
*/
#include "../general.h"

#include "../radio_app/radio_app.h"
#include "../APP/concenterApp.h"
#include "../APP/systemApp.h"
#include "../APP/radio_protocal.h"
#include "../interface_app/interface.h"





#define     NODE_SETTING_CMD_LENGTH    10

#define     CONCENTER_RADIO_MONITOR_CNT_MAX     10



/***** Type declarations *****/
typedef struct 
{
    uint32_t channelDispath;
    uint32_t synchronTimeCnt;
    uint32_t uploadPeriod;          // the unit is sec
    uint32_t uploadTimeCnt;          // the unit is sec
    uint32_t collectPeriod;         // the unit is sec
    uint32_t collectTimeCnt;         // the unit is sec
    uint32_t serialNum;         // the unit is sec
    
    uint8_t  monitorCnt;
    
    bool  synTimeFlag;    // 0: unsyntime; 1: synchron time
    bool  collectStart;    // 0: stop collect data; 1: start collect data
    bool  radioReceive;    // 0: stop receive radio; 1: continue receive radio

}concenter_para_t;





/***** Variable declarations *****/

concenter_para_t concenterParameter;




extflash_queue_s extflashWriteQ;
/***** Prototypes *****/




/***** Function definitions *****/



//***********************************************************************************
// brief:   init the concenter upload timer
// 
// parameter: 
//***********************************************************************************
void ConcenterAppInit(void)
{

    concenterParameter.channelDispath  = 0;
    concenterParameter.monitorCnt      = 0;
    concenterParameter.synchronTimeCnt = 0;
    concenterParameter.collectPeriod   = UPLOAD_PERIOD_DEFAULT;
    concenterParameter.collectTimeCnt  = 0;
    concenterParameter.serialNum       = 0;

    concenterParameter.synTimeFlag     = false;
    concenterParameter.collectStart     = false;
    concenterParameter.radioReceive    = false;

    InternalFlashInit();

    ExtflashRingQueueInit(&extflashWriteQ);

#ifdef  BOARD_CONFIG_DECEIVE

    SetRadioSrcAddr(CONFIG_DECEIVE_ID_DEFAULT);
    SetRadioSubSrcAddr(CONFIG_DECEIVE_ID_DEFAULT);

#else

    SetRadioSrcAddr( (((uint32_t)(g_rSysConfigInfo.DeviceId[0])) << 24) |
                     (((uint32_t)(g_rSysConfigInfo.DeviceId[1])) << 16) |
                     (((uint32_t)(g_rSysConfigInfo.DeviceId[2])) << 8) |
                     g_rSysConfigInfo.DeviceId[3]);
    SetRadioSubSrcAddr(0xffff0000 | (g_rSysConfigInfo.customId[0] << 8) | g_rSysConfigInfo.customId[1]);

#endif

    // ConcenterSleep();
}




//***********************************************************************************
// brief:   start the upload timer
// 
// parameter: 
//***********************************************************************************
void ConcenterUploadStart(void)
{
}


//***********************************************************************************
// brief:   stop the upload timer
// 
// parameter: 
//***********************************************************************************
void ConcenterUploadStop(void)
{
}

//***********************************************************************************
// brief:   set the upload timer period
// 
// parameter
// period:  the uint is ms
//***********************************************************************************
void ConcenterUploadPeriodSet(uint32_t period)
{

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
    concenterParameter.radioReceive = false;
    Nwk_poweroff();
    EasyLink_abort();
    RadioFrontDisable();
    ConcenterCollectStop();
    // wait the nwk disable the uart
    while(Nwk_get_state())
        Task_sleep(100 * CLOCK_UNIT_MS);
#ifdef BOARD_S2_2

    InterfaceEnable();

#endif
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
#ifdef BOARD_S2_2
    InterfaceDisable();
#endif
    concenterParameter.radioReceive = true;
#ifdef BOARD_S6_6
    if(GetUsbState() == USB_UNLINK_STATE)
#endif
    {
        Nwk_poweron();
    }
    RadioFrontRxEnable();
    EasyLink_setCtrl(EasyLink_Ctrl_AsyncRx_TimeOut, 0);
    RadioModeSet(RADIOMODE_RECEIVEPORT);


// for test
    // Calendar currentTime = {0,0,10,3,14,3,2018};
    // ConcenterTimeSychronization(&currentTime);
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
            case DEVICES_MENU_MODE:
            case DEVICES_SLEEP_MODE:
            Nwk_poweroff();
            Disp_poweroff();
            Task_sleep(100 * CLOCK_UNIT_MS);
            // wait for the gsm uart close
            while(Nwk_get_state())
                Task_sleep(100 * CLOCK_UNIT_MS);

            InterfaceEnable();

            RadioTestDisable();

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
        switch(deviceMode)
        {
            case DEVICES_ON_MODE:
            case DEVICES_MENU_MODE:
            case DEVICES_SLEEP_MODE:
            Nwk_poweron();
            
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
// brief:   Init the board as the config deceive
// 
// parameter: 
//***********************************************************************************
void ConcenterConfigDeceiveInit(void)
{
    Nwk_poweroff();

    deviceMode = DEVICES_CONFIG_MODE;
    concenterParameter.radioReceive = true;
    RadioFrontRxEnable();
    EasyLink_setCtrl(EasyLink_Ctrl_AsyncRx_TimeOut, 0);
    RadioModeSet(RADIOMODE_RECEIVEPORT);
    InterfaceEnable();
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
// brief:save the config to internal flash
// 
// parameter: 
//***********************************************************************************
void ConcenterStoreConfig(void)
{
    InternalFlashStoreConfig();
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
void ConcenterRadioMonitorClear(void)
{
    concenterParameter.monitorCnt = 0;
}

//***********************************************************************************
// brief:   start the collect sensor timer
// 
// parameter: 
//***********************************************************************************
void ConcenterCollectStart(void)
{
    uint8_t i;
    for(i = 0; i < MODULE_SENSOR_MAX; i++)
    {
        if((g_rSysConfigInfo.sensorModule[i] == SEN_TYPE_SHT2X) || (g_rSysConfigInfo.sensorModule[i] == SEN_TYPE_SHT2X))
            concenterParameter.collectStart      = true;
        break;
    }
}


//***********************************************************************************
// brief:   stop the collect sensor timer
// 
// parameter: 
//***********************************************************************************
void ConcenterCollectStop(void)
{
    concenterParameter.collectStart      = false;
}


//***********************************************************************************
// brief:   set the collect sensor timer period
// 
// parameter: 
// period:  the uint is sec
//***********************************************************************************
void ConcenterCollectPeriodSet(uint32_t period)
{
    concenterParameter.collectPeriod         = period;
}

//***********************************************************************************
// brief:   concenter collect the sensor data and save to extflash
// 
// parameter: 
//***********************************************************************************
void ConcenterCollectProcess(void)
{
    uint8_t     data[24];
    uint32_t    temp;
    Calendar    calendarTemp;

#ifdef BOARD_S2_2
    // save the deep temperature data
    DeepTemp_FxnTable.measureFxn(MAX31855_SPI_CH0);
        // sensor type
    data[17] = PARATYPE_TEMP_MAX31855;

    // length, note:do not include length self
    data[0] = 20;

    // sensor data
    temp     = DeepTemp_FxnTable.getValueFxn(MAX31855_SPI_CH0, SENSOR_DEEP_TEMP);
    data[18] = (uint8_t)(temp >> 16);
    data[19] = (uint8_t)(temp >> 8);
    data[20] = (uint8_t)(temp);
#endif


#ifdef BOARD_S6_6
    // save the deep temperature data
    NTC_FxnTable.measureFxn(NTC_CH0);
        // sensor type
    data[17] = PARATYPE_NTC;

    // length, note:do not include length self
    data[0] = 19;

    // sensor data
    temp     = NTC_FxnTable.getValueFxn(NTC_CH0, SENSOR_TEMP);
    data[18] = (uint8_t)(temp >> 8);
    data[19] = (uint8_t)(temp);
#endif    
    

    calendarTemp    = Rtc_get_calendar();
    
    // rssi
    data[1] = 0;

    // deceive ID
    data[2] = g_rSysConfigInfo.DeviceId[0];
    data[3] = g_rSysConfigInfo.DeviceId[1];
    data[4] = g_rSysConfigInfo.DeviceId[2];
    data[5] = g_rSysConfigInfo.DeviceId[3];
    
    // serial num
    data[6] = (uint8_t)(concenterParameter.serialNum>>8);
    data[7] = (uint8_t)concenterParameter.serialNum;
    
    // collect time
    data[8]  = TransHexToBcd((uint8_t)(calendarTemp.Year - 2000));
    data[9]  = TransHexToBcd((uint8_t)(calendarTemp.Month));
    data[10] = TransHexToBcd((uint8_t)(calendarTemp.DayOfMonth));
    data[11] = TransHexToBcd((uint8_t)(calendarTemp.Hours));
    data[12] = TransHexToBcd((uint8_t)(calendarTemp.Minutes));
    data[13] = TransHexToBcd((uint8_t)(calendarTemp.Seconds));

    // voltage
    temp     = Battery_get_voltage();
    data[14] = (uint8_t)(temp >> 8);
    data[15] = (uint8_t)(temp);

    // sensor id
    data[16] = 0;


    Flash_store_sensor_data(data, data[0]+1);

    concenterParameter.serialNum++;

#ifdef  BOARD_S6_6
    sensor_unpackage_to_memory(data, data[0]+1);
#endif
}



//***********************************************************************************
// brief:the concenter rtc process
// 
// parameter: 
//***********************************************************************************
void ConcenterRtcProcess(void)
{

    if(concenterParameter.radioReceive)
    {
        concenterParameter.monitorCnt++;
        if(concenterParameter.monitorCnt >= CONCENTER_RADIO_MONITOR_CNT_MAX)
        {
            ConcenterRadioMonitorClear();
            EasyLink_abort();
            RadioFrontDisable();
            RadioFrontRxEnable();
            EasyLink_setCtrl(EasyLink_Ctrl_AsyncRx_TimeOut, 0);
            RadioModeSet(RADIOMODE_RECEIVEPORT);
        }
    }

    Nwk_upload_time_isr();

    if(concenterParameter.collectStart)
    {
        concenterParameter.collectTimeCnt++;
        if(concenterParameter.collectTimeCnt >= concenterParameter.collectPeriod)
        {
            concenterParameter.collectTimeCnt = 0;
            ConcenterCollectProcess();
        }
    }

}




