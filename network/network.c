//***********************************************************************************
// Copyright 2017, Zksiot Development Ltd.
// Created by Linjie, 2017.08.08
// MCU:	MSP430F5529
// OS: TI-RTOS
// Project:
// File name: network.c
// Description: network process routine.
//***********************************************************************************
#include "../general.h"

#ifdef SUPPORT_NETWORK

typedef enum {
//Send MSG
    NMI_TX_COM_ACK  = 0x2001,
    NMI_TX_HEARTBEAT= 0x2002,
    NMI_TX_SETTING  = 0x2003,
    NMI_TX_NTP      = 0x2004,
    
    NMI_TX_GETIP_ACK= 0x2006,

    NMI_TX_SENSOR   = 0x2010,
    NMI_TX_GPS 	    = 0x2020,
    NMI_TX_LBS 	    = 0x2021,

//Receive MSG
    NMI_RX_COM_ACK  = 0xA001,
    NMI_RX_SETTING  = 0xA002,
    NMI_RX_ALARM    = 0xA003,
    NMI_RX_NTP      = 0xA004,
    NMI_RX_SETIP    = 0xA005,
    NMI_RX_GETIP    = 0xA006,
    NMI_RX_RESET    = 0xA007,
} NWK_MSG_ID;

#define NWK_MSG_BODY_START  12

typedef enum {
	PTI_COLLECT_PERIOD = 0x01,
	PTI_UPLOAD_PERIOD ,
	PTI_HIGHTEMP_ALARM,
	PTI_LOWTEMP_ALARM,		
	PTI_SENSOR_CODEC,		
}PARA_TYPE_ID;

typedef enum {
//COM_ACK
    TCA_OK = 0,
    TCA_FAIL = 1,
    TCA_MSG_ERROR = 2,
    TCA_NOT_SUPPORT = 3,
}TX_COM_ACK_RESULT;

typedef struct {
	uint8_t uuid[2];
    uint16_t serialNum;
    uint8_t poweron;
    uint8_t ntp;
    uint8_t moduleIndex;
    uint16_t hbTime;
    uint32_t uploadTime;
    uint32_t ntpTime;
//location
    NwkLocation_t location;
} NwkObject_t;

#define NWK_EVT_NONE                Event_Id_NONE
#define NWK_EVT_POWERON             Event_Id_00
#define NWK_EVT_SHUTDOWN            Event_Id_01
#define NWK_EVT_DATA_UPLOAD         Event_Id_02
#define NWK_EVT_HEARTBEAT           Event_Id_03
#define NWK_EVT_TEST                Event_Id_04
#define NWK_EVT_ACK                 Event_Id_05


#define NWK_EVT_ALL                 0xffff


#define NWKTASKSTACKSIZE           768

static Task_Struct nwkTaskStruct;
static uint8_t nwkTaskStack[NWKTASKSTACKSIZE];

static Event_Struct nwkEvtStruct;
static Event_Handle nwkEvtHandle;

NwkObject_t rNwkObject;
NwkMsgPacket_t rNwkMsgPacket;

typedef enum {
#ifdef SUPPORT_GSM
    NWK_MODULE_GSM,
#endif
    NWK_MODULE_MAX
} NWK_MODULE;

extern const Nwk_FxnTable Gsm_FxnTable;
static const Nwk_FxnTable *Nwk_FxnTablePtr[NWK_MODULE_MAX] = {
#ifdef SUPPORT_GSM
    &Gsm_FxnTable,
#endif
};

//***********************************************************************************
//
// Network protocol group package.
//
//***********************************************************************************
static void Nwk_group_package(NWK_MSG_ID msgId, NwkMsgPacket_t *pPacket)
{
    uint8_t i;
    uint16_t value = 0;
    uint32_t temp;
    NwkMsgPacket_t packet;

    packet.length = 0;
    //消息头
    //消息ID
    packet.buff[packet.length++] = HIBYTE(msgId);
    packet.buff[packet.length++] = LOBYTE(msgId);
    //消息体属性
    packet.buff[packet.length++] = 0;
    packet.buff[packet.length++] = 0;
    //Gateway ID
    for (i = 0; i < 4; i++) {
        packet.buff[packet.length++] = g_rSysConfigInfo.DeviceId[i];
    }
    //UUID
    packet.buff[packet.length++] = rNwkObject.uuid[0];
    packet.buff[packet.length++] = rNwkObject.uuid[1];
    //消息流水号
    packet.buff[packet.length++] = HIBYTE(rNwkObject.serialNum);
    packet.buff[packet.length++] = LOBYTE(rNwkObject.serialNum);
    rNwkObject.serialNum++;

    if (msgId == NMI_TX_SENSOR) {
        //消息体
        //固定字段类型
        packet.buff[packet.length++] = 0x01;
        //终端电压
#ifdef SUPPORT_BATTERY
        value = AONBatMonBatteryVoltageGet();
        value = ((value&0xff00)>>8)*1000 +1000*(value&0xff)/256;
#endif
        packet.buff[packet.length++] = HIBYTE(value);
        packet.buff[packet.length++] = LOBYTE(value);
        //终端网络信号
        Nwk_FxnTablePtr[rNwkObject.moduleIndex]->controlFxn(NWK_CONTROL_RSSI_GET, &packet.buff[packet.length++]);
        //无线信号强度RSSI
        packet.buff[packet.length++] = pPacket->buff[1];
        //Sensor ID - 4Byte
        //采集流水号 - 2Byte
        //采集时间
        //Sensor电压
        //参数项列表数据
        for (i = 0; i < pPacket->buff[0] - 3; i++) {//sensor data: length(1B) rssi(1B) customid(2B) devicedi(4B)...
            packet.buff[packet.length++] = pPacket->buff[4 + i];
        }
    } else if (msgId == NMI_TX_LBS) {
        temp = rNwkObject.location.longitude * 100000;
        packet.buff[packet.length++] = HIBYTE(HIWORD(temp));
        packet.buff[packet.length++] = LOBYTE(HIWORD(temp));
        packet.buff[packet.length++] = HIBYTE(LOWORD(temp));
        packet.buff[packet.length++] = LOBYTE(LOWORD(temp));
        temp = rNwkObject.location.latitude * 100000;
        packet.buff[packet.length++] = HIBYTE(HIWORD(temp));
        packet.buff[packet.length++] = LOBYTE(HIWORD(temp));
        packet.buff[packet.length++] = HIBYTE(LOWORD(temp));
        packet.buff[packet.length++] = LOBYTE(LOWORD(temp));
    } else if (msgId == NMI_TX_COM_ACK) {
        memcpy(&packet.buff[packet.length],pPacket->buff, 5);
        packet.length += 5;
    }

    //消息体属性
    value = packet.length - 12;
    packet.buff[2] = HIBYTE(value);
    packet.buff[3] = LOBYTE(value);

    //校验码
    packet.buff[packet.length++] = CheckCode8(&packet.buff[0], packet.length);

    //进行转义
    pPacket->length = Protocol_escape(&pPacket->buff[1], &packet.buff[0], packet.length);
    //消息标志位
    pPacket->buff[0] = PROTOCOL_TOKEN;
    pPacket->buff[pPacket->length + 1] = PROTOCOL_TOKEN;
    pPacket->length += 2;

}

//***********************************************************************************
//
// Network event post.
//
//***********************************************************************************
static void Nwk_event_post(UInt event)
{
    Event_post(nwkEvtHandle, event);
}

static uint8_t* mystrchar(uint8_t *buf, uint8_t c, uint16_t len)
{
    uint16_t i;
    
    for ( i = 0; i < len; i++) {
        if(buf[i] == c)
            return &buf[i];
    }
    return NULL;
}

//***********************************************************************************
//
// Network receive data process callback function.
//
//***********************************************************************************
static void Nwk_data_proc_callback(uint8_t *pBuff, uint16_t length)
{
    uint16_t msgId, index;
    Calendar calendar;
    uint8_t rxData[128];
	uint8_t  paratype;
	uint8_t *ptrstart,*ptrend;
    uint16_t package_length;
    //pBuff maybe include more one data package
    ptrstart = pBuff;
    while(1) {
        ptrstart = mystrchar(ptrstart, 0x7e, length);//first 0x7e

        if (ptrstart == NULL)
            return;

        ptrend = mystrchar(ptrstart+1, 0x7e, length-1);//second 0x7e
        if (ptrend == NULL)
            return;
          
        package_length = ptrend - ptrstart + 1;
            
        memcpy((char *)rxData, (char *)ptrstart, package_length);

       ptrstart = ptrend + 1;
       length -= package_length;

        //Recover transferred meaning
       package_length = Protocol_recover_escape(&rxData[0], &rxData[1], package_length - 2);
        if (package_length <= 1)
            break;

        //package check code
        if (rxData[package_length - 1] != CheckCode8(rxData, package_length - 1))
            break;

    // The data is valid.
    	package_length -= 1;

        msgId = (rxData[0] << 8) | rxData[1];
        switch (msgId) {
            case NMI_RX_COM_ACK:
                break;

            case NMI_RX_SETTING:
    			index = NWK_MSG_BODY_START;
    			while(index < package_length) {
    				paratype = rxData[index++];			
    				switch (paratype) {			
    				case PTI_COLLECT_PERIOD:
    					HIBYTE(HIWORD(g_rSysConfigInfo.collectPeriod)) = rxData[index++];
    					LOBYTE(HIWORD(g_rSysConfigInfo.collectPeriod)) = rxData[index++];
                        HIBYTE(LOWORD(g_rSysConfigInfo.collectPeriod)) = rxData[index++];
                        LOBYTE(LOWORD(g_rSysConfigInfo.collectPeriod)) = rxData[index++];
    					break;
    						
    				case PTI_UPLOAD_PERIOD:
    					HIBYTE(HIWORD(g_rSysConfigInfo.uploadPeriod)) = rxData[index++];
    					LOBYTE(HIWORD(g_rSysConfigInfo.uploadPeriod)) = rxData[index++];
                        HIBYTE(LOWORD(g_rSysConfigInfo.uploadPeriod)) = rxData[index++];
                        LOBYTE(LOWORD(g_rSysConfigInfo.uploadPeriod)) = rxData[index++];
    					break;
    					
    				case PTI_HIGHTEMP_ALARM:
    					g_rSysConfigInfo.alarmTemp[rxData[index]].high = (uint16_t)rxData[index+1]<<8 | (uint16_t)rxData[index+2];
    					index += 3;	
    					break;
    					
    				case PTI_LOWTEMP_ALARM:			
    					g_rSysConfigInfo.alarmTemp[rxData[index]].low = (uint16_t)rxData[index+1]<<8 | (uint16_t)rxData[index+2];
    					index += 3;	
    					break;
                    case PTI_SENSOR_CODEC:
                        
                        break;
    				}	
    			}
                
#ifdef FLASH_INTERNAL
                InternalFlashStoreConfig();
#endif
                //send ack to server
                rNwkMsgPacket.buff[0] = rxData[10];
                rNwkMsgPacket.buff[1] = rxData[11];
                rNwkMsgPacket.buff[2] = rxData[0];
                rNwkMsgPacket.buff[3] = rxData[1];
                rNwkMsgPacket.buff[4] = TCA_OK;
                Nwk_group_package(NMI_TX_COM_ACK, &rNwkMsgPacket);
                Nwk_event_post(NWK_EVT_ACK);
               // Nwk_FxnTablePtr[rNwkObject.moduleIndex]->controlFxn(NWK_CONTROL_TRANSMIT, &rNwkMsgPacket);
                    
                break;
            case NMI_RX_ALARM:
                /*
                index = NWK_MSG_BODY_START;
                HIBYTE(HIWORD(g_AlarmSensor.DeviceId)) = rxData[index++];
                LOBYTE(HIWORD(g_AlarmSensor.DeviceId)) = rxData[index++];
                HIBYTE(LOWORD(g_AlarmSensor.DeviceId)) = rxData[index++];
                LOBYTE(LOWORD(g_AlarmSensor.DeviceId)) = rxData[index++];
                HIBYTE(g_AlarmSensor.no) = rxData[index++];
                LOBYTE(g_AlarmSensor.no) = rxData[index++];  
                g_AlarmSensor.index      = rxData[index++];  
                g_AlarmSensor.type       = rxData[index++];
                if (!(g_AlarmSensor.type > SEN_TYPE_NONE && g_AlarmSensor.type < SEN_TYPE_MAX))
                    return;//invalid sensor type
                
                        
                if (Sensor_get_function_by_type(g_AlarmSensor.type) == (SENSOR_TEMP | SENSOR_HUMI)) {
                    HIBYTE(g_AlarmSensor.value.temp) = rxData[index++];
                    LOBYTE(g_AlarmSensor.value.temp) = rxData[index++];
                    HIBYTE(g_AlarmSensor.value.humi) = rxData[index++];
                    LOBYTE(g_AlarmSensor.value.humi) = rxData[index++];  
                }else if (Sensor_get_function_by_type(g_AlarmSensor.type) == (SENSOR_DEEP_TEMP)) {
                    HIBYTE(HIWORD(g_AlarmSensor.value.tempdeep)) = rxData[index++];
                    LOBYTE(HIWORD(g_AlarmSensor.value.tempdeep)) = rxData[index++];
                    HIBYTE(LOWORD(g_AlarmSensor.value.tempdeep)) = rxData[index++];
                    g_AlarmSensor.value.tempdeep >>= 8;         
                }
                else {
                    HIBYTE(g_AlarmSensor.value.temp) = rxData[index++];
                    LOBYTE(g_AlarmSensor.value.temp) = rxData[index++];
                }  

                g_bAlarmSensorFlag = 0x100;
                Sys_event_post(SYS_EVT_ALARM);
*/
                //send ack to server
                rNwkMsgPacket.buff[0] = rxData[10];
                rNwkMsgPacket.buff[1] = rxData[11];
                rNwkMsgPacket.buff[2] = rxData[0];
                rNwkMsgPacket.buff[3] = rxData[1];
                rNwkMsgPacket.buff[4] = TCA_OK;
                Nwk_group_package(NMI_TX_COM_ACK, &rNwkMsgPacket);
                Nwk_event_post(NWK_EVT_ACK);
                
                break;

            case NMI_RX_NTP:
    			if (package_length >= 18) {
                    index = NWK_MSG_BODY_START;
                    calendar.year  = rxData[index++] + CALENDAR_BASE_YEAR;
                    calendar.month = rxData[index++];
                    calendar.day   = rxData[index++];
                    calendar.hour  = rxData[index++];
                    calendar.min   = rxData[index++];
                    calendar.sec   = rxData[index++];
    	            Rtc_set_calendar(&calendar);
    	            rNwkObject.ntp = 1;
    			}
                break;
        }
    }
}

//***********************************************************************************
//
// Network init.
//
//***********************************************************************************
static void Nwk_init(void)
{
    uint8_t i;
    Nwk_Params params;

    params.dataProcCallbackFxn = Nwk_data_proc_callback;
    for (i = 0; i < NWK_MODULE_MAX; i++) {
        Nwk_FxnTablePtr[i]->initFxn(&params);
    }

    rNwkObject.poweron = 0;
    rNwkObject.ntp = 0;
    rNwkObject.hbTime = 0;
    rNwkObject.uploadTime = 0;
    rNwkObject.ntpTime = 0;
}

//***********************************************************************************
//
// Network event process.
//
//***********************************************************************************
static void Nwk_taskFxn(void)
{
    UInt eventId;

    /* Construct key process Event */
    Event_construct(&nwkEvtStruct, NULL);
    /* Obtain event instance handle */
    nwkEvtHandle = Event_handle(&nwkEvtStruct);

    Nwk_init();

    while (1) {
        Led_ctrl(LED_R, 0, 0, 0);
        Led_ctrl(LED_G, 0, 0, 0);
        Led_ctrl(LED_B, 0, 0, 0);
        eventId = Event_pend(nwkEvtHandle, 0, NWK_EVT_ALL, BIOS_WAIT_FOREVER);

        if (eventId & NWK_EVT_POWERON) {
            if (g_rSysConfigInfo.module & MODULE_GSM) {
                rNwkObject.moduleIndex = NWK_MODULE_GSM;
            } else {
                //No network module
                continue;
            }
                
            rNwkObject.ntp = 0;
            rNwkObject.poweron = 1;
            Nwk_FxnTablePtr[rNwkObject.moduleIndex]->openFxn();
        } else if (eventId & NWK_EVT_SHUTDOWN) {
            if (g_rSysConfigInfo.module & MODULE_GSM) {
                rNwkObject.moduleIndex = NWK_MODULE_GSM;
            } else {
                //No network module
                continue;
            }
            rNwkObject.poweron = 0;
            Nwk_FxnTablePtr[rNwkObject.moduleIndex]->closeFxn();
        }

        if (rNwkObject.poweron == 0)
            continue;

        if (eventId & NWK_EVT_TEST) {
            if (Nwk_FxnTablePtr[rNwkObject.moduleIndex]->controlFxn(NWK_CONTROL_TEST, NULL) == FALSE) {
                Nwk_event_post(NWK_EVT_TEST);
                continue;
            }
            while (1) {
                Led_ctrl(LED_G, 1, 0, 0);
                eventId = Event_pend(nwkEvtHandle, 0, NWK_EVT_ALL, BIOS_WAIT_FOREVER);
                if ((eventId & NWK_EVT_SHUTDOWN) == NWK_EVT_SHUTDOWN) {
                    Nwk_event_post(NWK_EVT_SHUTDOWN);
                    break;
                }
            }
            continue;
        }

        Led_ctrl(LED_B, 1, 0, 0);
        if (rNwkObject.ntp == 0) {
            Nwk_group_package(NMI_TX_NTP, &rNwkMsgPacket);
            Nwk_FxnTablePtr[rNwkObject.moduleIndex]->controlFxn(NWK_CONTROL_TRANSMIT, &rNwkMsgPacket);
            continue;
        }

        if (eventId & (NWK_EVT_DATA_UPLOAD | NWK_EVT_HEARTBEAT | NWK_EVT_ACK)) {
            //wakeup.
            if (Nwk_FxnTablePtr[rNwkObject.moduleIndex]->controlFxn(NWK_CONTROL_WAKEUP, NULL) == FALSE) {
                continue;
            }
            if (eventId & NWK_EVT_HEARTBEAT) {
                //send heartbeat data.
                Nwk_group_package(NMI_TX_HEARTBEAT, &rNwkMsgPacket);
                if (Nwk_FxnTablePtr[rNwkObject.moduleIndex]->controlFxn(NWK_CONTROL_TRANSMIT, &rNwkMsgPacket) == FALSE) {
                    continue;
                }
            } else if (eventId & NWK_EVT_ACK) {
                //send ack data.
                //Nwk_group_package(NMI_TX_COM_ACK, &rNwkMsgPacket);
                if (Nwk_FxnTablePtr[rNwkObject.moduleIndex]->controlFxn(NWK_CONTROL_TRANSMIT, &rNwkMsgPacket) == FALSE) {
                    continue;
                }
            } else {
                //NWK_EVT_DATA_UPLOAD
                uint8_t ret = TRUE;
                //query rssi.
                if (Nwk_FxnTablePtr[rNwkObject.moduleIndex]->controlFxn(NWK_CONTROL_RSSI_QUERY, NULL) == FALSE) {
                    continue;
                }
#ifdef FLASH_EXTERNAL
                //send data.
                while (Flash_load_sensor_data(rNwkMsgPacket.buff, FLASH_SENSOR_DATA_SIZE, 0) == ES_SUCCESS) {
                    Nwk_group_package(NMI_TX_SENSOR, &rNwkMsgPacket);
                    if (Nwk_FxnTablePtr[rNwkObject.moduleIndex]->controlFxn(NWK_CONTROL_TRANSMIT, &rNwkMsgPacket) == TRUE) {
                        Falsh_prtpoint_forward();
                    }
                    else
                    {
                        ret = FALSE;
                        break;
                    }
                }
#endif
                if (ret == TRUE) {
                    //query lbs.
                    if (Nwk_FxnTablePtr[rNwkObject.moduleIndex]->controlFxn(NWK_CONTROL_LBS_QUERY, &rNwkObject.location) == FALSE) {
                        continue;
                    }
                    //send lbs data.
                    Nwk_group_package(NMI_TX_LBS, &rNwkMsgPacket);
                    if (Nwk_FxnTablePtr[rNwkObject.moduleIndex]->controlFxn(NWK_CONTROL_TRANSMIT, &rNwkMsgPacket) == FALSE) {
                        continue;
                    }
                }
            }

            //sleep.
            if (Nwk_FxnTablePtr[rNwkObject.moduleIndex]->controlFxn(NWK_CONTROL_SLEEP, NULL) == FALSE) {
                continue;
            }
        }
    }
}

//***********************************************************************************
//
// Network task create.
//
//***********************************************************************************
void Nwk_task_create(void)
{
	Error_Block eb;
	Error_init(&eb);

    if (!(g_rSysConfigInfo.module & MODULE_NWK))
        return;

    /* Construct main system process Task threads */
    Task_Params taskParams;
    Task_Params_init(&taskParams);
    taskParams.stackSize = NWKTASKSTACKSIZE;
    taskParams.stack = &nwkTaskStack;
    taskParams.priority = 1;
    Task_construct(&nwkTaskStruct, (Task_FuncPtr)Nwk_taskFxn, &taskParams, &eb);
}

//***********************************************************************************
//
// Network data sent time counter isr.
//
//***********************************************************************************
void Nwk_upload_time_isr(void)
{
    if (!(g_rSysConfigInfo.module & MODULE_NWK))
        return;

    if (rNwkObject.poweron) {
        rNwkObject.uploadTime++;
        if (rNwkObject.uploadTime >= g_rSysConfigInfo.uploadPeriod) {
            rNwkObject.uploadTime = 0;
            rNwkObject.hbTime = 0;
            Nwk_event_post(NWK_EVT_DATA_UPLOAD);
        }
        rNwkObject.hbTime++;
        if (rNwkObject.hbTime >= g_rSysConfigInfo.hbPeriod) {
            rNwkObject.hbTime = 0;
            Nwk_event_post(NWK_EVT_HEARTBEAT);
        }
        rNwkObject.ntpTime++;
        if (rNwkObject.ntpTime >= g_rSysConfigInfo.ntpPeriod) {
            rNwkObject.ntpTime = 0;
            rNwkObject.ntp = 0;
        }
    }
}

//***********************************************************************************
//
// Network poweron.
//
//***********************************************************************************
void Nwk_poweron(void)
{
    if (!(g_rSysConfigInfo.module & MODULE_NWK))
        return;

//    if (g_rSysConfigInfo.status & STATUS_GSM_TEST)
//        Nwk_event_post(NWK_EVT_POWERON | NWK_EVT_TEST);
//    else
        Nwk_event_post(NWK_EVT_POWERON | NWK_EVT_DATA_UPLOAD);
}

//***********************************************************************************
//
// Network poweron.
//
//***********************************************************************************
void Nwk_poweroff(void)
{
    if (!(g_rSysConfigInfo.module & MODULE_NWK))
        return;

    Nwk_event_post(NWK_EVT_SHUTDOWN);
    Nwk_FxnTablePtr[rNwkObject.moduleIndex]->controlFxn(NWK_CONTROL_SHUTDOWN_MSG, NULL);
}

//***********************************************************************************
//
// Network get rssi.
//
//***********************************************************************************
uint8_t Nwk_get_rssi(void)
{
    uint8_t rssi;
    if (!(g_rSysConfigInfo.module & MODULE_NWK))
        return 0;

    Nwk_FxnTablePtr[rNwkObject.moduleIndex]->controlFxn(NWK_CONTROL_RSSI_GET, &rssi);
    return rssi;
}

//***********************************************************************************
//
// Network get rssi.
//
//***********************************************************************************
void Nwk_get_simccid(uint8_t *pBuff)
{
    if (!(g_rSysConfigInfo.module & MODULE_NWK))
        return;

    Nwk_FxnTablePtr[rNwkObject.moduleIndex]->controlFxn(NWK_CONTROL_SIMCCID_GET, pBuff);
}

#else
void Nwk_task_create(void)
{
}

void Nwk_upload_time_isr(void)
{
}

void Nwk_poweron(void)
{
}

void Nwk_poweroff(void)
{
}

uint8_t Nwk_get_rssi(void)
{
    return 0;
}

void Nwk_get_simccid(uint8_t *pBuff)
{
}


#endif  /* SUPPORT_NETWORK */

