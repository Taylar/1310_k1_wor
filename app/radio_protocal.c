/*
* @Author: zxt
* @Date:   2017-12-26 16:36:20
* @Last Modified by:   zxt
* @Last Modified time: 2018-01-22 18:11:27
*/
#include "../general.h"

#include "../APP/nodeApp.h"
#include "../APP/systemApp.h"
#include "../APP/radio_protocal.h"
#include "../APP/concenterApp.h"
#include "../radio_app/node_strategy.h"
#include "../radio_app/radio_app.h"


radio_protocal_t   protocalTxBuf;

static uint8_t     concenterRemainderCache;

// //***********************************************************************************
// // brief:   set the node protocal event
// // 
// // parameter: 
// //***********************************************************************************
// void NodeProtocalEvtSet(EasyLink_RxPacket *rxPacket)
// {
// 	protocalRxPacket		= rxPacket;
//     Event_post(systemAppEvtHandle, SYSTEMAPP_EVT_RADIO_NODE);
// }



static void AlineRadio_protocal_tStruct(radio_protocal_t *adjust)
{
	uint8_t*  addrTemp;

	addrTemp = (uint8_t *)adjust + 2;
	memcpy(addrTemp, (uint8_t*)(&adjust->dstAddr), RADIO_PROTOCAL_LOAD_MAX +8);
	
}

static void AlineRadio_protocal_tStruct_Decompile(radio_protocal_t *adjust)
{
	uint8_t*  addrTemp;
	uint8_t i;

	addrTemp = (uint8_t *)adjust;

	for (i = 0; i < (RADIO_PROTOCAL_LOAD_MAX + 6); i++)
	{
		addrTemp[RADIO_PROTOCAL_LOAD_MAX + 8 - 1 - i] =  addrTemp[RADIO_PROTOCAL_LOAD_MAX + 8 - 3 - i];
	}
}


//***********************************************************************************
// brief:   analysis the node protocal 
// 
// parameter: 
//***********************************************************************************
void NodeProtocalDispath(EasyLink_RxPacket * protocalRxPacket)
{
	uint8_t len, lenTemp, baseAddr;
	uint32_t	temp, temp2, tickTemp;
	radio_protocal_t	*bufTemp;
    Calendar    calendarTemp;


	len			= protocalRxPacket->len;

	tickTemp    = Clock_getTicks();
    NodeStrategyReceiveReceiveSuccess();

	// this buf may be include several message
	bufTemp		= (radio_protocal_t *)protocalRxPacket->payload;

	AlineRadio_protocal_tStruct_Decompile(bufTemp);

	SetRadioDstAddr(bufTemp->srcAddr);

	while(len)
	{
		// the receive data is not integrated
		if((bufTemp->len > len) || (bufTemp->len == 0))
			goto NodeDispath;
		
		// the resever length
		len 	-= bufTemp->len;

		lenTemp = bufTemp->len;

		switch(bufTemp->command)
		{
			case RADIO_PRO_CMD_SENSOR_DATA:
			break;

			case RADIO_PRO_CMD_SYN_TIME:
			// get the concenter tick
			temp   =  ((uint32_t)bufTemp->load[6]) << 24;
			temp  |=  ((uint32_t)bufTemp->load[7]) << 16;
			temp  |=  ((uint32_t)bufTemp->load[8]) << 8;
			temp  |=  ((uint32_t)bufTemp->load[9]);

			// get the channel
			temp2  =  ((uint32_t)bufTemp->load[10]) << 24;
			temp2 |=  ((uint32_t)bufTemp->load[11]) << 16;
			temp2 |=  ((uint32_t)bufTemp->load[12]) << 8;
			temp2 |=  ((uint32_t)bufTemp->load[13]);

			NodeStrategySetOffset_Channel(temp, tickTemp, temp2);

			calendarTemp.Year       = 2000 + bufTemp->load[0];
			calendarTemp.Month      = bufTemp->load[1];
			calendarTemp.DayOfMonth = bufTemp->load[2];
			calendarTemp.Hours      = bufTemp->load[3];
			calendarTemp.Minutes    = bufTemp->load[4];
			calendarTemp.Seconds    = bufTemp->load[5];
			
			Rtc_set_calendar(&calendarTemp);
			NodeStopBroadcast();
			NodeCollectStart();
			NodeUploadStart();
			break;

			case RADIO_PRO_CMD_SYN_TIME_REQ:
			break;

			case RADIO_PRO_CMD_SET_PARA:
			// there may be several setting parameter
			while(lenTemp)
			{
				baseAddr	= bufTemp->len - lenTemp;
				switch(bufTemp->load[baseAddr])
				{
					case PARASETTING_COLLECT_INTERVAL:
					if(lenTemp < 5)
						goto NodeDispath;
					temp = ((uint32_t)bufTemp->load[baseAddr+4] << 24) + 
						   ((uint32_t)bufTemp->load[baseAddr+3] << 16) +
						   ((uint32_t)bufTemp->load[baseAddr+2] << 8) +
						   ((uint32_t)bufTemp->load[baseAddr+1]);
					lenTemp -= 5;
					NodeCollectStop();
					NodeCollectPeriodSet(temp * CLOCK_UNIT_S);
					if(temp)
						NodeCollectStart();
					break;

					case PARASETTING_UPLOAD_INTERVAL:
					if(lenTemp < 5)
						goto NodeDispath;
					temp = ((uint32_t)bufTemp->load[baseAddr+4] << 24) + 
						   ((uint32_t)bufTemp->load[baseAddr+3] << 16) +
						   ((uint32_t)bufTemp->load[baseAddr+2] << 8) +
						   ((uint32_t)bufTemp->load[baseAddr+1]);
					lenTemp -= 5;

					NodeUploadStop();
					NodeUploadPeriodSet(temp);
					NodeStrategySetPeriod(temp);
					if(temp)
					{
						NodeStopBroadcast();
						NodeUploadStart();
					}
					else
					{
						NodeStartBroadcast();
					}

					break;

					case PARASETTING_LOW_TEMP_ALARM:
					if(lenTemp < 4)
						goto NodeDispath;
					
					NodeLowTemperatureSet(bufTemp->load[baseAddr+1], 
						((uint16_t)bufTemp->load[baseAddr+2] << 8) + bufTemp->load[baseAddr+3]);

					lenTemp -= 4;
					break;

					case PARASETTING_HIGH_TEMP_ALARM:
					if(lenTemp < 4)
						goto NodeDispath;
					
					NodeHighTemperatureSet(bufTemp->load[baseAddr+1], 
						((uint16_t)bufTemp->load[baseAddr+2] << 8) + bufTemp->load[baseAddr+3]);

					lenTemp -= 4;
					break;

					default:
					// error setting parameter
					goto NodeDispath;
				}
			}
				
			break;

			case RADIO_PRO_CMD_SET_PARA_ACK:
			break;

			case RADIO_PRO_CMD_ACK:
			// get the concenter tick
			temp   =  ((uint32_t)bufTemp->load[1]) << 24;
			temp  |=  ((uint32_t)bufTemp->load[2]) << 16;
			temp  |=  ((uint32_t)bufTemp->load[3]) << 8;
			temp  |=  ((uint32_t)bufTemp->load[4]);

			// get the channel
			temp2  =  ((uint32_t)bufTemp->load[5]) << 24;
			temp2 |=  ((uint32_t)bufTemp->load[6]) << 16;
			temp2 |=  ((uint32_t)bufTemp->load[7]) << 8;
			temp2 |=  ((uint32_t)bufTemp->load[8]);

			NodeStrategySetOffset_Channel(temp, tickTemp, temp2);

			if(bufTemp->load[0] == PROTOCAL_FAIL)
				NodeUploadFailProcess();
			else
				NodeUploadSucessProcess();
			
			break;

			default:
			goto NodeDispath;

		}

		// point to new message the head
		bufTemp		= (radio_protocal_t *)(protocalRxPacket->payload + bufTemp->len);
		AlineRadio_protocal_tStruct_Decompile(bufTemp);
	}

NodeDispath:
	NodeBroadcasting();
}




// //***********************************************************************************
// // brief:   set the Concenter protocal event
// // 
// // parameter: 
// //***********************************************************************************
// void ConcenterProtocalEvtSet(EasyLink_RxPacket *rxPacket)
// {
// 	protocalRxPacket		= rxPacket;
//     Event_post(systemAppEvtHandle, SYSTEMAPP_EVT_RADIO_CONCENTER);
// }



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
	AlineRadio_protocal_tStruct_Decompile(bufTemp);

	SetRadioDstAddr(bufTemp->srcAddr);
    ConcenterSaveChannel(bufTemp->srcAddr);

	while(len)
	{
		// the receive data is not integrated
		if((bufTemp->len > len) || (bufTemp->len == 0))
			return;
		
		len 	-= bufTemp->len;

		switch(bufTemp->command)
		{
			// receive the sensor data, save to flash
			case RADIO_PRO_CMD_SENSOR_DATA:

			// there may be several sensordata

			// save the data to flash
			// updata the rssi
			bufTemp->load[1]		= (uint8_t)(protocalRxPacket->rssi);

			if(ConcenterSensorDataSaveToQueue(bufTemp->load, bufTemp->load[0]+1) == true)
			{
				ConcenterRadioSendSensorDataAck(GetRadioSrcAddr(), GetRadioDstAddr(), ES_SUCCESS);
			}
			else
			{
				ConcenterRadioSendSensorDataAck(GetRadioSrcAddr(), GetRadioDstAddr(), ES_ERROR);
			}

			break;


			case RADIO_PRO_CMD_SYN_TIME_REQ:


			// send the time
			if(ConcenterReadSynTimeFlag())
				ConcenterRadioSendSynTime(bufTemp->dstAddr, bufTemp->srcAddr);

			break;


			case RADIO_PRO_CMD_SYN_TIME:
			break;


			case RADIO_PRO_CMD_SET_PARA:

			break;


			case RADIO_PRO_CMD_SET_PARA_ACK:

			break;


			case RADIO_PRO_CMD_ACK:

			break;
		}
		// point to new message the head
		bufTemp		= (radio_protocal_t *)(protocalRxPacket->payload + bufTemp->len);
		AlineRadio_protocal_tStruct_Decompile(bufTemp);
	}

	// receive several cmd in one radio packet, must return in one radio packet;
	RadioSend();
}



//***********************************************************************************
// brief:   send the sensor data to the strategy process
// 
// parameter: 
// dataP:	sensor data point
// length:	the sensor data length
//***********************************************************************************
bool NodeRadioSendSensorData(uint8_t * dataP, uint8_t length)
{
	if(length > RADIO_PROTOCAL_LOAD_MAX)
		return false;

	protocalTxBuf.len 		= length + 10;
	protocalTxBuf.dstAddr	= GetRadioDstAddr();
	protocalTxBuf.srcAddr	= GetRadioSrcAddr();

	// the remainderCache is not satisfy length
	if(NodeStrategyRemainderCache() < protocalTxBuf.len)
		return false;


	protocalTxBuf.command	= RADIO_PRO_CMD_SENSOR_DATA;

	memcpy(protocalTxBuf.load, dataP, length);

	AlineRadio_protocal_tStruct(&protocalTxBuf);

	NodeStrategySendPacket((uint8_t*)&protocalTxBuf, protocalTxBuf.len);
	return true;
}


//***********************************************************************************
// brief:   send the timesyn ack to the strategy process
// 
// parameter: 
//***********************************************************************************
void NodeRadioSendSynReq(void)
{
	protocalTxBuf.len = 10;
	protocalTxBuf.dstAddr	= GetRadioDstAddr();
	protocalTxBuf.srcAddr	= GetRadioSrcAddr();

	// the remainderCache is not satisfy length
	if(NodeStrategyRemainderCache() < protocalTxBuf.len)
		return ;

	protocalTxBuf.command	= RADIO_PRO_CMD_SYN_TIME_REQ;


	AlineRadio_protocal_tStruct(&protocalTxBuf);

	NodeStrategySendPacket((uint8_t*)&protocalTxBuf, protocalTxBuf.len);
}



//***********************************************************************************
// brief:   send the parameter setting ack to the strategy process
// 
// parameter: 
// status:	success or fail
//***********************************************************************************
void NodeRadioSendParaSetAck(ErrorStatus status)
{
	protocalTxBuf.command	= RADIO_PRO_CMD_SET_PARA_ACK;
	protocalTxBuf.dstAddr	= GetRadioDstAddr();
	protocalTxBuf.srcAddr	= GetRadioSrcAddr();
	protocalTxBuf.len = 10 + 1;

	// the remainderCache is not satisfy length
	if(NodeStrategyRemainderCache() < protocalTxBuf.len)
		return ;

	protocalTxBuf.load[0]		= (uint8_t)status;

	AlineRadio_protocal_tStruct(&protocalTxBuf);

	NodeStrategySendPacket((uint8_t*)&protocalTxBuf, protocalTxBuf.len);
}


//***********************************************************************************
// brief:   send the sensor data receive result to the node immediately
// 
// parameter: 
// srcAddr:	the concenter radio addr
// dstAddr:	the node radio addr
// status:	success or fail
//***********************************************************************************
void ConcenterRadioSendSensorDataAck(uint32_t srcAddr, uint32_t dstAddr, ErrorStatus status)
{
	uint32_t temp;

	protocalTxBuf.command	= RADIO_PRO_CMD_ACK;
	protocalTxBuf.dstAddr	= dstAddr;
	protocalTxBuf.srcAddr	= srcAddr;
	protocalTxBuf.len 		= 10 + 9;



	temp = ConcenterReadChannel(dstAddr);

	protocalTxBuf.load[5]	= (uint8_t)(temp >> 24);
	protocalTxBuf.load[6]	= (uint8_t)(temp >> 16);
	protocalTxBuf.load[7]	= (uint8_t)(temp >> 8);
	protocalTxBuf.load[8]	= (uint8_t)(temp);

	temp = Clock_getTicks();

	protocalTxBuf.load[0]	= (uint8_t)(status);
	protocalTxBuf.load[1]	= (uint8_t)(temp >> 24);
	protocalTxBuf.load[2]	= (uint8_t)(temp >> 16);
	protocalTxBuf.load[3]	= (uint8_t)(temp >> 8);
	protocalTxBuf.load[4]	= (uint8_t)(temp);



	SetRadioDstAddr(dstAddr);

	AlineRadio_protocal_tStruct(&protocalTxBuf);

    RadioCopyPacketToBuf(((uint8_t*)&protocalTxBuf), protocalTxBuf.len, 0, 0, EASYLINK_MAX_DATA_LENGTH - concenterRemainderCache);
    concenterRemainderCache -= protocalTxBuf.len;
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

	temp 					= ConcenterReadChannel(dstAddr);
	protocalTxBuf.load[10]	= (uint8_t)(temp >> 24);
	protocalTxBuf.load[11]	= (uint8_t)(temp >> 16);
	protocalTxBuf.load[12]	= (uint8_t)(temp >> 8);
	protocalTxBuf.load[13]	= (uint8_t)(temp);



	temp 					= Clock_getTicks();

	protocalTxBuf.load[6]	= (uint8_t)(temp >> 24);
	protocalTxBuf.load[7]	= (uint8_t)(temp >> 16);
	protocalTxBuf.load[8]	= (uint8_t)(temp >> 8);
	protocalTxBuf.load[9]	= (uint8_t)(temp);


	SetRadioDstAddr(dstAddr);

	AlineRadio_protocal_tStruct(&protocalTxBuf);

    RadioCopyPacketToBuf(((uint8_t*)&protocalTxBuf), protocalTxBuf.len, 0, 0, EASYLINK_MAX_DATA_LENGTH - concenterRemainderCache);
    concenterRemainderCache -= protocalTxBuf.len;
}



//***********************************************************************************
// brief:   send the node parameter setting immediately
// 
// parameter: 
// srcAddr:	the concenter radio addr
// dstAddr:	the node radio addr
// dataP:	the setting parameter point
// length:	the setting parameter length
//***********************************************************************************
void ConcenterRadioSendParaSet(uint32_t srcAddr, uint32_t dstAddr, uint8_t *dataP, uint8_t length)
{
	if(length > RADIO_PROTOCAL_LOAD_MAX)
		return;

	protocalTxBuf.command	= RADIO_PRO_CMD_SET_PARA;
	protocalTxBuf.dstAddr	= dstAddr;
	protocalTxBuf.srcAddr	= srcAddr;
	protocalTxBuf.len 		= 16;

	memcpy(protocalTxBuf.load, dataP, length);

	SetRadioDstAddr(dstAddr);

	AlineRadio_protocal_tStruct(&protocalTxBuf);

    RadioCopyPacketToBuf(((uint8_t*)&protocalTxBuf), protocalTxBuf.len, 0, 0, EASYLINK_MAX_DATA_LENGTH - concenterRemainderCache);
    concenterRemainderCache -= protocalTxBuf.len;
}
