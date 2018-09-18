#ifndef			__AUTO_FINDFREQ_H_
#define 		__AUTO_FINDFREQ_H_

#define 	AUTO_FINDFREQ_SWITCH_MAX 	10
#define 	AUTO_FINDFREQ_BUSY_MAX 		20
#define 	AUTO_FINDFREQ_SEND_MAX 		10


void AutoFreqInit(void);

void AutoFreqConcenterSuccess(void);

void AutoFreqNodeSuccess(void);

void AutoFreqNodeSwitchFreq(void);

void AutoFreqConcenterSwitchFreqProcess(void);

void AutoFreqConcenterOccupy(int8_t rssi);

void AutoFreqCarrierBusy(int8_t rssi);

void AutoFreqRecTimeout(void);

bool AutoFreqStateRead(void);



#endif // __AUTO_FINDFREQ_H_
