#ifndef		__S6_APP_H__
#define		__S6_APP_H__


/***** Defines *****/


/***** Type declarations *****/


/***** Variable declarations *****/
extern Clock_Struct sysLcdShutClkStruct, sysAlarmClkStruct, BatAlarmClkStruct;
extern Clock_Handle sysLcdShutClkHandle, sysAlarmClkHandle, BatAlarmClkHandle;

extern uint8_t buzzerAlarmCnt;



/***** function declarations *****/
void Sys_buzzer_init(void);

void Sys_buzzer_enable(void);

void Sys_buzzer_disable(void);

void Sys_lcd_start_timing(void);

void Sys_lcd_stop_timing(void);

void sys_Node_Lose_Alarm(void);

void S6HwInit(void);

void S6AppBatProcess(void);

void sensor_unpackage_to_memory(uint8_t *pData, uint16_t length);

void UsbIntProcess(void);

void S6Wakeup(void);

void S6Sleep(void);

void S6KeyApp(void);

#endif		//	__S6_APP_H__
