#include "../general.h"
#include "pass_radio_task.h"
#include "interface.h"

/***** Defines *****/
#define         INTERFACE_STACK_SIZE        1024

#define         INTERFACE_REC_TIMEOUT_MS    3
// **************************************************************************
// task
uint8_t         interfaceTaskStack[INTERFACE_STACK_SIZE];

Task_Struct     interfaceTaskStruct;


// **************************************************************************
// event 
Event_Struct interfaceEvtStruct;
Event_Handle interfaceEvtHandle;


Semaphore_Struct interfaceSemStruct;
Semaphore_Handle interfaceSemHandle;


/* Clock for the fast report timeout */

Clock_Struct interfaceRecTimeoutClock;     /* not static so you can see in ROV */

static Clock_Handle interfaceRecTimeoutClockHandle;


// **************************************************************************
// variable
static uint8_t     interfaceRecBuf[INTERFACE_DATA_MAX_LEN];
static uint8_t     interfaceIsrRecBuf[INTERFACE_DATA_MAX_LEN];
static uint8_t     interfaceSendBuf[INTERFACE_DATA_MAX_LEN];

uint8_t     recLen;
uint8_t     sendLen;
static uint8_t     recIsrLen;

void InterfaceTaskFxn(void);

// 
void InterfaceReceiveCb(uint8_t *datap, uint8_t len)
{
    uint8_t  datapLen;
    if(Clock_isActive(interfaceRecTimeoutClockHandle))
        Clock_stop(interfaceRecTimeoutClockHandle);


    Clock_setTimeout(interfaceRecTimeoutClockHandle,
            INTERFACE_REC_TIMEOUT_MS * 1000 / Clock_tickPeriod);
    Clock_start(interfaceRecTimeoutClockHandle);

    datapLen        = 0;
    while(len)
    {
        len--;
        interfaceIsrRecBuf[recIsrLen] = datap[datapLen];
        datapLen++;
        recIsrLen++;

        if(recIsrLen >= INTERFACE_DATA_MAX_LEN)
        {
            recLen    = recIsrLen;
            recIsrLen = 0;
            datapLen  = 0;

            memcpy(interfaceRecBuf, interfaceIsrRecBuf, recLen);
            Event_post(interfaceEvtHandle, INTERFACE_EVT_RX);
        }
    }
}


void InterfaceSend(uint8_t * datap, uint8_t len)
{
    sendLen =  len > INTERFACE_DATA_MAX_LEN? INTERFACE_DATA_MAX_LEN: len;
    memcpy(interfaceSendBuf, datap, sendLen);
    Event_post(interfaceEvtHandle, INTERFACE_EVT_TX);    
}


uint32_t HwInterfaceInit(INTERFACE_TYPE type, uint32_t baudRate, UART_CB_T cb)
{
    switch(type)
    {
        case INTERFACE_UART:
        UartSoftInit(baudRate, cb);
        break;

        case INTERFACE_SPI:

        break;

        default:
        break;
    }
    return 0;
}


// this may occur a rec error when InterfaceReceiveCb syn occur, this function shouldn't be interrupt
// by InterfaceReceiveCb, add the bi lock maybe will occur error
void InterfaceRecTimeroutCb(UArg arg0)
{
    recLen    = recIsrLen;
    recIsrLen = 0;

    memcpy(interfaceRecBuf, interfaceIsrRecBuf, recLen);
    Event_post(interfaceEvtHandle, INTERFACE_EVT_RX);
}



void InterfaceTaskCreate(void)
{
    Error_Block eb;
    Error_init(&eb);


    /* Create clock object which is used for fast report timeout */
    Clock_Params clkParams;
    clkParams.period = 0;
    clkParams.startFlag = FALSE;
    Clock_construct(&interfaceRecTimeoutClock, InterfaceRecTimeroutCb, 1, &clkParams);
    interfaceRecTimeoutClockHandle = Clock_handle(&interfaceRecTimeoutClock);


    /* Construct the semparams for interface  */
    Semaphore_Params semParams;
    Semaphore_Params_init(&semParams);
    semParams.mode = Semaphore_Mode_BINARY;
    Semaphore_construct(&interfaceSemStruct, 1, &semParams);
    interfaceSemHandle = Semaphore_handle(&interfaceSemStruct);

    /* Construct interface process Task threads */
    Task_Params taskParams;
    Task_Params_init(&taskParams);
    taskParams.stackSize = INTERFACE_STACK_SIZE;
    taskParams.stack = &interfaceTaskStack;
    taskParams.priority = 1;
    Task_construct(&interfaceTaskStruct, (Task_FuncPtr)InterfaceTaskFxn, &taskParams, &eb);
}


void InterfaceTaskFxn(void)
{
    uint32_t    eventId;

    /* Construct system process Event */
    Event_construct(&interfaceEvtStruct, NULL);
    /* Obtain event instance handle */
    interfaceEvtHandle = Event_handle(&interfaceEvtStruct);

    Semaphore_pend(interfaceSemHandle, BIOS_WAIT_FOREVER);
    HwInterfaceInit(INTERFACE_UART, 115200, InterfaceReceiveCb);
    Semaphore_post(interfaceSemHandle);

    // UartClose(UART_0);
    // HwInterfaceInit(INTERFACE_UART, 115200, InterfaceReceiveCb);

    for(;;)
    {
        eventId = Event_pend(interfaceEvtHandle, 0, INTERFACE_EVT_ALL, BIOS_WAIT_FOREVER);

        if(eventId & INTERFACE_EVT_RX)
        {
            InterfaceSend(interfaceRecBuf, recLen);
            RadioSendPacket(interfaceRecBuf, recLen, 0, PASSRADIO_ACK_TIMEOUT_TIME_MS);

        }

        if(eventId & INTERFACE_EVT_TX)
        {
            Semaphore_pend(interfaceSemHandle, BIOS_WAIT_FOREVER);
            UartSend(interfaceSendBuf, sendLen);
            Semaphore_post(interfaceSemHandle);
        }
    }
}



