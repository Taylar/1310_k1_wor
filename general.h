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

#define PLATFORM_NAME   "_1310"
//***********************************************************************************
//
// HW version define.
//
//***********************************************************************************
// #define BOARD_S6_1
//#define BOARD_S6_2
//#define BOARD_S6_3
//#define BOARD_S6_4

// #define BOARD_S3
// #define BOARD_B2S
#define BOARD_S6_6
// #define BOARD_CONFIG_DECEIVE


//#define FACTOR_RADIO_TEST
// #define RADIO_1310_50K_GPSK

//***********************************************************************************
//
// FW version define.
//
//***********************************************************************************
#define FW_VERSION              0x0082

//***********************************************************************************
//
//Device type define.
//
//***********************************************************************************
// #define S_A//一体机
#define S_G//网关
// #define S_C//采集器

//***********************************************************************************
//
//UI Language define.
//
//***********************************************************************************
//#define USE_ENGLISH_MENU


//***********************************************************************************
//
//project name define.
//
//pls define one project following，otherwise use zks  default.
/****************************************************************************/
#define  ZKS_PROJECT              //ZKS 项目


#ifdef ZKS_PROJECT
#define         COMPANY_NAME                "ZKS"
#else
error project name
#endif
#ifdef BOARD_S6_1
#define         BOARD_NAME              "_S6_1"
#elif defined( BOARD_S6_2)
#define         BOARD_NAME              "_S6_2"
#elif defined( BOARD_S6_6)
#define         BOARD_NAME              "_S6_6"
#elif defined( BOARD_B2S)
#define         BOARD_NAME              "_B2S"
#elif defined( BOARD_S3)
#define         BOARD_NAME              "_S3"
#elif defined( BOARD_S3_1)
#define         BOARD_NAME              "_S3_1"
#else
error borad name
#endif

#ifdef S_A
#define 		TYPE_NAME				"_A"
#elif defined(S_G)
#define 		TYPE_NAME				"_G"
#elif defined(S_C)
#define 		TYPE_NAME				"_C"
#else
error type define
#endif

#ifdef USE_ENGLISH_MENU
#define 		MENU_NAME				"_EN"
#else
#define 		MENU_NAME				""

#endif

#define PROJECT_NAME (COMPANY_NAME""PLATFORM_NAME""BOARD_NAME""TYPE_NAME""MENU_NAME)


//***********************************************************************************
//
// MCLK define.
//
//***********************************************************************************
#define MCU_MCLK                48000000
#define XT1_CLK                 32768
#define XT2_CLK                 24000000

#define delay_ns(x)     __delay_cycles((long)MCU_MCLK*(double)x/1000000000.0)
#define delay_us(x)     __delay_cycles((long)MCU_MCLK*(double)x/1000000.0)
#define delay_ms(x)     __delay_cycles((long)MCU_MCLK*(double)x/1000.0)

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
#define MSP430F55XX_ADC12
#define ADC_INTERNAL_REFERENCE

//Flash define
#define FLASH_INTERNAL
#define FLASH_EXTERNAL
#ifdef FLASH_EXTERNAL
//#define FLASH_W25Q256FV  //大于128Mb=16MB,采用4字节地址模式。
//#define FLASH_SIZE_128M
//#define FLASH_SENSOR_DATA_32BYTE
//#define MX66L1G45G  //1G flash
#endif

//#define USE_NEW_SYS_CONFIG
#ifdef USE_NEW_SYS_CONFIG
#undef FLASH_INTERNAL
#endif

//Sensor define
#define SUPPORT_SENSOR
#ifdef SUPPORT_SENSOR

#define SUPPORT_SHT2X
//#define SUPPORT_SHT3X

#define SUPPORT_NTC

#ifdef BOARD_S6_2
#define SUPPORT_LIGHT
#define SUPPORT_GSENSOR

#if 0
#define SUPPORT_DEEPTEMP
#else
#define SUPPORT_PT100
#endif

#endif

#ifdef BOARD_S6_3
#define SUPPORT_DEEPTEMP
//#define SUPPORT_GPS
#endif

#ifdef BOARD_S6_4
#define SUPPORT_LIGHT
#define SUPPORT_GSENSOR
#endif


#ifdef SUPPORT_LIGHT
#define SUPPORT_OPT3001
#endif

#ifdef SUPPORT_GSENSOR
#define SUPPORT_LIS2DS12
#endif

//#define SUPPORT_GPS
#ifdef SUPPORT_NTC
//#define NTC_ELIWELL_10K
//#define NTC_KEMIT_10K
//#define NTC_KEMIT_100K
//#define NTC_KEMIT_PT1000
//#define NTC_KEMIT_PT1000_2ADC
#define NTC_XINXIANG_10K
//#define NTC_TIANYOU_10K
#endif

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
// #define SUPPORT_LORA
// #define SUPPORT_LORA_APC  //LORA Auto Power Control
// #define SUPPORT_LORA_CHN_FLASH  //将设备通道号保存在flash
//#define LORA_FREQ  433800000 //中心频点433.8/433MHz
//#define LORA_OOK_TEST
// #ifdef LORA_OOK_TEST
//#define  OOK_TEST_AUTHENTICATION //天线认证
// #define  OOK_TEST_ANTENNA  //天线匹配调试
// #endif
//#define SUPROT_GATE_SET


//I2C define
#define SOFT_I2C_BUS


//LCD define
#define SUPPORT_DISP_SCREEN
#ifdef SUPPORT_DISP_SCREEN

#define SUPPORT_START_LOGO

//#define EPD_GDE0213B1
#define LCD_ST7567A
#define SUPPORT_MENU

#define SUPPORT_NETGATE_DISP_NODE   //网关显示收到的节点数据
#define SUPPORT_NETGATE_BIND_NODE   //网关绑定的节点，需要收到数据后判断是否超温
#define NETGATE_BIND_NODE_MAX       2 //5529上只能支持2个，USB内存不够

#endif

//Led define
#define SUPPORT_LED

//Battery function define.
#define SUPPORT_BATTERY
#ifdef SUPPORT_BATTERY
//#define SUPPORT_ADP5062
#endif

//Network define
#define SUPPORT_NETWORK
#ifdef SUPPORT_NETWORK
#define SUPPORT_GSM
#define SUPPORT_GSM_SHORT_CONNECT
#define SUPPORT_LBS
#ifdef SUPPORT_LBS
//#define USE_QUECTEL_API_FOR_LBS
#define USE_ENGINEERING_MODE_FOR_LBS
//#define SUPPOERT_LBS_NEARBY_CELL
#endif
//#define SUPPORT_IMEI

#define PACKAGE_ITEM_COUNT_MAX 10
#define SUPPORT_ZKS_PROTOCOL

//#define SUPPORT_FLIGHT_MODE // flight mode
#define SUPPORT_REMOTE_UPGRADE

#endif //SUPPORT_NETWORK

//Bluetooth define
// #define SUPPORT_BLUETOOTH_PRINT

//Key define
//#define SUPPORT_DOUBLE_CLICK

//USB upgrade firmware define
#define SUPPORT_USB_UPGRADE
#define SUPPORT_USB_UPGRADE_APP
// #define SUPPORT_USB_UPGRADE_BSL

#define SUPPORT_ENGMODE

#define SUPPORT_ALARM_RECORD_QURERY
#define ALARM_RECORD_QURERY_MAX_ITEM 10

#define SUPPORT_DEVICED_STATE_UPLOAD

#define SUPPORT_ALARM_SWITCH_PERIOD

#ifdef S_A//一体机


#undef  SUPPORT_LORA
#undef  SUPPORT_NETGATE_DISP_NODE   //网关显示收到的节点数据
#undef  SUPPORT_NETGATE_BIND_NODE   //网关绑定的节点，需要收到数据后判断是否超温
//#undef  SUPPORT_ENGMODE //for  more flash space
#define SUPPORT_UPLOADTIME_LIMIT

#elif defined(S_G)//gateway

#undef SUPPORT_ALARM_SWITCH_PERIOD
//#undef SUPPORT_DEVICED_STATE_UPLOAD
#undef SUPPORT_FLIGHT_MODE // flight mode

#undef SUPPORT_SENSOR

#undef SUPPORT_GSENSOR
#undef SUPPORT_LIS2DS12

#undef SUPPORT_LIGHT
#undef SUPPORT_OPT3001

#undef SUPPORT_DEEPTEMP
#undef SUPPORT_PT100

#undef SUPPORT_SHT2X
#undef SUPPORT_SHT3X
#undef SUPPORT_NTC

#elif defined(S_C)//collection

#undef SUPPORT_ALARM_SWITCH_PERIOD
#undef SUPPORT_NETWORK
#undef SUPPORT_GSM
#undef SUPPORT_NETGATE_DISP_NODE   //网关显示收到的节点数据
#undef SUPPORT_NETGATE_BIND_NODE   //网关绑定的节点，需要收到数据后判断是否超温
#undef SUPPORT_FLIGHT_MODE // flight mode
#undef SUPPORT_DEVICED_STATE_UPLOAD


#else
error
#endif


/****************************************************************************/
/*
 * ZKML ZKMeiLing
 *  M-COOL Cloud Platform Management System, API v1.0
 * */

#ifdef      ZKML_PROJECT

#undef      FW_VERSION
#define     FW_VERSION          0x0066

#define SUPPORT_MCOOL_PROTOCOL
#define SUPPOERT_LBS_NEARBY_CELL
#define SUPPORT_IMEI
#endif

/****************************************************************************/
#ifdef  G7_PROJECT

#undef  FW_VERSION
#define FW_VERSION          0x0080

#undef  SUPPORT_ZKS_PROTOCOL
#define SUPPORT_G7_PROTOCOL
#define SUPPOERT_LBS_NEARBY_CELL
#define SUPPORT_IMEI
#define CRC16_LSB
#undef  CRC16_HW_MODULE

#undef SUPPORT_FLIGHT_MODE  // no flight mode
#endif
/****************************************************************************/
#ifdef SFKJ_PROJECT

#define SUPPORT_SFKJ_UI                 //顺丰UI

#define FLASH_SENSOR_DATA_32BYTE        //数据记录32BYTE

#define SUPPORT_CHARGE_CHECK

#undef  SUPPORT_ALARM_RECORD_QURERY

#endif
/****************************************************************************/
#ifdef ZKS_PROJECT

#define SUPPORT_SENSOR_TRANSMIT_TIME

#endif
/****************************************************************************/


//***********************************************************************************
// S2_2 board define
//***********************************************************************************
#ifdef BOARD_B2S
#define SUPPORT_SENSOR
#define SUPPORT_SHT2X
#define SUPPORT_DEEPTEMP_PT100

#define SUPPORT_REMOTE_UPGRADE
#define SUPPORT_USB_UPGRADE

#define  SUPPORT_USB

#define SUPPORT_CHARGE_DECT

#undef SUPPORT_NTC
#undef SUPPORT_ENGMODE
#undef SUPPORT_DISP_SCREEN
#undef SUPPORT_MENU
#undef SUPPORT_START_LOGO
#undef SUPPORT_NETGATE_DISP_NODE

//#define SUPPORT_BOARD_OLD_S2S_1
#endif

//***********************************************************************************
// S1_2/3 board define
//***********************************************************************************
#ifdef BOARD_S3
#define SUPPORT_SENSOR
#define SUPPORT_SHT2X
//#define SUPPORT_SHT3X


#define SUPPORT_RADIO_UPGRADE

#define SUPPORT_DOUBLE_PRESS

//LCD undefine
#ifdef SUPPORT_DISP_SCREEN

#undef SUPPORT_DISP_SCREEN
#undef SUPPORT_START_LOGO

#undef LCD_ST7567A
#undef SUPPORT_MENU

#undef SUPPORT_NETGATE_DISP_NODE   //网关显示收到的节点数据
#undef SUPPORT_NETGATE_BIND_NODE   //网关绑定的节点，需要收到数据后判断是否超温
//#undef NETGATE_BIND_NODE_MAX
#endif // SUPPORT_DISP_SCREEN

#undef SUPPORT_NTC
#undef SUPPORT_ALARM_RECORD_QURERY
#undef SUPPORT_ENGMODE
#undef SUPPORT_FLASH_LOG

#define SUPPORT_RSSI_CHECK
#define SUPPORT_FREQ_FIND
// #define SUPPORT_STRATEGY_SORT
#define CONCENTER_MAX_CHANNEL       100

/* old S1*/
//#define SUPPORT_BOARD_OLD_S1
#ifdef SUPPORT_BOARD_OLD_S1
#undef SUPPORT_RSSI_CHECK
#undef  BOARD_NAME
#define BOARD_NAME              "_S3_1"
#undef  PROJECT_NAME
#define PROJECT_NAME (COMPANY_NAME""PLATFORM_NAME""BOARD_NAME""TYPE_NAME""MENU_NAME)
#endif //SUPPORT_BOARD_OLD_S1

/* Z4 */
//#define SUPPORT_BOARD_Z4
#ifdef SUPPORT_BOARD_Z4

#undef SUPPORT_SHT2X
#undef SUPPORT_SHT3X
#define SUPPORT_UPLOAD_ASSET_INFO

#undef  BOARD_NAME
#define BOARD_NAME              "_Z4"
#undef  PROJECT_NAME
#define PROJECT_NAME (COMPANY_NAME""PLATFORM_NAME""BOARD_NAME""TYPE_NAME""MENU_NAME)
#endif //SUPPORT_BOARD_Z4

#undef  FW_VERSION
#ifndef SUPPORT_BOARD_OLD_S1
#define FW_VERSION              0x0053
#else
#define FW_VERSION              0x0008
#endif
#endif

//***********************************************************************************
// config board define
//***********************************************************************************
#ifdef  BOARD_CONFIG_DECEIVE

// #undef SUPPORT_SHT2X
// #undef SUPPORT_MENU
#undef  BOARD_NAME
#define BOARD_NAME              "_S3_CONFIG"
#undef  PROJECT_NAME
#define PROJECT_NAME (COMPANY_NAME""PLATFORM_NAME""BOARD_NAME""TYPE_NAME""MENU_NAME)

#endif //BOARD_CONFIG_DECEIVE



//***********************************************************************************
//
// FW version define.
//
//***********************************************************************************
#ifdef BOARD_S6_6

#undef SUPPORT_DOUBLE_PRESS

#define SUPPORT_USB
#define SUPPORT_FREQ_FIND
// #define SUPPORT_STRATEGY_SORT
#define CONCENTER_MAX_CHANNEL       100
#ifdef SUPPORT_STRATEGY_SORT
#ifdef S_G
#define SUPPORT_STORE_ID_IN_EXTFLASH
#endif //S_G
#endif  //SUPPORT_STRATEGY_SORT

#undef  FW_VERSION

#ifdef S_A

#define FW_VERSION              0x0036

#elif defined(S_G)

#define SUPPORT_UPLOAD_ASSET_INFO
#define FW_VERSION              0x0053

#elif defined(S_C)

#define FW_VERSION              0x0036
#endif //S_A

#ifdef BOARD_CONFIG_DECEIVE
#undef  SUPPORT_RSSI_CHECK
#undef  SUPPORT_FREQ_FIND
#undef  SUPPORT_STRATEGY_SORT
#endif // BOARD_CONFIG_DECEIVE


#endif // BOARD_S6_6

#ifdef BOARD_B2S
#undef  FW_VERSION
    #ifdef S_A
    #define FW_VERSION              0x0040
    #elif defined(S_G)
    #define FW_VERSION              0x0040
    #elif defined(S_C)
    #undef SUPPORT_CHARGE_DECT
    #define FW_VERSION              0x0040
    #endif
#ifdef  UPPORT_BOARD_OLD_S2S_1
#undef  SUPPORT_CHARGE_DECT
#undef  FW_VERSION
#define FW_VERSION              0x0001
#undef  BOARD_NAME
#define BOARD_NAME              "_S2S_1"
#undef  PROJECT_NAME
#define PROJECT_NAME (COMPANY_NAME""PLATFORM_NAME""BOARD_NAME""TYPE_NAME""MENU_NAME)
#endif // UPPORT_BOARD_OLD_S2S_1
#endif // BOARD_B2S


//***********************************************************************************
//
// default setting.
//
//***********************************************************************************
#define CONFIG_DECEIVE_ID_DEFAULT       0XFFFFFFFA
#define RADIO_UPGRADE_ADDRESS           0x99999999
#define RADIO_BROCAST_ADDRESS           0x00000000

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
    SEN_TYPE_ORGPOLL,
    SEN_TYPE_HLW8012,
    SEN_TYPE_IM33XX,
    SEN_TYPE_INFRARED,
    SEN_TYPE_ASSET,
    SEN_TYPE_MAX
} SENSOR_TYPE;
#define ALARM_TEMP_HIGH         0x7fff
#define ALARM_TEMP_LOW          (-0x7fff)

//***********************************************************************************
//
//Status define.
//
//***********************************************************************************
#define STATUS_LCD_ALWAYS_ON        0x0001//lcd常开
#define STATUS_GSM_TEST             0x0002//gsm测试模式
#define STATUS_KEY_LED_ON           0x0004//按键灯亮灯
#define STATUS_ALARM_OFF            0x0008//本地报警开关
#define STATUS_SENSOR_NAME_ON       0x0010//网关显示采集器名字开关
#define STATUS_DISP_BIND_ONLY       0x0020//网关只显示绑定设备的信息开关
#define STATUS_HIDE_PWOF_MENU       0x0040//hide power off menu 
#define STATUS_HIDE_SHT_SENSOR      0x0080//dont display sht20 sensor
#define STATUS_ALARM_SWITCH_ON      0x0100//alarm upload period switch on

#define STATUS_LORA_MASTER      0x0001//lora做master或slaver
#define STATUS_LORA_TEST      	0x0002//LORA 测试模式
#define STATUS_LORA_APC      	0x0004//采集器自动增益控制
#define STATUS_LORA_ALARM      	0x0008//采集器发送报警信息到网关
#define STATUS_LORA_CHANGE_FREQ 0x0010//LORA的中心频根据客户码变化
#define STATUS_1310_MASTER      0x0020//1310做master或slaver



//***********************************************************************************
//
//system event define.
//
//***********************************************************************************
// #define SYS_EVT_NONE                Event_Id_NONE
// #define SYS_EVT_KEY                 Event_Id_00
// #define SYS_EVT_DISP                Event_Id_01
// #define SYS_EVT_SENSOR              Event_Id_02
// #define SYS_EVT_ALARM               Event_Id_03
// #define SYS_EVT_LORA_SENSOR         Event_Id_04
// #define SYS_EVT_G_SENSOR            Event_Id_05
// #define SYS_EVT_OPT3001             Event_Id_06
// #define SYS_EVT_GPS_STOR            Event_Id_07
// #define SYS_EVT_GSENSOR_STOR        Event_Id_08
// #define SYS_EVT_FEED_WATCHDOG       Event_Id_09
// #define SYS_EVT_SEARCH_BLE_RECORD   Event_Id_10
// #define SYS_EVT_CONFIG_SAVE         Event_Id_11
// #define SYS_EVT_G7_ALARM_SAVE       Event_Id_12
// #define SYS_EVT_ALARM_SAVE          Event_Id_13
// #define SYS_EVT_SYS_RUN_SAVE        Event_Id_14
// #define SYS_EVT_SHUTDODN            Event_Id_15



// #define SYS_EVT_ALL                 0xffff

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
//radio error define.
//***********************************************************************************
#define         RADIO_FREQ_SWITCH_ERROR         0X0001
#define         RADIO_FREQ_SEND_ERROR           0X0002
#define         RADIO_FREQ_RECIEVE_ERROR        0X0004
#define         RADIO_FREQ_CORE_ERROR           0X0008
#define         RADIO_ABORT_ERROR               0X0010

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
    uint8_t     ChNo;
    AlarmTemp_t AlarmInfo;
} BindNode_t;

typedef struct {
    uint32_t   err_restarts;
    uint32_t   i2c_errors;
    uint32_t   lora_send_errors;        
    uint32_t   lora_list_fulls; 
    uint32_t   wtd_restarts;    
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

    //uplaod period when alarm
    uint32_t alarmuploadPeriod;
    
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
    
    //offset 256 +20    
    BindNode_t  bindnode[NETGATE_BIND_NODE_MAX];

	//total size 276+9*2 = 294

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
#include "radio_app/auto_findfreq.h"

#if defined(SUPPORT_BOARD_OLD_S1) || defined(SUPPORT_BOARD_OLD_S2S_1)
    #include "app/old_s1/old_s1_node_app.h"
#endif

#ifdef SUPPORT_DEEPTEMP_PT100
// #include "ads1247/ads1247.h"
#include "sensor/pt100.h"
#include "sensor/ads1247/ads1247.h"
#endif

#ifdef SUPPORT_FLIGHT_MODE
#include "network/flight_mode.h"
#endif
#ifdef SUPPORT_SHT3X
#include "sensor/sht3x.h"
#endif
#ifdef SUPPORT_CHARGE_DECT
#include "chargedetect/chargedect.h"
#endif

//***********************************************************************************
//
//	Global variable define
//
//***********************************************************************************
EXTERN_ATTR volatile ConfigInfo_t g_rSysConfigInfo;

EXTERN_ATTR uint16_t     g_bAlarmSensorFlag; //
EXTERN_ATTR  Alarmdata_t    g_AlarmSensor;

#ifdef SUPPORT_ALARM_SWITCH_PERIOD
EXTERN_ATTR bool g_alarmFlag;
#endif //SUPPORT_ALARM_SWITCH_PERIOD

EXTERN_ATTR uint8_t radioError;

extern uint8_t deviceMode;

#endif	/* __ZKSIOT_GENERAL_H__ */

