#ifndef			_CONCENTERAPP_H__
#define			_CONCENTERAPP_H__


/***** Defines *****/

#define 		MEMSENSOR_NUM			(20)


/***** Type declarations *****/

typedef struct {
    uint32_t DeviceId;
    uint8_t index;
    uint8_t type;
union{
	struct {
    	int16_t temp;
    	uint16_t humi;
    };
    int32_t tempdeep;
	};
}sensordata_mem;





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

void ConcenterUploadEventSet(void);

uint32_t ConcenterReadChannel(uint32_t nodeAddr);

void ConcenterSaveChannel(uint32_t nodeAddr);

void ConcenterNodeSettingSuccess(uint32_t srcAddr, uint32_t dstAddr);

void ConcenterSleep(void);

void ConcenterWakeup(void);

void UsbIntProcess(void);

void ConcenterConfigDeceiveInit(void);

void ConcenterStoreConfig(void);

void ConcenterTimeSychronization(Calendar *currentTime);

uint8_t ConcenterReadSynTimeFlag(void);

void ConcenterRadioMonitorClear(void);

void ConcenterRtcProcess(void);

void ConcenterCollectStart(void);

void ConcenterCollectStop(void);

void ConcenterCollectPeriodSet(uint32_t period);

void ConcenterCollectProcess(void);


#endif			// _CONCENTERAPP_H__
