/*
* @Author: zxt
* @Date:   2017-12-26 16:36:20
* @Last Modified by:   zxt
* @Last Modified time: 2018-03-15 14:14:47
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
					temp = ((uint32_t)bufTemp->load[baseAddr+1] << 24) + 
						   ((uint32_t)bufTemp->load[baseAddr+2] << 16) +
						   ((uint32_t)bufTemp->load[baseAddr+3] << 8) +
						   ((uint32_t)bufTemp->load[baseAddr+4]);
					lenTemp -= 5;
					NodeCollectStop();
					g_rSysConfigInfo.collectPeriod = temp;
					
					break;

					case PARASETTING_UPLOAD_INTERVAL:
					if(lenTemp < 5)
						goto NodeDispath;
					temp = ((uint32_t)bufTemp->load[baseAddr+1] << 24) + 
						   ((uint32_t)bufTemp->load[baseAddr+2] << 16) +
						   ((uint32_t)bufTemp->load[baseAddr+3] << 8) +
						   ((uint32_t)bufTemp->load[baseAddr+4]);
					lenTemp -= 5;

					NodeUploadStop();
					g_rSysConfigInfo.uploadPeriod = temp;
					NodeStrategySetPeriod(temp);

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


					case PARASETTING_DECEIVE_ID:
					if(lenTemp < 5)
						goto NodeDispath;
					g_rSysConfigInfo.DeviceId[0] = bufTemp->load[baseAddr+1];
					g_rSysConfigInfo.DeviceId[1] = bufTemp->load[baseAddr+2];
					g_rSysConfigInfo.DeviceId[2] = bufTemp->load[baseAddr+3];
    				g_rSysConfigInfo.DeviceId[3] = bufTemp->load[baseAddr+4];

    				SetRadioSrcAddr( (((uint32_t)(g_rSysConfigInfo.DeviceId[0])) << 24) |
				                     (((uint32_t)(g_rSysConfigInfo.DeviceId[1])) << 16) |
				                     (((uint32_t)(g_rSysConfigInfo.DeviceId[2])) << 8) |
				                     g_rSysConfigInfo.DeviceId[3]);

					lenTemp -= 5;
					break;

					case PARASETTING_CUSTOM_ID:
					if(lenTemp < 3)
						goto NodeDispath;
    				g_rSysConfigInfo.customId[0] = bufTemp->load[baseAddr+1];
    				g_rSysConfigInfo.customId[1] = bufTemp->load[baseAddr+2];

    				NodeSetCustomId(0xffff0000 | (g_rSysConfigInfo.customId[0] << 8) | g_rSysConfigInfo.customId[1]);
					lenTemp -= 3;
					
					break;

					case PARASETTING_NTC_INTERVAL:
					if(lenTemp < 5)
						goto NodeDispath;
					g_rSysConfigInfo.ntpPeriod =	((uint32_t)bufTemp->load[baseAddr+1] << 24) + 
													((uint32_t)bufTemp->load[baseAddr+2] << 16) +
													((uint32_t)bufTemp->load[baseAddr+3] << 8) +
													((uint32_t)bufTemp->load[baseAddr+4]);
					lenTemp -= 5;
					break;

					case PARASETTING_LOW_VOLTAGE:
					if(lenTemp < 3)
						goto NodeDispath;
					g_rSysConfigInfo.batLowVol    = ((uint32_t)bufTemp->load[baseAddr+1] << 8) + 
													((uint32_t)bufTemp->load[baseAddr+2]);
					lenTemp -= 3;
					break;

					case PARASETTING_SOFT_VER:
					if(lenTemp < 3)
						goto NodeDispath;
					g_rSysConfigInfo.swVersion    = ((uint32_t)FW_VERSION << 8) + 
													((uint32_t)FW_VERSION);
					lenTemp -= 3;
					break;

					case PARASETTING_RF_PARA:
					if(lenTemp < 5)
						goto NodeDispath;
					g_rSysConfigInfo.rfPA    	= bufTemp->load[baseAddr+1];
					g_rSysConfigInfo.rfBW    	= bufTemp->load[baseAddr+2];
					g_rSysConfigInfo.rfSF    	= bufTemp->load[baseAddr+3];
					g_rSysConfigInfo.rfStatus 	= bufTemp->load[baseAddr+4];
					lenTemp -= 5;
					break;

					case PARASETTING_MODULE_SET:
					if(lenTemp < 5)
						goto NodeDispath;
					g_rSysConfigInfo.status    	=	((uint32_t)bufTemp->load[baseAddr+1] << 8) + 
													((uint32_t)bufTemp->load[baseAddr+2]);
					g_rSysConfigInfo.module    	=	((uint32_t)bufTemp->load[baseAddr+3] << 8) + 
													((uint32_t)bufTemp->load[baseAddr+4]);
					lenTemp -= 5;
					break;

					default:
					// error setting parameter
					InternalFlashStoreConfig();
					goto NodeDispath;
				}
			}
			InternalFlashStoreConfig();	
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

			// resever more 6 package and is the last ack
			if((Flash_get_unupload_items() > 6) && (len < 19))
			{
				NodeUploadProcess();
				RadioSend();
			}
			break;


			default:
			goto NodeDispath;

		}

		// point to new message the head
		bufTemp		= (radio_protocal_t *)(protocalRxPacket->payload + bufTemp->len);
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
	uint8_t len, lenTemp, baseAddr;
	uint32_t	temp;
	radio_protocal_t	*bufTemp;

	concenterRemainderCache = EASYLINK_MAX_DATA_LENGTH;
	len                     = protocalRxPacket->len;
	bufTemp                 = (radio_protocal_t *)protocalRxPacket->payload;

	ConcenterRadioMonitorClear();
	
	ClearRadioSendBuf();
	
	SetRadioDstAddr(bufTemp->srcAddr);
    ConcenterSaveChannel(bufTemp->srcAddr);


	while(len)
	{
		// the receive data is not integrated
		if((bufTemp->len > len) || (bufTemp->len == 0))
			return;
		
		len 	-= bufTemp->len;
		lenTemp = bufTemp->len;

		switch(bufTemp->command)
		{
			// receive the sensor data, save to flash
			case RADIO_PRO_CMD_SENSOR_DATA:

			// there may be several sensordata

			// save the data to flash
			// updata the rssi
			bufTemp->load[1]		= (uint8_t)(protocalRxPacket->rssi);

			sensor_unpackage_to_memory(bufTemp->load, bufTemp->load[0]+1);
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

			case RADIO_PRO_CMD_RESPOND_PARA:
			while(lenTemp)
			{
				baseAddr	= bufTemp->len - lenTemp;
				switch(bufTemp->load[baseAddr])
				{
					case PARASETTING_COLLECT_INTERVAL:
					if(lenTemp < 5)
						goto ConcenterConfigRespondEnd;
					temp = ((uint32_t)bufTemp->load[baseAddr+1] << 24) + 
						   ((uint32_t)bufTemp->load[baseAddr+2] << 16) +
						   ((uint32_t)bufTemp->load[baseAddr+3] << 8) +
						   ((uint32_t)bufTemp->load[baseAddr+4]);
					lenTemp -= 5;
					g_rSysConfigInfo.collectPeriod = temp;
					break;

					case PARASETTING_UPLOAD_INTERVAL:
					if(lenTemp < 5)
						goto ConcenterConfigRespondEnd;
					temp = ((uint32_t)bufTemp->load[baseAddr+1] << 24) + 
						   ((uint32_t)bufTemp->load[baseAddr+2] << 16) +
						   ((uint32_t)bufTemp->load[baseAddr+3] << 8) +
						   ((uint32_t)bufTemp->load[baseAddr+4]);
					lenTemp -= 5;

					g_rSysConfigInfo.uploadPeriod = temp;
					break;

					case PARASETTING_LOW_TEMP_ALARM:
					if(lenTemp < 4)
						goto ConcenterConfigRespondEnd;
					
					lenTemp -= 4;
					break;

					case PARASETTING_HIGH_TEMP_ALARM:
					if(lenTemp < 4)
						goto ConcenterConfigRespondEnd;
					
					lenTemp -= 4;
					break;


					case PARASETTING_DECEIVE_ID:
					if(lenTemp < 5)
						goto ConcenterConfigRespondEnd;
					g_rSysConfigInfo.DeviceId[0] = bufTemp->load[baseAddr+1];
					g_rSysConfigInfo.DeviceId[1] = bufTemp->load[baseAddr+2];
					g_rSysConfigInfo.DeviceId[2] = bufTemp->load[baseAddr+3];
    				g_rSysConfigInfo.DeviceId[3] = bufTemp->load[baseAddr+4];


					lenTemp -= 5;
					break;

					case PARASETTING_CUSTOM_ID:
					if(lenTemp < 3)
						goto ConcenterConfigRespondEnd;
    				g_rSysConfigInfo.customId[0] = bufTemp->load[baseAddr+1];
    				g_rSysConfigInfo.customId[1] = bufTemp->load[baseAddr+2];

					lenTemp -= 3;
					
					break;

					case PARASETTING_NTC_INTERVAL:
					if(lenTemp < 5)
						goto ConcenterConfigRespondEnd;
					g_rSysConfigInfo.ntpPeriod =	((uint32_t)bufTemp->load[baseAddr+1] << 24) + 
													((uint32_t)bufTemp->load[baseAddr+2] << 16) +
													((uint32_t)bufTemp->load[baseAddr+3] << 8) +
													((uint32_t)bufTemp->load[baseAddr+4]);
					lenTemp -= 5;
					break;

					case PARASETTING_LOW_VOLTAGE:
					if(lenTemp < 3)
						goto ConcenterConfigRespondEnd;
					g_rSysConfigInfo.batLowVol	=	((uint32_t)bufTemp->load[baseAddr+1] << 8) + 
													((uint32_t)bufTemp->load[baseAddr+2]);
					lenTemp -= 3;
					break;

					case PARASETTING_SOFT_VER:
					if(lenTemp < 3)
						goto ConcenterConfigRespondEnd;
					g_rSysConfigInfo.swVersion    = ((uint32_t)bufTemp->load[baseAddr+1] << 8) + 
													((uint32_t)bufTemp->load[baseAddr+2]);
					lenTemp -= 3;
					break;

					case PARASETTING_RF_PARA:
					if(lenTemp < 5)
						goto ConcenterConfigRespondEnd;
					g_rSysConfigInfo.rfPA    	= bufTemp->load[baseAddr+1];
					g_rSysConfigInfo.rfBW    	= bufTemp->load[baseAddr+2];
					g_rSysConfigInfo.rfSF    	= bufTemp->load[baseAddr+3];
					g_rSysConfigInfo.rfStatus 	= bufTemp->load[baseAddr+4];
					lenTemp -= 5;
					break;

					case PARASETTING_MODULE_SET:
					if(lenTemp < 5)
						goto ConcenterConfigRespondEnd;
					g_rSysConfigInfo.status    	=	((uint32_t)bufTemp->load[baseAddr+1] << 8) + 
													((uint32_t)bufTemp->load[baseAddr+2]);
					g_rSysConfigInfo.module    	=	((uint32_t)bufTemp->load[baseAddr+3] << 8) + 
													((uint32_t)bufTemp->load[baseAddr+4]);
					lenTemp -= 5;
					break;

					default:
					// error setting parameter
					goto ConcenterConfigRespondEnd;
				}
			}
ConcenterConfigRespondEnd:
#ifdef BOARD_CONFIG_DECEIVE
			UsbSend(AC_Send_Config);
#endif
			break;


			case RADIO_PRO_CMD_RESPOND_PARA_ACK:

			break;


			case RADIO_PRO_CMD_ACK:

			break;
		}
		// point to new message the head
		bufTemp		= (radio_protocal_t *)((uint8_t *)bufTemp + bufTemp->len);
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

	// the remainderCache is not satisfy length,15 is reverse for time syschro for bracasting
	if(NodeStrategyRemainderCache() < (protocalTxBuf.len + 15))
		return false;


	protocalTxBuf.command	= RADIO_PRO_CMD_SENSOR_DATA;

	memcpy(protocalTxBuf.load, dataP, length);


	return NodeStrategySendPacket((uint8_t*)&protocalTxBuf, protocalTxBuf.len);
}


//***********************************************************************************
// brief:   send the timesyn ack to the strategy process
// 
// parameter: 
//***********************************************************************************
bool NodeRadioSendSynReq(void)
{
	protocalTxBuf.len = 10;
	protocalTxBuf.dstAddr	= GetRadioDstAddr();
	protocalTxBuf.srcAddr	= GetRadioSrcAddr();

	// the remainderCache is not satisfy length
	if(NodeStrategyRemainderCache() < protocalTxBuf.len)
		return false;

	protocalTxBuf.command	= RADIO_PRO_CMD_SYN_TIME_REQ;



	return NodeStrategySendPacket((uint8_t*)&protocalTxBuf, protocalTxBuf.len);
}



//***********************************************************************************
// brief:   send the parameter setting ack to the strategy process
// 
// parameter: 
// status:	success or fail
//***********************************************************************************
bool NodeRadioSendParaSetAck(ErrorStatus status)
{
	protocalTxBuf.command	= RADIO_PRO_CMD_SET_PARA_ACK;
	protocalTxBuf.dstAddr	= GetRadioDstAddr();
	protocalTxBuf.srcAddr	= GetRadioSrcAddr();
	protocalTxBuf.len = 10 + 1;

	// the remainderCache is not satisfy length
	if(NodeStrategyRemainderCache() < protocalTxBuf.len)
		return false;

	protocalTxBuf.load[0]		= (uint8_t)status;


	return NodeStrategySendPacket((uint8_t*)&protocalTxBuf, protocalTxBuf.len);
}

//***********************************************************************************
// brief:   send the parameter setting to the config deceive
// 
// parameter: 
// status:	success or fail
//***********************************************************************************
bool NodeRadioSendConfig(void)
{
	uint8_t temp;
	temp = 0;

	protocalTxBuf.command	= RADIO_PRO_CMD_RESPOND_PARA;
	protocalTxBuf.dstAddr	= GetRadioDstAddr();
	protocalTxBuf.srcAddr	= GetRadioSrcAddr();


	protocalTxBuf.load[temp++]		= PARASETTING_COLLECT_INTERVAL;
	protocalTxBuf.load[temp++]		= (uint8_t)(g_rSysConfigInfo.collectPeriod>>24);
	protocalTxBuf.load[temp++]		= (uint8_t)(g_rSysConfigInfo.collectPeriod>>16);
	protocalTxBuf.load[temp++]		= (uint8_t)(g_rSysConfigInfo.collectPeriod>>8);
	protocalTxBuf.load[temp++]		= (uint8_t)(g_rSysConfigInfo.collectPeriod);


	protocalTxBuf.load[temp++]		= PARASETTING_UPLOAD_INTERVAL;
	protocalTxBuf.load[temp++]		= (uint8_t)(g_rSysConfigInfo.uploadPeriod>>24);
	protocalTxBuf.load[temp++]		= (uint8_t)(g_rSysConfigInfo.uploadPeriod>>16);
	protocalTxBuf.load[temp++]		= (uint8_t)(g_rSysConfigInfo.uploadPeriod>>8);
	protocalTxBuf.load[temp++]		= (uint8_t)(g_rSysConfigInfo.uploadPeriod);


	// protocalTxBuf.load[temp++]		= PARASETTING_LOW_TEMP_ALARM;
	// protocalTxBuf.load[temp++]		= 0;
	// protocalTxBuf.load[temp++]		= 0x7f;
	// protocalTxBuf.load[temp++]		= 0xff;


	// protocalTxBuf.load[temp++]		= PARASETTING_HIGH_TEMP_ALARM;
	// protocalTxBuf.load[temp++]		= 0;
	// protocalTxBuf.load[temp++]		= 0x7f;
	// protocalTxBuf.load[temp++]		= 0xff;

	protocalTxBuf.load[temp++]		= PARASETTING_DECEIVE_ID;
	protocalTxBuf.load[temp++]		= g_rSysConfigInfo.DeviceId[0];
	protocalTxBuf.load[temp++]		= g_rSysConfigInfo.DeviceId[1];
	protocalTxBuf.load[temp++]		= g_rSysConfigInfo.DeviceId[2];
	protocalTxBuf.load[temp++]		= g_rSysConfigInfo.DeviceId[3];

	protocalTxBuf.load[temp++]		= PARASETTING_CUSTOM_ID;
	protocalTxBuf.load[temp++]		= g_rSysConfigInfo.customId[0];
	protocalTxBuf.load[temp++]		= g_rSysConfigInfo.customId[1];

	protocalTxBuf.load[temp++]		= PARASETTING_NTC_INTERVAL;
	protocalTxBuf.load[temp++]		= (uint8_t)(g_rSysConfigInfo.ntpPeriod>>24);
	protocalTxBuf.load[temp++]		= (uint8_t)(g_rSysConfigInfo.ntpPeriod>>16);
	protocalTxBuf.load[temp++]		= (uint8_t)(g_rSysConfigInfo.ntpPeriod>>8);
	protocalTxBuf.load[temp++]		= (uint8_t)(g_rSysConfigInfo.ntpPeriod);


	protocalTxBuf.load[temp++]		= PARASETTING_LOW_VOLTAGE;
	protocalTxBuf.load[temp++]		= (uint8_t)(g_rSysConfigInfo.batLowVol>>8);
	protocalTxBuf.load[temp++]		= (uint8_t)(g_rSysConfigInfo.batLowVol);


	protocalTxBuf.load[temp++]		= PARASETTING_SOFT_VER;
	protocalTxBuf.load[temp++]		= (uint8_t)(FW_VERSION>>8);
	protocalTxBuf.load[temp++]		= (uint8_t)(FW_VERSION);

	protocalTxBuf.load[temp++]		= PARASETTING_RF_PARA;
	protocalTxBuf.load[temp++]		= (uint8_t)(g_rSysConfigInfo.rfPA);
	protocalTxBuf.load[temp++]		= (uint8_t)(g_rSysConfigInfo.rfBW);
	protocalTxBuf.load[temp++]		= (uint8_t)(g_rSysConfigInfo.rfSF);
	protocalTxBuf.load[temp++]		= (uint8_t)(g_rSysConfigInfo.rfStatus);

	protocalTxBuf.load[temp++]		= PARASETTING_MODULE_SET;
	protocalTxBuf.load[temp++]		= (uint8_t)(g_rSysConfigInfo.status>>8);
	protocalTxBuf.load[temp++]		= (uint8_t)(g_rSysConfigInfo.status);
	protocalTxBuf.load[temp++]		= (uint8_t)(g_rSysConfigInfo.module>>8);
	protocalTxBuf.load[temp++]		= (uint8_t)(g_rSysConfigInfo.module);

	protocalTxBuf.len = 10 + temp;

	RadioSendPacket((uint8_t*)&protocalTxBuf, protocalTxBuf.len, 0, 0);
	
	return true;
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
void ConcenterRadioSendParaSet(uint32_t srcAddr, uint32_t dstAddr)
{
    uint8_t temp;
	temp = 0;


	protocalTxBuf.command	= RADIO_PRO_CMD_SET_PARA;
	protocalTxBuf.dstAddr	= dstAddr;
	protocalTxBuf.srcAddr	= srcAddr;



	protocalTxBuf.load[temp++]		= PARASETTING_COLLECT_INTERVAL;
	protocalTxBuf.load[temp++]		= (uint8_t)(g_rSysConfigInfo.collectPeriod>>24);
	protocalTxBuf.load[temp++]		= (uint8_t)(g_rSysConfigInfo.collectPeriod>>16);
	protocalTxBuf.load[temp++]		= (uint8_t)(g_rSysConfigInfo.collectPeriod>>8);
	protocalTxBuf.load[temp++]		= (uint8_t)(g_rSysConfigInfo.collectPeriod);


	protocalTxBuf.load[temp++]		= PARASETTING_UPLOAD_INTERVAL;
	protocalTxBuf.load[temp++]		= (uint8_t)(g_rSysConfigInfo.uploadPeriod>>24);
	protocalTxBuf.load[temp++]		= (uint8_t)(g_rSysConfigInfo.uploadPeriod>>16);
	protocalTxBuf.load[temp++]		= (uint8_t)(g_rSysConfigInfo.uploadPeriod>>8);
	protocalTxBuf.load[temp++]		= (uint8_t)(g_rSysConfigInfo.uploadPeriod);


	// protocalTxBuf.load[temp++]		= PARASETTING_LOW_TEMP_ALARM;
	// protocalTxBuf.load[temp++]		= 0;
	// protocalTxBuf.load[temp++]		= 0x7f;
	// protocalTxBuf.load[temp++]		= 0xff;


	// protocalTxBuf.load[temp++]		= PARASETTING_HIGH_TEMP_ALARM;
	// protocalTxBuf.load[temp++]		= 0;
	// protocalTxBuf.load[temp++]		= 0x7f;
	// protocalTxBuf.load[temp++]		= 0xff;

	protocalTxBuf.load[temp++]		= PARASETTING_DECEIVE_ID;
	protocalTxBuf.load[temp++]		= g_rSysConfigInfo.DeviceId[0];
	protocalTxBuf.load[temp++]		= g_rSysConfigInfo.DeviceId[1];
	protocalTxBuf.load[temp++]		= g_rSysConfigInfo.DeviceId[2];
	protocalTxBuf.load[temp++]		= g_rSysConfigInfo.DeviceId[3];

	protocalTxBuf.load[temp++]		= PARASETTING_CUSTOM_ID;
	protocalTxBuf.load[temp++]		= g_rSysConfigInfo.customId[0];
	protocalTxBuf.load[temp++]		= g_rSysConfigInfo.customId[1];

	protocalTxBuf.load[temp++]		= PARASETTING_NTC_INTERVAL;
	protocalTxBuf.load[temp++]		= (uint8_t)(g_rSysConfigInfo.ntpPeriod>>24);
	protocalTxBuf.load[temp++]		= (uint8_t)(g_rSysConfigInfo.ntpPeriod>>16);
	protocalTxBuf.load[temp++]		= (uint8_t)(g_rSysConfigInfo.ntpPeriod>>8);
	protocalTxBuf.load[temp++]		= (uint8_t)(g_rSysConfigInfo.ntpPeriod);


	protocalTxBuf.load[temp++]		= PARASETTING_LOW_VOLTAGE;
	protocalTxBuf.load[temp++]		= (uint8_t)(g_rSysConfigInfo.batLowVol>>8);
	protocalTxBuf.load[temp++]		= (uint8_t)(g_rSysConfigInfo.batLowVol);

	protocalTxBuf.load[temp++]		= PARASETTING_SOFT_VER;
	protocalTxBuf.load[temp++]		= (uint8_t)(g_rSysConfigInfo.swVersion>>8);
	protocalTxBuf.load[temp++]		= (uint8_t)(g_rSysConfigInfo.swVersion);

	protocalTxBuf.load[temp++]		= PARASETTING_RF_PARA;
	protocalTxBuf.load[temp++]		= (uint8_t)(g_rSysConfigInfo.rfPA);
	protocalTxBuf.load[temp++]		= (uint8_t)(g_rSysConfigInfo.rfBW);
	protocalTxBuf.load[temp++]		= (uint8_t)(g_rSysConfigInfo.rfSF);
	protocalTxBuf.load[temp++]		= (uint8_t)(g_rSysConfigInfo.rfStatus);

	protocalTxBuf.load[temp++]		= PARASETTING_MODULE_SET;
	protocalTxBuf.load[temp++]		= (uint8_t)(g_rSysConfigInfo.status>>8);
	protocalTxBuf.load[temp++]		= (uint8_t)(g_rSysConfigInfo.status);
	protocalTxBuf.load[temp++]		= (uint8_t)(g_rSysConfigInfo.module>>8);
	protocalTxBuf.load[temp++]		= (uint8_t)(g_rSysConfigInfo.module);

	protocalTxBuf.len = 10 + temp;

	RadioSendPacket((uint8_t*)&protocalTxBuf, protocalTxBuf.len, 0, 0);
}
