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

#ifdef SUPPORT_SENSOR

#define TEMPERATURE_OVERLOAD        0x7fff
#define HUMIDTY_OVERLOAD            0xffff
#define DEEP_TEMP_OVERLOAD          0x7fffffff
typedef enum {
    SENSOR_NONE = 0x00,
	SENSOR_TEMP = 0x01,
	SENSOR_HUMI = 0x02,
	SENSOR_CO 	= 0x04,
	SENSOR_CO2  = 0x08,
    SENSOR_DEEP_TEMP = 0x10,
} SENSOR_FUNCTION;

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
    int32_t tempdeep;
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

#ifdef SUPPORT_NETGATE_DISP_NODE

extern uint8_t bUsbBuff[];

typedef struct {
    uint8_t  no;
    uint32_t DeviceId;
    uint8_t index;
    uint8_t type;
    SensorData_t value;
}sensordata_mem;

#define MEMSENSOR_NUM  (USB_BUFF_LENGTH/sizeof(sensordata_mem))

#endif 


extern SensorData_t rSensorData[MODULE_SENSOR_MAX];
extern const SensorHWAttrs_t rSensorHWAttrs[MODULE_SENSOR_MAX];

extern void Sensor_init(void);
extern void Sensor_measure(uint8_t store);
extern uint32_t Sensor_get_function(uint8_t chNum);
extern int16_t Sensor_get_temperatureC(uint8_t moduleNum);
extern uint16_t Sensor_get_humidty(uint8_t moduleNum);
extern uint32_t Sensor_get_deepTemperatureC(uint8_t chNum);
extern void Sensor_collect_time_isr(void);
extern void sensor_unpackage_to_memory(uint8_t *pData, uint16_t length);
//extern bool get_next_sensor_memory(sensordata_mem *pSensor);
extern uint32_t Sensor_get_function_by_type(SENSOR_TYPE type);


#endif  /* SUPPORT_SENSOR */

#endif	/* __ZKSIOT_SENSOR_H__ */

