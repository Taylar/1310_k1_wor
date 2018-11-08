/*
* @Author: zxt
* @Date:   2017-12-28 10:09:45
* @Last Modified by:   zxt
* @Last Modified time: 2018-11-08 11:25:56
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
#ifdef SUPPORT_STRATEGY_SORT
    AutoFreqConcenterStop();
#endif // SUPPORT_STRATEGY_SORT
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
        Flash_log("NoRec\n");
        ConcenterRadioMonitorClear();
        RadioAbort();
        // RadioSetRxMode();
        SetRadioDstAddr(0xdadadada);
        g_rSysConfigInfo.sysState.lora_send_errors ++;
        Flash_store_config();
        RadioEventPost(RADIO_EVT_SEND_CONFIG);
    }

#ifdef S_G
    // static uint32_t timingSendDataCnt = 0;
    // /* Send data regularly to ensure that the gateway can receive*/
    // if (timingSendDataCnt > (5 * 60)) {
    //     RadioModeSet(RADIOMODE_RECEIVEPORT);
    //     timingSendDataCnt = 0;
    //     SetRadioDstAddr(0xdadadada);
    //     ConcenterRadioSendParaSet(0xabababab, 0xbabababa);
    // }
    // timingSendDataCnt++;
#endif  // S_G

}



#ifdef SUPPORT_STRATEGY_SORT
#ifdef SUPPORT_STORE_ID_IN_EXTFLASH
uint16_t  nodeNumDispath;
uint16_t  nodeRecentId;
uint32_t  nodeIdRecentAddr;

//***********************************************************************************
// brief:dispath the channel to the node ande return the channel
// 
// parameter: 
//***********************************************************************************
uint16_t ConcenterSetNodeChannel(uint32_t nodeAddr, uint32_t channel)
{
    uint32_t nodeIdTemp;
    if(channel == RADIO_INVAILD_CHANNEL)
    {
        nodeRecentId = RADIO_INVAILD_CHANNEL;
        return RADIO_INVAILD_CHANNEL;
    }

    if(channel < nodeNumDispath)
    {
        Flash_load_nodeid((uint8_t*)(&nodeIdTemp), channel);
        if(nodeIdTemp == nodeAddr)
        {
            nodeRecentId = channel;
            return channel;
        }
    }
    nodeRecentId = nodeNumDispath;
    nodeNumDispath++;
    if(nodeNumDispath > FLASH_NODEID_STORE_NUMBER)
        nodeNumDispath = 0;
    Flash_store_nodeid((uint8_t*)(&nodeAddr), nodeRecentId);
    return nodeRecentId;
}

//***********************************************************************************
// brief:fine the node channel through the Node ID in the memory
// 
// parameter: 
//***********************************************************************************
uint16_t ConcenterReadNodeChannel(uint32_t nodeAddr)
{
    uint32_t i; 
    uint32_t nodeIdTemp;
    for(i = 0; i < nodeNumDispath; i++)
    {
        Flash_load_nodeid((uint8_t*)(&nodeIdTemp), i);
        if(nodeIdTemp == nodeAddr)
        {
            nodeRecentId = i;
            return i;
        }
    }
    return 0xffff;
}

#else
uint32_t nodeAddrTable[CONCENTER_MAX_CHANNEL];
uint16_t  nodeNumDispath;
uint16_t  nodeRecentId;

//***********************************************************************************
// brief:dispath the channel to the node ande return the channel
// 
// parameter: 
//***********************************************************************************
uint16_t ConcenterSetNodeChannel(uint32_t nodeAddr, uint32_t channel)
{

    if(channel < nodeNumDispath)
    {
        if(nodeAddrTable[channel] == nodeAddr)
        {
            nodeRecentId = channel;
            return channel;
        }
    }
    // could not find the nodeaddr in the nodeAddrTable
    if(nodeNumDispath >= CONCENTER_MAX_CHANNEL)
    {
        memset((uint8_t*)nodeAddrTable, 0, sizeof(nodeAddrTable));
        nodeNumDispath = 0;
    }
    nodeAddrTable[nodeNumDispath] = nodeAddr;
    nodeRecentId = nodeNumDispath;
    nodeNumDispath++;
    return (nodeRecentId);
}

//***********************************************************************************
// brief:fine the node channel through the Node ID in the memory
// 
// parameter: 
//***********************************************************************************
uint16_t ConcenterReadNodeChannel(uint32_t nodeAddr)
{
    uint8_t i; 
    for(i = 0; i < CONCENTER_MAX_CHANNEL; i++)
    {
        if(nodeAddrTable[i] == nodeAddr)
        {
            nodeRecentId = i;
            return i;
        }
    }
    return 0xffff;
}

#endif //SUPPORT_STORE_ID_IN_EXTFLASH

//***********************************************************************************
// brief:read the 
// 
// parameter: 
//***********************************************************************************
uint16_t ConcenterReadResentNodeChannel(void)
{
    return nodeRecentId;
}

#endif // SUPPORT_STRATEGY_SORT
