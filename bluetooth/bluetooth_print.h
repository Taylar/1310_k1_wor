//***********************************************************************************
// Copyright 2017, Zksiot Development Ltd.
// Created by Linjie, 2017.09.13
// MCU:	MSP430F5529
// OS: TI-RTOS
// Project:
// File name: bluetooth_print.h
// Description: bluetooth print process routine head.
//***********************************************************************************

#ifndef __ZKSIOT_BLUETOOTH_PRINT_H__
#define __ZKSIOT_BLUETOOTH_PRINT_H__

#ifdef SUPPORT_BLUETOOTH_PRINT

typedef enum {
    BTP_CMD_COMMON = 0,
    BTP_CMD_GET_DEVICE_NAME,
    BTP_CMD_CHANGE_MODE,
    BTP_CMD_SCAN_DEVICE_NAME,
    BTP_CMD_STOP_SCAN,
    BTP_CMD_SPP_CONNECT,
    BTP_CMD_MAX
} BTP_CMD_TYPE;

//Get device name
#define BTPCMD_GET_DEVICE_NAME      "AT+NAME\r\n"
#define BTPCMD_SET_DEVICE_NAME      "AT+NAME="
#define BTPCMD_CHANGE_MODE          "AT+SPPMODE=0\r\n"
#define BTPCMD_SCAN_DEVICE_NAME     "AT+SCAN=1\r\n"
#define BTPCMD_STOP_SCAN	        "AT+SCAN=0\r\n"
#define BTPCMD_SPP_CONNECT          "AT+SPPCONN="

#define DEVICE_NAME_SIZE            32

typedef enum {
    BTP_ACK_WAIT = 0,
    BTP_ACK_OK,
    BTP_ACK_ERROR,
    BTP_ACK_MAX
} BTP_ACK_STATE;

typedef enum {
    PRINT_PERIOD_IS_NULL = 0,
    ONE_MINUTE_PRINT_PERIOD,
    TWO_MINUTE_PRINT_PERIOD,
    FIVE_MINUTE_PRINT_PERIOD,
}BTP_PRINT_STRATEGY;
extern void Btp_init(void);
extern void Btp_poweron(void);
extern void Btp_poweroff(void);
extern ErrorStatus Btp_get_device_name(uint8_t *pNameBuf);
extern ErrorStatus Btp_set_device_name(uint8_t *pNameBuf);
extern ErrorStatus Btp_scan_device_name(void);
extern uint8_t Btp_is_connect(void);
extern void Btp_print_record(void);
extern uint32_t Btp_GetPrintNum();

#endif  /* SUPPORT_BLUETOOTH_PRINT */

#endif	/* __ZKSIOT_BLUETOOTH_PRINT_H__ */

