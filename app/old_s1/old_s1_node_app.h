#ifndef __OLD_S1_NODE_APP__
#define __OLD_S1_NODE_APP__

#define LENGTH_Pattern1TxPacket  27
#define LENGTH_Pattern2TxPacket  35
#define S1_OPERATING_MODE1       1
#define S1_OPERATING_MODE2       2

#define OLD_S1_RF_TX_DATA_FREQUE 433600006   // Khz
#define OLD_S1_RF_RX_DATA_FREQUE 433800003   // Khz

typedef struct {
    uint16_t serialNumber;          // 序列号
    Calendar measureCalendar;           // 采集时间
    uint16_t temperature;           // 温度
    uint16_t humidity;              // 湿度
    uint16_t voltage;               // 电压
} OldSensorData;

/*
 * 模式1的帧格式
 编号    功能                 适用模式    方向           传输速率               频段            起始位 长度  功能   客户标志码          TAG_ID   负载长度    类型  长度  序列号         类型  长度    温度 @1    类型  长度     湿度           类型  长度   电池电压        结束位
 1   上传传感数据     1     TAG->BB 38.4Kbps    433.6   EA   18  01                       10     0F  02          0B  02           0C  02          09  02           AE
 下标：                                                                                                                 0    1   2   3  4       5 6 7 8   9      10  11  12 13   14  15  16 17    18  19   20 21  22  23  24 25    26

*模式2的帧格式
编号     功能                  适用模式    方向             传输速率             频段            起始位 长度  功能  客户标志码          TAG_ID    负载长度   类型  长度  序列号        类型  长度    时间                                        类型  长度  温度 @1       类型  长度   湿度        类型  长度  电池电压        结束位
 1   上传传感数据     2      TAG->BB 38.4Kbps    433.6  EA   20  02                       18     0F  02          0A  06                     0B  02              0C  02         09 02           AE
 下标：                                                                                                                 0    1   2    3 4        5 6 7 8  9      10  11  12 13   14  15  16 17 18 19 20 21  22  23  24  25      26  27  28 29  30 31  32 33    34

编号     功能                   适用模式    方向            传输速率              频段                         起始位     TAG_ID后两位(2字节)   序列号(2字节)
 2   上传传感数据     2      BB->TAG 38.4Kbps     433.8MHz    BA


*/
extern uint8_t  g_oldS1OperatingMode;

extern void OldS1NodeApp_startSendSensorData(void);
extern void OldS1NodeApp_stopSendSensorData(void);

extern void OldS1NodeApp_setDataTxRfFreque(void);
extern void OldS1NodeApp_setDataRxRfFreque(void);
extern void OldS1NodeApp_init(void);
extern void OldS1NodeAPP_Mode2NodeUploadProcess(void);
extern void OldS1NodeAPP_scheduledUploadData(void);
extern void OldS1NodeApp_RtcIProcess(void);

extern void OldS1NodeApp_protocolProcessing(uint8_t *pData, uint8_t len);

extern void OldS1nodeAPP_setWorkMode(uint8_t mode);

#endif // __OLD_S1_NODE_APP__
