//***********************************************************************************
// Copyright 2017-2018, Zksiot Development Ltd.
// Created by yuanGuo, 2018.07.10
// Last modify by yuanGuo, 2018.07.31
// MCU:	MSP430F5529
// OS: TI-RTOS
// Project:
// File name: config_id_mode.c
// Description: used for configure system setting with id mode.
//***********************************************************************************

#include "../general.h"
#include "config_id_mode_type.h"
#include "config_id_mode.h"

#if SYS_CONFIG_USE_ID_MODE
// ret : write length
uint16_t sys_Get_Config_Id(uint8_t* pData, uint16_t buf_len, uint8_t id_start, uint8_t id_end, send_data_callback_func send_data_callback)
{
    uint8_t id = id_start;
    uint16_t pPos =0;
    uint8_t j;
    uint16_t ret_length = 0;

    for(id = id_start; id <= id_end; id++){

        // id in buffer and transfer
        pData[pPos++] = id;
        // format : id(1)data_len(1)data(n)
        switch(id){
            case 0x01:  // software version
                pData[pPos++] = 2; // length
                pData[pPos++] = HIBYTE(g_rSysConfigInfo.swVersion);
                pData[pPos++] = LOBYTE_ZKS(g_rSysConfigInfo.swVersion);
                break;
            case 0x02:  // device id
                pData[pPos++] = 4; // length
                memcpy(pData + pPos,g_rSysConfigInfo.DeviceId, 4);
                pPos += 4;
                break;
            case 0x03:  // custom id
                pData[pPos++] = 2; // length
                pData[pPos++] = g_rSysConfigInfo.customId[0];
                pData[pPos++] = g_rSysConfigInfo.customId[1];
                break;
            case 0x04:  // function config info
                pData[pPos++] = 2; // length
                pData[pPos++] = HIBYTE(g_rSysConfigInfo.status);
                pData[pPos++] = LOBYTE_ZKS(g_rSysConfigInfo.status);
                break;
            case 0x05:  // mode config
                pData[pPos++] = 2; // length
                pData[pPos++] = HIBYTE(g_rSysConfigInfo.module);
                pData[pPos++] = LOBYTE_ZKS(g_rSysConfigInfo.module);
                break;
            case 0x06:  // sensor channel configure
                //MODULE_SENSOR_MAX
                pData[pPos++] = MODULE_SENSOR_MAX; // length
                memcpy(pData + pPos, g_rSysConfigInfo.sensorModule, MODULE_SENSOR_MAX);
                pPos += MODULE_SENSOR_MAX ;//8;
                break;
            case 0x07:  // Alarm temp configure // max 8 channel
				pPos--;
                for(j=0; j < MODULE_SENSOR_MAX; j++){
                    pData[pPos++] = 0x07; // type
                    pData[pPos++] = 9; // length
                    pData[pPos++] = j; // x channel
                    pData[pPos++] = HIBYTE(g_rSysConfigInfo.alarmTemp[j].high);
                    pData[pPos++] = LOBYTE_ZKS(g_rSysConfigInfo.alarmTemp[j].high);
                    pData[pPos++] = HIBYTE(g_rSysConfigInfo.alarmTemp[j].low);
                    pData[pPos++] = LOBYTE_ZKS(g_rSysConfigInfo.alarmTemp[j].low);

                    pData[pPos++] = HIBYTE(g_rSysConfigInfo.alarmTemp[j].high_1);
                    pData[pPos++] = LOBYTE_ZKS(g_rSysConfigInfo.alarmTemp[j].high_1);
                    pData[pPos++] = HIBYTE(g_rSysConfigInfo.alarmTemp[j].low_1);
                    pData[pPos++] = LOBYTE_ZKS(g_rSysConfigInfo.alarmTemp[j].low_1);
                }
				
                //pPos += MODULE_SENSOR_MAX ;//8;

                break;
            case 0x08:  // collectPeriod

                pData[pPos++] = 4; // length
                pData[pPos++] = HIBYTE(HIWORD_ZKS(g_rSysConfigInfo.collectPeriod));
                pData[pPos++] = LOBYTE_ZKS(HIWORD_ZKS(g_rSysConfigInfo.collectPeriod));
                pData[pPos++] = HIBYTE(LOWORD_ZKS(g_rSysConfigInfo.collectPeriod));
                pData[pPos++] = LOBYTE_ZKS(LOWORD_ZKS(g_rSysConfigInfo.collectPeriod));
                break;
            case 0x09: // server ip
                pData[pPos++] = 4; // length
                pData[pPos++] = g_rSysConfigInfo.serverIpAddr[0];
                pData[pPos++] = g_rSysConfigInfo.serverIpAddr[1];
                pData[pPos++] = g_rSysConfigInfo.serverIpAddr[2];
                pData[pPos++] = g_rSysConfigInfo.serverIpAddr[3];
                break;
            case 0x0A: // server port
                pData[pPos++] = 2; // length
                pData[pPos++] = HIBYTE(g_rSysConfigInfo.serverIpPort);
                pData[pPos++] = LOBYTE_ZKS(g_rSysConfigInfo.serverIpPort);
                break;
            case 0x0B:  // heatbeat period
                pData[pPos++] = 2; // length
                pData[pPos++] = HIBYTE(g_rSysConfigInfo.hbPeriod);
                pData[pPos++] = LOBYTE_ZKS(g_rSysConfigInfo.hbPeriod);
                break;
            case 0x0C:  // upload period
                pData[pPos++] = 4; // length
                pData[pPos++] = HIBYTE(HIWORD_ZKS(g_rSysConfigInfo.uploadPeriod));
                pData[pPos++] = LOBYTE_ZKS(HIWORD_ZKS(g_rSysConfigInfo.uploadPeriod));
                pData[pPos++] = HIBYTE(LOWORD_ZKS(g_rSysConfigInfo.uploadPeriod));
                pData[pPos++] = LOBYTE_ZKS(LOWORD_ZKS(g_rSysConfigInfo.uploadPeriod));
                break;
            case 0x0D:  // NTP

                pData[pPos++] = 4; // length
                pData[pPos++] = HIBYTE(HIWORD_ZKS(g_rSysConfigInfo.ntpPeriod));
                pData[pPos++] = LOBYTE_ZKS(HIWORD_ZKS(g_rSysConfigInfo.ntpPeriod));
                pData[pPos++] = HIBYTE(LOWORD_ZKS(g_rSysConfigInfo.ntpPeriod));
                pData[pPos++] = LOBYTE_ZKS(LOWORD_ZKS(g_rSysConfigInfo.ntpPeriod));
                break;

#ifdef SUPPORT_LORA
            case 0x0E:  // RF Power
                pData[pPos++] = 1; // length
                pData[pPos++] = g_rSysConfigInfo.rfPA;
                break;
            case 0x0F:  // RF Bound width
                pData[pPos++] = 1; // length
                pData[pPos++] = g_rSysConfigInfo.rfBW;
                break;
            case 0x10:  // RF spreading factor
                pData[pPos++] = 1; // length
                pData[pPos++] = g_rSysConfigInfo.rfSF;
                break;
            case 0x11:  // RF status
                pData[pPos++] = 1; // length
                pData[pPos++] = g_rSysConfigInfo.rfStatus;
                break;

#else
            case 0x0e ... 0x11:
                pPos -= 1; // CMD 头去掉
                break;
#endif
            case 0x12:
                pData[pPos++] = 2; // length
                pData[pPos++] = HIBYTE(g_rSysConfigInfo.batLowVol);
                pData[pPos++] = LOBYTE_ZKS(g_rSysConfigInfo.batLowVol);
                break;
            case 0x13:  // Warning temp configure // max 8 channel

                pPos--;
                // all last data
                //uint8_t j;
                for(j=0; j < MODULE_SENSOR_MAX; j++){
                    pData[pPos++] = 0x13; // type
                    pData[pPos++] = 9; // length
                    pData[pPos++] = j; // x channel
                    pData[pPos++] = HIBYTE(g_rSysConfigInfo.WarningTemp[j].high);
                    pData[pPos++] = LOBYTE_ZKS(g_rSysConfigInfo.WarningTemp[j].high);
                    pData[pPos++] = HIBYTE(g_rSysConfigInfo.WarningTemp[j].low);
                    pData[pPos++] = LOBYTE_ZKS(g_rSysConfigInfo.WarningTemp[j].low);

                    pData[pPos++] = HIBYTE(g_rSysConfigInfo.WarningTemp[j].high_1);
                    pData[pPos++] = LOBYTE_ZKS(g_rSysConfigInfo.WarningTemp[j].high_1);
                    pData[pPos++] = HIBYTE(g_rSysConfigInfo.WarningTemp[j].low_1);
                    pData[pPos++] = LOBYTE_ZKS(g_rSysConfigInfo.WarningTemp[j].low_1);
                   // System_printf("get ch:%d hig:%d low:%d \n",j,g_rSysConfigInfo.WarningTemp[j].high,g_rSysConfigInfo.WarningTemp[j].low );
                   // System_flush();
                }
                break;
            case 0x14:  // bind gateway
#ifdef S_C
                pData[pPos++] = 4; // length
                memcpy(pData + pPos, g_rSysConfigInfo.BindGateway, 4);
                pPos += 4;
#else
                pPos -= 1; // CMD 头去掉
#endif
                break;
            case 0x15:
#ifdef SUPPORT_NETGATE_BIND_NODE

		pPos--;

                // all last data
                //uint8_t j;
                for(j=0; j < NETGATE_BIND_NODE_MAX; j++){

                    if( (g_rSysConfigInfo.bindnode[j].Deviceid != 0x00 )&& (g_rSysConfigInfo.bindnode[j].Deviceid != 0xffffffff))
                    {
                        pData[pPos++] = 0x15;  // ID

                        pData[pPos++] = 14; // length

                        pData[pPos++] = j;    // 0 index
                        // DID
                        pData[pPos++] = HIBYTE(HIWORD_ZKS(g_rSysConfigInfo.bindnode[j].Deviceid));
                        pData[pPos++] = LOBYTE_ZKS(HIWORD_ZKS(g_rSysConfigInfo.bindnode[j].Deviceid));
                        pData[pPos++] = HIBYTE(LOWORD_ZKS(g_rSysConfigInfo.bindnode[j].Deviceid));
                        pData[pPos++] = LOBYTE_ZKS(LOWORD_ZKS(g_rSysConfigInfo.bindnode[j].Deviceid));

                        pData[pPos++] = g_rSysConfigInfo.bindnode[j].ChNo; // channel

                        pData[pPos++] = HIBYTE(g_rSysConfigInfo.bindnode[j].AlarmInfo.high);
                        pData[pPos++] = LOBYTE_ZKS(g_rSysConfigInfo.bindnode[j].AlarmInfo.high);
                        pData[pPos++] = HIBYTE(g_rSysConfigInfo.bindnode[j].AlarmInfo.low);
                        pData[pPos++] = LOBYTE_ZKS(g_rSysConfigInfo.bindnode[j].AlarmInfo.low);

                        pData[pPos++] = HIBYTE(g_rSysConfigInfo.bindnode[j].AlarmInfo.high_1);
                        pData[pPos++] = LOBYTE_ZKS(g_rSysConfigInfo.bindnode[j].AlarmInfo.high_1);
                        pData[pPos++] = HIBYTE(g_rSysConfigInfo.bindnode[j].AlarmInfo.low_1);
                        pData[pPos++] = LOBYTE_ZKS(g_rSysConfigInfo.bindnode[j].AlarmInfo.low_1);
                    }

                }
#else
                pPos -= 1; // CMD 头去掉
#endif
                break;
            case 0x16:
#ifdef SUPPORT_APN_USER_PWD
                pData[pPos++] = strlen((const char*)g_rSysConfigInfo.apnuserpwd);    // length

                memcpy(pData+pPos, (const char*)g_rSysConfigInfo.apnuserpwd, pData[pPos-1]);
                pPos += pData[pPos-1];
#else
                pPos -= 1; // CMD 头去掉
#endif
                break;
            case 0x17:
                pData[pPos++] = strlen((const char*)g_rSysConfigInfo.serverAddr);    // length

                memcpy(pData+pPos, (const char*)g_rSysConfigInfo.serverAddr, pData[pPos-1]);

                pPos += pData[pPos-1];
                break;
            case ID_CALENDER_RTC_BCD : //0x18:
                pData[pPos++] = ID_CALENDER_RTC_BCD_LEN;    // length

                memcpy(pData+pPos, (const char*)&g_rSysConfigInfo.rtc, pData[pPos-1]);

                pPos += pData[pPos-1];
                break;
            case 0x19:
                pData[pPos++] = 20;    // length

                memcpy(pData+pPos, (const char*)&g_rSysConfigInfo.sysState, pData[pPos-1]);

                pPos += pData[pPos-1];
                break;
            case ID_ALARM_UPLOAD_PERIOD: //0x1A:  // alarmuploadPeriod
                pData[pPos++] = 4; // length

                pData[pPos++] =   HIBYTE(HIWORD_ZKS(g_rSysConfigInfo.alarmuploadPeriod));
                pData[pPos++] = LOBYTE_ZKS(HIWORD_ZKS(g_rSysConfigInfo.alarmuploadPeriod));
                pData[pPos++] = HIBYTE(LOWORD_ZKS(g_rSysConfigInfo.alarmuploadPeriod));
                pData[pPos++] = LOBYTE_ZKS(LOWORD_ZKS(g_rSysConfigInfo.alarmuploadPeriod));
                break;
            case ID_SERVER_ACCESS_CODE:
#ifdef ZKML_PROJECT
                pData[pPos++] = ID_SERVER_ACCESS_CODE_LEN_MAX; // length
                memcpy(pData+pPos,(char*)g_rSysConfigInfo.access_code, ID_SERVER_ACCESS_CODE_LEN_MAX); // copy new
                pPos += ID_SERVER_ACCESS_CODE_LEN_MAX;
#else
                pPos -= 1; // remove ID header
#endif
                break;
            case ID_SERVER_USER_NAME:
#ifdef ZKML_PROJECT
                pData[pPos++] = ID_SERVER_USER_NAME_LEN_MAX; // length
                memcpy(pData+pPos,(char*)g_rSysConfigInfo.user_name, ID_SERVER_USER_NAME_LEN_MAX); // copy new
                pPos += ID_SERVER_USER_NAME_LEN_MAX;
#else
                pPos -= 1; // remove ID header
#endif
                break;
            case ID_SERVER_USER_PWD:
#ifdef ZKML_PROJECT
                pData[pPos++] = ID_SERVER_USER_PWD_LEN_MAX; // length
                memcpy(pData+pPos,(char*)g_rSysConfigInfo.user_password, ID_SERVER_USER_PWD_LEN_MAX); // copy new
                pPos += ID_SERVER_USER_PWD_LEN_MAX;
#else
                pPos -= 1; // remove ID header
#endif
                break;
            case ID_SENSOR_CHN_ADJUST:  // channel adjust value // max 8 channel
                pPos--;
#ifdef ZKML_PROJECT
                // all last data
                //uint8_t j;
                for(j=0; j < MODULE_SENSOR_MAX; j++){
                    pData[pPos++] = ID_SENSOR_CHN_ADJUST; // type
                    pData[pPos++] = 5; // length
                    pData[pPos++] = j; // x channel
                    pData[pPos++] = HIBYTE(g_rSysConfigInfo.ChannelAdjust[j].AdjustTempValue);
                    pData[pPos++] = LOBYTE_ZKS(g_rSysConfigInfo.ChannelAdjust[j].AdjustTempValue);
                    pData[pPos++] = HIBYTE(g_rSysConfigInfo.ChannelAdjust[j].AdjustHumiValue);
                    pData[pPos++] = LOBYTE_ZKS(g_rSysConfigInfo.ChannelAdjust[j].AdjustHumiValue);
                }
#endif
                break;
            default:
                // 未支持的 ID 去掉ID 标识头,skip this ID
                pPos -= 1;
                break;
        }

        // 分批打包上传 //
        if(pPos >= 64){
#if 1
            if(NULL != send_data_callback){
                send_data_callback(pData, pPos);
            }
#else

            pPos = Usb_group_package(AC_Send_Config_ID, pData, pPos);
            USBCDCD_sendData(pData, pPos, USB_TXRX_TIMEROUT);
#endif
            ret_length += pPos;
            pPos = 0;
        }
    }

    // 尾包处理
    if(pPos > 0){
#if 1
        if(NULL != send_data_callback){
            send_data_callback(pData, pPos);
        }
#else
        pPos = Usb_group_package(AC_Send_Config_ID, pData, pPos);
        USBCDCD_sendData(pData, pPos, USB_TXRX_TIMEROUT);
#endif
        ret_length += pPos;
        pPos = 0;
    }

    return ret_length;
}


// return :position witch place parsed
uint16_t sys_Set_Config_Id(uint8_t* pData, uint16_t all_data_len)
{
    uint8_t id ;
    uint8_t index; // index for array setting
    uint8_t index_ch; // index for channel

    uint16_t pPos =0;
    uint16_t id_data_len;

    uint8_t j;

    while(pPos < all_data_len){

        // id in buffer and transfer
         id = pData[pPos++];
        // format : id(1)data_len(1)data(n)
        switch(id){
            case 0x01:  // software version
                if(all_data_len - pPos < 3){ // need 3 data
                    return pPos -1; // return the front before id
                }
                id_data_len = pData[pPos++]; // = 2; // length
                HIBYTE(g_rSysConfigInfo.swVersion) = pData[pPos];
                LOBYTE_ZKS(g_rSysConfigInfo.swVersion) = pData[pPos+1];
                pPos += 2;
                break;
            case 0x02:  // device id
                if(all_data_len - pPos < 5){ // need n data
                    return pPos -1; // return the front before id
                }

                id_data_len = pData[pPos++];// = 4; // length
                g_rSysConfigInfo.DeviceId[0] = pData[pPos];
                g_rSysConfigInfo.DeviceId[1] = pData[pPos+1];
                g_rSysConfigInfo.DeviceId[2] = pData[pPos+2];
                g_rSysConfigInfo.DeviceId[3] = pData[pPos+3];
                pPos += 4;
                break;
            case 0x03:  // custom id
                if(all_data_len - pPos < 3){ // need n data
                    return pPos -1; // return the front before id
                }

                id_data_len = pData[pPos++];// = 2; // length

                g_rSysConfigInfo.customId[0] = pData[pPos];
                g_rSysConfigInfo.customId[1] = pData[pPos+1];
                pPos += 2;
                break;
            case 0x04:  // function config info
                if(all_data_len - pPos < 3){ // need n data
                    return pPos -1; // return the front before id
                }
                id_data_len = pData[pPos++]; // = 2; // length

                HIBYTE(g_rSysConfigInfo.status) = pData[pPos];
                LOBYTE_ZKS(g_rSysConfigInfo.status) = pData[pPos+1];
                pPos += 2;
                break;
            case 0x05:  // mode config
                if(all_data_len - pPos < 3){ // need n data
                    return pPos -1; // return the front before id
                }
                id_data_len = pData[pPos++]; // = 2; // length

                HIBYTE(g_rSysConfigInfo.module) = pData[pPos];
                LOBYTE_ZKS(g_rSysConfigInfo.module) = pData[pPos+1];
                pPos += 2;
                break;
            case 0x06:  // sensor channel configure
                if(all_data_len - pPos < 9){ // need n data
                    return pPos -1; // return the front before id
                }
                //MODULE_SENSOR_MAX
                id_data_len = pData[pPos++]; // = MODULE_SENSOR_MAX; // length
                for(j=0; j < MODULE_SENSOR_MAX; j++){
                    g_rSysConfigInfo.sensorModule[j] = pData[pPos+j];
                }
                pPos += MODULE_SENSOR_MAX ;//8;
                break;
            case 0x07:  // Alarm temp configure // max 8 channel
                if(all_data_len - pPos < 10){ // need n data
                    return pPos -1; // return the front before id
                }

                id_data_len = pData[pPos++];// = 5; // length
                // first channel
                index_ch = pData[pPos]; // = 0; // 0 channel // MODULE_SENSOR_MAX ;//8;

                if(index_ch < MODULE_SENSOR_MAX){
                HIBYTE(g_rSysConfigInfo.alarmTemp[index_ch].high) = pData[pPos+1];
                LOBYTE_ZKS(g_rSysConfigInfo.alarmTemp[index_ch].high) = pData[pPos+2];
                HIBYTE(g_rSysConfigInfo.alarmTemp[index_ch].low) = pData[pPos+3];
                LOBYTE_ZKS(g_rSysConfigInfo.alarmTemp[index_ch].low) = pData[pPos+4];

                HIBYTE(g_rSysConfigInfo.alarmTemp[index_ch].high_1) = pData[pPos+5];
                LOBYTE_ZKS(g_rSysConfigInfo.alarmTemp[index_ch].high_1) = pData[pPos+6];
                HIBYTE(g_rSysConfigInfo.alarmTemp[index_ch].low_1) = pData[pPos+7];
                LOBYTE_ZKS(g_rSysConfigInfo.alarmTemp[index_ch].low_1) = pData[pPos+8];
                }

                pPos += 9;
                break;
            case 0x08:  // collectPeriod
                if(all_data_len - pPos < 5){ // need n data
                    return pPos -1; // return the front before id
                }

                id_data_len = pData[pPos++];// = 4; // length

                HIBYTE(HIWORD_ZKS(g_rSysConfigInfo.collectPeriod)) = pData[pPos];
                LOBYTE_ZKS(HIWORD_ZKS(g_rSysConfigInfo.collectPeriod)) = pData[pPos+1];
                HIBYTE(LOWORD_ZKS(g_rSysConfigInfo.collectPeriod)) = pData[pPos+2];
                LOBYTE_ZKS(LOWORD_ZKS(g_rSysConfigInfo.collectPeriod)) = pData[pPos+3];
                pPos += 4;
                break;
            case 0x09: // server ip
                if(all_data_len - pPos < 5){ // need n data
                    return pPos -1; // return the front before id
                }

                id_data_len = pData[pPos++];// = 4; // length

                g_rSysConfigInfo.serverIpAddr[0] = pData[pPos];
                g_rSysConfigInfo.serverIpAddr[1] = pData[pPos+1];
                g_rSysConfigInfo.serverIpAddr[2] = pData[pPos+2];
                g_rSysConfigInfo.serverIpAddr[3] = pData[pPos+3];
                pPos += 4;
                break;
            case 0x0A: // server port
                if(all_data_len - pPos < 3){ // need n data
                    return pPos -1; // return the front before id
                }

                id_data_len = pData[pPos++];// = 2; // length

                HIBYTE(g_rSysConfigInfo.serverIpPort) = pData[pPos];
                LOBYTE_ZKS(g_rSysConfigInfo.serverIpPort) = pData[pPos+1];
                pPos += 2;
                break;
            case 0x0B:  // heatbeat period
                if(all_data_len - pPos < 3){ // need n data
                    return pPos -1; // return the front before id
                }

                id_data_len = pData[pPos++];// = 2; // length

                HIBYTE(g_rSysConfigInfo.hbPeriod) = pData[pPos];
                LOBYTE_ZKS(g_rSysConfigInfo.hbPeriod) = pData[pPos+1];
                pPos += 2;
                break;
            case 0x0C:  // upload period
                if(all_data_len - pPos < 5){ // need n data
                    return pPos -1; // return the front before id
                }
                id_data_len = pData[pPos++]; //  = 4; // length

                HIBYTE(HIWORD_ZKS(g_rSysConfigInfo.uploadPeriod)) = pData[pPos];
                LOBYTE_ZKS(HIWORD_ZKS(g_rSysConfigInfo.uploadPeriod)) = pData[pPos+1];
                HIBYTE(LOWORD_ZKS(g_rSysConfigInfo.uploadPeriod)) = pData[pPos+2];
                LOBYTE_ZKS(LOWORD_ZKS(g_rSysConfigInfo.uploadPeriod)) = pData[pPos+3];
                pPos += 4;
                break;
            case 0x0D:  // NTP
                if(all_data_len - pPos < 5){ // need n data
                    return pPos -1; // return the front before id
                }
                id_data_len = pData[pPos++]; //  = 4; // length

                HIBYTE(HIWORD_ZKS(g_rSysConfigInfo.ntpPeriod)) = pData[pPos];
                LOBYTE_ZKS(HIWORD_ZKS(g_rSysConfigInfo.ntpPeriod)) = pData[pPos+1];
                HIBYTE(LOWORD_ZKS(g_rSysConfigInfo.ntpPeriod)) = pData[pPos+2];
                LOBYTE_ZKS(LOWORD_ZKS(g_rSysConfigInfo.ntpPeriod)) = pData[pPos+3];
                pPos += 4;
                break;
#ifdef SUPPORT_LORA
            case 0x0E:  // RF Power
                if(all_data_len - pPos < 2){ // need n data
                    return pPos -1; // return the front before id
                }
                id_data_len = pData[pPos++];// = 1; // length

                g_rSysConfigInfo.rfPA = pData[pPos];
                pPos += 1;
                break;
            case 0x0F:  // RF Bound width
                if(all_data_len - pPos < 2){ // need n data
                    return pPos -1; // return the front before id
                }
                id_data_len = pData[pPos++]; //  = 1; // length

                g_rSysConfigInfo.rfBW = pData[pPos];
                pPos += 1;
                break;
            case 0x10:  // RF spreading factor
                if(all_data_len - pPos < 2){ // need n data
                    return pPos -1; // return the front before id
                }
                id_data_len = pData[pPos++];// = 1; // length

                g_rSysConfigInfo.rfSF = pData[pPos];
                pPos += 1;
                break;
            case 0x11:  // RF status
                if(all_data_len - pPos < 2){ // need n data
                    return pPos -1; // return the front before id
                }
                id_data_len = pData[pPos++]; // = 1; // length

                g_rSysConfigInfo.rfStatus = pData[pPos];
                pPos += 1;
                break;
#else
            case 0x0e ... 0x11:
                pPos += 2; // len(1B)param(1B)
                break;
#endif

            case 0x12:
                if(all_data_len - pPos < 3){ // need n data
                    return pPos -1; // return the front before id
                }
                id_data_len = pData[pPos++];// = 2; // length

                HIBYTE(g_rSysConfigInfo.batLowVol) = pData[pPos];
                LOBYTE_ZKS(g_rSysConfigInfo.batLowVol) = pData[pPos+1];
                pPos += 2;
                break;
            case 0x13:  // Warning temp configure // max 8 channel
                if(all_data_len - pPos < 10){ // need n data
                    return pPos -1; // return the front before id
                }
                id_data_len = pData[pPos++]; // = 5; // length
                // first channel
                index_ch = pData[pPos]; //  = 0; // 0 channel

                if(index_ch < MODULE_SENSOR_MAX){
                HIBYTE(g_rSysConfigInfo.WarningTemp[index_ch].high) = pData[pPos+1];
                LOBYTE_ZKS(g_rSysConfigInfo.WarningTemp[index_ch].high) = pData[pPos+2];
                HIBYTE(g_rSysConfigInfo.WarningTemp[index_ch].low) = pData[pPos+3];
                LOBYTE_ZKS(g_rSysConfigInfo.WarningTemp[index_ch].low) = pData[pPos+4];

                HIBYTE(g_rSysConfigInfo.WarningTemp[index_ch].high_1) = pData[pPos+5];
                LOBYTE_ZKS(g_rSysConfigInfo.WarningTemp[index_ch].high_1) = pData[pPos+6];
                HIBYTE(g_rSysConfigInfo.WarningTemp[index_ch].low_1) = pData[pPos+7];
                LOBYTE_ZKS(g_rSysConfigInfo.WarningTemp[index_ch].low_1) = pData[pPos+8];
                //System_printf("set ch:%d hig:%d low:%d \n",index_ch,g_rSysConfigInfo.WarningTemp[j].high,g_rSysConfigInfo.WarningTemp[j].low );
                //System_flush();
                }

                pPos += 9;
                break;
            case 0x14:  // bind gateway
#ifdef S_C
                if(all_data_len - pPos < 5){ // need n data
                    return pPos -1; // return the front before id
                }
                id_data_len = pData[pPos++]; // = 4; // length

                g_rSysConfigInfo.BindGateway[0] = pData[pPos];
                g_rSysConfigInfo.BindGateway[1] = pData[pPos+1];
                g_rSysConfigInfo.BindGateway[2] = pData[pPos+2];
                g_rSysConfigInfo.BindGateway[3] = pData[pPos+3];
                pPos += 4;
#else
                pPos += 5;  // skip len(1b)param(4b)
#endif
                break;
            case 0x15:
#ifdef SUPPORT_NETGATE_BIND_NODE
                if(all_data_len - pPos < 15){ // need n data
                    return pPos -1; // return the front before id
                }
                id_data_len = pData[pPos++]; // = 10; // length
                // first channel
                index = pData[pPos]; // = 0; // 0 index
                if(index < NETGATE_BIND_NODE_MAX){
                // DID
                HIBYTE(HIWORD_ZKS(g_rSysConfigInfo.bindnode[index].Deviceid)) = pData[pPos+1];
                LOBYTE_ZKS(HIWORD_ZKS(g_rSysConfigInfo.bindnode[index].Deviceid)) = pData[pPos+2];
                HIBYTE(LOWORD_ZKS(g_rSysConfigInfo.bindnode[index].Deviceid)) = pData[pPos+3];
                LOBYTE_ZKS(LOWORD_ZKS(g_rSysConfigInfo.bindnode[index].Deviceid)) = pData[pPos+4];

                g_rSysConfigInfo.bindnode[index].ChNo = pData[pPos+5]; // channel

                HIBYTE(g_rSysConfigInfo.bindnode[index].AlarmInfo.high) = pData[pPos+6];
                LOBYTE_ZKS(g_rSysConfigInfo.bindnode[index].AlarmInfo.high) = pData[pPos+7];
                HIBYTE(g_rSysConfigInfo.bindnode[index].AlarmInfo.low) = pData[pPos+8];
                LOBYTE_ZKS(g_rSysConfigInfo.bindnode[index].AlarmInfo.low) = pData[pPos+9];

                HIBYTE(g_rSysConfigInfo.bindnode[index].AlarmInfo.high_1) = pData[pPos+10];
                LOBYTE_ZKS(g_rSysConfigInfo.bindnode[index].AlarmInfo.high_1) = pData[pPos+11];
                HIBYTE(g_rSysConfigInfo.bindnode[index].AlarmInfo.low_1) = pData[pPos+12];
                LOBYTE_ZKS(g_rSysConfigInfo.bindnode[index].AlarmInfo.low_1) = pData[pPos+13];
                }
                pPos += 14;

#else
                pPos += 11; // CMD 后面的字段不解析
#endif
                break;
            case 0x16:
                id_data_len = pData[pPos++]; // = strlen((const char*)g_rSysConfigInfo.apnuserpwd);    // length
#ifdef SUPPORT_APN_USER_PWD
                if(all_data_len - pPos < id_data_len){ // need n data
                    return pPos -2; // return the front before id
                }
                memset((char*)g_rSysConfigInfo.apnuserpwd, 0x0, sizeof(g_rSysConfigInfo.apnuserpwd)); // clear original data
                memcpy((char*)g_rSysConfigInfo.apnuserpwd,pData+pPos, pData[pPos-1]); // copy new
                pPos += pData[pPos-1];
#else
                pPos += id_data_len;
#endif
                break;
            case 0x17:

                id_data_len = pData[pPos++]; // = strlen((const char*)g_rSysConfigInfo.serverAddr);    // length

                if(all_data_len - pPos < id_data_len){ // need n data
                    return pPos -2; // return the front before id
                }

                memset((char*)g_rSysConfigInfo.serverAddr, 0x0, sizeof(g_rSysConfigInfo.serverAddr)); // clear original data
                memcpy((char*)g_rSysConfigInfo.serverAddr,pData+pPos,  pData[pPos-1]); // copy new

                pPos += pData[pPos-1];
                break;
            case ID_CALENDER_RTC_BCD : //0x18:
                if(all_data_len - pPos < 9){ // need n data
                    return pPos -1; // return the front before id
                }

                id_data_len = pData[pPos++]; // = ID_CALENDER_RTC_BCD_LEN;    // length 8
                memcpy((char*)&g_rSysConfigInfo.rtc, pData+pPos,  pData[pPos-1]);

                pPos += pData[pPos-1];
                break;
            case 0x19:
                if(all_data_len - pPos < 21){ // need n data
                    return pPos - 1; // return the front before id
                }
                id_data_len = pData[pPos++]; // = 20;    // length
                memcpy((char*)&g_rSysConfigInfo.sysState, pData+pPos, pData[pPos-1]);

                pPos += pData[pPos-1];
                break;
            case 0x1A:  //alarmuploadPeriod

                if(all_data_len - pPos < 5){ // need n data
                    return pPos -1; // return the front before id
                }

                id_data_len = pData[pPos++]; // = 4; // length

                HIBYTE(HIWORD_ZKS(g_rSysConfigInfo.alarmuploadPeriod)) = pData[pPos];
                LOBYTE_ZKS(HIWORD_ZKS(g_rSysConfigInfo.alarmuploadPeriod)) = pData[pPos+1];
                HIBYTE(LOWORD_ZKS(g_rSysConfigInfo.alarmuploadPeriod)) = pData[pPos+2];
                LOBYTE_ZKS(LOWORD_ZKS(g_rSysConfigInfo.alarmuploadPeriod)) = pData[pPos+3];
                pPos += 4;
                break;
            case ID_SERVER_ACCESS_CODE:
                id_data_len = pData[pPos++]; // length
                if(all_data_len - pPos < id_data_len){ // need n data
                    return pPos -2; // return the front before id
                }
#ifdef ZKML_PROJECT
                memset((char*)g_rSysConfigInfo.access_code, 0x0, sizeof(g_rSysConfigInfo.access_code)); // clear original data

                if(id_data_len <= ID_SERVER_ACCESS_CODE_LEN_MAX )
                    memcpy((char*)g_rSysConfigInfo.access_code,pData+pPos, id_data_len); // copy new
                else
                    memcpy((char*)g_rSysConfigInfo.access_code,pData+pPos, ID_SERVER_ACCESS_CODE_LEN_MAX); // copy new

                pPos += pData[pPos-1];
#else
                pPos += id_data_len;
#endif
                break;
            case ID_SERVER_USER_NAME:
                id_data_len = pData[pPos++]; // length
                if(all_data_len - pPos < id_data_len){ // need n data
                    return pPos -2; // return the front before id
                }
#ifdef ZKML_PROJECT
                memset((char*)g_rSysConfigInfo.user_name, 0x0, sizeof(g_rSysConfigInfo.user_name)); // clear original data

                if(id_data_len <= ID_SERVER_USER_NAME_LEN_MAX )
                    memcpy((char*)g_rSysConfigInfo.user_name,pData+pPos, id_data_len); // copy new
                else
                    memcpy((char*)g_rSysConfigInfo.user_name,pData+pPos, ID_SERVER_USER_NAME_LEN_MAX); // copy new

                pPos += pData[pPos-1];
#else
                pPos += id_data_len;
#endif
                break;
            case ID_SERVER_USER_PWD:
                id_data_len = pData[pPos++]; // length
                if(all_data_len - pPos < id_data_len){ // need n data
                    return pPos -2; // return the front before id
                }
#ifdef ZKML_PROJECT
                memset((char*)g_rSysConfigInfo.user_password, 0x0, sizeof(g_rSysConfigInfo.user_password)); // clear original data

                if(id_data_len <= ID_SERVER_USER_PWD_LEN_MAX )
                    memcpy((char*)g_rSysConfigInfo.user_password,pData+pPos, id_data_len); // copy new
                else
                    memcpy((char*)g_rSysConfigInfo.user_password,pData+pPos, ID_SERVER_USER_PWD_LEN_MAX); // copy new

                pPos += pData[pPos-1];
#else
                pPos += id_data_len;
#endif
                break;
            case ID_SENSOR_CHN_ADJUST:  // channel adjust value // max 8 channel
                if(all_data_len - pPos < 6){ // need n data
                    return pPos -1; // return the front before id
                }

                id_data_len = pData[pPos++];// = 5; // length
#ifdef ZKML_PROJECT
                // first channel
                index_ch = pData[pPos]; // = 0; // 0 channel // MODULE_SENSOR_MAX ;//8;

                if(index_ch < MODULE_SENSOR_MAX){
                HIBYTE(g_rSysConfigInfo.ChannelAdjust[index_ch].AdjustTempValue) = pData[pPos+1];
                LOBYTE_ZKS(g_rSysConfigInfo.ChannelAdjust[index_ch].AdjustTempValue) = pData[pPos+2];
                HIBYTE(g_rSysConfigInfo.ChannelAdjust[index_ch].AdjustHumiValue) = pData[pPos+3];
                LOBYTE_ZKS(g_rSysConfigInfo.ChannelAdjust[index_ch].AdjustHumiValue) = pData[pPos+4];
                }
                pPos += 5;
#else
                pPos += id_data_len;
#endif
                break;
            case 0xFF: //发送完成命令
				if(all_data_len - pPos < 1){ // need n data
                    return pPos -1; // return the front before id
                }
				id_data_len = pData[pPos++]; //1
				Flash_store_config();
				break;
            default:
                if(all_data_len - pPos < 1){ // need 1 data for id_data_len
                    return pPos -1; // return the front before id
                }
                id_data_len = pData[pPos++];//
                if(all_data_len - pPos < id_data_len){ // need n data for skip
                    return pPos -1; // return the front before id
                }
                pPos += id_data_len;    // skip this id data, read into next
                break;
                //return pPos;
        }
    }

    return pPos;
}

#endif
