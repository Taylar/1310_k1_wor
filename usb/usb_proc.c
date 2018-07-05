
//***********************************************************************************
// Copyright 2017, Zksiot Development Ltd.
// Created by Linjie, 2017.08.08
// MCU:	MSP430F5529
// OS: TI-RTOS
// Project:
// File name: usb_proc.c
// Description: usb process routine.
//***********************************************************************************
#include "../general.h"


uint8_t bUsbBuff[USB_BUFF_LENGTH];


//***********************************************************************************
//
// Usb protocol group package.
//
//***********************************************************************************
uint16_t Usb_group_package(USB_TX_MSG_ID msgId, uint8_t *pPacket, uint16_t dataLen)
{
    uint8_t  buff[USB_BUFF_LENGTH];
    uint16_t length,i;

    length = 2;
    //消息指令
    buff[length++] = msgId;

    for (i = 0; i < dataLen; i++) {
        buff[length++] = pPacket[i];
    }

    //消息长度
    buff[1] = (length - 2) & 0xff;
    buff[0] = ((length - 2) >> 8) & 0xff;

    //校验码
    buff[length++] = CheckCode8(&buff[0], length);

    //进行转义
    length = Protocol_escape(&pPacket[1], &buff[0], length);

    //消息标志位
    pPacket[0] = PROTOCOL_TOKEN;
    pPacket[length + 1] = PROTOCOL_TOKEN;
    length += 2;

    return length;
}

static int8_t datecmp(uint8_t* date1, uint8_t* date2, uint8_t len)
{
    uint8_t i;

    for(i = 0; i < len; ++i){

        if(*date1 > *date2)
            return 1;
        
        if(*date1 < *date2)
            return -1;

        date1++;
        date2++;
    }

    return 0;
}
//***********************************************************************************
//
// USB received data parse. For new protocl.
//
//***********************************************************************************
void Usb_data_parse(uint8_t *pData, uint16_t length)
{
    uint16_t len, size;
    Calendar calendar;
    uint32_t  addr,  datasize;
    uint8_t  tmpData[16],transmode;

    uint32_t readAddr;
    uint32_t writeAddr;
#ifdef FLASH_EXTERNAL
    FlashPrintRecordAddr_t recordAddr;
#endif
    //Check package head and tail
    if (pData[0] != PROTOCOL_TOKEN || pData[length - 1] != PROTOCOL_TOKEN)
        return; //Not valid package.

    //Recover transferred meaning
    length = Protocol_recover_escape(&pData[0], &pData[1], length - 2);
    if (length < 4)//len(2)cmd(1)data(n)crc(1)
        return;

    //package check code
    if (pData[length - 1] != CheckCode8(pData, length - 1))
        return;

    //Remove crc, len(2)cmd(1)data(n)
    length -= 1;

// The data is valid.
    switch (pData[2]) {
        case EV_Test:
            pData[0] = 0;
            len = Usb_group_package(AC_Ack, pData, 1);
            InterfaceSend(pData, len);
            break;

        case EV_Get_Config:
            memcpy((char *)pData, (char *)&g_rSysConfigInfo, sizeof(g_rSysConfigInfo));
            len = Usb_group_package(AC_Send_Config, pData, sizeof(g_rSysConfigInfo));
            InterfaceSend(pData, len);
            break;

        case EV_Set_Config:
            size = (pData[4] << 8) + pData[3];
            if (size > (length -3)) {
                size = (length -3);
            }
            if (size > sizeof(g_rSysConfigInfo)) {//为了前后兼容，当新或旧的工具写入数据时，只取当前提供的或程序需要的部分。
                size = sizeof(g_rSysConfigInfo);                
            }

            memcpy((char *)&g_rSysConfigInfo, (char *)&pData[3], size);
            g_rSysConfigInfo.size = sizeof(g_rSysConfigInfo);            
            Sys_event_post(SYSTEMAPP_EVT_STORE_SYS_CONFIG);

            pData[0] = 0;
            len = Usb_group_package(AC_Ack, pData, 1);
            InterfaceSend(pData, len);

#ifdef  BOARD_CONFIG_DECEIVE
            RadioUpgrade_stop();
            ConcenterRadioSendParaSet(GetRadioSrcAddr(), GetRadioDstAddr());
#endif
            break;

        case EV_Get_APN:       
             size = strlen((const char *)g_rSysConfigInfo.apnuserpwd);
             if(size > sizeof(g_rSysConfigInfo.apnuserpwd))
                size = sizeof(g_rSysConfigInfo.apnuserpwd);
             
             memcpy((char *)pData, (char *)&g_rSysConfigInfo.apnuserpwd, size);
             len = Usb_group_package(AC_Send_APN, pData, size);
            InterfaceSend(pData, len);
             break;

        case EV_Set_APN:           
            pData[length] = '\0';
            strcpy((char *)&g_rSysConfigInfo.apnuserpwd, (char *)&pData[3]);
            Sys_event_post(SYSTEMAPP_EVT_STORE_SYS_CONFIG);
             
            pData[0] = 0;            
            len = Usb_group_package(AC_Ack, pData, 1);
            InterfaceSend(pData, len);

            break;

        case EV_Get_Calendar:
            calendar = Rtc_get_calendar();

            calendar.Year       = TransHexToBcd((uint8_t)(calendar.Year - 2000)) + 0x2000;
            calendar.Month      = TransHexToBcd((uint8_t)(calendar.Month));
            calendar.DayOfMonth = TransHexToBcd((uint8_t)(calendar.DayOfMonth));
            calendar.Hours      = TransHexToBcd((uint8_t)(calendar.Hours));
            calendar.Minutes    = TransHexToBcd((uint8_t)(calendar.Minutes));
            calendar.Seconds    = TransHexToBcd((uint8_t)(calendar.Seconds));

            memcpy((char *)pData, (char *)&calendar, sizeof(Calendar));
            len = Usb_group_package(AC_Send_Calendar, pData, sizeof(Calendar));
            InterfaceSend(pData, len);
            break;

        case EV_Set_Calendar:
            memcpy((char *)&calendar, (char *)&pData[3], sizeof(Calendar));

            calendar.Year       = TransBcdToHex((uint8_t)(calendar.Year)) + 2000;
            calendar.Month      = TransBcdToHex((uint8_t)(calendar.Month));
            calendar.DayOfMonth = TransBcdToHex((uint8_t)(calendar.DayOfMonth));
            calendar.Hours      = TransBcdToHex((uint8_t)(calendar.Hours));
            calendar.Minutes    = TransBcdToHex((uint8_t)(calendar.Minutes));
            calendar.Seconds    = TransBcdToHex((uint8_t)(calendar.Seconds));

            Rtc_set_calendar(&calendar);
            pData[0] = 0;
            len = Usb_group_package(AC_Ack, pData, 1);
            InterfaceSend(pData, len);
            break;

        case EV_Get_Bluetooth_Name:
            break;

        case EV_Set_Bluetooth_Name:
            break;

		case EV_Usb_Upgrade:
#ifdef SUPPORT_USB_UPGRADE
            Usb_BSL_data_parse(pData, length);
#endif
			break;

        case EV_Reset_Data:
            Flash_reset_all();
            pData[0] = 0;
            len = Usb_group_package(AC_Ack, pData, 1);
            InterfaceSend(pData, len);
            __delay_cycles(60000);
            SysCtrlSystemReset();//重启
            break;
            
		case EV_Get_History_Data://len(2B) cmd(1B)  no(2B or 4B)
            if (length == 5) { //old  support 16bit
                HIBYTE(HIWORD(addr)) = 0;
                LOBYTE(HIWORD(addr)) = 0;
                HIBYTE(LOWORD(addr)) = pData[3];
                LOBYTE(LOWORD(addr)) = pData[4];
            }
            else {    //new support 32 bit
                HIBYTE(HIWORD(addr)) = pData[3];
                LOBYTE(HIWORD(addr)) = pData[4];
                HIBYTE(LOWORD(addr)) = pData[5];
                LOBYTE(LOWORD(addr)) = pData[6];            
            }
    	    Flash_load_sensor_data_history(pData, USB_BUFF_LENGTH, addr);
            
            if(pData[0]<255)//有效数据
    			size = pData[0]+1;
			else
                size = 10;//无效数据
            
			len = Usb_group_package(AC_Send_History_Data, pData, size);
            InterfaceSend(pData, len);
			break;
            
#ifdef FLASH_EXTERNAL
        case EV_Get_Device_Data://len(2B) cmd(1B) deviceid(4B)  startdate(6B):ymdhm 201801011259 enddate(6B)   mode(1B)
            if(Menu_is_record())
            {
                Menu_set_record(0);
                Flash_store_record_addr(0);
#ifdef  G7_PROJECT                
                BlePrintRecordStopNotify();
            }
            BlePrintingRecordNotify();
#else
            }
#endif  // G7_PROJECT
           
            memcpy(tmpData,&pData[3], 16);//deviceid(4B)  startdate(6B) enddate(6B)
            transmode = pData[19];

            writeAddr = Flash_get_sensor_writeaddr();
            readAddr = (writeAddr + FLASH_SENSOR_DATA_AREA_SIZE - FLASH_SENSOR_DATA_SIZE) % FLASH_SENSOR_DATA_AREA_SIZE;

            //find fisrt record
            while(1) {

                Flash_get_record(readAddr, pData, FLASH_SENSOR_DATA_SIZE);

                if( (0xff == pData[8]) ||
                    (0xff == pData[9]) ||
                    (0xff == pData[10])||
                    (0xff == pData[11])||
                    (0xff == pData[12])) {//invalid data
                    readAddr = (readAddr + FLASH_SENSOR_DATA_SIZE) % FLASH_SENSOR_DATA_AREA_SIZE;
                    break;
                }

                if(((tmpData[0] == 0xff) && 
                    (tmpData[1] == 0xff) && 
                    (tmpData[2] == 0xff) && 
                    (tmpData[3] == 0xff)) ||//设备所有数据，不判断deviceid
                   ((tmpData[0] == pData[2]) && 
                    (tmpData[1] == pData[3]) && 
                    (tmpData[2] == pData[4]) && 
                    (tmpData[3] == pData[5]))){//ID 符合的设备
                    
                    if((tmpData[4] == 0x20) && (datecmp(&tmpData[5], &pData[8],5)> 0)){//find   date < startdate    
                        readAddr = (readAddr + FLASH_SENSOR_DATA_SIZE) % FLASH_SENSOR_DATA_AREA_SIZE;
                        break;
                    }
               }                    
                
                readAddr = (readAddr + FLASH_SENSOR_DATA_AREA_SIZE - FLASH_SENSOR_DATA_SIZE) % FLASH_SENSOR_DATA_AREA_SIZE;

                if(readAddr == writeAddr){//all data 'date > startdate
                    readAddr = (readAddr + FLASH_SENSOR_DATA_SIZE) % FLASH_SENSOR_DATA_AREA_SIZE;
                    break;
                }
            }

            if(readAddr == writeAddr) {//no find
                pData[0] = 0;
                len = Usb_group_package(AC_Ack, pData, 1);
                InterfaceSend(pData, len);
            }
            else {
                
                while ((readAddr != writeAddr) ){
                    
                    Flash_get_record(readAddr, pData, FLASH_SENSOR_DATA_SIZE);
                    
                    if(((tmpData[0] == 0xff) && 
                        (tmpData[1] == 0xff) && 
                        (tmpData[2] == 0xff) && 
                        (tmpData[3] == 0xff)) ||//设备所有数据，不判断deviceid
                       ((tmpData[0] == pData[2]) && 
                        (tmpData[1] == pData[3]) && 
                        (tmpData[2] == pData[4]) && 
                        (tmpData[3] == pData[5]))){//ID 符合的设备
                        
                        if((tmpData[10] == 0x20) && (datecmp(&tmpData[11], &pData[8],5)>= 0)){//data <= enddate
                            
                            if(transmode){//不需要打包，传原始数据
                                len =  pData[0]+1;
                            }
                            else{
                                len = Usb_group_package(AC_Send_Device_Data, pData, pData[0]+1);                     
                            }

                            InterfaceSend(pData, len);
                            __delay_cycles(60000);
                        }
                        else{
                            break;//data > enddate
                        }
                    }
                    readAddr = (readAddr + FLASH_SENSOR_DATA_SIZE) % FLASH_SENSOR_DATA_AREA_SIZE;                    
                    
                }
            }
            break;
#endif
        case EV_Get_DevicePara://len(2B) cmd(1B)  para  chk
        
            len = GetDevicePara(pData[3],pData);
            len = Usb_group_package(AC_Send_DevicePara, pData, len);
            InterfaceSend(pData, len);
                
            break;
            
        case EV_Set_DevicePara://len(2B) cmd(1B)  para  chk
            if(SetDevicePara(pData+3, length-3))
                pData[0] = 0;     
            else
                pData[0] = 1;
           
            len = Usb_group_package(AC_Ack, pData, 1);
            InterfaceSend(pData, len);
            
            break;        
#ifdef FLASH_EXTERNAL
        case EV_Get_Unupload_Data://len(2B) cmd(1B)  mode(1B) chk(1B)
            transmode = pData[3];
            
            readAddr = Flash_get_sensor_readaddr();
            writeAddr = Flash_get_sensor_writeaddr();
            
            while ((readAddr != writeAddr)){
                Flash_get_record(readAddr, pData, FLASH_SENSOR_DATA_SIZE);
                
                if(transmode){//不需要打包，传原始数据
                    len =  pData[0]+1;
                }
                else{
                    len = Usb_group_package(AC_Send_Unupload_Data, pData, pData[0]+1);                     
                }

                InterfaceSend(pData, len);
                __delay_cycles(60000);

                readAddr = (readAddr + FLASH_SENSOR_DATA_SIZE) % FLASH_SENSOR_DATA_AREA_SIZE;
            }
            
            break;
            
        case EV_Get_Record_Data://len(2B) cmd(1B) mode(1B) chk(1B)
            transmode = pData[3];
            
            recordAddr  = Flash_get_record_addr();
            readAddr = recordAddr.start;
            writeAddr = recordAddr.end;
            
            while ((readAddr != writeAddr)){
                Flash_get_record(readAddr, pData, FLASH_SENSOR_DATA_SIZE);
                
                if(transmode){//不需要打包，传原始数据
                    len =  pData[0]+1;
                }
                else{
                    len = Usb_group_package(AC_Send_Record_Data, pData, pData[0]+1);                     
                }

                InterfaceSend(pData, len);
                __delay_cycles(60000);

                readAddr = (readAddr + FLASH_SENSOR_DATA_SIZE) % FLASH_SENSOR_DATA_AREA_SIZE;
            }

            break;

        case EV_Get_ReadFlash://len(2B) cmd(1B) addr(4B)  size(4B)  mode(1B) chk
            HIBYTE(HIWORD(addr)) = pData[3];
            LOBYTE(HIWORD(addr)) = pData[4];
            HIBYTE(LOWORD(addr)) = pData[5];
            LOBYTE(LOWORD(addr)) = pData[6];    
            HIBYTE(HIWORD(datasize)) = pData[7];
            LOBYTE(HIWORD(datasize)) = pData[8];
            HIBYTE(LOWORD(datasize)) = pData[9];
            LOBYTE(LOWORD(datasize)) = pData[10];                
            transmode = pData[11];

            
            size  = 256;

            while(datasize > 0 ){

                if(datasize < 256)
                    size  = datasize;
                
                Flash_read_rawdata(addr, pData, size); 
                
                if(transmode){//不需要打包，传原始数据
                    len = size;
                }
                else{
                    len = Usb_group_package(EV_Send_ReadFlash, pData, size);
                }
                
                InterfaceSend(pData, len);
                __delay_cycles(60000);

                datasize -= size;                 
            }
            break;
#endif
        default:
            pData[0] = 2;
            len = Usb_group_package(AC_Ack, pData, 1);
            InterfaceSend(pData, len);
            break;
    }
}



void UsbSend(USB_TX_MSG_ID msgId)
{
    uint16_t len;
    Calendar calendar;

    switch(msgId)
    {
        case AC_Ack:
        bUsbBuff[0] = 0;
        len = Usb_group_package(AC_Ack, bUsbBuff, 1);
        InterfaceSend(bUsbBuff, len);
        break;

        case AC_Send_Config:
        memcpy((char *)bUsbBuff, (char *)&g_rSysConfigInfo, sizeof(g_rSysConfigInfo));
        len = Usb_group_package(AC_Send_Config, bUsbBuff, sizeof(g_rSysConfigInfo));
        InterfaceSend(bUsbBuff, len);
        break;

        case AC_Send_APN:

        break;

        case AC_Send_Calendar:
        calendar = Rtc_get_calendar();

        calendar.Year       = TransHexToBcd((uint8_t)(calendar.Year - 2000)) + 0x2000;
        calendar.Month      = TransHexToBcd((uint8_t)(calendar.Month));
        calendar.DayOfMonth = TransHexToBcd((uint8_t)(calendar.DayOfMonth));
        calendar.Hours      = TransHexToBcd((uint8_t)(calendar.Hours));
        calendar.Minutes    = TransHexToBcd((uint8_t)(calendar.Minutes));
        calendar.Seconds    = TransHexToBcd((uint8_t)(calendar.Seconds));

        memcpy((char *)bUsbBuff, (char *)&calendar, sizeof(Calendar));
        len = Usb_group_package(AC_Send_Calendar, bUsbBuff, sizeof(Calendar));
        InterfaceSend(bUsbBuff, len);
        break;

        case AC_Send_Bluetooth_Name:

        break;

    }
}
