//***********************************************************************************
// Copyright 2017, Zksiot Development Ltd.
// Created by Linjie, 2017.08.08
// MCU:	MSP430F5529
// OS: TI-RTOS
// Project:
// File name: gsm.c
// Description: gsm process routine.
//***********************************************************************************
#include "../general.h"

#ifdef SUPPORT_GSM
#include "gsm.h"


#define     GSM_TIMEOUT_MS          20

#define GSM_POWER_PIN               IOID_2
#define GSM_KEY_PIN                 IOID_3

#define Gsm_power_ctrl(on)          PIN_setOutputValue(gsmPinHandle, GSM_POWER_PIN, on)
#define Gsm_pwrkey_ctrl(on)         PIN_setOutputValue(gsmPinHandle, GSM_KEY_PIN, !(on))



static const PIN_Config gsmPinTable[] = {
    GSM_POWER_PIN | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,       /* LED initially off          */
    GSM_KEY_PIN | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,       /* LED initially off          */
    PIN_TERMINATE
};


static void Gsm_event_post(UInt event);
static void Gsm_init(Nwk_Params *params);
static uint8_t Gsm_open(void);
static uint8_t Gsm_close(void);
static uint8_t Gsm_control(uint8_t cmd, void *arg);
static void Gsm_error_indicate(void);
static void Gsm_hwiIntCallback(uint8_t *dataP, uint8_t len);
const Nwk_FxnTable Gsm_FxnTable = {
    Gsm_init,
    Gsm_open,
    Gsm_close,
    Gsm_control,
};

static Event_Struct gsmEvtStruct;
static Event_Handle gsmEvtHandle;
static Swi_Struct gsmRxSwiStruct;
static Swi_Handle gsmRxSwiHandle;

GsmObject_t rGsmObject;

static PIN_State   gsmPinState;
static PIN_Handle  gsmPinHandle;


/* Clock for node period sending */

Clock_Struct gsmTimeOutClock;     /* not static so you can see in ROV */
Clock_Handle gsmTimeOutClockHandle;


//***********************************************************************************
//
// Gsm module io init.
//
//***********************************************************************************
static void Gsm_io_init(void)
{
    gsmPinHandle = PIN_open(&gsmPinState, gsmPinTable);
}

//***********************************************************************************
//
// Gsm module send AT command.
//
//***********************************************************************************
static void AT_send_data(uint8_t *pData, uint16_t length)
{
    Uart_send_burst_data(UART_0, pData, length);
}

//***********************************************************************************
//
// Gsm module send AT command.
//
//***********************************************************************************
static void AT_send_cmd(uint8_t *string)
{
    Uart_send_string(UART_0, string);
}

//***********************************************************************************
//
// Send ATCMD_UART_SYNC.
//
//***********************************************************************************
static void AT_save_setting(void)
{
    rGsmObject.cmdType = AT_CMD_COMMON;
    AT_send_cmd(ATCMD_SAVE_SETTING);
}

//***********************************************************************************
//
// Send ATCMD_UART_SYNC.
//
//***********************************************************************************
static void AT_uart_sync(void)
{
    rGsmObject.cmdType = AT_CMD_COMMON;
    AT_send_cmd(ATCMD_UART_SYNC);
}

//***********************************************************************************
//
// Send ATCMD_UART_SET_BAUDRATE.
//
//***********************************************************************************
static void AT_uart_set_baudrate(void)
{
    rGsmObject.cmdType = AT_CMD_COMMON;
    AT_send_cmd(ATCMD_UART_SET_BAUDRATE);
}

//***********************************************************************************
//
// Set echo mode cmd.
//
//***********************************************************************************
static void AT_set_echo_mode(uint8_t on)
{
    rGsmObject.cmdType = AT_CMD_COMMON;
    if (on) {
        AT_send_cmd(ATCMD_ECHO_ON);
    } else {
        AT_send_cmd(ATCMD_ECHO_OFF);
    }
}

#if 0
//***********************************************************************************
//
// Set low battery voltage alarm mode cmd.
//
//***********************************************************************************
static void AT_set_bat_alarm_mode(uint8_t on)
{
    rGsmObject.cmdType = AT_CMD_COMMON;
    if (on) {
        AT_send_cmd(ATCMD_BAT_ALARM_ON);
    } else {
        AT_send_cmd(ATCMD_BAT_ALARM_OFF);
    }
}

//***********************************************************************************
//
// Set low battery voltage shut down mode cmd.
//
//***********************************************************************************
static void AT_set_bat_shut_mode(uint8_t on)
{
    rGsmObject.cmdType = AT_CMD_COMMON;
    if (on) {
        AT_send_cmd(ATCMD_BAT_SHUT_ON);
    } else {
        AT_send_cmd(ATCMD_BAT_SHUT_OFF);
    }
}
#endif

//***********************************************************************************
//
// Send ATCMD_SIM_QUERY.
//
//***********************************************************************************
static void AT_sim_query(void)
{
    rGsmObject.cmdType = AT_CMD_SIM_QUERY;
    AT_send_cmd(ATCMD_SIM_QUERY);
}

//***********************************************************************************
//
// Send ATCMD_SIM_CCID.
//
//***********************************************************************************
static void AT_sim_ccid(void)
{
    rGsmObject.cmdType = AT_CMD_SIM_CCID;
    AT_send_cmd(ATCMD_SIM_CCID);
}

//***********************************************************************************
//
// Send ATCMD_CSQ_QUERY.
//
//***********************************************************************************
static void AT_csq_query(void)
{
    rGsmObject.cmdType = AT_CMD_CSQ_QUERY;
    AT_send_cmd(ATCMD_CSQ_QUERY);
}

//***********************************************************************************
//
// Send ATCMD_CREG_QUERY.
//
//***********************************************************************************
static void AT_creg_query(void)
{
    rGsmObject.cmdType = AT_CMD_CREG_QUERY;
    AT_send_cmd(ATCMD_CREG_QUERY);
}

//***********************************************************************************
//
// Send ATCMD_CGREG_QUERY.
//
//***********************************************************************************
static void AT_cgreg_query(void)
{
    rGsmObject.cmdType = AT_CMD_CGREG_QUERY;
    AT_send_cmd(ATCMD_CGREG_QUERY);
}


//***********************************************************************************
//
// Send AT_set_apn.
//
//***********************************************************************************
static void AT_set_apn(void)
{
    uint8_t buf[sizeof(ATCMD_SET_APN) + sizeof(g_rSysConfigInfo.apnuserpwd)];

    sprintf((char*)buf, ATCMD_SET_APN,g_rSysConfigInfo.apnuserpwd);
    rGsmObject.cmdType = AT_CMD_COMMON;
    AT_send_cmd(buf);
}

//***********************************************************************************
//
// Send ATCMD_START_TASK.
//
//***********************************************************************************
static void AT_start_task(void)
{
    rGsmObject.cmdType = AT_CMD_COMMON;
    AT_send_cmd(ATCMD_START_TASK);
}

//***********************************************************************************
//
// Send ATCMD_ACT_MS.
//
//***********************************************************************************
static void AT_active_ms(void)
{
    rGsmObject.cmdType = AT_CMD_COMMON;
    AT_send_cmd(ATCMD_ACT_MS);
}

#if 0
//***********************************************************************************
//
// Send ATCMD_GET_LOCAL_IP.
//
//***********************************************************************************
static void AT_get_local_ip(void)
{
    rGsmObject.cmdType = AT_CMD_GET_LOCAL_IP;
    AT_send_cmd(ATCMD_GET_LOCAL_IP);
}
#endif

//***********************************************************************************
//
// Send ATCMD_START_CONNECT.
//
//***********************************************************************************
static void AT_start_connect(void)
{
    uint8_t buff[48], index, length;

    strcpy((char *)buff, ATCMD_START_CONNECT);
    index = sizeof(ATCMD_START_CONNECT) - 1;
    length = sprintf((char *)(buff + index), "\"%d.%d.%d.%d\",\"%d\"\r\n", g_rSysConfigInfo.serverIpAddr[0],
                            g_rSysConfigInfo.serverIpAddr[1], g_rSysConfigInfo.serverIpAddr[2],
                            g_rSysConfigInfo.serverIpAddr[3], g_rSysConfigInfo.serverIpPort);
    index += length;
    buff[index] = '\0';
    rGsmObject.cmdType = AT_CMD_CONNECT;
    AT_send_cmd(buff);
}

//***********************************************************************************
//
// Send ATCMD_START_CONNECT.
//
//***********************************************************************************
static void AT_tcp_start_send_data(uint16_t length)
{
    uint8_t buff[16], index, len;

    strcpy((char *)buff, ATCMD_SEND_DATA);
    index = sizeof(ATCMD_SEND_DATA) - 1;
    len = sprintf((char *)(buff + index), "%d\r\n", length);
    index += len;
    buff[index] = '\0';
    rGsmObject.cmdType = AT_CMD_START_SEND_DATA;
    AT_send_cmd(buff);
}

//***********************************************************************************
//
// Send ATCMD_START_CONNECT.
//
//***********************************************************************************
static void AT_tcp_send_data(uint8_t *pBuff, uint16_t length)
{
    rGsmObject.cmdType = AT_CMD_SEND_DATA;
    AT_send_data(pBuff, length);
}

//***********************************************************************************
//
// Send ATCMD_ACK_QUERY.
//
//***********************************************************************************
static void AT_ack_query(void)
{
    rGsmObject.cmdType = AT_CMD_ACK_QUERY;
    AT_send_cmd(ATCMD_ACK_QUERY);
}

//***********************************************************************************
//
// Send ATCMD_CLOSE_CONNECT.
//
//***********************************************************************************
static void AT_close_connect(void)
{
    rGsmObject.cmdType = AT_CMD_CLOSE_CONNECT;
    AT_send_cmd(ATCMD_CLOSE_CONNECT);
}

//***********************************************************************************
//
// Send ATCMD_DEACT_MS.
//
//***********************************************************************************
static void AT_deactive_ms(void)
{
    rGsmObject.cmdType = AT_CMD_COMMON;
    AT_send_cmd(ATCMD_DEACT_MS);
}

//***********************************************************************************
//
// Set sleep mode cmd.
//
//***********************************************************************************
static void AT_set_sleep_mode(uint8_t on)
{
    rGsmObject.cmdType = AT_CMD_COMMON;
    if (on) {
        AT_send_cmd(ATCMD_SLEEP_ON);
    } else {
        AT_send_cmd(ATCMD_SLEEP_OFF);
    }
}

//***********************************************************************************
//
// Send ATCMD_SET_RECEIVE_HEAD.
//
//***********************************************************************************
static void AT_set_receive_head(void)
{
    rGsmObject.cmdType = AT_CMD_COMMON;
    AT_send_cmd(ATCMD_SET_RECEIVE_HEAD);
}

//***********************************************************************************
//
// Send ATCMD_GET_LOCATION.
//
//***********************************************************************************
static void AT_lbs_get_location(void)
{
    rGsmObject.cmdType = AT_CMD_GET_LOCATION;
    AT_send_cmd(ATCMD_GET_LOCATION);
}

//***********************************************************************************
//
// Send ATCMD_GET_LOCATION.
//
//***********************************************************************************
static void AT_auto_answer(void)
{
    rGsmObject.cmdType = AT_CMD_COMMON;
    AT_send_cmd(ATCMD_AUTO_ANSWER);
}

//***********************************************************************************
//
// Gsm module wait cmd ack.
//
//***********************************************************************************
static UInt Gsm_wait_ack(uint32_t timeout)
{
    UInt eventId;

    eventId = Event_pend(gsmEvtHandle, 0, GSM_EVT_SHUTDOWN | GSM_EVT_CMD_OK | GSM_EVT_CMD_ERROR, timeout * CLOCK_UNIT_MS);

    rGsmObject.cmdType = AT_CMD_NULL;

    return eventId;
}

//***********************************************************************************
//
// Gsm module poweron.
//
//***********************************************************************************
static void Gsm_poweron(void)
{
    if (rGsmObject.state == GSM_STATE_POWEROFF) {
        UartHwInit(UART_0, 38400, Gsm_hwiIntCallback, UART_GSM);

        Gsm_power_ctrl(1);
        Gsm_pwrkey_ctrl(1);
        Task_sleep(200 * CLOCK_UNIT_MS);
        Gsm_pwrkey_ctrl(0);
        Task_sleep(1200 * CLOCK_UNIT_MS);
        Gsm_pwrkey_ctrl(1);
//      Task_sleep(1000 * CLOCK_UNIT_MS);

        rGsmObject.sleep = 0;
        rGsmObject.actPDPCnt = 0;
        rGsmObject.uploadFailCnt = 0;
        rGsmObject.state = GSM_STATE_CONFIG;

        Event_pend(gsmEvtHandle, 0, GSM_EVT_ALL, BIOS_NO_WAIT);
    }
}

//***********************************************************************************
//
// Gsm module poweroff.
//
//***********************************************************************************
static void Gsm_poweroff(void)
{
    if (rGsmObject.state != GSM_STATE_POWEROFF) {
        UartClose(UART_0);
        UartPortDisable(UART_GSM);
        Gsm_pwrkey_ctrl(0);
        Task_sleep(1000 * CLOCK_UNIT_MS);
        Gsm_pwrkey_ctrl(1);
        Task_sleep(12000 * CLOCK_UNIT_MS);
        Gsm_power_ctrl(0);
        Task_sleep(800 * CLOCK_UNIT_MS);

        rGsmObject.sleep = 0;
        rGsmObject.actPDPCnt = 0;
        rGsmObject.uploadFailCnt = 0;
        rGsmObject.state = GSM_STATE_POWEROFF;
    }
}

//***********************************************************************************
//
// Gsm module reset process.
//
//***********************************************************************************
static void Gsm_reset(void)
{
    UInt eventId = 0;

    Led_ctrl(LED_B, 0, 0, 0);
    Task_sleep(500 * CLOCK_UNIT_MS);
    Gsm_error_indicate();

    Gsm_poweroff();

    if (rGsmObject.resetCnt < 250)
        rGsmObject.resetCnt++;

    if (rGsmObject.resetCnt >= 3) {
        eventId = Event_pend(gsmEvtHandle, 0, GSM_EVT_SHUTDOWN, 30 * 60 * CLOCK_UNIT_S);
    } else if (rGsmObject.resetCnt >= 2) {
        eventId = Event_pend(gsmEvtHandle, 0, GSM_EVT_SHUTDOWN, 10 * 60 * CLOCK_UNIT_S);
    }

    if (!(eventId & GSM_EVT_SHUTDOWN)) {
        Gsm_poweron();
    }
}

//***********************************************************************************
//
// Gsm module config process.
// Return:  RESULT_OK -- config ok
//          RESULT_RESET -- need reset module
//          RESULT_SHUTDOWN -- need shutdown module
//
//***********************************************************************************
static GSM_RESULT Gsm_module_config(void)
{
    uint8_t i;
    UInt eventId;

    rGsmObject.error = GSM_NO_ERR;

    //uart sync
    for (i = 30; i > 0; i--) {
        AT_uart_sync();
        eventId = Gsm_wait_ack(1000);
        if (eventId & (GSM_EVT_CMD_OK | GSM_EVT_SHUTDOWN))
            break;
    }
    if (eventId & GSM_EVT_SHUTDOWN) {
        return RESULT_SHUTDOWN;
    } else if (i == 0) {
        rGsmObject.error = GSM_ERR_UART_SYNC;
        return RESULT_RESET;
    }

    //uart set baudrate
    AT_uart_set_baudrate();
    eventId = Gsm_wait_ack(500);
    if (eventId & GSM_EVT_SHUTDOWN) {
        return RESULT_SHUTDOWN;
    }

    //uart shut echo mode
    AT_set_echo_mode(0);
    eventId = Gsm_wait_ack(500);
    if (eventId & GSM_EVT_SHUTDOWN) {
        return RESULT_SHUTDOWN;
    }

    //uart save setting
    AT_save_setting();
    eventId = Gsm_wait_ack(500);
    if (eventId & GSM_EVT_SHUTDOWN) {
        return RESULT_SHUTDOWN;
    }

    //uart set receive head
    for (i = 5; i > 0; i--) {
        AT_set_receive_head();
        eventId = Gsm_wait_ack(500);
        if (eventId & (GSM_EVT_CMD_OK | GSM_EVT_SHUTDOWN))
            break;
    }
    if (eventId & GSM_EVT_SHUTDOWN) {
        return RESULT_SHUTDOWN;
    } else if (i == 0) {
        rGsmObject.error = GSM_ERR_UART_SYNC;
        return RESULT_RESET;
    }

    return RESULT_OK;
}

//***********************************************************************************
//
// Gsm module gprs network query process.
// Return:  RESULT_OK -- gprs network query ok
//          RESULT_RESET -- need reset module
//          RESULT_SHUTDOWN -- need shutdown module
//
//***********************************************************************************
static GSM_RESULT Gsm_gprs_query(void)
{
    uint8_t i;
    UInt eventId;

    rGsmObject.error = GSM_NO_ERR;

    //sim card query
    for (i = 30; i > 0; i--) {
        AT_sim_query();
        eventId = Gsm_wait_ack(1000);
        if (eventId & (GSM_EVT_CMD_OK | GSM_EVT_SHUTDOWN))
            break;
    }
    if (eventId & GSM_EVT_SHUTDOWN) {
        return RESULT_SHUTDOWN;
    } else if (i == 0) {
        rGsmObject.error = GSM_ERR_SIM_QUERY;
        return RESULT_RESET;
    }

    //sim card get ccid
    for (i = 5; i > 0; i--) {
        AT_sim_ccid();
        eventId = Gsm_wait_ack(1000);
        if (eventId & (GSM_EVT_CMD_OK | GSM_EVT_SHUTDOWN))
            break;
    }
    if (eventId & GSM_EVT_SHUTDOWN) {
        return RESULT_SHUTDOWN;
    } else if (i == 0) {
        rGsmObject.error = GSM_ERR_SIM_QUERY;
        return RESULT_RESET;
    }

    //csq query
    for (i = 60; i > 0; i--) {
        AT_csq_query();
        eventId = Gsm_wait_ack(1000);
        if (eventId & (GSM_EVT_CMD_OK | GSM_EVT_SHUTDOWN))
            break;
    }
    if (eventId & GSM_EVT_SHUTDOWN) {
        return RESULT_SHUTDOWN;
    } else if (i == 0) {
        rGsmObject.error = GSM_ERR_CSQ_QUERY;
        return RESULT_RESET;
    }

    //cgreg query
    for (i = 60; i > 0; i--) {
        AT_cgreg_query();
        eventId = Gsm_wait_ack(1000);
        if (eventId & (GSM_EVT_CMD_OK | GSM_EVT_SHUTDOWN))
            break;
    }
    if (eventId & GSM_EVT_SHUTDOWN) {
        return RESULT_SHUTDOWN;
    } else if (i == 0) {
        rGsmObject.error = GSM_ERR_CGREG_QUERY;
        return RESULT_RESET;
    }

    return RESULT_OK;
}

//***********************************************************************************
//
// Gsm module active PDP process.
// Return:  RESULT_OK -- active PDP ok
//          RESULT_RESET -- need reset module
//          RESULT_SHUTDOWN -- need shutdown module
//          RESULT_GPRS_QUERY -- need goto gprs query
//
//***********************************************************************************
static GSM_RESULT Gsm_active_PDP(void)
{
    UInt eventId;

    rGsmObject.error = GSM_NO_ERR;

    //设置接入点 APN、用户名和密码
    if (g_rSysConfigInfo.apnuserpwd[0]){
        AT_set_apn();
        eventId = Gsm_wait_ack(500);
        if (eventId & GSM_EVT_SHUTDOWN){
            return RESULT_SHUTDOWN;
        }
    }
    
    //启动任务
    AT_start_task();
    eventId = Gsm_wait_ack(500);
    if (eventId & GSM_EVT_SHUTDOWN) {
        return RESULT_SHUTDOWN;
    }

    //激活移动场景（或发起 GPRS/CSD 无线连接）
    AT_active_ms();
    eventId = Gsm_wait_ack(150000);
    if (eventId & GSM_EVT_SHUTDOWN) {
        return RESULT_SHUTDOWN;
    } else if (eventId == 0) {
        //timeout
        rGsmObject.error = GSM_ERR_ACT;
        return RESULT_RESET;
    } else if (eventId & GSM_EVT_CMD_ERROR) {
        //去激活场景
        AT_deactive_ms();
        eventId = Gsm_wait_ack(90000);
        if (eventId & GSM_EVT_SHUTDOWN) {
            return RESULT_SHUTDOWN;
        } else if (eventId & GSM_EVT_CMD_OK) {
            return RESULT_GPRS_QUERY;
        } else {
            //timeout or de-act fail
            rGsmObject.error = GSM_ERR_DEACT;
            return RESULT_RESET;
        }
    }

    return RESULT_OK;
}

//***********************************************************************************
//
// Gsm module tcp connect process.
// Return:  RESULT_OK -- tcp connect ok
//          RESULT_RESET -- need reset module
//          RESULT_SHUTDOWN -- need shutdown module
//          RESULT_GPRS_QUERY -- need goto gprs query
//
//***********************************************************************************
static GSM_RESULT Gsm_tcp_connect(void)
{
    uint8_t i;
    UInt eventId;

    rGsmObject.error = GSM_NO_ERR;

    //建立 TCP 连接或注册 UDP 端口号
    for (i = 5; i > 0; i--) {
        AT_start_connect();
        eventId = Gsm_wait_ack(90000);
        if (!(eventId & GSM_EVT_CMD_ERROR))
            break;
    }
    if (eventId & GSM_EVT_SHUTDOWN) {
        return RESULT_SHUTDOWN;
    } else if (i == 0 || eventId == 0) {
        //去激活场景
        AT_deactive_ms();
        eventId = Gsm_wait_ack(90000);
        if (eventId & GSM_EVT_SHUTDOWN) {
            return RESULT_SHUTDOWN;
        } else if (eventId & GSM_EVT_CMD_OK) {
            return RESULT_GPRS_QUERY;
        } else {
            //timeout or de-act fail
            rGsmObject.error = GSM_ERR_DEACT;
            return RESULT_RESET;
        }
    }

    return RESULT_OK;
}

//***********************************************************************************
//
// Gsm module tcp upload process.
// Return:  RESULT_OK -- tcp upload ok
//          RESULT_RESET -- need reset module
//          RESULT_SHUTDOWN -- need shutdown module
//          RESULT_GPRS_QUERY -- need goto gprs query
//          RESULT_CONNECT -- need goto tcp connect
//
//***********************************************************************************
static GSM_RESULT Gsm_tcp_upload(uint8_t *pBuff, uint16_t length)
{
    uint8_t i;
    UInt eventId;

    rGsmObject.error = GSM_NO_ERR;

    //Get csq value
    AT_csq_query();
    eventId = Gsm_wait_ack(500);
    if (eventId & GSM_EVT_SHUTDOWN) {
        return RESULT_SHUTDOWN;
    } else if (!(eventId & GSM_EVT_CMD_OK)) {
        //timeout or error
        rGsmObject.error = GSM_ERR_CSQ_QUERY;
        return RESULT_RESET;
    }

    //tcp send data
    AT_tcp_start_send_data(length);
    eventId = Gsm_wait_ack(500);
    if (eventId & GSM_EVT_SHUTDOWN) {
        return RESULT_SHUTDOWN;
    } else if (!(eventId & GSM_EVT_CMD_OK)) {
        //timeout or error
        goto LAB_UPLOAD_CLOSE_CONNECT;
    }
    AT_tcp_send_data(pBuff, length);
    eventId = Gsm_wait_ack(500);
    if (eventId & GSM_EVT_SHUTDOWN) {
        return RESULT_SHUTDOWN;
    } else if (!(eventId & GSM_EVT_CMD_OK)) {
        //timeout or error
        goto LAB_UPLOAD_CLOSE_CONNECT;
    }

    //tcp send data ack query
    for (i = 180; i > 0; i--) {
        AT_ack_query();
        eventId = Gsm_wait_ack(500);
        if (eventId & (GSM_EVT_CMD_OK | GSM_EVT_SHUTDOWN))
            break;
    }
    if (eventId & GSM_EVT_SHUTDOWN) {
        return RESULT_SHUTDOWN;
    } else if (i == 0) {
        //timeout
        goto LAB_UPLOAD_CLOSE_CONNECT;
    }

    return RESULT_OK;

LAB_UPLOAD_CLOSE_CONNECT:
    AT_close_connect();
    eventId = Gsm_wait_ack(60000);
    if (eventId & GSM_EVT_SHUTDOWN) {
        return RESULT_SHUTDOWN;
    } else if (eventId & GSM_EVT_CMD_ERROR) {
        return RESULT_GPRS_QUERY;
    } else if (eventId == 0) {
        //timeout
        rGsmObject.error = GSM_ERR_TCP_CLOSE;
        return RESULT_RESET;
    }

    return RESULT_CONNECT;
}

//***********************************************************************************
//
// Gsm module query csq value process.
// Return:  RESULT_OK -- query csq ok
//          RESULT_ERROR -- query csq error
//          RESULT_SHUTDOWN -- need shutdown module
//
//***********************************************************************************
static GSM_RESULT Gsm_query_csq(void)
{
    UInt eventId;

    rGsmObject.error = GSM_NO_ERR;

    //Get csq value
    AT_csq_query();
    eventId = Gsm_wait_ack(500);
    if (eventId & GSM_EVT_SHUTDOWN) {
        return RESULT_SHUTDOWN;
    } else if (!(eventId & GSM_EVT_CMD_OK)) {
        //timeout or error
        rGsmObject.error = GSM_ERR_CSQ_QUERY;
        return RESULT_RESET;
    }

    return RESULT_OK;
}

//***********************************************************************************
//
// Gsm module get lbs information process.
// Return:  RESULT_OK -- get lbs information ok
//          RESULT_ERROR -- get lbs information error
//          RESULT_SHUTDOWN -- need shutdown module
//
//***********************************************************************************
static GSM_RESULT Gsm_get_lbs(void)
{
    UInt eventId;

    rGsmObject.error = GSM_NO_ERR;

    AT_lbs_get_location();
    eventId = Gsm_wait_ack(60000);
    if (eventId & GSM_EVT_SHUTDOWN) {
        return RESULT_SHUTDOWN;
    } else if (eventId == 0 ||(eventId & GSM_EVT_CMD_ERROR)) {
        //timeout
        rGsmObject.error = GSM_ERR_GET_LBS;
        return RESULT_ERROR;
    }

    return RESULT_OK;
}

//***********************************************************************************
//
// Gsm module sleep process.
// Return:  RESULT_OK -- sleep ok
//          RESULT_RESET -- need reset module
//          RESULT_SHUTDOWN -- need shutdown module
//
//***********************************************************************************
static GSM_RESULT Gsm_sleep(void)
{
    uint8_t i;
    UInt eventId;

    rGsmObject.error = GSM_NO_ERR;

    //set sleep mode
    for (i = 5; i > 0; i--) {
        AT_set_sleep_mode(1);
        eventId = Gsm_wait_ack(500);
        if (eventId & (GSM_EVT_CMD_OK | GSM_EVT_SHUTDOWN))
            break;
    }
    if (eventId & GSM_EVT_SHUTDOWN) {
        return RESULT_SHUTDOWN;
    } else if (i == 0) {
        rGsmObject.error = GSM_ERR_SLEEP_MODE;
        return RESULT_RESET;
    }

    return RESULT_OK;
}

//***********************************************************************************
//
// Gsm module wakeup process.
// Return:  RESULT_OK -- wakeup ok
//          RESULT_RESET -- need reset module
//          RESULT_SHUTDOWN -- need shutdown module
//
//***********************************************************************************
static GSM_RESULT Gsm_wakeup(void)
{
    uint8_t i;
    UInt eventId;

    rGsmObject.error = GSM_NO_ERR;

    AT_uart_sync();
    Task_sleep(200 * CLOCK_UNIT_MS);

    //set sleep mode
    for (i = 5; i > 0; i--) {
        AT_set_sleep_mode(0);
        eventId = Gsm_wait_ack(500);
        if (eventId & (GSM_EVT_CMD_OK | GSM_EVT_SHUTDOWN))
            break;
    }
    if (eventId & GSM_EVT_SHUTDOWN) {
        return RESULT_SHUTDOWN;
    } else if (i == 0) {
        rGsmObject.error = GSM_ERR_WAKEUP_MODE;
        return RESULT_RESET;
    }

    return RESULT_OK;
}

//***********************************************************************************
//
// Gsm module set auto answer state to test antenna.
// Return:  GSM_NO_ERR -- ok
//
//***********************************************************************************
static GSM_RESULT Gsm_test(void)
{
    uint8_t i;
    UInt eventId;

    rGsmObject.error = GSM_NO_ERR;

    //uart sync
    for (i = 30; i > 0; i--) {
        AT_uart_sync();
        eventId = Gsm_wait_ack(1000);
        if (eventId & (GSM_EVT_CMD_OK | GSM_EVT_SHUTDOWN))
            break;
    }
    if (eventId & GSM_EVT_SHUTDOWN) {
        return RESULT_SHUTDOWN;
    } else if (i == 0) {
        rGsmObject.error = GSM_ERR_UART_SYNC;
        return RESULT_RESET;
    }

    //sim card query
    for (i = 30; i > 0; i--) {
        AT_sim_query();
        eventId = Gsm_wait_ack(1000);
        if (eventId & (GSM_EVT_CMD_OK | GSM_EVT_SHUTDOWN))
            break;
    }
    if (eventId & GSM_EVT_SHUTDOWN) {
        return RESULT_SHUTDOWN;
    } else if (i == 0) {
        rGsmObject.error = GSM_ERR_SIM_QUERY;
        return RESULT_RESET;
    }

    //csq query
    for (i = 60; i > 0; i--) {
        AT_csq_query();
        eventId = Gsm_wait_ack(1000);
        if (eventId & (GSM_EVT_CMD_OK | GSM_EVT_SHUTDOWN))
            break;
    }
    if (eventId & GSM_EVT_SHUTDOWN) {
        return RESULT_SHUTDOWN;
    } else if (i == 0) {
        rGsmObject.error = GSM_ERR_CSQ_QUERY;
        return RESULT_RESET;
    }

    //creg query
    for (i = 60; i > 0; i--) {
        AT_creg_query();
        eventId = Gsm_wait_ack(1000);
        if (eventId & (GSM_EVT_CMD_OK | GSM_EVT_SHUTDOWN))
            break;
    }
    if (eventId & GSM_EVT_SHUTDOWN) {
        return RESULT_SHUTDOWN;
    } else if (i == 0) {
        rGsmObject.error = GSM_ERR_CREG_QUERY;
        return RESULT_RESET;
    }

    //auto answer
    for (i = 200; i > 0; i--) {
        AT_auto_answer();
        eventId = Gsm_wait_ack(500);
        if (eventId & (GSM_EVT_CMD_OK | GSM_EVT_SHUTDOWN))
            break;
    }
    if (eventId & GSM_EVT_SHUTDOWN) {
        return RESULT_SHUTDOWN;
    } else if (i == 0) {
        rGsmObject.error = GSM_ERR_AUTO_ANSWER;
        return RESULT_RESET;
    }

    return RESULT_OK;
}

//***********************************************************************************
//
// Gsm module transmit process.
//
//***********************************************************************************
static GSM_RESULT Gsm_transmit_process(uint8_t *pBuff, uint16_t length)
{
    GSM_RESULT result;

//Gsm config
    if (rGsmObject.state == GSM_STATE_CONFIG) {
        result = Gsm_module_config();
        if (result == RESULT_SHUTDOWN || result == RESULT_RESET) {
            return result;
        }
        rGsmObject.state = GSM_STATE_GPRS_QUERY;
    }

LAB_GPRS_QUERY:
//Gsm sim card and gprs network query
    if (rGsmObject.state == GSM_STATE_GPRS_QUERY) {
        result = Gsm_gprs_query();
        if (result == RESULT_SHUTDOWN || result == RESULT_RESET) {
            return result;
        }
        rGsmObject.state = GSM_STATE_ACTIVE_PDP;
    }

//Gsm active PDP
    if (rGsmObject.state == GSM_STATE_ACTIVE_PDP) {
        if (rGsmObject.actPDPCnt++ >= 3) {
            rGsmObject.error = GSM_ERR_ACT;
            return RESULT_RESET;
        }

        result = Gsm_active_PDP();
        if (result == RESULT_SHUTDOWN || result == RESULT_RESET) {
            return result;
        } else if (result == RESULT_GPRS_QUERY) {
            //Active PDP fail
            rGsmObject.state = GSM_STATE_GPRS_QUERY;
            goto LAB_GPRS_QUERY;
        }
        rGsmObject.state = GSM_STATE_TCP_CONNECT;
        rGsmObject.actPDPCnt = 0;
    }

LAB_CONNECT:
//Gsm connect server
    if (rGsmObject.state == GSM_STATE_TCP_CONNECT) {
        result = Gsm_tcp_connect();
        if (result == RESULT_SHUTDOWN || result == RESULT_RESET) {
            return result;
        } else if (result == RESULT_GPRS_QUERY) {
            //Active PDP fail
            rGsmObject.state = GSM_STATE_GPRS_QUERY;
            goto LAB_GPRS_QUERY;
        }
        rGsmObject.state = GSM_STATE_TCP_UPLOAD;
    }

    if (pBuff != NULL && length != 0) {
        if (rGsmObject.uploadFailCnt++ >= 3) {
            rGsmObject.error = GSM_ERR_UPLOAD;
            return RESULT_RESET;
        }

        result = Gsm_tcp_upload(pBuff, length);
        if (result == RESULT_SHUTDOWN || result == RESULT_RESET) {
            return result;
        } else if (result == RESULT_GPRS_QUERY) {
            rGsmObject.state = GSM_STATE_GPRS_QUERY;
            goto LAB_GPRS_QUERY;
        } else if (result == RESULT_CONNECT) {
            //Active PDP fail
            rGsmObject.state = GSM_STATE_TCP_CONNECT;
            goto LAB_CONNECT;
        }

        rGsmObject.uploadFailCnt = 0;
        rGsmObject.resetCnt = 0;
    }

    return RESULT_OK;
}

//***********************************************************************************
//
// Gsm AT command respond monitor.
//
//***********************************************************************************
static void Gsm_rxSwiFxn(void)
{
    char *ptr;
    uint8_t index;
    uint16_t value, rxLen;
    float latitudetmp;

    /* Disable preemption. */
    uart0RxData.buff[uart0RxData.length] = '\0';

    ptr = strstr((char *)uart0RxData.buff, "IPD");
    if (ptr != NULL) {
        index = ptr - (char *)uart0RxData.buff;
        rxLen = atoi(ptr + 3);
        if ((uart0RxData.length - index) > rxLen && rxLen < 128) {
            //Second 0x7e
            ptr = strstr((char *)ptr, "\x7e");
            rGsmObject.dataProcCallbackFxn((uint8_t *)ptr, rxLen);
        } else {
            //First 0x7e
        }
        return;
    }

    switch (rGsmObject.cmdType) {
        case AT_CMD_COMMON:
            ptr = strstr((char *)uart0RxData.buff, "OK");
            if (ptr != NULL) {
                Gsm_event_post(GSM_EVT_CMD_OK);
                break;
            }
            ptr = strstr((char *)uart0RxData.buff, "ERROR");
            if (ptr != NULL) {
                Gsm_event_post(GSM_EVT_CMD_ERROR);
            }
            break;

        case AT_CMD_SIM_QUERY:
            ptr = strstr((char *)uart0RxData.buff, "OK");
            if (ptr != NULL) {
                ptr = strstr((char *)uart0RxData.buff, "READY");
                if (ptr != NULL) {
                    Gsm_event_post(GSM_EVT_CMD_OK);
                }
            }
            break;

        case AT_CMD_SIM_CCID:
            ptr = strstr((char *)uart0RxData.buff, "OK");
            if (ptr != NULL) {
                ptr = strstr((char *)uart0RxData.buff, "+CCID:");
                if (ptr != NULL) {
                    memcpy((char *)rGsmObject.simCcid, ptr + 8, 20);
                    Gsm_event_post(GSM_EVT_CMD_OK);
                }
            }
            break;

        case AT_CMD_CSQ_QUERY:
            ptr = strstr((char *)uart0RxData.buff, "OK");
            if (ptr != NULL) {
                ptr = strstr((char *)uart0RxData.buff, "+CSQ:");
                if (ptr != NULL) {
                    rGsmObject.rssi = (uint8_t)atoi(ptr + 5);
                    if (rGsmObject.rssi != 99) {
                        if(rGsmObject.rssi > 31)rGsmObject.rssi = 31;//处理出现大于31的异常
                        Gsm_event_post(GSM_EVT_CMD_OK);
                    }
                }
            }
            break;

        case AT_CMD_CREG_QUERY:
            ptr = strstr((char *)uart0RxData.buff, "OK");
            if (ptr != NULL) {
                ptr = strstr((char *)uart0RxData.buff, "+CREG:");
                if (ptr != NULL) {
                    if (*(ptr + 9) == '1' || *(ptr + 9) == '5') {
                        Gsm_event_post(GSM_EVT_CMD_OK);
                    }
                }
            }
            break;

        case AT_CMD_CGREG_QUERY:
            ptr = strstr((char *)uart0RxData.buff, "OK");
            if (ptr != NULL) {
                ptr = strstr((char *)uart0RxData.buff, "+CGREG:");
                if (ptr != NULL) {
                    if (*(ptr + 10) == '1' || *(ptr + 10) == '5') {
                        Gsm_event_post(GSM_EVT_CMD_OK);
                    }
                }
            }
            break;

        case AT_CMD_GET_LOCAL_IP:
            ptr = strstr((char *)uart0RxData.buff, ".");
            if (ptr != NULL) {
                Gsm_event_post(GSM_EVT_CMD_OK);
            }
            break;

        case AT_CMD_CONNECT:
            ptr = strstr((char *)uart0RxData.buff, "ERROR");
            if (ptr != NULL) {
                ptr = strstr((char *)uart0RxData.buff, "ALREADY CONNECT");
                if (ptr != NULL) {
                    Gsm_event_post(GSM_EVT_CMD_OK);
                    break;
                } else {
                    Gsm_event_post(GSM_EVT_CMD_ERROR);
                    break;
                }
            }
            ptr = strstr((char *)uart0RxData.buff, "CONNECT OK");
            if (ptr != NULL) {
                Gsm_event_post(GSM_EVT_CMD_OK);
                break;
            }
            ptr = strstr((char *)uart0RxData.buff, "CONNECT FAIL");
            if (ptr != NULL) {
                Gsm_event_post(GSM_EVT_CMD_ERROR);
            }
            break;

        case AT_CMD_START_SEND_DATA:
            ptr = strstr((char *)uart0RxData.buff, ">");
            if (ptr != NULL) {
                Gsm_event_post(GSM_EVT_CMD_OK);
            }
            ptr = strstr((char *)uart0RxData.buff, "ERROR");
            if (ptr != NULL) {
                Gsm_event_post(GSM_EVT_CMD_ERROR);
                break;
            }
            break;

        case AT_CMD_SEND_DATA:
            ptr = strstr((char *)uart0RxData.buff, "SEND OK");
            if (ptr != NULL) {
                Gsm_event_post(GSM_EVT_CMD_OK);
                break;
            }
            ptr = strstr((char *)uart0RxData.buff, "SEND FAIL");
            if (ptr != NULL) {
                Gsm_event_post(GSM_EVT_CMD_ERROR);
            }
            break;

        case AT_CMD_ACK_QUERY:
            ptr = strstr((char *)uart0RxData.buff, "OK");
            if (ptr != NULL) {
                ptr = strstr((char *)uart0RxData.buff, "+QISACK:");
                if (ptr != NULL) {
                    //find last position
                    ptr = strrchr((char *)uart0RxData.buff, ',');
                    if (ptr != NULL) {
                        value = atoi(ptr + 1);
                        if (value == 0)
                            Gsm_event_post(GSM_EVT_CMD_OK);
                        else
                            Gsm_event_post(GSM_EVT_CMD_ERROR);
                    }
                }
            }
            break;

        case AT_CMD_CLOSE_CONNECT:
            ptr = strstr((char *)uart0RxData.buff, "CLOSE OK");
            if (ptr != NULL) {
                Gsm_event_post(GSM_EVT_CMD_OK);
                break;
            }
            ptr = strstr((char *)uart0RxData.buff, "ERROR");
            if (ptr != NULL) {
                Gsm_event_post(GSM_EVT_CMD_ERROR);
            }
            break;

        case AT_CMD_GET_LOCATION:
            ptr = strstr((char *)uart0RxData.buff, "ERROR");
            if (ptr != NULL) {
                Gsm_event_post(GSM_EVT_CMD_ERROR);
                break;
            }
            ptr = strstr((char *)uart0RxData.buff, "OK");
            if (ptr != NULL) {
                ptr = strstr((char *)uart0RxData.buff, "+QCELLLOC:");
                if (ptr != NULL) {
                    rGsmObject.longitude = atof(ptr + 10);
                    //find last position
                    ptr = strrchr((char *)uart0RxData.buff, ',');
                    if (ptr != NULL) {
                        latitudetmp = atof(ptr + 1);
                        if(latitudetmp)    
                            rGsmObject.latitude = atof(ptr + 1);
                        else {
                            rGsmObject.longitude = 0;
                            HIBYTE(HIWORD(rGsmObject.latitude)) = *(ptr + 1);
                            LOBYTE(HIWORD(rGsmObject.latitude)) = *(ptr + 2);
                            HIBYTE(LOWORD(rGsmObject.latitude)) = *(ptr + 3);        
                            LOBYTE(LOWORD(rGsmObject.latitude)) = *(ptr + 4);
                        }
                        Gsm_event_post(GSM_EVT_CMD_OK);
                    }
                }
            }
            break;

        default:
            break;
    }
}

//***********************************************************************************
//
// Gsm hwi isr callback function.
// note: call back in uart isr
//***********************************************************************************
static void Gsm_hwiIntCallback(uint8_t *dataP, uint8_t len)
{
    uint8_t datapLen;
    
    datapLen        = 0;
    while(len)
    {
        len--;
        uart0IsrRxData.buff[uart0IsrRxData.length] = dataP[datapLen];
        datapLen++;
        uart0IsrRxData.length++;

        if(uart0IsrRxData.length < 2)
            continue;

        if(Clock_isActive(gsmTimeOutClockHandle))
        {
            Clock_stop(gsmTimeOutClockHandle);
            Clock_setTimeout(gsmTimeOutClockHandle, GSM_TIMEOUT_MS * CLOCK_UNIT_MS);
            Clock_start(gsmTimeOutClockHandle);
        }

        if(((uart0IsrRxData.buff[uart0IsrRxData.length - 1] == '\n') && 
            (uart0IsrRxData.buff[uart0IsrRxData.length - 2] == '\r'))
            || (uart0IsrRxData.buff[uart0IsrRxData.length - 1] == '>')
            || (uart0IsrRxData.buff[uart0IsrRxData.length - 1] == 0x7e))
        {

            if(Clock_isActive(gsmTimeOutClockHandle))
            {
                Clock_stop(gsmTimeOutClockHandle);
            }
            Clock_setTimeout(gsmTimeOutClockHandle, GSM_TIMEOUT_MS * CLOCK_UNIT_MS);
            Clock_start(gsmTimeOutClockHandle);
        }


        if(uart0IsrRxData.length >= UART_BUFF_SIZE)
        {
            if(Clock_isActive(gsmTimeOutClockHandle))
            {
                Clock_stop(gsmTimeOutClockHandle);
            }
            Clock_setTimeout(gsmTimeOutClockHandle, GSM_TIMEOUT_MS * CLOCK_UNIT_MS);
            Clock_start(gsmTimeOutClockHandle);
        }
    }
}

//***********************************************************************************
//
// Gsm event post.
//
//***********************************************************************************
static void Gsm_event_post(UInt event)
{
    Event_post(gsmEvtHandle, event);
}


//***********************************************************************************
//
// Gsm receive timeout cb
//
//***********************************************************************************
static void GsmRxToutCb(UArg arg0)
{
    UInt key;
    // uint8_t i;
    
    /* Disable preemption. */
    key = Hwi_disable();

    memcpy(uart0RxData.buff, uart0IsrRxData.buff, uart0IsrRxData.length);
    // for (i = 0; i < uart0IsrRxData.length; ++i)
    // {
    //     System_printf("%c", uart0IsrRxData.buff[i]);
    // }
    // System_printf("end\n");

    uart0RxData.length    = uart0IsrRxData.length;
    uart0IsrRxData.length = 0;

    Hwi_restore(key);

    Swi_post(gsmRxSwiHandle);
}



//***********************************************************************************
//
// Gsm module init.
//
//***********************************************************************************
static void Gsm_init(Nwk_Params *params)
{
    Gsm_io_init();

    //Init UART.
    // UartHwInit(UART_0, 38400, Gsm_hwiIntCallback);

    Clock_Params clkParams;
    Clock_Params_init(&clkParams);
    clkParams.period = 0;
    clkParams.startFlag = FALSE;
    Clock_construct(&gsmTimeOutClock, GsmRxToutCb, 1, &clkParams);
    gsmTimeOutClockHandle = Clock_handle(&gsmTimeOutClock);

    rGsmObject.isOpen = 0;
    rGsmObject.actPDPCnt = 0;
    rGsmObject.uploadFailCnt = 0;
    rGsmObject.sleep = 0;
    rGsmObject.resetCnt = 0;
    rGsmObject.state = GSM_STATE_POWEROFF;

    Event_Params eventParam;
    Event_Params_init(&eventParam);
    /* Construct key process Event */
    Event_construct(&gsmEvtStruct, &eventParam);
    /* Obtain event instance handle */
    gsmEvtHandle = Event_handle(&gsmEvtStruct);

    /* Construct a swi Instance to monitor gsm uart */
    Swi_Params swiParams;
    Swi_Params_init(&swiParams);
    swiParams.arg0 = 1;
    swiParams.arg1 = 0;
    swiParams.priority = 1;
    swiParams.trigger = 0;
    Swi_construct(&gsmRxSwiStruct, (Swi_FuncPtr)Gsm_rxSwiFxn, &swiParams, NULL);
    gsmRxSwiHandle = Swi_handle(&gsmRxSwiStruct);

    rGsmObject.dataProcCallbackFxn = params->dataProcCallbackFxn;
}

//***********************************************************************************
//
// Gsm module open.
//
//***********************************************************************************
static uint8_t Gsm_open(void)
{
    if (rGsmObject.isOpen) {
        return FALSE;
    }

    Gsm_poweron();
    rGsmObject.isOpen = 1;
    return TRUE;
}

//***********************************************************************************
//
// Gsm module close.
//
//***********************************************************************************
static uint8_t Gsm_close(void)
{
    if (rGsmObject.isOpen == 0) {
        return FALSE;
    }

    Gsm_poweroff();
    rGsmObject.isOpen = 0;
    return TRUE;
}

//***********************************************************************************
//
// Gsm module control.
//
//***********************************************************************************
static uint8_t Gsm_control(uint8_t cmd, void *arg)
{
    uint8_t result;

    if (rGsmObject.isOpen == 0) {
        return FALSE;
    }

    switch (cmd) {
        case NWK_CONTROL_RSSI_GET:
            *(uint8_t *)arg = rGsmObject.rssi;
            break;

        case NWK_CONTROL_RSSI_QUERY:
            if (rGsmObject.state > GSM_STATE_GPRS_QUERY && rGsmObject.state <= GSM_STATE_TCP_UPLOAD) {
                result = Gsm_query_csq();
                if (result != RESULT_OK) {
                    rGsmObject.rssi = 0;
                    return FALSE;
                }
            }
            break;

        case NWK_CONTROL_SIMCCID_GET:
            memcpy((char *)arg, (char *)rGsmObject.simCcid, 20);
            break;

        case NWK_CONTROL_WAKEUP:
            if (rGsmObject.sleep) {
                result = Gsm_wakeup();
                if (result == RESULT_SHUTDOWN) {
                    return FALSE;
                } else if (result == RESULT_RESET) {
                    Gsm_reset();
                    return FALSE;
                }
                rGsmObject.sleep = 0;
            }
            break;

        case NWK_CONTROL_SLEEP:
            if (rGsmObject.sleep == 0) {
                result = Gsm_sleep();
                if (result == RESULT_SHUTDOWN) {
                    return FALSE;
                } else if (result == RESULT_RESET) {
                    Gsm_reset();
                    return FALSE;
                }
                rGsmObject.sleep = 1;
            }
            break;

        case NWK_CONTROL_LBS_QUERY:
            if (rGsmObject.state == GSM_STATE_TCP_UPLOAD) {
                result = Gsm_get_lbs();
                if (result != RESULT_OK) {
                    ((NwkLocation_t *)arg)->longitude = 360;
                    ((NwkLocation_t *)arg)->latitude = 360;
                    return FALSE;
                }

                ((NwkLocation_t *)arg)->longitude = rGsmObject.longitude;
                ((NwkLocation_t *)arg)->latitude = rGsmObject.latitude;
            }
            break;

        case NWK_CONTROL_TRANSMIT:
            result = Gsm_transmit_process(((NwkMsgPacket_t *)arg)->buff, ((NwkMsgPacket_t *)arg)->length);
            if (result == RESULT_SHUTDOWN) {
                return FALSE;
            } else if (result == RESULT_RESET) {
                Gsm_reset();
                return FALSE;
            }
            break;

        case NWK_CONTROL_SHUTDOWN_MSG:
            Gsm_event_post(GSM_EVT_SHUTDOWN);
            break;

        case NWK_CONTROL_TEST:
            result = Gsm_test();
            if (result == RESULT_SHUTDOWN) {
                return FALSE;
            } else if (result == RESULT_RESET) {
                Gsm_reset();
                return FALSE;
            }
            break;
    }

    return TRUE;
}

//***********************************************************************************
//
// Gsm error indicate.
//
//***********************************************************************************
static void Gsm_error_indicate(void)
{
    switch (rGsmObject.error) {
        case GSM_ERR_UART_SYNC:
            Led_ctrl(LED_R, 1, 250* CLOCK_UNIT_MS, 1);
            break;

        case GSM_ERR_SIM_QUERY:
            Led_ctrl(LED_R, 1, 250* CLOCK_UNIT_MS, 2);
            break;

        case GSM_ERR_CSQ_QUERY:
        case GSM_ERR_CREG_QUERY:
        case GSM_ERR_CGREG_QUERY:
            Led_ctrl(LED_R, 1, 250* CLOCK_UNIT_MS, 3);
            break;

        case GSM_ERR_ACT:
            Led_ctrl(LED_R, 1, 250* CLOCK_UNIT_MS, 4);
            break;

        case GSM_ERR_UPLOAD:
            Led_ctrl(LED_R, 1, 250* CLOCK_UNIT_MS, 5);
            break;

        case GSM_ERR_SLEEP_MODE:
        case GSM_ERR_WAKEUP_MODE:
            Led_ctrl(LED_R, 1, 250* CLOCK_UNIT_MS, 6);
            break;
    }
}


#endif  /* SUPPORT_GSM */

