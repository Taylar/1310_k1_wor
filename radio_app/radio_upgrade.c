#include "../general.h"

// extern function
extern uint16_t Usb_group_package(USB_TX_MSG_ID msgId, uint8_t *pPacket, uint16_t dataLen);

#ifdef SUPPORT_WATCHDOG
extern Watchdog_Handle watchdogHandle;
#endif


#define   RADIO_CODE_DATA_LENGTH   64     // copy from upgrade.c

static Semaphore_Handle RadioUpgadeSendDataSemHandle;
static Semaphore_Struct RadioUpgadeSendDataSemStruct;

static Clock_Struct radioUpgradeClkStruct;
static Clock_Handle radioUpgradeClkHandle;

static Clock_Struct radioUpgradeRxClkStruct;
static Clock_Handle radioUpgradeRxClkHandle;


// global static variable
static usb_upgrade_info_t sRadio_upgrade_info = {0};  // initialize zero
static usb_upgrade_info_t sTxRadio_upgrade_info = {0};  // initialize zero

static uint32_t sRadioUpgradeAddress;

static radio_upgrade_TxInfo_t  radio_upgrade_tx_info;

static uint8_t radio_upgrade_shake_handle_Cnt;
static uint32_t u32RadioUpgradeFileLen;

typedef void (*RadioUpgradeTimeoutCallback) (void);
RadioUpgradeTimeoutCallback clkRadioUpgradeTimeoutCallback;

/*---------------------- Function declaration -----------------*/
static void radioUpgradeTimeoutFxn(UArg arg0);
static void radioUpgradeRxTimeoutFx(UArg arg0);

static void radioUpgradeTxTimeoutFxn();
static void radioUpgradeShakeHanleTimeoutFxn();

static void RadioUpgrade_TxInfoInit(void);
static ErrorStatus RadioUpgrade_TxInfoInsert(uint32_t offset, uint8_t len);
static bool RadioUpgrade_isResend(void);
static ErrorStatus RadioUpgrade_TxInfoMatch(uint32_t offset, uint8_t len);

static UPGRADE_RESULT_E RaidoUpgrade_LoadCheck(void);

static void RadioUpagrade_JumpBoot(void);

static int RadioUpgrade_NodataParse(uint8_t *pData, uint16_t length);
static int RadioUpgrade_DataParse(uint8_t *pData, uint16_t length);

static void RadioUpgrade_DataTransmit(uint32_t offset, uint8_t len);
static void RadioUpgrade_DataTransmitAck(uint32_t offset, uint8_t len, ErrorStatus status);

static void RadioUpgrade_startFileSendTimer(void);
static void RadioUpgrade_stopFileSendTimer(void);

static void RadioUpgrade_SendRateReq(void);
static void RadioUpgrade_SendRateReqAck(uint8_t status);






void RadioUpgrade_Init(void)
{
    // Embed the upgrade information in the bin file
    radio_upgrade_tx_info.info[0].resendCount  = (uint8_t)PROJECT_INFO_NAME[0];
    radio_upgrade_tx_info.info[0].resendCount  = (uint8_t)PROJECT_INFO_VERSION;

    Semaphore_Params semParams;
    Semaphore_Params_init(&semParams);
    Semaphore_construct(&RadioUpgadeSendDataSemStruct, 1, &semParams);
    RadioUpgadeSendDataSemHandle = Semaphore_handle(&RadioUpgadeSendDataSemStruct);


    /* Construct a 1000ms periodic Clock Instance to is Tx */
    Clock_Params clkParams;
    Clock_Params_init(&clkParams);
    clkParams.period = RADIO_UPGRADE_SHAKE_HANDLE_TIMEROUT;
    clkParams.startFlag = FALSE;
    Clock_construct(&radioUpgradeClkStruct, (Clock_FuncPtr)radioUpgradeTimeoutFxn,
                    RADIO_UPGRADE_SHAKE_HANDLE_TIMEROUT, &clkParams);
    /* Obtain clock instance handle */
    radioUpgradeClkHandle = Clock_handle(&radioUpgradeClkStruct);

    /* Construct a 15000ms periodic Clock Instance to is Rx */
    Clock_Params_init(&clkParams);
    clkParams.period = RADIO_UPGRADE_RX_TIMEOUT;
    clkParams.startFlag = FALSE;
    Clock_construct(&radioUpgradeRxClkStruct, (Clock_FuncPtr)radioUpgradeRxTimeoutFx,
                    RADIO_UPGRADE_RX_TIMEOUT, &clkParams);
    /* Obtain clock instance handle */
    radioUpgradeRxClkHandle = Clock_handle(&radioUpgradeRxClkStruct);

    RadioUpgrade_TxInfoInit();
}


void RadioUpgrade_start(uint32_t upgradeFileLen, uint32_t address)
{
    RadioUpgrade_TxInfoInit();
    sTxRadio_upgrade_info.pack_num = 0;
    sTxRadio_upgrade_info.pack_offset = 0;
    sTxRadio_upgrade_info.pack_size = 0;
    sTxRadio_upgrade_info.fileLength = 0;
    sTxRadio_upgrade_info.endFlag    = false;
    radio_upgrade_shake_handle_Cnt = 0;

    u32RadioUpgradeFileLen = upgradeFileLen;

    sRadioUpgradeAddress = address;

    RadioModeSet(RADIOMODE_UPGRADE);

    Clock_stop(radioUpgradeClkHandle);
    clkRadioUpgradeTimeoutCallback = radioUpgradeShakeHanleTimeoutFxn;

    // Recount time
    Clock_setPeriod(radioUpgradeClkHandle, RADIO_UPGRADE_SHAKE_HANDLE_TIMEROUT);
    Clock_start(radioUpgradeClkHandle);
    RadioUpgrade_SendRateReq();
    Led_ctrl(LED_B, 1, 500 * CLOCK_UNIT_MS, 2);
}

void RadioUpgrade_stop(void)
{
    Clock_stop(radioUpgradeClkHandle);

    RadioSwitchingUserRate();

    if (RADIOMODE_RECEIVEPORT != RadioModeGet()) {
        Task_sleep(500 * CLOCK_UNIT_MS);
        Led_ctrl(LED_R, 1, 500 * CLOCK_UNIT_MS, 2);
        RadioModeSet(RADIOMODE_RECEIVEPORT);
    }
}

void RadioUpgrade_FileDataSend(void)
{
    uint8_t txLen = 0;

    Semaphore_pend(RadioUpgadeSendDataSemHandle, BIOS_WAIT_FOREVER);

    if (sTxRadio_upgrade_info.fileLength == 0 && sTxRadio_upgrade_info.pack_num == 0) {
        RadioUpgrade_TxInfoInit();
    }
    if (RadioUpgrade_isResend()) { // Retransmission data
        RadioUpgrade_TxInfoInsert(radio_upgrade_tx_info.info[0].offset, radio_upgrade_tx_info.info[0].frameLen);
        RadioUpgrade_DataTransmit(radio_upgrade_tx_info.info[0].offset, radio_upgrade_tx_info.info[0].frameLen);
        Semaphore_post(RadioUpgadeSendDataSemHandle);
        return;
    }

    if (sTxRadio_upgrade_info.endFlag) {
        RadioUpgrade_stop();
        Semaphore_post(RadioUpgadeSendDataSemHandle);
        return;
    }

    if ((u32RadioUpgradeFileLen - sTxRadio_upgrade_info.fileLength >= RADIO_CODE_DATA_LENGTH)) {
        txLen = RADIO_CODE_DATA_LENGTH;
    } else if ((u32RadioUpgradeFileLen - sTxRadio_upgrade_info.fileLength < RADIO_CODE_DATA_LENGTH) \
            && (u32RadioUpgradeFileLen - sTxRadio_upgrade_info.fileLength > 0)) {
        txLen = u32RadioUpgradeFileLen - sTxRadio_upgrade_info.fileLength;
    } else if (u32RadioUpgradeFileLen - sTxRadio_upgrade_info.fileLength == 0) {
        txLen = 0;
    }

   RadioUpgrade_TxInfoInsert(sTxRadio_upgrade_info.pack_offset, txLen);
   RadioUpgrade_DataTransmit(sTxRadio_upgrade_info.pack_offset, txLen);

   Semaphore_post(RadioUpgadeSendDataSemHandle);
}


extern radio_protocal_t   protocalTxBuf;
static void RadioUpgrade_DataTransmitAck(uint32_t offset, uint8_t len, ErrorStatus status)
{
    uint8_t i = 0;
    ClearRadioSendBuf();
    SetRadioDstAddr(CONFIG_DECEIVE_ID_DEFAULT);
    protocalTxBuf.dstAddr = GetRadioDstAddr();
    protocalTxBuf.srcAddr = GetRadioSrcAddr();
    protocalTxBuf.command = RADIO_PRO_CMD_UPGRADE_ACK;

    protocalTxBuf.load[i++] = len;

    if (ES_ERROR == status) {
        protocalTxBuf.load[i++] = 0xff;
        protocalTxBuf.load[i++] = 0xff;
        protocalTxBuf.load[i++] = 0xff;
        protocalTxBuf.load[i++] = 0xff;
    } else {
        protocalTxBuf.load[i++] = (uint8_t)((offset >> 24) & 0xff);
        protocalTxBuf.load[i++] = (uint8_t)((offset >> 16) & 0xff);
        protocalTxBuf.load[i++] = (uint8_t)((offset >> 8) & 0xff);
        protocalTxBuf.load[i++] = (uint8_t)((offset >> 0) & 0xff);
    }

    protocalTxBuf.len = 10 + i;
    RadioSendPacket((uint8_t*)&protocalTxBuf, protocalTxBuf.len, 0, 0);
}

//0x24 Instruction analysis
void RadioUpgrade_CmdDataParse(uint8_t *pData, uint16_t length)
{
    int ret;

    // Recount time
    Clock_stop(radioUpgradeRxClkHandle);
    Clock_setPeriod(radioUpgradeRxClkHandle, RADIO_UPGRADE_RX_TIMEOUT);


    RadioModeSet(RADIOMODE_UPGRADE);
    deviceMode = DEVICES_CONFIG_MODE;
    if (NULL == pData || 0 == length) {
        // do nothing
        Clock_start(radioUpgradeRxClkHandle);
        return;
    }

    uint16_t data_len = pData[0];

    // len(2)cmd(1)data(n) // len = n+1
    if (data_len <= 1) {// 空消息处理
#if defined(SUPPORT_BOARD_OLD_S1) || defined(SUPPORT_BOARD_OLD_S2S_1)
        g_rSysConfigInfo.rtc = Rtc_get_calendar();
        Flash_store_config();
#endif
        RadioUpgrade_NodataParse(pData, length);
        Clock_start(radioUpgradeRxClkHandle);
        return ;
    }

    //length = len(2)cmd(1)data(n)[pack_len(2)offset(4)pack_data(pack_len)]
    if (length < 5) {
        goto err_end;
    }
    // remove len(2)cmd(1), lengh = USB data(n)
    // 去掉消息包头 // 消息非空处理
    Clock_start(radioUpgradeRxClkHandle);
    ret = RadioUpgrade_DataParse(pData, length);
    if (0 != ret) {
        goto err_end;
    }
    return;

err_end:
    RadioUpgrade_DataTransmitAck(0xffffffff, 0xff, ES_ERROR);
    return;
}

//0x04 Instruction analysis
void RadioUpgrade_CmdACKDataParse(uint8_t *pData, uint16_t length)
{
//    uint8_t status;
    uint16_t pack_len = pData[0];
    uint32_t data_offset_addr = ((uint32_t)(pData[1]) << 24) | ((uint32_t)(pData[2]) << 16) | ((uint32_t)(pData[3]) << 8) | (uint32_t)pData[4];

    if (RADIOMODE_UPGRADE != RadioModeGet()) {
        return;
    }

    Led_ctrl(LED_R, 1, 30 * CLOCK_UNIT_MS, 1);
    if (pData == NULL || length == 0) {
        return;
    }

    RadioUpgrade_stopFileSendTimer();

    // Frame matching processing
    RadioUpgrade_TxInfoMatch(data_offset_addr, pack_len);

    if ((data_offset_addr == sTxRadio_upgrade_info.pack_offset) \
            &&  ( 0 == pack_len) \
            &&  sTxRadio_upgrade_info.endFlag) {
        RadioUpgrade_stop();
        Led_ctrl(LED_B, 1, 500 * CLOCK_UNIT_MS, 5);
        return;
    }

    // Recount time
    Clock_setPeriod(radioUpgradeClkHandle, RADIO_UPGRADE_TX_TIMEOUT);
    RadioUpgrade_startFileSendTimer();
    RadioUpgrade_FileDataSend();
}

//0x05 Instruction analysis
void RadioUpgrade_CmdRateSwitch(void)
{
    Led_ctrl(LED_B, 1, 500 * CLOCK_UNIT_MS, 4);
    RadioModeSet(RADIOMODE_UPGRADE);
    RadioUpgrade_SendRateReqAck(0);

    Clock_stop(radioUpgradeRxClkHandle);
    Clock_setPeriod(radioUpgradeRxClkHandle, RADIO_UPGRADE_RX_TIMEOUT);
    Clock_start(radioUpgradeRxClkHandle);
}


//0x25 Instruction analysis
void RadioUpgrade_CmdACKRateSwitch(uint8_t *pData, uint16_t length)
{
    if (RADIOMODE_UPGRADE != RadioModeGet()) {
        return;
    }

    if (pData[0] == 0) {
        Clock_stop(radioUpgradeClkHandle);
        clkRadioUpgradeTimeoutCallback = radioUpgradeTxTimeoutFxn;
        Clock_setPeriod(radioUpgradeClkHandle, RADIO_UPGRADE_TX_TIMEOUT);

        Led_ctrl(LED_B, 1, 500 * CLOCK_UNIT_MS, 4);
        RadioSwitchingUpgradeRate();

        Task_sleep(500 * CLOCK_UNIT_MS);
        RadioUpgrade_startFileSendTimer();
    }
}



// -----------------------Static Function------------------------------------------
static void radioUpgradeTimeoutFxn(UArg arg0)
{
    clkRadioUpgradeTimeoutCallback();
}

static void radioUpgradeRxTimeoutFx(UArg arg0)
{
    Clock_stop(radioUpgradeRxClkHandle);

    RadioModeSet(RADIOMODE_SENDPORT);
    deviceMode = DEVICES_ON_MODE;

    sRadio_upgrade_info.pack_num = 0;
    sRadio_upgrade_info.pack_offset = 0;
    sRadio_upgrade_info.pack_size = 0;
    sRadio_upgrade_info.fileLength = 0;

    RadioUpgradeRxFileDataTimout();
}

// Send timeout processing
static void radioUpgradeTxTimeoutFxn(void)
{
    RadioUpgradeSendFile();
    /*RadioUpgrade_FileDataSend();*/
}

static void radioUpgradeShakeHanleTimeoutFxn(void)
{
    if (radio_upgrade_shake_handle_Cnt > RADIO_UPGRADE_SHAKE_HANDLE_TIMES_MAX) {
        Led_ctrl(LED_R, 1, 0, 0);
        //System_printf("Upgrade handshake failed!!!\r\n");
        Clock_stop(radioUpgradeClkHandle);
        return;
    }

    RadioUpgrade_SendRateReq();
    radio_upgrade_shake_handle_Cnt++;
}

static void RadioUpgrade_TxInfoInit(void)
{
    uint16_t i = 0;

    radio_upgrade_tx_info.txCount = 0;
    for(; i < RADIO_UPGRADE_CACHE_LEN; i++) {
        radio_upgrade_tx_info.info[i].isNoReceive = false;
        radio_upgrade_tx_info.info[i].resendCount = 0;
        radio_upgrade_tx_info.info[i].frameLen = 0;
        radio_upgrade_tx_info.info[i].switchResend = 0;
    }
}

static ErrorStatus RadioUpgrade_TxInfoInsert(uint32_t offset, uint8_t len)
{
    uint16_t i = 0;
    ErrorStatus status = ES_ERROR;

    for (; i < RADIO_UPGRADE_CACHE_LEN; i++) {
        if (!radio_upgrade_tx_info.info[i].isNoReceive) {
            radio_upgrade_tx_info.info[i].offset = offset;
            radio_upgrade_tx_info.info[i].frameLen    = len;
            radio_upgrade_tx_info.info[i].isNoReceive = true;
            radio_upgrade_tx_info.info[i].resendCount = 0;
            radio_upgrade_tx_info.info[i].switchResend = 0;
            status = ES_SUCCESS;
        }

        // Whether to retransmit packets
        if (radio_upgrade_tx_info.info[i].offset == offset && radio_upgrade_tx_info.info[i].frameLen == len) {
            radio_upgrade_tx_info.info[i].resendCount++;
            status = ES_SUCCESS;
        }
    }


    if (status == ES_SUCCESS) {
        if (radio_upgrade_tx_info.txCount < RADIO_UPGRADE_CACHE_LEN) {
            radio_upgrade_tx_info.txCount++;
        } else {
            status = ES_ERROR;
        }
    }
    return status;
}

static bool RadioUpgrade_isResend(void)
{
    uint16_t i = 0;
    uint8_t buff[32] = {0};


    for (; i < RADIO_UPGRADE_CACHE_LEN; i++) {
        if (radio_upgrade_tx_info.info[i].isNoReceive && \
                radio_upgrade_tx_info.info[i].resendCount > RADIO_UPGRADE_RESEND_TIMES) {
//           System_printf("FrameLen: %d, offset:0x%02x%02x%02x%02x\r\n",radio_upgrade_tx_info.info[i].frameLen, (uint8_t)((radio_upgrade_tx_info.info[i].offset >> 24) & 0xff),\
//                         (uint8_t)((radio_upgrade_tx_info.info[i].offset >> 16) & 0xff), \
//                         (uint8_t)((radio_upgrade_tx_info.info[i].offset >> 8) & 0xff), \
//                         (uint8_t)((radio_upgrade_tx_info.info[i].offset >> 0) & 0xff));
//            System_printf("Node communication is abnormal,Switching waiting time!!!\r\n");
//            System_flush();
            RadioUpgrade_stopFileSendTimer();
            if (radio_upgrade_tx_info.info[i].switchResend > RADIO_UPGRAD_SWITCH_MAX) {
                RadioUpgrade_stop();
                //System_printf("Radio Upgrade Fail!!!\r\n");
                // Return usb error response, end here transmission
                bsl_ack_error(buff, 32);
                return false;
            }

            // Switching waiting time
            Clock_setPeriod(radioUpgradeClkHandle, RADIO_UPGRADE_TX_TIMEOUT * 2);
            RadioUpgrade_startFileSendTimer();

            radio_upgrade_tx_info.info[i].resendCount = 0;
            radio_upgrade_tx_info.info[i].switchResend++;
            return true;
        }
    }

    if (radio_upgrade_tx_info.txCount >= RADIO_UPGRADE_CACHE_LEN) {
        return true;
    }
    return false;
}


static ErrorStatus RadioUpgrade_TxInfoMatch(uint32_t offset, uint8_t len)
{
    uint16_t i = 0;
    ErrorStatus status = ES_ERROR;

    for (; i < RADIO_UPGRADE_CACHE_LEN; i++) {
        if (radio_upgrade_tx_info.info[i].offset == offset && radio_upgrade_tx_info.info[i].frameLen == len) {
            radio_upgrade_tx_info.info[i].isNoReceive = false;
            radio_upgrade_tx_info.info[i].resendCount = 0;
            radio_upgrade_tx_info.info[i].frameLen = 0;
            radio_upgrade_tx_info.info[i].switchResend = 0;
            status = ES_SUCCESS;
        }
    }

    if (status == ES_SUCCESS) {
        if (radio_upgrade_tx_info.txCount > 0) {
            radio_upgrade_tx_info.txCount--;
        } else {
            status = ES_ERROR;
        }
    }

    return status;
}

static UPGRADE_RESULT_E RaidoUpgrade_LoadCheck(void)
{
    if (sRadio_upgrade_info.fileLength <= 128 || sRadio_upgrade_info.fileLength > (128 * 1024)) {
        return UPGRADE_RESULT_CRC_ERR;
    }
    return Usb_bsl_UpgradeLoad_check(sRadio_upgrade_info.fileLength);
#if 0
    uint16_t crc2;
    uint8_t buff[64];
    uint32_t addr, readLen;
    upgrade_flag_t upgradeFlag;

    // check the crc
    addr    = 0;
    readLen = sRadio_upgrade_info.fileLength;  //1d896
    Flash_load_upgrade_data(addr, buff, 64);
    usb_upgrade_info.crc = ((uint16_t)buff[1] << 8) + buff[0];
    readLen = readLen - 128;
    addr = 128;
    SetContinueCRC16();
    while(readLen)
    {
        if(readLen >= RADIO_CODE_DATA_LENGTH)
        {
            WdtClear();
            Flash_load_upgrade_data(addr, buff, RADIO_CODE_DATA_LENGTH);
            addr += RADIO_CODE_DATA_LENGTH;
            crc2 = ContinueCRC16(buff, RADIO_CODE_DATA_LENGTH);
            readLen = readLen - RADIO_CODE_DATA_LENGTH;
        }
        else
        {
            Flash_load_upgrade_data(addr, buff, readLen);
            crc2 = ContinueCRC16(buff, readLen);
            readLen = 0;
        }
    }

    if(usb_upgrade_info.crc != crc2)
    {
        //upgradeInfo.upgradeStep = UPGRADE_START;
        return UPGRADE_RESULT_CRC_ERR;
    }

    memset(&upgradeFlag, 0xff, sizeof(upgrade_flag_t));
    strcpy((char*)upgradeFlag.validFlag, UPGRADE_FLAG);
    upgradeFlag.waiteUpgrade = 0x01;
    upgradeFlag.complete     = 0xff;
    upgradeFlag.crc          = crc2;//usb_upgrade_info.crc;
    upgradeFlag.fileLength   = sRadio_upgrade_info.fileLength;

    Flash_store_upgrade_info((uint8_t*)&upgradeFlag,sizeof(upgrade_flag_t));

    memset(&upgradeFlag, 0, sizeof(upgrade_flag_t));
    Flash_load_upgrade_info((uint8_t*)&upgradeFlag,sizeof(upgrade_flag_t));

    return UPGRADE_RESULT_LOADING_COMPLETE;
#endif
}

static void RadioUpagrade_JumpBoot(void)
{
#ifdef SUPPORT_RADIO_UPGRADE
    #ifdef SUPPORT_WATCHDOG
    if (watchdogHandle != NULL)
        Watchdog_close(watchdogHandle);
    #endif
//    IntMasterDisable();
//    // jump to the BOOT project
//    asm(" MOV R0, #0x0000 ");
//    asm(" LDR R1, [R0, #0x4] ");
//    // Reset the stack pointer,
//    asm(" LDR SP, [R0, #0x0] ");
//    // And jump.
//    asm(" BX R1 ");
    SysCtrlSystemReset();
//    RadioBootFxn();
#endif
}

// Empty message analysis
// ret -1:ERR   0:OK
extern void RadioSendData(void);
static int RadioUpgrade_NodataParse(uint8_t *pData, uint16_t length)
{
    UPGRADE_RESULT_E result;
    uint16_t pack_len = pData[0];
    uint32_t data_offset_addr = ((uint32_t)(pData[1]) << 24) | ((uint32_t)(pData[2]) << 16) | ((uint32_t)(pData[3]) << 8) | (uint32_t)pData[4];

    Led_ctrl(LED_B, 1, 500 * CLOCK_UNIT_MS, 1);
    // CRC check // MAKE BSL flag if OK
    result = RaidoUpgrade_LoadCheck();

    if(UPGRADE_RESULT_LOADING_COMPLETE == result) {
        RadioUpgrade_DataTransmitAck(data_offset_addr, pack_len, ES_SUCCESS);
        RadioSendData();
        Task_sleep(50 * CLOCK_UNIT_MS);
        RadioUpagrade_JumpBoot();
        return 0; // will not execute here
    } else {
        // Clear
        sRadio_upgrade_info.pack_num = 0;
        sRadio_upgrade_info.fileLength = 0;

        RadioUpgrade_DataTransmitAck(data_offset_addr, pack_len, ES_ERROR );
        RadioSendData();
        return -1;
    }
}
// Non-empty message analysis
// pack_len(1)offset(4)pack_data(pack_len)
// ret -1:ERR   0:OK
static int RadioUpgrade_DataParse(uint8_t *pData, uint16_t length)
{
    uint16_t pack_len = pData[0];
    uint32_t data_offset_addr = ((uint32_t)(pData[1]) << 24) | ((uint32_t)(pData[2]) << 16) | ((uint32_t)(pData[3]) << 8) | (uint32_t)pData[4];
    const uint8_t pack_head_len = 5; //

    // pack length check
    if(0xfffff == pack_len)
        return -1;
    // MAX upgrade firmware size
    if(data_offset_addr > (uint32_t)(148*1024LL))
        return -1;

    // data length check
    if(length - pack_head_len != pack_len)
        return -1;

    if(0 == data_offset_addr){   // first pack
        sRadio_upgrade_info.pack_num = 0;
        sRadio_upgrade_info.pack_offset = data_offset_addr;
        sRadio_upgrade_info.pack_size = pack_len;
        sRadio_upgrade_info.fileLength = pack_len;
    }else{
        sRadio_upgrade_info.pack_num++;
        sRadio_upgrade_info.pack_offset = data_offset_addr;
        sRadio_upgrade_info.pack_size = pack_len;
        sRadio_upgrade_info.fileLength = data_offset_addr + pack_len;
    }

    // truncate data
    Flash_store_upgrade_data(data_offset_addr, pData + pack_head_len, length - pack_head_len);

    RadioUpgrade_DataTransmitAck(data_offset_addr, pack_len, ES_SUCCESS);
    return 0;
}

static void RadioUpgrade_DataTransmit(uint32_t offset, uint8_t len)
{
    uint8_t i = 0;

    if (len < 64) {
       sTxRadio_upgrade_info.endFlag = true;
//       System_printf("len: %d, offset:0x%02x%02x%02x%02x\r\n", len, (uint8_t)((offset >> 24) & 0xff),(uint8_t)((offset >> 16) & 0xff),\
//                     (uint8_t)((offset >> 8) & 0xff), (uint8_t)((offset >> 0) & 0xff));
//       System_flush();
    }

    ClearRadioSendBuf();
    sTxRadio_upgrade_info.fileLength = offset + len;
    sTxRadio_upgrade_info.pack_num++;
    sTxRadio_upgrade_info.pack_offset = offset + len;

    SetRadioDstAddr(sRadioUpgradeAddress);
    protocalTxBuf.dstAddr = GetRadioDstAddr();
    protocalTxBuf.srcAddr = GetRadioSrcAddr();
    protocalTxBuf.command = RADIO_PRO_CMD_UPGRADE;

    protocalTxBuf.load[i++] = len;

    protocalTxBuf.load[i++] = (uint8_t)((offset >> 24) & 0xff);
    protocalTxBuf.load[i++] = (uint8_t)((offset >> 16) & 0xff);
    protocalTxBuf.load[i++] = (uint8_t)((offset >> 8) & 0xff);
    protocalTxBuf.load[i++] = (uint8_t)((offset >> 0) & 0xff);

    if (len != 0) {
        Flash_load_upgrade_data(offset, &protocalTxBuf.load[i], len);
    }

    protocalTxBuf.len = i + 10 + len;
    RadioSendPacket((uint8_t*)&protocalTxBuf, protocalTxBuf.len, 0, 0);
}

static void RadioUpgrade_startFileSendTimer(void)
{
    Clock_start(radioUpgradeClkHandle);
}

static void RadioUpgrade_stopFileSendTimer(void)
{
    Clock_stop(radioUpgradeClkHandle);
}


static void RadioUpgrade_SendRateReq(void)
{
    ClearRadioSendBuf();
    SetRadioDstAddr(sRadioUpgradeAddress);
    protocalTxBuf.dstAddr = GetRadioDstAddr();
    protocalTxBuf.srcAddr = GetRadioSrcAddr();
    protocalTxBuf.command = RADIO_PRO_CMD_RATE_SWITCH;

    protocalTxBuf.len = 10;
    RadioSendPacket((uint8_t*)&protocalTxBuf, protocalTxBuf.len, 0, 0);
}


static void RadioUpgrade_SendRateReqAck(uint8_t status)
{
    ClearRadioSendBuf();
    SetRadioDstAddr(CONFIG_DECEIVE_ID_DEFAULT);
    protocalTxBuf.dstAddr = GetRadioDstAddr();
    protocalTxBuf.srcAddr = GetRadioSrcAddr();
    protocalTxBuf.command = RADIO_PRO_CMD_RATE_SWTICH_ACK;
    protocalTxBuf.load[0] = status;

    protocalTxBuf.len = 10 + 1;
    RadioSendPacket((uint8_t*)&protocalTxBuf, protocalTxBuf.len, 0, 0);
}
