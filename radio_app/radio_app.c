/*
* @Author: zxt
* @Date:   2017-12-21 17:36:18
* @Last Modified by:   zxt
* @Last Modified time: 2018-11-07 15:49:05
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
#define PASSRADIO_TASK_STACK_SIZE 1024
#define PASSRADIO_TASK_PRIORITY   2



#define PASSRADIO_MAX_RETRIES           2

#define RADIO_ADDR_LEN                  4
#define RADIO_ADDR_LEN_MAX              12

#if (defined(BOARD_S6_6) || defined(BOARD_B2S))
#define RADIO_RSSI_FLITER               -40
#else
#define RADIO_RSSI_FLITER               -85
#endif



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

/* Clock for the fast report timeout */

Clock_Struct radioSendTimeoutClock;     /* not static so you can see in ROV */

Clock_Handle radioSendTimeoutClockHandle;



/***** Prototypes *****/

void RadioAppTaskFxn(void);

static void RxDoneCallback(EasyLink_RxPacket * rxPacket, EasyLink_Status status);

void RadioCoreErrorCb(RF_Handle h, RF_CmdHandle ch, RF_EventMask e);

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
// brief:   radio send data process
// 
// parameter:   none 
//***********************************************************************************
void RadioSendData(void)
{
    EasyLink_Status status;

    if(radioError)
        SystemResetAndSaveRtc();

radio_reSend:
    status = EasyLink_transmit(&currentRadioOperation.easyLinkTxPacket);
    switch(status)
    {
        case EasyLink_Status_Tx_Error:
        goto radio_reSend;

        case EasyLink_Status_Busy_Error:
        RadioAbort();
        goto radio_reSend;

        case EasyLink_Status_Param_Error:
        case EasyLink_Status_Config_Error:
        // should be reset
        g_rSysConfigInfo.sysState.wtd_restarts |= RADIO_FREQ_SEND_ERROR;
        Flash_log("TX R\n");
        SystemResetAndSaveRtc();
        break;
    }
}


//***********************************************************************************
// brief:   radio start receive data process
// 
// parameter:   none 
//***********************************************************************************
void RadioReceiveData(void)
{
    EasyLink_Status status;

    if(radioError)
        SystemResetAndSaveRtc();

    radioStatus = RADIOSTATUS_RECEIVING;
radio_reReceive:
    status = EasyLink_receiveAsync(RxDoneCallback, 0);
    switch(status)
    {
        case EasyLink_Status_Rx_Error:
        goto radio_reReceive;

        case EasyLink_Status_Busy_Error:
        RadioAbort();
        goto radio_reReceive;

        case EasyLink_Status_Config_Error:
        // should be reset
        g_rSysConfigInfo.sysState.wtd_restarts |= RADIO_FREQ_RECIEVE_ERROR;
        Flash_log("RX R\n");
        SystemResetAndSaveRtc();
        break;
    }
}

//***********************************************************************************
// brief:   radio Set the freqency
// 
// parameter:   none 
//***********************************************************************************
void RadioSetFrequency(uint32_t ui32Frequency)
{
    EasyLink_Status status;
    if(radioError)
        SystemResetAndSaveRtc();
radio_reSetFreq:
    status = EasyLink_setFrequency(ui32Frequency);
    switch(status)
    {
        case EasyLink_Status_Cmd_Error:
        goto radio_reSetFreq;

        case EasyLink_Status_Busy_Error:
        RadioAbort();
        goto radio_reSetFreq;

        case EasyLink_Status_Config_Error:
        // should be reset
        g_rSysConfigInfo.sysState.wtd_restarts |= RADIO_FREQ_SWITCH_ERROR;

        Flash_log("SF R\n");
        SystemResetAndSaveRtc();
        break;
    }
}

//***********************************************************************************
// brief:   Abort a previously call Async Tx/Rx.
// 
// parameter:   none 
//***********************************************************************************
void RadioAbort(void)
{
    EasyLink_Status status;
    if(radioError)
        SystemResetAndSaveRtc();
radio_reAbort:
    status = EasyLink_abort();
    switch(status)
    {
        case EasyLink_Status_Cmd_Error:
        goto radio_reAbort;

        case EasyLink_Status_Config_Error:
        // should be reset
        g_rSysConfigInfo.sysState.wtd_restarts |= RADIO_ABORT_ERROR;

        Flash_log("RF ST R\n");
        SystemResetAndSaveRtc();
        break;
    }
}


//***********************************************************************************
// brief:   radio Check rssi
// 
// parameter:   rssi the radio 
//***********************************************************************************
int8_t RadioCheckRssi(void)
{
    int8_t rssi, rssi2;
    rssi  = -128;
    rssi2 = -128;
    RadioAbort();
    EasyLink_setCtrl(EasyLink_Ctrl_AsyncRx_TimeOut, 0);
    RadioReceiveData();
    Task_sleep(CONCENTER_RADIO_DELAY_TIME_MS * CLOCK_UNIT_MS);
    EasyLink_getRssi(&rssi);
    Task_sleep(CONCENTER_RADIO_DELAY_TIME_MS * CLOCK_UNIT_MS);
    EasyLink_getRssi(&rssi2);
    RadioAbort();

    if(rssi > rssi2)
        return rssi;
    else
        return rssi2;
}


//***********************************************************************************
// brief:   radio send data timeout
// 
// parameter:   none 
//***********************************************************************************
void RadioSendTimeroutCb(UArg arg0)
{
    Sys_event_post(SYSTEMAPP_EVT_RADIO_ABORT);
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

    Clock_Params clkParams;
    Clock_Params_init(&clkParams);
    clkParams.period = 0;
    clkParams.startFlag = FALSE;
    Clock_construct(&radioSendTimeoutClock, RadioSendTimeroutCb, 1000 * CLOCK_UNIT_MS, &clkParams);
    radioSendTimeoutClockHandle = Clock_handle(&radioSendTimeoutClock);
    Clock_setTimeout(radioSendTimeoutClockHandle, 1000 * CLOCK_UNIT_MS);
    

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

    if(modeSet == RADIOMODE_UPGRADE)
    {
        radioMode   = RADIOMODE_UPGRADE;
    }
}

//***********************************************************************************
// brief:   get the radio mode
// 
// parameter:   
//***********************************************************************************
RadioOperationMode RadioModeGet(void)
{
    return (RadioOperationMode)radioMode;
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

    RadioUpgrade_Init();
    // init the easylink
    EasyLink_Params easyLink_params;
    EasyLink_Params_init(&easyLink_params);

#if defined(SUPPORT_BOARD_OLD_S1) || defined(SUPPORT_BOARD_OLD_S2S_1)
    easyLink_params.ui32ModType = RADIO_EASYLINK_MODULATION_S1_OLD;
#else
    
    #ifdef   RADIO_1310_50K_GPSK
        easyLink_params.ui32ModType = RADIO_EASYLINK_MODULATION_50K;
    #else
        easyLink_params.ui32ModType = RADIO_EASYLINK_MODULATION;
    #endif
#endif


    if(EasyLink_init(&easyLink_params) != EasyLink_Status_Success){ 
        System_abort("EasyLink_init failed");
    }
#ifndef BOARD_CONFIG_DECEIVE
#ifndef BOARD_S3
    Radio_setRxModeRfFrequency();
#endif //BOARD_BOARD_S1_2
#endif //BOARD_CONFIG_DECEIVE
    radioStatus = RADIOSTATUS_IDLE;

#if (defined(BOARD_S6_6) || defined(BOARD_B2S))

#ifdef  BOARD_CONFIG_DECEIVE
    g_rSysConfigInfo.rfStatus |= STATUS_1310_MASTER;
#endif  // BOARD_CONFIG_DECEIVE

#ifdef S_G//网关
    g_rSysConfigInfo.rfStatus |= STATUS_1310_MASTER;
#endif // S_G//网关

#ifdef S_C //节点
#if !defined(SUPPORT_BOARD_OLD_S2S_1)
            g_rSysConfigInfo.rfStatus &= ~STATUS_1310_MASTER;
#endif //!defined(SUPPORT_BOARD_OLD_S2S_1)
#endif // S_C //节点

    if(g_rSysConfigInfo.rfStatus & STATUS_1310_MASTER)
    {
        radioMode = RADIOMODE_RECEIVEPORT;
    }
    else
    {
        radioMode = RADIOMODE_SENDPORT;
    }

    
#endif  // BOARD_S6_6

#ifdef  BOARD_S3

    radioMode = RADIOMODE_SENDPORT;

#endif // BOARD_S3

    // set the default para
    RadioDefaultParaInit();

    if(radioMode == RADIOMODE_SENDPORT)
    {
        NodeAppInit();
#if defined(SUPPORT_BOARD_OLD_S1) || defined(SUPPORT_BOARD_OLD_S2S_1)
        OldS1NodeApp_init();
#endif

#if (defined(BOARD_S6_6) || defined(BOARD_B2S))
        NodeWakeup();
#endif // BOARD_S6_6
    }
    else
    {
        ConcenterAppInit();
    }

#ifdef SUPPORT_FREQ_FIND
    AutoFreqInit();
#endif  //SUPPORT_FREQ_FIND

#ifdef FACTOR_RADIO_TEST
    while(1)
    {
        EasyLink_setRfPower(14);
        RadioAbort();
        RadioSetFrequency(RADIO_BASE_FREQ);
        EasyLink_setCtrl(EasyLink_Ctrl_Test_Tone, 0);
    }
#endif

#ifdef BOARD_B2S
#ifdef S_C //节点
    Task_sleep(5000 *CLOCK_UNIT_MS);
#endif // S_C //节点
#endif // BOARD_B2S

    for(;;)
    {
        uint32_t events = Event_pend(radioOperationEventHandle, 0, RADIO_EVT_ALL, BIOS_WAIT_FOREVER);
#if (defined(S_G) && defined(SUPPORT_FREQ_FIND))

        if(events & RADIO_EVT_CHANNEL_CHECK)
        {
            AutoFreqConcenterSwitchFreqProcess();
        }
#endif // (defined(S_G) || defined(SUPPORT_FREQ_FIND))

        if(events & RADIO_EVT_SENSOR_PACK)
        {
            if (deviceMode != DEVICES_CONFIG_MODE)
            {
                NodeUploadProcess();
                NodeBroadcasting();
            }
        }

#if (defined SUPPORT_BOARD_OLD_S1) || (defined SUPPORT_BOARD_OLD_S2S_1)
        if(events & RADIO_EVT_EVT_OLD_S1_UPLOAD_NODE) {
            OldS1NodeAPP_Mode2NodeUploadProcess();
        }
#endif

        if (events & RADIO_EVT_TEST)
        {
            RadioAbort();
            EasyLink_setRfPower(14);
            RadioAbort();
            RadioSetFrequency(RADIO_BASE_FREQ);
            EasyLink_setCtrl(EasyLink_Ctrl_Test_Tone, 0);
            while(radioTestFlag)
            {
                Task_sleep(100 * CLOCK_UNIT_MS);
            }
            continue;
        }


        if (events & RADIO_EVT_RX)
        {
            if(radioStatus == RADIOSTATUS_RECEIVING)
            {

                radioStatus = RADIOSTATUS_IDLE;

                if((radioMode == RADIOMODE_RECEIVEPORT) || (radioMode == RADIOMODE_UPGRADE))
                {
               
#ifdef  BOARD_S3
                        NodeProtocalDispath(&radioRxPacket);
                        if (radioMode == RADIOMODE_RECEIVEPORT) {
                            Led_ctrl(LED_B, 1, 250 * CLOCK_UNIT_MS, 2);
                        }
#else
                        ConcenterProtocalDispath(&radioRxPacket);

    #ifdef  BOARD_CONFIG_DECEIVE
                        Led_ctrl(LED_B, 1, 250 * CLOCK_UNIT_MS, 1);
    #endif  //BOARD_CONFIG_DECEIVE

#endif  //BOARD_S3
                        if (currentRadioOperation.easyLinkTxPacket.len == 0) {
                            RadioReceiveData();
                        }
                 }

                if(radioMode == RADIOMODE_SENDPORT)
                {
#if defined(SUPPORT_BOARD_OLD_S1) || defined(SUPPORT_BOARD_OLD_S2S_1)
                    if (deviceMode == DEVICES_ON_MODE && g_oldS1OperatingMode == S1_OPERATING_MODE2) {
                        OldS1NodeApp_protocolProcessing(radioRxPacket.payload, radioRxPacket.len);
                    } else if (deviceMode == DEVICES_CONFIG_MODE) {
                        Led_ctrl(LED_B, 1, 250 * CLOCK_UNIT_MS, 2);
                        NodeProtocalDispath(&radioRxPacket);
                    }
#else
                    NodeProtocalDispath(&radioRxPacket);
                    if(deviceMode == DEVICES_CONFIG_MODE)
                        Led_ctrl(LED_B, 1, 250 * CLOCK_UNIT_MS, 2);
#endif
                }

            }
        }


        if (events & RADIO_EVT_TX)
        {
#if (defined(S_G) && defined(SUPPORT_FREQ_FIND))
            if(AutoFreqStateRead() == false)
            {
                Radio_setTxModeRfFrequency();
                rssi = RadioCheckRssi();
                if(rssi > RADIO_RSSI_FLITER)
                {
                    AutoFreqCarrierBusy(rssi);
                    continue;
                }
                else
                {
                    AutoFreqCarrierRssiSet(rssi);
                }
            }

#endif // (defined(S_G) || defined(SUPPORT_FREQ_FIND))


#ifdef SUPPORT_RSSI_CHECK
            Radio_setTxModeRfFrequency();
            if (RADIOMODE_UPGRADE != RadioModeGet() && (deviceMode != DEVICES_CONFIG_MODE) && (NodeContinueFlagRead() == 0)) {
                //i = 2;
                rssi = RadioCheckRssi();
            }
            else
            {
                rssi  = RADIO_RSSI_FLITER - 1;
            }
            NodeContinueFlagClear();
#else
            rssi  = RADIO_RSSI_FLITER - 1;
#endif  // SUPPORT_RSSI_CHECK

#ifdef      S_C
#if !defined(SUPPORT_BOARD_OLD_S1) && !defined(SUPPORT_BOARD_OLD_S2S_1)
            if((GetStrategyRegisterStatus() == false) && (deviceMode != DEVICES_CONFIG_MODE))
            {
                NodeStrategyBuffClear();
                NodeStartBroadcast();
                NodeRadioSendSynReq();
            }
#endif //!defined(SUPPORT_BOARD_OLD_S1) && !defined(SUPPORT_BOARD_OLD_S2S_1)
#endif //S_C

#ifdef SUPPORT_RSSI_CHECK            
            if(rssi > RADIO_RSSI_FLITER)
            {
                StrategyCheckRssiBusyProcess();
            }
            else 
#endif // SUPPORT_RSSI_CHECK
            if((currentRadioOperation.easyLinkTxPacket.len) <= EASYLINK_MAX_DATA_LENGTH && (currentRadioOperation.easyLinkTxPacket.len > 0))// && (rssi <= RADIO_RSSI_FLITER))
            {
                if (deviceMode != DEVICES_OFF_MODE)
                {
                    Led_ctrl(LED_B, 1, 200 * CLOCK_UNIT_MS, 1);
                }
                Semaphore_pend(radioAccessSemHandle, BIOS_WAIT_FOREVER);
#if defined(SUPPORT_BOARD_OLD_S1) || defined(SUPPORT_BOARD_OLD_S2S_1)
                if (deviceMode == DEVICES_ON_MODE && g_oldS1OperatingMode == S1_OPERATING_MODE2) {
                    RadioAbort();
                    OldS1NodeApp_setDataTxRfFreque();
                    RadioAbort();
                }
#else
                RadioAbort();
                Radio_setTxModeRfFrequency();
                RadioAbort();
#endif //SUPPORT_BOARD_OLD_S1


                Clock_start(radioSendTimeoutClockHandle);
                radioStatus = RADIOSTATUS_TRANSMITTING;
                RadioSendData();
                Clock_stop(radioSendTimeoutClockHandle);

#if defined(SUPPORT_BOARD_OLD_S1) || defined(SUPPORT_BOARD_OLD_S2S_1)
                    if (deviceMode == DEVICES_ON_MODE && g_oldS1OperatingMode == S1_OPERATING_MODE2) {
                        OldS1NodeApp_setDataRxRfFreque();
                    }
#else
                    Radio_setRxModeRfFrequency();
#endif //SUPPORT_BOARD_OLD_S1
                if(radioMode == RADIOMODE_SENDPORT)
                {
                    radioStatus = RADIOSTATUS_RECEIVING;
                    if(GetStrategyRegisterStatus() == false)
                        EasyLink_setCtrl(EasyLink_Ctrl_AsyncRx_TimeOut, EasyLink_ms_To_RadioTime(1500));
                    else
                        EasyLink_setCtrl(EasyLink_Ctrl_AsyncRx_TimeOut, EasyLink_ms_To_RadioTime(currentRadioOperation.ackTimeoutMs));
                    RadioReceiveData();
                }

                if(radioMode == RADIOMODE_RECEIVEPORT || radioMode == RADIOMODE_UPGRADE)
                {
#if defined(SUPPORT_FREQ_FIND) && defined(S_G)
                    if(AutoFreqStateRead() == false)
                        EasyLink_setCtrl(EasyLink_Ctrl_AsyncRx_TimeOut, EasyLink_ms_To_RadioTime(500));
                    else
#endif // SUPPORT_FREQ_FIND
                        EasyLink_setCtrl(EasyLink_Ctrl_AsyncRx_TimeOut, 0);

                    RadioReceiveData();
                }
#ifdef  SUPPORT_RADIO_UPGRADE
                if (radioMode == RADIOMODE_UPGRADE)
                {
                    RadioSwitchingUpgradeRate();
                }
#endif
                Semaphore_post(radioAccessSemHandle);
            }
            else
            {
                NodeStrategyBuffClear();
            }
        }


        



        if (events & RADIO_EVT_TOUT)
        {
#ifdef S_C//閲囬泦鍣�
            if(radioMode == RADIOMODE_SENDPORT)
            {
#if !defined(SUPPORT_BOARD_OLD_S1) && !defined(SUPPORT_BOARD_OLD_S2S_1)
                NodeStrategyReceiveTimeoutProcess();
#endif

            }
#endif  // S_C//閲囬泦鍣�

#if defined(SUPPORT_FREQ_FIND) && defined(S_G)
            if(AutoFreqStateRead() == false)
            {
                AutoFreqRecTimeout();
            }
#endif // SUPPORT_FREQ_FIND
        }

        if(events & RADIO_EVT_SET_RX_MODE)
        {
            radioMode = RADIOMODE_RECEIVEPORT;
            if(radioStatus != RADIOSTATUS_IDLE)
            {
                RadioAbort();
            }
            EasyLink_setCtrl(EasyLink_Ctrl_AsyncRx_TimeOut, 0);
            Radio_setRxModeRfFrequency();
            RadioReceiveData();
        }


        if(events & RADIO_EVT_SET_TX_MODE)
        {
            radioMode = RADIOMODE_SENDPORT;
            if(radioStatus == RADIOSTATUS_RECEIVING)
            {
                radioStatus = RADIOSTATUS_ABSORT;
                RadioAbort();
                radioStatus = RADIOSTATUS_IDLE;
            }
        }

        if(events & RADIO_EVT_DISABLE)
        {
            if(radioStatus == RADIOSTATUS_RECEIVING)
            {
                RadioAbort();
                radioStatus = RADIOSTATUS_IDLE;   
            }
        }

        if (events & RADIO_EVT_UPGRADE_SEND)
        {
//            System_printf("US\r\n");
            RadioUpgrade_FileDataSend();
        }

        if (events & RADIO_EVT_UPGRADE_RX_TIMEOUT) {
            RadioSwitchingUserRate();
        }

        if (events & (RADIO_EVT_FAIL | RADIO_EVT_RX_FAIL))
        {
            radioStatus = RADIOSTATUS_IDLE;
            if(radioMode == RADIOMODE_RECEIVEPORT || radioMode == RADIOMODE_UPGRADE)
            {
                RadioAbort();
                EasyLink_setCtrl(EasyLink_Ctrl_AsyncRx_TimeOut, 0);
                RadioReceiveData();
            }

        }

        if(events & RADIO_EVT_RADIO_REPAIL) 
        {
            if(GetStrategyRegisterStatus() == false)
            {
                NodeStrategyTimeoutProcess();
                RadioSend();
            }
        }

        if(events & RADIO_EVT_SEND_CONFIG) 
        {
#ifdef   BOARD_S3
            //RadioAbort();
            Radio_setConfigModeRfFrequency();
            NodeStrategyBuffClear();
            NodeRadioSendConfig();
#endif

#ifdef   BOARD_S6_6
            ConcenterRadioSendParaSet(0xabababab, 0xbabababa);
#endif

#ifdef   BOARD_B2S
            ConcenterRadioSendParaSet(0xabababab, 0xbabababa);
#endif
            // RadioSend();
        }

        if(events & RADIO_EVT_SEND_SYC) 
        {
            if (deviceMode != DEVICES_CONFIG_MODE)
            {
                NodeRadioSendSynReq();
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
//
// radio event post.
//
//***********************************************************************************
void RadioEventPost(UInt event)
{
    Event_post(radioOperationEventHandle, event);
}



//***********************************************************************************
// brief:   set the radio event to send data by radio
// 
// parameter:   none 
//***********************************************************************************
void RadioSensorDataPack(void)
{
    Event_post(radioOperationEventHandle, RADIO_EVT_SENSOR_PACK);
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
// brief:   send the Bin File in the Node
//
// parameter:   none
//***********************************************************************************
void RadioUpgradeSendFile(void)
{
    Event_post(radioOperationEventHandle, RADIO_EVT_UPGRADE_SEND);
}

void RadioUpgradeRxFileDataTimout(void)
{
    Event_post(radioOperationEventHandle,RADIO_EVT_UPGRADE_RX_TIMEOUT);
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
        // Sys_event_post(SYSTEMAPP_EVT_RADIO_ABORT);
        break;


        case EasyLink_Status_Rx_Error:
        default:
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


static uint8_t radioSubAddrNum = 0;

//***********************************************************************************
// brief:   set the first receive addr fliter
// 
//***********************************************************************************
void SetRadioSrcAddr(uint32_t addr)
{
    srcRadioAddr[0] = LOBYTE_ZKS(LOWORD_ZKS(addr));
    srcRadioAddr[1] = HIBYTE_ZKS(LOWORD_ZKS(addr));
    srcRadioAddr[2] = LOBYTE_ZKS(HIWORD_ZKS(addr));
    srcRadioAddr[3] = HIBYTE_ZKS(HIWORD_ZKS(addr));


    EasyLink_enableRxAddrFilter(srcRadioAddr, srcAddrLen, radioSubAddrNum+1);
}




//***********************************************************************************
// brief:   set the second receive addr fliter
// 
//***********************************************************************************
void SetRadioSubSrcAddr(uint32_t addr)
{
    if(radioSubAddrNum < 1)
        radioSubAddrNum = 1;
    
    srcRadioAddr[4] = LOBYTE_ZKS(LOWORD_ZKS(addr));
    srcRadioAddr[5] = HIBYTE_ZKS(LOWORD_ZKS(addr));
    srcRadioAddr[6] = LOBYTE_ZKS(HIWORD_ZKS(addr));
    srcRadioAddr[7] = HIBYTE_ZKS(HIWORD_ZKS(addr));

    EasyLink_enableRxAddrFilter(srcRadioAddr, srcAddrLen, radioSubAddrNum+1);
}

//***********************************************************************************
// brief:   set the concenter brocast addr, this addr all the deceive could receive radio data
// 
//***********************************************************************************
void SetRadioBrocastSrcAddr(uint32_t addr)
{
    if(radioSubAddrNum < 2)
        radioSubAddrNum = 2;
    
    srcRadioAddr[8]  = LOBYTE_ZKS(LOWORD_ZKS(addr));
    srcRadioAddr[9]  = HIBYTE_ZKS(LOWORD_ZKS(addr));
    srcRadioAddr[10] = LOBYTE_ZKS(HIWORD_ZKS(addr));
    srcRadioAddr[11] = HIBYTE_ZKS(HIWORD_ZKS(addr));

    EasyLink_enableRxAddrFilter(srcRadioAddr, srcAddrLen, radioSubAddrNum+1);
}


//***********************************************************************************
// brief:   set the dst radio addr
// 
//***********************************************************************************
void SetRadioDstAddr(uint32_t addr)
{
    dstRadioAddr[0] = LOBYTE_ZKS(LOWORD_ZKS(addr));
    dstRadioAddr[1] = HIBYTE_ZKS(LOWORD_ZKS(addr));
    dstRadioAddr[2] = LOBYTE_ZKS(HIWORD_ZKS(addr));
    dstRadioAddr[3] = HIBYTE_ZKS(HIWORD_ZKS(addr));
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

//***********************************************************************************
// brief:
//
//***********************************************************************************
void Radio_setConfigModeRfFrequency(void)
{
    uint32_t freq, dstFreq, diffFreq;

    freq = EasyLink_getFrequency();
    dstFreq = RADIO_BASE_FREQ;

    if (freq < dstFreq) {
        diffFreq = dstFreq - freq;
    } else {
        diffFreq = freq - dstFreq;
    }

    if (diffFreq < 20000) { ///< 20Khz
        return;
    }

    RadioSetFrequency(dstFreq);
}

//***********************************************************************************
// brief:
//
//***********************************************************************************
void Radio_setRxModeRfFrequency(void)
{
    uint32_t freq, dstFreq, diffFreq;

    freq = EasyLink_getFrequency();

#ifndef BOARD_CONFIG_DECEIVE

    if ((radioMode != RADIOMODE_UPGRADE) && (deviceMode != DEVICES_CONFIG_MODE)) {
#ifdef  S_C//閲囬泦鍣�
        dstFreq = RADIO_BASE_FREQ + RADIO_DIFF_UNIT_FREQ + ((g_rSysConfigInfo.rfBW>>4)*RADIO_BASE_UNIT_FREQ);
#endif  // S_C//閲囬泦鍣�

#ifdef  S_G//缃戝叧
    #ifdef SUPPORT_FREQ_FIND
        if(AutoFreqStateRead() == false)
            dstFreq = RADIO_BASE_FREQ + RADIO_DIFF_UNIT_FREQ + ((g_rSysConfigInfo.rfBW>>4)*RADIO_BASE_UNIT_FREQ);
        else
    #endif  // SUPPORT_FREQ_FIND
            dstFreq = RADIO_BASE_FREQ + ((g_rSysConfigInfo.rfBW>>4)*RADIO_BASE_UNIT_FREQ);
#endif  // S_G//缃戝叧

        if (freq < dstFreq) {
            diffFreq = dstFreq - freq;
        } else {
            diffFreq = freq - dstFreq;
        }

        if (diffFreq < 20000) { ///< 20Khz
            return;
        }

        RadioSetFrequency(dstFreq);
    }
#endif  // BOARD_CONFIG_DECEIVE
}

//***********************************************************************************
// brief:
//
//***********************************************************************************
void Radio_setTxModeRfFrequency(void)
{
    uint32_t freq, dstFreq, diffFreq;

    freq = EasyLink_getFrequency();

#ifndef BOARD_CONFIG_DECEIVE
    if ((radioMode != RADIOMODE_UPGRADE) && (deviceMode != DEVICES_CONFIG_MODE)) {
        
#ifdef  S_C//閲囬泦鍣�
        dstFreq = RADIO_BASE_FREQ + ((g_rSysConfigInfo.rfBW>>4)*RADIO_BASE_UNIT_FREQ);
#endif  // S_C//閲囬泦鍣�

#ifdef  S_G//缃戝叧
    #ifdef SUPPORT_FREQ_FIND
        if(AutoFreqStateRead() == false)
            dstFreq = RADIO_BASE_FREQ + ((g_rSysConfigInfo.rfBW>>4)*RADIO_BASE_UNIT_FREQ);
        else
    #endif // SUPPORT_FREQ_FIND
            dstFreq = RADIO_BASE_FREQ + RADIO_DIFF_UNIT_FREQ +  ((g_rSysConfigInfo.rfBW>>4)*RADIO_BASE_UNIT_FREQ);
#endif  // S_G//缃戝叧

        if (freq < dstFreq) {
            diffFreq = dstFreq - freq;
        } else {
            diffFreq = freq - dstFreq;
        }

        if (diffFreq < 20000) { ///< 20Khz
            return;
        }

        RadioSetFrequency(dstFreq);
    }
#endif  // BOARD_CONFIG_DECEIVE
}

extern EasyLink_PhyType GetEasyLinkParamsModType(void);
// Wireless rate is switched to upgrade rate
void RadioSwitchingUpgradeRate(void)
{
    EasyLink_Params easyLink_params;
    if (RADIO_EASYLINK_MODULATION_50K == GetEasyLinkParamsModType()) {
        return;
    }

    Task_sleep(50 * CLOCK_UNIT_MS);
    RadioAbort();
    EasyLink_Params_init(&easyLink_params);
    easyLink_params.ui32ModType = RADIO_EASYLINK_MODULATION_50K;
    if (EasyLink_init(&easyLink_params) != EasyLink_Status_Success){
        System_abort("EasyLink_init failed");
    }

    Task_sleep(500 * CLOCK_UNIT_MS);
    RadioDefaultParaInit();

    EasyLink_setRfPower(7);

    RadioAbort();
    /* Set the filter to the generated random address */
    if (EasyLink_enableRxAddrFilter(srcRadioAddr, srcAddrLen, 1) != EasyLink_Status_Success)
    {
        System_abort("EasyLink_enableRxAddrFilter failed");
    }
    RadioAbort();
    RadioReceiveData();
}

// Wireless rate is switched to user rate
void RadioSwitchingUserRate(void)
{
    EasyLink_Params easyLink_params;
    if (RADIO_EASYLINK_MODULATION == GetEasyLinkParamsModType()) {
        return;
    }

    RadioAbort();
    EasyLink_Params_init(&easyLink_params);
    easyLink_params.ui32ModType = RADIO_EASYLINK_MODULATION;
    if (EasyLink_init(&easyLink_params) != EasyLink_Status_Success){
        System_abort("EasyLink_init failed");
    }

    Task_sleep(500 * CLOCK_UNIT_MS);

    EasyLink_setRfPower(14);

    RadioDefaultParaInit();
    /* Set the filter to the generated random address */
    if (EasyLink_enableRxAddrFilter(srcRadioAddr, srcAddrLen, 1) != EasyLink_Status_Success)
    {
        System_abort("EasyLink_enableRxAddrFilter failed");
    }
    RadioAbort();
    RadioReceiveData();
}

// Wireless rate is switched to S1_OLD user rate
void RadioSwitchingS1OldUserRate(void)
{
    EasyLink_Params easyLink_params;
    uint32_t addrSize = 0;

    EasyLink_getCtrl(EasyLink_Ctrl_AddSize, &addrSize);
    if (RADIO_EASYLINK_MODULATION_S1_OLD == GetEasyLinkParamsModType() && addrSize == 0) {
        return;
    }

    Task_sleep(50 * CLOCK_UNIT_MS);
    RadioAbort();
    EasyLink_Params_init(&easyLink_params);
    easyLink_params.ui32ModType = RADIO_EASYLINK_MODULATION_S1_OLD;
    if (EasyLink_init(&easyLink_params) != EasyLink_Status_Success){
        System_abort("EasyLink_init failed");
    }

    Task_sleep(500 * CLOCK_UNIT_MS);
	EasyLink_setRfPower(14);

    EasyLink_setCtrl(EasyLink_Ctrl_AddSize, 0);
    /* Set the filter to the generated random address */
    if (EasyLink_enableRxAddrFilter(NULL, 1, 1) != EasyLink_Status_Success)
    {
        System_abort("EasyLink_enableRxAddrFilter failed");
    }
    RadioAbort();
    RadioReceiveData();
}


//***********************************************************************************
// brief: replace the radio core error while(1), save the data to flash
//***********************************************************************************
void RadioCoreErrorCb(RF_Handle h, RF_CmdHandle ch, RF_EventMask e)
{
    g_rSysConfigInfo.sysState.wtd_restarts |= RADIO_FREQ_CORE_ERROR;
    g_rSysConfigInfo.sysState.i2c_errors = e;
    Sys_event_post(SYSTEMAPP_EVT_STORE_SYS_CONFIG);
}
