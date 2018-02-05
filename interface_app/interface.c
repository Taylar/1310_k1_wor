#include "../general.h"
#include "../radio_app/radio_app.h"
#include "interface.h"

#if (defined BOARD_S2_2) || (defined BOARD_S6_6)

/***** Defines *****/
#define         INTERFACE_STACK_SIZE        512

#define         INTERFACE_REC_TIMEOUT_MS    3


#ifdef BOARD_S6_6
#define INTERFACE_REQ_PIN                            IOID_22

const PIN_Config Int_Req_PinTable[] = {
    INTERFACE_REQ_PIN | PIN_INPUT_EN | PIN_NOPULL | PIN_IRQ_NEGEDGE,       /* key isr enable          */
    PIN_TERMINATE
};

#endif


















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

static Clock_Struct interfaceRecTimeoutClock;     /* not static so you can see in ROV */

static Clock_Handle interfaceRecTimeoutClockHandle;


// **************************************************************************
// variable
UartRxData_t     uart0RxData;
UartRxData_t     uart0IsrRxData;
UartRxData_t     uart0TxData;


void InterfaceTaskFxn(void);

// call back in uart isr 
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
        uart0IsrRxData.buff[uart0IsrRxData.length] = datap[datapLen];
        datapLen++;
        uart0IsrRxData.length++;

        if(uart0IsrRxData.length >= UART_BUFF_SIZE)
        {
            uart0RxData.length    = uart0IsrRxData.length;
            uart0IsrRxData.length = 0;
            datapLen  = 0;

            memcpy(uart0RxData.buff, uart0IsrRxData.buff, uart0RxData.length);
            Event_post(interfaceEvtHandle, INTERFACE_EVT_RX);
        }
    }
}


void InterfaceSend(uint8_t * datap, uint8_t len)
{
    uart0TxData.length =  len > UART_BUFF_SIZE? UART_BUFF_SIZE: len;
    memcpy(uart0TxData.buff, datap, uart0TxData.length);
    Event_post(interfaceEvtHandle, INTERFACE_EVT_TX);    
}


uint32_t HwInterfaceInit(INTERFACE_TYPE type, uint32_t baudRate, UART_CB_T cb)
{
    switch(type)
    {
        case INTERFACE_UART:
        UartSoftInit(baudRate, cb, UART_INTERFACE);
        break;

        case INTERFACE_SPI:

        break;

        default:
        break;
    }
    return 0;
}


void InterfaceRecTimeroutCb(UArg arg0)
{
    UInt key;

    /* Disable preemption. */
    key = Hwi_disable();

    uart0RxData.length    = uart0IsrRxData.length;
    uart0IsrRxData.length = 0;
    memcpy(uart0RxData.buff, uart0IsrRxData.buff, uart0RxData.length);

    Hwi_restore(key);
    Event_post(interfaceEvtHandle, INTERFACE_EVT_RX);
}



void InterfaceTaskCreate(void)
{
    Error_Block eb;
    Error_init(&eb);


    /* Create clock object which is used for fast report timeout */
    Clock_Params clkParams;
    Clock_Params_init(&clkParams);
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
    taskParams.priority = 2;
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
    InterfaceEnable();
    Semaphore_post(interfaceSemHandle);

    // UartClose(UART_0);

    for(;;)
    {
        eventId = Event_pend(interfaceEvtHandle, 0, INTERFACE_EVT_ALL, BIOS_WAIT_FOREVER);

        if(eventId & INTERFACE_EVT_RX)
        {
            
            // InterfaceSend(uart0RxData.buff, uart0RxData.length);
        }

        if(eventId & INTERFACE_EVT_TX)
        {
            Semaphore_pend(interfaceSemHandle, BIOS_WAIT_FOREVER);
            UartSend(uart0TxData.buff, uart0TxData.length);
            Semaphore_post(interfaceSemHandle);
        }
    }
}

//***********************************************************************************
// brief:   Enable the interface 
// 
// parameter: 
// Cb:      
//***********************************************************************************
void InterfaceEnable(void)
{
    HwInterfaceInit(INTERFACE_UART, 115200, InterfaceReceiveCb);
}

//***********************************************************************************
// brief:   disable the interface
// 
// parameter: 
// Cb:      
//***********************************************************************************
void InterfaceDisable(void)
{
    UartClose(UART_0);
}

#endif
