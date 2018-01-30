#ifndef			__RADIO_APP_H__
#define			__RADIO_APP_H__

#include "../easylink/EasyLink.h"


#define         RADIO_EVT_RX            Event_Id_00
#define         RADIO_EVT_TX            Event_Id_01
#define         RADIO_EVT_TOUT          Event_Id_02
#define         RADIO_EVT_FAIL          Event_Id_03
#define         RADIO_EVT_ALL           0xffff


#define DEFAULT_DST_ADDR                0x12341234

#define PASSRADIO_ACK_TIMEOUT_TIME_MS (50)

#define RADIO_EASYLINK_MODULATION     EasyLink_Phy_Custom

enum RadioOperationStatus {
    RADIOSTATUS_SUCCESS,
    RADIOSTATUS_FAILED,
    RADIOSTATUS_FAILEDNOTCONNECTED,
};


typedef enum {
	RADIOMODE_SENDPORT,
	RADIOMODE_RECEIVEPORT,
}RadioOperationMode ;


// radioRxPacket
extern EasyLink_RxPacket radioRxPacket;




void RadioAppTaskCreate(void) ;

void RadioModeSet(RadioOperationMode modeSet);

void RadioAppTaskFxn(void);

bool RadioCopyPacketToBuf(uint8_t *dataP, uint8_t len, uint8_t maxNumberOfRetries, uint32_t ackTimeoutMs, uint8_t baseAddr);

void RadioSend(void);

void RadioSendPacket(uint8_t *dataP, uint8_t len, uint8_t maxNumberOfRetries, uint32_t ackTimeoutMs);

void RadioResendPacket(void);

uint32_t GetRadioSrcAddr(void);

uint32_t GetRadioDstAddr(void);

void SetRadioSrcAddr(uint32_t addr);

void SetRadioDstAddr(uint32_t addr);

uint32_t GetRadioSubSrcAddr(void);

void SetRadioSubSrcAddr(uint32_t addr);

void ClearRadioSendBuf(void);

#endif		// __RADIO_APP_H__
