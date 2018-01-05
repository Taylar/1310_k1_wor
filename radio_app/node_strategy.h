#ifndef			__NODE_STRATEGY_H__
#define			__NODE_STRATEGY_H__










void NodeStrategyInit(void (*Cb)(void));

void NodeStrategySetPeriod(uint32_t period);

void NodeStrategyReceiveTimeoutProcess(void);

void NodeStrategyReceiveReceiveSuccess(void);

uint8_t NodeStrategySendPacket(uint8_t *dataP, uint8_t len);

void NodeStrategyBusySet(bool boolFlag);

bool NodeStrategyBusyRead(void);

uint8_t NodeStrategyRemainderCache(void);

void NodeStrategyStop(void);

#endif			// __NODE_STRATEGY_H__
