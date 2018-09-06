/*
* @Author: zxt
* @Date:   2017-12-28 10:09:45
* @Last Modified by:   zxt
* @Last Modified time: 2018-09-05 17:28:28
*/
#include "../general.h"






#define     NODE_SETTING_CMD_LENGTH    10

#define     CONCENTER_RADIO_MONITOR_CNT_MAX     60



/***** Type declarations *****/
typedef struct 
{
    uint32_t channelDispath;
    uint32_t synchronTimeCnt;
    uint32_t uploadTimeCnt;          // the unit is sec
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
    concenterParameter.collectTimeCnt  = 0;
    concenterParameter.serialNum       = 0;

    concenterParameter.synTimeFlag     = false;
    concenterParameter.collectStart     = false;
    concenterParameter.radioReceive    = false;

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
    concenterParameter.radioReceive = false;
    RtcStop();
    RadioDisable();
    ConcenterCollectStop();
    // wait the nwk disable the uart
#ifdef  SUPPORT_NETWORK
    Nwk_poweroff();
    while(Nwk_is_Active())
        Task_sleep(100 * CLOCK_UNIT_MS);
#endif

#ifdef BOARD_B2_2

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
    
    RtcStart();
    
#if (defined BOARD_S6_6 || defined BOARD_B2_2)
    if(GetUsbState() == USB_UNLINK_STATE)
#endif
    {
#ifdef  SUPPORT_NETWORK
        Nwk_poweron();
#endif
    }
    if((g_rSysConfigInfo.rfStatus & STATUS_1310_MASTER) && (g_rSysConfigInfo.module & MODULE_RADIO))
    {
        concenterParameter.radioReceive = true;
        RadioModeSet(RADIOMODE_RECEIVEPORT);
    }


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
            ConcenterSleep();

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
#ifdef  SUPPORT_NETWORK
            Nwk_poweron();
#endif
            ConcenterWakeup();
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
#ifdef  SUPPORT_NETWORK
    Nwk_poweroff();
    while(Nwk_is_Active())
    Task_sleep(100 * CLOCK_UNIT_MS);
#endif

    deviceMode = DEVICES_CONFIG_MODE;
    concenterParameter.radioReceive = true;
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
// brief:   start the collect sensor timer
// 
// parameter: 
//***********************************************************************************
void ConcenterCollectStart(void)
{
    uint8_t i;
    uint8_t secTemp;

    secTemp                    = RtcGetSec();

    for(i = 0; i < MODULE_SENSOR_MAX; i++)
    {
        if((g_rSysConfigInfo.sensorModule[i] == SEN_TYPE_SHT2X) || (g_rSysConfigInfo.sensorModule[i] == SEN_TYPE_OPT3001)
                || (g_rSysConfigInfo.sensorModule[i] == SEN_TYPE_DEEPTEMP))
        {
            concenterParameter.collectStart      = true;

            // promise the next collect time is 30s 
            secTemp += 30;
            concenterParameter.collectTimeCnt = secTemp;
            break;
        }
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
// brief:the concenter rtc process
// 
// parameter: 
//***********************************************************************************
void ConcenterRtcProcess(void)
{

    if(/*concenterParameter.radioReceive &&*/ (g_rSysConfigInfo.rfStatus & STATUS_1310_MASTER))
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
    }

    

    if(Battery_get_voltage() <= g_rSysConfigInfo.batLowVol)
    {
#ifdef      SUPPORT_DISP_SCREEN
        Disp_poweroff();
#endif
        ConcenterSleep();
    }
    
    if(concenterParameter.collectStart)
    {
        concenterParameter.collectTimeCnt++;
        if(concenterParameter.collectTimeCnt >= g_rSysConfigInfo.collectPeriod)
        {
            concenterParameter.collectTimeCnt = (concenterParameter.collectTimeCnt - g_rSysConfigInfo.collectPeriod) % g_rSysConfigInfo.collectPeriod;
            Battery_voltage_measure();
        }
    }
}

#define CONCENTER_MAX_CHANNEL       100



//***********************************************************************************
// brief:concenter set the channel
// 
// parameter: 
//***********************************************************************************
uint32_t ConcenterSetChannel(uint32_t nodeAddr)
{

}


