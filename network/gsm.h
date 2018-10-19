//***********************************************************************************
// Copyright 2017, Zksiot Development Ltd.
// Created by Linjie, 2017.08.08
// MCU:	MSP430F5529
// OS: TI-RTOS
// Project:
// File name: gsm.h
// Description: gsm process routine head.
//***********************************************************************************

#ifndef __ZKSIOT_GSM_H__
#define __ZKSIOT_GSM_H__

#ifdef SUPPORT_GSM

//Save module setting
#define ATCMD_SAVE_SETTING          "AT&W\r\n"

//Uart control cmd
#define ATCMD_UART_SYNC             "AT\r\n"
#define ATCMD_UART_SET_BAUDRATE     "AT+IPR=38400\r\n"

//Set command echo mode
#define ATCMD_ECHO_ON               "ATE1\r\n"
#define ATCMD_ECHO_OFF              "ATE0\r\n"

//Set low battery voltage alarm & shut down mode.
#define ATCMD_BAT_ALARM_ON          "AT+QVBATT=0,3500,1\r\n"
#define ATCMD_BAT_ALARM_OFF         "AT+QVBATT=0,3500,0\r\n"
#define ATCMD_BAT_SHUT_ON           "AT+QVBATT=1,3100,1\r\n"
#define ATCMD_BAT_SHUT_OFF          "AT+QVBATT=1,3100,0\r\n"

//SIM card related cmd
#define ATCMD_SIM_QUERY             "AT+CPIN?\r\n"
#define ATCMD_SIM_CCID              "AT+CCID\r\n"

//Signal Quality Report cmd
#define ATCMD_CSQ_QUERY             "AT+CSQ\r\n"

//GSM Network Registration
#define ATCMD_CREG_QUERY            "AT+CREG?\r\n"

//GPRS Network Registration Status
#define ATCMD_CGREG_QUERY           "AT+CGREG?\r\n"

//Attach or Detach from GPRS Service
#define ATCMD_CGATT_QUERY           "AT+CGATT?\r\n"

//Start Task and Set APN, USER NAME, PASSWORD
//#define ATCMD_SET_APN               "AT+QIREGAPP="
#define ATCMD_SET_APN               "AT+QICSGP=1,%s\r\n"
#define ATCMD_START_TASK            "AT+QIREGAPP\r\n"

//Activity mobile scene
#define ATCMD_ACT_MS                "AT+QIACT\r\n"

//Get Local IP Address
#define ATCMD_GET_LOCAL_IP          "AT+QILOCIP\r\n"

//Set connect by domian or ip
#define ATCMD_SET_DOMAINORIP        "AT+QIDNSIP=%d\r\n"

//Start Up TCP or UDP Connection
#define ATCMD_START_CONNECT         "AT+QIOPEN=\"TCP\","

//Send Data Through TCP or UDP Connection
#define ATCMD_SEND_DATA             "AT+QISEND="

//Query Previous Connection Data Transmitting State
#define ATCMD_ACK_QUERY             "AT+QISACK\r\n"

//Close TCP or UDP Connection
#define ATCMD_CLOSE_CONNECT         "AT+QICLOSE\r\n"

//De-activity mobile scene
#define ATCMD_DEACT_MS              "AT+QIDEACT\r\n"

//Set sleep mode
#define ATCMD_SLEEP_ON              "AT+QSCLK=2\r\n"
#define ATCMD_SLEEP_OFF             "AT+QSCLK=0\r\n"

//LBS, Get Current Location.
#define ATCMD_GET_LOCATION          "AT+QCELLLOC=1\r\n"

//Config receive date ip head
#define ATCMD_SET_RECEIVE_HEAD      "AT+QIHEAD=1\r\n"

//Phone auto answer function, use to test antenna.
#define ATCMD_AUTO_ANSWER           "ATS0=1\r\n"

//Set engineering mode
#ifdef SUPPOERT_LBS_NEARBY_CELL
#define ATCMD_ENG_MODE_ON           "AT+QENG=1,1\r\n"
#else
#define ATCMD_ENG_MODE_ON           "AT+QENG=1,0\r\n"
#endif
#define ATCMD_ENG_MODE_OFF          "AT+QENG=0\r\n"
#define ATCMD_ENG_MODE_QUERY        "AT+QENG?\r\n"

//Get IMEI
#define ATCMD_IMEI_QUERY            "AT+GSN\r\n"

typedef enum {
    AT_CMD_NULL = 0,
    AT_CMD_COMMON,
    AT_CMD_SIM_QUERY,
    AT_CMD_SIM_CCID,
    AT_CMD_CSQ_QUERY,
    AT_CMD_CREG_QUERY,
    AT_CMD_CGREG_QUERY,
    AT_CMD_GET_LOCAL_IP,
    AT_CMD_CONNECT,
    AT_CMD_START_SEND_DATA,
    AT_CMD_SEND_DATA,
    AT_CMD_ACK_QUERY,
    AT_CMD_CLOSE_CONNECT,
    AT_CMD_GET_LOCATION,
    AT_CMD_ENG_MODE_QUERY,
    AT_CMD_IMEI_QUERY,
    AT_CMD_MAX
} GSM_AT_CMD_TYPE;

typedef enum {
//Init state
    GSM_STATE_POWERON = 0,

    GSM_STATE_CONFIG,
    GSM_STATE_GPRS_QUERY,
    GSM_STATE_ACTIVE_PDP,
    GSM_STATE_TCP_CONNECT,
    GSM_STATE_TCP_UPLOAD,

    GSM_STATE_TEST,
    GSM_STATE_RESET,
    GSM_STATE_POWEROFF,

    GSM_STATE_MAX
} GSM_STATE;

typedef enum {
    GSM_NO_ERR = 0,
    GSM_ERR_UART_SYNC,      //TCP UART sync cmd respond error, need reset
    GSM_ERR_SIM_QUERY,      //TCP sim card query cmd respond error, need reset
    GSM_ERR_CSQ_QUERY,      //TCP csq query cmd respond error, need reset
    GSM_ERR_CREG_QUERY,     //TCP creg query cmd respond error, need reset
    GSM_ERR_CGREG_QUERY,    //TCP cgreg query cmd respond error, need reset
    GSM_ERR_DEACT,          //TCP deactive cmd timeout error, need reset
    GSM_ERR_ACT,            //TCP active cmd error
    GSM_ERR_TCP_CLOSE,      //TCP close cmd respond timeout error, need reset
    GSM_ERR_UPLOAD,         //TCP upload retry error
    GSM_ERR_SLEEP_MODE,     //TCP sleep cmd respond error, need reset
    GSM_ERR_WAKEUP_MODE,    //TCP wakeup cmd respond error, need reset
    GSM_ERR_GET_LBS,        //TCP get lbs information error
    GSM_ERR_GET_NEARBY_LOCATION,
    GSM_ERR_SET_NEARBY_LOCATION_OFF,
    GSM_ERR_AUTO_ANSWER,    //Phone auto answer error
    GSM_ERR_MAX
} GSM_ERR_STATE;

typedef enum {
    RESULT_OK = 0,
    RESULT_ERROR,
    RESULT_GPRS_QUERY,
    RESULT_CONNECT,
    RESULT_RESET,
    RESULT_SHUTDOWN,
    RESULT_MAX
} GSM_RESULT;

#define GSM_EVT_NONE                Event_Id_NONE
#define GSM_EVT_SHUTDOWN            Event_Id_00
#define GSM_EVT_CMD_OK              Event_Id_01
#define GSM_EVT_CMD_ERROR           Event_Id_02
#define GSM_EVT_CMD_RECONNECT       Event_Id_03
#define GSM_EVT_ALL                 0xffff

typedef struct {
    GSM_ERR_STATE error;
//AT cmd type
	GSM_AT_CMD_TYPE cmdType;
//module state
    uint8_t state;
//module open flag
    uint8_t isOpen;
//module sleep flag
    uint8_t sleep;
//reset counter
	uint8_t resetCnt;
//Active PDP counter
	uint8_t actPDPCnt;
//upload fail counter
	uint8_t uploadFailCnt;
//GSM RSSI
	uint8_t rssi;
#ifdef SUPPORT_LBS
//LBS location
    NwkLocation_t location;
#endif
#ifdef SUPPORT_IMEI
//IMEI
	uint8_t imei[15];
#endif
//Sim card ccid
	uint8_t simCcid[20];
//callback function
	Nwk_CallbackFxn dataProcCallbackFxn;
} GsmObject_t;


#endif  /* SUPPORT_GSM */

#endif	/* __ZKSIOT_GSM_H__ */

