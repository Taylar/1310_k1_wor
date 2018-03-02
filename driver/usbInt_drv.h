#ifndef   	_USBINT_DRV_H_
#define		_USBINT_DRV_H

#define		USB_LINK_STATE			0
#define		USB_UNLINK_STATE		1


void UsbIntInit(void (*Cb)(void));

uint8_t GetUsbState(void);



#endif		// _USBINT_DRV_H
