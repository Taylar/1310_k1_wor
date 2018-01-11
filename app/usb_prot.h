#ifndef USB_PROT_H__
#define USB_PROT_H__


typedef enum {
// RX Messages.
	EV_Test = 0x01,
	EV_Get_Config,
	EV_Set_config,
	EV_Get_APN,
	EV_Set_APN,
	EV_Get_Calendar,
	EV_Set_Calendar,
	EV_Get_Bluetooth_Name,
	EV_Set_Bluetooth_Name,
	EV_Usb_Upgrade,
	EV_Get_DataRecord,
} USB_RX_MSG_ID;

typedef enum {
// TX Messages.
	AC_Ack = 0x80,
	AC_Send_Config = 0x82,
	AC_Send_APN = 0x84,
	AC_Send_Calendar = 0x86,
	AC_Send_Bluetooth_Name = 0x88,
	EV_Send_DataRecord = 0x8a,
} USB_TX_MSG_ID;

typedef struct {
    // Usb interrup flag.
    uint8_t interruptFlag;
    // Usb CDC data receivedflag.
    uint8_t cdcDataReceivedFlag;
} UsbEvent_t;

#define USB_BUFF_LENGTH           (256+54)
// Old protocol defined by DouQian. 
#define USB_PACKAGE_HEAD          0xcb
#define USB_PACKAGE_TAIL          0xbc

// New protocol defined by LinJie.
#define USB_PACKAGE_HEADL         0xff
#define USB_PACKAGE_HEADH         0x5a
#define USB_PACKAGE_TAILL         '\r'
#define USB_PACKAGE_TAILH         '\n'


extern void Usb_task_create(void);





#endif  	// USB_PROT_H__
