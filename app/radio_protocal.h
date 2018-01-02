#ifndef     __RADIO_PROTOCAL_H__
#define     __RADIO_PROTOCAL_H__



/***** Defines *****/
#define     RADIO_PROTOCAL_LOAD_MAX     (EASYLINK_MAX_DATA_LENGTH - 10)

#define 	PROTOCAL_SUCCESS			0
#define 	PROTOCAL_FAIL				1


// parameter type 
#define     PARATYPE_TEMP_HUMI_SHT20    0x01
#define     PARATYPE_NTC                0x02
#define     PARATYPE_ILLUMINATION       0x03
#define     PARATYPE_TEMP_MAX31855      0x04



// parameter setting type
#define 	PARASETTING_COLLECT_INTERVAL	0X01
#define 	PARASETTING_UPLOAD_INTERVAL		0X02
#define 	PARASETTING_LOW_TEMP_ALARM		0X03
#define 	PARASETTING_HIGH_TEMP_ALARM		0X04



// radio protocal cmd
#define     RADIO_PRO_CMD_SENSOR_DATA       0X00

#define     RADIO_PRO_CMD_SYN_TIME_REQ      0X01
#define     RADIO_PRO_CMD_SYN_TIME	        0X21

#define     RADIO_PRO_CMD_SET_PARA          0X22
#define     RADIO_PRO_CMD_SET_PARA_ACK      0X02


#define     RADIO_PRO_CMD_ACK               0X20




/***** Type declarations *****/
typedef struct {
    uint8_t     len;
    uint8_t     command;
    uint32_t    dstAddr;
    uint32_t    srcAddr;
    uint8_t     load[RADIO_PROTOCAL_LOAD_MAX];
}radio_protocal_t;


/***** Prototypes *****/

// void NodeProtocalEvtSet(EasyLink_RxPacket *rxPacket);

void NodeProtocalDispath(EasyLink_RxPacket * protocalRxPacket);

// void ConcenterProtocalEvtSet(EasyLink_RxPacket *rxPacket);

void ConcenterProtocalDispath(EasyLink_RxPacket * protocalRxPacket);

bool NodeRadioSendSensorData(uint8_t * dataP, uint8_t length);

void NodeRadioSendSynReq(void);

void NodeRadioSendParaSetAck(ErrorStatus status);

void ConcenterRadioSendSensorDataAck(uint32_t srcAddr, uint32_t dstAddr, ErrorStatus status);

void ConcenterRadioSendSynTime(uint32_t srcAddr, uint32_t dstAddr);

void ConcenterRadioSendParaSet(uint32_t srcAddr, uint32_t dstAddr, uint8_t *dataP, uint8_t length);


#endif      // __RADIO_PROTOCAL_H__
