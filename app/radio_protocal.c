/*
* @Author: justfortest
* @Date:   2017-12-26 16:36:20
* @Last Modified by:   zxt
* @Last Modified time: 2020-07-29 21:00:04
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
uint64_t cmdEvent, cmdEventWithRespon, cmdEventGroud;
uint32_t groundAddr;

uint16_t sendRetryTimes;
#define         RETRY_TIMES     3

#ifdef ZKS_S6_6_WOR_G
#define ADD_TO_GROUP      "Add group"
#define DELETE_FROM_GROUP "Delete group"
#define TERM_TEST         "Term test"
#define GROUP_TEST        "Group test"
#define TERM_CLOSE         ""
/*
 #define     RADIO_PRO_CMD_TERM_ADD_TO_GROUP         5              //终端添加到组
#define     RADIO_PRO_CMD_TERM_DELETE_FROM_GROUP    6               //从组删除
#define     RADIO_PRO_CMD_TERM_TEST                 7               //终端测试
#define     RADIO_PRO_CMD_GROUP_TEST                8              //群组测试
#define     RADIO_PRO_CMD_TERM_CLOSE_CTROL          9              //关闭制服
#define     RADIO_PRO_CMD_TERM_OPEN_CTROL           10              //打开制服
#define     RADIO_PRO_CMD_GROUP_CLOSE_CTROL         11                //关组制服
#define     RADIO_PRO_CMD_GROUP_OPEN_CTROL          12            //开组制服
#define     RADIO_PRO_CMD_TERM_UNLOCKING            13              //终端解锁
#define     RADIO_PRO_CMD_GROUP_UNLOCKING           14               //群组解锁

#define     RADIO_PRO_CMD_GROUP_POWER_HIGH          15                //群组电压高
#define     RADIO_PRO_CMD_GROUP_POWER_MID           16              //群组电压中
#define     RADIO_PRO_CMD_GROUP_POWER_LOW           17              //群组电压低
#define     RADIO_PRO_CMD_TERM_POWER_HIGH           18              //终端电压高
#define     RADIO_PRO_CMD_TERM_POWER_MID            19               //终端电压中
#define     RADIO_PRO_CMD_TERM_POWER_LOW            20              //终端电压低

#define     RADIO_PRO_CMD_FIXED_TERM_SUBDUE_START   21              //定员制服
#define     RADIO_PRO_CMD_FIXED_TERM_SUBDUE_STOP    22              //定员停止

#define     RADIO_PRO_CMD_GROUP_SUBDUE_START        23               //群组制服
#define     RADIO_PRO_CMD_GROUP_SUBDUE_STOP         24            //群组停止

#define     RADIO_PRO_CMD_ALL_SUBDUE_START          25             //全员制服
//#define     RADIO_PRO_CMD_ALL_SUBDUE_STOP                        //定员停止
#define     RADIO_PRO_CMD_TERM_CLOSE_BLOCKING       26             //关闭防塞
#define     RADIO_PRO_CMD_TERM_OPEN_BLOCKING        27             //打开防塞

#define     RADIO_PRO_CMD_OPEN_PREVENT_ESCAPE        28             //打开防逃
#define     RADIO_PRO_CMD_CLOSE_PREVENT_ESCAPE       29             //关闭防逃


#define     RADIO_PRO_CMD_ALL_WAKEUP                30             //唤醒设备，用于防逃的

#define     RADIO_PRO_CMD_OPEN_TERMINAL_PREVENT_ESCAPE        31   //开终端防逃
#define     RADIO_PRO_CMD_CLOSE_TERMINAL_PREVENT_ESCAPE       32   //关终端防逃
#define     RADIO_PRO_CMD_OPEN_GROUP_PREVENT_ESCAPE           33   //开组防逃
#define     RADIO_PRO_CMD_CLOSE_GROUP_PREVENT_ESCAPE          34   //关组防逃
 */
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
           sprintf((char*)buff,"%s","All subdue");
          break;

      case RADIO_PRO_CMD_OPEN_TERMINAL_PREVENT_ESCAPE:
           sprintf((char*)buff,"%s","open Term escape");
           break;
      case RADIO_PRO_CMD_CLOSE_TERMINAL_PREVENT_ESCAPE:
           sprintf((char*)buff,"%s","close Term escape");
           break;
      case RADIO_PRO_CMD_OPEN_GROUP_PREVENT_ESCAPE:
           sprintf((char*)buff,"%s","open group escape");
           break;
      case RADIO_PRO_CMD_CLOSE_GROUP_PREVENT_ESCAPE:
           sprintf((char*)buff,"%s","close group escape");
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
				SoundEventSet(SOUND_TYPE_SET_GROUP_SUSCESS);
				Sys_event_post(SYSTEMAPP_EVT_STORE_SYS_CONFIG);
			}
		break;


		case RADIO_PRO_CMD_TERM_DELETE_FROM_GROUP:
			if(dstDev == GetRadioSrcAddr()){
				SoundEventSet(SOUND_TYPE_CLEAR_GROUP_SUSCESS);
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
				PreventiveInsertTest();
			}
		break;


		case RADIO_PRO_CMD_GROUP_TEST:
			if(ground == GroudAddrGet()){
				SoundEventSet(SOUND_TYPE_SINGLE_TEST);
				PreventiveInsertTest();
			}
		break;


		case RADIO_PRO_CMD_TERM_CLOSE_CTROL:
			if(dstDev == GetRadioSrcAddr()){
				SoundEventSet(SOUND_TYPE_CONTROL_DISABLE);
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
				SoundEventSet(SOUND_TYPE_CONTROL_DISABLE);
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
				SoundEventSet(SOUND_TYPE_UNLOCK);
				eleShock_set(ELE_MOTO_ENABLE, 1);
				// Task_sleep(3000 * CLOCK_UNIT_MS);
				// eleShock_set(ELE_MOTO_ENABLE, 0);
			}
		break;


		case RADIO_PRO_CMD_GROUP_UNLOCKING:
			if(ground == GroudAddrGet()){
				SoundEventSet(SOUND_TYPE_UNLOCK);
				eleShock_set(ELE_MOTO_ENABLE, 1);
				// Task_sleep(3000 * CLOCK_UNIT_MS);
				// eleShock_set(ELE_MOTO_ENABLE, 0);
			}
		break;


		case RADIO_PRO_CMD_GROUP_POWER_HIGH:
			if(ground == GroudAddrGet()){
				SoundEventSet(SOUND_TYPE_SET_POWER_HIGH_SUCCESS);
				ElectricShockLevelSet(ELECTRIC_HIGH_LEVEL);
				g_rSysConfigInfo.electricLevel = ELECTRIC_HIGH_LEVEL;
				Sys_event_post(SYSTEMAPP_EVT_STORE_SYS_CONFIG);
			}
		break;


		case RADIO_PRO_CMD_GROUP_POWER_MID:
			if(ground == GroudAddrGet()){
				SoundEventSet(SOUND_TYPE_SET_POWER_MID_SUCCESS);
				ElectricShockLevelSet(ELECTRIC_MID_LEVEL);
				g_rSysConfigInfo.electricLevel = ELECTRIC_MID_LEVEL;
				Sys_event_post(SYSTEMAPP_EVT_STORE_SYS_CONFIG);
			}
		break;


		case RADIO_PRO_CMD_GROUP_POWER_LOW:
			if(ground == GroudAddrGet()){
				SoundEventSet(SOUND_TYPE_SET_POWER_LOW_SUCCESS);
				ElectricShockLevelSet(ELECTRIC_LOW_LEVEL);
				g_rSysConfigInfo.electricLevel = ELECTRIC_LOW_LEVEL;
				Sys_event_post(SYSTEMAPP_EVT_STORE_SYS_CONFIG);
			}
		break;


		case RADIO_PRO_CMD_TERM_POWER_HIGH:
			if(dstDev == GetRadioSrcAddr()){
				SoundEventSet(SOUND_TYPE_SET_POWER_HIGH_SUCCESS);
				ElectricShockLevelSet(ELECTRIC_HIGH_LEVEL);
				g_rSysConfigInfo.electricLevel = ELECTRIC_HIGH_LEVEL;
				Sys_event_post(SYSTEMAPP_EVT_STORE_SYS_CONFIG);
			}
		break;


		case RADIO_PRO_CMD_TERM_POWER_MID:
			if(dstDev == GetRadioSrcAddr()){
				SoundEventSet(SOUND_TYPE_SET_POWER_MID_SUCCESS);
				ElectricShockLevelSet(ELECTRIC_MID_LEVEL);
				g_rSysConfigInfo.electricLevel = ELECTRIC_MID_LEVEL;
				Sys_event_post(SYSTEMAPP_EVT_STORE_SYS_CONFIG);
			}
		break;


		case RADIO_PRO_CMD_TERM_POWER_LOW:
			if(dstDev == GetRadioSrcAddr()){
				SoundEventSet(SOUND_TYPE_SET_POWER_LOW_SUCCESS);
				ElectricShockLevelSet(ELECTRIC_LOW_LEVEL);
				g_rSysConfigInfo.electricLevel = ELECTRIC_LOW_LEVEL;
				Sys_event_post(SYSTEMAPP_EVT_STORE_SYS_CONFIG);
			}
		break;


		case RADIO_PRO_CMD_FIXED_TERM_SUBDUE_START:
			if(dstDev == GetRadioSrcAddr()){
				SoundEventSet(SOUND_TYPE_SHOCK_START);
				EletricPulseSetTime_S(ELECTRIC_SHOCK_TIME);
			}
		break;


		case RADIO_PRO_CMD_FIXED_TERM_SUBDUE_STOP:
			if(dstDev == GetRadioSrcAddr()){
				SoundEventSet(SOUND_TYPE_SHOCK_STOP);
				EletricPulseSetTime_S(0);
			}
		break;


		case RADIO_PRO_CMD_GROUP_SUBDUE_START:
			if(ground == GroudAddrGet()){
				SoundEventSet(SOUND_TYPE_SHOCK_START);
				EletricPulseSetTime_S(ELECTRIC_SHOCK_TIME);
			}
		break;


		case RADIO_PRO_CMD_GROUP_SUBDUE_STOP:
			if(ground == GroudAddrGet()){
				SoundEventSet(SOUND_TYPE_SHOCK_STOP);
				EletricPulseSetTime_S(0);
			}
		break;


		case RADIO_PRO_CMD_ALL_SUBDUE_START:
			EletricPulseSetTime_S(ELECTRIC_SHOCK_TIME);
			SoundEventSet(SOUND_TYPE_SHOCK_START);
		break;

		case RADIO_PRO_CMD_TERM_CLOSE_BLOCKING:
			if(dstDev == GetRadioSrcAddr()){
				g_rSysConfigInfo.electricFunc &= 0xffffffff^ELE_FUNC_ENABLE_PREVENT_INSERT;
				SoundEventSet(SOUND_TYPE_INSERT_DETECT_DISABLE);
				Sys_event_post(SYSTEMAPP_EVT_STORE_SYS_CONFIG);
			}
		break;


		case RADIO_PRO_CMD_TERM_OPEN_BLOCKING:
			if(dstDev == GetRadioSrcAddr()){
				g_rSysConfigInfo.electricFunc |= ELE_FUNC_ENABLE_PREVENT_INSERT;
				SoundEventSet(SOUND_TYPE_INSERT_DETECT_ENABLE);
				Sys_event_post(SYSTEMAPP_EVT_STORE_SYS_CONFIG);
			}
		break;


		case RADIO_PRO_CMD_OPEN_PREVENT_ESCAPE:
			g_rSysConfigInfo.electricFunc |= ELE_FUNC_ENABLE_PREVENT_ESCAPE;
			SoundEventSet(SOUND_TYPE_OPEN_ESCAPE);
			Sys_event_post(SYSTEMAPP_EVT_STORE_SYS_CONFIG);
		break;


		case RADIO_PRO_CMD_CLOSE_PREVENT_ESCAPE:
			g_rSysConfigInfo.electricFunc &= 0xffffffff^ELE_FUNC_ENABLE_PREVENT_ESCAPE;
			SoundEventSet(SOUND_TYPE_CLOSE_ESCAPE);
			Sys_event_post(SYSTEMAPP_EVT_STORE_SYS_CONFIG);
		break;

		case RADIO_PRO_CMD_OPEN_TERMINAL_PREVENT_ESCAPE:
			if(dstDev == GetRadioSrcAddr()){
				g_rSysConfigInfo.electricFunc |= ELE_FUNC_ENABLE_PREVENT_ESCAPE;
				SoundEventSet(SOUND_TYPE_OPEN_ESCAPE);
				Sys_event_post(SYSTEMAPP_EVT_STORE_SYS_CONFIG);
			}
		break;


		case RADIO_PRO_CMD_CLOSE_TERMINAL_PREVENT_ESCAPE:
			if(dstDev == GetRadioSrcAddr()){
				g_rSysConfigInfo.electricFunc &= 0xffffffff^ELE_FUNC_ENABLE_PREVENT_ESCAPE;
				SoundEventSet(SOUND_TYPE_CLOSE_ESCAPE);
				Sys_event_post(SYSTEMAPP_EVT_STORE_SYS_CONFIG);
			}
		break;

		case RADIO_PRO_CMD_OPEN_GROUP_PREVENT_ESCAPE:
			if(ground == GroudAddrGet()){
				g_rSysConfigInfo.electricFunc |= ELE_FUNC_ENABLE_PREVENT_ESCAPE;
				SoundEventSet(SOUND_TYPE_OPEN_ESCAPE);
				Sys_event_post(SYSTEMAPP_EVT_STORE_SYS_CONFIG);
			}
		break;


		case RADIO_PRO_CMD_CLOSE_GROUP_PREVENT_ESCAPE:
			if(ground == GroudAddrGet()){
				g_rSysConfigInfo.electricFunc &= 0xffffffff^ELE_FUNC_ENABLE_PREVENT_ESCAPE;
				SoundEventSet(SOUND_TYPE_CLOSE_ESCAPE);
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
		// sendRetryTimes = 0;
		// RadioCmdClearWithRespon();
		break;
#endif //S_G
	}
}
#ifdef S_C

#define ESCAPE_RSSI		(-55)
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
    uint32_t dstAddr,srcAddr;

    dstAddr = *((uint32_t*)protocalRxPacket->dstAddr);
    TxFrameRecord_t rxSensorDataAckRecord;
    memset(&rxSensorDataAckRecord, 0, sizeof(TxFrameRecord_t));

	len			= protocalRxPacket->len;

    NodeStrategyBuffClear();

	// this buf may be include several message
	bufTemp		= (radio_protocal_t *)protocalRxPacket->payload;

	// SetRadioDstAddr(bufTemp->srcAddr);
	// if(srcAddr == RADIO_CONTROLER_ADDRESS){
		if(protocalRxPacket->rssi > ESCAPE_RSSI)
			escapeTimeCnt = 0;
	// }

	HIBYTE_ZKS(remaindTimes) = bufTemp->load[0];
	LOBYTE_ZKS(remaindTimes) = bufTemp->load[1];
	HIBYTE_ZKS(cmdType)      = bufTemp->load[2];
	LOBYTE_ZKS(cmdType)      = bufTemp->load[3];

    HIBYTE_ZKS(HIWORD_ZKS(gourndTemp)) = bufTemp->load[4];
    LOBYTE_ZKS(HIWORD_ZKS(gourndTemp)) = bufTemp->load[5];
    HIBYTE_ZKS(LOWORD_ZKS(gourndTemp)) = bufTemp->load[6];
    LOBYTE_ZKS(LOWORD_ZKS(gourndTemp)) = bufTemp->load[7];

    srcAddr = gourndTemp;

	// while(len)
	// {
		// the receive data is not integrated
		// if((bufTemp->len > len) || (bufTemp->len == 0))
			// return;
		
		// the resever length
		// len 	= 0;

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
				RadioCmdProcess(cmdType, dstAddr, gourndTemp, srcAddr);
				if(dstAddr == GetRadioSrcAddr()){
				// wait for the S_G send the same msg
					Task_sleep((remaindTimes+2)*BROCAST_TIME_MS*CLOCK_UNIT_MS);
					RadioCmdSetWithNoResponBrocast(RADIO_PRO_CMD_ALL_RESP, RADIO_CONTROLER_ADDRESS);
				}
			break;

			
			case RADIO_PRO_CMD_SINGLE_WITH_NO_RESP:
			case RADIO_PRO_CMD_GROUND_WITH_NO_RESP:
				// wait for the S_G send the same msg
				Task_sleep((remaindTimes+2)*BROCAST_TIME_MS*CLOCK_UNIT_MS);
				RadioCmdProcess(cmdType, dstAddr, gourndTemp, srcAddr);
			break;

			default:
			return;

		}
	// }
}


#endif // S_C


//***********************************************************************************
// brief:   send the timesyn ack to the strategy process
// 
// parameter: 
//***********************************************************************************
void NodeRadioSendSynReq(void)
{
	protocalTxBuf.command	= RADIO_PRO_CMD_SYN_TIME_REQ;
	// protocalTxBuf.dstAddr	= GetRadioDstAddr();
	// protocalTxBuf.srcAddr	= GetRadioSrcAddr();
	// protocalTxBuf.len 		= 10;

	SetRadioDstAddr(GetRadioDstAddr());
	// SetRadioDstAddr(protocalTxBuf.dstAddr);

    RadioCopyPacketToBuf(((uint8_t*)&protocalTxBuf), 9, 0, 0, 0);
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
	uint16_t volTemp;

	protocalTxBuf.command	= RADIO_PRO_CMD_SINGLE;
	// protocalTxBuf.dstAddr	= GetRadioDstAddr();
	// protocalTxBuf.srcAddr	= GetRadioSrcAddr();
	// protocalTxBuf.len 		= 10+8;

	SetRadioDstAddr(GetRadioDstAddr());

	protocalTxBuf.load[0] 	= HIBYTE_ZKS(brocastTimes);
    protocalTxBuf.load[1] 	= LOBYTE_ZKS(brocastTimes);
    protocalTxBuf.load[2] 	= HIBYTE_ZKS(cmdType);
    protocalTxBuf.load[3] 	= LOBYTE_ZKS(cmdType); 

    addrTemp = GroudAddrGet();
    if((cmdType == RADIO_CMD_DESTROY_TYPE) ||
    	(cmdType == RADIO_CMD_LOW_VOL_TYPE) ||
    	(cmdType == RADIO_CMD_INSERT_TYPE))
    	addrTemp = GetRadioSrcAddr();

	volTemp = Battery_get_voltage();

    protocalTxBuf.load[8] 	= HIBYTE_ZKS(volTemp);
    protocalTxBuf.load[9] 	= LOBYTE_ZKS(volTemp);

    if(cmdType == RADIO_PRO_CMD_ALL_RESP)
	    RadioCopyPacketToBuf(((uint8_t*)&protocalTxBuf), 11, 0, 0, 0);
	else
	    RadioCopyPacketToBuf(((uint8_t*)&protocalTxBuf), 9, 0, 0, 0);
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
	uint16_t volTemp;

	protocalTxBuf.command	= RADIO_PRO_CMD_SINGLE_WITH_NO_RESP;
	// protocalTxBuf.dstAddr	= GetRadioDstAddr();
	// protocalTxBuf.srcAddr	= GetRadioSrcAddr();
	// protocalTxBuf.len 		= 10+8;

	SetRadioDstAddr(GetRadioDstAddr());
	// SetRadioDstAddr(protocalTxBuf.dstAddr);

	protocalTxBuf.load[0] 	= HIBYTE_ZKS(brocastTimes);
    protocalTxBuf.load[1] 	= LOBYTE_ZKS(brocastTimes);
    protocalTxBuf.load[2] 	= HIBYTE_ZKS(cmdType);
    protocalTxBuf.load[3] 	= LOBYTE_ZKS(cmdType); 

    addrTemp = GroudAddrGet();

    if((cmdType == RADIO_CMD_DESTROY_TYPE) ||
    	(cmdType == RADIO_CMD_LOW_VOL_TYPE) ||
    	(cmdType == RADIO_PRO_CMD_ALL_RESP) ||
    	(cmdType == RADIO_CMD_INSERT_TYPE))
    	addrTemp = GetRadioSrcAddr();


	protocalTxBuf.load[4] 	= HIBYTE_ZKS(HIWORD_ZKS(addrTemp));
    protocalTxBuf.load[5] 	= LOBYTE_ZKS(HIWORD_ZKS(addrTemp));
    protocalTxBuf.load[6] 	= HIBYTE_ZKS(LOWORD_ZKS(addrTemp));
    protocalTxBuf.load[7] 	= LOBYTE_ZKS(LOWORD_ZKS(addrTemp));

    volTemp = Battery_get_voltage();

    protocalTxBuf.load[8] 	= HIBYTE_ZKS(volTemp);
    protocalTxBuf.load[9] 	= LOBYTE_ZKS(volTemp);

    if(cmdType == RADIO_PRO_CMD_ALL_RESP)
	    RadioCopyPacketToBuf(((uint8_t*)&protocalTxBuf), 11, 0, 0, 0);
	else
	    RadioCopyPacketToBuf(((uint8_t*)&protocalTxBuf), 9, 0, 0, 0);
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
	uint16_t volTemp;

	protocalTxBuf.command	= RADIO_PRO_CMD_GROUND;
	// protocalTxBuf.dstAddr	= GetRadioDstAddr();
	// protocalTxBuf.srcAddr	= GetRadioSrcAddr();
	// protocalTxBuf.len 		= 10+8;

	SetRadioDstAddr(GetRadioDstAddr());
	// SetRadioDstAddr(protocalTxBuf.dstAddr);

	protocalTxBuf.load[0] 	= HIBYTE_ZKS(brocastTimes);
    protocalTxBuf.load[1] 	= LOBYTE_ZKS(brocastTimes);
    protocalTxBuf.load[2] 	= HIBYTE_ZKS(cmdType);
    protocalTxBuf.load[3] 	= LOBYTE_ZKS(cmdType); 

    addrTemp = GroudAddrGet();

    if((cmdType == RADIO_CMD_DESTROY_TYPE) ||
    	(cmdType == RADIO_CMD_LOW_VOL_TYPE) ||
    	(cmdType == RADIO_CMD_INSERT_TYPE))
    	addrTemp = GetRadioSrcAddr();

	protocalTxBuf.load[4] 	= HIBYTE_ZKS(HIWORD_ZKS(addrTemp));
    protocalTxBuf.load[5] 	= LOBYTE_ZKS(HIWORD_ZKS(addrTemp));
    protocalTxBuf.load[6] 	= HIBYTE_ZKS(LOWORD_ZKS(addrTemp));
    protocalTxBuf.load[7] 	= LOBYTE_ZKS(LOWORD_ZKS(addrTemp));     

    volTemp = Battery_get_voltage();

    protocalTxBuf.load[8] 	= HIBYTE_ZKS(volTemp);
    protocalTxBuf.load[9] 	= LOBYTE_ZKS(volTemp);

    if(cmdType == RADIO_PRO_CMD_ALL_RESP)
	    RadioCopyPacketToBuf(((uint8_t*)&protocalTxBuf), 11, 0, 0, 0);
	else
	    RadioCopyPacketToBuf(((uint8_t*)&protocalTxBuf), 9, 0, 0, 0);
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
	uint16_t volTemp;

	protocalTxBuf.command	= RADIO_PRO_CMD_GROUND_WITH_NO_RESP;
	// protocalTxBuf.dstAddr	= GetRadioDstAddr();
	// protocalTxBuf.srcAddr	= GetRadioSrcAddr();
	// protocalTxBuf.len 		= 10+8;

	SetRadioDstAddr(GetRadioDstAddr());
	// SetRadioDstAddr(protocalTxBuf.dstAddr);

	protocalTxBuf.load[0] 	= HIBYTE_ZKS(brocastTimes);
    protocalTxBuf.load[1] 	= LOBYTE_ZKS(brocastTimes);
    protocalTxBuf.load[2] 	= HIBYTE_ZKS(cmdType);
    protocalTxBuf.load[3] 	= LOBYTE_ZKS(cmdType); 

    addrTemp = GroudAddrGet();

    if((cmdType == RADIO_CMD_DESTROY_TYPE) ||
    	(cmdType == RADIO_CMD_LOW_VOL_TYPE) ||
    	(cmdType == RADIO_CMD_INSERT_TYPE))
    	addrTemp = GetRadioSrcAddr();

	protocalTxBuf.load[4] 	= HIBYTE_ZKS(HIWORD_ZKS(addrTemp));
    protocalTxBuf.load[5] 	= LOBYTE_ZKS(HIWORD_ZKS(addrTemp));
    protocalTxBuf.load[6] 	= HIBYTE_ZKS(LOWORD_ZKS(addrTemp));
    protocalTxBuf.load[7] 	= LOBYTE_ZKS(LOWORD_ZKS(addrTemp));   

    volTemp = Battery_get_voltage();

    protocalTxBuf.load[8] 	= HIBYTE_ZKS(volTemp);
    protocalTxBuf.load[9] 	= LOBYTE_ZKS(volTemp);

    if(cmdType == RADIO_PRO_CMD_ALL_RESP)
	    RadioCopyPacketToBuf(((uint8_t*)&protocalTxBuf), 11, 0, 0, 0);
	else
	    RadioCopyPacketToBuf(((uint8_t*)&protocalTxBuf), 9, 0, 0, 0);
}

uint16_t testTermVol;

uint16_t GetTestTermVol(void)
{
    uint16_t batValue = 0;
    batValue = ((testTermVol-BAT_VOLTAGE_LOW )*100)/ (BAT_VOLTAGE_FULL-BAT_VOLTAGE_LOW);
	return batValue;
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
    uint32_t dstAddr,srcAddr;

    dstAddr = *((uint32_t*)protocalRxPacket->dstAddr);


	concenterRemainderCache = EASYLINK_MAX_DATA_LENGTH;
	len                     = protocalRxPacket->len;
	bufTemp                 = (radio_protocal_t *)protocalRxPacket->payload;

	ClearRadioSendBuf();

	SetRadioDstAddr(srcAddr);
	
	HIBYTE_ZKS(remaindTimes) = bufTemp->load[0];
	LOBYTE_ZKS(remaindTimes) = bufTemp->load[1];
	HIBYTE_ZKS(cmdType)      = bufTemp->load[2];
	LOBYTE_ZKS(cmdType)      = bufTemp->load[3];

    HIBYTE_ZKS(HIWORD_ZKS(gourndTemp)) = bufTemp->load[4];
    LOBYTE_ZKS(HIWORD_ZKS(gourndTemp)) = bufTemp->load[5];
    HIBYTE_ZKS(LOWORD_ZKS(gourndTemp)) = bufTemp->load[6];
    LOBYTE_ZKS(LOWORD_ZKS(gourndTemp)) = bufTemp->load[7];

    HIBYTE_ZKS(testTermVol) = bufTemp->load[8];
	LOBYTE_ZKS(testTermVol) = bufTemp->load[9];
    srcAddr = gourndTemp;
	// while(len)
	// {
		// the receive data is not integrated
		// if((bufTemp->len > len) || (bufTemp->len == 0) || (len == 0))
		// 	break;
		
		// len 	-= bufTemp->len;
		switch(bufTemp->command)
		{
			case RADIO_PRO_CMD_SINGLE:
			case RADIO_PRO_CMD_SINGLE_WITH_NO_RESP:
			case RADIO_PRO_CMD_GROUND:
			case RADIO_PRO_CMD_GROUND_WITH_NO_RESP:
				RadioCmdProcess(cmdType, srcAddr, gourndTemp, srcAddr);
			break;



			default:

		    Sys_event_post(SYSTEMAPP_EVT_DISP);
		    return;

		}
		// point to new message the head
		// bufTemp		= (radio_protocal_t *)((uint8_t *)bufTemp + bufTemp->len);
	// }

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

// 鍙戦�佷笉闇�瑕佸洖澶嶇殑鎸囦护,鍗曟鍙戦��
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


// 娓呴櫎涓嶉渶瑕佸洖澶嶇殑鎸囦护
void RadioCmdClearWithNoRespon(void)
{
	uint8_t i;
	cmdEvent &= 0xffffffffffffffff ^ (0x1 << cmdType);
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
	RadioSendWithNoResp(cmdType);
	return cmdType;
}

// 鍙戦�佷笉闇�瑕佸洖澶嶇殑缇ょ粍鎸囦护锛屼互骞挎挱鐨勬柟寮忓彂鍑�
bool RadioCmdSetWithNoRespon(uint16_t cmd, uint32_t dstAddr, uint32_t ground)
{
	dstAddr = IntToHex(dstAddr);
	ground  = IntToHex(ground);
	GroudAddrSet(ground);

	if(
	(cmd == RADIO_PRO_CMD_TERM_ADD_TO_GROUP) ||
	(cmd == RADIO_PRO_CMD_TERM_DELETE_FROM_GROUP) ||
	(cmd == RADIO_PRO_CMD_TERM_TEST) ||
	(cmd == RADIO_PRO_CMD_TERM_CLOSE_CTROL) ||
	(cmd == RADIO_PRO_CMD_TERM_OPEN_CTROL) ||
	(cmd == RADIO_PRO_CMD_TERM_UNLOCKING) ||
	(cmd == RADIO_PRO_CMD_TERM_POWER_HIGH) ||
	(cmd == RADIO_PRO_CMD_TERM_POWER_MID) ||
	(cmd == RADIO_PRO_CMD_TERM_POWER_LOW) ||
	(cmd == RADIO_PRO_CMD_FIXED_TERM_SUBDUE_START) ||
	(cmd == RADIO_PRO_CMD_FIXED_TERM_SUBDUE_STOP) ||
	(cmd == RADIO_PRO_CMD_TERM_CLOSE_BLOCKING) ||
	(cmd == RADIO_PRO_CMD_TERM_OPEN_BLOCKING) ||
	(cmd == RADIO_PRO_CMD_OPEN_TERMINAL_PREVENT_ESCAPE) ||
	(cmd == RADIO_PRO_CMD_CLOSE_TERMINAL_PREVENT_ESCAPE)){
		if(dstAddr){
			SetRadioDstAddr(dstAddr);
		}
	}
	else{
		SetRadioDstAddr(RADIO_BROCAST_ADDRESS);
	}


	cmdTypeGroud = cmd;
	cmdEventGroud |= (0x1 << cmd);
	RadioSendBrocast();

#ifdef ZKS_S6_6_WOR_G
    log_opration_record(cmd,dstAddr,ground);
#endif

	return true;
}

// 娓呴櫎涓嶉渶瑕佸洖澶嶇殑缇ょ粍鎸囦护
void RadioCmdClearWithNoRespon_Groud(void)
{
	uint8_t i;

	cmdEventGroud &= 0xffffffffffffffff ^ (0x1 << cmdTypeGroud);
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
	RadioSendGroundWithNoResp(cmdTypeGroud);
	return cmdTypeGroud;
}



// 鍙戦�佺殑闇�瑕佸洖澶嶅懡浠�
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
	Semaphore_pend(recAckSemHandle, BIOS_NO_WAIT);
	WdtClear();
#ifdef ZKS_S6_6_WOR_G
    log_opration_record(cmd,dstAddr,ground);
#endif
	return Semaphore_pend(recAckSemHandle, 4 * CLOCK_UNIT_S);
	// return true;
#else
	RadioSend();
	return true;
#endif

}


void RadioCmdClearWithRespon(void)
{
	uint8_t i;
	if(sendRetryTimes == 0){
		cmdEventWithRespon &= 0xffffffffffffffff ^ (0x1 << cmdTypeWithRespon);
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
