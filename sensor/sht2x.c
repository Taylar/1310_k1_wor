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

	if(humidity >= 750)//correct the data
		humidity -= 750;

    if(humidity >= 10000)//????¡±?umidity¨¦¡ª??????¡ª¨¦?|???9.99
        humidity = 9999;
    
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

    //-- calculate temperature [?] --
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
    uint8_t error;
    error  = 0;
    if (g_rSysConfigInfo.sensorModule[chNum] == SEN_TYPE_SHT2X) {


        I2C_start();
        I2C_send_byte (Board_SHT2x_ADDR<<1); // I2C Adr
        if(I2C_ack_receive() == false)
        error |= 0x01;
        I2C_send_byte (SOFT_RESET); // Command
        if(I2C_ack_receive() == false)
        error |= 0x01;
        I2C_stop();

        Task_sleep(150 * CLOCK_UNIT_MS);

    }
}

//***********************************************************************************
//
// sort data.
//
//***********************************************************************************
void sort_data(int16_t *data, uint16_t len)
{
    uint16_t i , j;
    int16_t temp;

    for (i = 0; i < (len - 1); i++)
    {   
        for (j = 0; j < (len - 1 - i); j++)
            if (data[j] > data[j+1])   
            {   
                  temp = data[j];   
                  data[j] = data[j+1];   
                  data[j+1] = temp;   
            }   
    }  
}


//===========================================================================
//===========================================================================
uint8_t SHT2x_ReadUserRegister(uint8_t *pRegisterValue)
{
    uint8_t checksum; //variable for checksum byte
    uint8_t error=0; //variable for error code
    I2C_start();
    I2C_send_byte (Board_SHT2x_ADDR<<1);
    if(I2C_ack_receive() == false)
        error |= 0x01;
    I2C_send_byte (USER_REG_R);
    if(I2C_ack_receive() == false)
        error |= 0x01;
    I2C_start();
    I2C_send_byte (Board_SHT2x_ADDR<<1|0x01);
    if(I2C_ack_receive() == false)
        error |= 0x01;
    *pRegisterValue = I2C_receive_byte();
    I2C_ack_send();

    checksum=I2C_receive_byte();
    I2C_noack_send();
    error |= SHT2x_check_crc (pRegisterValue,1,checksum);
    I2C_stop();
    return error;
}


//===========================================================================
//===========================================================================
uint8_t SHT2x_WriteUserRegister(uint8_t *pRegisterValue)
{
    uint8_t error=0; //variable for error code
    I2C_start();
    I2C_send_byte (Board_SHT2x_ADDR<<1);
    if(I2C_ack_receive() == false)
        error |= 0x01;
    I2C_send_byte (USER_REG_W);
    if(I2C_ack_receive() == false)
        error |= 0x01;
    I2C_send_byte (*pRegisterValue);
    if(I2C_ack_receive() == false)
        error |= 0x01;
    I2C_stop();
    return error;
}


//===========================================================================
uint8_t SHT2x_MeasurePoll(uint8_t eSHT2xMeasureType, uint8_t *pMeasurand)
//===========================================================================
{
    uint8_t checksum; //checksum
    uint8_t error=0; //error variable
    uint16_t i=0; //counting variable

    //-- write I2C sensor address and command --
    I2C_start();
    I2C_send_byte (Board_SHT2x_ADDR<<1); // I2C Adr
    if(I2C_ack_receive() == false)
        error |= 0x01;
    switch(eSHT2xMeasureType)
    { 
        // HUMIDITY
        case 0: 
        I2C_send_byte (TRIG_H_MEASURE_POLL);
        if(I2C_ack_receive() == false)
        error |= 0x01;
        break;
        // TEMP
        case 1 :
        I2C_send_byte (TRIG_T_MEASURE_POLL);
        if(I2C_ack_receive() == false)
        error |= 0x01;
        break;
    }
    //-- poll every 10ms for measurement ready. Timeout after 20 retries (200ms)--

    do
    { 
        I2C_start();
        if(i++ >= 20) break;
        Task_sleep(25 * CLOCK_UNIT_MS);
        I2C_send_byte(Board_SHT2x_ADDR<<1 | 0x01);
    } while(I2C_ack_receive() == false);
    
    if (i>=20) error |= 0x01;

    //-- read two data bytes and one checksum byte --
    pMeasurand[0] = I2C_receive_byte();
    I2C_ack_send();

    pMeasurand[1] = I2C_receive_byte();
    I2C_ack_send();

    checksum      = I2C_receive_byte();
    I2C_noack_send();
    //-- verify checksum --
    error |= SHT2x_check_crc (pMeasurand,2,checksum);
    I2C_stop();
    return error;
}




//***********************************************************************************
//
// SHT2x measure.
//
//***********************************************************************************
static void SHT2x_measure(uint8_t chNum)
{
    uint8_t userRegister;
    uint8_t error;
    uint8_t buff[3];
    uint8_t retrys = 0;//?????3?

    if (g_rSysConfigInfo.sensorModule[chNum] == SEN_TYPE_SHT2X) {

        if (g_rSysConfigInfo.sensorModule[chNum] == SEN_TYPE_SHT2X) {
            SHT2x_init(chNum);
            retrys = 0;
            // Task_sleep(300 * CLOCK_UNIT_MS);
err_retrys:
            
            error |= SHT2x_ReadUserRegister(&userRegister); //get actual user reg
            userRegister = (userRegister & ~SHT2x_RES_MASK) | SHT2x_RES_8_12BIT;
            error |= SHT2x_WriteUserRegister(&userRegister); //write changed user reg

            error |= SHT2x_MeasurePoll(1, buff);
            if(error == 0)
            {
                buff[1] &= ~0x03;

                HIBYTE(rSensorData[chNum].temp) = buff[0];
                LOBYTE(rSensorData[chNum].temp) = buff[1];
            }
            else
            {
                rSensorData[chNum].temp = 0;
                if(retrys++ < 3)goto err_retrys;
            }

            error |= SHT2x_ReadUserRegister(&userRegister); //get actual user reg
            userRegister = (userRegister & ~SHT2x_RES_MASK) | SHT2x_RES_10_13BIT;
            error |= SHT2x_WriteUserRegister(&userRegister); //write changed user reg

            error |= SHT2x_MeasurePoll(0, buff);
            if(error == 0)
            {
                buff[1] &= ~0x03;

                HIBYTE(rSensorData[chNum].humi) = buff[0];
                LOBYTE(rSensorData[chNum].humi) = buff[1];
            }
            else
            {
                rSensorData[chNum].humi = 0;
                if(retrys++ < 3)goto err_retrys;
            }

#ifndef SUPPORT_BOARD_OLD_S1
            //convert rawdata to temperature
            rSensorData[chNum].temp = SHT2x_calc_temperatureC(rSensorData[chNum].temp );
            //convert rawdata to humidty
            rSensorData[chNum].humi= SHT2x_calc_humidty(rSensorData[chNum].humi);
#endif
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

