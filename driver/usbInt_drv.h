#ifndef   	_USBINT_DRV_H_
#define		_USBINT_DRV_H



#ifdef BOARD_S3
#define		USB_LINK_STATE			0
#define		USB_UNLINK_STATE		1
#else
#define		USB_LINK_STATE			1
#define		USB_UNLINK_STATE		0
#endif 

typedef enum {
   NO_CHARGE = 0,
   CHARGE_FULL,
   CHARGEING
}ChargeStateType;


void UsbIntInit(void (*Cb)(void));

uint8_t GetUsbState(void);

extern ChargeStateType Get_Charge_plug(void);


#endif		// _USBINT_DRV_H
