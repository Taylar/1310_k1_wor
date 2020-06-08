#ifndef     __RADIO_PROTOCAL_H__
#define     __RADIO_PROTOCAL_H__



/***** Defines *****/
#define     RADIO_PROTOCAL_LOAD_MAX     (EASYLINK_MAX_DATA_LENGTH - 10)

#define 	PROTOCAL_SUCCESS			0
#define 	PROTOCAL_FAIL				1



// radio cmd type
#define         RADIO_CMD_SYN_TYPE                      1
#define         RADIO_CMD_LOW_VOL_TYPE                  2
#define         RADIO_CMD_DESTROY_TYPE                  3
#define         RADIO_CMD_INSERT_TYPE                   4
#define         RADIO_CMD_SINGLE_SHOCK_TYPE             5
#define         RADIO_CMD_SINGLE_STOP_TYPE              6
#define         RADIO_CMD_GROUD_SET_TYPE                7
#define         RADIO_CMD_GROUD_SHOCK_TYPE              8
#define         RADIO_CMD_GROUD_STOP_TYPE               9



// radio protocal cmd
#define     RADIO_PRO_CMD_SYN_TIME_REQ              0XB1
#define     RADIO_PRO_CMD_SYN_TIME	                0XC1

#define     RADIO_PRO_CMD_LOW_VOL_EVT               0XB2
#define     RADIO_PRO_CMD_LOW_VOL_ACK               0XC2

#define     RADIO_PRO_CMD_DESTROY_EVT               0XB3
#define     RADIO_PRO_CMD_DESTROY_ACK               0XC3

#define     RADIO_PRO_CMD_INSERT_EVT                0XB4
#define     RADIO_PRO_CMD_INSERT_ACK                0XC4


#define     RADIO_PRO_CMD_SINGEL_SHORCK_EVT         0XB5
#define     RADIO_PRO_CMD_SINGEL_SHORCK_ACK         0XC5

#define     RADIO_PRO_CMD_SINGEL_STOP_EVT           0XB6
#define     RADIO_PRO_CMD_SINGEL_STOP_ACK           0XC6

#define     RADIO_PRO_CMD_GROUD_SET_EVT             0XB7
#define     RADIO_PRO_CMD_GROUD_SET_ACK             0XC7

#define     RADIO_PRO_CMD_GROUD_SHORCK_EVT          0XC8

#define     RADIO_PRO_CMD_GROUD_STOP_EVT            0XC9


#define     RADIO_PRO_CMD_TERM_ADD_TO_GROUP         0xD1              //�ն���ӵ���
#define     RADIO_PRO_CMD_TERM_DELETE_FROM_GROUP    0xD2               //����ɾ��
#define     RADIO_PRO_CMD_TERM_TEST                 0xD3               //�ն˲���
#define     RADIO_PRO_CMD_GROUP_TEST                0xD4              //Ⱥ�����
#define     RADIO_PRO_CMD_TERM_CLOSE_CTROL          0xD5              //�ر��Ʒ�
#define     RADIO_PRO_CMD_TERM_OPEN_CTROL           0xD6              //���Ʒ�
#define     RADIO_PRO_CMD_GROUP_CLOSE_CTROL         0xD7                //�����Ʒ�
#define     RADIO_PRO_CMD_GROUP_OPEN_CTROL          0xD8            //�����Ʒ�
#define     RADIO_PRO_CMD_TERM_UNLOCKING            0xD9              //�ն˽���
#define     RADIO_PRO_CMD_GROUP_UNLOCKING           0xDA               //Ⱥ�����

#define     RADIO_PRO_CMD_GROUP_POWER_HIGH          0xDB                //Ⱥ���ѹ��
#define     RADIO_PRO_CMD_GROUP_POWER_MID           0xDC              //Ⱥ���ѹ��
#define     RADIO_PRO_CMD_GROUP_POWER_LOW           0xDD              //Ⱥ���ѹ��
#define     RADIO_PRO_CMD_TERM_POWER_HIGH           0xDE              //�ն˵�ѹ��
#define     RADIO_PRO_CMD_TERM_POWER_MID            0xDF               //�ն˵�ѹ��
#define     RADIO_PRO_CMD_TERM_POWER_LOW            0xE1              //�ն˵�ѹ��

#define     RADIO_PRO_CMD_FIXED_TERM_SUBDUE_START   0xE2              //��Ա�Ʒ�
#define     RADIO_PRO_CMD_FIXED_TERM_SUBDUE_STOP    0xE3              //��Աֹͣ

#define     RADIO_PRO_CMD_GROUP_SUBDUE_START        0xE4               //Ⱥ���Ʒ�
#define     RADIO_PRO_CMD_GROUP_SUBDUE_STOP         0xE5            //Ⱥ��ֹͣ

#define     RADIO_PRO_CMD_ALL_SUBDUE_START          0xE6             //ȫԱ�Ʒ�
//#define     RADIO_PRO_CMD_ALL_SUBDUE_STOP                        //��Աֹͣ

uint8_t     concenterRemainderCache;

/***** Type declarations *****/

typedef  struct {
    uint8_t     len;
    uint8_t     command;
    uint32_t    dstAddr;
    uint32_t    srcAddr;
    uint8_t     load[RADIO_PROTOCAL_LOAD_MAX];
}__attribute__((packed)) radio_protocal_t;


/***** Prototypes *****/

// void NodeProtocalEvtSet(EasyLink_RxPacket *rxPacket);
#ifdef S_C
void NodeProtocalDispath(EasyLink_RxPacket * protocalRxPacket);
#else
#define NodeProtocalDispath(...)
#endif // S_C



void ConcenterProtocalDispath(EasyLink_RxPacket * protocalRxPacket);

void NodeRadioSendSynReq(void);

void ConcenterRadioSendSynTime(uint32_t srcAddr, uint32_t dstAddr);




void GroudAddrSet(uint32_t ground);
uint32_t GroudAddrGet(void);
void RadioCmdSetWithNoRes(uint16_t cmd, uint32_t dstAddr);
void RadioCmdClearWithNoRespon(void);
uint16_t RadioWithNoResPack(void);
void RadioCmdSetWithNoRes_Groud(uint16_t cmd, uint32_t ground);
void RadioCmdClearWithNoRespon_Groud(void);
uint16_t RadioWithNoRes_GroudPack(void);
void RadioCmdSetWithRespon(uint16_t cmd, uint32_t dstAddr, uint32_t ground);
void RadioCmdSetWithNoRespon(uint16_t cmd, uint32_t dstAddr, uint32_t ground);
void RadioCmdClearWithRespon(void);
uint16_t RadioWithResPack(void);


#endif      // __RADIO_PROTOCAL_H__
