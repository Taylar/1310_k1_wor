/*
* @Author: zxt
* @Date:   2017-12-26 16:36:20
* @Last Modified by:   zxt
* @Last Modified time: 2020-01-19 11:16:15
*/
#include "../general.h"

#include "../APP/nodeApp.h"
#include "../APP/systemApp.h"
#include "../APP/radio_protocal.h"
#include "../APP/concenterApp.h"
#include "../radio_app/node_strategy.h"
#include "../radio_app/radio_app.h"


radio_protocal_t   protocalTxBuf;

uint8_t     concenterRemainderCache;
bool nodeParaSetting = 0;

uint16_t cmdType, cmdTypeWithRespon, cmdTypeGroud;
uint32_t cmdEvent, cmdEventWithRespon, cmdEventGroud;
uint32_t groundAddr;
#ifdef S_C
//***********************************************************************************
// brief:   analysis the node protocal 
// 
// parameter: 
//***********************************************************************************
void NodeProtocalDispath(EasyLink_RxPacket * protocalRxPacket)
{
	uint8_t len;
	radio_protocal_t	*bufTemp;
    Calendar    calendarTemp;

    TxFrameRecord_t rxSensorDataAckRecord;
    memset(&rxSensorDataAckRecord, 0, sizeof(TxFrameRecord_t));

	len			= protocalRxPacket->len;

    NodeStrategyBuffClear();

	// this buf may be include several message
	bufTemp		= (radio_protocal_t *)protocalRxPacket->payload;

	// SetRadioDstAddr(bufTemp->srcAddr);

	while(len)
	{
		// the receive data is not integrated
		if((bufTemp->len > len) || (bufTemp->len == 0))
			return;
		
		// the resever length
		len 	-= bufTemp->len;

		switch(bufTemp->command)
		{
			case RADIO_PRO_CMD_SYN_TIME:

			calendarTemp.Year       = 2000 + bufTemp->load[0];
			calendarTemp.Month      = bufTemp->load[1];
			calendarTemp.DayOfMonth = bufTemp->load[2];
			calendarTemp.Hours      = bufTemp->load[3];
			calendarTemp.Minutes    = bufTemp->load[4];
			calendarTemp.Seconds    = bufTemp->load[5];
			Rtc_set_calendar(&calendarTemp);

			break;

			case RADIO_PRO_CMD_GROUD_SHORCK_EVT:
            eleShock_ctrl(ELE_PREVENT_INSERT2_ENABLE, 1, 500, 1);
            eleShock_ctrl(ELE_MOTO_ENABLE, 1, 500, 1);
            break;



			default:
			return;

		}

		// point to new message the head
		bufTemp		= (radio_protocal_t *)((uint8_t *)bufTemp + bufTemp->len);
	}
}


#endif // S_C


//***********************************************************************************
// brief:   send the timesyn ack to the strategy process
// 
// parameter: 
//***********************************************************************************
void NodeRadioSendSynReq(void)
{
	protocalTxBuf.len 		= 10;
	protocalTxBuf.dstAddr	= GetRadioDstAddr();
	protocalTxBuf.srcAddr	= GetRadioSrcAddr();
	protocalTxBuf.command	= RADIO_PRO_CMD_SYN_TIME_REQ;

	SetRadioDstAddr(protocalTxBuf.dstAddr);

    RadioCopyPacketToBuf(((uint8_t*)&protocalTxBuf), protocalTxBuf.len, 0, 0, 0);
}


//***********************************************************************************
// brief:   send low vol event
// 
// parameter: 
// srcAddr:	the concenter radio addr
// dstAddr:	the node radio addr
//***********************************************************************************
void NodeRadioSendLowVolEvt(void)
{


	protocalTxBuf.command	= RADIO_PRO_CMD_LOW_VOL_EVT;
	protocalTxBuf.dstAddr	= GetRadioSubDstAddr();
	protocalTxBuf.srcAddr	= GetRadioSrcAddr();
	protocalTxBuf.len 		= 10;

	SetRadioDstAddr(protocalTxBuf.dstAddr);

    RadioCopyPacketToBuf(((uint8_t*)&protocalTxBuf), protocalTxBuf.len, 0, 0, 0);
}


//***********************************************************************************
// brief:   send detroy event
// 
// parameter: 
// srcAddr:	the concenter radio addr
// dstAddr:	the node radio addr
//***********************************************************************************
void NodeRadioSendDestroyEvt(void)
{

	protocalTxBuf.command	= RADIO_PRO_CMD_DESTROY_EVT;
	protocalTxBuf.dstAddr	= GetRadioSubDstAddr();
	protocalTxBuf.srcAddr	= GetRadioSrcAddr();
	protocalTxBuf.len 		= 10;

	SetRadioDstAddr(protocalTxBuf.dstAddr);

    RadioCopyPacketToBuf(((uint8_t*)&protocalTxBuf), protocalTxBuf.len, 0, 0, 0);
}


//***********************************************************************************
// brief:   send insert event
// 
// parameter: 
// srcAddr:	the concenter radio addr
// dstAddr:	the node radio addr
//***********************************************************************************
void NodeRadioSendInsertEvt(void)
{


	protocalTxBuf.command	= RADIO_PRO_CMD_INSERT_EVT;
	protocalTxBuf.dstAddr	= GetRadioSubDstAddr();
	protocalTxBuf.srcAddr	= GetRadioSrcAddr();
	protocalTxBuf.len 		= 10;

	SetRadioDstAddr(protocalTxBuf.dstAddr);

    RadioCopyPacketToBuf(((uint8_t*)&protocalTxBuf), protocalTxBuf.len, 0, 0, 0);
}

//***********************************************************************************
// brief:   send insert event
// 
// parameter: 
// srcAddr:	the concenter radio addr
// dstAddr:	the node radio addr
//***********************************************************************************
void NodeRadioSendAck(uint8_t command)
{


	protocalTxBuf.command	= command;
	protocalTxBuf.dstAddr	= GetRadioSubDstAddr();
	protocalTxBuf.srcAddr	= GetRadioSrcAddr();
	protocalTxBuf.len 		= 10;

	SetRadioDstAddr(protocalTxBuf.dstAddr);

    RadioCopyPacketToBuf(((uint8_t*)&protocalTxBuf), protocalTxBuf.len, 0, 0, 0);
}

//***********************************************************************************
// brief:   analysis the concenter protocal
// 
// parameter: 
//***********************************************************************************
void ConcenterProtocalDispath(EasyLink_RxPacket * protocalRxPacket)
{
	uint8_t len;
	radio_protocal_t	*bufTemp;


	concenterRemainderCache = EASYLINK_MAX_DATA_LENGTH;
	len                     = protocalRxPacket->len;
	bufTemp                 = (radio_protocal_t *)protocalRxPacket->payload;

	ConcenterRadioMonitorClear();
	
	ClearRadioSendBuf();

	SetRadioDstAddr(bufTemp->srcAddr);



	while(len)
	{
		// the receive data is not integrated
		if((bufTemp->len > len) || (bufTemp->len == 0) || (len == 0))
			break;
		
		len 	-= bufTemp->len;
		switch(bufTemp->command)
		{

			default:
		    Sys_event_post(SYSTEMAPP_EVT_DISP);
		    return;

		}
		// point to new message the head
		bufTemp		= (radio_protocal_t *)((uint8_t *)bufTemp + bufTemp->len);
	}

    Sys_event_post(SYSTEMAPP_EVT_DISP);
}





//***********************************************************************************
// brief:   send time to the node immediately
// 
// parameter: 
// srcAddr:	the concenter radio addr
// dstAddr:	the node radio addr
//***********************************************************************************
void ConcenterRadioSendSynTime(uint32_t srcAddr, uint32_t dstAddr)
{
	Calendar	calendarTemp;
    uint32_t temp;


	protocalTxBuf.command	= RADIO_PRO_CMD_SYN_TIME;
	protocalTxBuf.dstAddr	= dstAddr;
	protocalTxBuf.srcAddr	= srcAddr;
	protocalTxBuf.len 		= 10+14;


	calendarTemp			= Rtc_get_calendar();

	protocalTxBuf.load[0]	= (uint8_t)(calendarTemp.Year - 2000);
	protocalTxBuf.load[1]	= calendarTemp.Month;
	protocalTxBuf.load[2]	= calendarTemp.DayOfMonth;
	protocalTxBuf.load[3]	= calendarTemp.Hours;
	protocalTxBuf.load[4]	= calendarTemp.Minutes;
	protocalTxBuf.load[5]	= calendarTemp.Seconds;

	temp 					= g_rSysConfigInfo.collectPeriod;

	protocalTxBuf.load[6]	= (uint8_t)(temp >> 24);
	protocalTxBuf.load[7]	= (uint8_t)(temp >> 16);
	protocalTxBuf.load[8]	= (uint8_t)(temp >> 8);
	protocalTxBuf.load[9]	= (uint8_t)(temp);

	SetRadioDstAddr(dstAddr);


    RadioCopyPacketToBuf(((uint8_t*)&protocalTxBuf), protocalTxBuf.len, 0, 0, 0);
}


//***********************************************************************************
// brief:   send single shock event
// 
// parameter: 
// srcAddr:	the concenter radio addr
// dstAddr:	the node radio addr
//***********************************************************************************
void ConcenterRadioSendSingleShockEvt(uint32_t srcAddr, uint32_t dstAddr)
{



	protocalTxBuf.command	= RADIO_PRO_CMD_SINGEL_SHORCK_ACK;
	protocalTxBuf.dstAddr	= dstAddr;
	protocalTxBuf.srcAddr	= srcAddr;
	protocalTxBuf.len 		= 10;

	SetRadioDstAddr(dstAddr);

    RadioCopyPacketToBuf(((uint8_t*)&protocalTxBuf), protocalTxBuf.len, 0, 0, 0);
}


//***********************************************************************************
// brief:   send single shock stop event
// 
// parameter: 
// srcAddr:	the concenter radio addr
// dstAddr:	the node radio addr
//***********************************************************************************
void ConcenterRadioSendSingleShockStopEvt(uint32_t srcAddr, uint32_t dstAddr)
{



	protocalTxBuf.command	= RADIO_PRO_CMD_SINGEL_STOP_EVT;
	protocalTxBuf.dstAddr	= dstAddr;
	protocalTxBuf.srcAddr	= srcAddr;
	protocalTxBuf.len 		= 10;

	SetRadioDstAddr(dstAddr);

    RadioCopyPacketToBuf(((uint8_t*)&protocalTxBuf), protocalTxBuf.len, 0, 0, 0);
}

//***********************************************************************************
// brief:   send the groud num to node
// 
// parameter: 
// srcAddr:	the concenter radio addr
// dstAddr:	the node radio addr
//***********************************************************************************
void ConcenterRadioSendGroudSetEvt(uint32_t srcAddr, uint32_t dstAddr, uint32_t ground)
{



	protocalTxBuf.command	= RADIO_PRO_CMD_GROUD_SET_EVT;
	protocalTxBuf.dstAddr	= dstAddr;
	protocalTxBuf.srcAddr	= srcAddr;
	protocalTxBuf.len 		= 10+4;

	protocalTxBuf.load[0] 	= HIBYTE_ZKS(HIWORD_ZKS(ground));
    protocalTxBuf.load[1] 	= LOBYTE_ZKS(HIWORD_ZKS(ground));
    protocalTxBuf.load[2] 	= HIBYTE_ZKS(LOWORD_ZKS(ground));
    protocalTxBuf.load[3] 	= LOBYTE_ZKS(LOWORD_ZKS(ground));

	SetRadioDstAddr(dstAddr);

    RadioCopyPacketToBuf(((uint8_t*)&protocalTxBuf), protocalTxBuf.len, 0, 0, 0);
}



//***********************************************************************************
// brief:   send the groud num to node
// 
// parameter: 
// srcAddr:	the concenter radio addr
// dstAddr:	the node radio addr
//***********************************************************************************
void ConcenterRadioSendGroudShockEvt(uint32_t srcAddr, uint32_t dstAddr, uint32_t ground)
{



	protocalTxBuf.command	= RADIO_PRO_CMD_GROUD_SHORCK_EVT;
	protocalTxBuf.dstAddr	= dstAddr;
	protocalTxBuf.srcAddr	= srcAddr;
	protocalTxBuf.len 		= 10+4;

	protocalTxBuf.load[0] 	= HIBYTE_ZKS(HIWORD_ZKS(ground));
    protocalTxBuf.load[1] 	= LOBYTE_ZKS(HIWORD_ZKS(ground));
    protocalTxBuf.load[2] 	= HIBYTE_ZKS(LOWORD_ZKS(ground));
    protocalTxBuf.load[3] 	= LOBYTE_ZKS(LOWORD_ZKS(ground));

	SetRadioDstAddr(dstAddr);

    RadioCopyPacketToBuf(((uint8_t*)&protocalTxBuf), protocalTxBuf.len, 0, 0, 0);
}



//***********************************************************************************
// brief:   send the groud num to node
// 
// parameter: 
// srcAddr:	the concenter radio addr
// dstAddr:	the node radio addr
//***********************************************************************************
void ConcenterRadioSendGroudShockStopEvt(uint32_t srcAddr, uint32_t dstAddr, uint32_t ground)
{



	protocalTxBuf.command	= RADIO_PRO_CMD_GROUD_STOP_EVT;
	protocalTxBuf.dstAddr	= dstAddr;
	protocalTxBuf.srcAddr	= srcAddr;
	protocalTxBuf.len 		= 10+4;

	protocalTxBuf.load[0] 	= HIBYTE_ZKS(HIWORD_ZKS(ground));
    protocalTxBuf.load[1] 	= LOBYTE_ZKS(HIWORD_ZKS(ground));
    protocalTxBuf.load[2] 	= HIBYTE_ZKS(LOWORD_ZKS(ground));
    protocalTxBuf.load[3] 	= LOBYTE_ZKS(LOWORD_ZKS(ground));

	SetRadioDstAddr(dstAddr);

    RadioCopyPacketToBuf(((uint8_t*)&protocalTxBuf), protocalTxBuf.len, 0, 0, 0);
}

//***********************************************************************************
// brief:   send the groud num to node
// 
// parameter: 
// srcAddr:	the concenter radio addr
// dstAddr:	the node radio addr
//***********************************************************************************
void ConcenterRadioSendAck(uint32_t srcAddr, uint32_t dstAddr, uint16_t cmd)
{



	protocalTxBuf.command	= cmd;
	protocalTxBuf.dstAddr	= dstAddr;
	protocalTxBuf.srcAddr	= srcAddr;
	protocalTxBuf.len 		= 10;

	SetRadioDstAddr(dstAddr);

    RadioCopyPacketToBuf(((uint8_t*)&protocalTxBuf), protocalTxBuf.len, 0, 0, 0);
}

void GroudAddrSet(uint32_t ground)
{
	groundAddr = ground;
}

uint32_t GroudAddrGet(void)
{
	return groundAddr;
}

// 发送不需要回复的指令
void RadioCmdSetWithNoRes(uint16_t cmd)
{
	cmdType = cmd;
	cmdEvent |= (0x1 << cmd);
	RadioSingleSend();
}

// 清除不需要回复的指令
void RadioCmdClearWithNoRespon(void)
{
	uint8_t i;
	cmdEvent &= 0xffffffff ^ (0x1 << cmdType);
	cmdType = 0;
	if(cmdEvent){
		for(i = 0; i++; i < 16){
			if(cmdEvent & (0x1 << i)){
				cmdType = i;
				break;
			}
		}
		RadioSingleSend();
	}
}

uint16_t RadioWithNoResPack(void)
{
#ifdef BOARD_S3
	NodeRadioSendAck(cmdType);
#endif //BOARD_S3

#ifdef BOARD_S6_6
	ConcenterRadioSendAck(GetRadioSrcAddr(), GetRadioDstAddr(), cmdType);
#endif //BOARD_S6_6
}



// 发送不需要回复的群组指令，以广播的方式发出
void RadioCmdSetWithNoRes_Groud(uint16_t cmd)
{
	cmdTypeGroud = cmd;
	cmdEventGroud |= (0x1 << cmd);
	RadioSendBrocast();
}

// 清除不需要回复的群组指令
void RadioCmdClearWithNoRespon_Groud(void)
{
	uint8_t i;

	cmdEventGroud &= 0xffffffff ^ (0x1 << cmdType);
	cmdTypeGroud = 0;
	if(cmdEventGroud){
		for(i = 0; i++; i < 16){
			if(cmdEventGroud & (0x1 << i)){
				cmdTypeGroud = i;
				break;
			}
		}
		RadioSendBrocast();
	}
}

uint16_t RadioWithNoRes_GroudPack(void)
{
	switch(cmdTypeGroud){
		case RADIO_CMD_GROUD_SHOCK_TYPE:
		ConcenterRadioSendGroudShockEvt(GetRadioSrcAddr(), GetRadioDstAddr(), GroudAddrGet());
		break;

		case RADIO_CMD_GROUD_STOP_TYPE:
		ConcenterRadioSendGroudShockStopEvt(GetRadioSrcAddr(), GetRadioDstAddr(), GroudAddrGet());
		break;
	}
	return cmdTypeGroud;
}

uint16_t sendRetryTimes;
#define 		RETRY_TIMES		3

// 发送的需要回复命令
void RadioCmdSetWithRespon(uint16_t cmd, uint32_t dstAddr)
{
	cmdTypeWithRespon = cmd;
	cmdEventWithRespon |= (0x1 << cmd);
	if(dstAddr){
		SetRadioDstAddr(dstAddr);
	}
	sendRetryTimes = RETRY_TIMES;
	RadioSend();
}

// 清除需要回复的指令
void RadioCmdClearWithRespon(void)
{
	uint8_t i;
	if(sendRetryTimes == 0){
		cmdEventWithRespon &= 0xffffffff ^ (0x1 << cmdTypeWithRespon);
		cmdTypeWithRespon = 0;
		sendRetryTimes = RETRY_TIMES;

		if(cmdEventWithRespon){
			for(i = 0; i < 16; i++){
				if(cmdEventWithRespon & (0x1 << i)){
					cmdTypeWithRespon = i;
					break;
				}
			}
			RadioSend();
		}
	}
	else{
	    sendRetryTimes--;
		RadioSend();
	}
	
}

uint16_t RadioWithResPack(void)
{
	switch(cmdTypeWithRespon){
		case RADIO_CMD_SYN_TYPE:
		NodeRadioSendSynReq();
		break;
		case RADIO_CMD_LOW_VOL_TYPE:
		NodeRadioSendLowVolEvt();
		break;
		case RADIO_CMD_DESTROY_TYPE:
		NodeRadioSendDestroyEvt();
		break;
		case RADIO_CMD_INSERT_TYPE:
		NodeRadioSendInsertEvt();
		break;
		case RADIO_CMD_SINGLE_SHOCK_TYPE:
		ConcenterRadioSendSingleShockEvt(GetRadioSrcAddr(), GetRadioDstAddr());
		break;
		case RADIO_CMD_SINGLE_STOP_TYPE:
		ConcenterRadioSendSingleShockStopEvt(GetRadioSrcAddr(), GetRadioDstAddr());
		break;
		case RADIO_CMD_GROUD_SET_TYPE:
		ConcenterRadioSendGroudSetEvt(GetRadioSrcAddr(), GetRadioDstAddr(), GroudAddrGet());
		break;
	}
	return cmdTypeWithRespon;
}
