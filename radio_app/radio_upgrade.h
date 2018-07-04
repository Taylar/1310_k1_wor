
#ifndef __ZKSIOT_RADIO_UPGRADE_H__
#define __ZKSIOT_RADIO_UPGRADE_H__

#define RADIO_UPGRADE_CACHE_LEN              1
#define RADIO_UPGRADE_RESEND_TIMES           5
#define RADIO_UPGRAD_SWITCH_MAX              3
#define RADIO_UPGRADE_TX_TIMEOUT             (200 * CLOCK_UNIT_MS)
#define RADIO_UPGRADE_SHAKE_HANDLE_TIMEROUT  (1500 * CLOCK_UNIT_MS)
#define RADIO_UPGRADE_SHAKE_HANDLE_TIMES_MAX 10
#define RADIO_UPGRADE_RX_TIMEOUT             (15 * 1000 * CLOCK_UNIT_MS)


typedef struct {
    bool     isNoReceive;
    uint8_t  frameLen;
    uint32_t offset;
    uint8_t  resendCount;
    uint8_t  switchResend;
}radio_upgrade_info_t;

typedef struct {
    uint8_t txCount;
    radio_upgrade_info_t info[RADIO_UPGRADE_CACHE_LEN];
}radio_upgrade_TxInfo_t;

//extern usb_upgrade_info_t sTxRadio_upgrade_info;

void RadioUpgrade_Init(void);
void RadioUpgrade_start(uint32_t upgradeFileLen);
void RadioUpgrade_stop(void);
void RadioUpgrade_FileDataSend(void);

void RadioUpgrade_CmdDataParse(uint8_t *pData, uint16_t length);
void RadioUpgrade_CmdACKDataParse(uint8_t *pData, uint16_t length);
void RadioUpgrade_CmdRateSwitch(void);
void RadioUpgrade_CmdACKRateSwitch(uint8_t *pData, uint16_t length);


#endif /* __ZKSIOT_RADIO_UPGRADE_H__ */
