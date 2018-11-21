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
#define         RADIO_EVT_SEND_CONFIG   Event_Id_14
#define         RADIO_EVT_SEND_SYC      Event_Id_15
#define         RADIO_EVT_CHANNEL_CHECK Event_Id_16
#define         RADIO_EVT_EVT_OLD_S1_UPLOAD_NODE Event_Id_17
#define         RADIO_EVT_ALL           0xffffffff



#define PASSRADIO_ACK_TIMEOUT_TIME_MS (150)
#define CONCENTER_RADIO_DELAY_TIME_MS (1)
#define RADIO_RECEIVE_TIMEOUT       RECEIVE_TIMEOUT[g_rSysConfigInfo.rfSF >> 4]

#define RADIO_EASYLINK_MODULATION      EasyLink_Phy_Custom
#define RADIO_EASYLINK_MODULATION_50K  EasyLink_Phy_50K_GPSK
#define RADIO_EASYLINK_MODULATION_S1_OLD EasyLink_Phy_Custom_s1_old

#define RADIO_BASE_FREQ                  433000000
#define RADIO_DIFF_UNIT_FREQ             125000
#define RADIO_BASE_UNIT_FREQ             250000

#define RADIO_MIN_POWER                 5

#define SET_RADIO_POWER                 (g_rSysConfigInfo.rfPA >> 4)
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


typedef enum
{
  // LORA [0: 433 MHz, 1: 433.25 MHz, 2: 433.50 MHz, 3: 433.75 MHz, 4: 434 MHz,
        // 5: 434.25 MHz, 6: 4334.50 MHz, 7: 434.75 MHz, 8: 435 MHz, 9: 435.25 MHz, other: Reserved]
    FREQ_435_25 = 0x90,      
    FREQ_435_00 = 0x80,
    FREQ_434_75 = 0x70,
    FREQ_434_50 = 0x60,
    FREQ_434_25 = 0x50,
    FREQ_434_00 = 0x40,
    FREQ_433_75 = 0x30,
    FREQ_433_50 = 0x20,
    FREQ_433_25 = 0x10,
    FREQ_433_00 = 0x00,
}t_FreqCenter;      //定义带宽枚举



// radioRxPacket
extern EasyLink_RxPacket radioRxPacket;

extern const uint16_t RECEIVE_TIMEOUT[5];



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

void SetRadioBrocastSrcAddr(uint32_t addr);

void ClearRadioSendBuf(void);

void RadioTestEnable(void);

void RadioTestDisable(void);

void RadioSetRxMode(void);

void RadioSetTxMode(void);

void RadioDisable(void);

uint8_t RadioStatueRead(void);

void Radio_setConfigModeRfFrequency(void);
void Radio_setRxModeRfFrequency(void);
void Radio_setTxModeRfFrequency(void);

void RadioSwitchingUpgradeRate(void);
void RadioSwitchingUserRate(void);
void RadioSwitchingSettingRate(void);

void RadioSwitchingS1OldUserRate(void);

void RadioAbort(void);

void RadioSetRfPower(uint8_t rfPower);


#endif		// __RADIO_APP_H__
