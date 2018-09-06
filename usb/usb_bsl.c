
//***********************************************************************************
// Copyright 2017, Zksiot Development Ltd.
// Created by yuanGuo, 2018.04.25
// MCU:	MSP430F5529
// OS: TI-RTOS
// Project:
// File name: usb_bsl.c
// Description: usb bootloader routine.
//***********************************************************************************
#include "../general.h"
#define CODE_DATA_LENGTH            64 // copy from upgrade.c
#ifdef SUPPORT_USB
// extern function
extern uint16_t Usb_group_package(USB_TX_MSG_ID msgId, uint8_t *pPacket, uint16_t dataLen);

#define USB_TXRX_TIMEROUT           (100L * 1000 / Clock_tickPeriod)

// global  variable
static usb_upgrade_info_t usb_upgrade_info = {0};  // initialize zero



/*---------------------- Function declaration -----------------*/
static void USB_BSL_jump_bsl(void);
static int BSL_data_parse(uint8_t *pData, uint16_t length);
static int BSL_Nodata_parse(uint8_t *pData, uint16_t length);

#ifdef BOARD_CONFIG_DECEIVE
static uint32_t Is_direct_upgrade(uint16_t crc);
#endif

void bsl_ack_upgrade_success(uint8_t *pData)
{
    uint16_t length;
    length = Usb_group_package(AC_Send_Upgrade_Success, pData, 0);
    InterfaceSend(pData, length);
}



void bsl_ack_return(uint8_t *pData, uint16_t length, USB_BSL_ACK_T bsl_ack)
{
    pData[0] = bsl_ack; // 2
    length = Usb_group_package(AC_Ack, pData, 1);
    InterfaceSend(pData, length);
    return ;
}

//***********************************************************************************
//
// USB BSL received data parse. For new protocl.
//
//***********************************************************************************
// USB 0x0A 指令包分析
void Usb_BSL_data_parse(uint8_t *pData, uint16_t length)
{
    int ret;

    // if USB-IAP execute, OTA-IAP disable
    //    UpgradeCancel();

    if(NULL == pData || 0 == length){
        // do nothing
        return;
    }
    //length = len(2)cmd(1)data(n)
   // 数据采用大端
    uint16_t data_len = pData[0] << 8 | pData[1];
    const uint8_t Usb_cmd = pData[2];

    // len(2)cmd(1)data(n) // len = n+1
    if(data_len <= 1){// 空消息处理
        BSL_Nodata_parse(pData, length);
        return ;
    }
    // usb cmd check
    if(Usb_cmd != EV_Usb_Upgrade){
        goto err_end;
    }
    //length = len(2)cmd(1)data(n)[pack_len(2)offset(4)pack_data(pack_len)]
    if(length < 9){
        goto err_end;
    }
    // remove len(2)cmd(1), lengh = USB data(n)
    // 去掉消息包头 // 消息非空处理
    ret = BSL_data_parse(pData +3, length-3);
    if(0 != ret){
        goto err_end;
    }
    bsl_ack_return(pData, length, BSL_ACK_OK);
    return;

err_end:
#ifdef BOARD_CONFIG_DECEIVE
    if (ret == -2) {
        return;
    }
#endif
    bsl_ack_error(pData, length);
    return;
}

// Jump to boot
#ifndef BOARD_CONFIG_DECEIVE
static void USB_BSL_jump_bsl(void)
{
#ifdef SUPPORT_USB_UPGRADE
    SysCtrlSystemReset();
#endif
}
#endif

// 非空消息 分析
// pack_len(2)offset(4)pack_data(pack_len)
// ret -1:ERR   0:OK
static int BSL_data_parse(uint8_t *pData, uint16_t length)
{
    uint16_t pack_len = (pData[0] << 8) | pData[1];
    uint32_t data_offset_addr = ((uint32_t)(pData[2]) << 24) | ((uint32_t)(pData[3]) << 16) | ((uint32_t)(pData[4]) << 8) | (uint32_t)pData[5];
    const uint8_t pack_head_len = 6; //

    // pack length check
    if(0xfffff == pack_len)
        return -1;
    // MAX upgrade firmware size
    if(data_offset_addr > (uint32_t)(148*1024LL))
        return -1;

    // data length check
    if(length - pack_head_len != pack_len)
        return -1;

    if(0 == data_offset_addr){   // first pack
        usb_upgrade_info.pack_num = 0;
        usb_upgrade_info.pack_offset = data_offset_addr;
        usb_upgrade_info.pack_size = pack_len;
        usb_upgrade_info.fileLength = pack_len;
        usb_upgrade_info.endFlag    = false;

#ifdef BOARD_CONFIG_DECEIVE
        uint16_t crc = (uint16_t)(pData[pack_head_len] | ((pData[pack_head_len + 1]) << 8));
        uint32_t fileLen = Is_direct_upgrade(crc);
        if (fileLen != 0xffffffff) {
            bsl_ack_upgrade_success(pData);
            usb_upgrade_info.endFlag = true;
            RadioUpgrade_start(fileLen, GetRadioDstAddr());
            return -2;
        }
#endif
    } else {
        usb_upgrade_info.pack_num++;
        usb_upgrade_info.pack_offset = data_offset_addr;
        usb_upgrade_info.pack_size = pack_len;
        usb_upgrade_info.fileLength = data_offset_addr + pack_len;
        //usb_upgrade_info.fileLength += pack_len;
    }

    // truncate data
    Flash_store_upgrade_data( data_offset_addr, pData + pack_head_len, length - pack_head_len);

    #define ZKS_BSL_VERIFY_DATA 0
#if ZKS_BSL_VERIFY_DATA
    uint8_t verify_dbuff[256];
    Flash_load_upgrade_data( data_offset_addr, verify_dbuff, length - pack_head_len);
    if(memcmp(pData + pack_head_len, verify_dbuff, length - pack_head_len) != 0){
        //Sys_buzzer_enable();
        return -1;
    }
#endif

    return 0;
}

// 空消息 分析
// ret -1:ERR   0:OK
static int BSL_Nodata_parse(uint8_t *pData, uint16_t length)
{
    UPGRADE_RESULT_E result;
    // CRC check // MAKE BSL flag if OK
    result = Usb_bsl_UpgradeLoad_check(usb_upgrade_info.fileLength);

#ifdef BOARD_CONFIG_DECEIVE
    if(UPGRADE_RESULT_LOADING_COMPLETE == result) {
        usb_upgrade_info.endFlag = true;
        RadioUpgrade_start(usb_upgrade_info.fileLength, GetRadioDstAddr());
    }
    return 0;
#else

    if(UPGRADE_RESULT_LOADING_COMPLETE == result){
        bsl_ack_return(pData, length, BSL_ACK_OK);
        USB_BSL_jump_bsl();
        return 0; // will not execute here
    }else{
        // Clear
        usb_upgrade_info.pack_num = 0;
        usb_upgrade_info.fileLength = 0;

        bsl_ack_return(pData, length, BSL_ACK_FAIL);
        return -1;
    }
#endif
}

#ifdef BOARD_CONFIG_DECEIVE
// brief: Is direct wireless upgrade
// return: File len
// note: return 0xffffffff Indicates that you cannot upgrade directly
static uint32_t Is_direct_upgrade(uint16_t crc)
{
    upgrade_flag_t upgradeFlag;
    uint8_t buff[2];
    uint16_t fileCrc;

    memset((char*)&upgradeFlag, 0x0, sizeof(upgrade_flag_t));
    // Loading upgrade information
    Flash_load_upgrade_info((uint8_t*)&upgradeFlag, sizeof(upgrade_flag_t));

    Flash_load_upgrade_data(0, buff, sizeof(buff));
    fileCrc = ((uint16_t)buff[1] << 8) + buff[0];

    if (upgradeFlag.fileLength >= 0xffffffff) {
          return 0xffffffff;
     }

     if (upgradeFlag.crc >= 0xffff) {
          return 0xffffffff;
     }

    if (crc == upgradeFlag.crc && crc == fileCrc) {
        return upgradeFlag.fileLength;
    }

    return 0xffffffff;
}
#endif

#endif //SUPPORT_USB

void bsl_ack_error(uint8_t *pData, uint16_t length)
{
    pData[0] = BSL_ACK_ERROR; // 2
    length = Usb_group_package(AC_Ack, pData, 1);
    InterfaceSend(pData, length);
    return;
}



UPGRADE_RESULT_E Usb_bsl_UpgradeLoad_check(uint32_t fileLen)
{
    uint16_t calCrc, fileCrc;
    uint8_t buff[64];
    uint32_t addr, readLen;
    upgrade_flag_t upgradeFlag;

    // check the crc
    addr    = 0; // offset external flash
    readLen = fileLen;//usb_upgrade_info.fileLength;  // 124K + 128
    Flash_load_upgrade_data(addr, buff, 64);
    /*usb_upgrade_info.crc*/ fileCrc = ((uint16_t)buff[1] << 8) + buff[0];
    readLen = readLen - 128;
    addr = 128;

    SetContinueCRC16();
    while(readLen)
    {
        if(readLen >= CODE_DATA_LENGTH)
        {
            WdtClear();
            Flash_load_upgrade_data(addr, buff, CODE_DATA_LENGTH);
            addr += CODE_DATA_LENGTH;
            calCrc = ContinueCRC16(buff, CODE_DATA_LENGTH);
            readLen = readLen - CODE_DATA_LENGTH;
        }
        else
        {
            Flash_load_upgrade_data(addr, buff, readLen);
            calCrc = ContinueCRC16(buff, readLen);
            readLen = 0;
        }
    }

    if(fileCrc != calCrc)
    {
        //upgradeInfo.upgradeStep = UPGRADE_START;
        return UPGRADE_RESULT_CRC_ERR;
    }

    memset(&upgradeFlag, 0xff, sizeof(upgrade_flag_t));
    strcpy((char*)upgradeFlag.validFlag, UPGRADE_FLAG);

#ifdef BOARD_CONFIG_DECEIVE
    upgradeFlag.waiteUpgrade = 0x00;
    upgradeFlag.complete     = 0x00;
#else
    upgradeFlag.waiteUpgrade = 0x01;
    upgradeFlag.complete     = 0xff;
#endif
    upgradeFlag.crc          = calCrc;//usb_upgrade_info.crc;
    upgradeFlag.fileLength   = fileLen;

    Flash_store_upgrade_info((uint8_t*)&upgradeFlag,sizeof(upgrade_flag_t));

    memset(&upgradeFlag, 0, sizeof(upgrade_flag_t));
    Flash_load_upgrade_info((uint8_t*)&upgradeFlag,sizeof(upgrade_flag_t));

    return UPGRADE_RESULT_LOADING_COMPLETE;
}


