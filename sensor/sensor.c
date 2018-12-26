//***********************************************************************************
// Copyright 2017, Zksiot Development Ltd.
// Created by Linjie, 2017.08.08
// MCU:	MSP430F5529
// OS: TI-RTOS
// Project:
// File name: sensor.c
// Description: sensor process routine.
//***********************************************************************************
#include "../general.h"

#ifdef SUPPORT_SENSOR

typedef struct {
    uint8_t sensorInit;
    uint16_t serialNum;
    uint32_t collectTime;
} SensorObject_t;

/*
typedef struct {
    uint8_t     length;
    uint8_t     rssi;
    uint32_t    devicedid;
    uint16_t    serielno;
    uint8_t     time[6];
    uint16_t    volatge;     
} SensorData_t;
*/
SensorData_t rSensorData[MODULE_SENSOR_MAX];
SensorObject_t rSensorObject;





#ifdef SUPPORT_SHT2X
extern const Sensor_FxnTable  SHT2X_FxnTable;
#else
const Sensor_FxnTable  SHT2X_FxnTable = {
	SENSOR_TEMP | SENSOR_HUMI,
	NULL,
    NULL,
	NULL,
};
#endif

#ifdef SUPPORT_SHT3X
extern const Sensor_FxnTable  SHT3X_FxnTable;
#else
const Sensor_FxnTable  SHT3X_FxnTable = {
    SENSOR_TEMP | SENSOR_HUMI,
    NULL,
    NULL,
    NULL,
};
#endif


#ifdef SUPPORT_NETGATE_DISP_NODE
#if 1
#define MEMSENSOR_BUFF_LENGTH USB_BUFF_LENGTH
sensordata_mem *pMemSensor = (sensordata_mem*)bUsbBuff;// use usb buffer save sensor data in memory  on MSP430F5529
#else
#define MEMSENSOR_BUFF_LENGTH  sizeof(sensordata_mem)*100//鏀寔100鍙拌妭鐐规暟鎹瓨鍌�
sensordata_mem pMemSensor[100];//use independent memory on MSP432P401R
#endif
#define MEMSENSOR_NUM  (MEMSENSOR_BUFF_LENGTH/sizeof(sensordata_mem))
#endif  //SUPPORT_NETGATE_DISP_NODE


#ifdef SUPPORT_NTC
extern const Sensor_FxnTable  NTC_FxnTable;
#else
const Sensor_FxnTable NTC_FxnTable = {
    SENSOR_TEMP,
    NULL,
    NULL,
    NULL,
};
#endif

#if defined( SUPPORT_DEEPTEMP) || defined(SUPPORT_DEEPTEMP_PT100)
extern const Sensor_FxnTable  DeepTemp_PT100_FxnTable;
#else
const Sensor_FxnTable  DeepTemp_PT100_FxnTable = {
    SENSOR_DEEP_TEMP,
    NULL,
    NULL,
    NULL,
};

#endif

#ifdef SUPPORT_OPT3001
extern const Sensor_FxnTable  OPT3001_FxnTable;
#else
const Sensor_FxnTable  OPT3001_FxnTable = {
    SENSOR_LIGHT,
    NULL,
    NULL,
    NULL,
};
#endif

#ifdef SUPPORT_LIS2DS12
extern const Sensor_FxnTable  LIS2D12_FxnTable;
#else
const Sensor_FxnTable  LIS2D12_FxnTable = {
    SENSOR_LIS2D12_TILT | SENSOR_LIS2D12_AMP,
    NULL,
    NULL,
    NULL,
};

#endif

//注意，下表的定义必须与SENSOR_TYPE的定义顺序一致
static const Sensor_FxnTable *Sensor_FxnTablePtr[]={
    NULL,
#ifdef SUPPORT_SHT2X
	&SHT2X_FxnTable,
#else
	&SHT3X_FxnTable,
#endif
	&NTC_FxnTable,
    &OPT3001_FxnTable,
    &DeepTemp_PT100_FxnTable,
    NULL,//&HCHO_FxnTable,
    NULL,//&PM25_FxnTable,
    NULL,//&CO2_FxnTable,
    &LIS2D12_FxnTable,
};


void RtcAddMinutes(Calendar *calendar, uint16_t minutes)
{
    calendar->Minutes += minutes;
    if(calendar->Minutes >= 60){
        calendar->Hours +=  calendar->Minutes / 60;
        calendar->Minutes %= 60;        
        if(calendar->Hours >= 24){
            calendar->Hours %= 24;
            calendar->DayOfMonth++;
            if(calendar->DayOfMonth > MonthMaxDay(calendar->Year, calendar->Month)){
                calendar->DayOfMonth = 1;
                calendar->Month++;
                if(calendar->Month >12){
                    calendar->Month = 1;
                    calendar->Year++;
                }
            }
        }
    }
}

int16_t RtcMinutesPassed(Calendar *lastCalendar, Calendar *curCalendar)
{
    uint16_t Minutes = 0;
    Calendar tmpCalendar;
    
    if(curCalendar->Year < lastCalendar->Year){  
        return -1;//just return -1, mean curCalendar is less  then lastCalendar
    }
    else if(curCalendar->Year == lastCalendar->Year){
        
        if (curCalendar->Month < lastCalendar->Month){
            return -1;//just return -1, mean curCalendar is less  then lastCalendar
        }
        else if (curCalendar->Month == lastCalendar->Month){            
            if (curCalendar->DayOfMonth < lastCalendar->DayOfMonth){
                return -1;//just return -1, mean curCalendar is less  then lastCalendar
            }    
            else if (curCalendar->DayOfMonth == lastCalendar->DayOfMonth){
                if (curCalendar->Hours < lastCalendar->Hours){
                    return -1;//just return -1, mean curCalendar is less  then lastCalendar
                }
                else if (curCalendar->Hours == lastCalendar->Hours){     
                    if (curCalendar->Minutes < lastCalendar->Minutes){
                        return -1;//just return -1, mean curCalendar is less  then lastCalendar
                    }
                    else{
                        return (curCalendar->Minutes - lastCalendar->Minutes);
                    }
                }
            }
        }
    }
    
    tmpCalendar = *lastCalendar;

    while( Minutes < 0x7fff){
        RtcAddMinutes(&tmpCalendar, 1);
        Minutes++;
        if( (curCalendar->Year == tmpCalendar.Year) &&        
            (curCalendar->Month == tmpCalendar.Month) &&
            (curCalendar->DayOfMonth == tmpCalendar.DayOfMonth) &&
            (curCalendar->Hours == tmpCalendar.Hours) &&
            (curCalendar->Minutes == tmpCalendar.Minutes)){
            break;
        }
    }

    return Minutes;
}

void RtcBcdToHex(Calendar *calendar){
    calendar->Year       = TransBcdToHex((uint8_t)(calendar->Year - 0x2000)) + 2000;
    calendar->Month      = TransBcdToHex((uint8_t)(calendar->Month));
    calendar->DayOfMonth = TransBcdToHex((uint8_t)(calendar->DayOfMonth));
    calendar->Hours      = TransBcdToHex((uint8_t)(calendar->Hours));
    calendar->Minutes    = TransBcdToHex((uint8_t)(calendar->Minutes));
    calendar->Seconds    = TransBcdToHex((uint8_t)(calendar->Seconds));
}


//***********************************************************************************
//
// Network protocol group package.
//
//***********************************************************************************
static void Sensor_store_package(void)
{
#ifdef FLASH_EXTERNAL
    uint8_t i, buff[FLASH_SENSOR_DATA_SIZE];
    uint16_t value = 0, length;
	int16_t Minutes;
    Calendar calendar;
    static Calendar  lastcalendar = {0,};
    int32_t value_32 = 0;
    int16_t temp;
	uint32_t collectPeriod = g_rSysConfigInfo.collectPeriod;

	//sensor data: length(1B) rssi(1B) customid(2B) devicedi(4B)...
	
#ifdef SUPPORT_ALARM_SWITCH_PERIOD
	if(g_alarmFlag)
	{
		if(collectPeriod > g_rSysConfigInfo.alarmuploadPeriod)
			collectPeriod = g_rSysConfigInfo.alarmuploadPeriod;
	}
#endif //SUPPORT_ALARM_SWITCH_PERIOD

    calendar = Rtc_get_calendar();    
    
    if(collectPeriod % 60 == 0){//只对采集周期为整分钟的情况进行计数器调整。
        
        if(lastcalendar.Year != 0){//not first time
            
            Minutes = RtcMinutesPassed(&lastcalendar, &calendar);
            
			if(Minutes < 0){ //the time become slow  more then last, maybe error time sync, go on and for next time sync
				//RtcAddMinutes(&lastcalendar, (collectPeriod / 60));//this will make lastcalendar > calendar forever.
				rSensorObject.collectTime = 0;
			}
            else if(Minutes  ==  (collectPeriod / 60)){//its the right time, go on ...
                lastcalendar = calendar;
                rSensorObject.collectTime = (Minutes*60 + calendar.Seconds - 30) % collectPeriod;
            }
            else if(Minutes < (collectPeriod / 60)){//the time become slow, maybe repeat, wait more one minutes
                rSensorObject.collectTime = collectPeriod - 60;
                return;
            }
            else if(Minutes <= 2*(collectPeriod / 60)){//the time become fast 2 period, maybe lost, add one time
                RtcAddMinutes(&lastcalendar, (collectPeriod / 60));
                rSensorObject.collectTime =  collectPeriod - 1;
            }   
            else { //the time become fast more then 2 period, maybe first time sync, go on ...
                lastcalendar  = calendar;
            }
        }
        else{//first time, dont judge, go on ...
            lastcalendar  = calendar;
        }
    }
	else{
		if(lastcalendar.Year == calendar.Year  &&
            lastcalendar.Month == calendar.Month &&
            lastcalendar.DayOfMonth == calendar.DayOfMonth &&
            lastcalendar.Hours == calendar.Hours &&
            lastcalendar.Minutes == calendar.Minutes &&
            lastcalendar.Seconds == calendar.Seconds)
			return;
		
		lastcalendar = calendar;
	}
    
    length = 0;
    //消息头
    //消息长度
    buff[length++] = 0;
    //无线信号强度RSSI
    buff[length++] = 0;
    //customid
    //buff[length++] = g_rSysConfigInfo.customId[0];
    //buff[length++] = g_rSysConfigInfo.customId[1];
    //Sensor ID
    for (i = 0; i < 4; i++)
        buff[length++] = g_rSysConfigInfo.DeviceId[i];
#ifdef BOARD_S3
    length = 0;
#endif //BOARD_S3
    //娑堟伅娴佹按鍙�
    buff[length++] = HIBYTE_ZKS(rSensorObject.serialNum);
    buff[length++] = LOBYTE_ZKS(rSensorObject.serialNum);
    if (rSensorObject.serialNum >= 0xfffe) {
        rSensorObject.serialNum = 0;
    }
    rSensorObject.serialNum++;
    //采集时间      

	buff[length++] = TransHexToBcd(lastcalendar.Year - 2000);
	buff[length++] = TransHexToBcd(lastcalendar.Month);
	buff[length++] = TransHexToBcd(lastcalendar.DayOfMonth);
	buff[length++] = TransHexToBcd(lastcalendar.Hours);	 
	buff[length++] = TransHexToBcd(lastcalendar.Minutes);
    if(collectPeriod % 60 == 0){//只对采集周期为整分钟的情况进行计数器调整。  
        buff[length++] = 0x30;
    }
    else{
        buff[length++] = TransHexToBcd(lastcalendar.Seconds);
    }

    //Sensor电压
#ifdef SUPPORT_BATTERY
    value =  Battery_get_voltage();
#endif
#ifdef SUPPORT_CHARGE_DECT
    value |=  ((Get_Charge_plug()==NO_CHARGE)? 0 :1)<<15;
#endif

    buff[length++] = HIBYTE_ZKS(value);
    buff[length++] = LOBYTE_ZKS(value);
    //参数项列表数据

    for (i = 0; i < MODULE_SENSOR_MAX; i++) {
        if (g_rSysConfigInfo.sensorModule[i] > SEN_TYPE_NONE && 
            g_rSysConfigInfo.sensorModule[i] < SEN_TYPE_MAX  &&
            Sensor_FxnTablePtr[g_rSysConfigInfo.sensorModule[i]] != NULL ){

			if(g_rSysConfigInfo.sensorModule[i] == SEN_TYPE_GSENSOR){            
				continue;//gsensor  data dont save.
			}
			
            buff[length++] = i;
            buff[length++] = g_rSysConfigInfo.sensorModule[i];
            if (Sensor_FxnTablePtr[g_rSysConfigInfo.sensorModule[i]]->function == (SENSOR_TEMP | SENSOR_HUMI)) {
                buff[length++] = HIBYTE_ZKS(rSensorData[i].temp);
                buff[length++] = LOBYTE_ZKS(rSensorData[i].temp);
                buff[length++] = HIBYTE_ZKS(rSensorData[i].humi);
                buff[length++] = LOBYTE_ZKS(rSensorData[i].humi);

#ifdef SUPPORT_G7_PROTOCOL
                SurroundingMonitor(rSensorData[i].temp);
#endif  // SUPPORT_G7_PROTOCOL

            } else if (Sensor_FxnTablePtr[g_rSysConfigInfo.sensorModule[i]]->function == (SENSOR_DEEP_TEMP)) {
                value_32 = rSensorData[i].tempdeep;
                buff[length++] = LOBYTE_ZKS(HIWORD_ZKS(value_32));
                buff[length++] = HIBYTE_ZKS(LOWORD_ZKS(value_32));
                buff[length++] = LOBYTE_ZKS(LOWORD_ZKS(value_32));
            }else if(Sensor_FxnTablePtr[g_rSysConfigInfo.sensorModule[i]]->function == (SENSOR_LIGHT))
            {
                buff[length++] = HIBYTE_ZKS(HIWORD_ZKS(rSensorData[i].lux));//HIBYTE_ZKS(HIWORD_ZKS(rSensorData[i].lux));
                value_32 = (rSensorData[i].lux&0x00ffffff);
                buff[length++] = LOBYTE_ZKS(HIWORD_ZKS(value_32));
                buff[length++] = HIBYTE_ZKS(LOWORD_ZKS(value_32));
                buff[length++] = LOBYTE_ZKS(LOWORD_ZKS(value_32));
            } else {
                buff[length++] = HIBYTE_ZKS(rSensorData[i].temp);
                buff[length++] = LOBYTE_ZKS(rSensorData[i].temp);
            }

            if (Sensor_FxnTablePtr[g_rSysConfigInfo.sensorModule[i]]->function & (SENSOR_TEMP | SENSOR_DEEP_TEMP)) {
#ifdef BOARD_S6_6
                if (Menu_is_record() ||  !(g_rSysConfigInfo.module & MODULE_BTP))
#endif
                {

                    if (Sensor_FxnTablePtr[g_rSysConfigInfo.sensorModule[i]]->function == (SENSOR_DEEP_TEMP)) {
                        temp = (int16_t)(rSensorData[i].tempdeep >> 4);
                    } else {
                        temp = rSensorData[i].temp;
                    }
                    if((g_rSysConfigInfo.alarmTemp[i].high != ALARM_TEMP_HIGH && temp >= g_rSysConfigInfo.alarmTemp[i].high) ||
                       (g_rSysConfigInfo.alarmTemp[i].low != ALARM_TEMP_LOW && temp <= g_rSysConfigInfo.alarmTemp[i].low)) {
                        //设备ID
                        HIBYTE_ZKS((HIWORD_ZKS(g_AlarmSensor.DeviceId))) = g_rSysConfigInfo.DeviceId[0];
                        LOBYTE_ZKS((HIWORD_ZKS(g_AlarmSensor.DeviceId))) = g_rSysConfigInfo.DeviceId[1];
                        HIBYTE_ZKS((LOWORD_ZKS(g_AlarmSensor.DeviceId))) = g_rSysConfigInfo.DeviceId[2];
                        LOBYTE_ZKS((LOWORD_ZKS(g_AlarmSensor.DeviceId))) = g_rSysConfigInfo.DeviceId[3];

                        //采集时间
                        #if 0
                        calendar = Rtc_get_calendar();
                        g_AlarmSensor.time[0] = calendar.Year - CALENDAR_BASE_YEAR;
                        g_AlarmSensor.time[1] = calendar.Month;
                        g_AlarmSensor.time[2] = calendar.DayOfMonth;
                        g_AlarmSensor.time[3] = calendar.Hours;
                        g_AlarmSensor.time[4] = calendar.Minutes;
                        g_AlarmSensor.time[5] = calendar.Seconds;
						#else
						memcpy(&g_AlarmSensor.time, buff+8, 6);
						#endif

                        //通道号
                        g_AlarmSensor.index   = i;

                        //类型
                        g_AlarmSensor.type    = SENSOR_DATA_TEMP;                        

                        //数据
                        g_AlarmSensor.value.tempdeep = temp;

#ifdef SUPPORT_ALARM_RECORD_QURERY
                        Flash_store_alarm_record((uint8_t*)(&g_AlarmSensor),sizeof(Alarmdata_t));

#endif
                        Sys_event_post(SYS_EVT_ALARM);
                        g_bAlarmSensorFlag |= (1 << i);
                    }
                    else                   
                    {   
                         //取消报警
                         if(g_bAlarmSensorFlag & (1 << i)){
                            g_bAlarmSensorFlag ^= (1 << i);
                         }
                    }
                    
                }
            }
        }
    }

	#ifdef SUPPORT_ALARM_SWITCH_PERIOD
	if(g_bAlarmSensorFlag && (g_rSysConfigInfo.status & STATUS_ALARM_SWITCH_ON))
		g_alarmFlag = 1;
	else
		g_alarmFlag = 0;		
	#endif //SUPPORT_ALARM_SWITCH_PERIOD

#ifdef      G7_PROJECT
    memcpy(&buff[length], (uint8_t *)G7GetLbs(), sizeof(NwkLocation_t));
    buff[0] = length - 1;
    Flash_store_sensor_data(buff, length + sizeof(NwkLocation_t));
#else

#ifdef BOARD_S3
#ifdef SUPPORT_UPLOAD_ASSET_INFO
    buff[length++] = 0;
    buff[length++] = SEN_TYPE_ASSET;
#endif 
    Flash_store_sensor_data(buff, 16);
#else
    buff[0] = length - 1;
    Flash_store_sensor_data(buff, length);
#endif // BOARD_S3

#endif  // G7_PROJECT

#endif  /* FLASH_EXTERNAL */
}


//***********************************************************************************
//
// Sensor init.
//
//***********************************************************************************
void Sensor_init(void)
{
    uint8_t i;

    rSensorObject.sensorInit = 0;
    rSensorObject.serialNum = 0;
//    rSensorObject.collectTime = 0;//have init by rtc call Sensor_set_collect_time

    //Init sensor GPIO.
//    for (i = 0; i< MODULE_SENSOR_MAX; ++i) {
//        if(rSensorHWAttrs[i].port !=0){
//            GPIO_setAsOutputPin(rSensorHWAttrs[i].port, rSensorHWAttrs[i].pin);
//            GPIO_setOutputLowOnPin(rSensorHWAttrs[i].port, rSensorHWAttrs[i].pin);
//        }
//    }

    //init sensor on setting channel
    for (i = 0; i< MODULE_SENSOR_MAX; ++i) {
        if ((g_rSysConfigInfo.sensorModule[i] > SEN_TYPE_NONE) && 
            (g_rSysConfigInfo.sensorModule[i] < SEN_TYPE_MAX) &&
            (Sensor_FxnTablePtr[g_rSysConfigInfo.sensorModule[i]] != NULL)&&
            (Sensor_FxnTablePtr[g_rSysConfigInfo.sensorModule[i]]->initFxn != NULL)) {
                Sensor_FxnTablePtr[g_rSysConfigInfo.sensorModule[i]]->initFxn(i);
        }
    }

#ifdef SUPPORT_NETGATE_DISP_NODE
    memset(pMemSensor, 0, MEMSENSOR_BUFF_LENGTH);//init sensor data in  memory
#endif

}

//***********************************************************************************
//
// Sensor measure start.
//
//***********************************************************************************
void Sensor_measure(uint8_t store)
{
    uint8_t i, sensor = 0;

    for (i = 0; i < MODULE_SENSOR_MAX; i++) {
        if ((g_rSysConfigInfo.sensorModule[i] > SEN_TYPE_NONE) && 
            (g_rSysConfigInfo.sensorModule[i] < SEN_TYPE_MAX) &&
            (Sensor_FxnTablePtr[g_rSysConfigInfo.sensorModule[i]] != NULL) &&
            (Sensor_FxnTablePtr[g_rSysConfigInfo.sensorModule[i]]->measureFxn != NULL)) {
            
            sensor++;
            Sensor_FxnTablePtr[g_rSysConfigInfo.sensorModule[i]]->measureFxn(i);
        }
    }
#ifndef SUPPORT_UPLOAD_ASSET_INFO
    if (sensor && store)
#endif //SUPPORT_UPLOAD_ASSET_INFO
    {
        Sensor_store_package();
    }
}

//***********************************************************************************
//
// Sensor calculate temperature degree Celsius.
//
//***********************************************************************************
int16_t Sensor_get_temperatureC(uint8_t chNum)
{
    int16_t temperatureC = TEMPERATURE_OVERLOAD;

    if ((g_rSysConfigInfo.sensorModule[chNum] > SEN_TYPE_NONE) && 
        (g_rSysConfigInfo.sensorModule[chNum] < SEN_TYPE_MAX) &&
        (Sensor_FxnTablePtr[g_rSysConfigInfo.sensorModule[chNum]] != NULL) &&
        (Sensor_FxnTablePtr[g_rSysConfigInfo.sensorModule[chNum]]->function & SENSOR_TEMP) &&
        (Sensor_FxnTablePtr[g_rSysConfigInfo.sensorModule[chNum]]->getValueFxn != NULL))
        temperatureC = Sensor_FxnTablePtr[g_rSysConfigInfo.sensorModule[chNum]]->getValueFxn(chNum, SENSOR_TEMP);

    return temperatureC;
}

//***********************************************************************************
//
// Sensor calculate humidty.
//
//***********************************************************************************
uint16_t Sensor_get_humidty(uint8_t chNum)
{
    uint16_t humidity = HUMIDTY_OVERLOAD;

    if ((g_rSysConfigInfo.sensorModule[chNum] > SEN_TYPE_NONE) && 
        (g_rSysConfigInfo.sensorModule[chNum]  < SEN_TYPE_MAX) &&
        (Sensor_FxnTablePtr[g_rSysConfigInfo.sensorModule[chNum]] != NULL) &&
        (Sensor_FxnTablePtr[g_rSysConfigInfo.sensorModule[chNum]]->function & SENSOR_HUMI) &&
        (Sensor_FxnTablePtr[g_rSysConfigInfo.sensorModule[chNum]]->getValueFxn != NULL))
        humidity = Sensor_FxnTablePtr[g_rSysConfigInfo.sensorModule[chNum]]->getValueFxn(chNum, SENSOR_HUMI);

    return humidity;
}

//***********************************************************************************
//
// max31855 Sensor calculate temperature degree Celsius.
//
//***********************************************************************************
uint32_t Sensor_get_deepTemperatureC(uint8_t chNum)
{
    uint32_t deepTemperatureC = DEEP_TEMP_OVERLOAD;

    if ((g_rSysConfigInfo.sensorModule[chNum] > SEN_TYPE_NONE) &&
        (g_rSysConfigInfo.sensorModule[chNum]  < SEN_TYPE_MAX) &&
        (Sensor_FxnTablePtr[g_rSysConfigInfo.sensorModule[chNum]] != NULL) &&
        (Sensor_FxnTablePtr[g_rSysConfigInfo.sensorModule[chNum]]->function & SENSOR_DEEP_TEMP) &&
        (Sensor_FxnTablePtr[g_rSysConfigInfo.sensorModule[chNum]]->getValueFxn != NULL))
        deepTemperatureC = Sensor_FxnTablePtr[g_rSysConfigInfo.sensorModule[chNum]]->getValueFxn(chNum, SENSOR_DEEP_TEMP);
    return deepTemperatureC;
}
//***********************************************************************************
//
// opt3001 Sensor calculate temperature degree Celsius.
//
//***********************************************************************************
uint32_t Sensor_get_lux(uint8_t chNum)
{
    uint32_t lux = DEEP_TEMP_OVERLOAD;

    if ((g_rSysConfigInfo.sensorModule[chNum] > SEN_TYPE_NONE) &&
        (g_rSysConfigInfo.sensorModule[chNum]  < SEN_TYPE_MAX) &&
        (Sensor_FxnTablePtr[g_rSysConfigInfo.sensorModule[chNum]] != NULL) &&
        (Sensor_FxnTablePtr[g_rSysConfigInfo.sensorModule[chNum]]->function & SENSOR_LIGHT)&&
        (Sensor_FxnTablePtr[g_rSysConfigInfo.sensorModule[chNum]]->getValueFxn != NULL))
        lux = Sensor_FxnTablePtr[g_rSysConfigInfo.sensorModule[chNum]]->getValueFxn(chNum, SENSOR_LIGHT);
    return lux;
}
//***********************************************************************************
//
// Sensor collect time counter isr.
//
//***********************************************************************************
void Sensor_collect_time_isr(void)
{
#if  defined(SUPPORT_BOARD_OLD_S1) || defined(SUPPORT_BOARD_OLD_S2S_1)
        return;
#endif

    rSensorObject.collectTime++;
	uint32_t collectPeriod = g_rSysConfigInfo.collectPeriod;
	
#ifdef SUPPORT_ALARM_SWITCH_PERIOD
	if(g_alarmFlag)
	{
		if(collectPeriod > g_rSysConfigInfo.alarmuploadPeriod)
			collectPeriod = g_rSysConfigInfo.alarmuploadPeriod;
	}
#endif // SUPPORT_ALARM_SWITCH_PERIOD
	
    if (rSensorObject.collectTime >= collectPeriod) {
        //在时间同步时可能将采集时间点改变，重新调整到30S.
        rSensorObject.collectTime = (rSensorObject.collectTime - collectPeriod) % collectPeriod;
        Sys_event_post(SYS_EVT_SENSOR);
    }
}

void Sensor_set_collect_time(uint32_t  collectTime)
{
    rSensorObject.collectTime = (rSensorObject.collectTime/60)*60 + collectTime;//考虑采集周期大于1分钟，保留采集计数器已流失时间的分钟数。
}

void Sensor_collect_time_Adjust(uint8_t  Localtime, uint8_t  Nettime)
{
	int8_t value = 0;

	if(Nettime > Localtime ){
		if((Nettime - Localtime) > 30){
			value = Nettime - Localtime - 60;
		}
		else{
			value = Nettime - Localtime;
		}

	}else if(Nettime < Localtime ){
		if((Localtime - Nettime ) > 30){
			value = 60 -(Localtime  - Nettime);
		}
		else{
			value = Nettime - Localtime;
		}
	}
		
    rSensorObject.collectTime = (rSensorObject.collectTime + g_rSysConfigInfo.collectPeriod + value) % g_rSysConfigInfo.collectPeriod;
	
}

uint32_t Sensor_get_function(uint8_t chNum)
{
    if ((g_rSysConfigInfo.sensorModule[chNum] > SEN_TYPE_NONE) && 
           (g_rSysConfigInfo.sensorModule[chNum]  < SEN_TYPE_MAX) &&
           (Sensor_FxnTablePtr[g_rSysConfigInfo.sensorModule[chNum]] != NULL))
        return Sensor_FxnTablePtr[g_rSysConfigInfo.sensorModule[chNum]]->function ;

    return SENSOR_NONE;
}

uint32_t Sensor_get_function_by_type(uint8_t type)
{
    if((type > SEN_TYPE_NONE && type < SEN_TYPE_MAX) && (Sensor_FxnTablePtr[type] != NULL))        
        return Sensor_FxnTablePtr[type]->function ;

    return SENSOR_NONE;
}
#else
uint32_t Sensor_get_function_by_type(uint8_t type)
{
    uint32_t  function = SENSOR_NONE;

    switch(type){
    case SEN_TYPE_SHT2X:
        function = SENSOR_TEMP | SENSOR_HUMI;
        break;
    case SEN_TYPE_NTC:
        function = SENSOR_TEMP;
        break;
    case SEN_TYPE_OPT3001:
        function = SENSOR_LIGHT;
        break;
    case SEN_TYPE_DEEPTEMP:
        function = SENSOR_DEEP_TEMP;
        break;
    case SEN_TYPE_HCHO:
        //function = SENSOR_TEMP;
        break;
    case SEN_TYPE_PM25:
        //function = SENSOR_TEMP;
        break;
    case SEN_TYPE_CO2:
        function = SENSOR_CO2;
        break;
    case SEN_TYPE_GSENSOR:
        function = SENSOR_LIS2D12_TILT | SENSOR_LIS2D12_AMP;
        break;
    default:
        break;
    }

    return function;
}
#endif  /* SUPPORT_SENSOR */

//***********************************************************************************
//
// Network protocol group package.
//
//***********************************************************************************
void Sensor_store_null_package(uint8_t *buff)
{
    uint8_t i;
    uint16_t value = 0, length;
    Calendar calendar;

    //sensor data: length(1B) rssi(1B) customid(2B) devicedi(4B)...
    
    length = 0;
    //消息头
    //消息长度
    buff[length++] = 0;
    //无线信号强度RSSI
    buff[length++] = 0;

    //Sensor ID
    for (i = 0; i < 4; i++)
        buff[length++] = g_rSysConfigInfo.DeviceId[i];
    //消息流水号
    buff[length++] = 0;//HIBYTE(rSensorObject.serialNum);
    buff[length++] = 0;//LOBYTE(rSensorObject.serialNum);
    //rSensorObject.serialNum++;
    //采集时间
    calendar = Rtc_get_calendar();
    buff[length++] = calendar.Year - CALENDAR_BASE_YEAR;
    buff[length++] = calendar.Month;
    buff[length++] = calendar.DayOfMonth;
    buff[length++] = calendar.Hours;
    buff[length++] = calendar.Minutes;
    buff[length++] = calendar.Seconds;
    //Sensor电压
#ifdef SUPPORT_BATTERY
    value =  Battery_get_voltage();
#endif
    buff[length++] = HIBYTE_ZKS(value);
    buff[length++] = LOBYTE_ZKS(value);
    //参数项列表数据
    buff[0] = length - 1;
}

#ifdef SUPPORT_NETGATE_DISP_NODE
#if 1
#define MEMSENSOR_BUFF_LENGTH USB_BUFF_LENGTH
sensordata_mem *pMemSensor = (sensordata_mem*)bUsbBuff;// use usb buffer save sensor data in memory  on MSP430F5529
#else
#define MEMSENSOR_BUFF_LENGTH  sizeof(sensordata_mem)*100//支持100台节点数据存储
sensordata_mem pMemSensor[100];//use independent memory on MSP432P401R
#endif
#define MEMSENSOR_NUM  (MEMSENSOR_BUFF_LENGTH/sizeof(sensordata_mem))



uint8_t MemSensorIndex = 0;

#ifdef SUPPORT_NETGATE_BIND_NODE
static bool IsBindNode(uint32_t DeviceId)
{
    uint8_t i; 

    for(i = 0; i < NETGATE_BIND_NODE_MAX; ++i){
        if(g_rSysConfigInfo.bindnode[i].Deviceid == DeviceId)
            return true;
    }
    return false;
}
#endif
//***********************************************************************************
//
// unpackage  sensor data and save  sensor mac , index, type and value to mem
//
//***********************************************************************************
#ifdef SUPPORT_NETGATE_DISP_NODE
void sensor_unpackage_to_memory(uint8_t *pData, uint16_t length)
{    
	uint8_t i;
    uint16_t Index;    
	sensordata_mem cursensor;
#ifdef SUPPORT_NETGATE_BIND_NODE
    bool isbind = false;
#endif
    
	Index = 2;//DeviceId  start

    HIBYTE_ZKS(HIWORD_ZKS(cursensor.DeviceId)) = pData[Index++];
    LOBYTE_ZKS(HIWORD_ZKS(cursensor.DeviceId)) = pData[Index++];
    HIBYTE_ZKS(LOWORD_ZKS(cursensor.DeviceId)) = pData[Index++];
    LOBYTE_ZKS(LOWORD_ZKS(cursensor.DeviceId)) = pData[Index++];  

#ifdef SUPPORT_NETGATE_BIND_NODE
    isbind = IsBindNode(cursensor.DeviceId);

    if(((g_rSysConfigInfo.status & STATUS_DISP_BIND_ONLY)) && !isbind){  //如果支持绑定节点，默认显示所有节点信息,除非设置STATUS_DISP_BIND_ONLY
        return;
    }
#endif

    
    
	Index = 16;//sensor  start

	while(Index < length)
	{
		cursensor.index = pData[Index++];
		cursensor.type  = pData[Index++];
		
		if (!(cursensor.type > SEN_TYPE_NONE && cursensor.type < SEN_TYPE_MAX))
			return;//invalid sensor type

#ifdef  SUPPORT_UPLOAD_ASSET_INFO
        if(cursensor.type == SEN_TYPE_ASSET) {
            cursensor.value.month   = pData[9];
            cursensor.value.day     = pData[10];
            cursensor.value.hour    = pData[11];
            cursensor.value.minutes = pData[12];
        }else
#endif  //SUPPORT_UPLOAD_ASSET_INFO

		if (Sensor_get_function_by_type(cursensor.type) == (SENSOR_TEMP | SENSOR_HUMI)) {
			HIBYTE_ZKS(cursensor.value.temp) = pData[Index++];
			LOBYTE_ZKS(cursensor.value.temp) = pData[Index++];
			HIBYTE_ZKS(cursensor.value.humi) = pData[Index++];
			LOBYTE_ZKS(cursensor.value.humi) = pData[Index++];	
		}
        else if (Sensor_get_function_by_type(cursensor.type)  == (SENSOR_DEEP_TEMP)) {
            HIBYTE_ZKS(HIWORD_ZKS(cursensor.value.tempdeep)) = pData[Index++];
            LOBYTE_ZKS(HIWORD_ZKS(cursensor.value.tempdeep)) = pData[Index++];
			HIBYTE_ZKS(LOWORD_ZKS(cursensor.value.tempdeep)) = pData[Index++];
			LOBYTE_ZKS(LOWORD_ZKS(cursensor.value.tempdeep)) = 0;			
            cursensor.value.tempdeep >>= 12;			
        }
        else if (Sensor_get_function_by_type(cursensor.type) == (SENSOR_LIGHT)) {

            HIBYTE_ZKS(HIWORD_ZKS(cursensor.value.lux)) = pData[Index++];
            LOBYTE_ZKS(HIWORD_ZKS(cursensor.value.lux)) = pData[Index++];
            HIBYTE_ZKS(LOWORD_ZKS(cursensor.value.lux)) = pData[Index++];
            LOBYTE_ZKS(LOWORD_ZKS(cursensor.value.lux)) = pData[Index++];
        }
		else {
			HIBYTE_ZKS(cursensor.value.temp) = pData[Index++];
		 	LOBYTE_ZKS(cursensor.value.temp) = pData[Index++];
		}  

#ifdef SUPPORT_NETGATE_BIND_NODE
        if(isbind){
            if (Sensor_get_function_by_type(cursensor.type) & SENSOR_TEMP ) {

                //判断接收的数据是否已绑定设备，是则需要判断是否超温
                for( i = 0; i < NETGATE_BIND_NODE_MAX; ++i){
                    if ( (cursensor.DeviceId == g_rSysConfigInfo.bindnode[i].Deviceid) && 
                        ((g_rSysConfigInfo.bindnode[i].ChNo == 0xff) || cursensor.index  == g_rSysConfigInfo.bindnode[i].ChNo) ){
                        if((g_rSysConfigInfo.bindnode[i].AlarmInfo.high != ALARM_TEMP_HIGH && cursensor.value.temp >= g_rSysConfigInfo.bindnode[i].AlarmInfo.high) ||
                           (g_rSysConfigInfo.bindnode[i].AlarmInfo.low != ALARM_TEMP_LOW && cursensor.value.temp <= g_rSysConfigInfo.bindnode[i].AlarmInfo.low)) {                       

                            g_AlarmSensor.DeviceId = cursensor.DeviceId;                                    
                            g_AlarmSensor.index      = cursensor.index;
                            g_AlarmSensor.type       = SENSOR_DATA_TEMP;
                            //all  data  saved to tempdeep
                            g_AlarmSensor.value.tempdeep = cursensor.value.temp;                                                                                       
                        
                            g_AlarmSensor.time[0]    = pData[8];
                            g_AlarmSensor.time[1]    = pData[9];
                            g_AlarmSensor.time[2]    = pData[10];
                            g_AlarmSensor.time[3]    = pData[11];
                            g_AlarmSensor.time[4]    = pData[12];
                            g_AlarmSensor.time[5]    = pData[13];

						#ifdef SUPPORT_ALARM_RECORD_QURERY							  
                            Sys_event_post(SYS_EVT_ALARM_SAVE);
                        #endif                            
                            //设定报警        
                            Sys_event_post(SYS_EVT_ALARM);
                            g_bAlarmSensorFlag |= 0x100;                        
                        }
                        else                   
                        {   
                             //取消报警
                             if(g_bAlarmSensorFlag & (0x100)){
                                g_bAlarmSensorFlag ^= (0x100);
                             }                         
                        }

                        break;
                    }
                }
            }
        }
#endif

        //save to mem

        //find in mem 
        for (i = 0; i < MEMSENSOR_NUM; ++i) {
            if ((pMemSensor + i)->DeviceId == cursensor.DeviceId &&
               (pMemSensor + i)->index == cursensor.index &&
               (pMemSensor + i)->type == cursensor.type )
                break;

        }
        
        if (i < MEMSENSOR_NUM) {//update
             memcpy(pMemSensor + i, &cursensor, sizeof(sensordata_mem));
        }
        else {
            //new sensor id
            memcpy(pMemSensor + MemSensorIndex, &cursensor, sizeof(sensordata_mem));

            MemSensorIndex = (MemSensorIndex + 1) % MEMSENSOR_NUM;
        }
        
        
	}

}

bool get_next_sensor_memory(sensordata_mem *pSensor)
{    
    static uint8_t dispSensorIndex = 0;

restart:
    if (((sensordata_mem*)pMemSensor + dispSensorIndex)->DeviceId != 0x00000000 ){//valid data  
        memcpy(pSensor, (sensordata_mem*)pMemSensor + dispSensorIndex, sizeof(sensordata_mem));
        dispSensorIndex = (dispSensorIndex + 1) % MEMSENSOR_NUM;
        return true;
    }
    else {
        if(dispSensorIndex == 0)    
            return false;
        else {
            dispSensorIndex = 0;
            goto restart;
        }    
    }
}
#endif
#endif

