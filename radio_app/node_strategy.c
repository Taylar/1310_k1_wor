/*
* @Author: zxt
* @Date:   2017-12-26 14:22:11
* @Last Modified by:   zxt
* @Last Modified time: 2018-06-08 14:36:13
*/
#include "../general.h"
#include <ti/sysbios/BIOS.h>
//#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26XX.h>



#include "node_strategy.h"
#include "../radio_app/radio_app.h"

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/aon_batmon.h)
#include DeviceFamily_constructPath(driverlib/trng.h)
/***** Defines *****/
#define     INVALID_CHANNEL             0XFFFFFFFF

#define     NODE_TIME_OFFSET_MAX_MS     (30) 


#define     FAIL_CONNECT_MAX_NUM               10

#define     FAIL_CONNECT_PERIOD_MAX_NUM        3

/***** Type declarations *****/
typedef struct {
    bool        init;
    bool        success;
    bool        busy;
    uint8_t     remainderCache;             // 
    uint8_t     failNum;                    // fail time
    uint8_t     periodNum;                  // fail period time 
    uint32_t    period;                     // the unit is sec
    uint32_t    channel;                // 
    uint32_t    channelNum;
    uint32_t    concenterAddr;
    uint32_t    concenterNum;
}node_strategy_t;

/***** Variable declarations *****/

/* Clock for node period sending */

Clock_Struct nodeStrategyStartClock;     /* not static so you can see in ROV */
Clock_Handle nodeStrategyStartClockHandle;

Semaphore_Struct radomFuncSem;  /* not static so you can see in ROV */
Semaphore_Handle radomFuncSemHandle;

// the 
static node_strategy_t      nodeStrategy;

/***** Prototypes *****/


static void (*NodeStrategyPeriodCb)(void);

static void (*NodeStrategyFailCb)(void);
/***** Function definitions *****/


static void NodeStrategyStartCb(UArg arg0)
{
    if((NodeStrategyPeriodCb != NULL) && nodeStrategy.busy)
        NodeStrategyPeriodCb();
}


//***********************************************************************************
// brief:   Init the NodeStrategy
// 
// parameter: 
// Cb:      Init the send radio event
//***********************************************************************************
void NodeStrategyInit(void (*Cb)(void), void (*StrategyFailCb)(void))
{
    if(nodeStrategy.init == false)
    {
        NodeStrategyReset();

        Clock_Params clkParams;
        Clock_Params_init(&clkParams);
        clkParams.period    = 0;
        clkParams.startFlag = FALSE;
        Clock_construct(&nodeStrategyStartClock, NodeStrategyStartCb, 1, &clkParams);
        nodeStrategyStartClockHandle = Clock_handle(&nodeStrategyStartClock);

        /* Create semaphore used for exclusive radio access */ 
    
        Semaphore_Params semParam;
        Semaphore_Params_init(&semParam);
        Semaphore_construct(&radomFuncSem, 1, &semParam);
        radomFuncSemHandle = Semaphore_handle(&radomFuncSem); 

        NodeStrategyPeriodCb        = Cb;

        NodeStrategyFailCb          = StrategyFailCb;
    }
}


//***********************************************************************************
// brief:   Reset the NodeStrategy parameter
// 
// parameter: 
// Cb:      Init the send radio event
//***********************************************************************************
void NodeStrategyReset(void)
{
    nodeStrategy.init         = true;
    nodeStrategy.success      = false;
    nodeStrategy.busy         = false;
    nodeStrategy.channel      = INVALID_CHANNEL;
    nodeStrategy.channelNum   = 0;
    nodeStrategy.failNum      = 0;
    nodeStrategy.periodNum    = 0;
    nodeStrategy.concenterNum = 0;


    nodeStrategy.remainderCache       = EASYLINK_MAX_DATA_LENGTH;
}

//***********************************************************************************
// brief:   set the node strategy peirod
// 
// parameter: 
// period:  the uint is sec
//***********************************************************************************
void NodeStrategySetPeriod(uint32_t period)
{
    if(Clock_isActive(nodeStrategyStartClockHandle))
    {
        Clock_stop(nodeStrategyStartClockHandle);
        nodeStrategy.period         = period;
        Clock_setPeriod(nodeStrategyStartClockHandle, (uint32_t)nodeStrategy.period * CLOCK_UNIT_S);
        Clock_start(nodeStrategyStartClockHandle);
    }
    else
    {
        nodeStrategy.period         = period;
        Clock_setPeriod(nodeStrategyStartClockHandle, (uint32_t)nodeStrategy.period * CLOCK_UNIT_S);
    }
}

//***********************************************************************************
// brief:   set the node strategy peirod
// 
// parameter: 
// period:  the uint is sec
//***********************************************************************************
void NodeStrategyStop(void)
{
    if(Clock_isActive(nodeStrategyStartClockHandle))
        Clock_stop(nodeStrategyStartClockHandle);
    ClearRadioSendBuf();
    
    nodeStrategy.busy           = false;
    nodeStrategy.success        = false;
}


//***********************************************************************************
// brief:   select the random time to send the radio packet
// 
// parameter: none
//***********************************************************************************
static void NodeStrategyStart(void)
{
    uint32_t randomNum;
    if(Clock_isActive(nodeStrategyStartClockHandle))
        Clock_stop(nodeStrategyStartClockHandle);

    // gennerate a ramdom num maybe waste almost 1sec
    /* Use the True Random Number Generator to generate sensor node address randomly */
    Semaphore_pend(radomFuncSemHandle, BIOS_WAIT_FOREVER);
    Power_setDependency(PowerCC26XX_PERIPH_TRNG);
    TRNGEnable();
    while (!(TRNGStatusGet() & TRNG_NUMBER_READY))
    {
        //wiat for randum number generator
    }
    randomNum = TRNGNumberGet(TRNG_LOW_WORD);

    while (!(TRNGStatusGet() & TRNG_NUMBER_READY))
    {
        //wiat for randum number generator
    }

    randomNum |= ((uint32_t)TRNGNumberGet(TRNG_HI_WORD)) << 16;

    TRNGDisable();
    Power_releaseDependency(PowerCC26XX_PERIPH_TRNG);
    Semaphore_post(radomFuncSemHandle);

    Clock_setTimeout(nodeStrategyStartClockHandle, randomNum % (nodeStrategy.period * CLOCK_UNIT_S));
    Clock_setPeriod(nodeStrategyStartClockHandle, nodeStrategy.period * CLOCK_UNIT_S);
    Clock_start(nodeStrategyStartClockHandle);
}




//***********************************************************************************
// brief:   adjust the time of the node sendout
// 
// parameter: none
//***********************************************************************************
void NodeStrategyReceiveTimeoutProcess(void)
{
    nodeStrategy.failNum += 2;
    if(nodeStrategy.success == true)
    {
        if(nodeStrategy.failNum > FAIL_CONNECT_MAX_NUM)
        {
            nodeStrategy.periodNum++;
            if(nodeStrategy.periodNum > FAIL_CONNECT_PERIOD_MAX_NUM)
            {
                nodeStrategy.periodNum = FAIL_CONNECT_PERIOD_MAX_NUM;
            }
            NodeStrategyFailCb();
            NodeStrategyStart();
            nodeStrategy.success    = false;
            nodeStrategy.failNum   = 0;
        }
    }
    else
    {
        NodeStrategyStart();
        nodeStrategy.success    = false;
        nodeStrategy.failNum   = 0;
    }
}


//***********************************************************************************
// brief:   adjust the time of the node sendout
// 
// parameter: none
//***********************************************************************************
void NodeStrategyReceiveSuccess(void)
{
    
    nodeStrategy.busy           = false;
    nodeStrategy.remainderCache = EASYLINK_MAX_DATA_LENGTH;
    if(nodeStrategy.failNum)
        nodeStrategy.failNum--;
    
    ClearRadioSendBuf();
    
    if(nodeStrategy.concenterNum)
    {
        if(nodeStrategy.concenterAddr != GetRadioDstAddr())
            nodeStrategy.concenterNum = 2;
    }
    else
    {
        nodeStrategy.concenterAddr = GetRadioDstAddr();
        nodeStrategy.concenterNum++;
    }
}


//***********************************************************************************
// brief:   adjust the time of the node sendout
// 
// parameter: none
//***********************************************************************************
bool NodeStrategySendPacket(uint8_t *dataP, uint8_t len)
{
    if(len > nodeStrategy.remainderCache)
        return false;
    

    RadioCopyPacketToBuf(dataP, len, 0, PASSRADIO_ACK_TIMEOUT_TIME_MS, EASYLINK_MAX_DATA_LENGTH - nodeStrategy.remainderCache);
    nodeStrategy.remainderCache -= len;
    nodeStrategy.busy       = true;

    if(nodeStrategy.success == false)
    {
        NodeStrategyStart();
    }

    return true;
}



//***********************************************************************************
// brief:   
// 
// parameter: 
//***********************************************************************************
void NodeStrategyBusySet(bool boolFlag)
{
    nodeStrategy.busy       = boolFlag;

    nodeStrategy.remainderCache = EASYLINK_MAX_DATA_LENGTH;
}


//***********************************************************************************
// brief:   
// 
// parameter: 
//***********************************************************************************
bool NodeStrategyBusyRead(void)
{
    return nodeStrategy.busy;
}


//***********************************************************************************
// brief:   
// 
// parameter: 
//***********************************************************************************
uint8_t NodeStrategyRemainderCache(void)
{
    return nodeStrategy.remainderCache;
}

//***********************************************************************************
// brief: get the register status   
// 
// parameter: 
//***********************************************************************************
bool GetStrategyRegisterStatus(void)
{
    return nodeStrategy.success;
}

//***********************************************************************************
// brief: set register success   
// 
// parameter: 
//***********************************************************************************
void StrategyRegisterSuccess(void)
{
    nodeStrategy.success = true;
}


//***********************************************************************************
// brief:   check the channel and the tick, adjust the channel and the peiriod of radio sending
// 
// parameter: 
//***********************************************************************************
void NodeStrategySetOffset_Channel(uint32_t concenterTick, uint32_t length, uint32_t channel)
{
    int32_t launchTime;
    uint8_t timerFlag = 0;
    
    // 
    if((nodeStrategy.periodNum >= FAIL_CONNECT_PERIOD_MAX_NUM) ||
        (nodeStrategy.concenterNum > 2) ||
        (channel >= nodeStrategy.channelNum))
    {
        nodeStrategy.success = true;
        return;
    }

    // transform to ms
    concenterTick = concenterTick / 100 + (length * 16 / 10);


    if(nodeStrategy.success)
    {
        if(channel != nodeStrategy.channel)
        {
            // readjust the timer
            goto ReadjustChannel;
        }

    }
    else
    {
        // need to register the new channel
ReadjustChannel:

        nodeStrategy.success        = true;
        nodeStrategy.channel        = channel;

        concenterTick = concenterTick % (nodeStrategy.period * 1000);
        // transform to ms
        launchTime  = nodeStrategy.channel * nodeStrategy.period * 1000 / nodeStrategy.channelNum;
        
        if(Clock_isActive(nodeStrategyStartClockHandle))
        {
            Clock_stop(nodeStrategyStartClockHandle);       
            timerFlag = 1;
        }
        
        if(launchTime > concenterTick)
        {
            Clock_setTimeout(nodeStrategyStartClockHandle, (launchTime - concenterTick) * CLOCK_UNIT_MS);
        }
        else
        {
            Clock_setTimeout(nodeStrategyStartClockHandle,((uint32_t)nodeStrategy.period*1000 - concenterTick + launchTime) * CLOCK_UNIT_MS);
        }
        Clock_setPeriod(nodeStrategyStartClockHandle, nodeStrategy.period * CLOCK_UNIT_S);
        
        if(timerFlag)
            Clock_start(nodeStrategyStartClockHandle);
    }
}
