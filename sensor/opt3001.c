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

#define   OPT3001_CONFIG_FC_1_TIMES     0x00
#define   OPT3001_CONFIG_FC_2_TIMES     0X01
#define   OPT3001_CONFIG_FC_4_TIMES     0X02
#define   OPT3001_CONFIG_FC_8_TIMES     0X03

#define   OPT3001_CONFIG_MODE_SHUTDOW       0X00 //关断模式
#define   OPT3001_CONFIG_MODE_SINGLE        0X01 //单次测量
#define   OPT3001_CONFIG_MODE_CONTINUE      0X03 //持续测量

#define   OPT3001_CONFIG_SCALE_40_95_LUX        0x00
#define   OPT3001_CONFIG_SCALE_81_90_LUX        0x01
#define   OPT3001_CONFIG_SCALE_163_80_LUX       0x02
#define   OPT3001_CONFIG_SCALE_327_60_LUX       0x03
#define   OPT3001_CONFIG_SCALE_655_20_LUX       0x04
#define   OPT3001_CONFIG_SCALE_1310_40_LUX      0x05
#define   OPT3001_CONFIG_SCALE_2620_80_LUX      0x06
#define   OPT3001_CONFIG_SCALE_5241_60_LUX      0x07
#define   OPT3001_CONFIG_SCALE_10483_20_LUX     0x08
#define   OPT3001_CONFIG_SCALE_20966_40_LUX     0x09
#define   OPT3001_CONFIG_SCALE_41932_80_LUX     0x0A
#define   OPT3001_CONFIG_SCALE_83865_60_LUX     0x0B
#define   OPT3001_CONFIG_FULL_SCALE_83865_60_LUX     0x0C


// 单位0.01lux
static const uint16_t OPT3001_RESOLUTION[]={
    1,2,4,8,16,32,64,128,256,512,1024,2048,2048
};

#pragma pack (1)

typedef union 
{
    uint16_t configReg;
    struct 
    {
        uint16_t FC:2;       //设置故障次数到达后触发中断
        uint16_t ME:1;       
        uint16_t POL:1;      //中断有效电频设置
        uint16_t L:1;        //中断机制选择
        uint16_t FL:1;       //累计小于最低阈值达到设置的次数，该位置1，读取后清除
        uint16_t FH:1;       //累计大于最大阈值达到设置的次数，该位置1，读取后清除
        uint16_t CRF:1;      //转换完成标记位
        uint16_t OVF:1;      //超过量程标记位
        uint16_t M:2;        //工作模式00：关断模式 01：单次触发模式，10,11：持续测量
        uint16_t CT:1;       //采样时间：0:100ms  1:800 ms
        uint16_t RN:4;       //采样范围设置
    };
}OPT3001_CONFIGURATION_T;

typedef union{
    uint16_t data;
    struct 
    {
        uint16_t value:12;
        uint16_t scale:4;
    };
}OPT3001_SCALE_VALUE_T;

#pragma pack ()

// #define OPT3001_SINGLE_MODE

//***********************************************************************************
//
// opt init.
//
//***********************************************************************************
static uint8_t S_OPT_CHANGLE = 0;


#define     OPT3001_INT_PIN     IOID_27
static PIN_State   optPinState;
static PIN_Handle  optPinHandle = NULL;


const PIN_Config optPinTable[] = {
    OPT3001_INT_PIN | PIN_INPUT_EN | PIN_IRQ_NEGEDGE,       /* key isr enable          */
    PIN_TERMINATE
};


//***********************************************************************************
//
// OPT301 Int Cb.
//
//***********************************************************************************
void Opt3001_int(void)
{
    openBoxOccur = 1;
    Sys_event_post(SYS_EVT_SENSOR);
}

//***********************************************************************************
//
// OPT301 calculate humidty.
//
//***********************************************************************************
static void opt3001_set_int_config(void)
{
    uint8_t buff_write[3] ={0};
    OPT3001_CONFIGURATION_T config;
    config.configReg = 0;
#ifdef OPT3001_SINGLE_MODE
    config.M = OPT3001_CONFIG_MODE_SHUTDOW;
#else
    config.M = OPT3001_CONFIG_MODE_CONTINUE;
#endif //OPT3001_SINGLE_MODE
    config.POL = 0;     //中断脚高电平有效
    config.L   = 0;     //窗口中断模式，当超出预设范围，中断脚有效
    config.RN  = OPT3001_CONFIG_FULL_SCALE_83865_60_LUX;    //全量程

    buff_write[0] = CMD_SET_CONFIGURATION_REG;
    buff_write[1] = (uint8_t)(config.configReg >> 8);
    buff_write[2] = (uint8_t)(config.configReg);
    I2c_write(Board_OPT3001_ADDR, buff_write, 3);
}

static void OPT3001_init(uint8_t chNum)
{
    uint8_t DEVICE_ID[2]      ={0};
    OPT3001_SCALE_VALUE_T limitSet;
    S_OPT_CHANGLE = chNum;
    uint8_t cnt = 0;
    uint8_t buff_write[3] = {0};

    if(optPinHandle == NULL) {
        optPinHandle = PIN_open(&optPinState, optPinTable);
        PIN_registerIntCb(optPinHandle, (PIN_IntCb)Opt3001_int);
    }

    for(cnt = 0; cnt < MODULE_SENSOR_MAX; cnt++){
        if(g_rSysConfigInfo.sensorModule[cnt] == SEN_TYPE_SHT2X)  {
            S_OPT_CHANGLE = cnt;
            break;
        }
    }
    I2c_regRead(Board_OPT3001_ADDR, CMD_DEVICE_ID_REG,DEVICE_ID, 2);
    opt3001_set_int_config();

    // 设置小于0.2lux触发中断
    limitSet.scale = OPT3001_CONFIG_SCALE_40_95_LUX;
    limitSet.value = 100;        //0.01 * 100

    buff_write[0] = CMD_LOW_LIMIT_REG;
    buff_write[1] = (uint8_t)(limitSet.data >> 8);
    buff_write[2] = (uint8_t)(limitSet.data);
    I2c_write(Board_OPT3001_ADDR, buff_write, 3);

    // 设置大于2lux触发中断
    limitSet.scale = OPT3001_CONFIG_SCALE_40_95_LUX;
    limitSet.value = 100;        //0.01 * 100

    buff_write[0] = CMD_HIG_LIMIT_REG;
    buff_write[1] = (uint8_t)(limitSet.data >> 8);
    buff_write[2] = (uint8_t)(limitSet.data);
    I2c_write(Board_OPT3001_ADDR, buff_write, 3);

}

//***********************************************************************************
//
// SHT2x measure.
//
//***********************************************************************************
static void OPT3001_measure(uint8_t chNum)
{
    uint8_t buff_read[2]  = {0}, buff_write[3];
    OPT3001_CONFIGURATION_T configTemp;
    OPT3001_SCALE_VALUE_T  valueTemp;

#ifdef OPT3001_SINGLE_MODE
    memset(buff_read,0x00,sizeof(buff_read));
    I2c_regRead(Board_OPT3001_ADDR,CMD_SET_CONFIGURATION_REG,buff_read,2);

    HIBYTE_ZKS(LOWORD_ZKS(configTemp.configReg)) = buff_read[0];
    LOBYTE_ZKS(LOWORD_ZKS(configTemp.configReg)) = buff_read[1];
    configTemp.M = OPT3001_CONFIG_MODE_SINGLE;

    buff_write[0] = CMD_SET_CONFIGURATION_REG;
    buff_write[1] = (uint8_t)(configTemp.configReg >> 8);
    buff_write[2] = (uint8_t)(configTemp.configReg);
    I2c_write(Board_OPT3001_ADDR, buff_write, 3);

    if(configTemp.CT)
        Task_sleep(850 * CLOCK_UNIT_MS); //
    else
        Task_sleep(120 * CLOCK_UNIT_MS); //
#endif //OPT3001_SINGLE_MODE

    I2c_regRead(Board_OPT3001_ADDR, CMD_RESULT_REG,buff_read, 2);
    if(buff_read[0] == 0xff && buff_read[1] == 0xff){
        rSensorData[chNum].lux = LIGHT_OVERLOAD;
        return;
    }

    HIBYTE_ZKS(LOWORD_ZKS(valueTemp.data)) = buff_read[0];
    LOBYTE_ZKS(LOWORD_ZKS(valueTemp.data)) = buff_read[1];
    if(valueTemp.scale > OPT3001_CONFIG_FULL_SCALE_83865_60_LUX)
        valueTemp.scale = OPT3001_CONFIG_FULL_SCALE_83865_60_LUX;


    rSensorData[chNum].lux = (uint32_t)valueTemp.value * OPT3001_RESOLUTION[valueTemp.scale];


    Task_sleep(60 * CLOCK_UNIT_MS);
    memset(buff_read,0x00,sizeof(buff_read));
    I2c_regRead(Board_OPT3001_ADDR,CMD_SET_CONFIGURATION_REG,buff_read,2);

    HIBYTE_ZKS(LOWORD_ZKS(configTemp.configReg)) = buff_read[0];
    LOBYTE_ZKS(LOWORD_ZKS(configTemp.configReg)) = buff_read[1];


#ifndef OPT3001_SINGLE_MODE
    if ( configTemp.FH ){
        rSensorData[chNum].lux = (rSensorData[chNum].lux |0x01000000);
    } else {
        rSensorData[chNum].lux = (rSensorData[chNum].lux |0x02000000);
    }
#endif //OPT3001_SINGLE_MODE

    // memset(buff_read,0x00,sizeof(buff_read));
    // Task_sleep(12 * CLOCK_UNIT_MS);
    // I2c_regRead(Board_OPT3001_ADDR,CMD_LOW_LIMIT_REG,buff_read,2);
    // memset(buff_read,0x00,sizeof(buff_read));
    // Task_sleep(12 * CLOCK_UNIT_MS);
    // I2c_regRead(Board_OPT3001_ADDR,CMD_HIG_LIMIT_REG,buff_read,2);
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
            return rSensorData[chNum].lux;//include state & value 
        }
    }
    return LIGHT_OVERLOAD;
}

const Sensor_FxnTable  OPT3001_FxnTable = {
    SENSOR_LIGHT,
    OPT3001_init,
    OPT3001_measure,
    OPT3001_get_value,
};

#endif
