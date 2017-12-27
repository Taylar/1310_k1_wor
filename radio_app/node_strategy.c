#include "../general.h"
#include <ti/sysbios/BIOS.h>
#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26XX.h>



#include "node_strategy.h"
#include "../radio_app/radio_app.h"

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/aon_batmon.h)
#include DeviceFamily_constructPath(driverlib/trng.h)
/***** Defines *****/


/***** Type declarations *****/
typedef struct {
    bool        init;
    bool        success;
    bool        busy;
    uint8_t     remainderCache;
    uint32_t    period;
}node_strategy_t;

/***** Variable declarations *****/

/* Clock for node period sending */

static Clock_Struct nodeStrategyPeriodClock;     /* not static so you can see in ROV */
static Clock_Handle nodeStrategyPeriodClockHandle;

static Clock_Struct nodeStrategyStartClock;     /* not static so you can see in ROV */
static Clock_Handle nodeStrategyStartClockHandle;



// the 
static node_strategy_t      nodeStrategy;

/***** Prototypes *****/


static void (*NodeStrategyPeriodCb)(void);
/***** Function definitions *****/

//***********************************************************************************
// brief:   the node period send Cb Event
// 
// parameter: none
//***********************************************************************************
static void NodeStrategyPeriodClockCb(UArg arg0)
{
    if((NodeStrategyPeriodCb != NULL) && (nodeStrategy.busy == true) && (nodeStrategy.success == true))
        NodeStrategyPeriodCb();
}


static void NodeStrategyStartCb(UArg arg0)
{
    if((NodeStrategyPeriodCb != NULL) && (nodeStrategy.busy == false))
        NodeStrategyPeriodCb();

    Clock_start(nodeStrategyPeriodClockHandle);
}


//***********************************************************************************
// brief:   Init the NodeStrategy
// 
// parameter: 
// Cb:      Init the send radio event
//***********************************************************************************
void NodeStrategyInit(void (*Cb)(void))
{
    if(nodeStrategy.init == false)
    {
        nodeStrategy.init    = true;
        nodeStrategy.success = false;
        nodeStrategy.busy    = false;

        nodeStrategy.remainderCache       = EASYLINK_MAX_DATA_LENGTH;

        Clock_Params clkParams;
        clkParams.period    = 0;
        clkParams.startFlag = FALSE;
        Clock_construct(&nodeStrategyStartClock, NodeStrategyStartCb, 1, &clkParams);
        nodeStrategyStartClockHandle = Clock_handle(&nodeStrategyStartClock);


        clkParams.period    = 0;
        clkParams.startFlag = FALSE;
        Clock_construct(&nodeStrategyPeriodClock, NodeStrategyPeriodClockCb, 1, &clkParams);
        nodeStrategyPeriodClockHandle = Clock_handle(&nodeStrategyPeriodClock);


        NodeStrategyPeriodCb        = Cb;       
    }
}

//***********************************************************************************
// brief:   set the node strategy peirod
// 
// parameter: 
// period:  the uint is ms
//***********************************************************************************
void NodeStrategySetPeriod(uint32_t period)
{
    Clock_setPeriod(nodeStrategyPeriodClockHandle, period);
}



static void NodeStrategyStart(void)
{
    uint32_t randomNum;

    if(Clock_isActive(nodeStrategyPeriodClockHandle))
        Clock_stop(nodeStrategyPeriodClockHandle);


    /* Use the True Random Number Generator to generate sensor node address randomly */;
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

    Clock_setTimeout(nodeStrategyStartClockHandle, randomNum % nodeStrategy.period);
    Clock_start(nodeStrategyStartClockHandle);
}

//***********************************************************************************
// brief:   adjust the time of the node sendout
// 
// parameter: none
//***********************************************************************************
void NodeStrategyReceiveTimeoutProcess(void)
{
    nodeStrategy.success    = false;
}


//***********************************************************************************
// brief:   adjust the time of the node sendout
// 
// parameter: none
//***********************************************************************************
void NodeStrategyReceiveReceiveSuccess(void)
{
    nodeStrategy.busy           = false;
    nodeStrategy.success        = true;
    nodeStrategy.remainderCache = EASYLINK_MAX_DATA_LENGTH;

}


//***********************************************************************************
// brief:   adjust the time of the node sendout
// 
// parameter: none
//***********************************************************************************
uint8_t NodeStrategySendPacket(uint8_t *dataP, uint8_t len)
{
    if(len > nodeStrategy.remainderCache)
        return 0xff;
    
    if(nodeStrategy.success == false)
    {
        NodeStrategyStart();
    }


    RadioCopyPacketToBuf(dataP, len, 0, PASSRADIO_ACK_TIMEOUT_TIME_MS, EASYLINK_MAX_DATA_LENGTH - nodeStrategy.remainderCache);

    nodeStrategy.remainderCache -= len;
    nodeStrategy.busy       = true;

    return 0x00;
}



void NodeStrategyBusySet(bool boolFlag)
{
    nodeStrategy.busy       = boolFlag;

    nodeStrategy.remainderCache = EASYLINK_MAX_DATA_LENGTH;
}


bool NodeStrategyBusyRead(void)
{
    return nodeStrategy.busy;
}


uint8_t NodeStrategyRemainderCache(void)
{
    return nodeStrategy.remainderCache;
}



