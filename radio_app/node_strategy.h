#ifndef			__NODE_STRATEGY_H__
#define			__NODE_STRATEGY_H__


#define     FAIL_CONNECT_MAX_NUM               5

#define     FAIL_CONNECT_PERIOD_MAX_NUM        2

#define     FAIL_CHECK_RSSI_BUSY_MAX_NUM	   20



void NodeStrategyReset(void);

void NodeStrategyInit(void (*StrategyFailCb)(void));

void NodeStrategySetPeriod(uint32_t period);

void NodeStrategyReceiveTimeoutProcess(void);

void NodeStrategyBuffClear(void);

bool NodeStrategySendPacket(uint8_t *dataP, uint8_t len);

bool NodeStrategyBusyRead(void);

uint8_t NodeStrategyRemainderCache(void);

void NodeStrategyStop(void);

void NodeStrategyStart(void);

void NodeStrategySetOffset_Channel(uint32_t concenterTick, uint32_t length, uint32_t channel);

bool GetStrategyRegisterStatus(void);

void StrategyRegisterSuccess(void);

void StrategyCheckRssiBusyProcess(void);

#endif			// __NODE_STRATEGY_H__
