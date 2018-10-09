#include "../general.h"

#include "../radio_app/radio_app.h"
#include "../radio_app/node_strategy.h"
#include "../APP/nodeApp.h"
#include "../APP/systemApp.h"
#include "../APP/radio_protocal.h"
/***** Defines *****/


/***** Type declarations *****/
typedef struct 
{
    uint32_t collectTimeCnt;         // the unit is sec
    uint32_t uploadTimeCnt;          // the unit is sec
    uint32_t customId;
    uint32_t sysTime;
    uint16_t monitorCnt;
    bool     broadcasting;
    bool     continueFlag;
    bool     configFlag;
}node_para_t;


static TxFrameRecord_t lastTxSensorDataRecord;

static node_para_t nodeParameter;

/***** Variable declarations *****/

Semaphore_Struct uploadSemStruct;
Semaphore_Handle uploadSemHandle;


/* Clock for node period collect */




uint8_t     offsetUnit; // for sensor data upload offset unit


/***** Prototypes *****/
//void NodeStrategyTimeoutProcess(void);


/***** Function definitions *****/


//***********************************************************************************
// brief:   
// 
// parameter: 
//***********************************************************************************
void NodeAppInit(void)
{
    nodeParameter.uploadTimeCnt  = 0;
    nodeParameter.collectTimeCnt = 0;
    nodeParameter.monitorCnt     = 0;
    
    
    offsetUnit                   = 0;


    nodeParameter.customId       = 0xffff0000 | (g_rSysConfigInfo.customId[0] << 8) | g_rSysConfigInfo.customId[1];


    Semaphore_Params semParams;
    Semaphore_Params_init(&semParams);
    Semaphore_construct(&uploadSemStruct, 1, &semParams);
    uploadSemHandle = Semaphore_handle(&uploadSemStruct);

    
    SetRadioSrcAddr( (((uint32_t)(g_rSysConfigInfo.DeviceId[0])) << 24) |
                     (((uint32_t)(g_rSysConfigInfo.DeviceId[1])) << 16) |
                     (((uint32_t)(g_rSysConfigInfo.DeviceId[2])) << 8) |
                     g_rSysConfigInfo.DeviceId[3]);
    SetRadioDstAddr(nodeParameter.customId);

    NodeStrategyInit(NodeStrategyTimeoutProcess);
    
    NodeStrategySetPeriod(g_rSysConfigInfo.collectPeriod);

    // NodeWakeup();
}


//***********************************************************************************
// brief:   set the upload timer period
// 
// parameter
// period:  the uint is sec
//***********************************************************************************
void NodeUploadPeriodSet(uint32_t period)
{

}



//***********************************************************************************
// brief:   Node send the sensor data to concenter
// 
// parameter: 
//***********************************************************************************
void NodeUploadProcess(void)
{
    uint8_t     data[24];
    uint32_t    dataItems;
    uint16_t serialNumber;

    // reverse the buf to other command
    Semaphore_pend(uploadSemHandle, BIOS_WAIT_FOREVER);

    NodeStrategyBuffClear();
    offsetUnit = 0;
    memset(&lastTxSensorDataRecord, 0, sizeof(lastTxSensorDataRecord));
    dataItems  = Flash_get_unupload_items();

    if(dataItems >= offsetUnit)
    {
        dataItems = dataItems - offsetUnit;
    }

    while(dataItems)
    {
#ifdef BOARD_S3
        Flash_load_sensor_data_by_offset(data+6, 16, offsetUnit);
#ifdef SUPPORT_UPLOAD_ASSET_INFO
        data[0] = 17;
#else
        data[0] = 21;
#endif  //SUPPORT_UPLOAD_ASSET_INFO
        data[1] = 0;
        data[2] = g_rSysConfigInfo.DeviceId[0];
        data[3] = g_rSysConfigInfo.DeviceId[1];
        data[4] = g_rSysConfigInfo.DeviceId[2];
        data[5] = g_rSysConfigInfo.DeviceId[3];
#else
        Flash_load_sensor_data_by_offset(data, 22, offsetUnit);
#endif  // BOARD_S3
        serialNumber = ((data[6] << 8) | data[7]);
        // the radio buf is full 
        if(NodeRadioSendSensorData(data, data[0] + 1) == false)
        {
            Semaphore_post(uploadSemHandle);
            return;
        }
        lastTxSensorDataRecord.lastFrameSerial[lastTxSensorDataRecord.Cnt] = serialNumber;
        lastTxSensorDataRecord.Cnt++;
        dataItems--;
        offsetUnit++;
    }
    Semaphore_post(uploadSemHandle);
}
//***********************************************************************************
// brief:   when the sensor data upload fail, needn't do everything
// 
// parameter: 
//***********************************************************************************
void NodeUploadOffectClear(void)
{
    offsetUnit = 0;
}

//***********************************************************************************
// brief:   move the fornt data point forward one unit
// 
// parameter: 
//***********************************************************************************
uint8_t NodeUploadSucessProcess(TxFrameRecord_t *temp)
{
    uint8_t flag = 1;
    uint8_t i = 0;

//    if(offsetUnit)
//    {
//        offsetUnit--;
//        Flash_moveto_next_sensor_data();
//    }
    Semaphore_pend(uploadSemHandle, BIOS_WAIT_FOREVER);
    if ((temp->Cnt != 0) && (lastTxSensorDataRecord.Cnt != 0) \
            &&  (temp->Cnt == lastTxSensorDataRecord.Cnt)) {
        for (i = 0; i < temp->Cnt; i++) {
            if (temp->lastFrameSerial[i] != lastTxSensorDataRecord.lastFrameSerial[i]) {
                flag = 0;
                break;
            }
        }
    } else {
        flag = 0;
    }

    if (flag) {
        for (i= 0; i < temp->Cnt; i++) {
            Flash_moveto_next_sensor_data();
        }
        memset(&lastTxSensorDataRecord, 0, sizeof(lastTxSensorDataRecord));
    }

    Semaphore_post(uploadSemHandle);
    return flag;
}


//***********************************************************************************
// brief:   set the collect sensor timer period
// 
// parameter: 
// period:  the uint is sec
//***********************************************************************************
void NodeCollectPeriodSet(uint32_t period)
{

}




//***********************************************************************************
// brief:   
// 
// parameter: 
//***********************************************************************************
void NodeLowTemperatureSet(uint8_t num, uint16_t alarmTemp)
{

}


//***********************************************************************************
// brief:   
// 
// parameter: 
//***********************************************************************************
void NodeHighTemperatureSet(uint8_t num, uint16_t alarmTemp)
{
    
}

//***********************************************************************************
// brief:   open the timer to send time syn request as broadcasting
// 
// parameter: 
//***********************************************************************************
void NodeBroadcasting(void)
{
 #if !defined(SUPPORT_BOARD_OLD_S1) || !defined(SUPPORT_BOARD_OLD_S2S_1)
    if(nodeParameter.broadcasting)
    {
        NodeStrategySetPeriod(g_rSysConfigInfo.collectPeriod);
        RadioEventPost(RADIO_EVT_SEND_SYC);
//        RadioSensorDataPack();
    }
#endif
}

//***********************************************************************************
// brief:   open the timer to send the device testing result
// 
// parameter: 
//***********************************************************************************
void NodeBroadcastTestResult(void)
{
    NodeStrategySetPeriod(NODE_BROADCAST_TESTRESUT_TIME);
    NodeRadioSendSynReq();
}

//***********************************************************************************
// brief:   start broadcast
// 
// parameter: 
//***********************************************************************************
void NodeStartBroadcast(void)
{
    SetRadioDstAddr(nodeParameter.customId);
    nodeParameter.broadcasting      = true;
}




//***********************************************************************************
// brief:   stop broadcast
// 
// parameter: 
//***********************************************************************************
void NodeStopBroadcast(void)
{
    nodeParameter.broadcasting      = false;
}


//***********************************************************************************
// brief:   make the node board into sleep mode
// 
// parameter: 
//***********************************************************************************
void NodeSleep(void)
{
    NodeStopBroadcast();
    NodeStrategyStop();
    offsetUnit = 0;
}

//***********************************************************************************
// brief:   make the node board into work mode
// 
// parameter: 
//***********************************************************************************
void NodeWakeup(void)
{
    NodeStrategyReset();
    NodeStartBroadcast();
    NodeBroadcasting();
    NodeStrategyStart();
}





//***********************************************************************************
// brief:Request the config and send the current config to configer
// 
// parameter: 
//***********************************************************************************
void NodeRequestConfig(void)
{
    // send the request
}


//***********************************************************************************
// brief:the node rtc process
// 
// parameter: 
//***********************************************************************************
void NodeRtcProcess(void)
{
#if defined (SUPPORT_BOARD_OLD_S1) || defined(SUPPORT_BOARD_OLD_S2S_1)
    static uint8_t count = 0;
    if ((deviceMode == DEVICES_OFF_MODE) || (RADIOMODE_UPGRADE == RadioModeGet())) {
        return;
    } else if (nodeParameter.collectTimeCnt >= g_rSysConfigInfo.collectPeriod) {
        OldS1NodeAPP_scheduledUploadData();
        nodeParameter.collectTimeCnt = 1;
    }

    if(Battery_get_voltage() <= g_rSysConfigInfo.batLowVol)
    {
        count++;
        if (count > 10)
        {
            count = 0;
            S1Sleep();
        }
        // SysCtrlSystemReset();
    }
    nodeParameter.collectTimeCnt++;
#else
    nodeParameter.sysTime++;
    if(nodeParameter.sysTime >= 3600)
    {
        RadioEventPost(RADIO_EVT_SEND_SYC);
        nodeParameter.sysTime       = 0;
    }
#endif
}


//***********************************************************************************
// brief:the the
// 
// parameter: 
//***********************************************************************************
void NodeStrategyTimeoutProcess(void)
{
    NodeStartBroadcast();
}


//***********************************************************************************
// brief:the node rtc process
// 
// parameter: 
//***********************************************************************************
void NodeSetCustomId(uint32_t customId)
{
    nodeParameter.customId = customId;
}





//***********************************************************************************
// brief:the node rtc process
// 
// parameter: 
//***********************************************************************************
uint32_t NodeGetCustomId(void)
{
    return nodeParameter.customId;
}

//***********************************************************************************
// brief:clear the node continue flag
// 
// parameter: 
//***********************************************************************************
void NodeContinueFlagClear(void)
{
    nodeParameter.configFlag = 0;
}


//***********************************************************************************
// brief:set the node continue flag
// 
// parameter: 
//***********************************************************************************
void NodeContinueFlagSet(void)
{
    nodeParameter.configFlag = 1;
}


//***********************************************************************************
// brief:read the node continue flag
// 
// parameter: 
//***********************************************************************************
bool NodeContinueFlagRead(void)
{
    return nodeParameter.configFlag;
}
