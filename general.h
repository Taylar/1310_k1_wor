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


// #define ZKS_S6_6_G
// #define ZKS_S6_6_CONFIG
 #define ZKS_S3_WOR
//#define ZKS_S6_6_WOR_G

//***********************************************************************************
//
// HW version define.
//
//***********************************************************************************
#if defined(ZKS_S6_6_G) || defined(ZKS_S6_6_CONFIG) || defined(ZKS_S6_6_WOR_G)

#define BOARD_S6_6
#endif

#if defined(ZKS_S3_WOR)
#define BOARD_S3
#endif



// #define FACTOR_RADIO_TEST
// #define FACTOR_RADIO_32K_TEST

//***********************************************************************************
//
// FW version define.
//
//***********************************************************************************
#define FW_VERSION              0x0001

//***********************************************************************************
//
// protocal version define.
//
//***********************************************************************************
#define PROTOCAL_VERSION              0x0001

//***********************************************************************************
//
//Device type define.
//
//***********************************************************************************

#if defined(ZKS_S6_6_G) || defined(ZKS_S6_6_CONFIG) || defined(ZKS_S6_6_WOR_G)
#define S_G //缂傚啯鍨甸崣锟�
#endif

#if defined(BOARD_S3)
#define S_C //
#endif


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
//pls define one project following use zks  default.
/****************************************************************************/

#define  ZKS_PROJECT              //ZKS

#ifdef ZKS_PROJECT
#define         COMPANY_NAME                "ZKS"
#elif defined( HAIER_PROJECT)
#define         COMPANY_NAME                "HAIER"
#else
error project name
#endif


#ifdef BOARD_S6_6
#define         BOARD_NAME              "_S6_6"
#elif defined( BOARD_S3)
#define         BOARD_NAME              "_S3"
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
//#define FLASH_W25Q256FV  //
//#define FLASH_SIZE_128M
//#define FLASH_SENSOR_DATA_32BYTE
//#define MX66L1G45G  //1G flash
#endif

//#define USE_NEW_SYS_CONFIG
#ifdef USE_NEW_SYS_CONFIG
#undef FLASH_INTERNAL
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


//I2C define
#define SOFT_I2C_BUS


//LCD define
#define SUPPORT_DISP_SCREEN
#ifdef SUPPORT_DISP_SCREEN

#define SUPPORT_START_LOGO

//#define EPD_GDE0213B1

#define LCD_ST7567A
// #define OLED_LX12864K1
//#define SUPPORT_MENU

#define SUPPORT_NETGATE_DISP_NODE   //
#define SUPPORT_NETGATE_BIND_NODE   //
#define NETGATE_BIND_NODE_MAX       2 //
#endif

//Led define
//#define SUPPORT_LED

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
// #define SUPPOERT_LBS_NEARBY_CELL
#endif
//#define SUPPORT_IMEI

#define PACKAGE_ITEM_COUNT_MAX 10
#define SUPPORT_ZKS_PROTOCOL

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

// #define SUPPORT_ALARM_RECORD_QURERY
#define ALARM_RECORD_QURERY_MAX_ITEM 10

#define SUPPORT_DEVICED_STATE_UPLOAD


#ifdef S_A//娑擄拷娴ｆ挻婧�


#undef  SUPPORT_LORA
#undef  SUPPORT_NETGATE_DISP_NODE   //
#undef  SUPPORT_NETGATE_BIND_NODE   //
//#undef  SUPPORT_ENGMODE //for  more flash space
// #define SUPPORT_UPLOADTIME_LIMIT

#elif defined(S_G)//gateway

//#undef SUPPORT_DEVICED_STATE_UPLOAD

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

#define SUPPORT_NETGATE_BIND_NODE
#elif defined(S_C)//collection

#undef SUPPORT_NETWORK
#undef SUPPORT_GSM
#undef SUPPORT_NETGATE_DISP_NODE   //
#undef SUPPORT_NETGATE_BIND_NODE   //
#undef SUPPORT_DEVICED_STATE_UPLOAD


#else
error
#endif


/****************************************************************************/
#ifdef ZKS_PROJECT

#define SUPPORT_SENSOR_TRANSMIT_TIME

#endif
/****************************************************************************/

//***********************************************************************************
// S1_2/3 board define
//***********************************************************************************
#ifdef BOARD_S3


#define SUPPORT_RADIO_UPGRADE
#define SUPPORT_DOUBLE_PRESS
#define SUPPORT_USB

//LCD undefine
#ifdef SUPPORT_DISP_SCREEN

#undef SUPPORT_DISP_SCREEN
#undef SUPPORT_START_LOGO

#undef LCD_ST7567A
#undef SUPPORT_MENU
#undef SUPPORT_DEVICED_STATE_UPLOAD

#undef SUPPORT_NETGATE_DISP_NODE   //缂冩垵鍙ч弰鍓с仛閺�璺哄煂閻ㄥ嫯濡悙瑙勬殶閹癸拷
#undef SUPPORT_NETGATE_BIND_NODE   //缂冩垵鍙х紒鎴濈暰閻ㄥ嫯濡悙鐧哥礉闂囷拷鐟曚焦鏁归崚鐗堟殶閹诡喖鎮楅崚銈嗘焽閺勵垰鎯佺搾鍛刊
//#undef NETGATE_BIND_NODE_MAX
#endif // SUPPORT_DISP_SCREEN

#undef SUPPORT_NTC
#undef SUPPORT_ALARM_RECORD_QURERY
#undef SUPPORT_ENGMODE
// #undef SUPPORT_FLASH_LOG

#define SUPPORT_RARIO_SPEED_SET
#define SUPPORT_RSSI_CHECK
#define CONCENTER_MAX_CHANNEL       100

#ifdef  SUPPORT_SHT2X
#define SENSOR_TYPE_NAME                    "_SHT20"
#else
#define SENSOR_TYPE_NAME                    "_SHT30"
#endif  // SUPPORT_SHT2X

#undef  PROJECT_NAME
#define PROJECT_NAME (COMPANY_NAME""PLATFORM_NAME""BOARD_NAME""TYPE_NAME""MENU_NAME""SENSOR_TYPE_NAME)





#endif // BOARD_S3








//***********************************************************************************
//
// BOARD_S6_6 define.
//
//***********************************************************************************
#ifdef BOARD_S6_6

#undef SUPPORT_DOUBLE_PRESS
#define FLASH_SENSOR_DATA_32BYTE
#define SUPPORT_CHARGE_DECT
#define SUPPORT_CHARGE_DECT_ALARM
#define SUPPORT_USB
#define SUPPORT_RARIO_SPEED_SET

#define CONCENTER_MAX_CHANNEL       100


// #define SUPPORT_TCP_MULTIL_LINK
// #define SUPPORT_NETWORK_SYC_RTC

#ifdef ZKS_S6_6_G
#define SUPPORT_UPLOAD_ASSET_INFO
#endif // ZKS_S6_6_G








#ifdef ZKS_S6_6_CONFIG
#undef  SUPPORT_RSSI_CHECK
#endif // ZKS_S6_6_CONFIG

#ifdef SUPPORT_CHARGE_DECT
#define         CHARGE_FUNC                 "_CHR"
#else
#define         CHARGE_FUNC                 ""
#endif //  SUPPORT_CHARGE_DECT

#ifdef SUPPORT_BLUETOOTH_PRINT
#define         BT_FUNC                 "_BT"
#else
#define         BT_FUNC                 ""
#endif //  SUPPORT_BLUETOOTH_PRINT

#ifdef ZKS_S6_6_WOR_G
#define         WOR_FUNC                 "_WOR"
#else
#define         WOR_FUNC                 ""
#endif //ZKS_S6_6_WOR_G

#undef  PROJECT_NAME
#define PROJECT_NAME (COMPANY_NAME""PLATFORM_NAME""BOARD_NAME""TYPE_NAME""MENU_NAME""WOR_FUNC""CHARGE_FUNC""BT_FUNC)
#endif // BOARD_S6_6






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

#define UPLOAD_PERIOD_DEFAULT           1

#define COLLECT_PERIOD_DEFAULT          1

#define HEARTBEAT_PERIOD_DEFAULT        600

#define NTP_DEFAULT                     (60*60)

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
#define ALARM_TEMP_HIGH         0x7fff
#define ALARM_TEMP_LOW          (-0x7fff)

//***********************************************************************************
//
//Status define.
//
//***********************************************************************************
#define STATUS_LCD_ALWAYS_ON        0x0001//lcd鐢绱�
#define STATUS_GSM_TEST             0x0002//gsm濞村鐦Ο鈥崇础
#define STATUS_KEY_LED_ON           0x0004//閹稿鏁悘顖欏瘨閻忥拷
#define STATUS_ALARM_OFF            0x0008//閺堫剙婀撮幎銉劅瀵拷閸忥拷
#define STATUS_SENSOR_NAME_ON       0x0010//缂冩垵鍙ч弰鍓с仛闁插洭娉﹂崳銊ユ倳鐎涙绱戦崗锟�
#define STATUS_DISP_BIND_ONLY       0x0020//缂冩垵鍙ч崣顏呮▔缁�铏圭拨鐎规俺顔曟径鍥╂畱娣団剝浼呭锟介崗锟�
#define STATUS_HIDE_PWOF_MENU       0x0040//hide power off menu 
#define STATUS_HIDE_SHT_SENSOR      0x0080//dont display sht20 sensor
#define STATUS_ALARM_SWITCH_ON      0x0100//alarm upload period switch on
#define STATUS_CHAGE_ALARM_SWITCH_ON    0x0200//charge check alarm switch on
//0x0400 0x0800 瀹歌尙绮＄悮顐ゆ暏娴滐拷
#define STATUS_TX_ONLY_GATE_ON       0x1000
#define STATUS_ALARM_GATE_ON         0x2000
#define STATUS_POWERON_RESET_DATA    0x4000

#define STATUS_LORA_MASTER      0x0001//lora閸嬫aster閹存潩laver
#define STATUS_LORA_TEST      	0x0002//LORA 濞村鐦Ο鈥崇础
#define STATUS_LORA_APC      	0x0004//闁插洭娉﹂崳銊ㄥ殰閸斻劌顤冮惄濠冨付閸掞拷
#define STATUS_LORA_ALARM      	0x0008//闁插洭娉﹂崳銊ュ絺闁焦濮ょ拃锔夸繆閹垰鍩岀純鎴濆彠
#define STATUS_LORA_CHANGE_FREQ 0x0010//LORA閻ㄥ嫪鑵戣箛鍐暥閺嶈宓佺�广垺鍩涢惍浣稿綁閸栵拷
#define STATUS_1310_MASTER      0x0020//1310閸嬫aster閹存潩laver


#define ALARM_RX_EXTERNAL_ALARM      0x100  //閹恒儲鏁归崚鏉款樆闁劍濮ょ拃锟�
#define ALARM_CHARGE_DECT_ALARM      0x200 //閺傤厾鏁搁幎銉劅
#define ALARM_NODE_LOSE_ALARM       0x400 // node lose alarm
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
#define         DEVICES_CONFIG_MODE            3    // the master in the config mode, waiting for rec radio data
#define         DEVICES_SLEEP_MODE             4	// in the power on mode and the lcd shutdown
#define         DEVICES_TEST_MODE              6	// in the eng mode



//***********************************************************************************
//radio error define.
//***********************************************************************************
#define 		STATUS_POWERON          		0x0001
#define         RADIO_FREQ_SEND_ERROR           0X0002
#define         RADIO_FREQ_RECIEVE_ERROR        0X0004
#define         RADIO_FREQ_CORE_ERROR           0X0008
#define         RADIO_ABORT_ERROR               0X0010
#define         RADIO_FREQ_SWITCH_ERROR         0X0020

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

#ifdef SUPPORT_NETGATE_BIND_NODE
    //offset 256 +20    
    BindNode_t  bindnode[NETGATE_BIND_NODE_MAX];
#endif

    //total size 276+9*2 = 294
    int8_t deepTempAdjust; // 0.1C

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
#include "electricshock/electricshock.h"
#include "electricshock/sound_driver.h"
#include "display/display.h"
#include "display/menu.h"
#include "display/menu_k1.h"
#include "battery/battery.h"
#include "easylink/EasyLink.h"
#include "radio_app/radio_app.h"
#include "radio_app/node_strategy.h"

#ifdef SUPPORT_BLUETOOTH_PRINT
#include "bluetooth/bluetooth_print.h"
#endif // SUPPORT_BLUETOOTH_PRINT

#include "network/upgrade.h"
#include "interface_app/interface.h"
#include "usb/usb_proc.h"
#include "app/concenterApp.h"
#include "app/radio_protocal.h"
#include "app/nodeApp.h"
#include "app/concenterApp.h"
#include "app/systemApp.h"
#include "app/s1_app.h"
#include "app/s6_app.h"
#include "engmode/engmode.h"
#include "usb/usb_bsl.h"

#include "app/radio_protocal.h"
//***********************************************************************************
//
//	Global variable define
//
//***********************************************************************************
EXTERN_ATTR volatile ConfigInfo_t g_rSysConfigInfo;

EXTERN_ATTR uint16_t     g_bAlarmSensorFlag; //


EXTERN_ATTR uint8_t radioError;
EXTERN_ATTR bool g_bNeedUploadRecord;
EXTERN_ATTR uint8_t deviceModeTemp;

#ifdef  SUPPORT_CHARGE_DECT_ALARM
EXTERN_ATTR uint8_t g_ChagerAlarmCnt;
extern void Sys_chagre_alarm_timer_isr(void);
#endif

#ifdef SUPPORT_SOFT_CHARGE_DECT
EXTERN_ATTR uint32_t minBat, maxBat;
EXTERN_ATTR uint8_t  chargeState;
#endif //SUPPORT_SOFT_CHARGE_DECT

EXTERN_ATTR uint8_t LinkNum;

EXTERN_ATTR uint8_t assetInfo[24];
EXTERN_ATTR uint8_t assetInfoValid;

EXTERN_ATTR uint8_t openBoxOccur;

EXTERN_ATTR uint8_t gatewayConfigTime;

EXTERN_ATTR uint8_t brocastTimes;

EXTERN_ATTR uint8_t destroyEleShock;

extern uint8_t deviceMode;

#endif	/* __ZKSIOT_GENERAL_H__ */

