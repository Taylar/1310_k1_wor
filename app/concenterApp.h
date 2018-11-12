#ifndef			_CONCENTERAPP_H__
#define			_CONCENTERAPP_H__


/***** Defines *****/



/***** Type declarations *****/






/***** Variable declarations *****/





/***** Prototypes *****/
void ConcenterAppInit(void);

void ConcenterAppHwInit(void);

void ConcenterUploadStart(void);

void ConcenterUploadStop(void);

void ConcenterUploadPeriodSet(uint32_t period);

bool ConcenterSensorDataSaveToQueue(uint8_t *dataP, uint8_t length);

void ConcenterSensorDataSave(void);

void ConcenterSensorDataUpload(void);

void ConcenterSaveChannel(uint32_t nodeAddr);

void ConcenterNodeSettingSuccess(uint32_t srcAddr, uint32_t dstAddr);

void ConcenterSleep(void);

void ConcenterWakeup(void);

void ConcenterConfigDeceiveInit(void);

void ConcenterStoreConfig(void);

void ConcenterTimeSychronization(Calendar *currentTime);

uint8_t ConcenterReadSynTimeFlag(void);

void ConcenterRadioMonitorClear(void);

void ConcenterRtcProcess(void);

void ConcenterResetRadioState(void);

void ConcenterCollectStart(void);

void ConcenterCollectStop(void);

#ifdef SUPPORT_STRATEGY_SORT

uint16_t ConcenterSetNodeChannel(uint32_t nodeAddr, uint32_t channel);

uint16_t ConcenterReadNodeChannel(uint32_t nodeAddr);

uint16_t ConcenterReadResentNodeChannel(void);

#endif // SUPPORT_STRATEGY_SORT

#endif			// _CONCENTERAPP_H__
