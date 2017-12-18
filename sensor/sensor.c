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

SensorData_t rSensorData[MODULE_SENSOR_MAX];
SensorObject_t rSensorObject;





extern const Sensor_FxnTable  NTC_FxnTable;
extern const Sensor_FxnTable  SHT2X_FxnTable;
extern const Sensor_FxnTable  DeepTemp_FxnTable;
//ע�⣬�±�Ķ��������SENSOR_TYPE�Ķ���˳��һ��
static const Sensor_FxnTable *Sensor_FxnTablePtr[]={
    NULL,
        
#ifdef SUPPORT_SHT2X
	&SHT2X_FxnTable
#else
    NULL
#endif
    ,

#ifdef SUPPORT_NTC
	&NTC_FxnTable
#else
    NULL
#endif
    ,

#ifdef SUPPORT_LIGHT
    &Light_FxnTable
#else
    NULL
#endif
    ,
    
#ifdef SUPPORT_DEEPTEMP
    &DeepTemp_FxnTable
#else
    NULL
#endif
    ,
    
#ifdef SUPPORT_HCHO
    &HCHO_FxnTable
#else
    NULL
#endif
    ,
#ifdef SUPPORT_PM25
    &PM25_FxnTable
#else
    NULL
#endif
    ,
    
#ifdef SUPPORT_CO2
    &CO2_FxnTable
#else
    NULL
#endif
    ,

};






#ifdef SUPPORT_NETGATE_DISP_NODE
sensordata_mem *pMemSensor = (sensordata_mem*)bUsbBuff;//  use usb buffer save sensor data in memory.
uint8_t MemSensorIndex = 0;

//***********************************************************************************
//
// unpackage  sensor data and save  sensor mac , index, type and value to mem
//
//***********************************************************************************
void sensor_unpackage_to_memory(uint8_t *pData, uint16_t length)
{    
	uint8_t i;
    uint16_t Index;    
	sensordata_mem cursensor;
    
	Index = 4;//DeviceId  start

    cursensor.no = 0;    //find sensor no in sensor table 

    HIBYTE(HIWORD(cursensor.DeviceId)) = pData[Index++];
    LOBYTE(HIWORD(cursensor.DeviceId)) = pData[Index++];
    HIBYTE(LOWORD(cursensor.DeviceId)) = pData[Index++];
    LOBYTE(LOWORD(cursensor.DeviceId)) = pData[Index++];  

	Index = 18;//sensor  start

	while(Index < length)
	{
		cursensor.index = pData[Index++];
		cursensor.type  = pData[Index++];
		
		if (!(cursensor.type > SEN_TYPE_NONE && cursensor.type < SEN_TYPE_MAX))
			return;//invalid sensor type

		
		if (Sensor_FxnTablePtr[cursensor.type]->function == (SENSOR_TEMP | SENSOR_HUMI)) {
			HIBYTE(cursensor.value.temp) = pData[Index++];
			LOBYTE(cursensor.value.temp) = pData[Index++];
			HIBYTE(cursensor.value.humi) = pData[Index++];
			LOBYTE(cursensor.value.humi) = pData[Index++];	
		}else if (Sensor_FxnTablePtr[cursensor.type]->function == (SENSOR_DEEP_TEMP)) {
            HIBYTE(HIWORD(cursensor.value.tempdeep)) = pData[Index++];
            LOBYTE(HIWORD(cursensor.value.tempdeep)) = pData[Index++];
			HIBYTE(LOWORD(cursensor.value.tempdeep)) = pData[Index++];
            cursensor.value.tempdeep >>= 8;			
        }
		else {
			HIBYTE(cursensor.value.temp) = pData[Index++];
		 	LOBYTE(cursensor.value.temp) = pData[Index++];
		}  

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
    rSensorObject.collectTime = 0;

    //Init sensor GPIO.
    for (i = 0; i< MODULE_SENSOR_MAX; ++i) {
        //GPIO_setAsOutputPin(rSensorHWAttrs[i].port, rSensorHWAttrs[i].pin);
        //GPIO_setOutputLowOnPin(rSensorHWAttrs[i].port, rSensorHWAttrs[i].pin);
    }

    //init sensor on setting channel
    for (i = 0; i< MODULE_SENSOR_MAX; ++i) {
        if ((g_rSysConfigInfo.sensorModule[i] > SEN_TYPE_NONE) && 
            (g_rSysConfigInfo.sensorModule[i] < SEN_TYPE_MAX) &&
            (Sensor_FxnTablePtr[g_rSysConfigInfo.sensorModule[i]] != NULL)) {
            Sensor_FxnTablePtr[g_rSysConfigInfo.sensorModule[i]]->initFxn(i);
        }
    }

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
            (Sensor_FxnTablePtr[g_rSysConfigInfo.sensorModule[i]] != NULL)) {
            
            sensor++;
            Sensor_FxnTablePtr[g_rSysConfigInfo.sensorModule[i]]->measureFxn(i);
        }
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
        (Sensor_FxnTablePtr[g_rSysConfigInfo.sensorModule[chNum]]->function & SENSOR_TEMP))
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
        (Sensor_FxnTablePtr[g_rSysConfigInfo.sensorModule[chNum]]->function & SENSOR_HUMI))
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
        (Sensor_FxnTablePtr[g_rSysConfigInfo.sensorModule[chNum]]->function & SENSOR_DEEP_TEMP))
        deepTemperatureC = Sensor_FxnTablePtr[g_rSysConfigInfo.sensorModule[chNum]]->getValueFxn(chNum, SENSOR_DEEP_TEMP);
    return deepTemperatureC;
}

//***********************************************************************************
//
// Sensor collect time counter isr.
//
//***********************************************************************************
void Sensor_collect_time_isr(void)
{
    rSensorObject.collectTime++;
    if (rSensorObject.collectTime >= g_rSysConfigInfo.collectPeriod) {
        rSensorObject.collectTime = 0;
        //Sys_event_post(SYS_EVT_SENSOR);
    }
}

uint32_t Sensor_get_function(uint8_t chNum)
{
    if ((g_rSysConfigInfo.sensorModule[chNum] > SEN_TYPE_NONE) && 
           (g_rSysConfigInfo.sensorModule[chNum]  < SEN_TYPE_MAX) &&
           (Sensor_FxnTablePtr[g_rSysConfigInfo.sensorModule[chNum]] != NULL))
        return Sensor_FxnTablePtr[g_rSysConfigInfo.sensorModule[chNum]]->function ;

    return SENSOR_NONE;
}

uint32_t Sensor_get_function_by_type(SENSOR_TYPE type)
{
    if((type > SEN_TYPE_NONE && type < SEN_TYPE_MAX) && (Sensor_FxnTablePtr[type] != NULL))        
        return Sensor_FxnTablePtr[type]->function ;

    return SENSOR_NONE;
}

#endif  /* SUPPORT_SENSOR */

