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
    uint32_t collectPeriod;         // the unit is sec
    uint32_t collectTimeCnt;         // the unit is sec
    uint32_t uploadPeriod;          // the unit is sec
    uint32_t uploadTimeCnt;          // the unit is sec
    uint32_t customId;
    uint32_t deceive;
    uint16_t serialNum;
    uint16_t sysTime;
    bool     collectStart;
    bool     uploadStart;
    bool     broadcasting;
    bool     configFlag;
    bool     synTimeFlag;
}node_para_t;

static node_para_t nodeParameter;

/***** Variable declarations *****/

/* Clock for node period collect */




uint8_t     offsetUnit; // for sensor data upload offset unit


/***** Prototypes *****/



/***** Function definitions *****/


//***********************************************************************************
// brief:   
// 
// parameter: 
//***********************************************************************************
void NodeAppInit(void (*Cb)(void))
{

    nodeParameter.serialNum      = 0;
    nodeParameter.sysTime        = 0;
    
    nodeParameter.uploadPeriod   = NODE_BROADCASTING_TIME;
    nodeParameter.uploadTimeCnt  = 0;
    nodeParameter.collectPeriod  = NODE_BROADCASTING_TIME;
    nodeParameter.collectTimeCnt = 0;
    nodeParameter.customId       = DEFAULT_DST_ADDR;
    
    nodeParameter.uploadStart    = false;
    nodeParameter.collectStart   = false;
    nodeParameter.synTimeFlag    = false;
    nodeParameter.configFlag     = InternalFlashLoadConfig();
    nodeParameter.configFlag     = true;
    
    offsetUnit                   = 0;

    if(nodeParameter.configFlag)
    {
        // nodeParameter.uploadPeriod  = g_rSysConfigInfo.uploadPeriod;
        // nodeParameter.collectPeriod = g_rSysConfigInfo.collectPeriod;
        // nodeParameter.customId      = 0xffff | *((uint16_t*)g_rSysConfigInfo.customId);
        // nodeParameter.deceive      = *((uint32_t *)g_rSysConfigInfo.DeviceId);

        // SetRadioSrcAddr(nodeParameter.deceive);
    }
    else
    {
        nodeParameter.uploadPeriod  = NODE_BROADCASTING_TIME;
        nodeParameter.collectPeriod = NODE_BROADCASTING_TIME;
        nodeParameter.customId      = DEFAULT_DST_ADDR;
    }

    SetRadioSrcAddr(CUSTOM_ID_DEFAULT);
    SetRadioDstAddr(DEFAULT_DST_ADDR);

    NodeStrategyInit(Cb);
    
    NodeStrategySetPeriod(nodeParameter.uploadPeriod);

    // NodeWakeup();
}

//***********************************************************************************
// brief:   
// 
// parameter: 
//***********************************************************************************
void NodeAppHwInit(void)
{
    Spi_init();

    I2c_init();

    Flash_init();

    SHT2X_FxnTable.initFxn(SHT2X_I2C_CH0);
}


//***********************************************************************************
// brief:   start the upload timer
// 
// parameter: 
//***********************************************************************************
void NodeUploadStart(void)
{
    nodeParameter.uploadStart = true;
}


//***********************************************************************************
// brief:   stop the upload timer
// 
// parameter: 
//***********************************************************************************
void NodeUploadStop(void)
{
    nodeParameter.uploadStart = false;
}

//***********************************************************************************
// brief:   set the upload timer period
// 
// parameter
// period:  the uint is sec
//***********************************************************************************
void NodeUploadPeriodSet(uint32_t period)
{
    nodeParameter.uploadPeriod      = period;
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
    // reverse the buf to other command
    dataItems  = Flash_get_unupload_items();
    if(dataItems >= offsetUnit)
    {
        dataItems = dataItems - offsetUnit;
    }
    else
    {
        offsetUnit = 0;
    }

    while(dataItems)
    {
        Flash_load_sensor_data(data, 22, offsetUnit);

        // the radio buf is full 
        if(NodeRadioSendSensorData(data, 22) == false)
        {
            return;
        }
        dataItems--;
        offsetUnit++;
    }
}
//***********************************************************************************
// brief:   when the sensor data upload fail, needn't do everything
// 
// parameter: 
//***********************************************************************************
void NodeUploadFailProcess(void)
{
    // if send fail should clear the buf,
    if(offsetUnit)
    {
        offsetUnit = 0;
    }
}

//***********************************************************************************
// brief:   move the fornt data point forward one unit
// 
// parameter: 
//***********************************************************************************
void NodeUploadSucessProcess(void)
{
    if(offsetUnit)
    {
        offsetUnit--;
        Falsh_prtpoint_forward();
    }
}

//***********************************************************************************
// brief:   start the collect sensor timer
// 
// parameter: 
//***********************************************************************************
void NodeCollectStart(void)
{
    nodeParameter.synTimeFlag       = true;
    nodeParameter.collectStart      = true;
}


//***********************************************************************************
// brief:   stop the collect sensor timer
// 
// parameter: 
//***********************************************************************************
void NodeCollectStop(void)
{
    nodeParameter.collectStart      = false;
    nodeParameter.synTimeFlag       = false;
}


//***********************************************************************************
// brief:   set the collect sensor timer period
// 
// parameter: 
// period:  the uint is sec
//***********************************************************************************
void NodeCollectPeriodSet(uint32_t period)
{
    nodeParameter.collectPeriod         = period;
}




//***********************************************************************************
// brief:   node save the data to extflash
// 
// parameter: 
//***********************************************************************************
void NodeCollectProcess(void)
{
    uint8_t     data[24];
    uint32_t    temp;
    Calendar    calendarTemp;


    // save the sht2x data
    SHT2X_FxnTable.measureFxn(SHT2X_I2C_CH0);
    
    

    calendarTemp    = Rtc_get_calendar();
    // length, note:do not include length self
    data[0] = 21;
    // rssi
    data[1] = 0;
    // deceive ID
    temp    = GetRadioSrcAddr();
    data[2] = (uint8_t)(temp>>24);
    data[3] = (uint8_t)(temp>>16);
    data[4] = (uint8_t)(temp>>8);
    data[5] = (uint8_t)(temp);
    
    // serial num
    data[6] = (uint8_t)(nodeParameter.serialNum>>8);
    data[7] = (uint8_t)nodeParameter.serialNum;
    
    // collect time
    data[8] = TransHexToBcd((uint8_t)(calendarTemp.Year - 2000));
    data[9] = TransHexToBcd((uint8_t)(calendarTemp.Month));
    data[10] = TransHexToBcd((uint8_t)(calendarTemp.DayOfMonth));
    data[11] = TransHexToBcd((uint8_t)(calendarTemp.Hours));
    data[12] = TransHexToBcd((uint8_t)(calendarTemp.Minutes));
    data[13] = TransHexToBcd((uint8_t)(calendarTemp.Seconds));

    // voltage
    temp     = AONBatMonBatteryVoltageGet();
    temp     = ((temp&0xff00)>>8)*1000 +1000*(temp&0xff)/256;
    data[14] = (uint8_t)(temp >> 8);
    data[15] = (uint8_t)(temp);

    // sensor id
    data[16] = 0;

    // sensor type
    data[17] = PARATYPE_TEMP_HUMI_SHT20;

    // sensor data
    temp     = SHT2X_FxnTable.getValueFxn(SHT2X_I2C_CH0, SENSOR_TEMP);
    data[18] = (uint8_t)(temp >> 8);
    data[19] = (uint8_t)(temp);


    temp     = SHT2X_FxnTable.getValueFxn(SHT2X_I2C_CH0, SENSOR_HUMI);
    data[20] = (uint8_t)(temp >> 8);
    data[21] = (uint8_t)(temp);

    Flash_store_sensor_data(data, data[0]+1);

    nodeParameter.serialNum++;
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
        NodeStrategySetPeriod(NODE_BROADCASTING_TIME);
        NodeRadioSendSynReq();
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


//***********************************************************************************
// brief:   make the node board into sleep mode
// 
// parameter: 
//***********************************************************************************
void NodeSleep(void)
{
    NodeStopBroadcast();
    NodeCollectStop();
    NodeUploadStop();
    NodeStrategyStop();
    deviceMode = DEVICES_OFF_MODE;
    nodeParameter.uploadStart         = false;
    nodeParameter.collectStart        = false;

    offsetUnit = 0;
    
}

//***********************************************************************************
// brief:   make the node board into work mode
// 
// parameter: 
//***********************************************************************************
void NodeWakeup(void)
{
    deviceMode = DEVICES_ON_MODE;
    NodeStrategyReset();
    if(nodeParameter.configFlag)
    {
        NodeStartBroadcast();
        NodeBroadcasting();
        if(nodeParameter.synTimeFlag)
        {
            NodeCollectStart();
        }
    }
}

//***********************************************************************************
// brief: set the custom id as the radio dst addr
// 
// parameter: 
//***********************************************************************************
void NodeSetCustomId(uint32_t id)
{
    nodeParameter.customId = id;
}


//***********************************************************************************
// brief:the node short key application
// 
// parameter: 
//***********************************************************************************
void NodeShortKeyApp(void)
{
    switch(deviceMode)
    {
        case DEVICES_ON_MODE:
        Led_ctrl(LED_B, 0, 500 * CLOCK_UNIT_MS, 1);
        break;

        case DEVICES_OFF_MODE:
        Led_ctrl(LED_R, 0, 500 * CLOCK_UNIT_MS, 1);
        break;
    }
}

//***********************************************************************************
// brief:the node long key application
// 
// parameter: 
//***********************************************************************************
void NodeLongKeyApp(void)
{
    switch(deviceMode)
    {
        case DEVICES_ON_MODE:
        NodeSleep();
        Led_ctrl(LED_R, 0, 250 * CLOCK_UNIT_MS, 6);
        break;

        case DEVICES_OFF_MODE:
        Led_ctrl(LED_B, 0, 250 * CLOCK_UNIT_MS, 6);
        NodeWakeup();
        break;
    }
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
// brief:the node sensor test
// 
// parameter: 
//***********************************************************************************
void NodeSensorTest(void)
{
    uint16_t voltageTemp;

    testResultInfoLen = 0;
    if(Flash_external_Selftest() == ES_SUCCESS)
    {
        testResultInfo[testResultInfoLen++] = 'F';
        testResultInfo[testResultInfoLen++] = 'l';
        testResultInfo[testResultInfoLen++] = 'a';
        testResultInfo[testResultInfoLen++] = 's';
        testResultInfo[testResultInfoLen++] = 'h';
        testResultInfo[testResultInfoLen++] = ':';
        testResultInfo[testResultInfoLen++] = 'O';
        testResultInfo[testResultInfoLen++] = 'K';
        testResultInfo[testResultInfoLen++] = '\n';
    }
    else
    {
        testResultInfo[testResultInfoLen++] = 'F';
        testResultInfo[testResultInfoLen++] = 'l';
        testResultInfo[testResultInfoLen++] = 'a';
        testResultInfo[testResultInfoLen++] = 's';
        testResultInfo[testResultInfoLen++] = 'h';
        testResultInfo[testResultInfoLen++] = ':';
        testResultInfo[testResultInfoLen++] = 'E';
        testResultInfo[testResultInfoLen++] = 'R';
        testResultInfo[testResultInfoLen++] = 'R';
        testResultInfo[testResultInfoLen++] = 'O';
        testResultInfo[testResultInfoLen++] = 'R';
        testResultInfo[testResultInfoLen++] = '\n';
    }

    SHT2X_FxnTable.measureFxn(SHT2X_I2C_CH0);
    DeepTemp_FxnTable.getValueFxn(MAX31855_SPI_CH0, SENSOR_DEEP_TEMP)/256;

    testResultInfoLen += sprintf((char *)(&testResultInfo[testResultInfoLen]), "TEMP: %02d",
                DeepTemp_FxnTable.getValueFxn(MAX31855_SPI_CH0, SENSOR_DEEP_TEMP)/256);

    testResultInfo[testResultInfoLen++] = '\n';

    voltageTemp = AONBatMonBatteryVoltageGet();
    voltageTemp = ((voltageTemp&0xff00)>>8)*1000 +1000*(voltageTemp&0xff)/256;
    testResultInfoLen += sprintf((char *)(&testResultInfo[testResultInfoLen]), "VOL: %d mV", voltageTemp);

    testResultInfo[testResultInfoLen++] = '\n';

    NodeBroadcastTestResult();
}




//***********************************************************************************
// brief:the node rtc process
// 
// parameter: 
//***********************************************************************************
void NodeRtcProcess(void)
{
    if(nodeParameter.collectStart)
    {
        nodeParameter.collectTimeCnt++;
        
        if(nodeParameter.collectTimeCnt >= nodeParameter.collectPeriod)
        {
            nodeParameter.collectTimeCnt = 0;
            Event_post(systemAppEvtHandle, SYSTEMAPP_EVT_COLLECT_NODE);
        }

        nodeParameter.sysTime++;
        if(nodeParameter.sysTime >= NODE_SYN_TIME_MAX)
        {
            NodeRadioSendSynReq();
            nodeParameter.sysTime       = 0;
        }
    }

    if(nodeParameter.uploadStart)
    {
        nodeParameter.uploadTimeCnt++;
        if(nodeParameter.uploadTimeCnt > nodeParameter.collectPeriod)
        {
            nodeParameter.uploadTimeCnt = 0;
            Event_post(systemAppEvtHandle, SYSTEMAPP_EVT_UPLOAD_NODE);
        }
    }
}

