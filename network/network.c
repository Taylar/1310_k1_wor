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

typedef enum {
	PTI_COLLECT_PERIOD = 0x01,
	PTI_UPLOAD_PERIOD ,
	PTI_HIGHTEMP_ALARM,
	PTI_LOWTEMP_ALARM,	
	PTI_BIND_GATEWAY,
	PTI_BIND_NODE,
	PTI_SENSOR_CODEC,	    	
}PARA_TYPE_ID;

bool SetDevicePara(uint8_t *rxData, uint16_t length)
{
	uint8_t  paratype,i;
    uint16_t index = 0;
    bool     congfig = false,  sensorcodec = false;
#ifdef     SUPPORT_NETGATE_DISP_NODE        
    uint32_t deviceid;
#endif
#ifdef SUPPORT_NETGATE_BIND_NODE
    uint8_t num;
#endif

    while(index < length) {

    	paratype = rxData[index++];			
    	switch (paratype) {			
    	case PTI_COLLECT_PERIOD:
            if((index + 4)<= length) {
    			HIBYTE(HIWORD(g_rSysConfigInfo.collectPeriod)) = rxData[index++];
    			LOBYTE(HIWORD(g_rSysConfigInfo.collectPeriod)) = rxData[index++];
                HIBYTE(LOWORD(g_rSysConfigInfo.collectPeriod)) = rxData[index++];
                LOBYTE(LOWORD(g_rSysConfigInfo.collectPeriod)) = rxData[index++];
                if(g_rSysConfigInfo.collectPeriod < 10)
                    g_rSysConfigInfo.collectPeriod = 10;
                
                congfig = true;
            }
            else{
                goto paraerror;
            }
            
    		break;

    	case PTI_UPLOAD_PERIOD:
            if((index + 4)<= length) {
    			HIBYTE(HIWORD(g_rSysConfigInfo.uploadPeriod)) = rxData[index++];
    			LOBYTE(HIWORD(g_rSysConfigInfo.uploadPeriod)) = rxData[index++];
                HIBYTE(LOWORD(g_rSysConfigInfo.uploadPeriod)) = rxData[index++];
                LOBYTE(LOWORD(g_rSysConfigInfo.uploadPeriod)) = rxData[index++];
                if(g_rSysConfigInfo.uploadPeriod < 10)
                    g_rSysConfigInfo.uploadPeriod = 10;
                
                congfig = true;
            }
            else{
                goto paraerror;
            }
            
    		break;
    		
    	case PTI_HIGHTEMP_ALARM:
            if((index + 3)<= length) {
                if(rxData[index] == 0xff){
                    for ( i = 0 ; i < MODULE_SENSOR_MAX; ++i) {
                        g_rSysConfigInfo.alarmTemp[i].high = (uint16_t)rxData[index+1]<<8 | (uint16_t)rxData[index+2];
                    }
                }
                else if(rxData[index] < MODULE_SENSOR_MAX){
                    
    			    g_rSysConfigInfo.alarmTemp[rxData[index]].high = (uint16_t)rxData[index+1]<<8 | (uint16_t)rxData[index+2];

                }
                else{
                    goto paraerror;
                }
                    
                index += 3;	
                congfig = true;
            }
            else{
                goto paraerror;
            }
            
    		break;
    		
    	case PTI_LOWTEMP_ALARM:		
            if((index + 3)<= length) {
                if(rxData[index] == 0xff){
                    for ( i = 0 ; i < MODULE_SENSOR_MAX; ++i) {
                        g_rSysConfigInfo.alarmTemp[i].low= (uint16_t)rxData[index+1]<<8 | (uint16_t)rxData[index+2];
                    }
                }
                else if(rxData[index] < MODULE_SENSOR_MAX){
                    
    			    g_rSysConfigInfo.alarmTemp[rxData[index]].low= (uint16_t)rxData[index+1]<<8 | (uint16_t)rxData[index+2];

                }
                else{
                    goto paraerror;
                }                
                    
                index += 3;	
                congfig = true;
            }
            else{
                goto paraerror;
            }
            
    		break;

        case PTI_BIND_GATEWAY:
            if((index + 4)<= length) {
                g_rSysConfigInfo.BindGateway[0] = rxData[index++];
                g_rSysConfigInfo.BindGateway[1] = rxData[index++];
                g_rSysConfigInfo.BindGateway[2] = rxData[index++];
                g_rSysConfigInfo.BindGateway[3] = rxData[index++];

                congfig = true;
            }
            else{
                goto paraerror;
            }
            
            break;


#ifdef SUPPORT_NETGATE_BIND_NODE
        case PTI_BIND_NODE:///nodenum(1B)  [  Deviceid(4B ) chno(1B) alarmTemp(4B)  ...]  chk(1B)

            num = rxData[index++];

            if (num <= NETGATE_BIND_NODE_MAX  && (index + num*9 <= length )){
                for(i=0; i < num; ++i) {
                    HIBYTE(HIWORD(g_rSysConfigInfo.bindnode[i].Deviceid)) = rxData[index++];
                    LOBYTE(HIWORD(g_rSysConfigInfo.bindnode[i].Deviceid)) = rxData[index++];
                    HIBYTE(LOWORD(g_rSysConfigInfo.bindnode[i].Deviceid)) = rxData[index++];
                    LOBYTE(LOWORD(g_rSysConfigInfo.bindnode[i].Deviceid)) = rxData[index++];
                    
                    g_rSysConfigInfo.bindnode[i].ChNo = rxData[index++];
                    
                    HIBYTE(g_rSysConfigInfo.bindnode[i].AlarmInfo.high) = rxData[index++];
                    LOBYTE(g_rSysConfigInfo.bindnode[i].AlarmInfo.high) = rxData[index++];
                    HIBYTE(g_rSysConfigInfo.bindnode[i].AlarmInfo.low)  = rxData[index++];
                    LOBYTE(g_rSysConfigInfo.bindnode[i].AlarmInfo.low)  = rxData[index++];
                    
                }
                congfig = true;           
            }
            else{
                goto paraerror;
            }

            break;
            
#endif    

            
#ifdef SUPPORT_NETGATE_DISP_NODE
        case PTI_SENSOR_CODEC:
            if((index + 6)<= length) {   
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
                
            }
            else{
                goto paraerror;
            }
            
            break;
#endif      
        default:
            return false;//参数有误
    	}	
    }

    if(congfig)        
        Flash_store_config();                

    if(congfig || sensorcodec) {
        return true;
    }
    
paraerror:
    
    return false;
}


#ifdef SUPPORT_ZKS_PROTOCOL
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
    NMI_TX_G_SENSOR = 0x2022,
    NMI_TX_CELL_INFO = 0x2023,

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
//COM_ACK
    TCA_OK = 0,
    TCA_FAIL = 1,
    TCA_MSG_ERROR = 2,
    TCA_NOT_SUPPORT = 3,
}TX_COM_ACK_RESULT;

typedef struct {
	uint8_t uuid[2];
    uint16_t serialNumSensor;
    uint16_t serialNumLbs;
    uint16_t serialNumGSensor;
    uint8_t poweron;
    uint8_t ntp;
    uint8_t moduleIndex;
    uint16_t hbTime;
    uint32_t uploadTime;
    uint32_t ntpTime;
#ifdef SUPPORT_LBS
//location
    NwkLocation_t location;
#endif
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
static void Nwk_group_package(NWK_MSG_ID msgId, NwkMsgPacket_t *pPackets)
{
    uint8_t i;
    uint16_t value = 0;
#ifdef USE_QUECTEL_API_FOR_LBS
    uint32_t temp;
#endif
    NwkMsgPacket_t packet;
    NwkMsgPacket_t *pPacket = pPackets;
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


    if (msgId == NMI_TX_SENSOR) {
        //消息流水号
        packet.buff[packet.length++] = HIBYTE(rNwkObject.serialNumSensor);
        packet.buff[packet.length++] = LOBYTE(rNwkObject.serialNumSensor);
        rNwkObject.serialNumSensor++;

        //消息体
        //固定字段类型
        packet.buff[packet.length++] = 0x01;
        //网关电压
#ifdef SUPPORT_BATTERY
        value = Battery_get_voltage();
#endif
        packet.buff[packet.length++] = HIBYTE(value);
        packet.buff[packet.length++] = LOBYTE(value);
        //网关网络信号
        Nwk_FxnTablePtr[rNwkObject.moduleIndex]->controlFxn(NWK_CONTROL_RSSI_GET, &packet.buff[packet.length++]);
        //无线信号强度RSSI
        //Sensor ID - 4Byte
        //采集流水号 - 2Byte
        //采集时间
        //Sensor电压
        //参数项列表数据
ONEDATA:
        for (i = 0; i < pPacket->buff[0]; i++) {//sensor data: length(1B) rssi(1B) devicedi(4B)...
            packet.buff[packet.length++] = pPacket->buff[1 + i];
        }

        //判断是否还有数据
        if(pPacket->buff[pPacket->buff[0] + 1]!= 0){
            //一条数据的结束
            packet.buff[packet.length++] = 0xFF;
            pPacket = (NwkMsgPacket_t*)&(pPacket->buff[pPacket->buff[0] + 1]);
            goto ONEDATA;
        }
    } else if(msgId == NMI_TX_GPS) {
        for (i = 0; i < pPacket->buff[0]; i++) {
             packet.buff[packet.length++] = pPacket->buff[1 + i];
        }

    }
#ifdef USE_QUECTEL_API_FOR_LBS
    else if (msgId == NMI_TX_LBS) {
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
    }
#endif
    else if(msgId == NMI_TX_G_SENSOR) {

        //消息流水号
        packet.buff[packet.length++] = HIBYTE(rNwkObject.serialNumGSensor);
        packet.buff[packet.length++] = LOBYTE(rNwkObject.serialNumGSensor);
        rNwkObject.serialNumGSensor++;

        for (i = 0; i < pPacket->buff[0]; i++) {
            packet.buff[packet.length++] = pPacket->buff[1 + i];
        }
    } else if (msgId == NMI_TX_COM_ACK) {
        //消息流水号
        packet.buff[packet.length++] = HIBYTE(rNwkObject.serialNumSensor);
        packet.buff[packet.length++] = LOBYTE(rNwkObject.serialNumSensor);
        rNwkObject.serialNumSensor++;

        memcpy(&packet.buff[packet.length],pPacket->buff, 5);
        packet.length += 5;
    }
#ifdef USE_ENGINEERING_MODE_FOR_LBS
    else if(msgId == NMI_TX_CELL_INFO) {
        //消息流水号
        packet.buff[packet.length++] = HIBYTE(rNwkObject.serialNumLbs);
        packet.buff[packet.length++] = LOBYTE(rNwkObject.serialNumLbs);
        rNwkObject.serialNumLbs++;

        packet.buff[packet.length++] = HIBYTE(rNwkObject.location.mcc);
        packet.buff[packet.length++] = LOBYTE(rNwkObject.location.mcc);
        packet.buff[packet.length++] = HIBYTE(rNwkObject.location.mnc);
        packet.buff[packet.length++] = LOBYTE(rNwkObject.location.mnc);
        packet.buff[packet.length++] = 0;   //小区序号，0代表当前服务小区
        packet.buff[packet.length++] = HIBYTE(rNwkObject.location.local.lac);
        packet.buff[packet.length++] = LOBYTE(rNwkObject.location.local.lac);
        packet.buff[packet.length++] = HIBYTE(HIWORD(rNwkObject.location.local.cellid));
        packet.buff[packet.length++] = LOBYTE(HIWORD(rNwkObject.location.local.cellid));
        packet.buff[packet.length++] = HIBYTE(LOWORD(rNwkObject.location.local.cellid));
        packet.buff[packet.length++] = LOBYTE(LOWORD(rNwkObject.location.local.cellid));
        packet.buff[packet.length++] = rNwkObject.location.local.dbm;
#ifdef SUPPOERT_LBS_NEARBY_CELL
        for (i = 0; i < LBS_NEARBY_CELL_MAX; i++) {
            // 无效数据不上传
            if (rNwkObject.location.nearby[i].cellid == 0)
                break;
            packet.buff[packet.length++] = i + 1;   //小区序号
            packet.buff[packet.length++] = HIBYTE(rNwkObject.location.nearby[i].lac);
            packet.buff[packet.length++] = LOBYTE(rNwkObject.location.nearby[i].lac);
            packet.buff[packet.length++] = HIBYTE(HIWORD(rNwkObject.location.nearby[i].cellid));
            packet.buff[packet.length++] = LOBYTE(HIWORD(rNwkObject.location.nearby[i].cellid));
            packet.buff[packet.length++] = HIBYTE(LOWORD(rNwkObject.location.nearby[i].cellid));
            packet.buff[packet.length++] = LOBYTE(LOWORD(rNwkObject.location.nearby[i].cellid));
            packet.buff[packet.length++] = rNwkObject.location.nearby[i].dbm;
        }
#endif
    }else {//其他打包序列号也需增加 如授时
        //消息流水号
        packet.buff[packet.length++] = HIBYTE(rNwkObject.serialNumSensor);
        packet.buff[packet.length++] = LOBYTE(rNwkObject.serialNumSensor);
        rNwkObject.serialNumSensor++;
    }
#endif

    //消息体属性
    value = packet.length - 12;
    packet.buff[2] = HIBYTE(value);
    packet.buff[3] = LOBYTE(value);

    //校验码
    packet.buff[packet.length++] = CheckCode8(&packet.buff[0], packet.length);

    //进行转义
    pPackets->length = Protocol_escape(&pPackets->buff[1], &packet.buff[0], packet.length);
    //消息标志位
    pPackets->buff[0] = PROTOCOL_TOKEN;
    pPackets->buff[pPackets->length + 1] = PROTOCOL_TOKEN;
    pPackets->length += 2;

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
	//uint8_t  paratype;
	uint8_t *ptrstart,*ptrend;
    uint16_t package_length;
    bool     congfig = false;
    //uint32_t deviceid;
    
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
                congfig = SetDevicePara(rxData + NWK_MSG_BODY_START ,package_length - NWK_MSG_BODY_START);
                if(congfig){
                    
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
                /*
            case NMI_RX_ALARM:
                index = NWK_MSG_BODY_START;
                if((index + 10) <= package_length) {//可能包含多条报警数据，目前只处理1条
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
                }                
                break;
                */
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
    uint8_t  package_count;
    uint8_t* pbuff;
    /* Construct key process Event */
    Event_construct(&nwkEvtStruct, NULL);
    /* Obtain event instance handle */
    nwkEvtHandle = Event_handle(&nwkEvtStruct);

    Nwk_init();

    while (1) {
        if(rNwkObject.poweron){
            Led_ctrl(LED_R, 0, 0, 0);
            Led_ctrl(LED_G, 0, 0, 0);
            Led_ctrl(LED_B, 0, 0, 0);
        }
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

            Battery_voltage_measure();
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
                if (Nwk_FxnTablePtr[rNwkObject.moduleIndex]->controlFxn(NWK_CONTROL_RSSI_QUERY, NULL) == FALSE) {
                    continue;
                }

#ifdef SUPPORT_LBS
            //send lbs data first
                //query lbs.
                if (Nwk_FxnTablePtr[rNwkObject.moduleIndex]->controlFxn(NWK_CONTROL_LBS_QUERY, &rNwkObject.location) == FALSE) {
                    continue;
                }
#ifdef USE_QUECTEL_API_FOR_LBS
                Nwk_group_package(NMI_TX_LBS, &rNwkMsgPacket);
#elif defined(USE_ENGINEERING_MODE_FOR_LBS)
                Nwk_group_package(NMI_TX_CELL_INFO, &rNwkMsgPacket);
#endif
                if (Nwk_FxnTablePtr[rNwkObject.moduleIndex]->controlFxn(NWK_CONTROL_TRANSMIT, &rNwkMsgPacket) == FALSE) {
                    continue;
                }           
#endif
                
#ifdef FLASH_EXTERNAL
                //send data second.
                bsensordata = 0;
                while(Flash_get_unupload_items()> 0){
                     package_count = 0;
                     memset(rNwkMsgPacket.buff,0x00,NWK_MSG_SIZE);
                     pbuff = rNwkMsgPacket.buff;
                     while((Flash_get_unupload_items()> 0)&&(package_count < PACKAGE_ITEM_COUNT_MAX)){
                        if ((pbuff + FLASH_SENSOR_DATA_SIZE) > (rNwkMsgPacket.buff + NWK_MSG_SIZE)){
                            break;
                        }

                        if(Flash_load_sensor_data_by_offset(pbuff, FLASH_SENSOR_DATA_SIZE, package_count) == ES_SUCCESS) {
                                bsensordata = 1;
                                package_count++;
                                pbuff = pbuff+ pbuff[0]+ 1;
                                pbuff[0] = 0;
                        }else{
                            break;
                         }

                    }

                    if(bsensordata == 1){
                        Nwk_group_package(NMI_TX_SENSOR, &rNwkMsgPacket);
                        if (Nwk_FxnTablePtr[rNwkObject.moduleIndex]->controlFxn(NWK_CONTROL_TRANSMIT, &rNwkMsgPacket) == FALSE) {
                            break;
                        }
                        else{
                            Flash_moveto_offset_sensor_data(package_count);
                        }
                    }
                }
                
                //当网关没有sensor数据时上传一个无效sensor数据以避免网关信息失联。
                if(!bsensordata){
                    Sensor_store_null_package(rNwkMsgPacket.buff);
                    Nwk_group_package(NMI_TX_SENSOR, &rNwkMsgPacket);
                    Nwk_FxnTablePtr[rNwkObject.moduleIndex]->controlFxn(NWK_CONTROL_TRANSMIT, &rNwkMsgPacket);
                }
                

#endif             

            }
            
#ifdef SUPPORT_GSM_SHORT_CONNECT
            if(g_rSysConfigInfo.uploadPeriod >= 60*5) {//uploadPeriod>=5min ,shutdown 
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
        else
        #ifdef SUPPORT_GSM_SHORT_CONNECT
        if (g_rSysConfigInfo.uploadPeriod < 60*5) //uploadPeriod>=10min ,dont send heartbeat 
        #endif
        {
            rNwkObject.hbTime++;
            if (rNwkObject.hbTime >= g_rSysConfigInfo.hbPeriod) {
                rNwkObject.hbTime = 0;
                Nwk_event_post(NWK_EVT_HEARTBEAT);
            }
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

    if (g_rSysConfigInfo.status & STATUS_GSM_TEST)
        Nwk_event_post(NWK_EVT_POWERON | NWK_EVT_TEST);
    else
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
#endif

