#ifndef			__PASS_RADIO_TASK_H__
#define			__PASS_RADIO_TASK_H__



#define PASSRADIO_ACK_TIMEOUT_TIME_MS (160)

#define RADIO_EASYLINK_MODULATION     EasyLink_Phy_Custom

enum RadioOperationStatus {
    RADIOSTATUS_SUCCESS,
    RADIOSTATUS_FAILED,
    RADIOSTATUS_FAILEDNOTCONNECTED,
};


enum RadioOperationMode {
	RADIOMODE_SENDPORT,
	RADIOMODE_RECEIVEPORT,
};






void PassRadioTaskCreate(void);

void RadioSendPacket(uint8_t *dataP, uint8_t len, uint8_t maxNumberOfRetries, uint32_t ackTimeoutMs);








#endif		// __PASS_RADIO_TASK_H__
