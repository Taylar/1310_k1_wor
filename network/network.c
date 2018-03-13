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


#define NWKTASKSTACKSIZE           1024

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
void Sensor_store_null_package(uint8_t *buff)
{
    uint8_t i;
    uint16_t value = 0, length;
    Calendar calendar;

    //sensor data: length(1B) rssi(1B) customid(2B) devicedi(4B)...
    
    length = 0;
    //ÏûÏ¢Í·
    //ÏûÏ¢³¤¶È
    buff[length++] = 0;
    //ÎÞÏßÐÅºÅÇ¿¶ÈRSSI
    buff[length++] = 0;

    //Sensor ID
    for (i = 0; i < 4; i++)
        buff[length++] = g_rSysConfigInfo.DeviceId[i];
    //ÏûÏ¢Á÷Ë®ºÅ
    buff[length++] = 0;
    buff[length++] = 0;
    //²É¼¯Ê±¼ä
    calendar = Rtc_get_calendar();
    buff[length++] = calendar.Year - CALENDAR_BASE_YEAR;
    buff[length++] = calendar.Month;
    buff[length++] = calendar.DayOfMonth;
    buff[length++] = calendar.Hours;
    buff[length++] = calendar.Minutes;
    buff[length++] = calendar.Seconds;
    //SensorµçÑ¹
#ifdef SUPPORT_BATTERY
    Battery_voltage_measure();
    value = Battery_get_voltage();
#endif
    buff[length++] = HIBYTE(value);
    buff[length++] = LOBYTE(value);
    //²ÎÊýÏîÁÐ±íÊý¾Ý
    buff[0] = length - 1;
}


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
    //ÏûÏ¢Í·
    //ÏûÏ¢ID
    packet.buff[packet.length++] = HIBYTE(msgId);
    packet.buff[packet.length++] = LOBYTE(msgId);
    //ÏûÏ¢ÌåÊôÐÔ
    packet.buff[packet.length++] = 0;
    packet.buff[packet.length++] = 0;
    //Gateway ID
    for (i = 0; i < 4; i++) {
        packet.buff[packet.length++] = g_rSysConfigInfo.DeviceId[i];
    }
    //UUID
    packet.buff[packet.length++] = rNwkObject.uuid[0];
    packet.buff[packet.length++] = rNwkObject.uuid[1];
    //ÏûÏ¢Á÷Ë®ºÅ
    packet.buff[packet.length++] = HIBYTE(rNwkObject.serialNum);
    packet.buff[packet.length++] = LOBYTE(rNwkObject.serialNum);
    rNwkObject.serialNum++;

    if (msgId == NMI_TX_SENSOR) {
        //ÏûÏ¢Ìå
        //¹Ì¶¨×Ö¶ÎÀàÐÍ
        packet.buff[packet.length++] = 0x01;
        //ÖÕ¶ËµçÑ¹
#ifdef SUPPORT_BATTERY
        Battery_voltage_measure();
        value = Battery_get_voltage();
#endif
        packet.buff[packet.length++] = HIBYTE(value);
        packet.buff[packet.length++] = LOBYTE(value);
        //ÖÕ¶ËÍøÂçÐÅºÅ
        Nwk_FxnTablePtr[rNwkObject.moduleIndex]->controlFxn(NWK_CONTROL_RSSI_GET, &packet.buff[packet.length++]);
        //ÎÞÏßÐÅºÅÇ¿¶ÈRSSI
        packet.buff[packet.length++] = pPacket->buff[1];
        //Sensor ID - 4Byte
        //²É¼¯Á÷Ë®ºÅ - 2Byte
        //²É¼¯Ê±¼ä
        //SensorµçÑ¹
        //²ÎÊýÏîÁÐ±íÊý¾Ý
        for (i = 0; i < pPacket->buff[0] - 1; i++) {//sensor data: length(1B) rssi(1B) devicedi(4B)...
            packet.buff[packet.length++] = pPacket->buff[2 + i];
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

    //ÏûÏ¢ÌåÊôÐÔ
    value = packet.length - 12;
    packet.buff[2] = HIBYTE(value);
    packet.buff[3] = LOBYTE(value);

    //Ð£ÑéÂë
    packet.buff[packet.length++] = CheckCode8(&packet.buff[0], packet.length);

    //½øÐÐ×ªÒå
    pPacket->length = Protocol_escape(&pPacket->buff[1], &packet.buff[0], packet.length);
    //ÏûÏ¢±êÖ¾Î»
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
    uint8_t  rxData[128];
	uint8_t  paratype;
	uint8_t *ptrstart,*ptrend;
    uint16_t package_length;
    bool     congfig = false,  sensorcodec = false;
#ifdef SUPPORT_NETGATE_DISP_NODE
    //uint32_t deviceid;
#endif
    
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
                        if((index + 4)<= package_length) {
        					HIBYTE(HIWORD(g_rSysConfigInfo.collectPeriod)) = rxData[index++];
        					LOBYTE(HIWORD(g_rSysConfigInfo.collectPeriod)) = rxData[index++];
                            HIBYTE(LOWORD(g_rSysConfigInfo.collectPeriod)) = rxData[index++];
                            LOBYTE(LOWORD(g_rSysConfigInfo.collectPeriod)) = rxData[index++];
                            congfig = true;
                        }
    					break;
    						
    				case PTI_UPLOAD_PERIOD:
                        if((index + 4)<= package_length) {
        					HIBYTE(HIWORD(g_rSysConfigInfo.uploadPeriod)) = rxData[index++];
        					LOBYTE(HIWORD(g_rSysConfigInfo.uploadPeriod)) = rxData[index++];
                            HIBYTE(LOWORD(g_rSysConfigInfo.uploadPeriod)) = rxData[index++];
                            LOBYTE(LOWORD(g_rSysConfigInfo.uploadPeriod)) = rxData[index++];
                            congfig = true;
                        }
    					break;
    					
    				case PTI_HIGHTEMP_ALARM:
                        if((index + 3)<= package_length) {
        					g_rSysConfigInfo.alarmTemp[rxData[index]].high = (uint16_t)rxData[index+1]<<8 | (uint16_t)rxData[index+2];
        					index += 3;	
                            congfig = true;
                        }
    					break;
    					
    				case PTI_LOWTEMP_ALARM:		
                        if((index + 3)<= package_length) {
        					g_rSysConfigInfo.alarmTemp[rxData[index]].low = (uint16_t)rxData[index+1]<<8 | (uint16_t)rxData[index+2];
        					index += 3;	
                            congfig = true;
                        }
    					break;
                        
#ifdef SUPPORT_NETGATE_DISP_NODE
                    case PTI_SENSOR_CODEC:
                        /*
                        if((index + 6)<= package_length) {   
                            HIBYTE(HIWORD(deviceid)) = rxData[index + 0];
                            LOBYTE(HIWORD(deviceid)) = rxData[index + 1];
                            HIBYTE(LOWORD(deviceid)) = rxData[index + 2];
                            LOBYTE(LOWORD(deviceid)) = rxData[index + 3];
                            index += (5 + rxData[4]);

                            if(rxData[4] == 1) {// is sensor  codec
                               // Flash_store_sensor_codec(rxData[5], deviceid);
                                sensorcodec = true;
                            }
                            else {// is sensor name                       //support later.
                               
                            }
                            
                        }*/
                        break;
#endif                        
    				}	
    			}
                
#ifdef FLASH_INTERNAL
                ConcenterStoreConfig();
#endif
                if(congfig || sensorcodec) {
                    
                    //send ack to server
                    rNwkMsgPacket.buff[0] = rxData[10];
                    rNwkMsgPacket.buff[1] = rxData[11];
                    rNwkMsgPacket.buff[2] = rxData[0];
                    rNwkMsgPacket.buff[3] = rxData[1];
                    rNwkMsgPacket.buff[4] = TCA_OK;
                    Nwk_group_package(NMI_TX_COM_ACK, &rNwkMsgPacket);
                    Nwk_event_post(NWK_EVT_ACK);
                }
                    
                break;
                
#ifdef SUPPORT_NETGATE_DISP_NODE                
            case NMI_RX_ALARM:
                /*
                index = NWK_MSG_BODY_START;
                if((index + 10) <= package_length) {//¿ÉÄÜ°üº¬¶àÌõ±¨¾¯Êý¾Ý£¬Ä¿Ç°Ö»´¦Àí1Ìõ
                    HIBYTE(HIWORD(g_AlarmSensor.DeviceId)) = rxData[index++];
                    LOBYTE(HIWORD(g_AlarmSensor.DeviceId)) = rxData[index++];
                    HIBYTE(LOWORD(g_AlarmSensor.DeviceId)) = rxData[index++];
                    LOBYTE(LOWORD(g_AlarmSensor.DeviceId)) = rxData[index++];
                    g_AlarmSensor.index      = rxData[index++];
                    g_AlarmSensor.type       = rxData[index++];
                    if (!(g_AlarmSensor.type > SENSOR_DATA_NONE && g_AlarmSensor.type < SENSOR_DATA_MAX))
                        break;//invalid sensor type

                    //all  data  saved to tempdeep
                    HIBYTE(HIWORD(g_AlarmSensor.value.tempdeep)) = rxData[index++];
                    LOBYTE(HIWORD(g_AlarmSensor.value.tempdeep)) = rxData[index++];
                    HIBYTE(LOWORD(g_AlarmSensor.value.tempdeep)) = rxData[index++];
                    LOBYTE(LOWORD(g_AlarmSensor.value.tempdeep)) = rxData[index++];                      

                    g_bAlarmSensorFlag = 0x100;
                    Sys_event_post(SYS_EVT_ALARM);

                    //send ack to server
                    rNwkMsgPacket.buff[0] = rxData[10];
                    rNwkMsgPacket.buff[1] = rxData[11];
                    rNwkMsgPacket.buff[2] = rxData[0];
                    rNwkMsgPacket.buff[3] = rxData[1];
                    rNwkMsgPacket.buff[4] = TCA_OK;
                    Nwk_group_package(NMI_TX_COM_ACK, &rNwkMsgPacket);
                    Nwk_event_post(NWK_EVT_ACK);
                }      */
                break;
#endif
            case NMI_RX_NTP:
                index = NWK_MSG_BODY_START;
                if((index + 6) <= package_length) {
                    calendar.Year       = TransBcdToHex(rxData[index++]) + CALENDAR_BASE_YEAR;
                    calendar.Month      = TransBcdToHex(rxData[index++]);
                    calendar.DayOfMonth = TransBcdToHex(rxData[index++]);
                    calendar.Hours      = TransBcdToHex(rxData[index++]);
                    calendar.Minutes    = TransBcdToHex(rxData[index++]);
                    calendar.Seconds    = TransBcdToHex(rxData[index++]);
    	            ConcenterTimeSychronization(&calendar);
    	            rNwkObject.ntp = 1;
                    ConcenterCollectStart();
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
    bool bsensordata;

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

#ifdef SUPPORT_GSM_SHORT_CONNECT
        Nwk_FxnTablePtr[rNwkObject.moduleIndex]->openFxn();
#endif

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
                //uint8_t ret = TRUE;
                //query rssi.
                if (Nwk_FxnTablePtr[rNwkObject.moduleIndex]->controlFxn(NWK_CONTROL_RSSI_QUERY, NULL) == FALSE) {
                    continue;
                }


                //query lbs.
                if (Nwk_FxnTablePtr[rNwkObject.moduleIndex]->controlFxn(NWK_CONTROL_LBS_QUERY, &rNwkObject.location) == FALSE) {
                    continue;
                }
                
                //send lbs data first
                Nwk_group_package(NMI_TX_LBS, &rNwkMsgPacket);
                if (Nwk_FxnTablePtr[rNwkObject.moduleIndex]->controlFxn(NWK_CONTROL_TRANSMIT, &rNwkMsgPacket) == FALSE) {
                    continue;
                }
                
#ifdef FLASH_EXTERNAL
                //send data second.
                bsensordata = 0;
                while (Flash_load_sensor_data(rNwkMsgPacket.buff, FLASH_SENSOR_DATA_SIZE, 0) == ES_SUCCESS) {
                    bsensordata = 1;
                    Nwk_group_package(NMI_TX_SENSOR, &rNwkMsgPacket);
                    if (Nwk_FxnTablePtr[rNwkObject.moduleIndex]->controlFxn(NWK_CONTROL_TRANSMIT, &rNwkMsgPacket) == TRUE) {
                        Falsh_prtpoint_forward();
                    }
                    else
                    {
                        // ret = FALSE;
                        break;
                    }
                }
#endif             
                //µ±Íø¹ØÃ»ÓÐsensorÊý¾ÝÊ±ÉÏ´«Ò»¸öÎÞÐ§sensorÊý¾ÝÒÔ±ÜÃâÍø¹ØÐÅÏ¢Ê§Áª¡£
                if(!bsensordata){
                    Sensor_store_null_package(rNwkMsgPacket.buff);
                    Nwk_group_package(NMI_TX_SENSOR, &rNwkMsgPacket);
                    Nwk_FxnTablePtr[rNwkObject.moduleIndex]->controlFxn(NWK_CONTROL_TRANSMIT, &rNwkMsgPacket);
                }

            }
            
#ifdef SUPPORT_GSM_SHORT_CONNECT
            if(g_rSysConfigInfo.uploadPeriod >= 60*10) {//uploadPeriod>=10min ,shutdown 
                Nwk_FxnTablePtr[rNwkObject.moduleIndex]->closeFxn();
            }
            else
#endif
            {                
                //sleep.            
                if (Nwk_FxnTablePtr[rNwkObject.moduleIndex]->controlFxn(NWK_CONTROL_SLEEP, NULL) == FALSE) {
                    continue;
                }
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

    Event_Params eventParam;
    Event_Params_init(&eventParam);
    /* Construct key process Event */
    Event_construct(&nwkEvtStruct, &eventParam);
    /* Obtain event instance handle */
    nwkEvtHandle = Event_handle(&nwkEvtStruct);
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


void Nwk_upload_data(void)
{
    if (!(g_rSysConfigInfo.module & MODULE_NWK))
        return;
    Nwk_event_post(NWK_EVT_DATA_UPLOAD);
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


//***********************************************************************************
//
// get network power state.
//
//***********************************************************************************
uint8_t Nwk_get_state(void)
{
    if (!(g_rSysConfigInfo.module & MODULE_NWK))
        return false;
    return rNwkObject.poweron;
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

