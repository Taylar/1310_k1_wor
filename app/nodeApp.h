#ifndef			__NODEAPP_H__
#define			__NODEAPP_H__


#define 			BROADCASTING_MAX			10
#define 			RADIO_APC_THRESHOLD_LOW 		(-90)
#define 			RADIO_APC_THRESHOLD_HIGH 		(-80)

typedef struct {
    uint16_t lastFrameSerial[6];
    uint8_t Cnt;
}TxFrameRecord_t;


#ifdef S_G
#define NodeAppInit(...)
#define NodeUploadPeriodSet(...)
#define NodeUploadProcess(...)
#define NodeCollectPeriodSet(...)
#define NodeCollectProcess(...)
#define NodeLowTemperatureSet(...)
#define NodeHighTemperatureSet(...)
#define NodeBroadcasting(...)
#define NodeStartBroadcast(...)
#define NodeStopBroadcast(...)
#define NodeIsBroadcast(...)
#define NodeSleep(...)
#define NodeWakeup(...)
#define NodeUploadOffectClear(...)
#define NodeUploadSucessProcess(temp)
#define NodeRtcProcess(...)
#define NodeSetCustomId(...)
#define NodeGetCustomId(...)
#define NodeStrategyTimeoutProcess(...)
#define NodeContinueFlagClear(...)
#define NodeContinueFlagSet(...)
#define NodeContinueFlagRead(...)

#else

void NodeAppInit(void);

void NodeUploadPeriodSet(uint32_t period);

void NodeUploadProcess(void);

void NodeCollectPeriodSet(uint32_t period);

void NodeCollectProcess(void);

void NodeLowTemperatureSet(uint8_t num, uint16_t alarmTemp);

void NodeHighTemperatureSet(uint8_t num, uint16_t alarmTemp);

void NodeBroadcasting(void);

void NodeStartBroadcast(void);

void NodeStopBroadcast(void);

bool NodeIsBroadcast(void);

void NodeSleep(void);

void NodeWakeup(void);

void NodeUploadOffectClear(void);

uint8_t NodeUploadSucessProcess(TxFrameRecord_t *temp);

void NodeRtcProcess(void);

void NodeSetCustomId(uint32_t customId);

uint32_t NodeGetCustomId(void);

void NodeStrategyTimeoutProcess(void);

void NodeContinueFlagClear(void);

void NodeContinueFlagSet(void);

bool NodeContinueFlagRead(void);


#endif  // S_G

void NodeAPC(int8_t rssi);

void NodeResetAPC(void);

void NodeSetAPC(void);
#endif			// __NODEAPP_H__
