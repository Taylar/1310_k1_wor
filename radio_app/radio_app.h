#ifndef			__RADIO_APP_H__
#define			__RADIO_APP_H__

#include "../easylink/EasyLink.h"


#define         RADIO_EVT_TEST          Event_Id_00
#define         RADIO_EVT_RX            Event_Id_01
#define         RADIO_EVT_TX            Event_Id_02
#define         RADIO_EVT_TOUT          Event_Id_03
#define         RADIO_EVT_RX_FAIL       Event_Id_04
#define         RADIO_EVT_TX_FAIL       Event_Id_05
#define         RADIO_EVT_FAIL          Event_Id_06
#define         RADIO_EVT_SET_RX_MODE   Event_Id_07
#define         RADIO_EVT_SET_TX_MODE   Event_Id_08
#define         RADIO_EVT_DISABLE	    Event_Id_09
#define         RADIO_EVT_UPGRADE_SEND  Event_Id_10
#define         RADIO_EVT_UPGRADE_RX_TIMEOUT   Event_Id_11
#define         RADIO_EVT_SENSOR_PACK   Event_Id_12
#define         RADIO_EVT_RADIO_REPAIL  Event_Id_13
#define         RADIO_EVT_ALL           0xffff



#define PASSRADIO_ACK_TIMEOUT_TIME_MS (300)

#define RADIO_EASYLINK_MODULATION      EasyLink_Phy_Custom
#define RADIO_EASYLINK_MODULATION_50K  EasyLink_Phy_50K_GPSK
#define RADIO_EASYLINK_MODULATION_S1_OLD EasyLink_Phy_Custom_s1_old

enum RadioOperationStatus {
    RADIOSTATUS_IDLE,
    RADIOSTATUS_RECEIVING,
    RADIOSTATUS_TRANSMITTING,
    RADIOSTATUS_ABSORT,
};


typedef enum {
	RADIOMODE_SENDPORT,
	RADIOMODE_RECEIVEPORT,
	RADIOMODE_UPGRADE,
}RadioOperationMode ;


// radioRxPacket
extern EasyLink_RxPacket radioRxPacket;




void RadioAppTaskCreate(void) ;

void RadioModeSet(RadioOperationMode modeSet);

RadioOperationMode RadioModeGet(void);

void RadioAppTaskFxn(void);

bool RadioCopyPacketToBuf(uint8_t *dataP, uint8_t len, uint8_t maxNumberOfRetries, uint32_t ackTimeoutMs, uint8_t baseAddr);

void RadioEventPost(UInt event);

void RadioSensorDataPack(void);

void RadioSend(void);

void RadioUpgradeSendFile(void);

void RadioUpgradeRxFileDataTimout(void);


void RadioSendPacket(uint8_t *dataP, uint8_t len, uint8_t maxNumberOfRetries, uint32_t ackTimeoutMs);

void RadioResendPacket(void);

uint32_t GetRadioSrcAddr(void);

uint32_t GetRadioDstAddr(void);

void SetRadioSrcAddr(uint32_t addr);

void SetRadioDstAddr(uint32_t addr);

uint32_t GetRadioSubSrcAddr(void);

void SetRadioSubSrcAddr(uint32_t addr);

void ClearRadioSendBuf(void);

void RadioTestEnable(void);

void RadioTestDisable(void);

void RadioSetRxMode(void);

void RadioSetTxMode(void);

void RadioDisable(void);

void RadioSwitchingUpgradeRate(void);

void RadioSwitchingUserRate(void);

void RadioSwitchingS1OldUserRate(void);

uint8_t RadioStatueRead(void);

#endif		// __RADIO_APP_H__
