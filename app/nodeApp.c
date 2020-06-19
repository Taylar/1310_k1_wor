#include "../general.h"

#include "../radio_app/radio_app.h"
#include "../radio_app/node_strategy.h"
#include "../APP/nodeApp.h"
#include "../APP/systemApp.h"
#include "../APP/radio_protocal.h"
/***** Defines *****/
#ifdef S_C

/***** Type declarations *****/
typedef struct 
{
    uint32_t collectTimeCnt;         // the unit is sec
    uint32_t uploadTimeCnt;          // the unit is sec
    uint32_t customId;
    uint32_t sysTime;
    uint16_t monitorCnt;
    uint8_t  radioPower;
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


    nodeParameter.customId       = (((uint32_t)(g_rSysConfigInfo.customId[0])) << 24) |
                                    (((uint32_t)(g_rSysConfigInfo.customId[1])) << 16) |
                                    (((uint32_t)(g_rSysConfigInfo.customId[2])) << 8) |
                                    g_rSysConfigInfo.customId[3];


    Semaphore_Params semParams;
    Semaphore_Params_init(&semParams);
    Semaphore_construct(&uploadSemStruct, 1, &semParams);
    uploadSemHandle = Semaphore_handle(&uploadSemStruct);

    
    SetRadioSrcAddr( (((uint32_t)(g_rSysConfigInfo.DeviceId[0])) << 24) |
                     (((uint32_t)(g_rSysConfigInfo.DeviceId[1])) << 16) |
                     (((uint32_t)(g_rSysConfigInfo.DeviceId[2])) << 8) |
                     g_rSysConfigInfo.DeviceId[3]);

    GroudAddrSet(nodeParameter.customId);
    SetRadioSubSrcAddr(nodeParameter.customId );
    SetRadioBrocastSrcAddr(RADIO_BROCAST_ADDRESS);


    SetRadioDstAddr(nodeParameter.customId);

    NodeStrategyInit(NodeStrategyTimeoutProcess);
    
    NodeStrategySetPeriod(1);
    NodeResetAPC();
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
//        Flash_moveto_offset_sensor_data(1);
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
#ifdef SUPPORT_UPLOAD_ASSET_INFO
        assetInfoValid = 0;
#else
        for (i= 0; i < temp->Cnt; i++) {
            Flash_moveto_offset_sensor_data(1);
        }
#endif //SUPPORT_UPLOAD_ASSET_INFO
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
    if(nodeParameter.broadcasting)
    {
        RadioEventPost(RADIO_EVT_SEND_SYC);
    }
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

bool NodeIsBroadcast(void)
{
    return nodeParameter.broadcasting;
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
#ifdef ZKS_S3_WOR
    RadioEventPost(RADIO_EVT_START_SNIFF);
#else
    RadioSend();
#endif //ZKS_S3_WOR
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
    nodeParameter.sysTime++;

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

//***********************************************************************************
// brief:read the node continue flag
// 
// parameter: 
//***********************************************************************************
void NodeAPC(int8_t rssi)
{
    if(rssi > RADIO_APC_THRESHOLD_HIGH)
    {
        if(nodeParameter.radioPower)
            nodeParameter.radioPower--; 
    }

    if(rssi < RADIO_APC_THRESHOLD_LOW)
    {
        if(nodeParameter.radioPower < (g_rSysConfigInfo.rfPA >> 4))
            nodeParameter.radioPower++;
    }
    //System_printf("rssi:%d,Power:%d\n", rssi, nodeParameter.radioPower);
}

//***********************************************************************************
// brief:read the node continue flag
// 
// parameter: 
//***********************************************************************************
void NodeResetAPC(void)
{
    nodeParameter.radioPower = g_rSysConfigInfo.rfPA >> 4;
}




//***********************************************************************************
// brief:read the node continue flag
// 
// parameter: 
//***********************************************************************************
void NodeSetAPC(void)
{
    if(g_rSysConfigInfo.rfStatus & STATUS_LORA_APC)
        RadioSetRfPower(nodeParameter.radioPower);
}


#endif // S_C
