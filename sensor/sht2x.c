//***********************************************************************************
// Copyright 2017, Zksiot Development Ltd.
// Created by Linjie, 2017.08.08
// MCU:	MSP430F5529
// OS: TI-RTOS
// Project:
// File name: sht2x.c
// Description: SHT2x humiture sensor process routine.
//***********************************************************************************
#include "../general.h"

#ifdef SUPPORT_SHT2X
#include "sht2x.h"

//***********************************************************************************
//
// SHT2x check crc.
//
//***********************************************************************************
static ErrorStatus SHT2x_check_crc(uint8_t *pData, uint8_t length, uint8_t checksum)
{
    uint8_t i, j, crc = 0;

    //calculates 8-Bit checksum with given polynomial
    for (i = 0; i < length; i++) {
        crc ^= pData[i];
        for (j = 0; j < 8; j++) {
			if (crc & 0x80)
				crc = (crc << 1) ^ POLYNOMIAL;
			else
				crc = (crc << 1);
        }
    }

    if (crc != checksum)
        return ES_ERROR;
    else
        return ES_SUCCESS;
}

//***********************************************************************************
//
// SHT2x calculate humidty. x100
//
//***********************************************************************************
static uint16_t SHT2x_calc_humidty(uint16_t humidty)
{
    uint16_t humidity;

    if (humidty == 0)
        return HUMIDTY_OVERLOAD;

    //-- calculate relative humidity [%RH] --
    humidity = (uint16_t)round(-6.0 + 125.0/65536 * (float)humidty * 100); // RH = -6 + 125 * SRH/2^16

    if(humidity > 10000)//将humidity限制在100
        humidity = 10000;
    
    return humidity;
}

//***********************************************************************************
//
// SHT2x calculate temperature degree Celsius. x100.
//
//***********************************************************************************
static int16_t SHT2x_calc_temperatureC(uint16_t temperature)
{
    int16_t temperatureC;

    if (temperature == 0)
        return TEMPERATURE_OVERLOAD;

    //-- calculate temperature [℃] --
    temperatureC= (int16_t)round((-46.85 + 175.72/65536 *(float)temperature) * 100); // T = -46.85 + 175.72 * ST/2^16

    return temperatureC;
}

//***********************************************************************************
//
// SHT2x init.
//
//***********************************************************************************
static void SHT2x_init(uint8_t chNum)
{
    uint8_t buff[1];

    if (g_rSysConfigInfo.sensorModule[chNum] == SEN_TYPE_SHT2X) {

        buff[0] = SOFT_RESET;
        I2c_write(Board_SHT2x_ADDR, buff, 1);
        Task_sleep(25 * CLOCK_UNIT_MS);

    }
}

//***********************************************************************************
//
// SHT2x measure.
//
//***********************************************************************************
static void SHT2x_measure(uint8_t chNum)
{
    uint8_t userReg;
    uint8_t buff[3];
    uint8_t retrys = 0;//出错时重试3次

    if (g_rSysConfigInfo.sensorModule[chNum] == SEN_TYPE_SHT2X) {

        if (g_rSysConfigInfo.sensorModule[chNum] == SEN_TYPE_SHT2X) {
            SHT2x_init(chNum);
err_retrys:
            I2c_regRead(Board_SHT2x_ADDR, USER_REG_R, &userReg, 1);
            userReg &= ~SHT2x_RES_MASK;

            //Set temperature measure resolution.
            buff[0] = USER_REG_W; 
            buff[1] = userReg | SHT2x_RES_8_12BIT; 
            I2c_write(Board_SHT2x_ADDR, buff, 2);
            //Start temperature measure.
            buff[0] = TRIG_T_MEASURE_POLL; 
            I2c_write(Board_SHT2x_ADDR, buff, 1);
            //Wait temperature measure finish.
            // 11ms for 11bit, 22ms for 12bit, 43ms for 13bit, 85ms for 14bit.
            Task_sleep(25 * CLOCK_UNIT_MS);
            I2c_read(Board_SHT2x_ADDR, buff, 3);
            //Check crc.
            if (SHT2x_check_crc(buff, 2, buff[2]) == ES_SUCCESS) {
                //clear bits [1..0] (status bits)
                buff[1] &= ~0x03;

                HIBYTE(rSensorData[chNum].temp) = buff[0];
                LOBYTE(rSensorData[chNum].temp) = buff[1];
            } else {
                rSensorData[chNum].temp = 0;
                if(retrys++ < 3)goto err_retrys;
            }

            //Set humidity measure resolution.
            buff[0] = USER_REG_W; 
            buff[1] = userReg | SHT2x_RES_10_13BIT; 
            I2c_write(Board_SHT2x_ADDR, buff, 2);
            //Start humidity measure.
            buff[0] = TRIG_H_MEASURE_POLL; 
            I2c_write(Board_SHT2x_ADDR, buff, 1);
            //Wait humidity measure finish.
            // 4ms for 8bit, 9ms for 10bit, 15ms for 11bit, 29ms for 12bit.
            Task_sleep(12 * CLOCK_UNIT_MS);
            I2c_read(Board_SHT2x_ADDR, buff, 3);
            //Check crc.
            if (SHT2x_check_crc(buff, 2, buff[2]) == ES_SUCCESS) {
                //clear bits [1..0] (status bits)
                buff[1] &= ~0x03;

                HIBYTE(rSensorData[chNum].humi) = buff[0];
                LOBYTE(rSensorData[chNum].humi) = buff[1];
            } else {
                rSensorData[chNum].humi = 0;
                if(retrys++ < 3)goto err_retrys;
            }           
            //convert rawdata to temperature
            rSensorData[chNum].temp = SHT2x_calc_temperatureC(rSensorData[chNum].temp );
            //convert rawdata to humidty
            rSensorData[chNum].humi= SHT2x_calc_humidty(rSensorData[chNum].humi);
        }

    }
}

//***********************************************************************************
//
// SHT2x calculate humidty.
//
//***********************************************************************************
static int32_t SHT2x_get_value(uint8_t chNum, SENSOR_FUNCTION function)
{
    if (g_rSysConfigInfo.sensorModule[chNum] == SEN_TYPE_SHT2X) {

        if(function & SENSOR_TEMP){
            return rSensorData[chNum].temp;
        } else if(function & SENSOR_HUMI){
            return rSensorData[chNum].humi;
    	}
    }

    return TEMPERATURE_OVERLOAD;
}

const Sensor_FxnTable  SHT2X_FxnTable = {
	SENSOR_TEMP | SENSOR_HUMI,
	SHT2x_init,
    SHT2x_measure,
	SHT2x_get_value,
};


#endif  /* SUPPORT_SHT2X */

