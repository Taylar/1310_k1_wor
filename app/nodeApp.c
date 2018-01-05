#include "../general.h"

#include "../radio_app/radio_app.h"
#include "../radio_app/node_strategy.h"
#include "../APP/nodeApp.h"
#include "../APP/systemApp.h"
#include "../APP/radio_protocal.h"
/***** Defines *****/
#define NODE_BROADCASTING_TIME         10

#define DEFAULT_DST_ADDR                0

/***** Type declarations *****/
typedef struct 
{
    uint32_t collectPeriod;
    uint32_t uploadPeriod;
    uint32_t customId;
    uint16_t serialNum;
    bool     broadcasting;
}node_para_t;

static node_para_t nodeParameter;

/***** Variable declarations *****/

/* Clock for node period collect */

Clock_Struct nodeCollectPeriodClock;     /* not static so you can see in ROV */
static Clock_Handle nodeCollectPeriodClockHandle;

Clock_Struct nodeUploadPeriodClock;     /* not static so you can see in ROV */
static Clock_Handle nodeUploadPeriodClockHandle;


/***** Prototypes *****/



/***** Function definitions *****/
//***********************************************************************************
// brief: set the upload period event   
// 
// parameter: 
//***********************************************************************************
static void NodeUploadPeriodCb(UArg arg0)
{
    Event_post(systemAppEvtHandle, SYSTEMAPP_EVT_UPLOAD);
}

//***********************************************************************************
// brief: set the upload sensor event event   
// 
// parameter: 
//***********************************************************************************
static void NodeCollectPeriodCb(UArg arg0)
{
    Event_post(systemAppEvtHandle, SYSTEMAPP_EVT_COLLECT);
}



//***********************************************************************************
// brief:   
// 
// parameter: 
//***********************************************************************************
void NodeAppInit(void (*Cb)(void))
{

    nodeParameter.serialNum     = 0;
    nodeParameter.uploadPeriod  = NODE_BROADCASTING_TIME * CLOCK_UNIT_S;
    nodeParameter.collectPeriod = NODE_BROADCASTING_TIME * CLOCK_UNIT_S;
    nodeParameter.broadcasting  = true;
    nodeParameter.customId      = DEFAULT_DST_ADDR;

    Clock_Params clkParams;
    clkParams.period    = 0;
    clkParams.startFlag = FALSE;
    Clock_construct(&nodeUploadPeriodClock, NodeUploadPeriodCb, 1, &clkParams);
    nodeUploadPeriodClockHandle = Clock_handle(&nodeUploadPeriodClock);
    NodeUploadPeriodSet(nodeParameter.uploadPeriod);

    clkParams.period    = 0;
    clkParams.startFlag = FALSE;
    Clock_construct(&nodeCollectPeriodClock, NodeCollectPeriodCb, 1, &clkParams);
    nodeCollectPeriodClockHandle = Clock_handle(&nodeCollectPeriodClock);
    NodeCollectPeriodSet(nodeParameter.collectPeriod);

    NodeStrategyInit(Cb);

    NodeStrategySetPeriod(nodeParameter.uploadPeriod);

    
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

    //Flash_init();

    SHT2X_FxnTable.initFxn(SHT2X_I2C_CH0);

    Led_init();
}


//***********************************************************************************
// brief:   start the upload timer
// 
// parameter: 
//***********************************************************************************
void NodeUploadStart(void)
{

    if((Clock_isActive(nodeUploadPeriodClockHandle) == false) && (nodeParameter.uploadPeriod))
        Clock_start(nodeUploadPeriodClockHandle);
}


//***********************************************************************************
// brief:   stop the upload timer
// 
// parameter: 
//***********************************************************************************
void NodeUploadStop(void)
{
    if(Clock_isActive(nodeUploadPeriodClockHandle))
        Clock_stop(nodeUploadPeriodClockHandle);
}

//***********************************************************************************
// brief:   set the upload timer period
// 
// parameter
// period:  the uint is ms
//***********************************************************************************
void NodeUploadPeriodSet(uint32_t period)
{
    nodeParameter.uploadPeriod      = period;
    if(period == 0)
        Clock_stop(nodeUploadPeriodClockHandle);
    else
        Clock_setPeriod(nodeUploadPeriodClockHandle, period);
}



//***********************************************************************************
// brief:   Node send the sensor data to concenter
// 
// parameter: 
//***********************************************************************************
void NodeUploadProcess(void)
{
    uint8_t     data[24];

    //reverse the buf to other command
    while(Flash_get_unupload_items() < 32)
    {
        Flash_load_sensor_data(data, 22);

        // the radio buf is full 
        if(NodeRadioSendSensorData(data, 22) == false)
        {
            Flash_recovery_last_sensor_data();
            return;
        }
    }


}
//***********************************************************************************
// brief:   when the sensor data upload, recover the extflash store
// 
// note:    this maybe make an error because the node would send several sensor data
// in a radio packet and couldn't known which one was fail
// parameter: 
//***********************************************************************************
// void NodeUploadFailProcess(void)
// {
//     Flash_recovery_last_sensor_data();
// }


//***********************************************************************************
// brief:   start the collect sensor timer
// 
// parameter: 
//***********************************************************************************
void NodeCollectStart(void)
{
    if(Clock_isActive(nodeCollectPeriodClockHandle) == false)
        Clock_start(nodeCollectPeriodClockHandle);
}


//***********************************************************************************
// brief:   stop the collect sensor timer
// 
// parameter: 
//***********************************************************************************
void NodeCollectStop(void)
{
    if(Clock_isActive(nodeCollectPeriodClockHandle))
        Clock_stop(nodeCollectPeriodClockHandle);
}


//***********************************************************************************
// brief:   set the collect sensor timer period
// 
// parameter: 
// period:  the uint is ms
//***********************************************************************************
void NodeCollectPeriodSet(uint32_t period)
{
    nodeParameter.collectPeriod         = period;
    if(period == 0)
        Clock_stop(nodeCollectPeriodClockHandle);
    else
        Clock_setPeriod(nodeCollectPeriodClockHandle, period);
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
    data[8] = (uint8_t)(calendarTemp.year - 2000);
    data[9] = (uint8_t)(calendarTemp.month);
    data[10] = (uint8_t)(calendarTemp.day);
    data[11] = (uint8_t)(calendarTemp.hour);
    data[12] = (uint8_t)(calendarTemp.min);
    data[13] = (uint8_t)(calendarTemp.sec);

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
    temp     = SHT2X_FxnTable.getValueFxn(SHT2X_I2C_CH0, SHT2X_TEMP);
    data[18] = (uint8_t)(temp >> 8);
    data[19] = (uint8_t)(temp);


    temp     = SHT2X_FxnTable.getValueFxn(SHT2X_I2C_CH0, SHT2X_HUMI);
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
        NodeStrategySetPeriod(NODE_BROADCASTING_TIME*CLOCK_UNIT_S);
        NodeRadioSendSynReq();
    }
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
}

//***********************************************************************************
// brief:   make the node board into work mode
// 
// parameter: 
//***********************************************************************************
void NodeWakeup(void)
{
    NodeStartBroadcast();
    NodeBroadcasting();
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


