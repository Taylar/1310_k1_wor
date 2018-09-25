#include "../../general.h"
#include <ti/sysbios/BIOS.h>
//#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26XX.h>
#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/aon_batmon.h)
#include DeviceFamily_constructPath(driverlib/trng.h)

#ifdef SUPPORT_BOARD_OLD_S1

uint8_t  g_oldS1OperatingMode = S1_OPERATING_MODE2;

Clock_Struct oldS1StartClock;
Clock_Handle oldS1StartClockHandle;

static uint16_t frameSerialNum;
static uint16_t mode2TxFrameSerialNum;
static uint16_t mode2TxLastTwoID;
static uint8_t  txResendCnt;

Semaphore_Struct oldS1UploadeSemStruct;
Semaphore_Handle oldS1UploadeSemHandle;

//static uint8_t sensorDataSendSuccessFlag = true;

static uint32_t getRandom(void);

static void PackMode1UplodData(OldSensorData sensorData, uint8_t *pdata);
static void PackMode2UplodData(OldSensorData sensorData, uint8_t *pdata);

static void SendDataTimingFxn(UArg arg0);

static void txResendCntClear(void);
static void txResendProcess(void);

void OldS1NodeApp_setDataTxRfFreque(void)
{
    uint32_t freq;
    freq = EasyLink_getFrequency();

    if (freq < OLD_S1_RF_TX_DATA_FREQUE) {
        freq = OLD_S1_RF_TX_DATA_FREQUE - freq;
    } else {
        freq = freq - OLD_S1_RF_TX_DATA_FREQUE;
    }

    if (freq  <  100000) {
        return;
    }

    EasyLink_Status status = EasyLink_setFrequency(OLD_S1_RF_TX_DATA_FREQUE);
    if (status != EasyLink_Status_Success) {
         System_printf("setDataTxRfFreque state = %d\r\n", status);
         System_flush();
    } else {
        RadioAbort();
        EasyLink_setFrequency(OLD_S1_RF_TX_DATA_FREQUE);
    }
}

void OldS1NodeApp_setDataRxRfFreque(void)
{
    uint32_t freq;
    freq = EasyLink_getFrequency();

    if (freq < OLD_S1_RF_RX_DATA_FREQUE) {
        freq = OLD_S1_RF_RX_DATA_FREQUE - freq;
    } else {
        freq = freq - OLD_S1_RF_RX_DATA_FREQUE;
    }

    if (freq < 100000) {
        return;
    }

//    uint32_t time1, time2;

//    time1 = Clock_getTicks();
//    Task_sleep(1 * CLOCK_UNIT_MS);
//    time2 = Clock_getTicks();
//    System_printf("1ms = %d\r\n", (time2-time1));
//
//    time1 = Clock_getTicks();
    EasyLink_Status status = EasyLink_setFrequency(OLD_S1_RF_RX_DATA_FREQUE);
    if (status != EasyLink_Status_Success) {
         System_printf("setDataRxRfFreque state = %d\r\n", status);
         System_flush();
    } else {
        RadioAbort();
        EasyLink_setFrequency(OLD_S1_RF_RX_DATA_FREQUE);
    }
//    time2 = Clock_getTicks();
//    System_printf("shijianRfS: = %d\r\n", (time2-time1));
//    System_flush();
}

void OldS1NodeApp_init(void)
{
    /* Construct a 1000ms periodic Clock Instance to is Tx */
    Clock_Params clkParams;
    Clock_Params_init(&clkParams);
    clkParams.period = 1100 * CLOCK_UNIT_MS;
    clkParams.startFlag = FALSE;
    Clock_construct(&oldS1StartClock, (Clock_FuncPtr)SendDataTimingFxn,
                    ((getRandom() % 30) * CLOCK_UNIT_MS), &clkParams);
    /* Obtain clock instance handle */
    oldS1StartClockHandle = Clock_handle(&oldS1StartClock);


    Semaphore_Params semParams;
    Semaphore_Params_init(&semParams);
    Semaphore_construct(&oldS1UploadeSemStruct, 1, &semParams);
    oldS1UploadeSemHandle = Semaphore_handle(&oldS1UploadeSemStruct);

    //
    SetRadioDstAddr(0xdadadada);
    ConcenterRadioSendParaSet(0xabababab, 0xbabababa);

    txResendCnt = 0;
}

void OldS1NodeAPP_Mode2NodeUploadProcess(void)
{
    uint8_t     data[LENGTH_Pattern2TxPacket];
    uint32_t    dataItems;
    OldSensorData sensorData;

    Semaphore_pend(oldS1UploadeSemHandle, BIOS_WAIT_FOREVER);
    OldS1NodeApp_stopSendSensorData();
    // reverse the buf to other command
    dataItems  = Flash_get_unupload_items();

    if (dataItems > 0) {
        RadioSwitchingS1OldUserRate();

        Flash_load_sensor_data_by_offset(data, 22, 0);
        sensorData.serialNumber = ((data[6] << 8) | data[7]);

        sensorData.measureCalendar.Year = data[8];
        sensorData.measureCalendar.Month = data[9];
        sensorData.measureCalendar.DayOfMonth = data[10];
        sensorData.measureCalendar.Hours = data[11];
        sensorData.measureCalendar.Minutes = data[12];
        sensorData.measureCalendar.Seconds = data[13];

        sensorData.voltage = ((data[14] << 8) | data[15]);

        sensorData.temperature = ((data[18] << 8) | data[19]);
        sensorData.humidity = ((data[20] << 8) | data[21]);

        PackMode2UplodData(sensorData, data);

        ClearRadioSendBuf();

        RadioCopyPacketToBuf(data, LENGTH_Pattern2TxPacket, 0, PASSRADIO_ACK_TIMEOUT_TIME_MS, 0);
        OldS1NodeApp_startSendSensorData();
    }

    Semaphore_post(oldS1UploadeSemHandle);
}

void OldS1NodeAPP_scheduledUploadData(void)
{
    uint16_t randDelay;
    OldSensorData sensorData;
    uint8_t buff[LENGTH_Pattern1TxPacket] = {0};

    if (g_oldS1OperatingMode == S1_OPERATING_MODE1 && deviceMode == DEVICES_ON_MODE ) {
        Sensor_measure(0);
        sensorData.serialNumber = frameSerialNum;
        frameSerialNum++;
        sensorData.humidity = rSensorData[0].humi;
        sensorData.temperature = rSensorData[0].temp;
        sensorData.voltage = Battery_get_voltage();

        PackMode1UplodData(sensorData, buff);
        RadioSwitchingS1OldUserRate();

        randDelay = 15 * (getRandom() % 20) + 225;
        Task_sleep(randDelay * CLOCK_UNIT_MS);

        ClearRadioSendBuf();
        RadioSendPacket(buff, LENGTH_Pattern1TxPacket, 0, 0);
        RadioModeSet(RADIOMODE_SENDPORT);
    } else if(g_oldS1OperatingMode == S1_OPERATING_MODE2) {
        Sensor_measure(1);
        if (deviceMode == DEVICES_ON_MODE) {
            Event_post(systemAppEvtHandle, SYS_EVT_EVT_OLD_S1_UPLOAD_NODE);
        }
    }
}


void OldS1NodeApp_protocolProcessing(uint8_t *pData, uint8_t len)
{
    /*
    功能                            适用模式            方向            传输速率                   频段                  起始位      TAG_ID后两位(2字节)   序列号 (2字节)
    上传传感数据的ACK  2        BB->TAG 38.4Kbps    433.8MHz    BA
    */

    static uint16_t lastSerialNum = 0xffff;
    static uint8_t ackRepeatTimes = 0;

    uint16_t serialNum, id;
    bool flag = false;

//    ClearRadioSendBuf();
    OldS1NodeApp_stopSendSensorData();
    // Software version after (version: 15 74)
    if (5 == len && pData[0] == 0xba) {
        id = (uint16_t)((pData[1] << 8) | pData[2]);
        serialNum = ((pData[3] << 8) | pData[4]);
        if ((id == mode2TxLastTwoID) && (serialNum == mode2TxFrameSerialNum)) {
            flag = true;
        }
    } else if (3 == len && pData[0] == 0xba) { // Software version before (version: 15 74)
        serialNum = ((pData[1] << 8) | pData[2]);
        if ((serialNum == mode2TxFrameSerialNum)) {
            flag = true;
        }
    }

    id  = Flash_get_unupload_items();
    if (flag) {
        ackRepeatTimes++; /// 重复次数，如果ack重复次数超过10条表示有两条数据的序列号重复了
        txResendCntClear();
        if ((serialNum != lastSerialNum) || (ackRepeatTimes > 20)) {
            ackRepeatTimes = 0;
            lastSerialNum = serialNum;
            if (id > 0) {
                Flash_moveto_next_sensor_data();
            }
        }
    }

    if (id > 0 && flag) {
        OldS1NodeAPP_Mode2NodeUploadProcess();
    } else {
        RadioModeSet(RADIOMODE_SENDPORT);
    }
}

/*
 *功能：设置S1的工作模式
 *参数 mode  S1_OPERATING_MODE1-模式1不带反馈
 *          S1_OPERATING_MODE2-带反馈
 * */
void OldS1nodeAPP_setWorkMode(uint8_t mode)
{
    g_oldS1OperatingMode = mode;
}

extern Semaphore_Handle radomFuncSemHandle;
static uint32_t getRandom(void)
{
    uint32_t randomNum;
    // gennerate a ramdom num maybe waste almost 1sec
    /* Use the True Random Number Generator to generate sensor node address randomly */
    Semaphore_pend(radomFuncSemHandle, BIOS_WAIT_FOREVER);
    Power_setDependency(PowerCC26XX_PERIPH_TRNG);
    TRNGEnable();
    while (!(TRNGStatusGet() & TRNG_NUMBER_READY)) {
        //wiat for randum number generator
    }
    randomNum = TRNGNumberGet(TRNG_LOW_WORD);

    while (!(TRNGStatusGet() & TRNG_NUMBER_READY)) {
            //wiat for randum number generator
    }

    randomNum |= ((uint32_t)TRNGNumberGet(TRNG_HI_WORD)) << 16;

    TRNGDisable();
    Power_releaseDependency(PowerCC26XX_PERIPH_TRNG);
    Semaphore_post(radomFuncSemHandle);

    return randomNum;
}


/*
* 模式1的帧格式
 编号    功能                 适用模式    方向           传输速率               频段            起始位 长度  功能   客户标志码          TAG_ID   负载长度    类型  长度  序列号         类型  长度    温度 @1    类型  长度     湿度           类型  长度   电池电压        结束位
 1   上传传感数据     1     TAG->BB 38.4Kbps    433.6   EA   18  01                       10     0F  02          0B  02           0C  02          09  02           AE
 下标：                                                                                                                 0    1   2   3  4       5 6 7 8   9      10  11  12 13   14  15  16 17    18  19   20 21  22  23  24 25    26
*/
static void PackMode1UplodData(OldSensorData sensorData, uint8_t *pdata)
{
    if (NULL == pdata) {
        return;
    }

    uint8_t len = 0;
    uint32_t tmp;

    // start
    pdata[len++] = 0xEA;

    // length
    pdata[len++] = 0x18;

    // cmd
    pdata[len++] = 0x01;

    // customer
    pdata[len++] =  g_rSysConfigInfo.customId[0];
    pdata[len++] =  g_rSysConfigInfo.customId[1];

    // id
    pdata[len++] =  g_rSysConfigInfo.DeviceId[0];
    pdata[len++] =  g_rSysConfigInfo.DeviceId[1];
    pdata[len++] =  g_rSysConfigInfo.DeviceId[2];
    pdata[len++] =  g_rSysConfigInfo.DeviceId[3];

    // payload length
    pdata[len++] = 0x10;

    // serial
    pdata[len++] = 0x0F;
    pdata[len++] = 0x02;
    pdata[len++] = sensorData.serialNumber >> 8;
    pdata[len++] = sensorData.serialNumber & 0x00FF;

    // SHT20 temperature
    pdata[len++]= 0x0B;
    pdata[len++] = 0x02;
    pdata[len++] = sensorData.temperature >> 8;
    pdata[len++] = sensorData.temperature & 0x00FF;

    // SHT20 humidity
    pdata[len++] = 0x0C;
    pdata[len++] = 0x02;
    pdata[len++] = sensorData.humidity >> 8;
    pdata[len++] = sensorData.humidity & 0x00FF;

    // 计算公式： U = (上传值)*4/1023, 单位：V;
    tmp = ((uint32_t)(sensorData.voltage * 1023 / 4)) / 1000;
    sensorData.voltage = (uint16_t)tmp;
    // voltage
    pdata[len++] = 0x09;
    pdata[len++] = 0x02;
    pdata[len++] = sensorData.voltage >> 8;
    pdata[len++] = sensorData.voltage & 0x00FF;

    // end
    pdata[len] = 0xAE;
}

/*
*模式2的帧格式
编号     功能                  适用模式    方向             传输速率             频段            起始位 长度  功能  客户标志码          TAG_ID    负载长度   类型  长度  序列号        类型  长度    时间                                        类型  长度  温度 @1       类型  长度   湿度        类型  长度  电池电压        结束位
 1   上传传感数据     2      TAG->BB 38.4Kbps    433.6  EA   20  02                       18     0F  02          0A  06                     0B  02              0C  02         09 02           AE
 下标：                                                                                                                 0    1   2    3 4        5 6 7 8  9      10  11  12 13   14  15  16 17 18 19 20 21  22  23  24  25      26  27  28 29  30 31  32 33    34
*/
static void PackMode2UplodData(OldSensorData sensorData, uint8_t *pdata)
{
    if (NULL == pdata) {
        return;
    }

    uint8_t len = 0;
    uint16_t tmp;

    pdata[len++] = 0xEA;                            // 起始位
    pdata[len++] = 0x20;                            // 数据包长度
    pdata[len++] = 0x02;                            // 数据包类型
    pdata[len++] = g_rSysConfigInfo.customId[0];          // 客户标识码
    pdata[len++] = g_rSysConfigInfo.customId[1];
    pdata[len++] = g_rSysConfigInfo.DeviceId[0];                // Local ID
    pdata[len++] = g_rSysConfigInfo.DeviceId[1];
    pdata[len++] = g_rSysConfigInfo.DeviceId[2];
    pdata[len++] = g_rSysConfigInfo.DeviceId[3];

    mode2TxLastTwoID = ((g_rSysConfigInfo.DeviceId[2] << 8) | g_rSysConfigInfo.DeviceId[3]);
    mode2TxFrameSerialNum = sensorData.serialNumber;

    pdata[len++] = 0x18;                            // 有效负载长度

    pdata[len++] = 0x0F;                           // 序列号
    pdata[len++] = 0x02;
    pdata[len++] = sensorData.serialNumber >> 8;
    pdata[len++] = sensorData.serialNumber & 0x00FF;

    pdata[len++] = 0x0A;                           // 采集时间
    pdata[len++] = 0x06;

    pdata[len++] = sensorData.measureCalendar.Year & 0x00FF;
    pdata[len++] = sensorData.measureCalendar.Month;
    pdata[len++] = sensorData.measureCalendar.DayOfMonth;
    pdata[len++] = sensorData.measureCalendar.Hours;
    pdata[len++] = sensorData.measureCalendar.Minutes;
    pdata[len++] = sensorData.measureCalendar.Seconds;

    pdata[len++] = 0x0B;                           // 温度
    pdata[len++] = 0x02;
    pdata[len++] = sensorData.temperature >> 8;
    pdata[len++] = sensorData.temperature & 0x00FF;

    pdata[len++] = 0x0C;                           // 湿度
    pdata[len++] = 0x02;
    pdata[len++] = sensorData.humidity >> 8;
    pdata[len++] = sensorData.humidity & 0x00FF;

    // 计算公式： U = (上传值)*4/1023, 单位：V;
    tmp = ((uint32_t)(sensorData.voltage * 1023 / 4)) / 1000;
    sensorData.voltage = (uint16_t)tmp;
    pdata[len++] = 0x09;                           // 电压
    pdata[len++] = 0x02;
    pdata[len++] = sensorData.voltage >> 8;
    pdata[len++] = sensorData.voltage & 0x00FF;

    pdata[len++] = 0xAE;                           // 结束位
}

static void SendDataTimingFxn(UArg arg0)
{
    txResendProcess();
    RadioSend();
}

void OldS1NodeApp_startSendSensorData(void)
{
    if (Clock_isActive(oldS1StartClockHandle)) {
            Clock_stop(oldS1StartClockHandle);
    }

    Clock_setTimeout(oldS1StartClockHandle, ((getRandom() % 30) * CLOCK_UNIT_MS));
    Clock_setPeriod(oldS1StartClockHandle, 1000 * CLOCK_UNIT_MS);
    Clock_start(oldS1StartClockHandle);
}

void OldS1NodeApp_stopSendSensorData(void)
{
    if (Clock_isActive(oldS1StartClockHandle)) {
        ClearRadioSendBuf();
        Clock_stop(oldS1StartClockHandle);
    }
}


static void txResendCntClear(void)
{
    txResendCnt = 0;
}

static void txResendProcess(void)
{
    if (txResendCnt > 6) {
        txResendCnt = 0;
        OldS1NodeApp_stopSendSensorData();
    }

    txResendCnt++;
}
#endif
