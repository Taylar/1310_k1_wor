#include "../general.h"


#include "../APP/concenterApp.h"
#include "../APP/systemApp.h"
#include "../APP/radio_protocal.h"
#include "../interface_app/interface.h"

/***** Defines *****/
//External flash sensor data pointer store position
#define NODE_ADDR_FLASH_POS            (0X18000)            // base 
//External flash sensor data pointer size
#define NODE_ADDR_FLASH_SIZE                    8
//External flash sensor data pointer number
#define NODE_ADDR_FLASH_BLOCK_NUM               32
//External flash sensor data pointer number
#define NODE_ADDR_FLASH_BLOCK_SIZE              128
//External flash sensor data pointer position offset
#define NODE_ADDR_FLASH_AREA_SIZE      (NODE_ADDR_FLASH_BLOCK_SIZE * NODE_ADDR_FLASH_BLOCK_NUM * NODE_ADDR_FLASH_SIZE)





#define     NODE_SETTING_CMD_LENGTH    10



/***** Type declarations *****/
typedef struct 
{
    uint32_t nodeCollectPeriod;
    uint32_t nodeUploadPeriod;
    uint32_t uploadNetPeriod;
    uint32_t nodeNum;
}concenter_para_t;

// static concenter_para_t concenterParameter;



/***** Variable declarations *****/



/* Clock for node period collect */
static Clock_Struct concenterUploadClock;     /* not static so you can see in ROV */
static Clock_Handle concenterUploadClockHandle;




/***** Prototypes *****/




/***** Function definitions *****/

//***********************************************************************************
// brief:   set the concenter upload event
// 
// parameter: 
//***********************************************************************************
static void ConcenterUploadTimerCb(UArg arg0)
{
    Event_post(systemAppEvtHandle, SYSTEMAPP_EVT_NET_UPLOAD);
}


//***********************************************************************************
// brief:   init the concenter upload timer
// 
// parameter: 
//***********************************************************************************
void ConcenterAppInit(void)
{
    Clock_Params clkParams;

    clkParams.period    = 0;
    clkParams.startFlag = FALSE;
    Clock_construct(&concenterUploadClock, ConcenterUploadTimerCb, 1, &clkParams);
    concenterUploadClockHandle = Clock_handle(&concenterUploadClock);

}

//***********************************************************************************
// brief:   
// 
// parameter: 
//***********************************************************************************
void ConcenterAppHwInit(void)
{
    Spi_init();

    // Flash_init();

    Led_init();

    DeepTemp_FxnTable.initFxn(MAX31855_SPI_CH0);
}



//***********************************************************************************
// brief:   start the upload timer
// 
// parameter: 
//***********************************************************************************
void ConcenterUploadStart(void)
{
    if(Clock_isActive(concenterUploadClockHandle) == false)
        Clock_start(concenterUploadClockHandle);
}


//***********************************************************************************
// brief:   stop the upload timer
// 
// parameter: 
//***********************************************************************************
void ConcenterUploadStop(void)
{
    if(Clock_isActive(concenterUploadClockHandle))
        Clock_stop(concenterUploadClockHandle);
}

//***********************************************************************************
// brief:   set the upload timer period
// 
// parameter
// period:  the uint is ms
//***********************************************************************************
void ConcenterUploadPeriodSet(uint32_t period)
{
    Clock_setPeriod(concenterUploadClockHandle, period * CLOCK_UNIT_MS);
}




//***********************************************************************************
// brief:  save the sensor data to extflash
// 
// parameter: 
//***********************************************************************************
void ConcenterSensorDataSave(uint8_t *dataP, uint8_t length)
{
    Flash_store_sensor_data(dataP, length);
}


//***********************************************************************************
// brief:   concenter upload the data to the internet
// 
// parameter: 
//***********************************************************************************
void ConcenterSensorDataUpload(void)
{
    uint8_t dataBuf[32];
    if(Flash_get_unupload_items())
    {
        Flash_load_sensor_data(dataBuf, 32);
        InterfaceSend(dataBuf, 32);
    }
}       


void ConcenterStoreParameter(uint8_t *dataP, uint8_t length)
{
    
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
    
}

//***********************************************************************************
// brief:   make the Concenter board into work mode
// 
// parameter: 
//***********************************************************************************
void ConcenterWakeup(void)
{

}

