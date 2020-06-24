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
	EV_Get_History_Data   = 0x0B,  //鏍规嵁缂栧彿鑾峰彇鍘嗗彶鏁版嵁
    EV_Reset_Data         = 0x0C,  //娓呴櫎鎵�鏈夋暟鎹�
    
    EV_Get_DevicePara    = 0x0F, //鏌ヨ璁惧鍙傛暟
    EV_Set_DevicePara    = 0x10, //璁剧疆璁惧鍙傛暟
    EV_Get_Device_Data   = 0x11, //鑾峰彇鎸囧畾鏃堕棿娈垫暟鎹�
    EV_Get_Unupload_Data = 0x12,//鑾峰彇鏈笂浼犳暟鎹�
    EV_Get_Record_Data   = 0x13,//鑾峰彇寮�濮嬭褰曠殑鏁版嵁
    EV_Verify_Code       = 0x14,//楠岃瘉璁惧瀵嗙爜
	EV_Get_SIM_CCID		 = 0x15,//get sim ccid
	EV_Get_Cur_Vol		 = 0x16,//get current volatge
	EV_Get_SensorData    = 0x18,//get sendor data


    EV_Upgrade_BSL      = 0x20,  // Upgrade BSL
    EV_Set_DevicesId    = 0x21,
	
	EV_Get_ReadFlash    = 0x71,
} USB_RX_MSG_ID;

typedef enum {
// TX Messages.
	AC_Ack              = 0x80,
	AC_Send_Project_Mess = 0x81,
	AC_Send_Config      = 0x82, // structure mode
    AC_Send_Config_ID   = 0x83, // ID mode

	AC_Send_APN         = 0x84,
	EV_Send_SIM_CCID	= 0x85,//send sim ccid
	AC_Send_Calendar    = 0x86,
	AC_Send_Bluetooth_Name = 0x88,
	
	AC_Send_History_Data  = 0x8B,// 鏍规嵁缂栧彿鑾峰彇鍘嗗彶鏁版嵁,0.0.48鐗堟湰涔嬪墠璇啓鎴愪簡8A,闇�瑕佸伐鍏峰吋瀹�
	
    AC_Send_DevicePara    = 0x8F, //杩斿洖璁惧鍙傛暟

    AC_Send_Device_Data  = 0x91,//鑾峰彇鎸囧畾鏃堕棿娈垫暟鎹�
    AC_Send_Unupload_Data = 0x92,//鑾峰彇鏈笂浼犳暟鎹�
    AC_Send_Record_Data   = 0x93,//鑾峰彇寮�濮嬭褰曠殑鏁版嵁

    AC_Send_Upgrade_Success = 0x94,
    AC_Send_Voltage = 0x95,//涓婂彂褰撳墠鐢靛帇

    AC_Send_SensorData = 0x98,//涓婂彂sensor 鏁版嵁

    
    EV_Send_ReadFlash = 0xF1,
} USB_TX_MSG_ID;
typedef enum {
   UNKONW_PLATFORM       = 0x00,
   TI_430_F5529_PLATFORM = 0x01,
   TI_430_5635_PLATFORM  = 0x02,
   TI_432_401R_PLATFORM  = 0x03,
   TI_1310_PLATFORM      = 0x04,

}PLATFORM_TYPE;
typedef struct {
    // Usb interrup flag.
    uint8_t interruptFlag;
    // Usb CDC data receivedflag.
    uint8_t cdcDataReceivedFlag;
} UsbEvent_t;

#define USB_BUFF_LENGTH           (436+64)
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

int Usb_data_parse(uint8_t *pData, uint16_t length);

void UsbSend(USB_TX_MSG_ID msgId);
void UsbSend_NodeConfig(USB_TX_MSG_ID msgId, uint8_t* buff, uint8_t bufLen);




#endif	/* __ZKSIOT_USB_PROC_H__ */
