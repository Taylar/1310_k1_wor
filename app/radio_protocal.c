/*
* @Author: zxt
* @Date:   2017-12-26 16:36:20
* @Last Modified by:   zxt
* @Last Modified time: 2019-12-17 16:58:41
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

#ifdef S_C
//***********************************************************************************
// brief:   analysis the node protocal 
// 
// parameter: 
//***********************************************************************************
void NodeProtocalDispath(EasyLink_RxPacket * protocalRxPacket)
{
	uint8_t len, lenTemp, baseAddr, flag, sendContinue;
	uint32_t	temp;
	radio_protocal_t	*bufTemp;
    Calendar    calendarTemp;
    uint16_t serialNum;
    uint8_t setChannel = 0;
#ifdef  SUPPORT_STRATEGY_SORT
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
	sendContinue = 1;
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
			temp   =  ((uint32_t)bufTemp->load[10]) << 24;
			temp  |=  ((uint32_t)bufTemp->load[11]) << 16;
			temp  |=  ((uint32_t)bufTemp->load[12]) << 8;
			temp  |=  ((uint32_t)bufTemp->load[13]);

			// get the channel
			temp2  =  ((uint32_t)bufTemp->load[14]) << 24;
			temp2 |=  ((uint32_t)bufTemp->load[15]) << 16;
			temp2 |=  ((uint32_t)bufTemp->load[16]) << 8;
			temp2 |=  ((uint32_t)bufTemp->load[17]);

			if(GetStrategyRegisterStatus() == false)
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
#ifndef SUPPORT_BOARD_Z4
			temp   =  ((uint32_t)bufTemp->load[6]) << 24;
			temp  |=  ((uint32_t)bufTemp->load[7]) << 16;
			temp  |=  ((uint32_t)bufTemp->load[8]) << 8;
			temp  |=  ((uint32_t)bufTemp->load[9]);

			if(temp == 0)
			{
				temp = g_rSysConfigInfo.collectPeriod;
			}
			g_rSysConfigInfo.uploadPeriod = temp;
			NodeStrategySetPeriod(temp);
#endif  // SUPPORT_BOARD_Z4

#ifdef SUPPORT_BOARD_Z4
			if(NodeIsBroadcast() == true)
			{
				SensorSerialNumAdd();
				serialNum    = GetSensorSerialNum();
				assetInfo[0] = HIBYTE_ZKS(serialNum);
				assetInfo[1] = LOBYTE_ZKS(serialNum);
				assetInfo[2] = TransHexToBcd(calendarTemp.Year -2000); //TransHexToBcd(lastcalendar.Year - 2000);
				assetInfo[3] = TransHexToBcd(calendarTemp.Month);
				assetInfo[4] = TransHexToBcd(calendarTemp.DayOfMonth);
			    assetInfo[5] = TransHexToBcd(calendarTemp.Hours);
			    assetInfo[6] = TransHexToBcd(calendarTemp.Minutes);
			    // assetInfo[7] = TransHexToBcd(calendarTemp.Seconds);
			    NodeContinueFlagSet();
			    RadioSensorDataPack();
			    RadioSend();
			}
#endif //SUPPORT_BOARD_Z4
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
					if (g_rSysConfigInfo.collectPeriod == 0) {
					    g_rSysConfigInfo.collectPeriod = COLLECT_PERIOD_DEFAULT;
					}
					
					// NodeStrategySetPeriod(g_rSysConfigInfo.collectPeriod);
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
					// NodeStrategySetPeriod(temp);

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
					if((g_rSysConfigInfo.rfSF >> 4) > RADIO_EASYLINK_MODULATION_S1_OLD)
						g_rSysConfigInfo.rfSF = RADIO_EASYLINK_MODULATION << 4;
					if((g_rSysConfigInfo.rfPA >> 4) < RADIO_MIN_POWER)
						g_rSysConfigInfo.rfPA = (RADIO_MIN_POWER<<4) + (g_rSysConfigInfo.rfPA & 0xf);
#if defined (SUPPORT_BOARD_OLD_S1) || defined(SUPPORT_BOARD_OLD_S2S_1)
                    /* µ±ÉèÎªmastÊ±S3_1ºÍS2S_1µÄ¹¤×÷Ä£Ê½ÎªÄ£Ê½1£¬ÆäËüÄ£Ê½2*/
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

#ifdef  SUPPORT_RARIO_APC_SET
			if(setChannel == 0)
				NodeAPC(bufTemp->load[3]);
#endif // SUPPORT_RARIO_APC_SET

#ifdef  SUPPORT_STRATEGY_SORT
			if(setChannel == 0)
			{
				// get the concenter tick
				temp   =  ((uint32_t)bufTemp->load[4]) << 24;
				temp  |=  ((uint32_t)bufTemp->load[5]) << 16;
				temp  |=  ((uint32_t)bufTemp->load[6]) << 8;
				temp  |=  ((uint32_t)bufTemp->load[7]);

				NodeStrategySetOffset_Channel(temp, protocalRxPacket->len, NodeStrategyGetChannel());
			}
#endif	// SUPPORT_STRATEGY_SORT
			setChannel = 1;
			StrategyRegisterSuccess();

			if(bufTemp->load[0] == PROTOCAL_FAIL) {
				NodeUploadOffectClear();
				sendContinue = 0;
			} else {
                HIBYTE_ZKS(serialNum) = bufTemp->load[1];
                LOBYTE_ZKS(serialNum) = bufTemp->load[2];
                rxSensorDataAckRecord.lastFrameSerial[rxSensorDataAckRecord.Cnt] = serialNum;
                rxSensorDataAckRecord.Cnt++;
				flag = 1;
			}
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

			case RADIO_PRO_CMD_WAKEUP_SENSOR :
			    if( !(g_rSysConfigInfo.rfStatus&STATUS_1310_MASTER)){

			        calendarTemp.Year       = ((bufTemp->load[0] << 8) | bufTemp->load[1]);
			        calendarTemp.Month      = bufTemp->load[2];
			        calendarTemp.DayOfMonth = bufTemp->load[3];
			        calendarTemp.Hours      = bufTemp->load[4];
			        calendarTemp.Minutes    = bufTemp->load[5];
			        calendarTemp.Seconds    = bufTemp->load[6];

			         Rtc_set_calendar(&calendarTemp);

			         Z5TxLoseAlarm();//
			         lenTemp -= 6;

			    }
			    break;

			case RADIO_PRO_CMD_CONFIG_CONTROL:
				switch(bufTemp->load[0])
				{
					case CONFIG_CONTROL_CLEAR_DATA:
					Led_set(LED_R, 1);
					Led_set(LED_G, 1);
					Led_set(LED_B, 1);
					Flash_reset_all();
					Task_sleep(500* CLOCK_UNIT_MS);
					SystemResetAndSaveRtc();
					break;

					case CONFIG_CONTROL_GET_VOLTAGE:
					NodeRadioSend_CMD_CONFIG_CONTROL_ACK(CONFIG_CONTROL_SEND_VOLTAGE);
					return;

					case CONFIG_CONTROL_GET_SENSORDATA:
					NodeRadioSend_CMD_CONFIG_CONTROL_ACK(CONFIG_CONTROL_SEND_SENSORDATA);
					return;
				}

			case RADIO_PRO_CMD_GATWAY_SET_RF:
				if(deviceMode == DEVICES_CONFIG_MODE)
				{
					g_rSysConfigInfo.customId[0] = bufTemp->load[0];
					g_rSysConfigInfo.customId[1] = bufTemp->load[1];
					g_rSysConfigInfo.rfBW        = bufTemp->load[2];
					g_rSysConfigInfo.rfSF        = bufTemp->load[3];

					if(bufTemp->load[4] & STATUS_LORA_CHANGE_FREQ)
						g_rSysConfigInfo.rfStatus    |= STATUS_LORA_CHANGE_FREQ;
					else
						g_rSysConfigInfo.rfStatus    &= STATUS_LORA_CHANGE_FREQ^0xff;

					Task_sleep(1000 * CLOCK_UNIT_MS);
					SystemResetAndSaveRtc();
					// configModeTimeCnt = S1_CONFIG_MODE_TIME - 1;
					// Sys_event_post(SYSTEMAPP_EVT_STORE_SYS_CONFIG);
				}
				break;
			case RADIO_PRO_CMD_GATESEND_REQUEST:
			nodeParaSetting = 1;
			break;


			case RADIO_PRO_CMD_REMOTE_SET_CMD:
				readyNodeConfig = (node_configpara_t *)(bufTemp->load);
				NodeRemoteSetAnalysis();
				NodeRadioSendRemoteSetAck();
			break;


			case RADIO_PRO_CMD_SPE_BROCAST:
				Task_sleep((bufTemp->load[0]+1) * 40 * CLOCK_UNIT_MS);
			    RadioEventPost(RADIO_EVT_WAKEUP_SEND_ACK);
			return;


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
#ifndef SUPPORT_UPLOAD_ASSET_INFO
	    if((Flash_get_unupload_items() > 0) && sendContinue)
	    {
	    	NodeContinueFlagSet();
	    	// clear the offect, the buf has been clear
		    NodeUploadProcess();
		    // waiting the gateway to change to receive
		    RadioSend();
	    }
#endif //SUPPORT_UPLOAD_ASSET_INFO
	    NodeBroadcasting();
    }
}


#endif // S_C

uint32_t lastNodeAddr = 0;

void ConcenterClearSynNode(void)
{
	lastNodeAddr = 0;
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
// brief:   send the parameter cmd ack to the config tool
// 
// parameter: 
// status:	success or fail
//***********************************************************************************
void NodeRadioSend_CMD_CONFIG_CONTROL_ACK(uint8_t cmd)
{
	uint8_t index;
	uint16_t temp;

	index = 0;

	protocalTxBuf.command	= RADIO_PRO_CMD_CONFIG_CONTROL_ACK;
	protocalTxBuf.dstAddr	= GetRadioDstAddr();
	protocalTxBuf.srcAddr	= GetRadioSrcAddr();

	// the remainderCache is not satisfy length
	if(NodeStrategyRemainderCache() < protocalTxBuf.len)
		return;

	protocalTxBuf.load[index++]		= cmd;
	switch(cmd)
	{

		case CONFIG_CONTROL_SEND_VOLTAGE:
		temp = Battery_get_voltage();

		protocalTxBuf.load[index++] = (uint8_t)(temp >> 8);
		protocalTxBuf.load[index++] = (uint8_t)(temp);
		break;

#ifdef  SUPPORT_SENSOR
		case CONFIG_CONTROL_SEND_SENSORDATA:
		index += Sensor_data_pack(protocalTxBuf.load+index);
		break;
#endif //SUPPORT_SENSOR

		default:
		break;
	}


	protocalTxBuf.len = 10 + index;

	RadioSendPacket((uint8_t*)&protocalTxBuf, protocalTxBuf.len, 0, 0);
	NodeStrategySendPacket((uint8_t*)&protocalTxBuf, protocalTxBuf.len);
}

//***********************************************************************************
// brief:   send the remote setting ack to gateway
// 
// parameter: 
// status:	success or fail
//***********************************************************************************
void NodeRadioSendRemoteSetAck(void)
{

	protocalTxBuf.command	= RADIO_PRO_CMD_REMOTE_SET_ACK;
	protocalTxBuf.dstAddr	= GetRadioDstAddr();
	protocalTxBuf.srcAddr	= GetRadioSrcAddr();

	protocalTxBuf.len = 10;
	RadioSendPacket((uint8_t*)&protocalTxBuf, protocalTxBuf.len, 0, 0);
}


//***********************************************************************************
// brief:   send the brocast ack to gateway
// 
// parameter: 
// status:	success or fail
//***********************************************************************************
void NodeRadioSendBrocastAck(void)
{

	protocalTxBuf.command	= RADIO_PRO_CMD_SPE_BROCAST_ACK;
	protocalTxBuf.dstAddr	= GetRadioDstAddr();
	protocalTxBuf.srcAddr	= GetRadioSrcAddr();

	protocalTxBuf.len = 10;
	RadioCopyPacketToBuf((uint8_t*)&protocalTxBuf, protocalTxBuf.len, 0, 0, 0);
}


void NodeRemoteSetAnalysis(void)
{
    uint8_t i;
	if(readyNodeConfig->preConfigType & RADIO_CONFIGTYPE_COLLECTPERIOD)
		g_rSysConfigInfo.collectPeriod = readyNodeConfig->collectPeriod;

	if(readyNodeConfig->preConfigType & RADIO_CONFIGTYPE_UPLOADPERIOD)
		g_rSysConfigInfo.uploadPeriod = readyNodeConfig->uploadPeriod;

	if(readyNodeConfig->preConfigType & RADIO_CONFIGTYPE_ALARMTEMP_HIGH)
	{
		if(readyNodeConfig->setChn == 0xFF)
		{
			for ( i = 0 ; i < MODULE_SENSOR_MAX; ++i) {
                g_rSysConfigInfo.alarmTemp[i].high = readyNodeConfig->alarmTempHighValue;
            }
		}
		else if(readyNodeConfig->setChn < MODULE_SENSOR_MAX)
		{
			for(i = 0; i < 8; i++){
				if((0x01 << i) & readyNodeConfig->setChn)
				{
					g_rSysConfigInfo.alarmTemp[i].high = readyNodeConfig->alarmTempHighValue;
				}
			}
		}
	}

	if(readyNodeConfig->preConfigType & RADIO_CONFIGTYPE_ALARMTEMP_LOW)
	{
		if(readyNodeConfig->setChn == 0xFF)
		{
			for ( i = 0 ; i < MODULE_SENSOR_MAX; ++i) {
                g_rSysConfigInfo.alarmTemp[i].low = readyNodeConfig->alarmTempLowValue;
            }
		}
		else if(readyNodeConfig->setChn < MODULE_SENSOR_MAX)
		{
			for(i = 0; i < MODULE_SENSOR_MAX; i++){
				if((0x01 << i) & readyNodeConfig->setChn)
				{
					g_rSysConfigInfo.alarmTemp[i].low = readyNodeConfig->alarmTempLowValue;
				}
			}
		}
	}		


	Sys_event_post(SYSTEMAPP_EVT_STORE_SYS_CONFIG);
}



node_configpara_t nodeConfig[NODE_CONFIG_MAX], nodeSetComplete, *readyNodeConfig;

//***********************************************************************************
// brief:   analysis the concenter protocal
// 
// parameter: 
//***********************************************************************************
void ConcenterProtocalDispath(EasyLink_RxPacket * protocalRxPacket)
{
	uint8_t len;
	radio_protocal_t	*bufTemp;
	uint16_t serialNum;
	uint8_t syncTime;
#ifdef SUPPORT_NETGATE_DISP_NODE
	uint8_t buff[32];
#endif //SUPPORT_NETGATE_DISP_NODE

#ifdef BOARD_CONFIG_DECEIVE
	uint8_t lenTemp, baseAddr;
	uint32_t    temp;
    Calendar    calendarTemp;
#endif //BOARD_CONFIG_DECEIVE

#ifdef BOARD_CONFIG_DECEIVE
	int8_t i8Rssi;
	i8Rssi = protocalRxPacket->rssi;
//    System_printf("Rssi = %d\r\n", i8Rssi);
//    System_flush();
	if(RADIOMODE_UPGRADE != RadioModeGet())
	{
		if(i8Rssi < -55)
			return;
	}
	else if(i8Rssi < -65)
	{

	}

#endif



	concenterRemainderCache = EASYLINK_MAX_DATA_LENGTH;
	len                     = protocalRxPacket->len;
	bufTemp                 = (radio_protocal_t *)protocalRxPacket->payload;

	ConcenterRadioMonitorClear();
	
	ClearRadioSendBuf();
#ifdef SUPPORT_DISP_SCREEN
	if(g_rSysConfigInfo.rfStatus & STATUS_LORA_TEST) {    //display rssi            

        Disp_number(0,6,protocalRxPacket->rssi <0?-protocalRxPacket->rssi:protocalRxPacket->rssi, 3, FONT_8X16);

        sprintf((char*)buff, "%02x%02x%02x%02x",    HIBYTE_ZKS(HIWORD_ZKS(bufTemp->srcAddr)),
                                                    LOBYTE_ZKS(HIWORD_ZKS(bufTemp->srcAddr)),
                                                    HIBYTE_ZKS(LOWORD_ZKS(bufTemp->srcAddr)),
                                                    LOBYTE_ZKS(LOWORD_ZKS(bufTemp->srcAddr)));
        Disp_msg(8, 6, buff, FONT_8X16);
        Led_ctrl(LED_B, 1, 250 * CLOCK_UNIT_MS, 2);
    }
#endif //S_G
	if(bufTemp->srcAddr != lastNodeAddr)
		syncTime = 1;
	else
		syncTime = 0;
	lastNodeAddr = bufTemp->srcAddr;
	SetRadioDstAddr(bufTemp->srcAddr);



	while(len)
	{
		// the receive data is not integrated
		if((bufTemp->len > len) || (bufTemp->len == 0) || (len == 0))
			break;
		
		len 	-= bufTemp->len;
#ifdef BOARD_CONFIG_DECEIVE
		lenTemp = bufTemp->len;
#endif
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
				ConcenterRadioSendSensorDataAck(GetRadioSrcAddr(), GetRadioDstAddr(), serialNum, ES_SUCCESS, protocalRxPacket->rssi);
			}
			else
			{
				ConcenterRadioSendSensorDataAck(GetRadioSrcAddr(), GetRadioDstAddr(), serialNum, ES_ERROR, protocalRxPacket->rssi);
			}


		//	if(g_rSysConfigInfo.status&STATUS_ALARM_GATE_ON){

			    //sys_Node_Lose_Alarm();
		//	}

			break;


			case RADIO_PRO_CMD_SYN_TIME_REQ:
			// send the time
			// if(ConcenterReadSynTimeFlag())
			{
#ifdef SUPPORT_STRATEGY_SORT
				ConcenterSetNodeChannel(bufTemp->srcAddr, (((uint32_t)(bufTemp->load[0]))<<8) + (uint32_t)(bufTemp->load[1]));
#endif // SUPPORT_STRATEGY_SORT				
				// ConcenterRadioSendSynTime(bufTemp->dstAddr, bufTemp->srcAddr);
			}

#ifdef  S_G
#ifdef SUPPORT_NETGATE_DISP_NODE
				buff[1] = protocalRxPacket->rssi;  // rssi
				buff[2] = protocalRxPacket->payload[9];  //id
				buff[3] = protocalRxPacket->payload[8];  //id
				buff[4] = protocalRxPacket->payload[7];  //id
				buff[5] = protocalRxPacket->payload[6];  //id
				buff[16] = 0;  // sensor num
				buff[17] = SEN_TYPE_INVALID;  //sensor type
				sensor_unpackage_to_memory(buff, 17);
#endif // SUPPORT_NETGATE_DISP_NODE
#endif // S_G
			syncTime = 1;
			break;


			case RADIO_PRO_CMD_SYN_TIME:
			break;


			case RADIO_PRO_CMD_SET_PARA:

			break;


			case RADIO_PRO_CMD_SET_PARA_ACK:

			break;

			case RADIO_PRO_CMD_RESPOND_PARA:
#ifdef BOARD_CONFIG_DECEIVE
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
            
			UsbSend(AC_Send_Config);
#else
			if(gatewayConfigTime)
			{
				syncTime = 0;
				ConcenterSendRfConfig(bufTemp->dstAddr, bufTemp->srcAddr);
				RadioSwitchRate();
				gatewayConfigTime = 0;
			}
#endif  //BOARD_CONFIG_DECEIVE
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
			case RADIO_PRO_CMD_CHANNEL_CHECK:
				ConcenterChannelOccupyAck(bufTemp->dstAddr, bufTemp->srcAddr);
			break;
#if 0
            case RADIO_PRO_CMD_LOSE_ALARM:
              //  if(g_rSysConfigInfo.rfStatus&STATUS_LOSE_ALARM){
                   sys_Node_Lose_Alarm();
            //    }
            break;
#endif
			case RADIO_PRO_CMD_CHANNEL_OCCUPY:
			    AutoFreqConcenterOccupy(protocalRxPacket->rssi);
			    return;

			case RADIO_PRO_CMD_CONFIG_CONTROL_ACK:
				switch(bufTemp->load[0])
				{
					case CONFIG_CONTROL_SEND_VOLTAGE:
					UsbSend_NodeConfig(AC_Send_Voltage, bufTemp->load+1, bufTemp->len-10);
					break;

					case CONFIG_CONTROL_SEND_SENSORDATA:
					UsbSend_NodeConfig(AC_Send_SensorData, bufTemp->load+1, bufTemp->len-10);
					break;
				}
			break;

			case RADIO_PRO_CMD_REMOTE_SET_ACK:
			Radio_ClearNodeConfig(GetRadioDstAddr());
			
			break;


			case RADIO_PRO_CMD_SPE_BROCAST_ACK:
#ifdef SUPPORT_NETGATE_DISP_NODE
				buff[1] = protocalRxPacket->rssi;  // rssi
				buff[2] = protocalRxPacket->payload[9];  //id
				buff[3] = protocalRxPacket->payload[8];  //id
				buff[4] = protocalRxPacket->payload[7];  //id
				buff[5] = protocalRxPacket->payload[6];  //id
				buff[16] = 0;  // sensor num
				buff[17] = SEN_TYPE_INVALID;  //sensor type
				sensor_unpackage_to_memory(buff, 17);
				return;
#endif // SUPPORT_NETGATE_DISP_NODE

		}
		// point to new message the head
		bufTemp		= (radio_protocal_t *)((uint8_t *)bufTemp + bufTemp->len);
	}

	// receive several cmd in one radio packet, must return in one radio packet;
	//if( !(g_rSysConfigInfo.status&STATUS_ALARM_GATE_ON) && ((RadioModeGet() != RADIOMODE_UPGRADE)))
	if( RadioModeGet() != RADIOMODE_UPGRADE)
	{
		Task_sleep(CONCENTER_RADIO_DELAY_TIME_MS * CLOCK_UNIT_MS);

		readyNodeConfig = Radio_GetNodeConfig(GetRadioDstAddr());
		if(readyNodeConfig != NULL)
		{
			nodeParaSetting = 1;
			ConcenterSendRequest(((radio_protocal_t *)protocalRxPacket->payload)->dstAddr, ((radio_protocal_t *)protocalRxPacket->payload)->srcAddr);
		}

		if(syncTime)
			ConcenterRadioSendSynTime(((radio_protocal_t *)protocalRxPacket->payload)->dstAddr, ((radio_protocal_t *)protocalRxPacket->payload)->srcAddr);
	    RadioSend();
	}
    Sys_event_post(SYSTEMAPP_EVT_DISP);
}



//***********************************************************************************
// brief:   send the sensor data receive result to the node immediately
// 
// parameter: 
// srcAddr:	the concenter radio addr
// dstAddr:	the node radio addr
// status:	success or fail
//***********************************************************************************
void ConcenterRadioSendSensorDataAck(uint32_t srcAddr, uint32_t dstAddr, uint16_t serialNum, ErrorStatus status, int8_t rssi)
{


	protocalTxBuf.command	= RADIO_PRO_CMD_ACK;
	protocalTxBuf.dstAddr	= dstAddr;
	protocalTxBuf.srcAddr	= srcAddr;
#ifdef  SUPPORT_STRATEGY_SORT
	protocalTxBuf.len 		= 10 + 8;
#else
	protocalTxBuf.len 		= 10 + 4;
#endif // SUPPORT_STRATEGY_SORT

	protocalTxBuf.load[0]	= (uint8_t)(status);
	protocalTxBuf.load[1]   = HIBYTE_ZKS(serialNum);
	protocalTxBuf.load[2]   = LOBYTE_ZKS(serialNum);
	protocalTxBuf.load[3]   = rssi;

#ifdef  SUPPORT_STRATEGY_SORT
    uint32_t temp;
	temp = Clock_getTicks();
	protocalTxBuf.load[4]	= (uint8_t)(temp >> 24);
	protocalTxBuf.load[5]	= (uint8_t)(temp >> 16);
	protocalTxBuf.load[6]	= (uint8_t)(temp >> 8);
	protocalTxBuf.load[7]	= (uint8_t)(temp);

	// temp = ConcenterReadNodeChannel(dstAddr);
 //    protocalTxBuf.load[7]   = (uint8_t)(temp >> 24);
 //    protocalTxBuf.load[8]   = (uint8_t)(temp >> 16);
 //    protocalTxBuf.load[9]   = (uint8_t)(temp >> 8);
 //    protocalTxBuf.load[10]   = (uint8_t)(temp);

#endif

	SetRadioDstAddr(dstAddr);


    if(RadioCopyPacketToBuf(((uint8_t*)&protocalTxBuf), protocalTxBuf.len, 0, 0, EASYLINK_MAX_DATA_LENGTH - concenterRemainderCache) == true)
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

	if(RadioCopyPacketToBuf(((uint8_t*)&protocalTxBuf), protocalTxBuf.len, 0, 0, EASYLINK_MAX_DATA_LENGTH - concenterRemainderCache) == true)
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

	if(RadioCopyPacketToBuf(((uint8_t*)&protocalTxBuf), protocalTxBuf.len, 0, 0, EASYLINK_MAX_DATA_LENGTH - concenterRemainderCache) == true)
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
#ifdef SUPPORT_STRATEGY_SORT
	protocalTxBuf.len 		= 10+18;
#endif  // SUPPORT_STRATEGY_SORT

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
#ifdef SUPPORT_STRATEGY_SORT
	temp 					= Clock_getTicks();

	protocalTxBuf.load[10]	= (uint8_t)(temp >> 24);
	protocalTxBuf.load[11]	= (uint8_t)(temp >> 16);
	protocalTxBuf.load[12]	= (uint8_t)(temp >> 8);
	protocalTxBuf.load[13]	= (uint8_t)(temp);

	temp 					= ConcenterReadResentNodeChannel();
	protocalTxBuf.load[14]	= (uint8_t)(temp >> 24);
	protocalTxBuf.load[15]	= (uint8_t)(temp >> 16);
	protocalTxBuf.load[16]	= (uint8_t)(temp >> 8);
	protocalTxBuf.load[17]	= (uint8_t)(temp);

#endif  // SUPPORT_STRATEGY_SORT
	SetRadioDstAddr(dstAddr);


    if(RadioCopyPacketToBuf(((uint8_t*)&protocalTxBuf), protocalTxBuf.len, 0, 0, EASYLINK_MAX_DATA_LENGTH - concenterRemainderCache) == true)
	    concenterRemainderCache -= protocalTxBuf.len;
}


void ConcenterRadioSendCmd(uint32_t srcAddr, uint32_t dstAddr, uint8_t cmd)
{
    uint8_t temp;
	temp = 0;


	protocalTxBuf.command	= RADIO_PRO_CMD_CONFIG_CONTROL;
	protocalTxBuf.dstAddr	= dstAddr;
	protocalTxBuf.srcAddr	= srcAddr;

	protocalTxBuf.load[temp++]		= cmd;

	protocalTxBuf.len = 10 + temp;
	ClearRadioSendBuf();
	RadioSendPacket((uint8_t*)&protocalTxBuf, protocalTxBuf.len, 0, 0);
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
	ClearRadioSendBuf();
	RadioSendPacket((uint8_t*)&protocalTxBuf, protocalTxBuf.len, 0, 0);
}

//***********************************************************************************
// brief:   send the packet to wakeup sensor
//
// parameter:
// srcAddr: the concenter radio addr
// dstAddr: the node radio addr

//***********************************************************************************
void GateWakeUpSensorSend(uint32_t srcAddr, uint32_t dstAddr){

    uint8_t temp;
    temp = 0;

    Calendar rtc = Rtc_get_calendar();
        rtc = Rtc_get_calendar();
        //protocalTxBuf.load[temp++]      = PARASETTING_RTC_SET;
        protocalTxBuf.load[temp++]      = ((rtc.Year >> 8) & 0xff);
        protocalTxBuf.load[temp++]      = rtc.Year & 0xff;
        protocalTxBuf.load[temp++]      = rtc.Month;
        protocalTxBuf.load[temp++]      = rtc.DayOfMonth;
        protocalTxBuf.load[temp++]      = rtc.Hours;
        protocalTxBuf.load[temp++]      = rtc.Minutes;
        protocalTxBuf.load[temp++]      = rtc.Seconds;



    protocalTxBuf.command   = RADIO_PRO_CMD_WAKEUP_SENSOR;
    protocalTxBuf.dstAddr   = dstAddr;
    protocalTxBuf.srcAddr   = srcAddr;

    protocalTxBuf.len = 10 + temp;

	ClearRadioSendBuf();
    RadioSendPacket((uint8_t*)&protocalTxBuf, protocalTxBuf.len, 0, 0);
}

void ConcenterSendRfConfig(uint32_t srcAddr, uint32_t dstAddr)
{
    uint8_t temp;
	temp = 0;


	protocalTxBuf.command	= RADIO_PRO_CMD_GATWAY_SET_RF;
	protocalTxBuf.dstAddr	= dstAddr;
	protocalTxBuf.srcAddr	= srcAddr;

	protocalTxBuf.load[temp++]		= g_rSysConfigInfo.customId[0];
	protocalTxBuf.load[temp++]		= g_rSysConfigInfo.customId[1];

	protocalTxBuf.load[temp++]		= g_rSysConfigInfo.rfBW;
	protocalTxBuf.load[temp++]		= g_rSysConfigInfo.rfSF;
	protocalTxBuf.load[temp++]		= g_rSysConfigInfo.rfStatus;



	protocalTxBuf.len = 10 + temp;
	ClearRadioSendBuf();
	RadioSendPacket((uint8_t*)&protocalTxBuf, protocalTxBuf.len, 0, 0);
}

void ConcenterSendRemoteSeting(uint32_t srcAddr, uint32_t dstAddr)
{
    uint8_t temp;
	temp = 0;


	protocalTxBuf.command	= RADIO_PRO_CMD_REMOTE_SET_CMD;
	protocalTxBuf.dstAddr	= dstAddr;
	protocalTxBuf.srcAddr	= srcAddr;

	memcpy(protocalTxBuf.load, (uint8_t *)readyNodeConfig, sizeof(node_configpara_t));
	temp = sizeof(node_configpara_t);

	protocalTxBuf.len = 10 + temp;
	ClearRadioSendBuf();
	RadioSendPacket((uint8_t*)&protocalTxBuf, protocalTxBuf.len, 0, 0);

	RadioSend();
	nodeParaSetting = 0;
}


void ConcenterSendRequest(uint32_t srcAddr, uint32_t dstAddr)
{
    uint8_t temp;
	temp = 0;


	protocalTxBuf.command	= RADIO_PRO_CMD_GATESEND_REQUEST;
	protocalTxBuf.dstAddr	= dstAddr;
	protocalTxBuf.srcAddr	= srcAddr;

	protocalTxBuf.len = 10 + temp;

	if(RadioCopyPacketToBuf(((uint8_t*)&protocalTxBuf), protocalTxBuf.len, 0, 0, EASYLINK_MAX_DATA_LENGTH - concenterRemainderCache) == true)
	    concenterRemainderCache -= protocalTxBuf.len;
}

bool Radio_SetNodeConfig(node_configpara_t *para)
{
	uint16_t i;
	static uint16_t lastSettingId = 0;

	if(nodeParaSetting)
	{
		return false;
	}
	for(i = 0; i < NODE_CONFIG_MAX; i++)
	{
		if(nodeConfig[i].devID == para->devID)
		{
			nodeConfig[i] = *para;
			return true;
		}
	}

	for(i = 0; i < NODE_CONFIG_MAX; i++)
	{
		if(nodeConfig[i].preConfigType == 0)
		{
			nodeConfig[i] = *para;
			lastSettingId = i;
			return true;
		}
	}

	lastSettingId++;
	if(lastSettingId >= NODE_CONFIG_MAX)
		lastSettingId = 0;

	nodeConfig[i] = *para;

	return true;
}

node_configpara_t * Radio_GetNodeConfig(uint32_t devID)
{
	uint16_t i ;
	for(i = 0; i < NODE_CONFIG_MAX; i++)
	{
		if(nodeConfig[i].devID == devID && nodeConfig[i].preConfigType)
		{
			return &nodeConfig[i];
		}
	}
	return NULL;
}


void Radio_ClearNodeConfig(uint32_t devID)
{
    uint16_t i;
	for(i = 0; i < NODE_CONFIG_MAX; i++)
	{
		if((nodeConfig[i].devID == devID) && (devID != 0))
		{
			nodeSetComplete = nodeConfig[i];
			Nwk_nodeSetAckEventSet();
			nodeConfig[i].preConfigType = 0;
			nodeConfig[i].devID = 0;
		}
	}
}

void ConcentorSetConfigCheck(void)
{
	if(nodeParaSetting == false)
		return;
	ConcenterSendRemoteSeting(GetRadioSrcAddr(), GetRadioDstAddr());
}


//***********************************************************************************
// brief:   send time to the node immediately
// 
// parameter: 
// srcAddr:	the concenter radio addr
// dstAddr:	the node radio addr
//***********************************************************************************
void ConcenterRadioSendSniff(uint32_t srcAddr, uint32_t dstAddr)
{

	protocalTxBuf.command	= RADIO_PRO_CMD_SPE_BROCAST;
	protocalTxBuf.dstAddr	= dstAddr;
	protocalTxBuf.srcAddr	= srcAddr;
	protocalTxBuf.len 		= 10+1;
	protocalTxBuf.load[0]	= brocastTimes;
	brocastTimes --;

	SetRadioDstAddr(dstAddr);

    ClearRadioSendBuf();
    concenterRemainderCache = EASYLINK_MAX_DATA_LENGTH;

    if(RadioCopyPacketToBuf(((uint8_t*)&protocalTxBuf), protocalTxBuf.len, 0, 0, EASYLINK_MAX_DATA_LENGTH - concenterRemainderCache) == true)
	    concenterRemainderCache -= protocalTxBuf.len;
}


#if 0
//***********************************************************************************
// brief:   send the packet to wakeup sensor
//
// parameter:
// srcAddr: the concenter radio addr
// dstAddr: the node radio addr

//***********************************************************************************
void SensorTxLoseAlarm(void){

    uint8_t temp;
    temp = 0;

    protocalTxBuf.command   = RADIO_PRO_CMD_LOSE_ALARM;
    protocalTxBuf.dstAddr   = GetRadioDstAddr();
    protocalTxBuf.srcAddr   = GetRadioSrcAddr();

    protocalTxBuf.len = 10 + temp;

    RadioSendPacket((uint8_t*)&protocalTxBuf, protocalTxBuf.len, 0, 0);

}




#endif

