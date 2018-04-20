#ifndef		__S6_APP_H__
#define		__S6_APP_H__


/***** Defines *****/


/***** Type declarations *****/


/***** Variable declarations *****/



/***** function declarations *****/


void Sys_lcd_start_timing(void);

void Sys_lcd_stop_timing(void);

void S6HwInit(void);

void S6ShortKeyApp(void);

void S6ConcenterLongKeyApp(void);

void S6ShortKey1App(void);

void S6LongKey1App(void);

void S6AppRtcProcess(void);

void sensor_unpackage_to_memory(uint8_t *pData, uint16_t length);

bool get_next_sensor_memory(sensordata_mem *pSensor);


#endif		//	__S6_APP_H__
