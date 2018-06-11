//***********************************************************************************
// Copyright 2017, Zksiot Development Ltd.
// Created by Linjie, 2017.08.08
// MCU:	MSP430F5529
// OS: TI-RTOS
// Project:
// File name: usb_proc.h
// Description: usb process routine head.
//***********************************************************************************

#ifndef __ZKSIOT_USB_PROC_H__
#define __ZKSIOT_USB_PROC_H__

typedef enum {
// RX Messages.
	EV_Test         = 0x01,
	EV_Get_Config   = 0x02,
	EV_Set_Config,
	EV_Get_APN      = 0x04,
	EV_Set_APN,
	EV_Get_Calendar = 0x06,
	EV_Set_Calendar,
	EV_Get_Bluetooth_Name = 0x08,
	EV_Set_Bluetooth_Name,
	EV_Usb_Upgrade,	
	EV_Get_History_Data   = 0x0B,  //根据编号获取历史数据
    EV_Reset_Data         = 0x0C,  //清除所有数据
    
    EV_Get_DevicePara    = 0x0F, //查询设备参数
    EV_Set_DevicePara    = 0x10, //设置设备参数
    EV_Get_Device_Data   = 0x11, //获取指定时间段数据
    EV_Get_Unupload_Data = 0x12,//获取未上传数据
    EV_Get_Record_Data   = 0x13,//获取开始记录的数据
    EV_Verify_Code       = 0x14,//验证设备密码

    EV_Upgrade_BSL = 0x20,  // Upgrade BSL
	
	EV_Get_ReadFlash    = 0x71,
} USB_RX_MSG_ID;

typedef enum {
// TX Messages.
	AC_Ack              = 0x80,
	AC_Send_Config      = 0x82,
	AC_Send_APN         = 0x84,
	AC_Send_Calendar    = 0x86,
	AC_Send_Bluetooth_Name = 0x88,
	
	AC_Send_History_Data  = 0x8B,// 根据编号获取历史数据,0.0.48版本之前误写成了8A,需要工具兼容
	
    AC_Send_DevicePara    = 0x8F, //返回设备参数

    AC_Send_Device_Data  = 0x91,//获取指定时间段数据
    AC_Send_Unupload_Data = 0x92,//获取未上传数据
    AC_Send_Record_Data   = 0x93,//获取开始记录的数据

    
    EV_Send_ReadFlash = 0xF1,
} USB_TX_MSG_ID;

typedef struct {
    // Usb interrup flag.
    uint8_t interruptFlag;
    // Usb CDC data receivedflag.
    uint8_t cdcDataReceivedFlag;
} UsbEvent_t;

#define USB_BUFF_LENGTH           (256+44)
// Old protocol defined by DouQian. 
#define USB_PACKAGE_HEAD          0xcb
#define USB_PACKAGE_TAIL          0xbc

// New protocol defined by LinJie.
#define USB_PACKAGE_HEADL         0xff
#define USB_PACKAGE_HEADH         0x5a
#define USB_PACKAGE_TAILL         '\r'
#define USB_PACKAGE_TAILH         '\n'

#define USB_DECEIVE_CODE_LENGTH		(8)

#define USB_DECEIVE_CODE		"88888888"

#ifdef  G7_PROJECT
#undef	USB_DECEIVE_CODE
#define USB_DECEIVE_CODE		"G7180504"
#endif

void Usb_data_parse(uint8_t *pData, uint16_t length);


#endif	/* __ZKSIOT_USB_PROC_H__ */
