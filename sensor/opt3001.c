//***********************************************************************************
// Copyright 2017, Zksiot Development Ltd.
// Created by min.zeng, 2017.10.24
// MCU: MSP430F5529
// OS: TI-RTOS
// Project:
// File name: gps.c
// Description: Display API function process routine.
//***********************************************************************************
#include "../general.h"
#ifdef SUPPORT_OPT3001
#include "opt3001.h"
//***********************************************************************************
//
// opt init.
//
//***********************************************************************************
static uint8_t S_OPT_CHANGLE = 0;
//***********************************************************************************
//
// OPT301 calculate humidty.
//
//***********************************************************************************
static void opt3001_set_int_config(void)
{
    uint8_t buff_write[3] ={0};
    buff_write[0] = CMD_SET_CONFIGURATION_REG;
    buff_write[1] = (uint8_t)(CMD_CTRL_RESET >> 8);
    buff_write[2] = (uint8_t)(CMD_CTRL_RESET);
    I2c_write(Board_OPT3001_ADDR, buff_write, 3);
}

static void OPT3001_init(uint8_t chNum)
{
    uint8_t DEVICE_ID[2]      ={0};
    S_OPT_CHANGLE = chNum;
    uint8_t cnt = 0;
    uint8_t buff_write[3] = {0};
    for(cnt = 0; cnt < MODULE_SENSOR_MAX; cnt++){
      if(g_rSysConfigInfo.sensorModule[cnt] == SEN_TYPE_SHT2X)  {
          S_OPT_CHANGLE = cnt;
          break;
      }
    }
    I2c_regRead(Board_OPT3001_ADDR, CMD_DEVICE_ID_REG,DEVICE_ID, 2);
    opt3001_set_int_config();
    buff_write[0] = CMD_LOW_LIMIT_REG;
    buff_write[1] =  0x10;
    buff_write[2] = 0x0A;
    I2c_write(Board_OPT3001_ADDR, buff_write, 3);
    buff_write[0] = CMD_HIG_LIMIT_REG;
    buff_write[1] = 0x10;
    buff_write[2] = 0x64;
    I2c_write(Board_OPT3001_ADDR, buff_write, 3);

}

//***********************************************************************************
//
// SHT2x measure.
//
//***********************************************************************************
static void OPT3001_measure(uint8_t chNum)
{
    uint8_t buff_read[2]  = {0};
    uint32_t lux_hex = 0;
    uint32_t pow_n = 0;

    I2c_regRead(Board_OPT3001_ADDR, CMD_RESULT_REG,buff_read, 2);

    HIBYTE(LOWORD(lux_hex)) = buff_read[0];
    LOBYTE(LOWORD(lux_hex)) = buff_read[1];
    pow_n   = (uint32_t)(buff_read[0] >> 4);
    lux_hex = (uint32_t)((buff_read[0]&0x0f) << 8);
    lux_hex = (uint32_t)(lux_hex+buff_read[1]);

    switch(pow_n)
    {
    case RANG_0...RANG_2:
         pow_n = 1;
    break;
    case RANG_3...RANG_6:
         pow_n = 8;
    break;
    case RANG_7:
         pow_n = 128;
    break;
    case RANG_8:
         pow_n = 256;
    break;
    case RANG_9:
         pow_n = 512;
    break;
    case RANG_A:
         pow_n = 1024;
    break;
    case RANG_B:
         pow_n = 2048;
    break;
    }

    rSensorData[chNum].lux = pow_n*lux_hex;


    Task_sleep(60 * CLOCK_UNIT_MS);
    memset(buff_read,0x00,sizeof(buff_read));
    I2c_regRead(Board_OPT3001_ADDR,CMD_SET_CONFIGURATION_REG,buff_read,2);
    if ( buff_read[1]&have_hig_limit_event ){
        rSensorData[chNum].lux = (rSensorData[chNum].lux |0x01000000);
    } else {
        rSensorData[chNum].lux = (rSensorData[chNum].lux |0x02000000);
    }


    memset(buff_read,0x00,sizeof(buff_read));
    Task_sleep(12 * CLOCK_UNIT_MS);
    I2c_regRead(Board_OPT3001_ADDR,CMD_LOW_LIMIT_REG,buff_read,2);
    memset(buff_read,0x00,sizeof(buff_read));
    Task_sleep(12 * CLOCK_UNIT_MS);
    I2c_regRead(Board_OPT3001_ADDR,CMD_HIG_LIMIT_REG,buff_read,2);
}

//***********************************************************************************
//
// OPT301 calculate humidty.
//
//***********************************************************************************
static int32_t OPT3001_get_value(uint8_t chNum, SENSOR_FUNCTION function)
{
    if (g_rSysConfigInfo.sensorModule[chNum] == SEN_TYPE_OPT3001) {

        if(function & SENSOR_LIGHT){
            return rSensorData[chNum].lux&0x00ffffff;
        }
    }
    return TEMPERATURE_OVERLOAD;
}

const Sensor_FxnTable  OPT3001_FxnTable = {
    SENSOR_LIGHT,
    OPT3001_init,
    OPT3001_measure,
    OPT3001_get_value,
};

#endif
