#include "../general.h"
#include "interface.h"

// **************************************************************************
// task
#define         INTERFACE_STACK_SIZE        1024
uint8_t         interfaceTaskStack[INTERFACE_STACK_SIZE];

Task_Struct     interfaceTaskStruct;


// **************************************************************************
// event 
Event_Struct interfaceEvtStruct;
Event_Handle interfaceEvtHandle;


Semaphore_Struct interfaceSemStruct;
Semaphore_Handle interfaceSemHandle;

// **************************************************************************
// variable
static uint8_t     interfaceRecBuf[INTERFACE_DATA_MAX_LEN];
static uint8_t     interfaceSendBuf[INTERFACE_DATA_MAX_LEN];

static uint8_t     recLen;
static uint8_t     sendLen;

void InterfaceTaskFxn(void);

void InterfaceReceiveCb(uint8_t *datap, uint8_t len)
{
    recLen    =  len > INTERFACE_DATA_MAX_LEN?INTERFACE_STACK_SIZE: len;
    memcpy(interfaceRecBuf, datap, recLen);
    Event_post(interfaceEvtHandle, INTERFACE_EVT_RX);    
}


void InterfaceSend(uint8_t * datap, uint8_t len)
{
    sendLen =  len > INTERFACE_DATA_MAX_LEN? INTERFACE_STACK_SIZE: len;
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




void InterfaceTaskCreate(void)
{
    Error_Block eb;
    Error_init(&eb);

    /* Construct main system process Task threads */
    Task_Params taskParams;
    Task_Params_init(&taskParams);
    taskParams.stackSize = INTERFACE_STACK_SIZE;
    taskParams.stack = &interfaceTaskStack;
    taskParams.priority = 1;
    Task_construct(&interfaceTaskStruct, (Task_FuncPtr)InterfaceTaskFxn, &taskParams, &eb);


    Semaphore_Params semParams;
    Semaphore_Params_init(&semParams);
    semParams.mode = Semaphore_Mode_BINARY;
    Semaphore_construct(&interfaceSemStruct, 1, &semParams);
    interfaceSemHandle = Semaphore_handle(&interfaceSemStruct);
}

uint8_t test[20] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14};

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

    InterfaceSend(test, 20);

    for(;;)
    {
        eventId = Event_pend(interfaceEvtHandle, 0, INTERFACE_EVT_ALL, BIOS_WAIT_FOREVER);

        if(eventId & INTERFACE_EVT_RX)
        {
            InterfaceSend(interfaceRecBuf, recLen);
        }

        if(eventId & INTERFACE_EVT_TX)
        {
            Semaphore_pend(interfaceSemHandle, BIOS_WAIT_FOREVER);
            UartSend(interfaceSendBuf, sendLen);
            Semaphore_post(interfaceSemHandle);
        }
    }
}



