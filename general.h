//***********************************************************************************
// Copyright 2017, Zksiot Development Ltd.
// Created by Zhengxuntai, 2017.12.15
// MCU:	cc1310
// OS: TI-RTOS
// Project:
// File name: general.h
// Description: General define file.
//***********************************************************************************

#ifndef __ZKSIOT_GENERAL_H__
#define __ZKSIOT_GENERAL_H__

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

/* XDC module Headers */
#include <xdc/std.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>

/* BIOS module Headers */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Event.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Mailbox.h>
#include <ti/sysbios/knl/Swi.h>
#include <ti/sysbios/hal/Hwi.h>

#include <ti/devices/cc13x0/driverlib/aon_batmon.h>
#include <ti/devices/cc13x0/driverlib/flash.h>

/* TI-RTOS Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/I2C.h>
#include <ti/drivers/SPI.h>
#include <ti/drivers/Watchdog.h>



// #ifdef VARIABLES_DEFINE
#define EXTERN_ATTR
// #else
// #define EXTERN_ATTR extern
// #endif

//***********************************************************************************
//
// HW version define.
//
//***********************************************************************************




//***********************************************************************************
//
// FW version define.
//
//***********************************************************************************
#define FW_VERSION              0x0001

//***********************************************************************************
//
// MCLK define.
//
//***********************************************************************************
#define MCU_MCLK                48000000
#define XT1_CLK                 32768
#define XT2_CLK                 24000000

//***********************************************************************************
//
// Clock define.
//
//***********************************************************************************
#define CLOCK_UNIT_S            (1000000 / Clock_tickPeriod)
#define CLOCK_UNIT_MS           (1000 / Clock_tickPeriod)

//***********************************************************************************
//
// Function define.
//
//***********************************************************************************
// Mcu little-ending or big-ending define
#define LITTLE_ENDIAN


//WatchDog define
#define SUPPORT_WATCHDOG

//ADC define.

//Flash define
#define FLASH_INTERNAL
#define FLASH_EXTERNAL
#ifdef FLASH_EXTERNAL
//#define FLASH_W25Q256FV
#endif

//Sensor define
#define SUPPORT_DEEPTEMP

#define SUPPORT_SENSOR
#ifdef SUPPORT_SENSOR
#define SUPPORT_SHT2X
#endif
//CRC function define.


//Radio define

//Network define
#define SUPPORT_NETWORK
#ifdef SUPPORT_NETWORK
#define SUPPORT_GSM
#endif


//Queue function define
#define QUEUE_ALLOW_COVER

//SPI define
#define USCI_B0_SPI

//I2C define
#define I2C_BUS

//LCD define


//Led define
#define SUPPORT_LED

//Battery function define.
#define SUPPORT_BATTERY


//***********************************************************************************
//
//MODULE define.
//
//***********************************************************************************
//No MODULE
#define MODULE_NULL             0
#define MODULE_GSM              0x0001
#define MODULE_WIFI             0x0002
#define MODULE_LAN              0x0004
#define MODULE_LORA             0x0008
#define MODULE_CC1310           0x0010
#define MODULE_GPS              0x0020
#define MODULE_BTP              0x0040

#define MODULE_NWK              (MODULE_GSM | MODULE_WIFI | MODULE_LAN)
#define MODULE_RADIO            (MODULE_LORA| MODULE_CC1310)

#define MODULE_SENSOR_MAX       1


//***********************************************************************************
//
//Status define.
//
//***********************************************************************************
// devices type
#define         DEVICES_TYPE_GATEWAY            0
#define         DEVICES_TYPE_NODE               1

//***********************************************************************************
//
//system event define.
//
//***********************************************************************************


//***********************************************************************************
//
//struct define.
//
//***********************************************************************************


typedef enum {
    SYS_STATE_STANDBY = 0,
    SYS_STATE_POWERON,
    SYS_STATE_RUNNING,
    SYS_STATE_IDLE
} SYS_STATE;

typedef struct {
    //System state.
    SYS_STATE state;
    //System state step.
    uint8_t stateStep;
    //System alarm cnt.
    uint8_t alarmCnt;
} SysTask_t;

typedef struct {
    //High Temperature Alarm value
	int16_t high;
    //Low Temperature Alarm value
	int16_t low;
} AlarmTemp_t;

typedef struct {
	uint16_t size;
	uint16_t swVersion;
	uint8_t DeviceId[4];
	uint8_t customId[2];

    //Status Flag
	uint16_t status;
    //Module config
	uint16_t module;

    //Sensor config
    uint8_t sensorModule[MODULE_SENSOR_MAX];
    //Sensor alarm temperature
    AlarmTemp_t alarmTemp[MODULE_SENSOR_MAX];
    //Sensor collect period
	uint32_t collectPeriod;

    //Network server ip address
	uint8_t serverIpAddr[4];
    //Network server ip port
	uint16_t serverIpPort;
    //Network heartbeat period
	uint16_t hbPeriod;
    //Network upload period
	uint32_t uploadPeriod;
    //Network ntp period
	uint32_t ntpPeriod;

    //RF PA power setting.
	uint8_t rfPA;
    //RF bandwidth setting.
	uint8_t rfBW;
    //RF spreading factor setting.
	uint8_t rfSF;
    //RF status setting.
	uint8_t rfStatus;

    //GNSS collect period
	uint32_t gnssPeriod;
    
    //battery voltage of shutdown
    uint16_t batLowVol;
    

    uint8_t reserver[44];

    //apn user pwd
    uint8_t apnuserpwd[64];
    
} ConfigInfo_t;

//***********************************************************************************
//
// System I/O define.
//
//***********************************************************************************


//***********************************************************************************
//
// Include files.
//
//***********************************************************************************
#include "Board.h"
#include "function.h"
// #include "driver/adc_drv.h"
#include "driver/rtc_drv.h"
#include "driver/i2c_drv.h"
#include "driver/spi_drv.h"
#include "driver/uart_drv.h"
#include "flash/extflash.h"
#include "flash/internalFlash.h"
#include "key/key_proc.h"
#include "sensor/sht2x.h"
// #include "sensor/sensor.h"
#include "display/led_drv.h"
#include "easylink/EasyLink.h"
#include "network/network.h"

//***********************************************************************************
//
//	Global variable define
//
//***********************************************************************************
extern ConfigInfo_t g_rSysConfigInfo;

extern uint8_t devicesType;


#endif	/* __ZKSIOT_GENERAL_H__ */

