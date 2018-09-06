//***********************************************************************************
// Copyright 2017-2018, Zksiot Development Ltd.
// Created by yuanGuo, 2018.07.10
// MCU:	MSP430F5529
// OS: TI-RTOS
// Project:
// File name: config_id_mode_type.h
// Description: used for configure system setting with id mode.
//***********************************************************************************

#ifndef __ZKSIOT_CONFIG_ID_MODE_TYPE_H__
#define __ZKSIOT_CONFIG_ID_MODE_TYPE_H__

typedef enum _enumID_TYPE_T{
    // TYPE                     // length
    ID_SWVERSION    = 0x01,     // length 2
    ID_DEVICEID     = 0X02,     // length 4
    ID_CUSTOMID     = 0X03,     // length 2
    ID_FUNCCONFIG   = 0X04,     // length 2 // STATUS
    ID_MODULECONFIG         = 0X05,     // 2
    ID_SENSOR_CH_CONFIG     = 0X06,     // 8
    ID_ALARM_TEMP_CONFIG    = 0X07,     // 5
    ID_COLLECT_PERIOD       = 0X08,     // 4
    ID_SERVER_IP            = 0X09,     // 4
    ID_SERVER_PORT          = 0X0A,     // 2
    ID_HEARTBEAT_PERIOD     = 0X0B,     // 2
    ID_UPLOAD_PERIOD        = 0X0C,     // 4
    ID_NTP_PERIOD           = 0X0D,     // 4
    ID_RF_PA        = 0X0E,     // 1
    ID_RF_BW        = 0X0F,     // 1
    ID_RF_SF        = 0X10,     // 1
    ID_RF_STATUS    = 0X11,     // 1
    ID_BATVOL_SHUTDOWN      = 0X12,     // 2
    ID_ALARMTEMP_CONFIG     = 0X13,     // 5
    ID_BIND_GATEWAY         = 0X14,     // 4
    ID_BIND_NODE            = 0X15,     // 11
    ID_APNUSER_PWD          = 0X16,     // n ?? < 64
    ID_APNSERVER_ADDR       = 0X17,     // n ?? < 64
    ID_CALENDER_RTC_BCD     = 0X18,     // length 8 // reserver
    ID_SYSTEM_STATE         = 0X19,     // length 20
    ID_ALARM_UPLOAD_PERIOD  = 0X1A,     // 4

    ID_SERVER_ACCESS_CODE   = 0X1B,     // n
    ID_SERVER_USER_NAME     = 0X1C,     // N
    ID_SERVER_USER_PWD      = 0X1D,     // N
    ID_SENSOR_CHN_ADJUST    = 0X1E,     // 5    // sensor channel adjust cmd

	ID_IDMODE_SET_COMPLTER  = 0xFF,     //
}enumID_TYPE_T;

#define ID_MODE_START_ID ID_SWVERSION
#define ID_MODE_END_ID  ID_ALARM_UPLOAD_PERIOD

// ID-mode  length define
// TYPE_LEN                 // length   // TYPE     // comment
#define ID_SWVERSION_LEN    2           // = 0x01, // length 2
#define ID_DEVICEID_LEN     4           // = 0X02, // length 4
#define ID_CUSTOMID_LEN     2           // = 0X03, // length 2
#define ID_FUNCCONFIG_LEN   2           // = 0X04, // length 2
#define ID_MODULECONFIG_LEN         2   // = 0X05, // 2
#define ID_SENSOR_CH_CONFIG_LEN     8   // = 0X06, // 8
#define ID_ALARM_TEMP_CONFIG_LEN    9   // = 0X07, // 8
#define ID_COLLECT_PERIOD_LEN       4   // = 0X08, // 4
#define ID_SERVER_IP_LEN            4   // = 0X09, // 4
#define ID_SERVER_PORT_LEN          2   // = 0X0A, // 2
#define ID_HEARTBEAT_PERIOD_LEN     2   // = 0X0B, // 2
#define ID_UPLOAD_PERIOD_LEN        4   // = 0X0C, // 4
#define ID_NTP_PERIOD_LEN           4   // = 0X0D, // 4
#define ID_RF_PA_LEN        1           // = 0X0E, // 1
#define ID_RF_BW_LEN        1           // = 0X0F, // 1
#define ID_RF_SF_LEN        1           // = 0X10, // 1
#define ID_RF_STATUS_LEN    1           // = 0X11, // 1
#define ID_BATVOL_SHUTDOWN_LEN      2   // = 0X12, // 2
#define ID_ALARM_WARING_CONFIG_LEN  9   // = 0X13, // 9
#define ID_BIND_GATEWAY_LEN         4   // = 0X14, // 4
#define ID_BIND_NODE_LEN            11  // = 0X15, // 10
#define ID_APNUSER_PWD_LEN_MAX      64  // = 0X16, // n ?? < 64
#define ID_APNSERVER_ADDR_LEN_MAX   64  // = 0X17, // n ?? < 64
#define ID_CALENDER_RTC_BCD_LEN      8  // = 0X18, // length 8 reserver
#define ID_SYSTEM_STATE_LEN         20  // = 0X19  // length 20
#define ID_ALARM_UPLOAD_PERIOD_LEN          4

#define ID_SERVER_ACCESS_CODE_LEN_MAX   20      // n
#define ID_SERVER_USER_NAME_LEN_MAX     10      // N
#define ID_SERVER_USER_PWD_LEN_MAX      10      // N
#define ID_SENSOR_CHN_ADJUST_LEN_MAX    5       // 5

// ID-mode length define end

#endif	/* __ZKSIOT_CONFIG_ID_MODE_TYPE_H__ */
