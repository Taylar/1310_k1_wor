//***********************************************************************************
// Copyright 2018, Zksiot Development Ltd.
// Created by yuanGuo, 2018.04.25
// MCU:	MSP430F5529
// OS: TI-RTOS
// Project:
// File name: usb_bsl.h
// Description: usb bootloader routine head. // USB-IAP
//***********************************************************************************

#ifndef __ZKSIOT_USB_BSL_H__
#define __ZKSIOT_USB_BSL_H__

extern const UpgradeBL_t UpgradeBootLoader;

typedef enum _USB_BSL_RET{
    BSL_ACK_OK = 0,
    BSL_ACK_FAIL = 1,
    BSL_ACK_ERROR = 2,
    BSL_ACK_NOSUPPORT = 3
}USB_BSL_ACK_T;

typedef struct _usb_upgrade_info{
    uint32_t pack_num;
    uint32_t pack_size;
    uint32_t pack_offset;
    uint32_t fileLength;
    uint16_t crc; // used for calculate
    bool     endFlag;
}usb_upgrade_info_t;


extern void bsl_ack_error(uint8_t *pData, uint16_t length);
extern void bsl_ack_return(uint8_t *pData, uint16_t length, USB_BSL_ACK_T bsl_ack);
UPGRADE_RESULT_E Usb_bsl_UpgradeLoad_check(uint32_t fileLen);
extern void Usb_BSL_data_parse(uint8_t *pData, uint16_t length);

#endif	/* __ZKSIOT_USB_BSL_H__ */

