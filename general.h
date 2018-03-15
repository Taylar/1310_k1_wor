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
#include <ti/devices/cc13x0/driverlib/aon_ioc.h>
#include <ti/devices/cc13x0/driverlib/sys_ctrl.h>

#include <ti/drivers/ADC.h>
#include <ti/drivers/adc/ADCCC26XX.h>

/* TI-RTOS Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/I2C.h>
#include <ti/drivers/SPI.h>
#include <ti/drivers/Watchdog.h>
#include <ti/drivers/watchdog/WatchdogCC26XX.h>



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
// #define BOARD_S1_2
// #define BOARD_S2_2
#define BOARD_S6_6
// #define BOARD_CONFIG_DECEIVE


// #define FACTOR_RADIO_TEST

//***********************************************************************************
//
// FW version define.
//
//***********************************************************************************
#define FW_VERSION              0x0006


//***********************************************************************************
//
// default setting.
//
//***********************************************************************************
#define CONFIG_DECEIVE_ID_DEFAULT       0XFFFFFFFA

#define DECEIVE_ID_DEFAULT              0XFFFFFFFB

#define CUSTOM_ID_DEFAULT               0X0000

#define UPLOAD_PERIOD_DEFAULT           60

#define COLLECT_PERIOD_DEFAULT          60

#define HEARTBEAT_PERIOD_DEFAULT        60

#define NTC_DEFAULT                     (60*60)

#define NODE_BROADCASTING_TIME          UPLOAD_PERIOD_DEFAULT
#define NODE_BROADCAST_TESTRESUT_TIME   UPLOAD_PERIOD_DEFAULT

#define DEFAULT_DST_ADDR                0xFFFFFFFD


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


//wdt WatchDog define
// #define SUPPORT_WATCHDOG


#define SUPPORT_SENSOR
#ifdef SUPPORT_SENSOR
#define SUPPORT_SHT2X
#endif

/*
// *********************************************************
#ifdef BOARD_S1_2

//Flash define
#define FLASH_INTERNAL
#define FLASH_EXTERNAL
#ifdef FLASH_EXTERNAL
//#define FLASH_W25Q256FV
#endif

#define SUPPORT_SENSOR
#ifdef SUPPORT_SENSOR
#define SUPPORT_SHT2X
#endif


//Led define
#define SUPPORT_LED

#endif

// *********************************************************
#ifdef BOARD_S2_2
//Flash define
#define FLASH_INTERNAL
#define FLASH_EXTERNAL
#ifdef FLASH_EXTERNAL
//#define FLASH_W25Q256FV
#endif

#define SUPPORT_DEEPTEMP

//Network define
#define SUPPORT_NETWORK
#ifdef SUPPORT_NETWORK
#define SUPPORT_GSM
#endif


//Led define
#define SUPPORT_LED


//Battery function define.
#define SUPPORT_BATTERY

#endif


// *********************************************************
#ifdef BOARD_S6_6

//Flash define
#define FLASH_INTERNAL
#define FLASH_EXTERNAL
#ifdef FLASH_EXTERNAL
//#define FLASH_W25Q256FV
#endif

//Network define
#define SUPPORT_NETWORK
#ifdef SUPPORT_NETWORK
#define SUPPORT_GSM
#endif


//LCD define
#define SUPPORT_DISP_SCREEN
#ifdef SUPPORT_DISP_SCREEN
//#define AUTO_SHUTDOWN_LCD
//Electronic Paper Display module
//#define EPD_GDE0213B1
#define LCD_ST7567A
#define SUPPORT_MENU
//#define SUPPORT_NETGATE_DISP_NODE
#endif


//Led define
#define SUPPORT_LED

//Battery function define.
#define SUPPORT_BATTERY

#endif
// *********************************************************
*/
//ADC define.

//Flash define
#define FLASH_INTERNAL
#define FLASH_EXTERNAL
#ifdef FLASH_EXTERNAL
//#define FLASH_W25Q256FV
#endif

#define SUPPORT_SENSOR
#ifdef SUPPORT_SENSOR
#define SUPPORT_SHT2X
#endif


//Led define
#define SUPPORT_LED

//Battery function define.
#define SUPPORT_BATTERY


//LCD define
#define SUPPORT_DISP_SCREEN
#ifdef SUPPORT_DISP_SCREEN
//#define AUTO_SHUTDOWN_LCD
//Electronic Paper Display module
//#define EPD_GDE0213B1
#define LCD_ST7567A
#define SUPPORT_MENU
#define SUPPORT_NETGATE_DISP_NODE

#endif

//Network define
#define SUPPORT_NETWORK
#ifdef SUPPORT_NETWORK
#define SUPPORT_GSM
#endif

// deep temperature
#define SUPPORT_DEEPTEMP

// NTC

#ifdef  BOARD_S6_6
#define SUPPORT_NTC
#endif

#ifdef SUPPORT_NTC
//#define NTC_ELIWELL_10K
//#define NTC_KEMIT_10K
//#define NTC_KEMIT_100K
//#define NTC_KEMIT_PT1000
#define NTC_XINXIANG_10K
//#define NTC_TIANYOU_10K

#endif



//Radio define

//CRC function define.



//Queue function define
#define QUEUE_ALLOW_COVER

//SPI define
#define USCI_B0_SPI

//I2C define
#define I2C_BUS







typedef enum {
    SENSOR_NONE = 0x00,
    SENSOR_TEMP = 0x01,
    SENSOR_HUMI = 0x02,
    SENSOR_CO   = 0x04,
    SENSOR_CO2  = 0x08,
    SENSOR_DEEP_TEMP = 0x10,
} SENSOR_FUNCTION;

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



#define MODULE_SENSOR_MAX       8
typedef enum {
    SEN_TYPE_NONE = 0,
    SEN_TYPE_SHT2X,
    SEN_TYPE_NTC,
    SEN_TYPE_LIGHT,
    SEN_TYPE_DEEPTEMP,
    SEN_TYPE_HCHO,
    SEN_TYPE_PM25,
    SEN_TYPE_CO2,
    SEN_TYPE_MAX
} SENSOR_TYPE;

#define ALARM_TEMP_HIGH         0x7fff
#define ALARM_TEMP_LOW          (-0x7fff)


//***********************************************************************************
//
//Status define.
//
//***********************************************************************************
#define STATUS_LCD_ALWAYS_ON    0x0001
#define STATUS_GSM_TEST         0x0002
#define STATUS_KEY_LED_ON       0x0004
#define STATUS_ALARM_OFF        0x0008//本地报警开关
#define STATUS_SENSOR_NAME_ON   0x0010//网关显示采集器名字开关


#define STATUS_LORA_MASTER      0x0001//
#define STATUS_LORA_TEST        0x0002//LORA 测试模式
#define STATUS_LORA_APC         0x0004//采集器自动增益控制
#define STATUS_LORA_ALARM       0x0008//采集器发送报警信息到网关





//***********************************************************************************
//
//mode define.
//
//***********************************************************************************
// devices type
#define         DEVICES_TYPE_GATEWAY            0
#define         DEVICES_TYPE_NODE               1

// device power mode 
#define         DEVICES_OFF_MODE               0
#define         DEVICES_ON_MODE                1
#define         DEVICES_MENU_MODE              2
#define         DEVICES_CONFIG_MODE            3
#define         DEVICES_SLEEP_MODE             4
#define         DEVICES_TEST_MODE              5

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
    
}__attribute__((packed)) ConfigInfo_t;

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
#include "battery/battery.h"
// #include "driver/adc_drv.h"
#include "driver/rtc_drv.h"
#include "driver/i2c_drv.h"
#include "driver/spi_drv.h"
#include "driver/adc_drv.h"
#include "driver/wdt_drv.h"
#include "driver/uart_drv.h"
#include "driver/usbInt_drv.h"
#include "flash/extflash.h"
#include "flash/extflash_ringq.h"
#include "flash/internalFlash.h"
#include "key/key_proc.h"
#include "sensor/max31855.h"
#include "sensor/sht2x.h"
#include "sensor/ntc.h"
#include "display/display.h"
#include "display/menu.h"
#include "display/led_drv.h"
#include "easylink/EasyLink.h"
#include "easylink/radio_front.h"
#include "radio_app/radio_app.h"
#include "radio_app/node_strategy.h"
#include "network/network.h"
#include "app/concenterApp.h"
#include "app/usb_prot.h"
#include "app/radio_protocal.h"
#include "app/nodeApp.h"
#include "app/concenterApp.h"
#include "app/systemApp.h"
#include "app/s1_app.h"
#include "app/s2_app.h"
#include "app/s6_app.h"

//***********************************************************************************
//
//	Global variable define
//
//***********************************************************************************
extern ConfigInfo_t g_rSysConfigInfo;


extern uint8_t deviceMode;

#endif	/* __ZKSIOT_GENERAL_H__ */
