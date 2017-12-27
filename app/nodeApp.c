#include "../general.h"

#include "../APP/nodeApp.h"
#include "../APP/systemApp.h"
#include "../APP/radio_protocal.h"
/***** Defines *****/

/***** Type declarations *****/


/***** Variable declarations *****/

/* Clock for node period collect */

static Clock_Struct nodeCollectPeriodClock;     /* not static so you can see in ROV */
static Clock_Handle nodeCollectPeriodClockHandle;

static Clock_Struct nodeUploadPeriodClock;     /* not static so you can see in ROV */
static Clock_Handle nodeUploadPeriodClockHandle;

static uint16_t nodeSensorDataSerialNum;

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
void NodeAppInit(void)
{
    Clock_Params clkParams;
    clkParams.period    = 0;
    clkParams.startFlag = FALSE;
    Clock_construct(&nodeUploadPeriodClock, NodeUploadPeriodCb, 1, &clkParams);
    nodeUploadPeriodClockHandle = Clock_handle(&nodeUploadPeriodClock);


    clkParams.period    = 0;
    clkParams.startFlag = FALSE;
    Clock_construct(&nodeCollectPeriodClock, NodeCollectPeriodCb, 1, &clkParams);
    nodeCollectPeriodClockHandle = Clock_handle(&nodeCollectPeriodClock);

    nodeSensorDataSerialNum = 0;
}


//***********************************************************************************
// brief:   start the upload timer
// 
// parameter: 
//***********************************************************************************
void NodeUploadStart(void)
{
    if(Clock_isActive(nodeUploadPeriodClockHandle) == false)
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
    Clock_setPeriod(nodeUploadPeriodClockHandle, period * CLOCK_UNIT_MS);
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
    uint32_t    voltageTemp, humiTemp, temperatureTemp;
    Calendar    calendarTemp;


    // save the sht2x data
    SHT2X_FxnTable.measureFxn(SEN_I2C_CH0);
    
    temperatureTemp = SHT2X_FxnTable.getValueFxn(SEN_I2C_CH0, SENSOR_TEMP);
    humiTemp        = SHT2X_FxnTable.getValueFxn(SEN_I2C_CH0, SENSOR_HUMI);
    
    voltageTemp     = HWREG(AON_BATMON_BASE + AON_BATMON_O_BAT);//AONBatMonBatteryVoltageGet();

    voltageTemp     = ((voltageTemp&0xff00)>>8)*1000 +1000*(voltageTemp&0xff)/255;

    calendarTemp    = Rtc_get_calendar();
    // length
    data[0] = 22;
    // rssi
    data[1] = 0;
    // deceive ID
    data[2] = 0;
    data[3] = 0;
    data[4] = 0;
    data[5] = 0;
    
    // serial num
    data[6] = (uint8_t)(nodeSensorDataSerialNum>>8);
    data[7] = (uint8_t)nodeSensorDataSerialNum;
    
    // collect time
    data[8] = (uint8_t)(calendarTemp.year - 2000);
    data[9] = (uint8_t)(calendarTemp.month);
    data[10] = (uint8_t)(calendarTemp.day);
    data[11] = (uint8_t)(calendarTemp.hour);
    data[12] = (uint8_t)(calendarTemp.min);
    data[13] = (uint8_t)(calendarTemp.sec);

    // voltage
    data[14] = (uint8_t)(voltageTemp >> 8);
    data[15] = (uint8_t)(voltageTemp);

    // sensor id
    data[16] = 0;

    // sensor type
    data[17] = PARATYPE_TEMP_HUMI_SHT20;

    // sensor data
    data[18] = (uint8_t)(temperatureTemp >> 8);
    data[19] = (uint8_t)(temperatureTemp);
    data[20] = (uint8_t)(humiTemp >> 8);
    data[21] = (uint8_t)(humiTemp);

    Flash_store_sensor_data(data, 22);
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





