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
	PTI_SEND_ALARM,
	PTI_DEVICE_ID = 0x10,
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
    			HIBYTE_ZKS(HIWORD_ZKS(g_rSysConfigInfo.collectPeriod)) = rxData[index++];
    			LOBYTE_ZKS(HIWORD_ZKS(g_rSysConfigInfo.collectPeriod)) = rxData[index++];
                HIBYTE_ZKS(LOWORD_ZKS(g_rSysConfigInfo.collectPeriod)) = rxData[index++];
                LOBYTE_ZKS(LOWORD_ZKS(g_rSysConfigInfo.collectPeriod)) = rxData[index++];
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
    			HIBYTE_ZKS(HIWORD_ZKS(g_rSysConfigInfo.uploadPeriod)) = rxData[index++];
    			LOBYTE_ZKS(HIWORD_ZKS(g_rSysConfigInfo.uploadPeriod)) = rxData[index++];
                HIBYTE_ZKS(LOWORD_ZKS(g_rSysConfigInfo.uploadPeriod)) = rxData[index++];
                LOBYTE_ZKS(LOWORD_ZKS(g_rSysConfigInfo.uploadPeriod)) = rxData[index++];
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
#ifdef S_C
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
#endif

#ifdef SUPPORT_NETGATE_BIND_NODE
        case PTI_BIND_NODE:///nodenum(1B)  [  Deviceid(4B ) chno(1B) alarmTemp(4B)  ...]  chk(1B)

            num = rxData[index++];

            memset(&g_rSysConfigInfo.bindnode, 0, sizeof(g_rSysConfigInfo.bindnode));
            if (num <= NETGATE_BIND_NODE_MAX  && (index + num*13 <= length )){
                for(i=0; i < num; ++i) {
                    HIBYTE_ZKS(HIWORD_ZKS(g_rSysConfigInfo.bindnode[i].Deviceid)) = rxData[index++];
                    LOBYTE_ZKS(HIWORD_ZKS(g_rSysConfigInfo.bindnode[i].Deviceid)) = rxData[index++];
                    HIBYTE_ZKS(LOWORD_ZKS(g_rSysConfigInfo.bindnode[i].Deviceid)) = rxData[index++];
                    LOBYTE_ZKS(LOWORD_ZKS(g_rSysConfigInfo.bindnode[i].Deviceid)) = rxData[index++];
                    
                    g_rSysConfigInfo.bindnode[i].ChNo = rxData[index++];
                    
                    HIBYTE_ZKS(g_rSysConfigInfo.bindnode[i].AlarmInfo.high) = rxData[index++];
                    LOBYTE_ZKS(g_rSysConfigInfo.bindnode[i].AlarmInfo.high) = rxData[index++];
                    HIBYTE_ZKS(g_rSysConfigInfo.bindnode[i].AlarmInfo.low)  = rxData[index++];
                    LOBYTE_ZKS(g_rSysConfigInfo.bindnode[i].AlarmInfo.low)  = rxData[index++];

					index +=4;//鍏煎鍗忚锛�5529涓嶆敮鎸佹箍搴︽姤璀︿俊鎭�
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
                HIBYTE_ZKS(HIWORD_ZKS(deviceid)) = rxData[index + 0];
                LOBYTE_ZKS(HIWORD_ZKS(deviceid)) = rxData[index + 1];
                HIBYTE_ZKS(LOWORD_ZKS(deviceid)) = rxData[index + 2];
                LOBYTE_ZKS(LOWORD_ZKS(deviceid)) = rxData[index + 3];
                index += (5 + rxData[4]);

                if(rxData[4] == 1) {// is sensor  codec
                    Flash_store_sensor_codec(rxData[5], deviceid);                            
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

#ifndef S_A
        case PTI_SEND_ALARM:
            if(((index + 1)<= length) && (g_rSysConfigInfo.rfStatus & STATUS_LORA_MASTER)) {// maseter is sensor 

                if(rxData[index++])
                    g_rSysConfigInfo.rfStatus |= STATUS_LORA_ALARM;
                else
        			g_rSysConfigInfo.rfStatus &= ~STATUS_LORA_ALARM;
                    
                congfig = true;
            }
            else{
                goto paraerror;
            }
            
    		break;            
#endif
        default:
            return false;//鍙傛暟鏈夎
    	}	
    }

    if(congfig)      
        Sys_event_post(SYSTEMAPP_EVT_STORE_SYS_CONFIG);

    if(congfig || sensorcodec) {
        return true;
    }
    
paraerror:
    
    return false;
}

uint8_t GetDevicePara(uint8_t paratype, uint8_t *rxData)
{
	uint8_t i;
    uint16_t index = 0;    
#ifdef     SUPPORT_NETGATE_DISP_NODE        
    uint32_t deviceid;
#endif
#ifdef SUPPORT_NETGATE_BIND_NODE
    uint8_t num;
#endif

	rxData[index++] = paratype;		

	switch (paratype) {			
	case PTI_COLLECT_PERIOD:
		rxData[index++] = HIBYTE_ZKS(HIWORD_ZKS(g_rSysConfigInfo.collectPeriod));
		rxData[index++] = LOBYTE_ZKS(HIWORD_ZKS(g_rSysConfigInfo.collectPeriod));
        rxData[index++] = HIBYTE_ZKS(LOWORD_ZKS(g_rSysConfigInfo.collectPeriod));
        rxData[index++] = LOBYTE_ZKS(LOWORD_ZKS(g_rSysConfigInfo.collectPeriod));                 
		break;
			
	case PTI_UPLOAD_PERIOD:
		rxData[index++] = HIBYTE_ZKS(HIWORD_ZKS(g_rSysConfigInfo.uploadPeriod));
		rxData[index++] = LOBYTE_ZKS(HIWORD_ZKS(g_rSysConfigInfo.uploadPeriod));
        rxData[index++] = HIBYTE_ZKS(LOWORD_ZKS(g_rSysConfigInfo.uploadPeriod));
        rxData[index++] = LOBYTE_ZKS(LOWORD_ZKS(g_rSysConfigInfo.uploadPeriod));
		break;
		
	case PTI_HIGHTEMP_ALARM:
        for ( i = 0 ; i < MODULE_SENSOR_MAX; ++i) {
            rxData[index++] = i;
            rxData[index++] = HIBYTE_ZKS(g_rSysConfigInfo.alarmTemp[i].high);
            rxData[index++] = LOBYTE_ZKS(g_rSysConfigInfo.alarmTemp[i].high);
        }
		break;
		
	case PTI_LOWTEMP_ALARM:		
        for ( i = 0 ; i < MODULE_SENSOR_MAX; ++i) {
            rxData[index++] = i;
            rxData[index++] = HIBYTE_ZKS(g_rSysConfigInfo.alarmTemp[i].low);
            rxData[index++] = LOBYTE_ZKS(g_rSysConfigInfo.alarmTemp[i].low);
        }        
		break;
        
#ifdef S_C
    case PTI_BIND_GATEWAY:        
        rxData[index++] = g_rSysConfigInfo.BindGateway[0];
        rxData[index++] = g_rSysConfigInfo.BindGateway[1];
        rxData[index++] = g_rSysConfigInfo.BindGateway[2];
        rxData[index++] = g_rSysConfigInfo.BindGateway[3];
        break;
#endif

#ifdef SUPPORT_NETGATE_BIND_NODE
    case PTI_BIND_NODE:///nodenum(1B)  [  Deviceid(4B ) chno(1B) alarmTemp(4B)  ...]  chk(1B)

        num = 0;
        rxData[index++] = num;//index =1

        
        for( i = 0 ; i < NETGATE_BIND_NODE_MAX; ++i) {
            
            if(g_rSysConfigInfo.bindnode[i].Deviceid != 0){
                
                num++;     
                rxData[index++] = HIBYTE_ZKS(HIWORD_ZKS(g_rSysConfigInfo.bindnode[i].Deviceid));
                rxData[index++] = LOBYTE_ZKS(HIWORD_ZKS(g_rSysConfigInfo.bindnode[i].Deviceid));
                rxData[index++] = HIBYTE_ZKS(LOWORD_ZKS(g_rSysConfigInfo.bindnode[i].Deviceid));
                rxData[index++] = LOBYTE_ZKS(LOWORD_ZKS(g_rSysConfigInfo.bindnode[i].Deviceid));                
                rxData[index++] = g_rSysConfigInfo.bindnode[i].ChNo;                
                rxData[index++] = HIBYTE_ZKS(g_rSysConfigInfo.bindnode[i].AlarmInfo.high);
                rxData[index++] = LOBYTE_ZKS(g_rSysConfigInfo.bindnode[i].AlarmInfo.high);
                rxData[index++] = HIBYTE_ZKS(g_rSysConfigInfo.bindnode[i].AlarmInfo.low) ;
                rxData[index++] = LOBYTE_ZKS(g_rSysConfigInfo.bindnode[i].AlarmInfo.low) ;      
				rxData[index++] = 0xff;		//鍏煎鍗忚锛�5529涓嶆敮鎸佹箍搴︽姤璀︿俊鎭�
				rxData[index++] = 0xff;
				rxData[index++] = 0xff;
				rxData[index++] = 0xff;
            }    
        }

        rxData[1] = num;       
        break;
        
#endif    

        
#ifdef SUPPORT_NETGATE_DISP_NODE
    case PTI_SENSOR_CODEC:
        rxData[index++] = HIBYTE_ZKS(HIWORD_ZKS(deviceid));
        rxData[index++] = LOBYTE_ZKS(HIWORD_ZKS(deviceid));
        rxData[index++] = HIBYTE_ZKS(LOWORD_ZKS(deviceid));
        rxData[index++] = LOBYTE_ZKS(LOWORD_ZKS(deviceid));
        rxData[index++] = 1;
        rxData[index++] = Flash_load_sensor_codec(deviceid);                            
        break;
        
#endif  
#ifndef S_A
    case PTI_SEND_ALARM:
        if((g_rSysConfigInfo.rfStatus & STATUS_LORA_MASTER))// maseter is sensor        
            rxData[index++] = (g_rSysConfigInfo.rfStatus & STATUS_LORA_ALARM);
        else
            rxData[index++] = 0xff;        
        break;
#endif 

    case PTI_DEVICE_ID:
        rxData[index++] = g_rSysConfigInfo.DeviceId[0];
        rxData[index++] = g_rSysConfigInfo.DeviceId[1];
        rxData[index++] = g_rSysConfigInfo.DeviceId[2];
        rxData[index++] = g_rSysConfigInfo.DeviceId[3];
        break;
        
    default:
        return 0;//paratype is error
	}	

    return index;//return the length of para
}


#ifdef SUPPORT_ZKS_PROTOCOL
#ifdef SUPPORT_NETWORK

typedef enum {
//Send MSG
    NMI_TX_COM_ACK  = 0x2001,
    NMI_TX_HEARTBEAT= 0x2002,
    NMI_TX_SETTING  = 0x2003,
    NMI_TX_NTP      = 0x2004,
    NMI_TX_SYS_STATE= 0x2005,
    NMI_TX_GETIP_ACK= 0x2006,

    NMI_TX_SENSOR   = 0x2010,
    NMI_TX_GPS 	    = 0x2020,
    NMI_TX_LBS 	    = 0x2021,
    NMI_TX_G_SENSOR = 0x2022,
    NMI_TX_CELL_INFO = 0x2023,

    NMI_TX_UPGRADE_INFO_REQ = 0x2024,
    NMI_TX_UPGRADE_DATA_REQ = 0x2025,
    NMI_TX_UPGRADE_ACK      = 0x2026,

//Receive MSG
    NMI_RX_COM_ACK  = 0xA001,
    NMI_RX_SETTING  = 0xA002,
    NMI_RX_ALARM    = 0xA003,
    NMI_RX_NTP      = 0xA004,
    NMI_RX_SETIP    = 0xA005,
    NMI_RX_GETIP    = 0xA006,
    NMI_RX_RESET    = 0xA007,
    NMI_RX_UPGRADE_INFO_ACK = 0xA008,
    NMI_RX_UPGRADE_DATA_ACK = 0xA009,
    NMI_RX_UPGRADE_REQ      = 0xA00A,
    NMI_RX_FLIGHT_MODE      = 0xA00B,    

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
#ifdef  SUPPORT_REMOTE_UPGRADE
    uint8_t upgradeState;
#endif
    uint16_t serialNumSysState;
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
#define NWK_EVT_UPGRADE             Event_Id_06


#define NWK_EVT_ALL                 0xffff


#define NWKTASKSTACKSIZE           1024

#define NWK_KEEP_CONNET_MAX_TIME   	(5*60) //

static Task_Struct nwkTaskStruct;
static uint8_t nwkTaskStack[NWKTASKSTACKSIZE];

static Event_Struct nwkEvtStruct;
static Event_Handle nwkEvtHandle;

NwkObject_t rNwkObject;
NwkMsgPacket_t rNwkMsgPacket;
#define COM_ACK_BUF_LEN  5
uint8_t ComAckBuff[COM_ACK_BUF_LEN];

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

#ifdef  SUPPORT_REMOTE_UPGRADE

extern Watchdog_Handle watchdogHandle;
static void Nwk_event_post(UInt event);
static void Nwk_group_package(NWK_MSG_ID msgId, NwkMsgPacket_t *pPackets);
typedef enum{
    UPGRADE_STATE_OFF = 0,
    UPGRADE_STATE_CHANGE_SEVER,
    UPGRADE_STATE_START,    
    UPGRADE_STATE_WAIT_INFO,
    UPGRADE_STATE_LOADING,
}UPGRADE_STATE_E;




#ifdef SUPPORT_UPLOADTIME_LIMIT
#define     UPLOAD_MAX_CONTINUE_TIME    15

void Nwk_upload_set(void)
{
    rNwkObject.uploadTime = 0;
    rNwkObject.hbTime = 0;
	if(nwkEvtHandle)
    	Event_post(nwkEvtHandle, NWK_EVT_DATA_UPLOAD);
}
#endif // SUPPORT_UPLOADTIME_LIMIT

//***********************************************************************************
//
// Network upgrade process.
//
//***********************************************************************************
void Nwk_upgrade_process(void)
{
    uint16_t result;

    UpgradePackTimeoutReset();
    if(UpgradeMonitorGet() >= UPGRADE_TIMEOUT)
    {
        UpgradeMonitorReset();
        rNwkMsgPacket.buff[0] = 0xff;
        goto UpgradeTimeout;
    }


    switch(rNwkObject.upgradeState)
    {
        case UPGRADE_STATE_CHANGE_SEVER:
        Nwk_FxnTablePtr[rNwkObject.moduleIndex]->closeFxn();
        memcpy((char*)g_rSysConfigInfo.serverAddr, "114.215.122.32", 40);
        g_rSysConfigInfo.serverIpPort = 12201;

        rNwkObject.upgradeState = UPGRADE_STATE_START;
        Nwk_event_post(NWK_EVT_UPGRADE);
        break;

        case UPGRADE_STATE_OFF:
        // send the ack to upgrade server
        rNwkObject.upgradeState = UPGRADE_STATE_START;
        Nwk_group_package(NMI_TX_COM_ACK, &rNwkMsgPacket);
        Nwk_FxnTablePtr[rNwkObject.moduleIndex]->controlFxn(NWK_CONTROL_TRANSMIT, &rNwkMsgPacket);
        Nwk_FxnTablePtr[rNwkObject.moduleIndex]->closeFxn();
        memcpy((char*)g_rSysConfigInfo.serverAddr, "114.215.122.32", 40);
        g_rSysConfigInfo.serverIpPort = 12201;

        rNwkObject.upgradeState = UPGRADE_STATE_START;
        Nwk_event_post(NWK_EVT_UPGRADE);
        break;

        case UPGRADE_STATE_START:
RerequestVersion:
        ClearUpgradeInfo();
        // send the requestion to request the updata version
        Nwk_group_package(NMI_TX_UPGRADE_INFO_REQ, &rNwkMsgPacket);
        Nwk_FxnTablePtr[rNwkObject.moduleIndex]->controlFxn(NWK_CONTROL_TRANSMIT, &rNwkMsgPacket);
        rNwkObject.upgradeState = UPGRADE_STATE_WAIT_INFO;
        break;

        case UPGRADE_STATE_WAIT_INFO:
        result = ReadUpgradeInfoFlag();
        if(result == UPGRADE_RESULT_UNREC_VER)
        {
            goto RerequestVersion;
        }
//        result = UpgradeSetInfo(((uint16_t)rNwkMsgPacket.buff[0] << 8) + rNwkMsgPacket.buff[1],
//                                ((uint32_t)rNwkMsgPacket.buff[2] << 24) + ((uint32_t)rNwkMsgPacket.buff[3] << 16) +
//                                ((uint16_t)rNwkMsgPacket.buff[4] << 8) + ((uint16_t)rNwkMsgPacket.buff[5]));

        if(result == UPGRADE_RESULT_NEEDNOT_UPDATA)
        {
            rNwkObject.upgradeState = UPGRADE_STATE_OFF;
            Nwk_FxnTablePtr[rNwkObject.moduleIndex]->closeFxn();
            Flash_load_config();
            // send the ack to the g7 server

        }
        else
        {
            // request the code data
            rNwkObject.upgradeState = UPGRADE_STATE_LOADING;
            Nwk_group_package(NMI_TX_UPGRADE_DATA_REQ, &rNwkMsgPacket);
            Nwk_FxnTablePtr[rNwkObject.moduleIndex]->controlFxn(NWK_CONTROL_TRANSMIT, &rNwkMsgPacket);
        }

        break;

        case UPGRADE_STATE_LOADING:
        result = UpgradeLoading(((uint16_t)rNwkMsgPacket.buff[0] << 8) + rNwkMsgPacket.buff[1], 
                                ((uint16_t)rNwkMsgPacket.buff[2] << 8) + rNwkMsgPacket.buff[3], 
                                ((uint16_t)rNwkMsgPacket.buff[4] << 8) + rNwkMsgPacket.buff[5], 
                                    &rNwkMsgPacket.buff[6]);

        switch(result)
        {
            case UPGRADE_RESULT_LOADING_COMPLETE:
            // send the updata sucess to upgrade server
            rNwkMsgPacket.buff[0] = 0x00;
            Nwk_group_package(NMI_TX_UPGRADE_ACK, &rNwkMsgPacket);
            Nwk_FxnTablePtr[rNwkObject.moduleIndex]->controlFxn(NWK_CONTROL_TRANSMIT, &rNwkMsgPacket);
            
#ifdef SUPPORT_WATCHDOG
            if (watchdogHandle != NULL) 
                Watchdog_close(watchdogHandle); 
#endif
            // __disable_interrupt();
            Task_sleep(200*CLOCK_UNIT_MS);
            SysCtrlSystemReset();
            break;

            case UPGRADE_RESULT_NEXT_PACKAGE:
            case UPGRADE_RESULT_PACKNUM_ERR:
            // request the next package
            Nwk_group_package(NMI_TX_UPGRADE_DATA_REQ, &rNwkMsgPacket);
            Nwk_FxnTablePtr[rNwkObject.moduleIndex]->controlFxn(NWK_CONTROL_TRANSMIT, &rNwkMsgPacket);            

            break;

            case UPGRADE_RESULT_CRC_ERR:
            // send the updata crc error to upgrade server
            rNwkMsgPacket.buff[0] = 0x01;
            goto UpgradeFail;

            case UPGRADE_RESULT_ERR:
            // send the updata fail to upgrade server
            rNwkMsgPacket.buff[0] = 0xff;
UpgradeFail:
            UpgradeCancel();
UpgradeTimeout:
            Nwk_group_package(NMI_TX_UPGRADE_ACK, &rNwkMsgPacket);
            Nwk_FxnTablePtr[rNwkObject.moduleIndex]->controlFxn(NWK_CONTROL_TRANSMIT, &rNwkMsgPacket);
            rNwkObject.upgradeState = UPGRADE_STATE_OFF;
            Nwk_FxnTablePtr[rNwkObject.moduleIndex]->closeFxn();
            Flash_load_config();
            break;

        }
        break;
    }
}
#endif

static void Send_ack_to_server(uint8_t* rxData)
{
    ComAckBuff[0] = rxData[10];
    ComAckBuff[1] = rxData[11];
    ComAckBuff[2] = rxData[0];
    ComAckBuff[3] = rxData[1];
    ComAckBuff[4] = TCA_OK;
    Nwk_event_post(NWK_EVT_ACK);
}

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

#define SUPPORT_BB_TIME

    NwkMsgPacket_t packet;
    NwkMsgPacket_t *pPacket = pPackets;
    packet.length = 0;
    //娑堟伅澶�
    //娑堟伅ID
    packet.buff[packet.length++] = HIBYTE_ZKS(msgId);
    packet.buff[packet.length++] = LOBYTE_ZKS(msgId);
    //娑堟伅浣撳睘鎬�
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
        //娑堟伅娴佹按鍙�
        packet.buff[packet.length++] = HIBYTE_ZKS(rNwkObject.serialNumSensor);
        packet.buff[packet.length++] = LOBYTE_ZKS(rNwkObject.serialNumSensor);
        rNwkObject.serialNumSensor++;

        //娑堟伅浣�
        //鍥哄畾瀛楁绫诲瀷
        #ifdef SUPPORT_BB_TIME
        packet.buff[packet.length++] = 0x03;
        #else
        packet.buff[packet.length++] = 0x01;
        #endif
        
        //缃戝叧鐢靛帇
#ifdef SUPPORT_BATTERY
        value = Battery_get_voltage();
#endif
        packet.buff[packet.length++] = HIBYTE_ZKS(value);
        packet.buff[packet.length++] = LOBYTE_ZKS(value);
        //缃戝叧缃戠粶淇″彿
        Nwk_FxnTablePtr[rNwkObject.moduleIndex]->controlFxn(NWK_CONTROL_RSSI_GET, &packet.buff[packet.length++]);

        #ifdef SUPPORT_BB_TIME
        Calendar calendar;
        calendar = Rtc_get_calendar();        
        packet.buff[packet.length++] = TransHexToBcd(calendar.Year - CALENDAR_BASE_YEAR);
        packet.buff[packet.length++] = TransHexToBcd(calendar.Month);
        packet.buff[packet.length++] = TransHexToBcd(calendar.DayOfMonth);
        packet.buff[packet.length++] = TransHexToBcd(calendar.Hours);
        packet.buff[packet.length++] = TransHexToBcd(calendar.Minutes);
        packet.buff[packet.length++] = TransHexToBcd(calendar.Seconds);
        #endif
        //鏃犵嚎淇″彿寮哄害RSSI
        //Sensor ID - 4Byte
        //閲囬泦娴佹按鍙� - 2Byte
        //閲囬泦鏃堕棿
        //Sensor鐢靛帇
        //鍙傛暟椤瑰垪琛ㄦ暟鎹�
ONEDATA:
        for (i = 0; i < pPacket->buff[0]; i++) {//sensor data: length(1B) rssi(1B) devicedi(4B)...
            packet.buff[packet.length++] = pPacket->buff[1 + i];
        }

        //鍒ゆ柇鏄惁杩樻湁鏁版嵁
        if(pPacket->buff[pPacket->buff[0] + 1]!= 0){
            //涓�鏉℃暟鎹殑缁撴潫
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
        packet.buff[packet.length++] = HIBYTE_ZKS(HIWORD_ZKS(temp));
        packet.buff[packet.length++] = LOBYTE_ZKS(HIWORD_ZKS(temp));
        packet.buff[packet.length++] = HIBYTE_ZKS(LOWORD_ZKS(temp));
        packet.buff[packet.length++] = LOBYTE_ZKS(LOWORD_ZKS(temp));
        temp = rNwkObject.location.latitude * 100000;
        packet.buff[packet.length++] = HIBYTE_ZKS(HIWORD_ZKS(temp));
        packet.buff[packet.length++] = LOBYTE_ZKS(HIWORD_ZKS(temp));
        packet.buff[packet.length++] = HIBYTE_ZKS(LOWORD_ZKS(temp));
        packet.buff[packet.length++] = LOBYTE_ZKS(LOWORD_ZKS(temp));
    }
#endif
    else if(msgId == NMI_TX_G_SENSOR) {

        //娑堟伅娴佹按鍙�
        packet.buff[packet.length++] = HIBYTE_ZKS(rNwkObject.serialNumGSensor);
        packet.buff[packet.length++] = LOBYTE_ZKS(rNwkObject.serialNumGSensor);
        rNwkObject.serialNumGSensor++;

        for (i = 0; i < pPacket->buff[0]; i++) {
            packet.buff[packet.length++] = pPacket->buff[1 + i];
        }
    } else if (msgId == NMI_TX_COM_ACK) {
        //娑堟伅娴佹按鍙�
        packet.buff[packet.length++] = HIBYTE_ZKS(rNwkObject.serialNumSensor);
        packet.buff[packet.length++] = LOBYTE_ZKS(rNwkObject.serialNumSensor);
        rNwkObject.serialNumSensor++;

        memcpy(&packet.buff[packet.length],ComAckBuff, COM_ACK_BUF_LEN);
        packet.length += COM_ACK_BUF_LEN;
    }
#ifdef USE_ENGINEERING_MODE_FOR_LBS
    else if(msgId == NMI_TX_CELL_INFO) {
        //娑堟伅娴佹按鍙�
        packet.buff[packet.length++] = HIBYTE_ZKS(rNwkObject.serialNumLbs);
        packet.buff[packet.length++] = LOBYTE_ZKS(rNwkObject.serialNumLbs);
        rNwkObject.serialNumLbs++;

        packet.buff[packet.length++] = HIBYTE_ZKS(rNwkObject.location.mcc);
        packet.buff[packet.length++] = LOBYTE_ZKS(rNwkObject.location.mcc);
        packet.buff[packet.length++] = HIBYTE_ZKS(rNwkObject.location.mnc);
        packet.buff[packet.length++] = LOBYTE_ZKS(rNwkObject.location.mnc);
        packet.buff[packet.length++] = 0;   //灏忓尯搴忓彿锛�0浠ｈ〃褰撳墠鏈嶅姟灏忓尯
        packet.buff[packet.length++] = HIBYTE_ZKS(rNwkObject.location.local.lac);
        packet.buff[packet.length++] = LOBYTE_ZKS(rNwkObject.location.local.lac);
        packet.buff[packet.length++] = HIBYTE_ZKS(HIWORD_ZKS(rNwkObject.location.local.cellid));
        packet.buff[packet.length++] = LOBYTE_ZKS(HIWORD_ZKS(rNwkObject.location.local.cellid));
        packet.buff[packet.length++] = HIBYTE_ZKS(LOWORD_ZKS(rNwkObject.location.local.cellid));
        packet.buff[packet.length++] = LOBYTE_ZKS(LOWORD_ZKS(rNwkObject.location.local.cellid));
        packet.buff[packet.length++] = rNwkObject.location.local.dbm;
#ifdef SUPPOERT_LBS_NEARBY_CELL
        for (i = 0; i < LBS_NEARBY_CELL_MAX; i++) {
            // 鏃犳晥鏁版嵁涓嶄笂浼�
            if (rNwkObject.location.nearby[i].cellid == 0)
                break;
            packet.buff[packet.length++] = i + 1;   //灏忓尯搴忓彿
            packet.buff[packet.length++] = HIBYTE_ZKS(rNwkObject.location.nearby[i].lac);
            packet.buff[packet.length++] = LOBYTE_ZKS(rNwkObject.location.nearby[i].lac);
            packet.buff[packet.length++] = HIBYTE_ZKS(HIWORD_ZKS(rNwkObject.location.nearby[i].cellid));
            packet.buff[packet.length++] = LOBYTE_ZKS(HIWORD_ZKS(rNwkObject.location.nearby[i].cellid));
            packet.buff[packet.length++] = HIBYTE_ZKS(LOWORD_ZKS(rNwkObject.location.nearby[i].cellid));
            packet.buff[packet.length++] = LOBYTE_ZKS(LOWORD_ZKS(rNwkObject.location.nearby[i].cellid));
            packet.buff[packet.length++] = rNwkObject.location.nearby[i].dbm;
        }
#endif
    }
#endif

#ifdef      SUPPORT_REMOTE_UPGRADE
    else if(msgId == NMI_TX_UPGRADE_INFO_REQ) {
        packet.buff[packet.length++] = HIBYTE_ZKS(rNwkObject.serialNumSensor);
        packet.buff[packet.length++] = LOBYTE_ZKS(rNwkObject.serialNumSensor);
        rNwkObject.serialNumSensor++;
        // current version
        packet.buff[packet.length++] = (uint8_t)(FW_VERSION>>8);
        packet.buff[packet.length++] = (uint8_t)(FW_VERSION);

        // project name
        packet.buff[packet.length++] = sizeof(PROJECT_NAME);
        strcpy((char*)&packet.buff[packet.length], PROJECT_NAME);
        packet.length += sizeof(PROJECT_NAME);
        
    }
    else if(msgId == NMI_TX_UPGRADE_DATA_REQ)
    {
        packet.buff[packet.length++] = HIBYTE_ZKS(rNwkObject.serialNumSensor);
        packet.buff[packet.length++] = LOBYTE_ZKS(rNwkObject.serialNumSensor);
        rNwkObject.serialNumSensor++;
        // updata software version
        packet.buff[packet.length++] = (uint8_t)(UpgradeGetVersion() >> 8);
        packet.buff[packet.length++] = (uint8_t)UpgradeGetVersion();

        packet.buff[packet.length++] = (uint8_t)(UpgradeGetNextPackNum() >> 8);
        packet.buff[packet.length++] = (uint8_t)UpgradeGetNextPackNum();

        packet.buff[packet.length++] = (uint8_t)(NWK_UPGRADE_PACKAGE_LENGTH >> 8);
        packet.buff[packet.length++] = (uint8_t)NWK_UPGRADE_PACKAGE_LENGTH;

        // project name
        packet.buff[packet.length++] = sizeof(PROJECT_NAME);
        strcpy((char*)&packet.buff[packet.length], PROJECT_NAME);
        packet.length += sizeof(PROJECT_NAME);
    }
    else if(msgId == NMI_TX_UPGRADE_ACK)
    {
        packet.buff[packet.length++] = HIBYTE_ZKS(rNwkObject.serialNumSensor);
        packet.buff[packet.length++] = LOBYTE_ZKS(rNwkObject.serialNumSensor);
        rNwkObject.serialNumSensor++;
        // current version
        if(pPacket->buff[0] == 0x00)
        {
            packet.buff[packet.length++] = (uint8_t)(UpgradeGetVersion()>>8);
            packet.buff[packet.length++] = (uint8_t)(UpgradeGetVersion());
        }
        else
        {
            packet.buff[packet.length++] = (uint8_t)(FW_VERSION>>8);
            packet.buff[packet.length++] = (uint8_t)(FW_VERSION);
        }

        // the upgrade result
        packet.buff[packet.length++] = pPacket->buff[0];
    }
#endif
#ifdef SUPPORT_DEVICED_STATE_UPLOAD
    else if(msgId == NMI_TX_SYS_STATE){
        //娑堟伅娴佹按鍙�
        packet.buff[packet.length++] = HIBYTE_ZKS(rNwkObject.serialNumSysState);
        packet.buff[packet.length++] = LOBYTE_ZKS(rNwkObject.serialNumSysState);
        rNwkObject.serialNumSysState++;
        memcpy(packet.buff + packet.length,pPacket->buff+1,pPacket->buff[0] - 1);
        packet.length = packet.length + pPacket->buff[0] - 1;

        if( pPacket->buff[1]== TYPE_POWER_ON){
            //extern const uint8_t PROJECT_INFO_NAME[];
            //extern const uint16_t PROJECT_INFO_VERSION;

            packet.buff[packet.length++] = strlen(PROJECT_NAME) + 5;

            memcpy((char *) packet.buff + packet.length,  PROJECT_NAME, strlen(PROJECT_NAME));
            packet.length = packet.length + strlen(PROJECT_NAME);   

            packet.buff[packet.length++] = ':';
            packet.buff[packet.length++] = '0'+ ((FW_VERSION & 0xf000)>>12);
            packet.buff[packet.length++] = '0'+ ((FW_VERSION & 0x0f00)>>8);
            packet.buff[packet.length++] = '0'+ ((FW_VERSION & 0x00f0)>>4);
            packet.buff[packet.length++] = '0'+ ((FW_VERSION & 0x000f)>>0);
        }
    }
#endif
    else {//鍏朵粬鎵撳寘搴忓垪鍙蜂篃闇�澧炲姞 濡傛巿鏃�
        //娑堟伅娴佹按鍙�
        packet.buff[packet.length++] = HIBYTE_ZKS(rNwkObject.serialNumSensor);
        packet.buff[packet.length++] = LOBYTE_ZKS(rNwkObject.serialNumSensor);
        rNwkObject.serialNumSensor++;
    }
    //娑堟伅浣撳睘鎬�
    value = packet.length - 12;
    packet.buff[2] = HIBYTE_ZKS(value);
    packet.buff[3] = LOBYTE_ZKS(value);

    //鏍￠獙鐮�
    packet.buff[packet.length++] = CheckCode8(&packet.buff[0], packet.length);

    //杩涜杞箟
    pPackets->length = Protocol_escape(&pPackets->buff[1], &packet.buff[0], packet.length);
    //娑堟伅鏍囧織浣�
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
	if(nwkEvtHandle)
	    Event_post(nwkEvtHandle, event);
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
    uint8_t  *rxData = 0;
    // uint8_t  rxData[128];
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
            
        // memcpy((char *)rxData, (char *)ptrstart, package_length);
        rxData = ptrstart;

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
                    Send_ack_to_server(rxData);
                }
                    
                break;

#ifdef SUPPORT_NETGATE_DISP_NODE
            case NMI_RX_ALARM:
                index = NWK_MSG_BODY_START;
                if((index + 10) <= package_length) {//鍙兘鍖呭惈澶氭潯鎶ヨ鏁版嵁锛岀洰鍓嶅彧澶勭悊1鏉�
                    HIBYTE_ZKS(HIWORD_ZKS(g_AlarmSensor.DeviceId)) = rxData[index++];
                    LOBYTE_ZKS(HIWORD_ZKS(g_AlarmSensor.DeviceId)) = rxData[index++];
                    HIBYTE_ZKS(LOWORD_ZKS(g_AlarmSensor.DeviceId)) = rxData[index++];
                    LOBYTE_ZKS(LOWORD_ZKS(g_AlarmSensor.DeviceId)) = rxData[index++];
                    g_AlarmSensor.index      = rxData[index++];
                    g_AlarmSensor.type       = rxData[index++];
                    if (!(g_AlarmSensor.type > SENSOR_DATA_NONE && g_AlarmSensor.type < SENSOR_DATA_MAX))
                        break;//invalid sensor type

                    //all  data  saved to tempdeep
                    HIBYTE_ZKS(HIWORD_ZKS(g_AlarmSensor.value.tempdeep)) = rxData[index++];
                    LOBYTE_ZKS(HIWORD_ZKS(g_AlarmSensor.value.tempdeep)) = rxData[index++];
                    HIBYTE_ZKS(LOWORD_ZKS(g_AlarmSensor.value.tempdeep)) = rxData[index++];
                    LOBYTE_ZKS(LOWORD_ZKS(g_AlarmSensor.value.tempdeep)) = rxData[index++];

                    if((index + 6) <= package_length){

                        g_AlarmSensor.time[0] = rxData[index++];
                        g_AlarmSensor.time[1] = rxData[index++];
                        g_AlarmSensor.time[2] = rxData[index++];
                        g_AlarmSensor.time[3] = rxData[index++];
                        g_AlarmSensor.time[4] = rxData[index++];
                        g_AlarmSensor.time[5] = rxData[index++];
                    }
#ifdef SUPPORT_ALARM_RECORD_QURERY					
                    Sys_event_post(SYS_EVT_ALARM_SAVE);
#endif
                    g_bAlarmSensorFlag = 0x100;

                    Sys_event_post(SYS_EVT_ALARM);


                    //send ack to server
                    Send_ack_to_server(rxData);
                }                
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
    			}
                break;

#ifdef SUPPORT_FLIGHT_MODE
            case NMI_RX_FLIGHT_MODE:
                index = NWK_MSG_BODY_START;

                // Message: UINT16 2(B)<TIMES-MINUTES>
                // Get MINUTES TO FLIGHT MODE
                uint16_t minutes_flight = 0;

                HIBYTE_ZKS(minutes_flight) = rxData[index++];
                LOBYTE_ZKS(minutes_flight) = rxData[index++];

                Flight_mode_setting( minutes_flight);

                //send ack to server
                Send_ack_to_server(rxData);
                break;
#endif

#ifdef      SUPPORT_REMOTE_UPGRADE

            case NMI_RX_UPGRADE_INFO_ACK:
                index = NWK_MSG_BODY_START;
                memcpy(rNwkMsgPacket.buff, &rxData[index], 6);
                SetUpgradeInfo(((uint16_t)rNwkMsgPacket.buff[0] << 8) + rNwkMsgPacket.buff[1],
                               ((uint32_t)rNwkMsgPacket.buff[2] << 24) + ((uint32_t)rNwkMsgPacket.buff[3] << 16) +
                               ((uint16_t)rNwkMsgPacket.buff[4] << 8) + ((uint16_t)rNwkMsgPacket.buff[5]));
                Nwk_event_post(NWK_EVT_UPGRADE);
                break;

            case NMI_RX_UPGRADE_DATA_ACK:
                index = NWK_MSG_BODY_START;

                rNwkMsgPacket.length = 6 + ((uint16_t)rxData[index+4]<<8) + rxData[index+5];

                memcpy(rNwkMsgPacket.buff, &rxData[index], rNwkMsgPacket.length);
                Nwk_event_post(NWK_EVT_UPGRADE);
                break;

            case NMI_RX_UPGRADE_REQ:
                //send ack to server
                Send_ack_to_server(rxData);
                rNwkObject.upgradeState = UPGRADE_STATE_CHANGE_SEVER;
                Nwk_event_post(NWK_EVT_UPGRADE);
                break;
#endif
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

    rNwkObject.poweron      = 0;
    rNwkObject.ntp          = 0;
    rNwkObject.upgradeState = UPGRADE_STATE_OFF;
    rNwkObject.hbTime       = 0;
    rNwkObject.uploadTime   = 0;
    rNwkObject.ntpTime      = 0;
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
    // the systask process first
    Task_sleep(50 * CLOCK_UNIT_MS);


    Nwk_init();
    UpgradeInit();

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
            //Nwk_FxnTablePtr[rNwkObject.moduleIndex]->controlFxn(NWK_CONTROL_SHUTDOWN_MSG, NULL);
            Nwk_FxnTablePtr[rNwkObject.moduleIndex]->closeFxn();
        }

        if (rNwkObject.poweron == 0)
            continue;

#ifdef SUPPORT_FLIGHT_MODE
        // When in flight mode, priority process ACK event
		if(Flight_mode_isFlightMode()){
			if (eventId & NWK_EVT_ACK) {

#ifdef  SUPPORT_DEVICED_STATE_UPLOAD
	            Flash_store_devices_state(TYPE_FLIGHT_MODE_START);
#endif
                Nwk_group_package(NMI_TX_COM_ACK, &rNwkMsgPacket);
				//send ack data.
				if (Nwk_FxnTablePtr[rNwkObject.moduleIndex]->controlFxn(NWK_CONTROL_TRANSMIT, &rNwkMsgPacket) == FALSE) {

				}

			}
#ifdef  SUPPORT_DEVICED_STATE_UPLOAD
               while(Flash_get_deviced_state_items() > 0 )
               {
                    memset(rNwkMsgPacket.buff,0x00,NWK_MSG_SIZE);
                    pbuff = rNwkMsgPacket.buff;
                    if(Flash_load_deviced_state_data(pbuff, FLASH_DEVICED_STATE_DATA_SIZE) == ES_SUCCESS){
                        Nwk_group_package(NMI_TX_SYS_STATE, &rNwkMsgPacket);

                        if (Nwk_FxnTablePtr[rNwkObject.moduleIndex]->controlFxn(NWK_CONTROL_TRANSMIT, &rNwkMsgPacket) == FALSE) {
                            break;
                        }
                        else
                        {
                            Flash_moveto_next_deviced_state_data();
                        }
                    }

                }
#endif
			// flush event,
            Flight_mode_entry(Flight_mode_get_timesetting());
            Nwk_event_post(NWK_EVT_SHUTDOWN);
            // close GSM immediate
            Nwk_poweroff();
            continue;
        }
#endif


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


#ifdef  SUPPORT_REMOTE_UPGRADE
        if(eventId & NWK_EVT_UPGRADE)
        {
            //wakeup.
            if (Nwk_FxnTablePtr[rNwkObject.moduleIndex]->controlFxn(NWK_CONTROL_WAKEUP, NULL) == FALSE) {
                continue;
            }
            if(eventId & NWK_EVT_ACK)
            {
                Nwk_group_package(NMI_TX_COM_ACK, &rNwkMsgPacket);
                if (Nwk_FxnTablePtr[rNwkObject.moduleIndex]->controlFxn(NWK_CONTROL_TRANSMIT, &rNwkMsgPacket) == FALSE) {
                    continue;
                }
            }
            Nwk_upgrade_process();
        }

// code updata process
        if(rNwkObject.upgradeState != UPGRADE_STATE_OFF)
        {
            continue;
        }

#endif

        if (rNwkObject.ntp == 0) {
            //wakeup.
            if (Nwk_FxnTablePtr[rNwkObject.moduleIndex]->controlFxn(NWK_CONTROL_WAKEUP, NULL) == FALSE) {
                continue;
            }
            Nwk_group_package(NMI_TX_NTP, &rNwkMsgPacket);
            Nwk_FxnTablePtr[rNwkObject.moduleIndex]->controlFxn(NWK_CONTROL_TRANSMIT, &rNwkMsgPacket);
            continue;
        }

        if (eventId & (NWK_EVT_DATA_UPLOAD | NWK_EVT_HEARTBEAT | NWK_EVT_ACK)) {
#ifdef      SUPPORT_UPLOADTIME_LIMIT
            rNwkObject.uploadTime = 0;
#endif      // SUPPORT_UPLOADTIME_LIMIT
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
                Nwk_group_package(NMI_TX_COM_ACK, &rNwkMsgPacket);
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

            #ifdef SUPPORT_FLIGHT_MODE
                    // When Get Flight Mode, terminate UPLOAD data
                    if(Flight_mode_isFlightMode()){
                        break;
                    }
            #endif

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
#ifdef      SUPPORT_UPLOADTIME_LIMIT
                        if(rNwkObject.uploadTime >= UPLOAD_MAX_CONTINUE_TIME){
                            Event_pend(nwkEvtHandle, 0, NWK_EVT_DATA_UPLOAD | NWK_EVT_HEARTBEAT, BIOS_NO_WAIT);
                            rNwkObject.uploadTime = 0;
                            rNwkObject.hbTime = 0;
                            break;
                        }
#endif      // SUPPORT_UPLOADTIME_LIMIT
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
                
                //褰撶綉鍏虫病鏈塻ensor鏁版嵁鏃朵笂浼犱竴涓棤鏁坰ensor鏁版嵁浠ラ伩鍏嶇綉鍏充俊鎭け鑱斻��
                if(!bsensordata){
                    memset(rNwkMsgPacket.buff,0x00,NWK_MSG_SIZE);
                    Sensor_store_null_package(rNwkMsgPacket.buff);
                    Nwk_group_package(NMI_TX_SENSOR, &rNwkMsgPacket);
                    Nwk_FxnTablePtr[rNwkObject.moduleIndex]->controlFxn(NWK_CONTROL_TRANSMIT, &rNwkMsgPacket);
                }
                
#ifdef SUPPORT_DEVICED_STATE_UPLOAD
                //绯荤粺鐘舵�佷俊鎭�

                    while(Flash_get_deviced_state_items() > 0 )
                    {
                        memset(rNwkMsgPacket.buff,0x00,NWK_MSG_SIZE);
                        pbuff = rNwkMsgPacket.buff;
                        if(Flash_load_deviced_state_data(pbuff, FLASH_DEVICED_STATE_DATA_SIZE) == ES_SUCCESS){
                            Nwk_group_package(NMI_TX_SYS_STATE, &rNwkMsgPacket);
                        }
                        else{
                            break;
                        }


                        if (Nwk_FxnTablePtr[rNwkObject.moduleIndex]->controlFxn(NWK_CONTROL_TRANSMIT, &rNwkMsgPacket) == FALSE) {
                            break;
                        }
                        else
                        {
                            Flash_moveto_next_deviced_state_data();
                        }
                    }

#endif
#ifdef SUPPORT_GSENSOR
                //send g_sensor data second.
                while (Flash_load_g_sensor_data(rNwkMsgPacket.buff, FLASH_G_SENSOR_DATA_SIZE) == ES_SUCCESS) {
                    Nwk_group_package(NMI_TX_G_SENSOR, &rNwkMsgPacket);
                    if (Nwk_FxnTablePtr[rNwkObject.moduleIndex]->controlFxn(NWK_CONTROL_TRANSMIT, &rNwkMsgPacket) == FALSE) {
                        Flash_recovery_last_g_sensor_data();
                        //ret = FALSE;
                        break;
                    }
                }
#endif

#ifdef SUPPORT_GPS
                //send gps data second.
                while (Flash_load_gps_data(rNwkMsgPacket.buff, FLASH_GPS_DATA_SIZE) == ES_SUCCESS) {
                    Nwk_group_package(NMI_TX_GPS, &rNwkMsgPacket);
                    if (Nwk_FxnTablePtr[rNwkObject.moduleIndex]->controlFxn(NWK_CONTROL_TRANSMIT, &rNwkMsgPacket) == FALSE) {
                        Flash_recovery_last_gps_data();
                        //ret = FALSE;
                        break;
                    }
                }
#endif
#endif             

            }
            
#ifdef SUPPORT_GSM_SHORT_CONNECT
            uint32_t temp;
            temp = g_rSysConfigInfo.uploadPeriod;
#ifdef SUPPORT_ALARM_SWITCH_PERIOD
            if(g_alarmFlag)
                temp = g_rSysConfigInfo.alarmuploadPeriod;
#endif // SUPPORT_ALARM_SWITCH_PERIOD
            if(temp >= NWK_KEEP_CONNET_MAX_TIME) {//uploadPeriod>=5min ,shutdown 
                Nwk_FxnTablePtr[rNwkObject.moduleIndex]->closeFxn();
            }
            else
#endif // SUPPORT_GSM_SHORT_CONNECT
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

    /* Construct key process Event */
    Event_construct(&nwkEvtStruct, NULL);
    /* Obtain event instance handle */
    nwkEvtHandle = Event_handle(&nwkEvtStruct);

    //if (!(g_rSysConfigInfo.module & MODULE_NWK))
    //    return;

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
    uint32_t temp;
    if (!(g_rSysConfigInfo.module & MODULE_NWK))
        return;

    if (rNwkObject.poweron) {
        rNwkObject.uploadTime++;
        temp = g_rSysConfigInfo.uploadPeriod;

#ifdef  SUPPORT_ALARM_SWITCH_PERIOD
        if(g_alarmFlag)
            temp = g_rSysConfigInfo.alarmuploadPeriod;
#endif // SUPPORT_ALARM_SWITCH_PERIOD
        if (rNwkObject.uploadTime >= temp) {
            rNwkObject.uploadTime = 0;
            rNwkObject.hbTime = 0;
            Nwk_event_post(NWK_EVT_DATA_UPLOAD);
        }
        else
        #ifdef SUPPORT_GSM_SHORT_CONNECT
        if (temp < NWK_KEEP_CONNET_MAX_TIME) //uploadPeriod>=5min ,dont send heartbeat 
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
#ifdef  SUPPORT_REMOTE_UPGRADE
        if(rNwkObject.upgradeState != UPGRADE_STATE_OFF)
        {
            if(UpgradeMonitor() == 1)
                Nwk_event_post(NWK_EVT_UPGRADE);
        }
#endif
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

void Nwk_send_upload_event(void)
{
    if (!(g_rSysConfigInfo.module & MODULE_NWK))
        return;

    if (rNwkObject.poweron) {
        Nwk_event_post(NWK_EVT_DATA_UPLOAD);
    }
}
//***********************************************************************************
//
// Network get rssi.
//
//***********************************************************************************
uint8_t Nwk_get_rssi(void)
{
    uint8_t rssi=0;
    if (!(g_rSysConfigInfo.module & MODULE_NWK))
        return 0;

    if (rNwkObject.poweron == 0)
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


 // 1,actived, 0 shutdown
char Nwk_is_Active(void)
{
    if (!(g_rSysConfigInfo.module & MODULE_NWK))
        return 0;

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

