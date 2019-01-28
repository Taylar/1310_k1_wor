//***********************************************************************************
// Copyright 2018, Zksiot Development Ltd.
// Created by yuanGuo, 2018.06.07
// MCU:	MSP430F5529
// OS: TI-RTOS
// Project:
// File name: print_list.h
// Description: print list data.
//***********************************************************************************

#ifndef __ZKSIOT_PRINT_LIST_H__
#define __ZKSIOT_PRINT_LIST_H__

#include "../general.h"

#ifdef SUPPORT_NETGATE_BIND_NODE
#ifdef SUPPORT_BLUETOOTH_PRINT
#define SUPPORT_BLE_PRINT_LIST_MODE
#endif// SUPPORT_BLUETOOTH_PRINT
#define DEUBG_BLE_PRINT 0
#else
// 非绑定状态下 调试开关
//#define SUPPORT_BLE_PRINT_LIST_MODE
#define DEUBG_BLE_PRINT 0
#endif

#ifdef SUPPORT_BLE_PRINT_LIST_MODE
#define LSIT_PRINT_SELECT_MAX 3 // Select three times trigger once print or Echo a Menu to choice print/cancel

#pragma pack(1)
typedef struct Record_time{
    struct {
    uint8_t Year;
    uint8_t Month;
    uint8_t Date;
    } YMD;
    struct {
    uint8_t Hour;
    uint8_t Minute;
    uint8_t Second;
    } HMS;
}Record_time_t;

typedef struct Record_Header{
    uint8_t Length; // LE 32-1 or 64-1
    uint8_t RSSI;
    //uint32_t DeviceID;
    uint8_t DeviceID[4];
    uint16_t SerialNum;
    Record_time_t Record_time;
    uint16_t Voltage;
    uint8_t DataType[0];
}Record_Header_t;

typedef struct Record_SData{
    uint8_t sensor_chn; // channel
    uint8_t sensor_type;    // type
    union {
        struct {
        int16_t temp;   // SHT20 TEMP
        uint16_t humi;  // SHT20 HUMI
        }SHT20_tempHumi;
        int16_t NTC_value;   //
        uint32_t  lux;
        uint32_t tempDeep; //MAX31885 // tempdeep // otp100
        uint16_t HCHO_value; //
        uint16_t PM25_value;
        uint16_t CO2_value;
        uint8_t YJWR_pollution;
        uint8_t value[4]; // current max 4byte
    }; //variable length
}Record_SData_t;    // Sensor data
#pragma pack()

extern void Btp_printList_record_bind_node(void);
extern char PrintList_getSelectedDID_NUM(uint32_t *DIDBuffer, uint8_t *DID_Num);

#endif //

#endif	/* __ZKSIOT_PRINT_LIST_H__ */

