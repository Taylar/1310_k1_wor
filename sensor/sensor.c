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
#ifdef SUPPORT_SENSOR_ADJUST
    uint32_t updataTime;
#endif //SUPPORT_SENSOR_ADJUST
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
#ifdef SUPPORT_SENSOR_ADJUST
SensorData_t rSensorDataBuf[1][SENSOR_ADJUST_BUF_MAX];
uint8_t      rSensorUpdataNum, rSensorUpdataFull;
static Event_Struct sensorEvtStruct;
static Event_Handle sensorEvtHandle;
#endif //SUPPORT_SENSOR_ADJUST
static Calendar  lastcalendar;



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

#ifdef SUPPORT_SENSOR_ADJUST
#define SENSOR_EVT_NONE             Event_Id_NONE
#define SENSOR_EVT_UPDATA           Event_Id_00
#define SENSOR_EVT_STORE            Event_Id_01
#define SENSOR_EVT_ALL              0xffff
#endif //SUPPORT_SENSOR_ADJUST

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

void SensorSerialNumAdd(void)
{
    if (rSensorObject.serialNum >= 0xfffe) {
        rSensorObject.serialNum = 0;
    }
    rSensorObject.serialNum++;
}

uint16_t GetSensorSerialNum(void)
{
    return rSensorObject.serialNum;
}

uint8_t Sensor_data_pack(uint8_t *buff)
{
    uint8_t i, length = 0;
    int32_t value_32 = 0;


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
        }
    }
    return length;
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
#ifdef SUPPORT_LIGHT
    if(openBoxOccur){
        collectPeriod = 10; // 发生箱事件，立刻进行采集与存储数据
    }
#endif //SUPPORT_LIGHT

    if(collectPeriod % 60 == 0){//只对采集周期为整分钟的情况进行计数器调整。
        
        if(lastcalendar.Year != 0){//not first time
            
            Minutes = RtcMinutesPassed(&lastcalendar, &calendar);
            
			if(Minutes < 0){ //the time become slow  more then last, maybe error time sync, go on and for next time sync
				//RtcAddMinutes(&lastcalendar, (collectPeriod / 60));//this will make lastcalendar > calendar forever.
                Minutes =  RtcMinutesPassed(&calendar, &lastcalendar);
                if(Minutes > 2*(collectPeriod / 60)) {
                    lastcalendar = calendar;
                }
                else {
                    rSensorObject.collectTime = 0;
                    return;
                }
			}
            else if(Minutes  ==  (collectPeriod / 60)){//its the right time, go on ...
                lastcalendar = calendar;
                rSensorObject.collectTime = (Minutes*60 + calendar.Seconds - 30) % collectPeriod;
            }
            else if(Minutes < (collectPeriod / 60)){//the time become slow, maybe repeat, wait more one minutes
                if(calendar.Seconds > 30)
                    rSensorObject.collectTime = calendar.Seconds - 30 + Minutes*60;
                else
                    rSensorObject.collectTime = calendar.Seconds + 30 + Minutes*60;
                return;
            }
            else if(Minutes <= 2*(collectPeriod / 60)){//the time become fast 2 period, maybe lost, add one time
                RtcAddMinutes(&lastcalendar, (collectPeriod / 60));

                if ((Minutes ==  (2*(collectPeriod / 60)))&&(calendar.Seconds >= 30))
                    rSensorObject.collectTime =  collectPeriod - 1;
                else
                    rSensorObject.collectTime =  (Minutes-1)*60 - collectPeriod + calendar.Seconds + 30;

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
    SensorSerialNumAdd();
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


#ifdef BOARD_S3
#ifdef SUPPORT_UPLOAD_ASSET_INFO
    buff[length++] = 0;
    buff[length++] = SEN_TYPE_ASSET;
    assetInfoValid = 1;
    memcpy(assetInfo, buff, 24);
#ifdef SUPPORT_LIGHT
    if(openBoxOccur){
        openBoxOccur = 0; // 发生箱事件，立刻进行数据发送
        NodeContinueFlagClear();
        RadioEventPost(RADIO_EVT_TX);
    }
#endif //SUPPORT_LIGHT

#else
    Flash_store_sensor_data(buff, 16);
#endif 
#else
    buff[0] = length - 1;
    Flash_store_sensor_data(buff, length);
#endif // BOARD_S3


#endif  /* FLASH_EXTERNAL */
#ifdef  SUPPORT_DEVICED_STATE_UPLOAD
    if(g_bNeedUploadRecord){
        Flash_store_devices_state(TYPE_RECORD_START);
        g_bNeedUploadRecord = 0;
    }       
#endif
}

#ifndef SUPPORT_UPLOAD_ASSET_INFO
static void just_reset_time_by_sensor_data(Calendar *rtc)
{
    //1B00660242740000190424154730105C00010AF0150102030100119E
    //1B 00 66 02 42 74 00 01 190424154830105C00010AF0150D02030100131E

    uint8_t buff[FLASH_SENSOR_DATA_SIZE]={0};
    if(Flash_load_sensor_data_lately(buff)!=ES_ERROR)
    {
#ifdef BOARD_S3
        HIBYTE_ZKS(rtc->Year) = 0x20;
        LOBYTE_ZKS(rtc->Year) = buff[2];
        rtc->Month        = buff[3];
        rtc->DayOfMonth   = buff[4];
        rtc->Hours        = buff[5];
        rtc->Minutes      = buff[6];
        rtc->Seconds      = buff[7];
#else
        HIBYTE_ZKS(rtc->Year) = 0x20;
        LOBYTE_ZKS(rtc->Year) = buff[8];
        rtc->Month        = buff[9];
        rtc->DayOfMonth   = buff[10];
        rtc->Hours        = buff[11];
        rtc->Minutes      = buff[12];
        rtc->Seconds      = buff[13];

#endif //BOARD_S3
        RtcBcdToHex(rtc);
        Rtc_set_calendar(rtc);
    }
    else
    {
        LOBYTE_ZKS(rtc->Year) = 0;
        rtc->Month        = 0;
        rtc->DayOfMonth   = 0;
        rtc->Hours        = 0;
        rtc->Minutes      = 0;
        rtc->Seconds      = 0;
    }


}
#endif //SUPPORT_UPLOAD_ASSET_INFO
//***********************************************************************************
//
// Sensor init.
//
//***********************************************************************************
void Sensor_init(void)
{
    uint8_t i;

#ifdef SUPPORT_SENSOR_ADJUST
    /* Construct key process Event */
    Event_construct(&sensorEvtStruct, NULL);
    /* Obtain event instance handle */
    sensorEvtHandle = Event_handle(&sensorEvtStruct);
#endif //SUPPORT_SENSOR_ADJUST

#ifndef SUPPORT_UPLOAD_ASSET_INFO
    just_reset_time_by_sensor_data(&lastcalendar);
#endif //SUPPORT_UPLOAD_ASSET_INFO

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

#ifdef SUPPORT_SENSOR_ADJUST
void Sensor_colect_event_set(void)
{
    if(sensorEvtHandle)
        Event_post(sensorEvtHandle, SENSOR_EVT_STORE);
    Sys_event_post(SYS_EVT_SENSOR);
}


//***********************************************************************************
//
// Sensor measure updata.
//
//***********************************************************************************
void Sensor_data_adujst(void)
{
    uint8_t i, j, validCnt;
    int32_t  tempSum, tempMin, tempMax;

    for (i = 0; i < MODULE_SENSOR_MAX; i++) {
        if ((g_rSysConfigInfo.sensorModule[i] > SEN_TYPE_NONE) &&
            (g_rSysConfigInfo.sensorModule[i] < SEN_TYPE_MAX) &&
            (Sensor_FxnTablePtr[g_rSysConfigInfo.sensorModule[i]] != NULL) &&
            (Sensor_FxnTablePtr[g_rSysConfigInfo.sensorModule[i]]->measureFxn != NULL) &&
            (Sensor_FxnTablePtr[g_rSysConfigInfo.sensorModule[i]]->function == (SENSOR_DEEP_TEMP))) {

            tempSum = 0;
            validCnt = 0;

            tempMin = DEEP_TEMP_OVERLOAD;
            tempMax = - DEEP_TEMP_OVERLOAD;

            for(j = 0; j < SENSOR_ADJUST_BUF_MAX; j++){

                if(rSensorDataBuf[0][j].tempdeep != DEEP_TEMP_OVERLOAD){
                    tempSum += rSensorDataBuf[0][j].tempdeep;

                    if(rSensorDataBuf[0][j].tempdeep > tempMax)
                        tempMax = rSensorDataBuf[0][j].tempdeep;

                    if(rSensorDataBuf[0][j].tempdeep < tempMin)
                        tempMin = rSensorDataBuf[0][j].tempdeep;

                    validCnt++;
                }

            }
            if(validCnt >= 3){
                if(Sensor_FxnTablePtr[g_rSysConfigInfo.sensorModule[i]]->function == (SENSOR_DEEP_TEMP)){
                    rSensorData[i].tempdeep = (((tempSum - tempMax - tempMin) / (validCnt - 2)) & 0xFFFFFFF0) + (rSensorData[i].tempdeep & 0xf);
                }

            }
        }
    }
}


//***********************************************************************************
//
// Sensor measure updata.
//
//***********************************************************************************
void Sensor_measure_updata(void)
{
    uint8_t i;
    for (i = 0; i < MODULE_SENSOR_MAX; i++) {
        if ((g_rSysConfigInfo.sensorModule[i] > SEN_TYPE_NONE) &&
            (g_rSysConfigInfo.sensorModule[i] < SEN_TYPE_MAX) &&
            (Sensor_FxnTablePtr[g_rSysConfigInfo.sensorModule[i]] != NULL) &&
            (Sensor_FxnTablePtr[g_rSysConfigInfo.sensorModule[i]]->measureFxn != NULL)) {

            if((Sensor_FxnTablePtr[g_rSysConfigInfo.sensorModule[i]]->function & (SENSOR_TEMP | SENSOR_HUMI | SENSOR_DEEP_TEMP)) == 0){
                Sensor_FxnTablePtr[g_rSysConfigInfo.sensorModule[i]]->measureFxn(i);

            }
        }
    }
}

//***********************************************************************************
//
// Sensor measure updata.
//
//***********************************************************************************
void Sensor_measure_temp_updata(void)
{
    uint8_t i, sensor = 0;

    if(rSensorUpdataNum == SENSOR_ADJUST_BUF_MAX)
        rSensorUpdataNum = 0;

    for (i = 0; i < MODULE_SENSOR_MAX; i++) {
        if ((g_rSysConfigInfo.sensorModule[i] > SEN_TYPE_NONE) &&
            (g_rSysConfigInfo.sensorModule[i] < SEN_TYPE_MAX) &&
            (Sensor_FxnTablePtr[g_rSysConfigInfo.sensorModule[i]] != NULL) &&
            (Sensor_FxnTablePtr[g_rSysConfigInfo.sensorModule[i]]->measureFxn != NULL)) {

            if(Sensor_FxnTablePtr[g_rSysConfigInfo.sensorModule[i]]->function == (SENSOR_TEMP | SENSOR_HUMI)){
                Sensor_FxnTablePtr[g_rSysConfigInfo.sensorModule[i]]->measureFxn(i);

            }
            else if(Sensor_FxnTablePtr[g_rSysConfigInfo.sensorModule[i]]->function == (SENSOR_DEEP_TEMP)){
                Sensor_FxnTablePtr[g_rSysConfigInfo.sensorModule[i]]->measureFxn(i);
                rSensorDataBuf[0][rSensorUpdataNum].tempdeep = rSensorData[i].tempdeep;
                sensor++;
            }
            else if(Sensor_FxnTablePtr[g_rSysConfigInfo.sensorModule[i]]->function == (SENSOR_TEMP)){
                Sensor_FxnTablePtr[g_rSysConfigInfo.sensorModule[i]]->measureFxn(i);
            }
        }
    }

    if(sensor){
        rSensorUpdataNum++;
        if(rSensorUpdataNum >= SENSOR_ADJUST_BUF_MAX)
            rSensorUpdataFull = 1;

        if(rSensorUpdataFull){
            Sensor_data_adujst();
        }
    }
}

void Sensor_process(void)
{
    uint32_t eventId;
    eventId = Event_pend(sensorEvtHandle, 0, SENSOR_EVT_ALL, BIOS_NO_WAIT);

    if(eventId & SENSOR_EVT_UPDATA){
        Sensor_measure_temp_updata();
    }

    if(eventId & SENSOR_EVT_STORE){
        Sensor_measure_updata();
        Sensor_store_package();
    }
}
#endif //SUPPORT_SENSOR_ADJUST;

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
    uint32_t lux = LIGHT_OVERLOAD;

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
	
#ifdef SUPPORT_SENSOR_ADJUST
    rSensorObject.updataTime ++;
    if(rSensorObject.updataTime >= SENSOR_ADJUST_UPDATA_TIME){
        rSensorObject.updataTime = 0;
        if(sensorEvtHandle)
            Event_post(sensorEvtHandle, SENSOR_EVT_UPDATA);
        Sys_event_post(SYS_EVT_SENSOR);
    }
#endif //SUPPORT_SENSOR_ADJUST

    if (rSensorObject.collectTime >= collectPeriod) {
        //在时间同步时可能将采集时间点改变，重新调整到30S.
        rSensorObject.collectTime = (rSensorObject.collectTime - collectPeriod) % collectPeriod;
#ifdef SUPPORT_SENSOR_ADJUST
        if(sensorEvtHandle)
            Event_post(sensorEvtHandle, SENSOR_EVT_STORE);
#endif //SUPPORT_SENSOR_ADJUST
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
    buff[length++] = TransHexToBcd(calendar.Year - CALENDAR_BASE_YEAR);
    buff[length++] = TransHexToBcd(calendar.Month);
    buff[length++] = TransHexToBcd(calendar.DayOfMonth);
    buff[length++] = TransHexToBcd(calendar.Hours);
    buff[length++] = TransHexToBcd(calendar.Minutes);
    buff[length++] = TransHexToBcd(calendar.Seconds);
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
static int8_t IsBindNode(uint32_t DeviceId)
{
    uint8_t i; 

    for(i = 0; i < NETGATE_BIND_NODE_MAX; ++i){
        if(g_rSysConfigInfo.bindnode[i].Deviceid == DeviceId){
            return i;
    }
    }
    return -1;
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
    int8_t isbindNum = - 1;
    int32_t temp;
#endif

    cursensor.rssi = pData[1];
	Index = 2;//DeviceId  start

    HIBYTE_ZKS(HIWORD_ZKS(cursensor.DeviceId)) = pData[Index++];
    LOBYTE_ZKS(HIWORD_ZKS(cursensor.DeviceId)) = pData[Index++];
    HIBYTE_ZKS(LOWORD_ZKS(cursensor.DeviceId)) = pData[Index++];
    LOBYTE_ZKS(LOWORD_ZKS(cursensor.DeviceId)) = pData[Index++];  

#ifdef SUPPORT_NETGATE_BIND_NODE
    isbindNum = IsBindNode(cursensor.DeviceId);

    if(((g_rSysConfigInfo.status & STATUS_DISP_BIND_ONLY)) && (isbindNum == -1)){  //如果支持绑定节点，默认显示所有节点信息,除非设置STATUS_DISP_BIND_ONLY
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
            cursensor.value.tempdeep >>= 8;			
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
        if( isbindNum != -1 ){

                    //judge is bound and judge the value is over setting alarm threshold
            if ( (g_rSysConfigInfo.bindnode[isbindNum].ChNo == 0xff) || (cursensor.index  == g_rSysConfigInfo.bindnode[isbindNum].ChNo) ){

                // temp alrm
				if( Sensor_get_function_by_type(cursensor.type) & (SENSOR_TEMP | SENSOR_DEEP_TEMP)){

					if(Sensor_get_function_by_type(cursensor.type)&SENSOR_DEEP_TEMP){
						temp = cursensor.value.tempdeep>>4;
					}
					else{
						temp = cursensor.value.temp;
					}

					if( (g_rSysConfigInfo.bindnode[isbindNum].AlarmInfo.high != ALARM_TEMP_HIGH && temp >= g_rSysConfigInfo.bindnode[isbindNum].AlarmInfo.high) ||
						(g_rSysConfigInfo.bindnode[isbindNum].AlarmInfo.low != ALARM_TEMP_LOW && temp <= g_rSysConfigInfo.bindnode[isbindNum].AlarmInfo.low))  {

                            g_AlarmSensor.DeviceId = cursensor.DeviceId;                                    
                            g_AlarmSensor.index      = cursensor.index;
                            g_AlarmSensor.type       = SENSOR_DATA_TEMP;
                            //all  data  saved to tempdeep
						    g_AlarmSensor.value.tempdeep = temp;
                        
                            g_AlarmSensor.time[0]    = pData[8];
                            g_AlarmSensor.time[1]    = pData[9];
                            g_AlarmSensor.time[2]    = pData[10];
                            g_AlarmSensor.time[3]    = pData[11];
                            g_AlarmSensor.time[4]    = pData[12];
                            g_AlarmSensor.time[5]    = pData[13];

						#ifdef SUPPORT_ALARM_RECORD_QURERY							  
						    Flash_store_alarm_record((uint8_t*)(&g_AlarmSensor),sizeof(Alarmdata_t));
                        #endif                            
                            //设定报警
                            Sys_event_post(SYS_EVT_ALARM);
                            g_bAlarmSensorFlag |= ALARM_RX_EXTERNAL_ALARM;                        
                    }
#if 0
					else {
                         //取消报警
                         if(g_bAlarmSensorFlag & (ALARM_RX_EXTERNAL_ALARM)){
                            g_bAlarmSensorFlag ^= (ALARM_RX_EXTERNAL_ALARM);
                         }                         
                    }
#endif
            	}
        	}
		}        
#endif

        //======================================================
        if((g_rSysConfigInfo.status&STATUS_ALARM_GATE_ON) && (cursensor.type == SEN_TYPE_ASSET)){

            g_AlarmSensor.DeviceId = cursensor.DeviceId;
            g_AlarmSensor.index      = cursensor.index;
            g_AlarmSensor.type       = SEN_TYPE_ASSET;

            g_AlarmSensor.time[0]    = pData[8];
            g_AlarmSensor.time[1]    = cursensor.value.month;
            g_AlarmSensor.time[2]    =cursensor.value.day;
            g_AlarmSensor.time[3]    = cursensor.value.hour;
            g_AlarmSensor.time[4]    = cursensor.value.minutes;
            Sys_event_post(SYS_EVT_ALARM);
            g_bAlarmSensorFlag |= ALARM_NODE_LOSE_ALARM;


        }

        //save to mem

        //find in mem 
        for (i = 0; i < MEMSENSOR_NUM; ++i) {
            if ((pMemSensor + i)->DeviceId == cursensor.DeviceId &&
               ((pMemSensor + i)->index == cursensor.index || (pMemSensor + i)->type == SEN_TYPE_INVALID) &&
               ((pMemSensor + i)->type == cursensor.type || cursensor.type == SEN_TYPE_INVALID || (pMemSensor + i)->type == SEN_TYPE_INVALID) ){
                    if(cursensor.type == SEN_TYPE_INVALID)
                        return;
                    
                    break;
            }

        }
        
        if (i < MEMSENSOR_NUM) {//update
             memcpy(pMemSensor + i, &cursensor, sizeof(sensordata_mem));
        }
        else {

            //new sensor id or sensor
            for(i = 0; i < MEMSENSOR_NUM; i++){
                if((pMemSensor + i)->DeviceId == cursensor.DeviceId)
                    break;
            }

            if(i == MEMSENSOR_NUM){
                LinkNum++;
                if(LinkNum > MEMSENSOR_NUM)
                    LinkNum = MEMSENSOR_NUM;
            }

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

uint16_t CheckNode(uint32_t checkId)
{
    uint16_t i;

    for(i = 0; i < MEMSENSOR_NUM; i++){
        if((pMemSensor + i)->DeviceId == checkId)
            break;
    }
    if(i < MEMSENSOR_NUM)
        return 0;
    else
        return 0xffff;
}
#endif
#endif

