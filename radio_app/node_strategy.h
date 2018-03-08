#ifndef			__NODE_STRATEGY_H__
#define			__NODE_STRATEGY_H__








void NodeStrategyReset(void);

void NodeStrategyInit(void (*Cb)(void), void (*StrategyFailCb)(void));

void NodeStrategySetPeriod(uint32_t period);

void NodeStrategyReceiveTimeoutProcess(void);

void NodeStrategyReceiveReceiveSuccess(void);

bool NodeStrategySendPacket(uint8_t *dataP, uint8_t len);

void NodeStrategyBusySet(bool boolFlag);

bool NodeStrategyBusyRead(void);

uint8_t NodeStrategyRemainderCache(void);

void NodeStrategyStop(void);

void NodeStrategySetOffset_Channel(uint32_t concenterTick, uint32_t nodeTick, uint32_t channel);

#endif			// __NODE_STRATEGY_H__
