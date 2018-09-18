/*
* @Author: zxt
* @Date:   2017-12-28 10:09:45
* @Last Modified by:   zxt
* @Last Modified time: 2018-09-13 15:17:31
*/
#include "../general.h"






#define     NODE_SETTING_CMD_LENGTH    10

#define     CONCENTER_RADIO_MONITOR_CNT_MAX     60



/***** Type declarations *****/
typedef struct 
{
    uint32_t channelDispath;
    uint8_t  monitorCnt;
    bool     synTimeFlag;    // 0: unsyntime; 1: synchron time
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

    concenterParameter.synTimeFlag     = false;

    ExtflashRingQueueInit(&extflashWriteQ);

#ifdef  BOARD_CONFIG_DECEIVE

    SetRadioSrcAddr(CONFIG_DECEIVE_ID_DEFAULT);
    SetRadioSubSrcAddr(CONFIG_DECEIVE_ID_DEFAULT);

#else

// *******************************for test*************************
    // g_rSysConfigInfo.DeviceId[0] = (uint8_t)((DECEIVE_ID_DEFAULT>>24)&0xff);
    // g_rSysConfigInfo.DeviceId[1] = (uint8_t)((DECEIVE_ID_DEFAULT>>16)&0xff);
    // g_rSysConfigInfo.DeviceId[2] = (uint8_t)((DECEIVE_ID_DEFAULT>>8)&0xff);
    // g_rSysConfigInfo.DeviceId[3] = (uint8_t)((DECEIVE_ID_DEFAULT)&0xff);;

    // g_rSysConfigInfo.customId[0] = (uint8_t)(CUSTOM_ID_DEFAULT >> 8);
    // g_rSysConfigInfo.customId[1] = (uint8_t)(CUSTOM_ID_DEFAULT);
// *******************************

    SetRadioSrcAddr( (((uint32_t)(g_rSysConfigInfo.DeviceId[0])) << 24) |
                     (((uint32_t)(g_rSysConfigInfo.DeviceId[1])) << 16) |
                     (((uint32_t)(g_rSysConfigInfo.DeviceId[2])) << 8) |
                     g_rSysConfigInfo.DeviceId[3]);
    SetRadioSubSrcAddr(0xffff0000 | (g_rSysConfigInfo.customId[0] << 8) | g_rSysConfigInfo.customId[1]);
    SetRadioBrocastSrcAddr(RADIO_BROCAST_ADDRESS);
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
//        Event_post(systemAppEvtHandle, SYSTEMAPP_EVT_STORE_CONCENTER);
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
    uint8_t dataP[FLASH_SENSOR_DATA_SIZE];
    Calendar calendar;


    if(ExtflashRingQueuePoll(&extflashWriteQ, dataP) == true)
    {
        calendar = Rtc_get_calendar();
        dataP[dataP[0]+1] = 0xfe;
        dataP[dataP[0]+2] = 0xfe;
        dataP[dataP[0]+3] = TransHexToBcd((uint8_t)(calendar.Year - 2000));
        dataP[dataP[0]+4] = TransHexToBcd((uint8_t)(calendar.Month));
        dataP[dataP[0]+5] = TransHexToBcd((uint8_t)(calendar.DayOfMonth));
        dataP[dataP[0]+6] = TransHexToBcd((uint8_t)(calendar.Hours));
        dataP[dataP[0]+7] = TransHexToBcd((uint8_t)(calendar.Minutes));
        dataP[dataP[0]+8] = TransHexToBcd((uint8_t)(calendar.Seconds));
        dataP[0]         += 8;

        Flash_store_sensor_data(dataP, FLASH_SENSOR_DATA_SIZE);
        Event_post(systemAppEvtHandle, SYSTEMAPP_EVT_STORE_CONCENTER);
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
        Flash_load_sensor_data_by_offset(data, 22, dataItems);

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
// brief:   make the concenter board into sleep mode
// 
// parameter: 
//***********************************************************************************
void ConcenterSleep(void)
{
    concenterParameter.synTimeFlag  = false;
    RadioDisable();
}

//***********************************************************************************
// brief:   make the Concenter board into work mode
// 
// parameter: 
//***********************************************************************************
void ConcenterWakeup(void)
{
    RadioModeSet(RADIOMODE_RECEIVEPORT);
}


//***********************************************************************************
// brief:   Init the board as the config deceive
// 
// parameter: 
//***********************************************************************************
void ConcenterConfigDeceiveInit(void)
{
#ifdef  SUPPORT_NETWORK
    Nwk_poweroff();
    while(Nwk_is_Active())
    Task_sleep(100 * CLOCK_UNIT_MS);
#endif

    deviceMode = DEVICES_CONFIG_MODE;
    EasyLink_setCtrl(EasyLink_Ctrl_AsyncRx_TimeOut, 0);
    RadioModeSet(RADIOMODE_RECEIVEPORT);
    InterfaceEnable();
}




//***********************************************************************************
// brief:save the config to internal flash
// 
// parameter: 
//***********************************************************************************
void ConcenterStoreConfig(void)
{
    Sys_event_post(SYSTEMAPP_EVT_STORE_SYS_CONFIG);
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
// brief:the concenter rtc process
// 
// parameter: 
//***********************************************************************************
void ConcenterRtcProcess(void)
{

    concenterParameter.monitorCnt++;
    if(concenterParameter.monitorCnt >= CONCENTER_RADIO_MONITOR_CNT_MAX)
    {
        ConcenterRadioMonitorClear();
        EasyLink_abort();
        // RadioSetRxMode();
        SetRadioDstAddr(0xdadadada);
        RadioEventPost(RADIO_EVT_SEND_CONFIG);
    }

#ifdef S_G
    static uint32_t timingSendDataCnt = 0;
    /* Send data regularly to ensure that the gateway can receive*/
    if (timingSendDataCnt > (5 * 60)) {
        RadioModeSet(RADIOMODE_RECEIVEPORT);
        timingSendDataCnt = 0;
        SetRadioDstAddr(0xdadadada);
        ConcenterRadioSendParaSet(0xabababab, 0xbabababa);
    }
    timingSendDataCnt++;
#endif  // S_G

}



#ifdef SUPPORT_STRATEGY_SORT
uint32_t nodeAddrTable[CONCENTER_MAX_CHANNEL];
uint8_t  nodeNumDispath;

//***********************************************************************************
// brief:concenter set the channel
// 
// parameter: 
//***********************************************************************************
uint8_t ConcenterSetNodeChannel(uint32_t nodeAddr)
{
    uint8_t i;
    for(i = 0; i < CONCENTER_MAX_CHANNEL; i++)
    {
        if(nodeAddrTable[i] == nodeAddr)
        {
            return i;
        }
    }
    // could not find the nodeaddr in the nodeAddrTable
    if(nodeNumDispath >= CONCENTER_MAX_CHANNEL)
    {
        memset((uint8_t*)nodeAddrTable, 0, sizeof(nodeAddrTable));
        nodeNumDispath = 0;
    }
    nodeAddrTable[nodeNumDispath] = nodeAddr;
    nodeNumDispath++;
    return (nodeNumDispath - 1);
}

uint8_t ConcenterReadNodeChannel(uint32_t nodeAddr)
{
    uint8_t i; 
    for(i = 0; i < CONCENTER_MAX_CHANNEL; i++)
    {
        if(nodeAddrTable[i] == nodeAddr)
        {
            return i;
        }
    }
    return 0xff;
}

#endif // SUPPORT_STRATEGY_SORT
