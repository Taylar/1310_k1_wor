//***********************************************************************************
// Copyright 2017, Zksiot Development Ltd.
// Created by Linjie, 2017.08.08
// MCU:	MSP430F5529
// OS: TI-RTOS
// Project:
// File name: radio.h
// Description: radio process routine head.
//***********************************************************************************

#ifndef __ZKSIOT_SENSOR_H__
#define __ZKSIOT_SENSOR_H__



#define TEMPERATURE_OVERLOAD        0x7fff
#define HUMIDTY_OVERLOAD            0xffff
#define DEEP_TEMP_OVERLOAD          0x007fffff
#define LIGHT_OVERLOAD              0xffffffff

//define alarm value of sensor, must be same with tools 
#define ALARM_TEMP_HIGH         0x7fff
#define ALARM_TEMP_LOW          (-0x7fff)
#define ALARM_HUMI_HIGH         0x7fff
#define ALARM_HUMI_LOW          (-0x7fff)
//#define DEEP_TEMP_H              0x007fffff
//#define DEEP_TEMP_L             (-0x007fffff)

#define SENSOR_ADJUST_BUF_MAX           6
#define SENSOR_ADJUST_UPDATA_TIME       10
typedef enum {
    SEN_TYPE_NONE = 0,
    SEN_TYPE_SHT2X,
    SEN_TYPE_NTC,
    SEN_TYPE_OPT3001,
    SEN_TYPE_DEEPTEMP,
    SEN_TYPE_HCHO,
    SEN_TYPE_PM25,
    SEN_TYPE_CO2,
    SEN_TYPE_GSENSOR,
    SEN_TYPE_ORGPOLL,
    SEN_TYPE_HLW8012,
    SEN_TYPE_IM33XX,
    SEN_TYPE_INFRARED,
    SEN_TYPE_ASSET,
    SEN_TYPE_MAX
} SENSOR_TYPE;

typedef enum {
    SENSOR_NONE = 0x00,
	SENSOR_TEMP = 0x01,
	SENSOR_HUMI = 0x02,
	SENSOR_CO 	= 0x04,
	SENSOR_CO2  = 0x08,
    SENSOR_DEEP_TEMP = 0x10,
    SENSOR_LIGHT     = 0x20,
    SENSOR_LIS2D12_TILT  = 0x40,
    SENSOR_LIS2D12_AMP= 0x80,
} SENSOR_FUNCTION;

#ifdef SUPPORT_SENSOR_ADJUST
#define SENSOR_ADJUST_BUF_MAX           6
#define SENSOR_ADJUST_UPDATA_TIME       10
#endif //SUPPORT_SENSOR_ADJUST

typedef enum {
    SENSOR_DATA_NONE = 0x00,
	SENSOR_DATA_TEMP,   //温度
	SENSOR_DATA_HUMI,   //湿度
    SENSOR_DATA_LIGHT,  //光照
   	SENSOR_DATA_HCHO,   //甲醛
	SENSOR_DATA_PM25,   //PM2.5
	SENSOR_DATA_CO2,    //CO2
	SENSOR_DATA_YJWR,   //有机污染
	SENSOR_DATA_CO,     //CO
	SENSOR_DATA_TILT, //G_sensor TITLE
	SENSOR_DATA_AMP, //G_sensor amplitude
    SENSOR_DATA_MAX,	
} SENSOR_DATA_TYPE;


typedef enum {
	SEN_I2C_CH0 = 0,
	SEN_I2C_CH1,
	SEN_I2C_CH2,
	SEN_I2C_MAX,

	SEN_ADC_CH0 = 0x10,
	SEN_ADC_CH1,
	SEN_ADC_CH2,
	SEN_ADC_CH3,
	SEN_ADC_CH4,
	SEN_ADC_MAX,

    SEN_SPI_CH0 = 0x20,
	SEN_SPI_CH1,
	SEN_SPI_CH2,
	SEN_SPI_MAX,
} SENSOR_CH_NUM;

typedef union{
    struct {
    int16_t temp;
    uint16_t humi;
    };
    struct {
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minutes;
    };
    int32_t tempdeep;
    uint32_t  lux;
} SensorData_t;

typedef struct {
    //Sensor channel number.
    uint8_t chNum;
    //Sensor hardware port.
    uint8_t port;
    //Sensor hardware pin.
    uint16_t pin;
    //Sensor hardware ADC channel.
    uint8_t adcCh;
} SensorHWAttrs_t;

typedef void (*Sensor_initFxn) (uint8_t chNum);
typedef void (*Sensor_measureFxn)(uint8_t chNum);
typedef int32_t (*Sensor_getValueFxn)(uint8_t chNum, SENSOR_FUNCTION function);

typedef struct Sensor_FxnTable {
    uint32_t  function;
	Sensor_initFxn  initFxn;
    Sensor_measureFxn  measureFxn;	
	Sensor_getValueFxn getValueFxn;
} Sensor_FxnTable;

#if 1

extern uint8_t bUsbBuff[];

typedef struct {
    uint32_t DeviceId;
    uint8_t index;
    uint8_t type;
    SensorData_t value;
}sensordata_mem;


#endif

#pragma pack (1)
typedef struct {
    uint32_t DeviceId;
    uint8_t index;
    uint8_t type;
    SensorData_t value;
    uint8_t time[6];
}Alarmdata_t;
#pragma pack ()

#ifdef SUPPORT_SENSOR
extern SensorData_t rSensorData[MODULE_SENSOR_MAX];
extern const SensorHWAttrs_t rSensorHWAttrs[MODULE_SENSOR_MAX];

extern void Sensor_init(void);
extern void Sensor_measure(uint8_t store);
extern uint32_t Sensor_get_function(uint8_t chNum);
extern int16_t Sensor_get_temperatureC(uint8_t moduleNum);
extern uint16_t Sensor_get_humidty(uint8_t moduleNum);
extern uint32_t Sensor_get_deepTemperatureC(uint8_t chNum);
extern uint32_t Sensor_get_lux(uint8_t chNum);
extern void Sensor_collect_time_isr(void);
extern void Sensor_set_collect_time(uint32_t  collectTime);
extern void Sensor_collect_time_Adjust(uint8_t  Localtime, uint8_t  Nettime);
#endif  /* SUPPORT_SENSOR */

extern void sensor_unpackage_to_memory(uint8_t *pData, uint16_t length);
#ifdef SUPPORT_NETGATE_DISP_NODE
extern bool get_next_sensor_memory(sensordata_mem *pSensor);
#endif
extern uint32_t Sensor_get_function_by_type(uint8_t type);
extern void Sensor_store_null_package(uint8_t *buff);

extern void Sensor_process(void);
#endif	/* __ZKSIOT_SENSOR_H__ */

