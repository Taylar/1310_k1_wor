/*
* @Author: zxt
* @Date:   2017-12-21 17:36:18
* @Last Modified by:   zxt
* @Last Modified time: 2018-09-11 17:50:11
*/
#include "../general.h"
//#include "../radio_app/radio_app.h"
//#include "interface.h"
//#include "../app/usb_prot.h"

// #if (defined BOARD_B2S) || (defined BOARD_S6_6)

/***** Defines *****/
#define         INTERFACE_STACK_SIZE        840

#define         INTERFACE_REC_TIMEOUT_MS    3


#if (defined(BOARD_S6_6) || defined(BOARD_B2S))
//#define INTERFACE_REQ_PIN                            IOID_22
//
//const PIN_Config Int_Req_PinTable[] = {
//    INTERFACE_REQ_PIN | PIN_INPUT_EN | PIN_NOPULL | PIN_IRQ_NEGEDGE,       /* key isr enable          */
//    PIN_TERMINATE
//};

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

bool            interfaceFlag;

uint8_t         interfaceBusyFlag;
uint8_t         interfaceTimerFlag;

void InterfaceTaskFxn(void);

// call back in uart isr 
void InterfaceReceiveCb(void)
{
    if(interfaceBusyFlag == 0)
    {

        interfaceBusyFlag = 1;
        Clock_start(interfaceRecTimeoutClockHandle);
    }

    interfaceTimerFlag = 1;
}


void InterfaceSend(uint8_t * datap, uint16_t len)
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

    if(interfaceTimerFlag == 1)
    {
        interfaceTimerFlag = 0;
    }
    else
    {
        if(interfaceBusyFlag == 1)
        {
            Clock_stop(interfaceRecTimeoutClockHandle);
            interfaceBusyFlag = 0;
            uart0RxData.length    = g_rUart1RxData.length;
            g_rUart1RxData.length = 0;
            memcpy(uart0RxData.buff, g_rUart1RxData.buff, uart0RxData.length);;
            Event_post(interfaceEvtHandle, INTERFACE_EVT_RX);
        }    
    }

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
    Clock_setTimeout(interfaceRecTimeoutClockHandle, 8 * CLOCK_UNIT_MS);
    Clock_setPeriod(interfaceRecTimeoutClockHandle, 8 * CLOCK_UNIT_MS);

    interfaceBusyFlag  = 0;
    interfaceTimerFlag = 0;

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

    Event_Params eventParam;
    Event_Params_init(&eventParam);
    /* Construct system process Event */
    Event_construct(&interfaceEvtStruct, &eventParam);
    /* Obtain event instance handle */
    interfaceEvtHandle = Event_handle(&interfaceEvtStruct);

    // the systask process first
    Task_sleep(10 * CLOCK_UNIT_MS);

    interfaceFlag       = FALSE;

    for(;;)
    {
        eventId = Event_pend(interfaceEvtHandle, 0, INTERFACE_EVT_ALL, BIOS_WAIT_FOREVER);

        if(eventId & INTERFACE_EVT_RX)
        {
            Usb_data_parse(uart0RxData.buff, uart0RxData.length);
        }

        if(eventId & INTERFACE_EVT_TX)
        {
            Semaphore_pend(interfaceSemHandle, BIOS_WAIT_FOREVER);

            if(interfaceFlag)
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
    Semaphore_pend(interfaceSemHandle, BIOS_WAIT_FOREVER);
    interfaceFlag       = true;
    HwInterfaceInit(INTERFACE_UART, 115200, InterfaceReceiveCb);
    Semaphore_post(interfaceSemHandle);
}

//***********************************************************************************
// brief:   disable the interface
// 
// parameter: 
// Cb:      
//***********************************************************************************
void InterfaceDisable(void)
{
    Semaphore_pend(interfaceSemHandle, BIOS_WAIT_FOREVER);
    interfaceFlag       = FALSE;
    UartClose(UART_0);
    UartPortDisable(UART_INTERFACE);
    Semaphore_post(interfaceSemHandle);
}

// #endif
