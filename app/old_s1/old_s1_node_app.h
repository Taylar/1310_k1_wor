#ifndef __OLD_S1_NODE_APP__
#define __OLD_S1_NODE_APP__

#define LENGTH_Pattern1TxPacket  27
#define LENGTH_Pattern2TxPacket  35
#define S1_OPERATING_MODE1       1
#define S1_OPERATING_MODE2       2

#define OLD_S1_RF_TX_DATA_FREQUE 433600006   // Khz
#define OLD_S1_RF_RX_DATA_FREQUE 433800003   // Khz

typedef struct {
    uint16_t serialNumber;          // ���к�
    Calendar measureCalendar;           // �ɼ�ʱ��
    uint16_t temperature;           // �¶�
    uint16_t humidity;              // ʪ��
    uint16_t voltage;               // ��ѹ
} OldSensorData;

/*
 * ģʽ1��֡��ʽ
 ���    ����                 ����ģʽ    ����           ��������               Ƶ��            ��ʼλ ����  ����   �ͻ���־��          TAG_ID   ���س���    ����  ����  ���к�         ����  ����    �¶� @1    ����  ����     ʪ��           ����  ����   ��ص�ѹ        ����λ
 1   �ϴ���������     1     TAG->BB 38.4Kbps    433.6   EA   18  01                       10     0F  02          0B  02           0C  02          09  02           AE
 �±꣺                                                                                                                 0    1   2   3  4       5 6 7 8   9      10  11  12 13   14  15  16 17    18  19   20 21  22  23  24 25    26

*ģʽ2��֡��ʽ
���     ����                  ����ģʽ    ����             ��������             Ƶ��            ��ʼλ ����  ����  �ͻ���־��          TAG_ID    ���س���   ����  ����  ���к�        ����  ����    ʱ��                                        ����  ����  �¶� @1       ����  ����   ʪ��        ����  ����  ��ص�ѹ        ����λ
 1   �ϴ���������     2      TAG->BB 38.4Kbps    433.6  EA   20  02                       18     0F  02          0A  06                     0B  02              0C  02         09 02           AE
 �±꣺                                                                                                                 0    1   2    3 4        5 6 7 8  9      10  11  12 13   14  15  16 17 18 19 20 21  22  23  24  25      26  27  28 29  30 31  32 33    34

���     ����                   ����ģʽ    ����            ��������              Ƶ��                         ��ʼλ     TAG_ID����λ(2�ֽ�)   ���к�(2�ֽ�)
 2   �ϴ���������     2      BB->TAG 38.4Kbps     433.8MHz    BA


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
