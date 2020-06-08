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


#define     RADIO_PRO_CMD_TERM_ADD_TO_GROUP         0xD1              //终端添加到组
#define     RADIO_PRO_CMD_TERM_DELETE_FROM_GROUP    0xD2               //从组删除
#define     RADIO_PRO_CMD_TERM_TEST                 0xD3               //终端测试
#define     RADIO_PRO_CMD_GROUP_TEST                0xD4              //群组测试
#define     RADIO_PRO_CMD_TERM_CLOSE_CTROL          0xD5              //关闭制服
#define     RADIO_PRO_CMD_TERM_OPEN_CTROL           0xD6              //打开制服
#define     RADIO_PRO_CMD_GROUP_CLOSE_CTROL         0xD7                //关组制服
#define     RADIO_PRO_CMD_GROUP_OPEN_CTROL          0xD8            //开组制服
#define     RADIO_PRO_CMD_TERM_UNLOCKING            0xD9              //终端解锁
#define     RADIO_PRO_CMD_GROUP_UNLOCKING           0xDA               //群组解锁

#define     RADIO_PRO_CMD_GROUP_POWER_HIGH          0xDB                //群组电压高
#define     RADIO_PRO_CMD_GROUP_POWER_MID           0xDC              //群组电压中
#define     RADIO_PRO_CMD_GROUP_POWER_LOW           0xDD              //群组电压低
#define     RADIO_PRO_CMD_TERM_POWER_HIGH           0xDE              //终端电压高
#define     RADIO_PRO_CMD_TERM_POWER_MID            0xDF               //终端电压中
#define     RADIO_PRO_CMD_TERM_POWER_LOW            0xE1              //终端电压低

#define     RADIO_PRO_CMD_FIXED_TERM_SUBDUE_START   0xE2              //定员制服
#define     RADIO_PRO_CMD_FIXED_TERM_SUBDUE_STOP    0xE3              //定员停止

#define     RADIO_PRO_CMD_GROUP_SUBDUE_START        0xE4               //群组制服
#define     RADIO_PRO_CMD_GROUP_SUBDUE_STOP         0xE5            //群组停止

#define     RADIO_PRO_CMD_ALL_SUBDUE_START          0xE6             //全员制服
//#define     RADIO_PRO_CMD_ALL_SUBDUE_STOP                        //定员停止

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
