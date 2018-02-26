/*
* @Author: zxt
* @Date:   2017-12-28 10:09:45
* @Last Modified by:   zxt
* @Last Modified time: 2018-02-26 10:25:07
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
    uint32_t synchronTimeCnt;
    uint32_t uploadPeriod;          // the unit is sec
    uint32_t uploadTimeCnt;          // the unit is sec
    uint32_t collectPeriod;         // the unit is sec
    uint32_t collectTimeCnt;         // the unit is sec
    uint32_t serialNum;         // the unit is sec
    
    uint8_t  monitorCnt;
    uint8_t  screenSleepMonitorCnt;
    
    bool  configFlag;    // 0: unload the config; 1: has load the config
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
    if(concenterParameter.configFlag)
    {
        concenterParameter.radioReceive = false;
        Nwk_poweroff();
        EasyLink_abort();
        RadioFrontDisable();
        ConcenterCollectStop();
        // wait the nwk disable the uart
        Task_sleep(500 * CLOCK_UNIT_MS);
    }
    InterfaceEnable();
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
    InterfaceDisable();
    if(concenterParameter.configFlag)
    {
        concenterParameter.radioReceive = true;
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
        InterfaceSend("zxt test", 9);
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
    concenterParameter.collectStart      = true;
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
    data[8] = TransHexToBcd((uint8_t)(calendarTemp.Year - 2000));
    data[9] = TransHexToBcd((uint8_t)(calendarTemp.Month));
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
}



//***********************************************************************************
// brief:the concenter rtc process
// 
// parameter: 
//***********************************************************************************
void ConcenterRtcProcess(void)
{
    
#ifdef BOARD_S6_6
    if(Disp_powerState())
    {
        concenterParameter.screenSleepMonitorCnt ++;
        if(concenterParameter.screenSleepMonitorCnt >= SCREEN_SLEEP_TIME)
        {
            Disp_poweroff();
            deviceMode = DEVICES_SLEEP_MODE;
        }
    }
#endif

    if(concenterParameter.radioReceive)
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

    Nwk_ntp_syn();

    if(concenterParameter.collectStart)
    {
        concenterParameter.collectTimeCnt++;
        if(concenterParameter.collectTimeCnt >= concenterParameter.collectPeriod)
        {
            concenterParameter.collectTimeCnt = 0;
            ConcenterCollectProcess();
            ConcenterUploadEventSet();
        }
    }

}





#ifdef SUPPORT_NETGATE_DISP_NODE

sensordata_mem pMemSensor[MEMSENSOR_NUM];//  
uint8_t MemSensorIndex = 0;

//***********************************************************************************
//
// unpackage  sensor data and save  sensor mac , index, type and value to mem
//
//***********************************************************************************
void sensor_unpackage_to_memory(uint8_t *pData, uint16_t length)
{    
    uint8_t i;
    uint16_t Index;    
    sensordata_mem cursensor;
    
    Index = 2;//DeviceId  start

    HIBYTE(HIWORD(cursensor.DeviceId)) = pData[Index++];
    LOBYTE(HIWORD(cursensor.DeviceId)) = pData[Index++];
    HIBYTE(LOWORD(cursensor.DeviceId)) = pData[Index++];
    LOBYTE(LOWORD(cursensor.DeviceId)) = pData[Index++];  
    
    Index = 16;//sensor  start

    while(Index < length)
    {
        cursensor.index = pData[Index++];
        cursensor.type  = pData[Index++];
        
        switch(cursensor.type)
        {
            case PARATYPE_TEMP_HUMI_SHT20:
            HIBYTE(cursensor.temp) = pData[Index++];
            LOBYTE(cursensor.temp) = pData[Index++];
            HIBYTE(cursensor.humi) = pData[Index++];
            LOBYTE(cursensor.humi) = pData[Index++];
            break;

            case PARATYPE_NTC:
            HIBYTE(cursensor.temp) = pData[Index++];
            LOBYTE(cursensor.temp) = pData[Index++];
            break;

            case PARATYPE_ILLUMINATION:
            break;

            case PARATYPE_TEMP_MAX31855:
            HIBYTE(HIWORD(cursensor.tempdeep)) = pData[Index++];
            LOBYTE(HIWORD(cursensor.tempdeep)) = pData[Index++];
            HIBYTE(LOWORD(cursensor.tempdeep)) = pData[Index++];
            cursensor.tempdeep >>= 8;
            break;

        }

        //find in mem 
        for (i = 0; i < MEMSENSOR_NUM; ++i) {
            if (((pMemSensor) + i)->DeviceId == cursensor.DeviceId &&
               ((pMemSensor) + i)->index == cursensor.index &&
               ((pMemSensor) + i)->type == cursensor.type )
                break;

        }
        
        if (i < MEMSENSOR_NUM) {//update
             memcpy((pMemSensor) + i, &cursensor, sizeof(sensordata_mem));
        }
        else {
            //new sensor id
            memcpy((pMemSensor) + MemSensorIndex, &cursensor, sizeof(sensordata_mem));

            MemSensorIndex = (MemSensorIndex + 1) % MEMSENSOR_NUM;
        }
        
        
    }

}

bool get_next_sensor_memory(sensordata_mem *pSensor)
{    
    static uint8_t dispSensorIndex = 0;

restart:
    if (((sensordata_mem*)(pMemSensor) + dispSensorIndex)->DeviceId != 0x00000000 ){//valid data  
        memcpy(pSensor, (sensordata_mem*)(pMemSensor) + dispSensorIndex, sizeof(sensordata_mem));
        dispSensorIndex = (dispSensorIndex + 1) % MEMSENSOR_NUM;
        return true;
    }
    else {
        if(dispSensorIndex == 0)    
            return false;
        else {
            dispSensorIndex = 0;
            goto restart;
        }
    }
}
#endif


