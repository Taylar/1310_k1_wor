#include "../general.h"
#include "zks/easylink/EasyLink.h"
#include "pass_radio_task.h"
#include "interface.h"




/***** Defines *****/
#define PASSRADIO_TASK_STACK_SIZE 1024
#define PASSRADIO_TASK_PRIORITY   3


#define         RADIO_EVT_RX            Event_Id_00
#define         RADIO_EVT_TX            Event_Id_01
#define         RADIO_EVT_TOUT          Event_Id_02
#define         RADIO_EVT_FAIL          Event_Id_03
#define         RADIO_EVT_ALL           0xffff


#define PASSRADIO_MAX_RETRIES 2

#define RADIO_ADDR_LEN                  8


/***** Type declarations *****/
struct RadioOperation {
    EasyLink_TxPacket easyLinkTxPacket;
    uint8_t retriesDone;
    uint8_t maxNumberOfRetries;
    uint32_t ackTimeoutMs;
    enum RadioOperationStatus result;
};


/***** Variable declarations *****/
static Task_Params passRadioTaskParams;
Task_Struct passRadioTask;        /* not static so you can see in ROV */
static uint8_t nodeRadioTaskStack[PASSRADIO_TASK_STACK_SIZE];
Semaphore_Struct radioAccessSem;  /* not static so you can see in ROV */
static Semaphore_Handle radioAccessSemHandle;
Event_Struct radioOperationEvent; /* not static so you can see in ROV */
static Event_Handle radioOperationEventHandle;

static uint8_t  srcRadioAddr[RADIO_ADDR_LEN];
static uint8_t  srcAddrLen;
static uint8_t  dstRadioAddr[RADIO_ADDR_LEN];
static uint8_t  dstAddrLen;

static uint8_t  radioMode;


static struct RadioOperation currentRadioOperation;

static EasyLink_RxPacket radioRxPacket;


/***** Prototypes *****/

void PassRadioTaskFxn(void);

static void RxDoneCallback(EasyLink_RxPacket * rxPacket, EasyLink_Status status);

void RadioResendPacket(void);



/***** Function definitions *****/

static void RadioDefaultParaInit(void)
{

    memset(&srcRadioAddr, 0, sizeof(srcRadioAddr));
    memset(&dstRadioAddr, 0, sizeof(dstRadioAddr));
    srcAddrLen      = RADIO_ADDR_LEN;
    dstAddrLen      = RADIO_ADDR_LEN;

    radioMode       = RADIOMODE_SENDPORT;
    // set the radio addr length
    EasyLink_setCtrl(EasyLink_Ctrl_AddSize, RADIO_ADDR_LEN);
}



void PassRadioTaskCreate(void) 
{

    /* Create semaphore used for exclusive radio access */ 
    Semaphore_Params semParam;
    Semaphore_Params_init(&semParam);
    Semaphore_construct(&radioAccessSem, 1, &semParam);
    radioAccessSemHandle = Semaphore_handle(&radioAccessSem);


    /* Create event used internally for state changes */
    Event_Params eventParam;
    Event_Params_init(&eventParam);
    Event_construct(&radioOperationEvent, &eventParam);
    radioOperationEventHandle = Event_handle(&radioOperationEvent);

    /* Create the radio protocol task */
    Task_Params_init(&passRadioTaskParams);
    passRadioTaskParams.stackSize = PASSRADIO_TASK_STACK_SIZE;
    passRadioTaskParams.priority = PASSRADIO_TASK_PRIORITY;
    passRadioTaskParams.stack = &nodeRadioTaskStack;
    Task_construct(&passRadioTask, (Task_FuncPtr)PassRadioTaskFxn, &passRadioTaskParams, NULL);
}


void PassRadioTaskFxn(void)
{
    if(EasyLink_init(RADIO_EASYLINK_MODULATION) != EasyLink_Status_Success) {
        System_abort("EasyLink_init failed");
    }

    RadioDefaultParaInit();
    /* If you wich to use a frequency other than the default use
     * the below API
     * EasyLink_setFrequency(868000000);
     */
    // radioMode       = RADIOMODE_RECEIVEPORT;
    
    /* Set the filter to the generated random address */
    if (EasyLink_enableRxAddrFilter(srcRadioAddr, srcAddrLen, 1) != EasyLink_Status_Success)
    {
        System_abort("EasyLink_enableRxAddrFilter failed");
    }

    if(radioMode == RADIOMODE_RECEIVEPORT)
        EasyLink_receiveAsync(RxDoneCallback, 0);

    for(;;)
    {
        uint32_t events = Event_pend(radioOperationEventHandle, 0, RADIO_EVT_ALL, BIOS_WAIT_FOREVER);

        if (events & RADIO_EVT_RX)
        {
            // protocol distribute


            if(radioMode == RADIOMODE_RECEIVEPORT)
            {
                RadioSendPacket(radioRxPacket.payload, radioRxPacket.len, 0, 0);
                // RadioSendPacket2(radioRxPacket.payload, radioRxPacket.len, 0, 0);
            }
            else
            {
                InterfaceSend(radioRxPacket.payload, radioRxPacket.len);
            }

            if(radioMode == RADIOMODE_RECEIVEPORT)
                EasyLink_receiveAsync(RxDoneCallback, 0);
        }

        if (events & RADIO_EVT_TX)
        {
            // stop receive radio, otherwise couldn't send successful
            EasyLink_abort();
            EasyLink_transmit(&currentRadioOperation.easyLinkTxPacket);
            if(radioMode == RADIOMODE_SENDPORT)
            {
                EasyLink_setCtrl(EasyLink_Ctrl_AsyncRx_TimeOut, EasyLink_ms_To_RadioTime(currentRadioOperation.ackTimeoutMs));
                EasyLink_receiveAsync(RxDoneCallback, 0);
            }

            if(radioMode == RADIOMODE_RECEIVEPORT)
            {
                EasyLink_setCtrl(EasyLink_Ctrl_AsyncRx_TimeOut, 0);
                EasyLink_receiveAsync(RxDoneCallback, 0);
            }
        }


        if (events & RADIO_EVT_TOUT)
        {
            /* If we haven't resent it the maximum number of times yet, then resend packet */
            if (currentRadioOperation.retriesDone < currentRadioOperation.maxNumberOfRetries)
            {
                RadioResendPacket();
            }
            else
            {
                /* Else return send fail */
                Event_post(radioOperationEventHandle, RADIO_EVT_FAIL);
            }
            
        }


        if (events & RADIO_EVT_FAIL)
        {

        }


    }
}


void RadioSendPacket(uint8_t *dataP, uint8_t len, uint8_t maxNumberOfRetries, uint32_t ackTimeoutMs)
{
    /* Set destination address in EasyLink API */
    memcpy(currentRadioOperation.easyLinkTxPacket.dstAddr, dstRadioAddr, dstAddrLen);

    currentRadioOperation.easyLinkTxPacket.len      = len;
    memcpy(currentRadioOperation.easyLinkTxPacket.payload, dataP, len);

    /* Copy ADC packet to payload
     * Note that the EasyLink API will implcitily both add the length byte and the destination address byte. */

    /* Setup retries */
    currentRadioOperation.maxNumberOfRetries = maxNumberOfRetries;
    currentRadioOperation.ackTimeoutMs = ackTimeoutMs;
    currentRadioOperation.retriesDone = 0;

    Event_post(radioOperationEventHandle, RADIO_EVT_TX);

}





void RadioResendPacket(void)
{
    EasyLink_transmit(&currentRadioOperation.easyLinkTxPacket);
            
    if(radioMode == RADIOMODE_SENDPORT)
        EasyLink_receiveAsync(RxDoneCallback, 0);

    currentRadioOperation.retriesDone++;
}




static void RxDoneCallback(EasyLink_RxPacket * rxPacket, EasyLink_Status status)
{

    /* If this callback is called because of a packet received */
    if (status == EasyLink_Status_Success)
    {
        memcpy(&radioRxPacket, rxPacket, sizeof(EasyLink_RxPacket));
        Event_post(radioOperationEventHandle, RADIO_EVT_RX);
    }
    /* did the Rx timeout */
    else if(status == EasyLink_Status_Rx_Timeout)
    {
        /* Post a RADIO_EVT_TOUT event */
        Event_post(radioOperationEventHandle, RADIO_EVT_TOUT);
    }
    else
    {
        /* The Ack receiption may have been corrupted causing an error.
         * Treat this as a timeout
         */
        Event_post(radioOperationEventHandle, RADIO_EVT_TOUT);
    }
}
