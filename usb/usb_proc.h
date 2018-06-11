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
	EV_Get_History_Data   = 0x0B,  //���ݱ�Ż�ȡ��ʷ����
    EV_Reset_Data         = 0x0C,  //�����������
    
    EV_Get_DevicePara    = 0x0F, //��ѯ�豸����
    EV_Set_DevicePara    = 0x10, //�����豸����
    EV_Get_Device_Data   = 0x11, //��ȡָ��ʱ�������
    EV_Get_Unupload_Data = 0x12,//��ȡδ�ϴ�����
    EV_Get_Record_Data   = 0x13,//��ȡ��ʼ��¼������
    EV_Verify_Code       = 0x14,//��֤�豸����

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
	
	AC_Send_History_Data  = 0x8B,// ���ݱ�Ż�ȡ��ʷ����,0.0.48�汾֮ǰ��д����8A,��Ҫ���߼���
	
    AC_Send_DevicePara    = 0x8F, //�����豸����

    AC_Send_Device_Data  = 0x91,//��ȡָ��ʱ�������
    AC_Send_Unupload_Data = 0x92,//��ȡδ�ϴ�����
    AC_Send_Record_Data   = 0x93,//��ȡ��ʼ��¼������

    
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
