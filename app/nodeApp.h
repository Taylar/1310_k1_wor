#ifndef			__NODEAPP_H__
#define			__NODEAPP_H__










void NodeAppInit(void (*Cb)(void));

void NodeAppHwInit(void);

void NodeUploadStart(void);

void NodeUploadStop(void);

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

#endif			// __NODEAPP_H__
