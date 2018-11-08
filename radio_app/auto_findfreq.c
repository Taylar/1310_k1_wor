/*
* @Author: zxt
* @Date:   2018-09-13 11:38:46
* @Last Modified by:   zxt
* @Last Modified time: 2018-11-08 11:21:52
*/
#include "../general.h"

/***** Type declarations *****/
typedef struct {
    bool        success;
    uint8_t    	switchTimes;                     // switch Freq times
    uint8_t    	checkRssiCnt;                     // check the rssi busy time times
    uint8_t    	sendCnt;                     // send to find check the channel if occupy times
    int8_t      curRssi;
    int8_t      carrierRssi[AUTO_FINDFREQ_SWITCH_MAX];
    uint8_t 	occupyReply[AUTO_FINDFREQ_SWITCH_MAX];
}auto_findfreq_t;

auto_findfreq_t autoFindfreq;

//***********************************************************************************
// brief:  Init the Auto frequency switch function
// 
// parameter: 
//***********************************************************************************
void AutoFreqInit(void)
{
	uint8_t i;
	g_rSysConfigInfo.rfBW     = 0;
	autoFindfreq.success      = false;
	autoFindfreq.switchTimes  = 0;
	autoFindfreq.checkRssiCnt = 0;
	autoFindfreq.sendCnt      = 0;
	autoFindfreq.curRssi      = -127;
	for(i = 0; i < AUTO_FINDFREQ_SWITCH_MAX; i++)
	{
	    autoFindfreq.occupyReply[i] = 0;
	}

#ifdef S_G//网关
    RadioEventPost(RADIO_EVT_CHANNEL_CHECK);
#endif //S_G//网关

}

//***********************************************************************************
// brief:  Concenter have check all the frequency
// 
// parameter: 
//***********************************************************************************
void AutoFreqConcenterSuccess(void)
{
	uint8_t i, index = 0;
	int8_t rssiTemp;
	rssiTemp = autoFindfreq.carrierRssi[0];

	for(i = 0; i < AUTO_FINDFREQ_SWITCH_MAX; i++)
	{
		if(autoFindfreq.occupyReply[i] == 0)
		{
			break;
		}
	}

	if(i < AUTO_FINDFREQ_SWITCH_MAX)
	{
		// disable all occypy center freq
		for(i = 0; i < AUTO_FINDFREQ_SWITCH_MAX; i++)
		{
			if(autoFindfreq.occupyReply[i] == 1)
			{
				autoFindfreq.carrierRssi[i] = 0;
			}
		}
	}

	for(i = 0; i < AUTO_FINDFREQ_SWITCH_MAX; i++)
	{
		if(rssiTemp > autoFindfreq.carrierRssi[i])
		{
			rssiTemp = autoFindfreq.carrierRssi[i];
			index = i;
		}
	}
	g_rSysConfigInfo.rfBW = index << 4;
	autoFindfreq.success  = true;
	RadioSetRxMode();
}


//***********************************************************************************
// brief:  node have receive the feedback
// 
// parameter: 
//***********************************************************************************
void AutoFreqNodeSuccess(void)
{
	autoFindfreq.success  = true;
}


//***********************************************************************************
// brief:  node switch the freq to find the concenter
// 
// parameter: 
//***********************************************************************************
void AutoFreqNodeSwitchFreq(void)
{
	autoFindfreq.switchTimes ++;
	if(autoFindfreq.switchTimes >= AUTO_FINDFREQ_SWITCH_MAX)
	{
		autoFindfreq.switchTimes = 0;
	}
	g_rSysConfigInfo.rfBW = autoFindfreq.switchTimes << 4;

	RadioSend();
}



//***********************************************************************************
// brief:  concenter switch the freq to find idle freqency
// 
// parameter: 
//***********************************************************************************
void AutoFreqConcenterSwitchFreqProcess(void)
{
    uint32_t randomNum;

	if(autoFindfreq.switchTimes >= AUTO_FINDFREQ_SWITCH_MAX)
	{
		AutoFreqConcenterSuccess();
		return;
	}
	randomNum = RandomDataGenerate();

	Task_sleep(randomNum % (300 * CLOCK_UNIT_MS));
	

	if((autoFindfreq.checkRssiCnt >= AUTO_FINDFREQ_BUSY_MAX) || (autoFindfreq.sendCnt >= AUTO_FINDFREQ_SEND_MAX))
	{
		autoFindfreq.carrierRssi[autoFindfreq.switchTimes] = autoFindfreq.curRssi + autoFindfreq.checkRssiCnt;
		autoFindfreq.switchTimes ++;
		g_rSysConfigInfo.rfBW     = autoFindfreq.switchTimes << 4;
		autoFindfreq.curRssi      = -127;
		autoFindfreq.checkRssiCnt = 0;
		autoFindfreq.sendCnt      = 0;
	}


	ConcenterRadioMonitorClear();
	ClearRadioSendBuf();
	ConcenterChannelCheckCmdSend(GetRadioSrcAddr(), RADIO_BROCAST_ADDRESS);
	RadioSend();
}

//***********************************************************************************
// brief:  concenter stop find the idle freq
// 
// parameter: 
//***********************************************************************************
void AutoFreqConcenterStop(void)
{
	autoFindfreq.switchTimes = AUTO_FINDFREQ_SWITCH_MAX;
}

//***********************************************************************************
// brief:  the center freq was occupy by other concenter process
// 
// parameter: 
// 			rssi : the radio rssi power
//***********************************************************************************
void AutoFreqConcenterOccupy(int8_t rssi)
{
	if(rssi > autoFindfreq.curRssi)
	{
		autoFindfreq.curRssi = rssi;
	}
	autoFindfreq.occupyReply[autoFindfreq.switchTimes] = 1;

	autoFindfreq.sendCnt = AUTO_FINDFREQ_SEND_MAX;
	RadioEventPost(RADIO_EVT_CHANNEL_CHECK);
}

//***********************************************************************************
// brief:  the rssi over the limit process
// 
// parameter: 
//***********************************************************************************
void AutoFreqCarrierBusy(int8_t rssi)
{
	if(rssi > autoFindfreq.curRssi)
	{
		autoFindfreq.curRssi = rssi;
	}
	autoFindfreq.checkRssiCnt ++;
	// SWITCH NEXT FREQ
    RadioEventPost(RADIO_EVT_CHANNEL_CHECK);
}


//***********************************************************************************
// brief:  currentl center freqency rssi set
// 
// parameter: 
//***********************************************************************************
void AutoFreqCarrierRssiSet(int8_t rssi)
{
	if(rssi > autoFindfreq.curRssi)
	{
		autoFindfreq.curRssi = rssi;
	}
}


//***********************************************************************************
// brief:  the rssi over the limit process
// 
// parameter: 
//***********************************************************************************
void AutoFreqRecTimeout(void)
{
	autoFindfreq.sendCnt ++;
	// SWITCH NEXT FREQ
    RadioEventPost(RADIO_EVT_CHANNEL_CHECK);
}



//***********************************************************************************
// brief:  read the auto frequency status
// 
// parameter: 
//***********************************************************************************
bool AutoFreqStateRead(void)
{
	return autoFindfreq.success;
}
