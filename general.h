//***********************************************************************************
// Copyright 2017, Zksiot Development Ltd.
// Created by Linjie, 2017.08.08
// MCU:	MSP430F5529
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
#include <ctype.h>
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

#include <ti/devices/cc13x0/inc/hw_rfc_dbell.h>
#include <ti/devices/cc13x0/driverlib/aon_batmon.h>
#include <ti/devices/cc13x0/driverlib/flash.h>
#include <ti/devices/cc13x0/driverlib/aon_ioc.h>
#include <ti/devices/cc13x0/driverlib/sys_ctrl.h>
#include <ti/devices/cc13x0/driverlib/rf_prop_cmd.h>

#include <ti/drivers/ADC.h>
#include <ti/drivers/adc/ADCCC26XX.h>

#include <ti/drivers/PWM.h>
#include <ti/drivers/pwm/PWMTimerCC26XX.h>

/* TI-RTOS Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/I2C.h>
#include <ti/drivers/SPI.h>
#include <ti/drivers/Watchdog.h>
#include <ti/drivers/watchdog/WatchdogCC26XX.h>



#ifdef VARIABLES_DEFINE
#define EXTERN_ATTR
#else 
#define EXTERN_ATTR extern
#endif

//***********************************************************************************
//
// HW version define.
//
//***********************************************************************************
// #define BOARD_S1_2
//#define BOARD_S2_2
#define BOARD_S6_6
// #define BOARD_CONFIG_DECEIVE

//#define FACTOR_RADIO_TEST

// #define RADIO_1310_50K_GPSK

//***********************************************************************************
// S6_6 board define
//***********************************************************************************
#ifdef BOARD_S6_6
#define SUPPORT_SENSOR
#define SUPPORT_SHT2X

#define SUPPORT_RADIO


#define SUPPORT_DISP_SCREEN

#define SUPPORT_NETGATE_DISP_NODE

#define SUPPORT_MENU

#define SUPPORT_NETWORK

//#define SUPPORT_LIGHT

#define SUPPORT_ENGMODE

// #define SKY_66115

#define SUPPORT_REMOTE_UPGRADE

#define SUPPORT_USB_UPGRADE

#define SUPPORT_STRATEGY_SORT

// system state info upload
#ifndef BOARD_CONFIG_DECEIVE
//#define SUPPORT_DEVICED_STATE_UPLOAD
#endif
#endif

//***********************************************************************************
// S2_2 board define
//***********************************************************************************
#ifdef BOARD_S2_2
#define SUPPORT_SENSOR
#define SUPPORT_SHT2X
#define SUPPORT_DEEPTEMP_PT100

#define SUPPORT_RADIO

#define SUPPORT_NETWORK
#define SUPPORT_REMOTE_UPGRADE
#define SUPPORT_USB_UPGRADE
#endif

//***********************************************************************************
// S1_2/3 board define
//***********************************************************************************
#ifdef BOARD_S1_2
#define SUPPORT_SENSOR
#define SUPPORT_SHT2X

#define SUPPORT_RADIO

#define SUPPORT_RADIO_UPGRADE

/* old S1*/
//#define SUPPORT_BOARD_OLD_S1
#endif

//***********************************************************************************
// config board define
//***********************************************************************************
#ifdef  BOARD_CONFIG_DECEIVE

#undef SUPPORT_SHT2X
#undef SUPPORT_MENU

#endif //BOARD_CONFIG_DECEIVE



//***********************************************************************************
//
// FW version define.
//
//***********************************************************************************
#ifdef BOARD_S6_6
#define FW_VERSION              0x0030
#endif

#ifdef BOARD_S1_2
    #ifndef SUPPORT_BOARD_OLD_S1
    #define FW_VERSION              0x0030
    #else
    #define FW_VERSION              0x0004
    #endif
#endif

#ifdef BOARD_S2_2
#define FW_VERSION              0x0030
#endif


//***********************************************************************************
//
// default setting.
//
//***********************************************************************************
#define CONFIG_DECEIVE_ID_DEFAULT       0XFFFFFFFA
#define RADIO_UPGRADE_ADDRESS           0x99999999

#define DECEIVE_ID_DEFAULT              0X88888886

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

//Device type define.
//#define END_DEVICE
//#define ACCESS_POINT
//#define RANGE_EXTENDER

//WatchDog define
#define SUPPORT_WATCHDOG


//ADC define.

//Flash define
#define FLASH_INTERNAL
#define FLASH_EXTERNAL
#ifdef FLASH_EXTERNAL
//#define FLASH_W25Q256FV  //大于128Mb=16MB,采用4字节地址模式。
//#define FLASH_SIZE_128M
//#define FLASH_SENSOR_DATA_32BYTE
//#define SUPPORT_FLASH_LOG
//#define MX66L1G45G  //1G flash
#endif



#ifdef SUPPORT_NTC
//#define NTC_ELIWELL_10K
//#define NTC_KEMIT_10K
//#define NTC_KEMIT_100K
//#define NTC_KEMIT_PT1000
//#define NTC_KEMIT_PT1000_2ADC
#define NTC_XINXIANG_10K
//#define NTC_TIANYOU_10K
#endif

//CRC function define.
#define SUPPORT_CRC16
#ifdef SUPPORT_CRC16
//#define CRC16_HW_MODULE
#ifndef CRC16_HW_MODULE
//#define CRC16_MSB
#define CRC16_LSB
#endif
#endif

//Radio define
#ifdef SUPPORT_RADIO
#define SUPPORT_CC1310
#endif


//Led define
#define SUPPORT_LED

//Battery function define.
#define SUPPORT_BATTERY


//LCD define
#ifdef SUPPORT_DISP_SCREEN

//#define EPD_GDE0213B1
#define LCD_ST7567A
#define SUPPORT_MENU


//#define SUPPORT_NETGATE_BIND_NODE   //网关绑定的节点，需要收到数据后判断是否超温
#define NETGATE_BIND_NODE_MAX       30

#endif

//Led define
#define SUPPORT_LED

//Battery function define.
#define SUPPORT_BATTERY
#ifdef SUPPORT_BATTERY
//#define SUPPORT_ADP5062
#endif

//Network define
#ifdef SUPPORT_NETWORK
#define SUPPORT_GSM
#define SUPPORT_GSM_SHORT_CONNECT
#define SUPPORT_LBS
#ifdef SUPPORT_LBS
//#define USE_QUECTEL_API_FOR_LBS
#define USE_ENGINEERING_MODE_FOR_LBS
#define SUPPOERT_LBS_NEARBY_CELL
#endif
#define SUPPORT_IMEI

#define PACKAGE_ITEM_COUNT_MAX 10
#define SUPPORT_ZKS_PROTOCOL

#endif

#ifdef SUPPORT_NTC
//#define NTC_ELIWELL_10K
//#define NTC_KEMIT_10K
//#define NTC_KEMIT_100K
//#define NTC_KEMIT_PT1000
#define NTC_XINXIANG_10K
//#define NTC_TIANYOU_10K

#endif


#ifdef SUPPORT_LIGHT
#define SUPPORT_OPT3001
#endif

//Radio define

//CRC function define.



//Queue function define
#define QUEUE_ALLOW_COVER

//SPI define
#define USCI_B0_SPI

//I2C define
#define I2C_BUS
#define SOFT_I2C_BUS



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
#define MODULE_LCD              0x0080

#define MODULE_NWK              (MODULE_GSM | MODULE_WIFI | MODULE_LAN)
#define MODULE_RADIO            (MODULE_LORA| MODULE_CC1310)


#define MODULE_SENSOR_MAX       8
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
    SEN_TYPE_ORG_POLL,
    SEN_TYPE_HLW8012,
    SEN_TYPE_MAX
} SENSOR_TYPE;

#define ALARM_TEMP_HIGH         0x7fff
#define ALARM_TEMP_LOW          (-0x7fff)


//***********************************************************************************
//
//Status define.
//
//***********************************************************************************
#define STATUS_LCD_ALWAYS_ON    0x0001//lcd常开
#define STATUS_GSM_TEST         0x0002//gsm测试模式
#define STATUS_KEY_LED_ON       0x0004//按键灯亮灯
#define STATUS_ALARM_OFF        0x0008//本地报警开关
#define STATUS_SENSOR_NAME_ON   0x0010//网关显示采集器名字开关
#define STATUS_DISP_NOBINDNODE  0x0020//网关显示非绑定设备的信息开关



#define STATUS_LORA_TEST      	0x0002//LORA 测试模式
#define STATUS_LORA_APC      	0x0004//采集器自动增益控制
#define STATUS_LORA_ALARM      	0x0008//采集器发送报警信息到网关
#define STATUS_LORA_CHANGE_FREQ 0x0010//LORA的中心频根据客户码变化
#define STATUS_1310_MASTER      0x0020//1310做master或slaver


//***********************************************************************************
//
//sysState define.
//
//***********************************************************************************
#define STATUS_POWERON          0x0001

//***********************************************************************************
//
//mode define.
//
//***********************************************************************************
// devices type
#define         DEVICES_TYPE_GATEWAY            0
#define         DEVICES_TYPE_NODE               1

// device power mode 
#define         DEVICES_OFF_MODE               0    // power down mode
#define         DEVICES_ON_MODE                1    // power on mode
#define         DEVICES_MENU_MODE              2    // in the menu display
#define         DEVICES_CONFIG_MODE            3    // 
#define         DEVICES_SLEEP_MODE             4
#define         DEVICES_TEST_MODE              5

#ifdef SUPPORT_BOARD_OLD_S1
#define        OLD_S1_DEVICES_RADIO_UPGRADE    6
#endif

//***********************************************************************************
//
//system event define.
//
//***********************************************************************************

#include "driver/rtc_drv.h"

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

#pragma pack (1)

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
    uint32_t    Deviceid;
    int8_t      ChNo;
    AlarmTemp_t AlarmInfo;
} BindNode_t;

typedef struct {
    uint32_t   err_restarts;
    uint32_t   i2c_errors;
    uint32_t   lora_send_errors;        
    uint32_t   lora_list_fulls; 
    uint32_t   reserve;    
}SystemState_t;

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
     //Sensor warning  temperature
    AlarmTemp_t WarningTemp[MODULE_SENSOR_MAX];    

    //bind gateway
    uint8_t BindGateway[4];
    
    //record  rtc time
    Calendar  rtc;

    //offset 128
    //apn user pwd 
    uint8_t apnuserpwd[64];
    uint8_t serverAddr[64];

   //offset 256
    SystemState_t  sysState;  //size =20
    
#ifdef SUPPORT_NETGATE_BIND_NODE   //
    //offset 256 +20    
    BindNode_t  bindnode[NETGATE_BIND_NODE_MAX];

#endif
    
} ConfigInfo_t;
#pragma pack ()


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
#include "driver/adc_drv.h"
#include "driver/rtc_drv.h"

#ifdef   SOFT_I2C_BUS
#include "driver/soft_i2c_drv.h"
#else
#include "driver/i2c_drv.h"
#endif

#include "driver/spi_drv.h"
#include "driver/wdt_drv.h"
#include "driver/uart_drv.h"
#include "driver/usbInt_drv.h"
#include "driver/pwm_drv.h"
#include "flash/extflash.h"
#include "flash/extflash_ringq.h"
#include "flash/internalFlash.h"
#include "key/key_proc.h"
#include "display/display.h"
#include "display/menu.h"
#include "display/led_drv.h"
#include "sensor/sensor.h"
#include "battery/battery.h"
#include "easylink/EasyLink.h"
#include "radio_app/radio_app.h"
#include "radio_app/node_strategy.h"
#include "network/network.h"
#include "network/upgrade.h"
#include "interface_app/interface.h"
#include "usb/usb_proc.h"
#include "app/concenterApp.h"
#include "app/radio_protocal.h"
#include "app/nodeApp.h"
#include "app/concenterApp.h"
#include "app/systemApp.h"
#include "app/s1_app.h"
#include "app/s2_app.h"
#include "app/s6_app.h"
#include "engmode/engmode.h"
#include "usb/usb_bsl.h"
#include "radio_app/radio_upgrade.h"

#ifdef SUPPORT_BOARD_OLD_S1
    #include "app/old_s1/old_s1_node_app.h"
#endif

#ifdef SUPPORT_DEEPTEMP_PT100
    #include "sensor/ads1247/ads1247.h"
    #include "sensor/pt100.h"
#endif

//***********************************************************************************
//
//	Global variable define
//
//***********************************************************************************
EXTERN_ATTR volatile ConfigInfo_t g_rSysConfigInfo;

EXTERN_ATTR uint16_t     g_bAlarmSensorFlag; //
EXTERN_ATTR  Alarmdata_t    g_AlarmSensor;

extern uint8_t deviceMode;

#endif	/* __ZKSIOT_GENERAL_H__ */
