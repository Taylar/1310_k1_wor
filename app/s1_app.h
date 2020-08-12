#ifndef		__S1_APP_H__
#define		__S1_APP_H__


/***** Defines *****/
#define S1_CONFIG_MODE_TIME		60

/***** Type declarations *****/


/***** Variable declarations *****/
extern uint32_t configModeTimeCnt;          // the unit is sec



/***** function declarations *****/

void PreventiveInsertTest(void);
void PreventiveInsertCntClear(void);

void S1HwInit(void);

void S1ShortKeyApp(void);

void S1LongKeyApp(void);

void S1DoubleKeyApp(void);

void S1AppRtcProcess(void);

void S1Wakeup(void);

void S1Sleep(void);

#endif		//	__S1_APP_H__

