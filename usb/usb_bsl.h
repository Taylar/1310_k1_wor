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

// #define         UPGRADE_TIMEOUT                     (30*60)

// #define         UPGRADE_FLAG                "valid info"
// #define         PROJECT_NAME                "ZKS_CC1310_NodeGateway"

// #define         NWK_UPGRADE_PACKAGE_LENGTH          256
// #define         UPGRADE_FILE_INFO_LENGTH            128

// #define         BOOT_LOADER_ADDR                    0x1000
// typedef void (*UpgradeBL_t) (void);

extern const UpgradeBL_t UpgradeBootLoader;

// typedef enum {
//     UPGRADE_RESULT_LOADING_COMPLETE = 0,
//     UPGRADE_RESULT_NEXT_PACKAGE,
//     UPGRADE_RESULT_PACKNUM_ERR,
//     UPGRADE_RESULT_CRC_ERR,
//     UPGRADE_RESULT_ERR,
//     UPGRADE_RESULT_NEED_UPDATA,
//     UPGRADE_RESULT_NEEDNOT_UPDATA,
// } UPGRADE_RESULT_E;

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

// typedef struct
// {
//     uint8_t     validFlag[sizeof(UPGRADE_FLAG)];
//     uint8_t     waiteUpgrade;       // if program need to upgrade 0: needn't;   1: need
//     uint8_t     complete;           // test this flag when upgrade complete, and then clear it;     1:updata complete; 0: donot occur upgrade
//     uint16_t    crc;
//     uint32_t    fileLength;         // the len is the file length that include UPGRADE_FILE_INFO_LENGTH;
// }upgrade_flag_t;


extern void Usb_BSL_data_parse(uint8_t *pData, uint16_t length);
extern void bsl_ack_error(uint8_t *pData, uint16_t length);
extern void bsl_ack_return(uint8_t *pData, uint16_t length, USB_BSL_ACK_T bsl_ack);

#endif	/* __ZKSIOT_USB_BSL_H__ */

