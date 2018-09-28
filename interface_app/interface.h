#ifndef			__INTERFACE_H__
#define			__INTERFACE_H__



typedef	enum 
{
	INTERFACE_UART,
	INTERFACE_SPI,
}INTERFACE_TYPE;



#define			INTERFACE_EVT_RX			Event_Id_00
#define			INTERFACE_EVT_TX			Event_Id_01
#define			INTERFACE_EVT_ALL			0xffff




void InterfaceTaskCreate(void);

void InterfaceSend(uint8_t * datap, uint16_t len);

void InterfaceSendImmediately(uint8_t * datap, uint16_t len);

void InterfaceEnable(void);

void InterfaceDisable(void);


#endif			// __INTERFACE_H__
