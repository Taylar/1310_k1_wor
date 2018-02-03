//***********************************************************************************
// Copyright 2017, Zksiot Development Ltd.
// Created by Linjie, 2017.08.08
// MCU:	MSP430F5529
// OS: TI-RTOS
// Project:
// File name: network.h
// Description: network process routine head.
//***********************************************************************************

#ifndef __ZKSIOT_NETWORK_H__
#define __ZKSIOT_NETWORK_H__

#ifdef SUPPORT_NETWORK

#define NWK_MSG_SIZE            160

typedef struct {
    uint8_t buff[NWK_MSG_SIZE];
    uint16_t length;
} NwkMsgPacket_t;

typedef struct {
//LBS longitude
    float longitude;
//LBS latitude
    float latitude;
} NwkLocation_t;


typedef void (*Nwk_CallbackFxn) (uint8_t *pBuff, uint16_t length);

typedef struct Nwk_Params {
    //Callback function pointer
    Nwk_CallbackFxn dataProcCallbackFxn;
} Nwk_Params;

typedef void (*Nwk_initFxn) (Nwk_Params *params);
typedef uint8_t (*Nwk_openFxn) (void);
typedef uint8_t (*Nwk_closeFxn) (void);
typedef uint8_t (*Nwk_controlFxn) (uint8_t cmd, void *arg);

typedef struct Nwk_FxnTable {
    Nwk_initFxn     initFxn;
    Nwk_openFxn     openFxn;
    Nwk_closeFxn    closeFxn;
    Nwk_controlFxn  controlFxn;
} Nwk_FxnTable;

typedef enum {
    NWK_CONTROL_RSSI_GET,
    NWK_CONTROL_RSSI_QUERY,
    NWK_CONTROL_SIMCCID_GET,
    NWK_CONTROL_WAKEUP,
    NWK_CONTROL_SLEEP,
    NWK_CONTROL_LBS_QUERY,
    NWK_CONTROL_TRANSMIT,
    NWK_CONTROL_SHUTDOWN_MSG,
    NWK_CONTROL_TEST
} NWK_CONTROL;

#endif  /* SUPPORT_NETWORK */

extern void Nwk_task_create(void);
extern void Nwk_upload_time_isr(void);
extern void Nwk_poweron(void);
extern void Nwk_poweroff(void);
extern uint8_t Nwk_get_rssi(void);
extern void Nwk_get_simccid(uint8_t *pBuff);
extern void Nwk_upload_data(void);
extern void Nwk_ntp_syn(void);

#endif	/* __ZKSIOT_NETWORK_H__ */

