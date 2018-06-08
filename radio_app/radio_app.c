/*
* @Author: zxt
* @Date:   2017-12-21 17:36:18
* @Last Modified by:   zxt
* @Last Modified time: 2018-06-08 12:01:28
*/
#include "../general.h"
#include "zks/easylink/EasyLink.h"
#include "radio_app.h"
#include "node_strategy.h"
#include "../interface_app/interface.h"
#include "../app/radio_protocal.h"
#include "../app/concenterApp.h"
#include "../app/nodeApp.h"


/***** Defines *****/
#define PASSRADIO_TASK_STACK_SIZE 768
#define PASSRADIO_TASK_PRIORITY   3



#define PASSRADIO_MAX_RETRIES           2

#define RADIO_ADDR_LEN                  4
#define RADIO_ADDR_LEN_MAX              8






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
Semaphore_Handle radioAccessSemHandle;
Event_Struct radioOperationEvent; /* not static so you can see in ROV */
static Event_Handle radioOperationEventHandle;

static uint8_t  srcRadioAddr[RADIO_ADDR_LEN_MAX];
static uint8_t  srcAddrLen;
static uint8_t  dstRadioAddr[RADIO_ADDR_LEN];
static uint8_t  dstAddrLen;

static uint8_t  radioMode;


static struct RadioOperation currentRadioOperation;

EasyLink_RxPacket radioRxPacket;

static uint8_t  radioTestFlag;

uint8_t radioStatus;

/***** Prototypes *****/

void RadioAppTaskFxn(void);

static void RxDoneCallback(EasyLink_RxPacket * rxPacket, EasyLink_Status status);

void RadioResendPacket(void);



/***** Function definitions *****/

//***********************************************************************************
// brief:   set the radio addr length
// 
// parameter:   none 
//***********************************************************************************
static void RadioDefaultParaInit(void)
{

    srcAddrLen      = RADIO_ADDR_LEN;
    dstAddrLen      = RADIO_ADDR_LEN;
    // set the radio addr length
    EasyLink_setCtrl(EasyLink_Ctrl_AddSize, RADIO_ADDR_LEN);
}



//***********************************************************************************
// brief:   create the radio task
// 
// parameter:   none 
//***********************************************************************************
void RadioAppTaskCreate(void)
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
    Task_construct(&passRadioTask, (Task_FuncPtr)RadioAppTaskFxn, &passRadioTaskParams, NULL);
}


//***********************************************************************************
// brief:   set the radio tx mode or rx mode
// 
// parameter:   none 
//***********************************************************************************
void RadioModeSet(RadioOperationMode modeSet)
{
    if(modeSet == RADIOMODE_SENDPORT)
    {
        RadioSetTxMode();
    }

    if(modeSet == RADIOMODE_RECEIVEPORT)
    {
        RadioSetRxMode();
    }
}

//***********************************************************************************
// brief:   radio task 
// 
// parameter:   none 
//***********************************************************************************
void RadioAppTaskFxn(void)
{
    int8_t rssi;

    // the sys task process first, should read the g_rSysConfigInfo
    Task_sleep(50 * CLOCK_UNIT_MS);

    // init the easylink
    EasyLink_Params easyLink_params;
    EasyLink_Params_init(&easyLink_params);
    
    easyLink_params.ui32ModType = RADIO_EASYLINK_MODULATION;
    
    if(EasyLink_init(&easyLink_params) != EasyLink_Status_Success){ 
        System_abort("EasyLink_init failed");
    }

    radioStatus = RADIOSTATUS_IDLE;

#ifdef  BOARD_S6_6
    RadioFrontInit();

#ifdef  BOARD_CONFIG_DECEIVE
    g_rSysConfigInfo.rfStatus |= STATUS_1310_MASTER;
#endif  // BOARD_CONFIG_DECEIVE

    if(g_rSysConfigInfo.rfStatus & STATUS_1310_MASTER)
    {
        RadioFrontRxEnable();
        radioMode = RADIOMODE_RECEIVEPORT;
    }
    else
    {
        radioMode = RADIOMODE_SENDPORT;
    }
    
#endif  // BOARD_S6_6

#ifdef  BOARD_S1_2

    radioMode = RADIOMODE_SENDPORT;

#endif // BOARD_S1_2

    // set the default para
    RadioDefaultParaInit();

    if(radioMode == RADIOMODE_SENDPORT)
    {
        NodeAppInit(RadioSend);
    }
    else
    {
        ConcenterAppInit();
    }

    

#ifdef FACTOR_RADIO_TEST
    while(1)
    {
        RadioFrontTxEnable();
        EasyLink_abort();
        EasyLink_transmit(&currentRadioOperation.easyLinkTxPacket);
    }
#endif



    for(;;)
    {
        uint32_t events = Event_pend(radioOperationEventHandle, 0, RADIO_EVT_ALL, BIOS_WAIT_FOREVER);


        if (events & RADIO_EVT_TEST)
        {
            while(radioTestFlag)
            {
                RadioFrontTxEnable();
                EasyLink_abort();
                EasyLink_transmit(&currentRadioOperation.easyLinkTxPacket);
            }
            continue;
        }


        if (events & RADIO_EVT_TX)
        {
            Semaphore_pend(radioAccessSemHandle, BIOS_WAIT_FOREVER);
            
            EasyLink_getRssi(&rssi);
            if((currentRadioOperation.easyLinkTxPacket.len) <= 128 && (currentRadioOperation.easyLinkTxPacket.len > 0))// && (rssi <= -110))
            {
                // stop receive radio, otherwise couldn't send successful


                if(radioMode == RADIOMODE_RECEIVEPORT)
                {
#ifndef  BOARD_S1_2
                    Led_toggle(LED_B);
#endif
                    RadioFrontTxEnable();
                }
                
                if(radioStatus == RADIOSTATUS_RECEIVING)
                {
                    radioStatus = RADIOSTATUS_ABSORT;
                    EasyLink_abort();                    
                    radioStatus = RADIOSTATUS_IDLE;
                }

                radioStatus = RADIOSTATUS_TRANSMITTING;
                if (EasyLink_transmit(&currentRadioOperation.easyLinkTxPacket) != EasyLink_Status_Success)
                {
                    System_abort("EasyLink_transmit failed");
                }

                if(radioMode == RADIOMODE_SENDPORT)
                {
                    radioStatus = RADIOSTATUS_RECEIVING;
                    EasyLink_setCtrl(EasyLink_Ctrl_AsyncRx_TimeOut, EasyLink_ms_To_RadioTime(currentRadioOperation.ackTimeoutMs));
                    EasyLink_receiveAsync(RxDoneCallback, 0);
                }

                if(radioMode == RADIOMODE_RECEIVEPORT)
                {
                    radioStatus = RADIOSTATUS_RECEIVING;
                    RadioFrontRxEnable();
                    EasyLink_setCtrl(EasyLink_Ctrl_AsyncRx_TimeOut, 0);
                    if(EasyLink_receiveAsync(RxDoneCallback, 0) != EasyLink_Status_Success)
                    {
                        System_printf("open 1310 receive fail");
                    }
#ifndef  BOARD_S1_2
                    Led_toggle(LED_B);
#endif
                }
            }
            else
            {
                if((rssi > -110) && (currentRadioOperation.easyLinkTxPacket.len) <= 128 && (currentRadioOperation.easyLinkTxPacket.len > 0))
                {
                    Event_post(radioOperationEventHandle, RADIO_EVT_TOUT);
                }
            }

            Semaphore_post(radioAccessSemHandle);
        }


        if (events & RADIO_EVT_RX)
        {
            if(radioStatus == RADIOSTATUS_RECEIVING)
            {

                radioStatus = RADIOSTATUS_IDLE;

                if(radioMode == RADIOMODE_RECEIVEPORT)
                {
           
    #ifdef  BOARD_S1_2
                    NodeProtocalDispath(&radioRxPacket);
                    Led_ctrl(LED_B, 1, 250 * CLOCK_UNIT_MS, 2);
    #else
                    ConcenterProtocalDispath(&radioRxPacket);

    #ifdef  BOARD_CONFIG_DECEIVE
                    Led_ctrl(LED_B, 1, 250 * CLOCK_UNIT_MS, 1);
    #endif

    #endif           
                    radioStatus = RADIOSTATUS_RECEIVING;
                    EasyLink_receiveAsync(RxDoneCallback, 0);
                }

                if(radioMode == RADIOMODE_SENDPORT)
                {
                    NodeProtocalDispath(&radioRxPacket);
                }

            }
        }



        if (events & RADIO_EVT_TOUT)
        {
            radioStatus = RADIOSTATUS_IDLE;

            if(radioMode == RADIOMODE_SENDPORT)
            {
                NodeStrategyReceiveTimeoutProcess();
            }

            /* If we haven't resent it the maximum number of times yet, then resend packet */
            if (currentRadioOperation.retriesDone < currentRadioOperation.maxNumberOfRetries)
            {
                RadioResendPacket();
            }
            else
            {
                /* Else return send fail */
                // Event_post(radioOperationEventHandle, RADIO_EVT_FAIL);
            }
        }

        if(events & RADIO_EVT_SET_RX_MODE)
        {
            radioMode = RADIOMODE_RECEIVEPORT;
            if(radioStatus == RADIOSTATUS_IDLE)
            {
                radioStatus = RADIOSTATUS_RECEIVING;
                EasyLink_setCtrl(EasyLink_Ctrl_AsyncRx_TimeOut, 0);
                if(EasyLink_receiveAsync(RxDoneCallback, 0) != EasyLink_Status_Success)
                {
                    System_printf("open 1310 receive fail");
                }
            }
        }


        if(events & RADIO_EVT_SET_TX_MODE)
        {
            radioMode = RADIOMODE_SENDPORT;
            if(radioStatus == RADIOSTATUS_RECEIVING)
            {
                radioStatus = RADIOSTATUS_ABSORT;
                EasyLink_abort();
                radioStatus = RADIOSTATUS_IDLE;
            }
        }

        if(events & RADIO_EVT_DISABLE)
        {
            if(radioStatus == RADIOSTATUS_RECEIVING)
            {
                EasyLink_abort();
                RadioFrontDisable();
                radioStatus = RADIOSTATUS_IDLE;   
            }
        }


        if (events & (RADIO_EVT_FAIL | RADIO_EVT_RX_FAIL))
        {
            radioStatus = RADIOSTATUS_IDLE;
            if(radioMode == RADIOMODE_RECEIVEPORT)
            {
                radioStatus = RADIOSTATUS_RECEIVING;
                RadioFrontRxEnable();
                EasyLink_setCtrl(EasyLink_Ctrl_AsyncRx_TimeOut, 0);
                if(EasyLink_receiveAsync(RxDoneCallback, 0) != EasyLink_Status_Success)
                {
                    System_printf("open 1310 receive fail");
                }
            }

        }

    }
}



//***********************************************************************************
// brief:   copy the packet to Txbuf, but do not send immediately
// 
// parameter: 
// dataP:   the data be sent
// len:     the buf length
// maxNumberOfRetries:  the max Retries timers
// ackTimeoutMs:    the time out of Receiving
//***********************************************************************************
bool RadioCopyPacketToBuf(uint8_t *dataP, uint8_t len, uint8_t maxNumberOfRetries, uint32_t ackTimeoutMs, uint8_t baseAddr)
{
    if(Semaphore_pend(radioAccessSemHandle, BIOS_WAIT_FOREVER) == false)
        return false;
    /* Set destination address in EasyLink API */
    memcpy(currentRadioOperation.easyLinkTxPacket.dstAddr, dstRadioAddr, dstAddrLen);

    currentRadioOperation.easyLinkTxPacket.len      += len;
    memcpy(currentRadioOperation.easyLinkTxPacket.payload+baseAddr, dataP, len);

    /* Copy ADC packet to payload
     * Note that the EasyLink API will implcitily both add the length byte and the destination address byte. */

    /* Setup retries */
    currentRadioOperation.maxNumberOfRetries = maxNumberOfRetries;
    currentRadioOperation.ackTimeoutMs = ackTimeoutMs;
    currentRadioOperation.retriesDone = 0;

    Semaphore_post(radioAccessSemHandle);
    return true;
}


//***********************************************************************************
// brief:   set the radio event to send data by radio
// 
// parameter:   none 
//***********************************************************************************
void RadioSend(void)
{
    Event_post(radioOperationEventHandle, RADIO_EVT_TX);
}

//***********************************************************************************
// brief:   set the radio at rx mode
// 
// parameter:   none 
//***********************************************************************************
void RadioSetRxMode(void)
{
    Event_post(radioOperationEventHandle, RADIO_EVT_SET_RX_MODE);
}

//***********************************************************************************
// brief:   set the radio at tx mode
// 
// parameter:   none 
//***********************************************************************************
void RadioSetTxMode(void)
{
    Event_post(radioOperationEventHandle, RADIO_EVT_SET_TX_MODE);
}


//***********************************************************************************
// brief:   disable the radio 
// 
// parameter:   none 
//***********************************************************************************
void RadioDisable(void)
{
    Event_post(radioOperationEventHandle, RADIO_EVT_DISABLE);
}




//***********************************************************************************
// brief:   copy the packet to Txbuf, but do not send immediately
// 
// parameter: 
// dataP:   the data be sent
// len:     the buf length
// maxNumberOfRetries:  the max Retries timers
// ackTimeoutMs:    the time out of Receiving
//***********************************************************************************
void RadioSendPacket(uint8_t *dataP, uint8_t len, uint8_t maxNumberOfRetries, uint32_t ackTimeoutMs)
{
    RadioCopyPacketToBuf(dataP, len, maxNumberOfRetries, ackTimeoutMs, 0);
    Event_post(radioOperationEventHandle, RADIO_EVT_TX);
}







//***********************************************************************************
// brief:   resend the pack, the currentRadioOperation.easyLinkTxPacket may be modify
//          by other process
// 
//***********************************************************************************
void RadioResendPacket(void)
{
    EasyLink_transmit(&currentRadioOperation.easyLinkTxPacket);
            
    if(radioMode == RADIOMODE_RECEIVEPORT)
        EasyLink_receiveAsync(RxDoneCallback, 0);

    currentRadioOperation.retriesDone++;
}




//***********************************************************************************
// brief:   easyLink callback 
// 
//***********************************************************************************
static void RxDoneCallback(EasyLink_RxPacket * rxPacket, EasyLink_Status status)
{
    switch(status)
    {
        case EasyLink_Status_Success:
        memcpy(&radioRxPacket, rxPacket, sizeof(EasyLink_RxPacket));
        Event_post(radioOperationEventHandle, RADIO_EVT_RX);
        break;


        case EasyLink_Status_Config_Error:
        case EasyLink_Status_Param_Error:
        case EasyLink_Status_Mem_Error:
        case EasyLink_Status_Cmd_Error:
        case EasyLink_Status_Rx_Buffer_Error:
        case EasyLink_Status_Busy_Error:
        Event_post(radioOperationEventHandle, RADIO_EVT_FAIL);
        break;


        case EasyLink_Status_Tx_Error:
        // could not abort the easylink tx in the cb, maybe occur error
        // SystemEventSet(SYSTEMAPP_EVT_RADIO_ABORT);
        break;


        case EasyLink_Status_Rx_Error:
        Event_post(radioOperationEventHandle, RADIO_EVT_RX_FAIL);
        break;


        case EasyLink_Status_Rx_Timeout:
        Event_post(radioOperationEventHandle, RADIO_EVT_TOUT);
        break;

        case EasyLink_Status_Aborted:
        break;
    }
}


//***********************************************************************************
// brief:   get the first receive fliter addr
// 
//***********************************************************************************
uint32_t GetRadioSrcAddr(void)
{
    return *((uint32_t*)srcRadioAddr);
}


//***********************************************************************************
// brief:   get the second receive fliter addr
// 
//***********************************************************************************
uint32_t GetRadioSubSrcAddr(void)
{
    return *((uint32_t*)(srcRadioAddr + 4));
}


//***********************************************************************************
// brief:   get the dst addr
// 
//***********************************************************************************
uint32_t GetRadioDstAddr(void)
{
    return *((uint32_t*)dstRadioAddr);
}


static uint8_t radioSubAddrFlag = 0;

//***********************************************************************************
// brief:   set the first receive addr fliter
// 
//***********************************************************************************
void SetRadioSrcAddr(uint32_t addr)
{
    srcRadioAddr[0] = LOBYTE(LOWORD(addr));
    srcRadioAddr[1] = HIBYTE(LOWORD(addr));
    srcRadioAddr[2] = LOBYTE(HIWORD(addr));
    srcRadioAddr[3] = HIBYTE(HIWORD(addr));

    if(radioSubAddrFlag)
        EasyLink_enableRxAddrFilter(srcRadioAddr, srcAddrLen, 2);
    else
        EasyLink_enableRxAddrFilter(srcRadioAddr, srcAddrLen, 1);
}




//***********************************************************************************
// brief:   set the second receive addr fliter
// 
//***********************************************************************************
void SetRadioSubSrcAddr(uint32_t addr)
{
    radioSubAddrFlag = 1;
    
    srcRadioAddr[4] = LOBYTE(LOWORD(addr));
    srcRadioAddr[5] = HIBYTE(LOWORD(addr));
    srcRadioAddr[6] = LOBYTE(HIWORD(addr));
    srcRadioAddr[7] = HIBYTE(HIWORD(addr));

    EasyLink_enableRxAddrFilter(srcRadioAddr, srcAddrLen, 2);
}




//***********************************************************************************
// brief:   set the dst radio addr
// 
//***********************************************************************************
void SetRadioDstAddr(uint32_t addr)
{
    dstRadioAddr[0] = LOBYTE(LOWORD(addr));
    dstRadioAddr[1] = HIBYTE(LOWORD(addr));
    dstRadioAddr[2] = LOBYTE(HIWORD(addr));
    dstRadioAddr[3] = HIBYTE(HIWORD(addr));
}


//***********************************************************************************
// brief:   clear the radio send buf
// 
//***********************************************************************************
void ClearRadioSendBuf(void)
{
    Semaphore_pend(radioAccessSemHandle, BIOS_WAIT_FOREVER);
    currentRadioOperation.easyLinkTxPacket.len = 0;
    Semaphore_post(radioAccessSemHandle);
}


//***********************************************************************************
// brief:   enable the radio test
// 
//***********************************************************************************
void RadioTestEnable(void)
{
    radioTestFlag = true;
    Event_post(radioOperationEventHandle, RADIO_EVT_TEST);
}

//***********************************************************************************
// brief:   disable the radio test
// 
//***********************************************************************************
void RadioTestDisable(void)
{
    radioTestFlag = false;
}


//***********************************************************************************
// brief:   return the radio status
// 
//***********************************************************************************
uint8_t RadioStatueRead(void)
{
    return radioStatus;
}

