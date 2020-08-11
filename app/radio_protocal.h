#ifndef     __RADIO_PROTOCAL_H__
#define     __RADIO_PROTOCAL_H__



/***** Defines *****/
#define     RADIO_PROTOCAL_LOAD_MAX     (EASYLINK_MAX_DATA_LENGTH - 10)

#define 	PROTOCAL_SUCCESS			0
#define 	PROTOCAL_FAIL				1



// radio cmd type


#define     RADIO_CMD_SYN_TYPE                      1
#define     RADIO_CMD_LOW_VOL_TYPE                  2
#define     RADIO_CMD_DESTROY_TYPE                  3
#define     RADIO_CMD_INSERT_TYPE                   4
#define     RADIO_PRO_CMD_TERM_ADD_TO_GROUP         5              //�ն���ӵ���
#define     RADIO_PRO_CMD_TERM_DELETE_FROM_GROUP    6               //����ɾ��
#define     RADIO_PRO_CMD_TERM_TEST                 7               //�ն˲���
#define     RADIO_PRO_CMD_GROUP_TEST                8              //Ⱥ�����
#define     RADIO_PRO_CMD_TERM_CLOSE_CTROL          9              //�ر��Ʒ�
#define     RADIO_PRO_CMD_TERM_OPEN_CTROL           10              //���Ʒ�
#define     RADIO_PRO_CMD_GROUP_CLOSE_CTROL         11                //�����Ʒ�
#define     RADIO_PRO_CMD_GROUP_OPEN_CTROL          12            //�����Ʒ�
#define     RADIO_PRO_CMD_TERM_UNLOCKING            13              //�ն˽���
#define     RADIO_PRO_CMD_GROUP_UNLOCKING           14               //Ⱥ�����

#define     RADIO_PRO_CMD_GROUP_POWER_HIGH          15                //Ⱥ���ѹ��
#define     RADIO_PRO_CMD_GROUP_POWER_MID           16              //Ⱥ���ѹ��
#define     RADIO_PRO_CMD_GROUP_POWER_LOW           17              //Ⱥ���ѹ��
#define     RADIO_PRO_CMD_TERM_POWER_HIGH           18              //�ն˵�ѹ��
#define     RADIO_PRO_CMD_TERM_POWER_MID            19               //�ն˵�ѹ��
#define     RADIO_PRO_CMD_TERM_POWER_LOW            20              //�ն˵�ѹ��

#define     RADIO_PRO_CMD_FIXED_TERM_SUBDUE_START   21              //��Ա�Ʒ�
#define     RADIO_PRO_CMD_FIXED_TERM_SUBDUE_STOP    22              //��Աֹͣ

#define     RADIO_PRO_CMD_GROUP_SUBDUE_START        23               //Ⱥ���Ʒ�
#define     RADIO_PRO_CMD_GROUP_SUBDUE_STOP         24            //Ⱥ��ֹͣ

#define     RADIO_PRO_CMD_ALL_SUBDUE_START          25             //ȫԱ�Ʒ�
//#define     RADIO_PRO_CMD_ALL_SUBDUE_STOP                        //��Աֹͣ
#define     RADIO_PRO_CMD_TERM_CLOSE_BLOCKING       26             //�رշ���
#define     RADIO_PRO_CMD_TERM_OPEN_BLOCKING        27             //�򿪷���

#define     RADIO_PRO_CMD_OPEN_PREVENT_ESCAPE        28             //�򿪷���
#define     RADIO_PRO_CMD_CLOSE_PREVENT_ESCAPE       29             //�رշ���


#define     RADIO_PRO_CMD_ALL_WAKEUP          		30             //�����豸�����ڷ��ӵ�

#define     RADIO_PRO_CMD_OPEN_TERMINAL_PREVENT_ESCAPE        31   //���ն˷���
#define     RADIO_PRO_CMD_CLOSE_TERMINAL_PREVENT_ESCAPE       32   //���ն˷���
#define     RADIO_PRO_CMD_OPEN_GROUP_PREVENT_ESCAPE           33   //�������
#define     RADIO_PRO_CMD_CLOSE_GROUP_PREVENT_ESCAPE          34   //�������
#define     RADIO_PRO_CMD_PREVENT_ESCAPE_ALARM		          35   //���ӱ���

#define     RADIO_PRO_CMD_REQUES_TERM_LOG		          	  36   //�����������־
#define     RADIO_PRO_CMD_LOG_SEND                            37   //���������־

#define     RADIO_PRO_CMD_MOTO_RUN		          		      38   //��������100ms
#define     RADIO_PRO_CMD_ALL_SUBDUE_STOP                     39     //ȫԱֹͣ


#define     RADIO_PRO_CMD_ALL_RESP          		63             //����Ӧ��

// radio protocal cmd
#define     RADIO_PRO_CMD_SYN_TIME_REQ              0XB1
#define     RADIO_PRO_CMD_SYN_TIME              	0XC1

#define     RADIO_PRO_CMD_SINGLE              		0XB2
#define     RADIO_PRO_CMD_SINGLE_RESP              	0XC2


#define     RADIO_PRO_CMD_SINGLE_WITH_NO_RESP       0XB3


#define     RADIO_PRO_CMD_GROUND              		0XB4


#define     RADIO_PRO_CMD_GROUND_WITH_NO_RESP       0XB5




extern uint8_t     	concenterRemainderCache;
extern uint8_t 		nodeSendingLog;
extern uint32_t 	nodegLogCnt;
/***** Type declarations *****/

typedef  struct {
    uint8_t     command;
    // uint8_t     len;
    // uint32_t    dstAddr;
    uint32_t    srcAddr;
    uint16_t	cmdType;
    union{
    	uint8_t 	rtc[6];
    	struct
    	{
		    uint32_t    ground;
		    uint16_t	brocastRemainder;
    	};
    	struct
    	{
	    	uint16_t 	vol;
    		uint32_t    resever;
    	};
    };
    uint8_t     load[RADIO_PROTOCAL_LOAD_MAX];
}__attribute__((packed)) radio_protocal_t;

#define  RADIO_BUF_LEN		15


/***** Prototypes *****/

// void NodeProtocalEvtSet(EasyLink_RxPacket *rxPacket);
#ifdef S_C
void NodeProtocalDispath(EasyLink_RxPacket * protocalRxPacket);
#else
#define NodeProtocalDispath(...)
#endif // S_C



void ConcenterProtocalDispath(EasyLink_RxPacket * protocalRxPacket);


#define  INVALID_GROUND				0xFFFFFFFF
void GroudAddrSet(uint32_t ground);
uint32_t GroudAddrGet(void);

bool RadioCmdSetWithNoResponBrocast(uint16_t cmd, uint32_t dstAddr);
void RadioCmdSetWithNoRes(uint16_t cmd, uint32_t dstAddr);
void RadioCmdClearWithNoRespon(void);
uint32_t RadioWithNoResPack(void);

bool RadioCmdSetWithNoRespon(uint16_t cmd, uint32_t dstAddr, uint32_t ground);
void RadioCmdClearWithNoRespon_Groud(void);
uint32_t RadioWithNoRes_GroudPack(void);

bool RadioCmdSetWithRespon(uint16_t cmd, uint32_t dstAddr, uint32_t ground);
void RadioCmdClearWithRespon(void);
uint32_t RadioWithResPack(void);
uint16_t GetTestTermVol(void);

#endif      // __RADIO_PROTOCAL_H__
