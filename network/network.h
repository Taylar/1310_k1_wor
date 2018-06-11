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




#define NWK_MSG_SIZE            UART_BUFF_SIZE


typedef struct {
    uint8_t buff[NWK_MSG_SIZE];
    uint16_t length;
} NwkMsgPacket_t;

#ifdef SUPPORT_LBS
#ifdef USE_QUECTEL_API_FOR_LBS

typedef struct {
//LBS longitude
    float longitude;
//LBS latitude
    float latitude;
} NwkLocation_t;

#elif defined(USE_ENGINEERING_MODE_FOR_LBS)

#define LBS_NEARBY_CELL_MAX     2

typedef struct {
    uint16_t lac;
    uint32_t cellid;
    int8_t dbm;
} NwkCellInfo_t;

typedef struct {
	uint16_t mcc;
	uint16_t mnc;
    NwkCellInfo_t local;
#ifdef SUPPOERT_LBS_NEARBY_CELL
    NwkCellInfo_t nearby[LBS_NEARBY_CELL_MAX];
#endif
} NwkLocation_t;

#endif
#endif

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
    NWK_CONTROL_NEARBY_LOCATION,
    NWK_CONTROL_NEARBY_LOCATION_OFF,
    NWK_CONTROL_TRANSMIT,
    NWK_CONTROL_SHUTDOWN_MSG,
    NWK_CONTROL_IMEI_GET,
    NWK_CONTROL_TEST
} NWK_CONTROL;

#endif  /* SUPPORT_NETWORK */

extern void Nwk_task_create(void);
extern void Nwk_upload_time_isr(void);
extern void Nwk_poweron(void);
extern void Nwk_poweroff(void);
extern uint8_t Nwk_get_rssi(void);
extern void Nwk_get_simccid(uint8_t *pBuff);
extern char Nwk_is_Active(void);
extern bool SetDevicePara(uint8_t *rxData, uint16_t length);
extern uint8_t GetDevicePara(uint8_t paratype, uint8_t *rxData);
uint8_t Nwk_get_state(void);

#endif	/* __ZKSIOT_NETWORK_H__ */

