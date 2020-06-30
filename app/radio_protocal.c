/*
* @Author: justfortest
* @Date:   2017-12-26 16:36:20
* @Last Modified by:   zxt
* @Last Modified time: 2020-06-30 17:21:32
*/
#include "../general.h"

#include "../APP/nodeApp.h"
#include "../APP/systemApp.h"
#include "../APP/radio_protocal.h"
#include "../APP/concenterApp.h"
#include "../radio_app/node_strategy.h"
#include "../radio_app/radio_app.h"

extern Semaphore_Handle recAckSemHandle;

radio_protocal_t   protocalTxBuf;

uint8_t     concenterRemainderCache;
bool nodeParaSetting = 0;

#define 		CMD_EVENT_MAX	64
uint16_t cmdType, cmdTypeWithRespon, cmdTypeGroud;
uint32_t cmdEvent, cmdEventWithRespon, cmdEventGroud;
uint32_t groundAddr;

uint16_t sendRetryTimes;
#define         RETRY_TIMES     3

#ifdef ZKS_S6_6_WOR_G
#define ADD_TO_GROUP      "Add group"
#define DELETE_FROM_GROUP "Delete group"
#define TERM_TEST         "Term test"
#define GROUP_TEST        "Group test"
#define TERM_CLOSE         ""
static void log_opration_record(uint8_t cmd,uint32_t deviceId,uint32_t groupId)
{
    uint8_t buff[32] = {0},index = 0;
   switch(cmd)
   {
      case RADIO_PRO_CMD_TERM_ADD_TO_GROUP:
           sprintf((char*)buff,"%s","Add group ");
           break;
      case RADIO_PRO_CMD_TERM_DELETE_FROM_GROUP:
          sprintf((char*)buff,"%s","Delete group ");
          break;
      case RADIO_PRO_CMD_TERM_TEST:
          sprintf((char*)buff,"%s","Term test ");
          break;
      case RADIO_PRO_CMD_GROUP_TEST:
          sprintf((char*)buff,"%s","Group test ");
          break;
      case RADIO_PRO_CMD_TERM_CLOSE_CTROL:
          sprintf((char*)buff,"%s","Term close ");
          break;
      case RADIO_PRO_CMD_TERM_OPEN_CTROL:
          sprintf((char*)buff,"%s","Term open ");
          break;
      case RADIO_PRO_CMD_GROUP_CLOSE_CTROL:
          sprintf((char*)buff,"%s","Group close ");
          break;
      case RADIO_PRO_CMD_GROUP_OPEN_CTROL:
          sprintf((char*)buff,"%s","Group open ");
          break;
      case RADIO_PRO_CMD_TERM_UNLOCKING:
          sprintf((char*)buff,"%s","Term unlock ");
          break;
      case RADIO_PRO_CMD_GROUP_UNLOCKING:
          sprintf((char*)buff,"%s","Group unlock ");
          break;
      case RADIO_PRO_CMD_GROUP_POWER_HIGH:
          sprintf((char*)buff,"%s","G Power high ");
          break;
      case RADIO_PRO_CMD_GROUP_POWER_MID:
          sprintf((char*)buff,"%s","G Power mid ");
          break;
      case RADIO_PRO_CMD_GROUP_POWER_LOW:
          sprintf((char*)buff,"%s","G Power low ");
          break;
      case RADIO_PRO_CMD_TERM_POWER_HIGH:
          sprintf((char*)buff,"%s","T Power high ");
          break;
      case RADIO_PRO_CMD_TERM_POWER_MID:
          sprintf((char*)buff,"%s","T Power mid ");
          break;
      case RADIO_PRO_CMD_TERM_POWER_LOW:
          sprintf((char*)buff,"%s","T Power low ");
          break;

      case RADIO_PRO_CMD_FIXED_TERM_SUBDUE_START:
           sprintf((char*)buff,"%s","Term subdue start ");
          break;
      case RADIO_PRO_CMD_FIXED_TERM_SUBDUE_STOP:
           sprintf((char*)buff,"%s","Term subdue stop ");
          break;

      case RADIO_PRO_CMD_GROUP_SUBDUE_START:
          sprintf((char*)buff,"%s","group subdue start ");
          break;
      case RADIO_PRO_CMD_GROUP_SUBDUE_STOP:
          sprintf((char*)buff,"%s","group subdue stop ");
          break;
      case RADIO_PRO_CMD_ALL_SUBDUE_START:
          sprintf((char*)buff,"%s","All subdue 制服所有");
          break;

      default:
	      return;


   }
   index = strlen((char*)buff);
   buff[index++] =  'T';
   buff[index++] =   deviceId/1000+0x30;
   buff[index++]  =  deviceId%1000/100+0x30;
   buff[index++]  =  deviceId%100/10+0x30;
   buff[index++]  =  deviceId%10 + 0x30;
   buff[index++] =  'G';
   buff[index++] =   groupId/1000+0x30;
   buff[index++]  =  groupId%1000/100+0x30;
   buff[index++]  =  groupId%100/10+0x30;
   buff[index++]  =  groupId%10 + 0x30;
   buff[index++]  =  '\n';
   Flash_log(buff);
}
#endif

void RadioCmdProcess(uint32_t cmdType, uint32_t dstDev, uint32_t ground, uint32_t srcDev)
{
	switch(cmdType){

#ifdef S_C

		case RADIO_PRO_CMD_TERM_ADD_TO_GROUP:
			if(dstDev == GetRadioSrcAddr()){
				GroudAddrSet(ground);
				g_rSysConfigInfo.customId[0] = (uint8_t)(ground >> 24);
				g_rSysConfigInfo.customId[1] = (uint8_t)(ground >> 16);
				g_rSysConfigInfo.customId[2] = (uint8_t)(ground >> 8);
				g_rSysConfigInfo.customId[3] = (uint8_t)(ground);
				SoundEventSet(SOUND_TYPE_SET_GROUND_SUSCESS);
				Sys_event_post(SYSTEMAPP_EVT_STORE_SYS_CONFIG);
			}
		break;


		case RADIO_PRO_CMD_TERM_DELETE_FROM_GROUP:
			if(dstDev == GetRadioSrcAddr()){
				SoundEventSet(SOUND_TYPE_DI);
				GroudAddrSet(INVALID_GROUND);
				g_rSysConfigInfo.customId[0] = (uint8_t)(INVALID_GROUND >> 24);
				g_rSysConfigInfo.customId[1] = (uint8_t)(INVALID_GROUND >> 16);
				g_rSysConfigInfo.customId[2] = (uint8_t)(INVALID_GROUND >> 8);
				g_rSysConfigInfo.customId[3] = (uint8_t)(INVALID_GROUND);
				Sys_event_post(SYSTEMAPP_EVT_STORE_SYS_CONFIG);
			}
		break;


		case RADIO_PRO_CMD_TERM_TEST:
			if(dstDev == GetRadioSrcAddr()){
				SoundEventSet(SOUND_TYPE_SINGLE_TEST);
			}
		break;


		case RADIO_PRO_CMD_GROUP_TEST:
			if(ground == GroudAddrGet()){
				SoundEventSet(SOUND_TYPE_SINGLE_TEST);
			}
		break;


		case RADIO_PRO_CMD_TERM_CLOSE_CTROL:
			if(dstDev == GetRadioSrcAddr()){
				SoundEventSet(SOUND_TYPE_DI);
				electricshockEnable = 0;
				ElectricShockPowerDisable();
				EletricPulseSetTime_S(0);
				g_rSysConfigInfo.electricFunc &= 0xffffffff^ELE_FUNC_ENABLE_SHOCK;
				Sys_event_post(SYSTEMAPP_EVT_STORE_SYS_CONFIG);
			}
		break;


		case RADIO_PRO_CMD_TERM_OPEN_CTROL:
			if(dstDev == GetRadioSrcAddr()){
				SoundEventSet(SOUND_TYPE_CONTROL_ENABLE);
				electricshockEnable = 1;
				// ElectricShockPowerEnable();
				g_rSysConfigInfo.electricFunc |= ELE_FUNC_ENABLE_SHOCK;
				Sys_event_post(SYSTEMAPP_EVT_STORE_SYS_CONFIG);
			}
		break;


		case RADIO_PRO_CMD_GROUP_CLOSE_CTROL:
			if(ground == GroudAddrGet()){
				SoundEventSet(SOUND_TYPE_DI);
				electricshockEnable = 0;
				ElectricShockPowerDisable();
				EletricPulseSetTime_S(0);
				g_rSysConfigInfo.electricFunc &= 0xffffffff^ELE_FUNC_ENABLE_SHOCK;
				Sys_event_post(SYSTEMAPP_EVT_STORE_SYS_CONFIG);
			}
		break;


		case RADIO_PRO_CMD_GROUP_OPEN_CTROL:
			if(ground == GroudAddrGet()){
				SoundEventSet(SOUND_TYPE_CONTROL_ENABLE);
				electricshockEnable = 1;
				// ElectricShockPowerEnable();
				g_rSysConfigInfo.electricFunc |= ELE_FUNC_ENABLE_SHOCK;
				Sys_event_post(SYSTEMAPP_EVT_STORE_SYS_CONFIG);
			}
		break;


		case RADIO_PRO_CMD_TERM_UNLOCKING:
			if(dstDev == GetRadioSrcAddr()){
				SoundEventSet(SOUND_TYPE_DI);
				eleShock_set(ELE_MOTO_ENABLE, 1);
				Task_sleep(3000 * CLOCK_UNIT_MS);
				eleShock_set(ELE_MOTO_ENABLE, 0);
			}
		break;


		case RADIO_PRO_CMD_GROUP_UNLOCKING:
			if(ground == GroudAddrGet()){
				SoundEventSet(SOUND_TYPE_DI);
				eleShock_set(ELE_MOTO_ENABLE, 1);
				Task_sleep(3000 * CLOCK_UNIT_MS);
				eleShock_set(ELE_MOTO_ENABLE, 0);
			}
		break;


		case RADIO_PRO_CMD_GROUP_POWER_HIGH:
			if(ground == GroudAddrGet()){
				SoundEventSet(SOUND_TYPE_SET_POWER_SUSCESS);
				ElectricShockLevelSet(ELECTRIC_HIGH_LEVEL);
				g_rSysConfigInfo.electricLevel = ELECTRIC_HIGH_LEVEL;
				Sys_event_post(SYSTEMAPP_EVT_STORE_SYS_CONFIG);
			}
		break;


		case RADIO_PRO_CMD_GROUP_POWER_MID:
			if(ground == GroudAddrGet()){
				SoundEventSet(SOUND_TYPE_SET_POWER_SUSCESS);
				ElectricShockLevelSet(ELECTRIC_MID_LEVEL);
				g_rSysConfigInfo.electricLevel = ELECTRIC_MID_LEVEL;
				Sys_event_post(SYSTEMAPP_EVT_STORE_SYS_CONFIG);
			}
		break;


		case RADIO_PRO_CMD_GROUP_POWER_LOW:
			if(ground == GroudAddrGet()){
				SoundEventSet(SOUND_TYPE_SET_POWER_SUSCESS);
				ElectricShockLevelSet(ELECTRIC_LOW_LEVEL);
				g_rSysConfigInfo.electricLevel = ELECTRIC_LOW_LEVEL;
				Sys_event_post(SYSTEMAPP_EVT_STORE_SYS_CONFIG);
			}
		break;


		case RADIO_PRO_CMD_TERM_POWER_HIGH:
			if(dstDev == GetRadioSrcAddr()){
				SoundEventSet(SOUND_TYPE_SET_POWER_SUSCESS);
				ElectricShockLevelSet(ELECTRIC_HIGH_LEVEL);
				g_rSysConfigInfo.electricLevel = ELECTRIC_HIGH_LEVEL;
				Sys_event_post(SYSTEMAPP_EVT_STORE_SYS_CONFIG);
			}
		break;


		case RADIO_PRO_CMD_TERM_POWER_MID:
			if(dstDev == GetRadioSrcAddr()){
				SoundEventSet(SOUND_TYPE_SET_POWER_SUSCESS);
				ElectricShockLevelSet(ELECTRIC_MID_LEVEL);
				g_rSysConfigInfo.electricLevel = ELECTRIC_MID_LEVEL;
				Sys_event_post(SYSTEMAPP_EVT_STORE_SYS_CONFIG);
			}
		break;


		case RADIO_PRO_CMD_TERM_POWER_LOW:
			if(dstDev == GetRadioSrcAddr()){
				SoundEventSet(SOUND_TYPE_SET_POWER_SUSCESS);
				ElectricShockLevelSet(ELECTRIC_LOW_LEVEL);
				g_rSysConfigInfo.electricLevel = ELECTRIC_LOW_LEVEL;
				Sys_event_post(SYSTEMAPP_EVT_STORE_SYS_CONFIG);
			}
		break;


		case RADIO_PRO_CMD_FIXED_TERM_SUBDUE_START:
			if(dstDev == GetRadioSrcAddr()){
				SoundEventSet(SOUND_TYPE_DI_DI);
				EletricPulseSetTime_S(ELECTRIC_SHOCK_TIME);
			}
		break;


		case RADIO_PRO_CMD_FIXED_TERM_SUBDUE_STOP:
			if(dstDev == GetRadioSrcAddr()){
				SoundEventSet(SOUND_TYPE_DI_DI);
				EletricPulseSetTime_S(0);
			}
		break;


		case RADIO_PRO_CMD_GROUP_SUBDUE_START:
			if(ground == GroudAddrGet()){
				SoundEventSet(SOUND_TYPE_DI_DI);
				EletricPulseSetTime_S(ELECTRIC_SHOCK_TIME);
			}
		break;


		case RADIO_PRO_CMD_GROUP_SUBDUE_STOP:
			if(ground == GroudAddrGet()){
				SoundEventSet(SOUND_TYPE_DI_DI);
				EletricPulseSetTime_S(0);
			}
		break;


		case RADIO_PRO_CMD_ALL_SUBDUE_START:
			EletricPulseSetTime_S(ELECTRIC_SHOCK_TIME);
			SoundEventSet(SOUND_TYPE_DI_DI_DI);
		break;

		case RADIO_PRO_CMD_TERM_CLOSE_BLOCKING:
			if(dstDev == GetRadioSrcAddr()){
				g_rSysConfigInfo.electricFunc &= 0xffffffff^ELE_FUNC_ENABLE_PREVENT_INSERT;
				SoundEventSet(SOUND_TYPE_DI_DI_DI);
				Sys_event_post(SYSTEMAPP_EVT_STORE_SYS_CONFIG);
			}
		break;


		case RADIO_PRO_CMD_TERM_OPEN_BLOCKING:
			if(dstDev == GetRadioSrcAddr()){
				g_rSysConfigInfo.electricFunc |= ELE_FUNC_ENABLE_PREVENT_INSERT;
				SoundEventSet(SOUND_TYPE_DI_DI_DI);
				Sys_event_post(SYSTEMAPP_EVT_STORE_SYS_CONFIG);
			}
		break;


		case RADIO_PRO_CMD_OPEN_PREVENT_ESCAPE:
			g_rSysConfigInfo.electricFunc |= ELE_FUNC_ENABLE_PREVENT_ESCAPE;
			SoundEventSet(SOUND_TYPE_DI_DI_DI);
			Sys_event_post(SYSTEMAPP_EVT_STORE_SYS_CONFIG);
		break;


		case RADIO_PRO_CMD_CLOSE_PREVENT_ESCAPE:
			g_rSysConfigInfo.electricFunc &= 0xffffffff^ELE_FUNC_ENABLE_PREVENT_ESCAPE;
			SoundEventSet(SOUND_TYPE_DI_DI_DI);
			Sys_event_post(SYSTEMAPP_EVT_STORE_SYS_CONFIG);
		break;

		case RADIO_PRO_CMD_OPEN_TERMINAL_PREVENT_ESCAPE:
			if(dstDev == GetRadioSrcAddr()){
				g_rSysConfigInfo.electricFunc |= ELE_FUNC_ENABLE_PREVENT_ESCAPE;
				SoundEventSet(SOUND_TYPE_DI_DI_DI);
				Sys_event_post(SYSTEMAPP_EVT_STORE_SYS_CONFIG);
			}
		break;


		case RADIO_PRO_CMD_CLOSE_TERMINAL_PREVENT_ESCAPE:
			if(dstDev == GetRadioSrcAddr()){
				g_rSysConfigInfo.electricFunc &= 0xffffffff^ELE_FUNC_ENABLE_PREVENT_ESCAPE;
				SoundEventSet(SOUND_TYPE_DI_DI_DI);
				Sys_event_post(SYSTEMAPP_EVT_STORE_SYS_CONFIG);
			}
		break;

		case RADIO_PRO_CMD_OPEN_GROUP_PREVENT_ESCAPE:
			if(ground == GroudAddrGet()){
				g_rSysConfigInfo.electricFunc |= ELE_FUNC_ENABLE_PREVENT_ESCAPE;
				SoundEventSet(SOUND_TYPE_DI_DI_DI);
				Sys_event_post(SYSTEMAPP_EVT_STORE_SYS_CONFIG);
			}
		break;


		case RADIO_PRO_CMD_CLOSE_GROUP_PREVENT_ESCAPE:
			if(ground == GroudAddrGet()){
				g_rSysConfigInfo.electricFunc &= 0xffffffff^ELE_FUNC_ENABLE_PREVENT_ESCAPE;
				SoundEventSet(SOUND_TYPE_DI_DI_DI);
				Sys_event_post(SYSTEMAPP_EVT_STORE_SYS_CONFIG);
			}
		break;


#endif //S_C

#ifdef S_G
		case RADIO_CMD_DESTROY_TYPE:
		insertAlarm(srcDev, ALARM_TYPE_DESTORY);
		break;

		case RADIO_CMD_LOW_VOL_TYPE:
		insertAlarm(srcDev, ALARM_TYPE_LOW_POWER);
		// Menu_low_power_display(srcDev);
		break;

		case RADIO_CMD_INSERT_TYPE:
		insertAlarm(srcDev, ALARM_TYPE_UNWEAR);
		// Menu_not_wearing_well_display(srcDev);
		break;


		case RADIO_PRO_CMD_ALL_RESP:

		Semaphore_post(recAckSemHandle);
		sendRetryTimes = 0;
		RadioCmdClearWithRespon();
		break;
#endif //S_G
	}
}
#ifdef S_C

#define ESCAPE_RSSI		(-60)
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
    uint16_t cmdType;
    uint16_t remaindTimes;
    uint32_t gourndTemp;


    TxFrameRecord_t rxSensorDataAckRecord;
    memset(&rxSensorDataAckRecord, 0, sizeof(TxFrameRecord_t));

	len			= protocalRxPacket->len;

    NodeStrategyBuffClear();

	// this buf may be include several message
	bufTemp		= (radio_protocal_t *)protocalRxPacket->payload;

	// SetRadioDstAddr(bufTemp->srcAddr);
	if(bufTemp->srcAddr == RADIO_CONTROLER_ADDRESS){
		if(protocalRxPacket->rssi > ESCAPE_RSSI)
			escapeTimeCnt = 0;
	}

	HIBYTE_ZKS(remaindTimes) = bufTemp->load[0];
	LOBYTE_ZKS(remaindTimes) = bufTemp->load[1];
	HIBYTE_ZKS(cmdType)      = bufTemp->load[2];
	LOBYTE_ZKS(cmdType)      = bufTemp->load[3];

    HIBYTE_ZKS(HIWORD_ZKS(gourndTemp)) = bufTemp->load[4];
    LOBYTE_ZKS(HIWORD_ZKS(gourndTemp)) = bufTemp->load[5];
    HIBYTE_ZKS(LOWORD_ZKS(gourndTemp)) = bufTemp->load[6];
    LOBYTE_ZKS(LOWORD_ZKS(gourndTemp)) = bufTemp->load[7];

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

			case RADIO_PRO_CMD_SINGLE:
			case RADIO_PRO_CMD_GROUND:
				RadioCmdProcess(cmdType, bufTemp->dstAddr, gourndTemp, bufTemp->srcAddr);
				if(bufTemp->dstAddr == GetRadioSrcAddr()){
					// Task_sleep(remaindTimes*BROCAST_TIME_MS*CLOCK_UNIT_MS);
					// RadioCmdSetWithNoRes(RADIO_PRO_CMD_ALL_RESP, bufTemp->srcAddr);
				}
			break;

			
			case RADIO_PRO_CMD_SINGLE_WITH_NO_RESP:
			case RADIO_PRO_CMD_GROUND_WITH_NO_RESP:
				RadioCmdProcess(cmdType, bufTemp->dstAddr, gourndTemp, bufTemp->srcAddr);
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
void RadioSendWithResp(uint16_t cmdType)
{
	uint32_t addrTemp;

	protocalTxBuf.command	= RADIO_PRO_CMD_SINGLE;
	protocalTxBuf.dstAddr	= GetRadioDstAddr();
	protocalTxBuf.srcAddr	= GetRadioSrcAddr();
	protocalTxBuf.len 		= 10+8;

	SetRadioDstAddr(protocalTxBuf.dstAddr);

	protocalTxBuf.load[0] 	= HIBYTE_ZKS(brocastTimes);
    protocalTxBuf.load[1] 	= LOBYTE_ZKS(brocastTimes);
    protocalTxBuf.load[2] 	= HIBYTE_ZKS(cmdType);
    protocalTxBuf.load[3] 	= LOBYTE_ZKS(cmdType); 

    addrTemp = GroudAddrGet();
	protocalTxBuf.load[4] 	= HIBYTE_ZKS(HIWORD_ZKS(addrTemp));
    protocalTxBuf.load[5] 	= LOBYTE_ZKS(HIWORD_ZKS(addrTemp));
    protocalTxBuf.load[6] 	= HIBYTE_ZKS(LOWORD_ZKS(addrTemp));
    protocalTxBuf.load[7] 	= LOBYTE_ZKS(LOWORD_ZKS(addrTemp));     

    RadioCopyPacketToBuf(((uint8_t*)&protocalTxBuf), protocalTxBuf.len, 0, 0, 0);
}



//***********************************************************************************
// brief:   send insert event
// 
// parameter: 
// srcAddr:	the concenter radio addr
// dstAddr:	the node radio addr
//***********************************************************************************
void RadioSendWithNoResp(uint16_t cmdType)
{
	uint32_t addrTemp;

	protocalTxBuf.command	= RADIO_PRO_CMD_SINGLE_WITH_NO_RESP;
	protocalTxBuf.dstAddr	= GetRadioDstAddr();
	protocalTxBuf.srcAddr	= GetRadioSrcAddr();
	protocalTxBuf.len 		= 10+8;

	SetRadioDstAddr(protocalTxBuf.dstAddr);

	protocalTxBuf.load[0] 	= HIBYTE_ZKS(brocastTimes);
    protocalTxBuf.load[1] 	= LOBYTE_ZKS(brocastTimes);
    protocalTxBuf.load[2] 	= HIBYTE_ZKS(cmdType);
    protocalTxBuf.load[3] 	= LOBYTE_ZKS(cmdType); 

    addrTemp = GroudAddrGet();
	protocalTxBuf.load[4] 	= HIBYTE_ZKS(HIWORD_ZKS(addrTemp));
    protocalTxBuf.load[5] 	= LOBYTE_ZKS(HIWORD_ZKS(addrTemp));
    protocalTxBuf.load[6] 	= HIBYTE_ZKS(LOWORD_ZKS(addrTemp));
    protocalTxBuf.load[7] 	= LOBYTE_ZKS(LOWORD_ZKS(addrTemp));   

    RadioCopyPacketToBuf(((uint8_t*)&protocalTxBuf), protocalTxBuf.len, 0, 0, 0);
}


//***********************************************************************************
// brief:   send low vol event
// 
// parameter: 
// srcAddr:	the concenter radio addr
// dstAddr:	the node radio addr
//***********************************************************************************
void RadioSendGroundWithResp(uint16_t cmdType)
{
	uint32_t addrTemp;

	protocalTxBuf.command	= RADIO_PRO_CMD_GROUND;
	protocalTxBuf.dstAddr	= GetRadioDstAddr();
	protocalTxBuf.srcAddr	= GetRadioSrcAddr();
	protocalTxBuf.len 		= 10+8;

	SetRadioDstAddr(protocalTxBuf.dstAddr);

	protocalTxBuf.load[0] 	= HIBYTE_ZKS(brocastTimes);
    protocalTxBuf.load[1] 	= LOBYTE_ZKS(brocastTimes);
    protocalTxBuf.load[2] 	= HIBYTE_ZKS(cmdType);
    protocalTxBuf.load[3] 	= LOBYTE_ZKS(cmdType); 

    addrTemp = GroudAddrGet();
	protocalTxBuf.load[4] 	= HIBYTE_ZKS(HIWORD_ZKS(addrTemp));
    protocalTxBuf.load[5] 	= LOBYTE_ZKS(HIWORD_ZKS(addrTemp));
    protocalTxBuf.load[6] 	= HIBYTE_ZKS(LOWORD_ZKS(addrTemp));
    protocalTxBuf.load[7] 	= LOBYTE_ZKS(LOWORD_ZKS(addrTemp));     

    RadioCopyPacketToBuf(((uint8_t*)&protocalTxBuf), protocalTxBuf.len, 0, 0, 0);
}



//***********************************************************************************
// brief:   send insert event
// 
// parameter: 
// srcAddr:	the concenter radio addr
// dstAddr:	the node radio addr
//***********************************************************************************
void RadioSendGroundWithNoResp(uint16_t cmdType)
{
	uint32_t addrTemp;

	protocalTxBuf.command	= RADIO_PRO_CMD_GROUND_WITH_NO_RESP;
	protocalTxBuf.dstAddr	= GetRadioDstAddr();
	protocalTxBuf.srcAddr	= GetRadioSrcAddr();
	protocalTxBuf.len 		= 10+8;

	SetRadioDstAddr(protocalTxBuf.dstAddr);

	protocalTxBuf.load[0] 	= HIBYTE_ZKS(brocastTimes);
    protocalTxBuf.load[1] 	= LOBYTE_ZKS(brocastTimes);
    protocalTxBuf.load[2] 	= HIBYTE_ZKS(cmdType);
    protocalTxBuf.load[3] 	= LOBYTE_ZKS(cmdType); 

    addrTemp = GroudAddrGet();
	protocalTxBuf.load[4] 	= HIBYTE_ZKS(HIWORD_ZKS(addrTemp));
    protocalTxBuf.load[5] 	= LOBYTE_ZKS(HIWORD_ZKS(addrTemp));
    protocalTxBuf.load[6] 	= HIBYTE_ZKS(LOWORD_ZKS(addrTemp));
    protocalTxBuf.load[7] 	= LOBYTE_ZKS(LOWORD_ZKS(addrTemp));   

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
    uint16_t cmdType;
    uint16_t remaindTimes;
    uint32_t gourndTemp;



	concenterRemainderCache = EASYLINK_MAX_DATA_LENGTH;
	len                     = protocalRxPacket->len;
	bufTemp                 = (radio_protocal_t *)protocalRxPacket->payload;

	ClearRadioSendBuf();

	SetRadioDstAddr(bufTemp->srcAddr);
	
	HIBYTE_ZKS(remaindTimes) = bufTemp->load[0];
	LOBYTE_ZKS(remaindTimes) = bufTemp->load[1];
	HIBYTE_ZKS(cmdType)      = bufTemp->load[2];
	LOBYTE_ZKS(cmdType)      = bufTemp->load[3];

    HIBYTE_ZKS(HIWORD_ZKS(gourndTemp)) = bufTemp->load[4];
    LOBYTE_ZKS(HIWORD_ZKS(gourndTemp)) = bufTemp->load[5];
    HIBYTE_ZKS(LOWORD_ZKS(gourndTemp)) = bufTemp->load[6];
    LOBYTE_ZKS(LOWORD_ZKS(gourndTemp)) = bufTemp->load[7];

	while(len)
	{
		// the receive data is not integrated
		if((bufTemp->len > len) || (bufTemp->len == 0) || (len == 0))
			break;
		
		len 	-= bufTemp->len;
		switch(bufTemp->command)
		{
			case RADIO_PRO_CMD_SINGLE:
			case RADIO_PRO_CMD_SINGLE_WITH_NO_RESP:
			case RADIO_PRO_CMD_GROUND:
			case RADIO_PRO_CMD_GROUND_WITH_NO_RESP:
				RadioCmdProcess(cmdType, bufTemp->srcAddr, gourndTemp, bufTemp->srcAddr);
			break;



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



uint32_t IntToHex(uint32_t iData)
{
	uint32_t hexData = 0;
	uint8_t i;
	for(i = 0; i<8; i++){
		hexData |= ((iData%10) << (4*i));
		iData /= 10;
	}
	return hexData;

}


void GroudAddrSet(uint32_t ground)
{
	groundAddr = ground;
}

uint32_t GroudAddrGet(void)
{
	return groundAddr;
}

// 发送不需要回复的指令,单次发送
void RadioCmdSetWithNoRes(uint16_t cmd, uint32_t dstAddr)
{
	cmdType = cmd;
	cmdEvent |= (0x1 << cmd);
	if(dstAddr){
		SetRadioDstAddr(dstAddr);
	}
	RadioSingleSend();
}

bool RadioCmdSetWithNoResponBrocast(uint16_t cmd, uint32_t dstAddr)
{
	if(dstAddr){
		SetRadioDstAddr(dstAddr);
	}
	cmdTypeGroud = cmd;
	cmdEventGroud |= (0x1 << cmd);
	RadioSendBrocast();

	return true;
}


// 清除不需要回复的指令
void RadioCmdClearWithNoRespon(void)
{
	uint8_t i;
	cmdEvent &= 0xffffffff ^ (0x1 << cmdType);
	cmdType = 0;
	if(cmdEvent){
		for(i = 0; i < CMD_EVENT_MAX; i++){
			if(cmdEvent & (0x1 << i)){
				cmdType = i;
				break;
			}
		}
		RadioSingleSend();
	}
}

uint32_t RadioWithNoResPack(void)
{
	RadioSendWithResp(cmdType);
	return cmdType;
}

// 发送不需要回复的群组指令，以广播的方式发出
bool RadioCmdSetWithNoRespon(uint16_t cmd, uint32_t dstAddr, uint32_t ground)
{
	dstAddr = IntToHex(dstAddr);
	ground  = IntToHex(ground);
	GroudAddrSet(ground);
	SetRadioDstAddr((g_rSysConfigInfo.customId[0] << 8) | g_rSysConfigInfo.customId[1]);
	cmdTypeGroud = cmd;
	cmdEventGroud |= (0x1 << cmd);
	RadioSendBrocast();

#ifdef ZKS_S6_6_WOR_G
    log_opration_record(cmd,dstAddr,ground);
#endif

	return true;
}

// 清除不需要回复的群组指令
void RadioCmdClearWithNoRespon_Groud(void)
{
	uint8_t i;

	cmdEventGroud &= 0xffffffff ^ (0x1 << cmdTypeGroud);
	cmdTypeGroud = 0;
	if(cmdEventGroud){
		for(i = 0; i < CMD_EVENT_MAX; i++){
			if(cmdEventGroud & (0x1 << i)){
				cmdTypeGroud = i;
				break;
			}
		}
		RadioSendBrocast();
	}
}

uint32_t RadioWithNoRes_GroudPack(void)
{
	RadioSendWithNoResp(cmdTypeGroud);
	return cmdTypeGroud;
}



// 发送的需要回复命令
bool RadioCmdSetWithRespon(uint16_t cmd, uint32_t dstAddr, uint32_t ground)
{
	dstAddr = IntToHex(dstAddr);
	ground  = IntToHex(ground);
	GroudAddrSet(ground);
	cmdTypeWithRespon = cmd;
	cmdEventWithRespon |= (0x1 << cmd);
	if(dstAddr){
		SetRadioDstAddr(dstAddr);
	}
	sendRetryTimes = RETRY_TIMES;
#ifdef S_G

	RadioSendBrocast();
	// Semaphore_pend(recAckSemHandle, BIOS_NO_WAIT);
	// WdtClear();
	// return Semaphore_pend(recAckSemHandle, 7 * CLOCK_UNIT_S);
#ifdef ZKS_S6_6_WOR_G
    log_opration_record(cmd,dstAddr,ground);
#endif
	return true;
#else
	RadioSend();
	return true;
#endif

}


void RadioCmdClearWithRespon(void)
{
	uint8_t i;
	if(sendRetryTimes == 0){
		cmdEventWithRespon &= 0xffffffff ^ (0x1 << cmdTypeWithRespon);
		cmdTypeWithRespon = 0;
		sendRetryTimes = RETRY_TIMES;

		if(cmdEventWithRespon){
			for(i = 0; i < CMD_EVENT_MAX; i++){
				if(cmdEventWithRespon & (0x1 << i)){
					cmdTypeWithRespon = i;
					break;
				}
			}
#ifdef S_G
			RadioSendBrocast();
#else
			RadioSend();
#endif
		}
	}
	else{
		if(cmdEventWithRespon){
		    sendRetryTimes--;
#ifdef S_G
			RadioSendBrocast();
#else
			RadioSend();
#endif
		}
	}
	
}

uint32_t RadioWithResPack(void)
{
	RadioSendGroundWithResp(cmdTypeWithRespon);
	return cmdTypeWithRespon;
}
