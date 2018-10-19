/*
* @Author: zxt
* @Date:   2017-12-26 16:36:20
* @Last Modified by:   zxt
* @Last Modified time: 2018-10-19 11:48:21
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


//***********************************************************************************
// brief:   analysis the node protocal 
// 
// parameter: 
//***********************************************************************************
void NodeProtocalDispath(EasyLink_RxPacket * protocalRxPacket)
{
	uint8_t len, lenTemp, baseAddr, flag;
	uint32_t	temp;
	radio_protocal_t	*bufTemp;
    Calendar    calendarTemp;
    uint16_t serialNum;
#ifdef  SUPPORT_STRATEGY_SORT
    uint8_t setChannel = 0;
    uint32_t    temp2;
#endif // SUPPORT_STRATEGY_SORT

    TxFrameRecord_t rxSensorDataAckRecord;
    memset(&rxSensorDataAckRecord, 0, sizeof(TxFrameRecord_t));



	len			= protocalRxPacket->len;

    NodeStrategyBuffClear();

	// this buf may be include several message
	bufTemp		= (radio_protocal_t *)protocalRxPacket->payload;

	SetRadioDstAddr(bufTemp->srcAddr);

	flag = 0;
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
#ifdef  SUPPORT_STRATEGY_SORT
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

			NodeStrategySetOffset_Channel(temp, protocalRxPacket->len, temp2);
#endif
			StrategyRegisterSuccess();
			calendarTemp.Year       = 2000 + bufTemp->load[0];
			calendarTemp.Month      = bufTemp->load[1];
			calendarTemp.DayOfMonth = bufTemp->load[2];
			calendarTemp.Hours      = bufTemp->load[3];
			calendarTemp.Minutes    = bufTemp->load[4];
			calendarTemp.Seconds    = bufTemp->load[5];
			
			Rtc_set_calendar(&calendarTemp);
			NodeStopBroadcast();
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
					g_rSysConfigInfo.collectPeriod = temp;
					if (g_rSysConfigInfo.collectPeriod == 0 || g_rSysConfigInfo.collectPeriod >= 0xffff) {
					    g_rSysConfigInfo.collectPeriod = COLLECT_PERIOD_DEFAULT;
					}
					
					break;

					case PARASETTING_UPLOAD_INTERVAL:
					if(lenTemp < 5)
						goto NodeDispath;
					temp = ((uint32_t)bufTemp->load[baseAddr+1] << 24) + 
						   ((uint32_t)bufTemp->load[baseAddr+2] << 16) +
						   ((uint32_t)bufTemp->load[baseAddr+3] << 8) +
						   ((uint32_t)bufTemp->load[baseAddr+4]);
					lenTemp -= 5;

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
				                     (((uint32_t)(g_rSysConfigInfo.DeviceId[2])) << 8)  |
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
				    if(g_rSysConfigInfo.batLowVol > BAT_VOLTAGE_L2) {
				        g_rSysConfigInfo.batLowVol = BAT_VOLTAGE_L2;
				    }

				    if(g_rSysConfigInfo.batLowVol < BAT_VOLTAGE_LOW) {
				        g_rSysConfigInfo.batLowVol = BAT_VOLTAGE_LOW;
				    }
					lenTemp -= 3;
					break;

					case PARASETTING_SOFT_VER:
					if(lenTemp < 3)
						goto NodeDispath;
					// g_rSysConfigInfo.swVersion    = FW_VERSION;
					lenTemp -= 3;
					break;

					case PARASETTING_RF_PARA:
					if(lenTemp < 5)
						goto NodeDispath;
					g_rSysConfigInfo.rfPA    	= bufTemp->load[baseAddr+1];
					g_rSysConfigInfo.rfBW    	= bufTemp->load[baseAddr+2];
					g_rSysConfigInfo.rfSF    	= bufTemp->load[baseAddr+3];
					g_rSysConfigInfo.rfStatus 	= bufTemp->load[baseAddr+4];
#if defined (SUPPORT_BOARD_OLD_S1) || defined(SUPPORT_BOARD_OLD_S2S_1)
                    /* 当设为mast时S3_1和S2S_1的工作模式为模式1，其它模式2*/
                    if (g_rSysConfigInfo.rfStatus & STATUS_1310_MASTER) {
                        OldS1nodeAPP_setWorkMode(S1_OPERATING_MODE1);
                    } else {
                        OldS1nodeAPP_setWorkMode(S1_OPERATING_MODE2);
                    }
#endif
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

					case PARASETTING_RTC_SET:
	                if(lenTemp < 7) {
	                    goto NodeDispath;
	                }
			        calendarTemp.Year       = ((bufTemp->load[baseAddr + 1] << 8) | bufTemp->load[baseAddr + 2]);
			        calendarTemp.Month      = bufTemp->load[baseAddr + 3];
			        calendarTemp.DayOfMonth = bufTemp->load[baseAddr + 4];
			        calendarTemp.Hours      = bufTemp->load[baseAddr + 5];
			        calendarTemp.Minutes    = bufTemp->load[baseAddr + 6];
			        calendarTemp.Seconds    = bufTemp->load[baseAddr + 7];
			        Rtc_set_calendar(&calendarTemp);
			        lenTemp -= 7;
			        break;

					default:
					// error setting parameter
					Sys_event_post(SYSTEMAPP_EVT_STORE_SYS_CONFIG);
					goto NodeDispath;
				}
			}
			Sys_event_post(SYSTEMAPP_EVT_STORE_SYS_CONFIG);
			break;

			case RADIO_PRO_CMD_SET_PARA_ACK:
			break;

			case RADIO_PRO_CMD_ACK:
#ifdef  SUPPORT_STRATEGY_SORT
			if(setChannel == 0)
			{
				// get the concenter tick
				temp   =  ((uint32_t)bufTemp->load[3]) << 24;
				temp  |=  ((uint32_t)bufTemp->load[4]) << 16;
				temp  |=  ((uint32_t)bufTemp->load[5]) << 8;
				temp  |=  ((uint32_t)bufTemp->load[6]);

				NodeStrategySetOffset_Channel(temp, protocalRxPacket->len, NodeStrategyGetChannel());
				setChannel = 1;	
			}
#endif	// SUPPORT_STRATEGY_SORT
			StrategyRegisterSuccess();

			if(bufTemp->load[0] == PROTOCAL_FAIL) {
				NodeUploadOffectClear();
			} else {
                HIBYTE_ZKS(serialNum) = bufTemp->load[1];
                LOBYTE_ZKS(serialNum) = bufTemp->load[2];
                rxSensorDataAckRecord.lastFrameSerial[rxSensorDataAckRecord.Cnt] = serialNum;
                rxSensorDataAckRecord.Cnt++;
			}

			flag = 1;
			// resever more 6 package and is the last ack
			
			break;

#ifdef SUPPORT_RADIO_UPGRADE
			case RADIO_PRO_CMD_UPGRADE :
			    RadioUpgrade_CmdDataParse(bufTemp->load, lenTemp - 10);
			    Led_ctrl(LED_G, 1, 30 * CLOCK_UNIT_MS, 1);
			break;

			case RADIO_PRO_CMD_RATE_SWITCH :
			    RadioUpgrade_CmdRateSwitch();
			break;
#endif

			default:
			goto NodeDispath;

		}

		// point to new message the head
		bufTemp		= (radio_protocal_t *)((uint8_t *)bufTemp + bufTemp->len);
	}

NodeDispath:
    if ((RadioModeGet() != RADIOMODE_UPGRADE) && (deviceMode != DEVICES_CONFIG_MODE))
    {
        if (1 == flag) {
            flag = NodeUploadSucessProcess(&rxSensorDataAckRecord);
        }
	    if(Flash_get_unupload_items() > 0)
	    {
#ifdef SUPPORT_RSSI_CHECK
	    	NodeContinueFlagSet();
#endif // SUPPORT_RSSI_CHECK
	    	// clear the offect, the buf has been clear
		    NodeUploadProcess();
		    // waiting the gateway to change to receive
		    RadioSend();
	    }
	    NodeBroadcasting();
    }
}





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
	Calendar    calendarTemp;
	uint16_t serialNum;

#ifdef BOARD_CONFIG_DECEIVE
	int8_t i8Rssi;
	i8Rssi = protocalRxPacket->rssi;
//    System_printf("Rssi = %d\r\n", i8Rssi);
//    System_flush();
    if (RADIOMODE_UPGRADE != RadioModeGet() && i8Rssi < -65) {
        return;
    }
#endif

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
		lenTemp = bufTemp->len;

		switch(bufTemp->command)
		{
			// receive the sensor data, save to flash
			case RADIO_PRO_CMD_SENSOR_DATA:

			// there may be several sensordata

			// save the data to flash
			// updata the rssi
			bufTemp->load[1]		= (uint8_t)(protocalRxPacket->rssi);

#ifdef  S_G
#ifdef SUPPORT_NETGATE_DISP_NODE
			sensor_unpackage_to_memory(bufTemp->load, bufTemp->load[0]+1);
#endif // SUPPORT_NETGATE_DISP_NODE
#endif // S_G
			HIBYTE_ZKS(serialNum) = bufTemp->load[6];
			LOBYTE_ZKS(serialNum) = bufTemp->load[7];

			if(ConcenterSensorDataSaveToQueue(bufTemp->load, bufTemp->load[0]+1) == true)
			{
				ConcenterRadioSendSensorDataAck(GetRadioSrcAddr(), GetRadioDstAddr(), serialNum, ES_SUCCESS);
			}
			else
			{
				ConcenterRadioSendSensorDataAck(GetRadioSrcAddr(), GetRadioDstAddr(), serialNum, ES_ERROR);
			}

			break;


			case RADIO_PRO_CMD_SYN_TIME_REQ:
#ifdef SUPPORT_STRATEGY_SORT
			ConcenterSetNodeChannel(bufTemp->srcAddr, ((uint16_t)bufTemp->load[0])<<8 + bufTemp->load[1]);
#endif // SUPPORT_STRATEGY_SORT

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

                    case PARASETTING_RTC_SET:
                    if(lenTemp < 7) {
                        goto ConcenterConfigRespondEnd;
                    }
                    calendarTemp.Year       = ((bufTemp->load[baseAddr + 1] << 8) | bufTemp->load[baseAddr + 2]);
                    calendarTemp.Month      = bufTemp->load[baseAddr +3];
                    calendarTemp.DayOfMonth = bufTemp->load[baseAddr + 4];
                    calendarTemp.Hours      = bufTemp->load[baseAddr + 5];
                    calendarTemp.Minutes    = bufTemp->load[baseAddr + 6];
                    calendarTemp.Seconds    = bufTemp->load[baseAddr + 7];
                    Rtc_set_calendar(&calendarTemp);
                    lenTemp -= 7;
                    break;

					default:
					// error setting parameter
					goto ConcenterConfigRespondEnd;
				}
			}
ConcenterConfigRespondEnd:
#ifdef BOARD_CONFIG_DECEIVE
            g_rSysConfigInfo.sensorModule[0] = SEN_TYPE_SHT2X;
			UsbSend(AC_Send_Config);
#endif
			break;

#ifdef BOARD_CONFIG_DECEIVE
			case RADIO_PRO_CMD_UPGRADE_ACK :
			    RadioUpgrade_CmdACKDataParse(bufTemp->load, lenTemp - 10);
			break;

			case RADIO_PRO_CMD_RATE_SWTICH_ACK :
			    RadioUpgrade_CmdACKRateSwitch(bufTemp->load, lenTemp - 10);
			break;
#endif
			case RADIO_PRO_CMD_RESPOND_PARA_ACK:

			break;


			case RADIO_PRO_CMD_ACK:

			break;
#ifdef  SUPPORT_FREQ_FIND
			case RADIO_PRO_CMD_CHANNEL_CHECK:
				ConcenterChannelOccupyAck(bufTemp->dstAddr, bufTemp->srcAddr);
			break;

			case RADIO_PRO_CMD_CHANNEL_OCCUPY:
			    AutoFreqConcenterOccupy(protocalRxPacket->rssi);
			break;
#endif  // SUPPORT_FREQ_FIND

		}
		// point to new message the head
		bufTemp		= (radio_protocal_t *)((uint8_t *)bufTemp + bufTemp->len);
	}

	// receive several cmd in one radio packet, must return in one radio packet;
	if (RadioModeGet() != RADIOMODE_UPGRADE)
	{
		Task_sleep(CONCENTER_RADIO_DELAY_TIME_MS * CLOCK_UNIT_MS);
	    RadioSend();
	}
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
	protocalTxBuf.len = 10 + 2;
	protocalTxBuf.dstAddr	= GetRadioDstAddr();
	protocalTxBuf.srcAddr	= GetRadioSrcAddr();

	// the remainderCache is not satisfy length
	if(NodeStrategyRemainderCache() < protocalTxBuf.len)
		return false;

	protocalTxBuf.command	= RADIO_PRO_CMD_SYN_TIME_REQ;
	if(GetStrategyRegisterStatus() == false)
	{
		protocalTxBuf.load[0]	= (uint8_t)(NodeStrategyGetChannel() >> 8);
		protocalTxBuf.load[1]	= (uint8_t)NodeStrategyGetChannel();
	}
	else
	{
		protocalTxBuf.load[0]	= (uint8_t)(RADIO_INVAILD_CHANNEL >> 8);
		protocalTxBuf.load[1]	= (uint8_t)RADIO_INVAILD_CHANNEL;
	}

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

    Calendar rtc = Rtc_get_calendar();
    rtc = Rtc_get_calendar();
    protocalTxBuf.load[temp++]      = PARASETTING_RTC_SET;
    protocalTxBuf.load[temp++]      = ((rtc.Year >> 8) & 0xff);
    protocalTxBuf.load[temp++]      = rtc.Year & 0xff;
    protocalTxBuf.load[temp++]      = rtc.Month;
    protocalTxBuf.load[temp++]      = rtc.DayOfMonth;
    protocalTxBuf.load[temp++]      = rtc.Hours;
    protocalTxBuf.load[temp++]      = rtc.Minutes;
    protocalTxBuf.load[temp++]      = rtc.Seconds;

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
void ConcenterRadioSendSensorDataAck(uint32_t srcAddr, uint32_t dstAddr, uint16_t serialNum, ErrorStatus status)
{


	protocalTxBuf.command	= RADIO_PRO_CMD_ACK;
	protocalTxBuf.dstAddr	= dstAddr;
	protocalTxBuf.srcAddr	= srcAddr;
	protocalTxBuf.len 		= 10 + 7;


	protocalTxBuf.load[0]	= (uint8_t)(status);
	protocalTxBuf.load[1]   = HIBYTE_ZKS(serialNum);
	protocalTxBuf.load[2]   = LOBYTE_ZKS(serialNum);

#ifdef  SUPPORT_STRATEGY_SORT
    uint32_t temp;
	temp = Clock_getTicks();
	protocalTxBuf.load[3]	= (uint8_t)(temp >> 24);
	protocalTxBuf.load[4]	= (uint8_t)(temp >> 16);
	protocalTxBuf.load[5]	= (uint8_t)(temp >> 8);
	protocalTxBuf.load[6]	= (uint8_t)(temp);

	// temp = ConcenterReadNodeChannel(dstAddr);
 //    protocalTxBuf.load[7]   = (uint8_t)(temp >> 24);
 //    protocalTxBuf.load[8]   = (uint8_t)(temp >> 16);
 //    protocalTxBuf.load[9]   = (uint8_t)(temp >> 8);
 //    protocalTxBuf.load[10]   = (uint8_t)(temp);

#endif

	SetRadioDstAddr(dstAddr);


    RadioCopyPacketToBuf(((uint8_t*)&protocalTxBuf), protocalTxBuf.len, 0, 0, EASYLINK_MAX_DATA_LENGTH - concenterRemainderCache);
    concenterRemainderCache -= protocalTxBuf.len;
}


//***********************************************************************************
// brief:   the concenter send the command to check if there is other concenter occupy the channel
// 
// parameter: 
// srcAddr:	the concenter radio addr
// dstAddr:	the node radio addr
//***********************************************************************************
void ConcenterChannelCheckCmdSend(uint32_t srcAddr, uint32_t dstAddr)
{
	concenterRemainderCache = EASYLINK_MAX_DATA_LENGTH;
	
	protocalTxBuf.command	= RADIO_PRO_CMD_CHANNEL_CHECK;
	protocalTxBuf.dstAddr	= dstAddr;
	protocalTxBuf.srcAddr	= srcAddr;
	protocalTxBuf.len 		= 10;

	RadioCopyPacketToBuf(((uint8_t*)&protocalTxBuf), protocalTxBuf.len, 0, 0, EASYLINK_MAX_DATA_LENGTH - concenterRemainderCache);
    concenterRemainderCache -= protocalTxBuf.len;
}

//***********************************************************************************
// brief:   echo the channel has been occupy
// 
// parameter: 
// srcAddr:	the concenter radio addr
// dstAddr:	the node radio addr
//***********************************************************************************
void ConcenterChannelOccupyAck(uint32_t srcAddr, uint32_t dstAddr)
{
	protocalTxBuf.command	= RADIO_PRO_CMD_CHANNEL_OCCUPY;
	protocalTxBuf.dstAddr	= dstAddr;
	protocalTxBuf.srcAddr	= srcAddr;
	protocalTxBuf.len 		= 10;

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

#ifdef SUPPORT_STRATEGY_SORT
    uint32_t temp;
	temp 					= Clock_getTicks();

	protocalTxBuf.load[6]	= (uint8_t)(temp >> 24);
	protocalTxBuf.load[7]	= (uint8_t)(temp >> 16);
	protocalTxBuf.load[8]	= (uint8_t)(temp >> 8);
	protocalTxBuf.load[9]	= (uint8_t)(temp);

	temp 					= ConcenterReadNodeChannel(dstAddr);
	protocalTxBuf.load[10]	= (uint8_t)(temp >> 24);
	protocalTxBuf.load[11]	= (uint8_t)(temp >> 16);
	protocalTxBuf.load[12]	= (uint8_t)(temp >> 8);
	protocalTxBuf.load[13]	= (uint8_t)(temp);

#endif  // SUPPORT_STRATEGY_SORT
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

	Calendar rtc = Rtc_get_calendar();
	rtc = Rtc_get_calendar();
    protocalTxBuf.load[temp++]      = PARASETTING_RTC_SET;
    protocalTxBuf.load[temp++]      = ((rtc.Year >> 8) & 0xff);
    protocalTxBuf.load[temp++]      = rtc.Year & 0xff;
    protocalTxBuf.load[temp++]      = rtc.Month;
    protocalTxBuf.load[temp++]      = rtc.DayOfMonth;
    protocalTxBuf.load[temp++]      = rtc.Hours;
    protocalTxBuf.load[temp++]      = rtc.Minutes;
    protocalTxBuf.load[temp++]      = rtc.Seconds;

	protocalTxBuf.len = 10 + temp;

	RadioSendPacket((uint8_t*)&protocalTxBuf, protocalTxBuf.len, 0, 0);
}
