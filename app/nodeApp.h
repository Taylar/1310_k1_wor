#ifndef			__NODEAPP_H__
#define			__NODEAPP_H__


#define 			BROADCASTING_MAX			10


typedef struct {
    uint16_t lastFrameSerial[6];
    uint8_t Cnt;
}TxFrameRecord_t;




void NodeAppInit(void (*Cb)(void));

void NodeUploadPeriodSet(uint32_t period);

void NodeUploadProcess(void);

void NodeCollectStart(void);

void NodeCollectStop(void);

void NodeCollectPeriodSet(uint32_t period);

void NodeCollectProcess(void);

void NodeLowTemperatureSet(uint8_t num, uint16_t alarmTemp);

void NodeHighTemperatureSet(uint8_t num, uint16_t alarmTemp);

void NodeBroadcasting(void);

void NodeStartBroadcast(void);

void NodeStopBroadcast(void);

void NodeSleep(void);

void NodeWakeup(void);

void NodeUploadOffectClear(void);

void NodeUploadSucessProcess(TxFrameRecord_t *temp);

void NodeRtcProcess(void);

void NodeSetCustomId(uint32_t customId);

uint32_t NodeGetCustomId(void);

void NodeStrategyTimeoutProcess(void);

void NodeBroadcastCount(void);

bool NodeBroadcastTimesCheck(void);

void NodeBroadcastCountClear(void);

void SetNodeContinueFlag(void);

void ClearNodeContinueFlag(void);

uint8_t ReadNodeContinueFlag(void);

#endif			// __NODEAPP_H__
