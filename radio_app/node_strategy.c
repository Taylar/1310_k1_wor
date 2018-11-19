/*
* @Author: zxt
* @Date:   2017-12-26 14:22:11
* @Last Modified by:   zxt
* @Last Modified time: 2018-11-19 15:34:40
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




/***** Type declarations *****/
typedef struct {
    bool        init;
    bool        success;                    // register state
    uint8_t     remainderCache;             // 
    uint8_t     sendCnt;                    // send times in one period
    uint8_t     radioBusyCnt;               // send times in one period
    uint8_t     periodFailNum;              // fail period times 
    uint32_t    period;                     // the upload period, the unit is sec
    uint32_t    channel;                    // the concentor dispath the channel to the node
    uint32_t    channelNum;                 // max node channel num
    int32_t     offsetTicks;                 // max node channel num
}node_strategy_t;

/***** Variable declarations *****/

/* Clock for node period sending */

Clock_Struct nodeStrategyStartClock;     /* not static so you can see in ROV */
Clock_Handle nodeStrategyStartClockHandle;

// the 
static node_strategy_t      nodeStrategy;

/***** Prototypes *****/

static void (*NodeStrategyFailCb)(void);
/***** Function definitions *****/


static void NodeStrategyStartCb(UArg arg0)
{
    RadioEventPost(RADIO_EVT_TX);
}


//***********************************************************************************
// brief:   Init the NodeStrategy
// 
// parameter: 
// Cb:      Init the send radio event
//***********************************************************************************
void NodeStrategyInit(void (*StrategyFailCb)(void))
{
    if(nodeStrategy.init == false)
    {
        NodeStrategyReset();

        nodeStrategy.init         = true;
        nodeStrategy.channel      = g_rSysConfigInfo.alarmuploadPeriod; // the alarmupload period as node channel
        Clock_Params clkParams;
        Clock_Params_init(&clkParams);
        clkParams.period    = 0;
        clkParams.startFlag = FALSE;
        Clock_construct(&nodeStrategyStartClock, NodeStrategyStartCb, 1, &clkParams);
        nodeStrategyStartClockHandle = Clock_handle(&nodeStrategyStartClock);

        /* Create semaphore used for exclusive radio access */ 
    
        

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
    nodeStrategy.success       = false;
    nodeStrategy.channelNum    = CONCENTER_MAX_CHANNEL;
    nodeStrategy.sendCnt       = 0;
    nodeStrategy.radioBusyCnt  = 0;
    nodeStrategy.periodFailNum = 0;
    nodeStrategy.offsetTicks   = 0;


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
        if(nodeStrategy.period != period)
        {
            nodeStrategy.period         = period;
            Clock_stop(nodeStrategyStartClockHandle);
            Clock_setPeriod(nodeStrategyStartClockHandle, (uint32_t)nodeStrategy.period * CLOCK_UNIT_S);
            Clock_start(nodeStrategyStartClockHandle);
        }
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
}



//***********************************************************************************
// brief:   select the random time to send the radio packet
// 
// parameter: none
//***********************************************************************************
void NodeStrategyStart(void)
{
    uint32_t randomNum;


    if(Clock_isActive(nodeStrategyStartClockHandle))
        Clock_stop(nodeStrategyStartClockHandle);

    
    randomNum = RandomDataGenerate();
    // Clock_setTimeout(nodeStrategyStartClockHandle, randomNum % (nodeStrategy.period * CLOCK_UNIT_S));
    // Clock_setTimeout(nodeStrategyStartClockHandle, (randomNum % 100) * 200 * CLOCK_UNIT_MS/*(g_rSysConfigInfo.collectPeriod / 10 * CLOCK_UNIT_S))*/);
    if(!(g_rSysConfigInfo.rfStatus & STATUS_LORA_CHANGE_FREQ))
        Clock_setTimeout(nodeStrategyStartClockHandle, randomNum % (3 * CLOCK_UNIT_S));
    else
        Clock_setTimeout(nodeStrategyStartClockHandle, randomNum % (nodeStrategy.period / FAIL_CONNECT_MAX_NUM * CLOCK_UNIT_S));
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
    if(nodeStrategy.success == true)
    {
        nodeStrategy.periodFailNum++;
        if(nodeStrategy.periodFailNum >= FAIL_CONNECT_PERIOD_MAX_NUM)
        {
            nodeStrategy.periodFailNum = 0;
            nodeStrategy.success       = false;
            nodeStrategy.sendCnt       = 0;
            nodeStrategy.radioBusyCnt  = 0;

            if(!(g_rSysConfigInfo.rfStatus & STATUS_LORA_CHANGE_FREQ))
                AutoFreqNodeResetCurFreq();

            NodeStrategyFailCb();
        }
    }
    else
    {
        // every period only send FAIL_CONNECT_MAX_NUM times
        nodeStrategy.sendCnt ++;
        if(nodeStrategy.sendCnt < FAIL_CONNECT_MAX_NUM)
        {
            NodeStrategyStart();
        }
        else
        {
            nodeStrategy.sendCnt = 0;
            if(!(g_rSysConfigInfo.rfStatus & STATUS_LORA_CHANGE_FREQ))
                AutoFreqNodeSwitchFreq();
            
        }
    }
}


//***********************************************************************************
// brief:   adjust the time of the node sendout
// 
// parameter: none
//***********************************************************************************
void NodeStrategyBuffClear(void)
{
    nodeStrategy.remainderCache = EASYLINK_MAX_DATA_LENGTH;
    ClearRadioSendBuf();
}

//***********************************************************************************
// brief:   
// 
// parameter: none
//***********************************************************************************
void NodeStrategyPeriodReset(void)
{
    nodeStrategy.sendCnt      = 0;
    nodeStrategy.radioBusyCnt = 0;
}



//***********************************************************************************
// brief:   adjust the time of the node sendout
// 
// parameter: none
//***********************************************************************************
bool NodeStrategySendPacket(uint8_t *dataP, uint8_t len)
{
    bool flag = true;
    if(len > nodeStrategy.remainderCache)
        return false;

    flag = RadioCopyPacketToBuf(dataP, len, 0, RADIO_RECEIVE_TIMEOUT, EASYLINK_MAX_DATA_LENGTH - nodeStrategy.remainderCache);
    nodeStrategy.remainderCache -= len;

    return flag;
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
    nodeStrategy.success       = true;
    nodeStrategy.periodFailNum = 0;
    nodeStrategy.sendCnt       = 0;
    nodeStrategy.radioBusyCnt = 0;
}


//***********************************************************************************
// brief: check the radio in air is busy, and the recheck the rssi later
// 
// parameter: 
//***********************************************************************************
void StrategyCheckRssiBusyProcess(void)
{
    nodeStrategy.radioBusyCnt++;
    if(nodeStrategy.radioBusyCnt < FAIL_CHECK_RSSI_BUSY_MAX_NUM)
    {
        NodeStrategyReceiveTimeoutProcess();
        if(nodeStrategy.sendCnt)
            nodeStrategy.sendCnt --;
    }
    else
    {
        nodeStrategy.radioBusyCnt = 0;
        nodeStrategy.sendCnt      = 0;
        if(!(g_rSysConfigInfo.rfStatus & STATUS_LORA_CHANGE_FREQ))
            AutoFreqNodeSwitchFreq();
    }
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

    // transform to ms
    concenterTick = concenterTick / 100 + (length * 16 / 10);


    // if(nodeStrategy.success)
    // {
    //     if(channel != nodeStrategy.channel)
    //     {
    //         // readjust the timer
    //         goto ReadjustChannel;
    //     }
    //     else
    //     {
    //         offsetTicksTemp = concenterTick - Clock_getTicks();
    //         if(offsetTicksTemp > nodeStrategy.offsetTicks)
    //         {
    //             if((offsetTicksTemp - nodeStrategy.offsetTicks) > 200 * CLOCK_UNIT_MS)
    //                 goto ReadjustChannel;
    //         }
    //         else
    //         {
    //             if((nodeStrategy.offsetTicks - offsetTicksTemp) > 200 * CLOCK_UNIT_MS)
    //                 goto ReadjustChannel;
    //         }
    //     }
    // }
    // else
    {
        // need to register the new channel
//ReadjustChannel:

        nodeStrategy.offsetTicks = concenterTick - Clock_getTicks();
        nodeStrategy.success     = true;
        nodeStrategy.channel     = channel;

        concenterTick = concenterTick % (nodeStrategy.period * 1000);
        // transform to ms
        // launchTime  = nodeStrategy.channel * nodeStrategy.period * 1000 / nodeStrategy.channelNum;
        // the channel time space fix 
        launchTime  = nodeStrategy.channel * SORT_CHANNEL_TIME_SLOT;
        
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

        if(channel != g_rSysConfigInfo.alarmuploadPeriod)
        {
            g_rSysConfigInfo.alarmuploadPeriod = channel;
            Sys_event_post(SYSTEMAPP_EVT_STORE_SYS_CONFIG);
        }
    }
}


//***********************************************************************************
// brief:   get the channel dispatch from concentor
// 
// parameter: 
//***********************************************************************************
uint32_t NodeStrategyGetChannel(void)
{
    return nodeStrategy.channel;
}


