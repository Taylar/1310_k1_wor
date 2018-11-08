
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
    //娑堟伅鎸囦护
    buff[length++] = msgId;

    for (i = 0; i < dataLen; i++) {
        buff[length++] = pPacket[i];
    }

    //娑堟伅闀垮害
    buff[1] = (length - 2) & 0xff;
    buff[0] = ((length - 2) >> 8) & 0xff;

    //鏍￠獙鐮�
    buff[length++] = CheckCode8(&buff[0], length);

    //杩涜杞箟
    length = Protocol_escape(&pPacket[1], &buff[0], length);

    //娑堟伅鏍囧織浣�
    pPacket[0] = PROTOCOL_TOKEN;
    pPacket[length + 1] = PROTOCOL_TOKEN;
    length += 2;

    return length;
}

#ifdef SUPPORT_USB
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


/*
 * 閻㈠彉绨崚婵嗩潗閸栨牕鐡ㄩ崒銊ュ隘閸╃喐妞� 妫ｆ牕鐔幍鍥у隘闁姤绔荤粚杞扮啊
 * 閸掆晝鏁ゅ顏嗗箚鐎涙ê鍋嶉惃鍕閹傜瑢鐏忕偓澧栭崠铏规畱閺勵垰鎯侀張澶嬫櫏閺佺増宓佺�涙ê鍋�
 * 閸欘垯浜掔拋锛勭暬閸戠儤婀侀弫鍫濈摠閸屻劍鏆熼幑顔炬畱鏉堝湱鏅�
 * */
/* 閸欘亣顕伴崜宥夋桨 DID 閺佺増宓� 閸�?Date data
 * 0-----13 <>
 * 濮ｆ梻娲块幒銉嚢 娑撯偓娑�?FLASH_SENSOR_DATA_SIZE 閼哄倸鐨� FLASH_SENSOR_DATA_SIZE - FLASH_SENSOR_HEAD_DATA_SIZE 娑擃亜鐡ч懞鍌濐嚢閸欐牗妞傞梻?
 **/
#define  FLASH_SENSOR_HEAD_DATA_SIZE 14

//#define FLASH_SENSOR_LAST_SECTOR_SENSOR_DATA_ADDR (FLASH_SENSOR_DATA_AREA_SIZE - FLASH_SECTOR_SIZE)

static uint32_t Flash_sensor_get_sensor_data_readAddr(uint32_t writeAddr, uint8_t *pData)
{
    uint32_t readAddr=0;
    int flag = 0, i;
    // last Sector valid ?
    readAddr = (uint32_t)(readAddr + FLASH_SENSOR_DATA_AREA_SIZE - FLASH_SECTOR_SIZE); // 閺堚偓閸氬簼绔存稉顏呭閸栬櫣娈戞＃鏍х摠閸屻劋缍呯純?
    memset(pData, 0xff, FLASH_SENSOR_DATA_SIZE);
    Flash_get_record(readAddr, pData, FLASH_SENSOR_DATA_SIZE); // FLASH_SENSOR_HEAD_DATA_SIZE);

    for(i = 0; i <FLASH_SENSOR_DATA_SIZE; i++ ){
        if(0xff != pData[i]){
            flag = 1;
            break;
        }
    }

    if(flag){   // valid // 閸愭瑦澧栭崠铏规畱娑撳绔存稉顏呭閸栬櫣娈戞＃鏍х摠閸屻劋缍呯純?
        readAddr = ((writeAddr/FLASH_SECTOR_SIZE)*FLASH_SECTOR_SIZE + FLASH_SECTOR_SIZE) % FLASH_SENSOR_DATA_AREA_SIZE;
    }else{
        readAddr = 0;
    }

    return readAddr;
}

static int8_t RecordFindFirstValidAddrByDID_Date( uint32_t readAddr,  uint32_t writeAddr, uint32_t *startAddr, uint8_t *tmpData, uint8_t *pData)
{
    uint32_t sumRecordNum;
    uint32_t halfRecordNum;
    uint32_t halfAddr;
    uint32_t beginAddr = readAddr,endAddr = writeAddr;

    // get sum Record
    if(readAddr > writeAddr){
        sumRecordNum = ((FLASH_SENSOR_DATA_AREA_SIZE - readAddr) + writeAddr)/FLASH_SENSOR_DATA_SIZE;
    }else{
        sumRecordNum = ((writeAddr - readAddr)/FLASH_SENSOR_DATA_SIZE);
    }

    if(tmpData[4]/* modeYear */ != 0x20){  // just support 20xx xx xx xx xx    閺嶇厧绱￠惃鍕闂傚瓨顔�
        *startAddr = writeAddr;
        return -1; //
    }

#ifndef S_G //  闂堢偟缍夐崗铏剰閸愬吀绗� 鏉堝湱鏅Λ鈧弻?
    //
    uint32_t lastRAddr; // last Record
    // 閸忓牆顕亸鐐殶閹诡喛绻樼悰灞藉灲閺�?婵″倹鐏夊▽鈩冩箒閺堝鏅ラ弮鍫曟？ 閸楁娊鈧偓閸戠儤鎮崇槐?
    lastRAddr = (readAddr + sumRecordNum*FLASH_SENSOR_DATA_SIZE)%FLASH_SENSOR_DATA_AREA_SIZE;
    // last Record judment
    Flash_get_record(lastRAddr, pData, FLASH_SENSOR_HEAD_DATA_SIZE);
    if( (datecmp(&tmpData[5], &pData[8],5)> 0)) {// No valid data in storage Region
        *startAddr = writeAddr;
        return -1;
    }
    // 妫ｆ牞顔囪ぐ鏇炲灲閺�?
    Flash_get_record(readAddr, pData, FLASH_SENSOR_HEAD_DATA_SIZE);
    if( (datecmp(&tmpData[5], &pData[8],5) < 0)) {// Valid data at the beginning
        *startAddr = readAddr;
        return -1;
    }

#endif

    while(sumRecordNum > 0){



#ifdef S_G
        BEGIN_STEP:
#endif
        halfRecordNum = (uint32_t)(sumRecordNum/2);
        halfAddr = (uint32_t)((beginAddr + halfRecordNum*FLASH_SENSOR_DATA_SIZE) % FLASH_SENSOR_DATA_AREA_SIZE);
        Flash_get_record(halfAddr, pData, FLASH_SENSOR_HEAD_DATA_SIZE);

        if(halfRecordNum == 0){
            *startAddr = endAddr;
            return 0;
        }

#ifdef S_G
        if((tmpData[0] == pData[2]) &&
            (tmpData[1] == pData[3]) &&
            (tmpData[2] == pData[4]) &&
            (tmpData[3] == pData[5])){//ID 缁楋箑鎮庨惃鍕啎婢�?
#endif
            if(beginAddr == endAddr){ // 閹兼粎鍌ㄧ�瑰本鐦�
                *startAddr = beginAddr;
                return 0;
            }

            // CMP Y M D H M // 楠�?閺�?閺�?閺�?閸�?// 娑撳秴顕粔鎺曠箻鐞涘苯顕В?
            if( datecmp(&tmpData[5], &pData[8],5) > 0) {//invalid data //halfRecord Date < startDate 閹碘偓闂団偓閺佺増宓佺挧宄邦潗閸︺劌鎮楅崡濠囧劥
                beginAddr = halfAddr;
                sumRecordNum = halfRecordNum;
                continue;
            }
            if(datecmp(&tmpData[5], &pData[8],5) < 0) {//invalid data //halfRecord Date > startDate 閹碘偓闂団偓閺佺増宓佺挧宄邦潗閸︺劌澧犻崡濠囧劥
                endAddr = halfAddr;
                sumRecordNum = halfRecordNum;
                continue;
            }

            if(0 ==  datecmp(&tmpData[5], &pData[8],5)) {//valid data //halfRecord Date == startDate 閹碘偓闂団偓閺佺増宓侀幍鎯у煂
                *startAddr = halfAddr;
                return 0;
            }

#ifdef S_G
       }else{
           uint32_t inextRecord;
           uint32_t nextAddr;
           for(inextRecord = halfRecordNum-1; inextRecord > 0 ; inextRecord--)
           {
               if ((GetUsbState() == USB_UNLINK_STATE)){
                   *startAddr = writeAddr;
                   return -1;//usb閺傤厼绱戦敍灞戒粻濮濄垺澧界悰灞烩偓?
               }

               nextAddr = (uint32_t)((beginAddr + inextRecord*FLASH_SENSOR_DATA_SIZE) % FLASH_SENSOR_DATA_AREA_SIZE);
               Flash_get_record(nextAddr, pData, FLASH_SENSOR_HEAD_DATA_SIZE);
               //Flash_get_record(nextAddr, pData, 6);    // ID

               if((tmpData[0] == pData[2]) &&
                   (tmpData[1] == pData[3]) &&
                   (tmpData[2] == pData[4]) &&
                   (tmpData[3] == pData[5])){//ID 缁楋箑鎮庨惃鍕啎婢�?

                   //Flash_get_record(nextAddr, pData+6, FLASH_SENSOR_HEAD_DATA_SIZE-6);  // TIME

                   // CMP Y M D H M // 楠�?閺�?閺�?閺�?閸�?// 娑撳秴顕粔鎺曠箻鐞涘苯顕В?
                   //invalid data //halfRecord Date < startDate 閹碘偓闂団偓閺佺増宓佺挧宄邦潗閸︺劌鎮楅崡濠囧劥
                   if((beginAddr == nextAddr) ||( datecmp(&tmpData[5], &pData[8],5) > 0) ){ // 閸撳秴宕愬▓鍨梾閺�?
                       beginAddr = halfAddr;
                       sumRecordNum = halfRecordNum;
                       goto BEGIN_STEP;
                   }

                   if(datecmp(&tmpData[5], &pData[8],5) < 0) {//invalid data //halfRecord Date > startDate 閹碘偓闂団偓閺佺増宓佺挧宄邦潗閸︺劌澧犻崡濠囧劥
                       endAddr = nextAddr;
                       sumRecordNum = inextRecord;
                       goto BEGIN_STEP;
                   }

                   if(0 ==  datecmp(&tmpData[5], &pData[8],5)) {//valid data //halfRecord Date == startDate 閹碘偓闂団偓閺佺増宓侀幍鎯у煂
                       *startAddr = nextAddr;
                       return 0;
                   }
              }
           }
           // 濞屸剝婀侀幍鎯у煂
           beginAddr = halfAddr;
           sumRecordNum = halfRecordNum;
           continue;
       }
#endif
    }
    // ...
    *startAddr = writeAddr;
    return -1;
}

//#ifdef USE_NEW_SYS_CONFIG
//#define SYS_CONFIG_USE_ID_MODE 1
//#endif

#include "config_id_mode.h"
#ifdef SYS_CONFIG_USE_ID_MODE

// ret: eq send_len
static uint16_t send_data_usb_callback(uint8_t* pData, uint16_t buf_len)
{
    uint16_t pPos = buf_len;
    pPos = Usb_group_package(AC_Send_Config_ID, pData, pPos);
    USBCDCD_sendData(pData, pPos, USB_TXRX_TIMEROUT);
    return pPos;
}

// len(2)cmd(1)data(len-1)
// data: id_start(1)id_end(1)
void EV_Get_Config_ID_parser(uint8_t* pData, uint16_t buf_len)
{
    uint16_t len;

    //length = len(2)cmd(1)data(n)  // 忙鈥⒙懊β嵚┾�♀�∶р�澛ヂぢ�
    uint16_t data_len = (pData[0] << 8) | pData[1];
    uint8_t id_start, id_end;

#if 0   // for test  // Compatible with structural body mode
    if(data_len <=1){
        memcpy((char *)pData, (char *)&g_rSysConfigInfo, sizeof(g_rSysConfigInfo));
        len = Usb_group_package(AC_Send_Config, pData, sizeof(g_rSysConfigInfo));
        USBCDCD_sendData(pData, len, USB_TXRX_TIMEROUT);
        return ; // 0x82
    }
#endif

    if(data_len == 2){
        id_start = pData[3];
        id_end = pData[3];
    }
    if(data_len == 3){
        id_start = pData[3];
        id_end = pData[4];
    }

    if(data_len <=3){
        sys_Get_Config_Id(pData,  buf_len,  id_start,  id_end , send_data_usb_callback);
    }

    return ;
}


void EV_Set_Config_ID_parser(uint8_t* pData, uint16_t length)
{
    uint16_t len;

    //length = len(2)cmd(1)data(n)  // 忙鈥⒙懊β嵚┾�♀�∶р�澛ヂぢ�
    uint16_t data_len = (pData[0] << 8) | pData[1];

    // postion to data
    sys_Set_Config_Id( pData+3, data_len-1);
    //sys_Set_Config_Id( pData+3, length-4);

    // store configure
    // Flash_store_config();

    // return ack
    pData[0] = 0;
    len = Usb_group_package(AC_Ack, pData, 1);
    USBCDCD_sendData(pData, len, USB_TXRX_TIMEROUT);

    return ;
}
#endif // SYS_CONFIG_USE_ID_MODE

//***********************************************************************************
//
// USB received data parse. For new protocol.
//
//***********************************************************************************
int Usb_data_parse(uint8_t *pData, uint16_t length)
{
#ifdef SUPPORT_BLUETOOTH_PRINT
    ErrorStatus ret;
#endif
    uint16_t len, size;
    Calendar calendar;
    uint32_t  addr,  datasize;
    uint8_t  tmpData[16],transmode;
    char     *ptr;

    uint32_t readAddr;
    uint32_t writeAddr;
#ifdef FLASH_EXTERNAL
    FlashPrintRecordAddr_t recordAddr;
#endif
    //Check package head and tail
    if (pData[0] != PROTOCOL_TOKEN || pData[length - 1] != PROTOCOL_TOKEN)
        return -1; //Not valid package.

    //Recover transferred meaning
    length = Protocol_recover_escape(&pData[0], &pData[1], length - 2);
    if (length < 4)//len(2)cmd(1)data(n)crc(1)
        return -1;

    //package check code
    if (pData[length - 1] != CheckCode8(pData, length - 1))
        return -1;

    //Remove crc, len(2)cmd(1)data(n)
    length -= 1;

// The data is valid.
    switch (pData[2]) {
        case EV_Test:
            pData[0] = 0x01;
            pData[1] = 0x01;
            pData[2] = TI_1310_PLATFORM;

            pData[3] = 0x02;
            pData[4] = strlen(PROJECT_NAME);
            memcpy((char *)(pData + 5),PROJECT_NAME, strlen(PROJECT_NAME));
            len = Usb_group_package(AC_Send_Project_Mess, pData, strlen(PROJECT_NAME) + 5);
            InterfaceSendImmediately(pData, len);
            break;

        case EV_Get_Config:
#if SYS_CONFIG_USE_ID_MODE
            EV_Get_Config_ID_parser( pData,  length);
#else
            memcpy((char *)pData, (char *)&g_rSysConfigInfo, sizeof(g_rSysConfigInfo));
            len = Usb_group_package(AC_Send_Config, pData, sizeof(g_rSysConfigInfo));
            InterfaceSendImmediately(pData, len);
#endif
            break;

        case EV_Set_Config:
#ifdef SYS_CONFIG_USE_ID_MODE
            EV_Set_Config_ID_parser(pData, length);
#else
            size = (pData[4] << 8) + pData[3];
            if (size > (length -3)) {
                size = (length -3);
            }
            if (size > sizeof(g_rSysConfigInfo)) {//涓轰簡鍓嶅悗鍏煎锛屽綋鏂版垨鏃х殑宸ュ叿鍐欏叆鏁版嵁鏃讹紝鍙彇褰撳墠鎻愪緵鐨勬垨绋嬪簭闇�瑕佺殑閮ㄥ垎銆�
                size = sizeof(g_rSysConfigInfo);                
            }

            memcpy((char *)&g_rSysConfigInfo, (char *)&pData[3], size);
            g_rSysConfigInfo.size = sizeof(g_rSysConfigInfo);            
            Sys_event_post(SYSTEMAPP_EVT_STORE_SYS_CONFIG);

            pData[0] = 0;
            len = Usb_group_package(AC_Ack, pData, 1);
            InterfaceSendImmediately(pData, len);
#endif // SYS_CONFIG_USE_ID_MODE

#ifdef  BOARD_CONFIG_DECEIVE
            RadioUpgrade_stop();
            ConcenterRadioSendParaSet(GetRadioSrcAddr(), GetRadioDstAddr());
#endif //BOARD_CONFIG_DECEIVE
            break;

        case EV_Get_APN:       
             size = strlen((const char *)g_rSysConfigInfo.apnuserpwd);
             if(size > sizeof(g_rSysConfigInfo.apnuserpwd))
                size = sizeof(g_rSysConfigInfo.apnuserpwd);
             
             memcpy((char *)pData, (char *)&g_rSysConfigInfo.apnuserpwd, size);
             len = Usb_group_package(AC_Send_APN, pData, size);
            InterfaceSendImmediately(pData, len);
             break;

        case EV_Set_APN:           
            pData[length] = '\0';
            strcpy((char *)&g_rSysConfigInfo.apnuserpwd, (char *)&pData[3]);
            Sys_event_post(SYSTEMAPP_EVT_STORE_SYS_CONFIG);
             
            pData[0] = 0;            
            len = Usb_group_package(AC_Ack, pData, 1);
            InterfaceSendImmediately(pData, len);

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
            InterfaceSendImmediately(pData, len);
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
            InterfaceSendImmediately(pData, len);
            break;

        case EV_Get_Bluetooth_Name:
#ifdef SUPPORT_BLUETOOTH_PRINT
            Btp_poweron();
            ret = Btp_get_device_name(pData);
            Btp_poweroff();
            if (ret == ES_ERROR) {
                pData[0] = 1;
                len = Usb_group_package(AC_Ack, pData, 1);
            } else {
                len = Usb_group_package(AC_Send_Bluetooth_Name, pData, strlen((char *)pData));
            }
            USBCDCD_sendData(pData, len, USB_TXRX_TIMEROUT);
#endif
            break;

        case EV_Set_Bluetooth_Name:
#ifdef SUPPORT_BLUETOOTH_PRINT
            Btp_poweron();
            pData[length] = '\0';
            if (Btp_set_device_name(&pData[3]) == ES_ERROR)
                pData[0] = 1;
            else
                pData[0] = 0;
            Btp_poweroff();
            len = Usb_group_package(AC_Ack, pData, 1);
            USBCDCD_sendData(pData, len, USB_TXRX_TIMEROUT);
#endif
            break;

		case EV_Usb_Upgrade:
#ifdef SUPPORT_USB_UPGRADE
		    Usb_BSL_data_parse(pData, length);
#endif
			break;

#ifdef  SUPPORT_USB_UPGRADE_BSL
        case EV_Upgrade_BSL:
            Usb_Upgrade_BSL_data_parse(pData, length);
            break;
#endif
        case EV_Reset_Data:
            Flash_reset_all();
            pData[0] = 0;
            len = Usb_group_package(AC_Ack, pData, 1);
            InterfaceSendImmediately(pData, len);
            __delay_cycles(60000);
            SysCtrlSystemReset();//閲嶅惎
            break;
            
		case EV_Get_History_Data://len(2B) cmd(1B)  no(2B or 4B)
            if (length == 5) { //old  support 16bit
                HIBYTE_ZKS(HIWORD_ZKS(addr)) = 0;
                LOBYTE_ZKS(HIWORD_ZKS(addr)) = 0;
                HIBYTE_ZKS(LOWORD_ZKS(addr)) = pData[3];
                LOBYTE_ZKS(LOWORD_ZKS(addr)) = pData[4];
            }
            else {    //new support 32 bit
                HIBYTE_ZKS(HIWORD_ZKS(addr)) = pData[3];
                LOBYTE_ZKS(HIWORD_ZKS(addr)) = pData[4];
                HIBYTE_ZKS(LOWORD_ZKS(addr)) = pData[5];
                LOBYTE_ZKS(LOWORD_ZKS(addr)) = pData[6];            
            }
    	    Flash_load_sensor_data_history(pData, USB_BUFF_LENGTH, addr);
            
            if(pData[0]<255)//鏈夋晥鏁版嵁
    			size = pData[0]+1;
			else
                size = 10;//鏃犳晥鏁版嵁
            
			len = Usb_group_package(AC_Send_History_Data, pData, size);
            InterfaceSendImmediately(pData, len);
			break;
            
#ifdef FLASH_EXTERNAL
        case EV_Get_Device_Data://len(2B) cmd(1B) deviceid(4B)  startdate(6B):ymdhm 201801011259 enddate(6B)   mode(1B)
#ifdef  BOARD_S6_6
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
#endif  // BOARD_S6_6
           
            memcpy(tmpData,&pData[3], 16);//deviceid(4B)  startdate(6B) enddate(6B)
            transmode = pData[19];

            writeAddr = Flash_get_sensor_writeaddr();
#if 0
            // 氓沤鸥氓搂鈥姑�斆β斥��
            readAddr = (writeAddr + FLASH_SENSOR_DATA_AREA_SIZE - FLASH_SENSOR_DATA_SIZE) % FLASH_SENSOR_DATA_AREA_SIZE;
            // Find first Record date optimize
            /* ALG: CMP YMDHM, NOT CMP DID , FIND FIRST  Date record,盲陆驴莽鈥澛� 莽潞驴忙鈧�?忙聬艙莽麓垄  */
            while(1) {

                if ((GetUsbState() == USB_UNLINK_STATE))
                    return;//usb忙鈥撀ヂ尖偓茂录艗氓聛艙忙颅垄忙鈥奥∨捗ｂ偓?

#ifdef      SUPPORT_WATCHDOG
                WdtClear();
#endif  


                Flash_get_record(readAddr, pData, FLASH_SENSOR_HEAD_DATA_SIZE);
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
                    (tmpData[3] == 0xff)) ||//璁惧鎵�鏈夋暟鎹紝涓嶅垽鏂璬eviceid
                   ((tmpData[0] == pData[2]) && 
                    (tmpData[1] == pData[3]) && 
                    (tmpData[2] == pData[4]) && 
                    (tmpData[3] == pData[5]))){//ID 绗﹀悎鐨勮澶�
                    
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
#else
            // Use optimize algorithm to find out the first valid record address
            readAddr = Flash_sensor_get_sensor_data_readAddr( writeAddr, pData); // 忙艙鈥懊︹�⑺喢韭姑р�⑴�
    #ifdef S_G
            // 忙艙陋忙艗鈥∶ヂID,忙艗鈥∶ヂ∶︹�斅睹┾�斅疵︹�撀♀�灻︹�扳偓忙艙鈥懊︹�⒙懊β嵚┢捖矫β伱ヂ济モ�÷好寂捗┾�♀�∶р�澛郝棵︹偓搂忙聬艙莽麓?忙鈥奥久ニ喡懊ぢ糕偓盲赂?忙鈥斅睹┾�斅疵γヂ愃喢♀�灻︹�⒙懊β�?
            if((tmpData[0] == 0xff) &&
                (tmpData[1] == 0xff) &&
                (tmpData[2] == 0xff) &&
                (tmpData[3] == 0xff))
           {
                while(1) {

                    if ((GetUsbState() == USB_UNLINK_STATE))
                        return 0;//usb忙鈥撀ヂ尖偓茂录艗氓聛艙忙颅垄忙鈥奥∨捗ｂ偓?
#ifdef      SUPPORT_WATCHDOG
                    WdtClear();
#endif  

                    Flash_get_record(readAddr, pData, FLASH_SENSOR_HEAD_DATA_SIZE);
                    if((tmpData[4] == 0x20) && (datecmp(&tmpData[5], &pData[8],5) < 0)){//find   date < startdate
                        // 猫驴鈥澝モ�号久ぢ糕偓盲赂陋氓陇搂盲潞沤忙鸥楼猫炉垄忙鈥斅睹┾�斅疵♀�� 氓鈥奥嵜ぢ糕偓盲赂陋忙艙鈥懊︹�⑺喢ヂ溍モ�毬ぢ铰嵜�?
                        readAddr = (readAddr + FLASH_SENSOR_DATA_AREA_SIZE -  FLASH_SENSOR_DATA_SIZE) % FLASH_SENSOR_DATA_AREA_SIZE;
                        break;
                    }

                    readAddr = (readAddr +  FLASH_SENSOR_DATA_SIZE) % FLASH_SENSOR_DATA_AREA_SIZE;
                    if(readAddr == writeAddr){//all data 'date < startdate
                        readAddr = writeAddr;
                        break;
                    }
                }
            }else
    #endif
            {
                #if 0 // for test
                readAddr = (uint32_t)0;
                //writeAddr = (uint32_t)(1000*FLASH_SENSOR_DATA_SIZE);
                writeAddr = (uint32_t)(64000); // 1000 忙聺隆猫庐隆氓陆?
                #endif
                /* 忙鈧澝ζ捖趁寂∶ぢ铰棵р��?莽潞驴忙鈧β惻撁�?+ 盲潞艗氓藛鈥犆β斥�⒚ε嘎ッ︹��?+ 忙篓隆莽鲁艩氓藛鈥犆β得郝棵︹偓搂忙鸥楼忙鈥�?氓掳陆氓驴芦氓庐拧盲陆聧氓藛?氓庐啪茅鈩⑩�γ┡撯偓猫娄聛猫炉禄茅艙鈧♀�灻︹�⒙懊β�?氓艙掳氓聺鈧� */
                uint32_t startAddr=0;
                RecordFindFirstValidAddrByDID_Date( readAddr,  writeAddr, &startAddr, tmpData, pData);
                readAddr = startAddr;
            }
#endif

            if(readAddr == writeAddr) {//no find
                pData[0] = 0;
                len = Usb_group_package(AC_Ack, pData, 1);
                InterfaceSendImmediately(pData, len);
            }
            else {
                
                while ((readAddr != writeAddr) ){
#ifdef      SUPPORT_WATCHDOG
                    WdtClear();
#endif                      
                    Flash_get_record(readAddr, pData, FLASH_SENSOR_DATA_SIZE);
                    // 閺堝鏅ID
                    if((0xff != pData[2]) &&
                        (0xff != pData[3]) &&
                        (0xff != pData[4]) &&
                        (0xff != pData[5]))
                    if(((tmpData[0] == 0xff) && 
                        (tmpData[1] == 0xff) && 
                        (tmpData[2] == 0xff) && 
                        (tmpData[3] == 0xff)) ||//璁惧鎵�鏈夋暟鎹紝涓嶅垽鏂璬eviceid
                       ((tmpData[0] == pData[2]) && 
                        (tmpData[1] == pData[3]) && 
                        (tmpData[2] == pData[4]) && 
                        (tmpData[3] == pData[5]))){//ID 绗﹀悎鐨勮澶�
                        
                        if((tmpData[10] == 0x20) && (datecmp(&tmpData[11], &pData[8],5)>= 0)){//data <= enddate
                            
                            if(transmode){//涓嶉渶瑕佹墦鍖咃紝浼犲師濮嬫暟鎹�
                                len =  pData[0]+1;
                            }
                            else{
                                len = Usb_group_package(AC_Send_Device_Data, pData, pData[0]+1);                     
                            }

                            InterfaceSendImmediately(pData, len);
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
            InterfaceSendImmediately(pData, len);
                
            break;
            
        case EV_Set_DevicePara://len(2B) cmd(1B)  para  chk
            if(SetDevicePara(pData+3, length-3))
                pData[0] = 0;     
            else
                pData[0] = 1;
           
            len = Usb_group_package(AC_Ack, pData, 1);
            InterfaceSendImmediately(pData, len);
            
            break;        
#ifdef FLASH_EXTERNAL
        case EV_Get_Unupload_Data://len(2B) cmd(1B)  mode(1B) chk(1B)
#ifdef SUPPORT_MENU
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
#endif // G7_PROJECT
#endif //SUPPORT_MENU

            transmode = pData[3];
            
            readAddr = Flash_get_sensor_readaddr();
            writeAddr = Flash_get_sensor_writeaddr();
            
            while ((readAddr != writeAddr)){
                Flash_get_record(readAddr, pData, FLASH_SENSOR_DATA_SIZE);
                
                if(transmode){//涓嶉渶瑕佹墦鍖咃紝浼犲師濮嬫暟鎹�
                    len =  pData[0]+1;
                }
                else{
                    len = Usb_group_package(AC_Send_Unupload_Data, pData, pData[0]+1);                     
                }

                InterfaceSendImmediately(pData, len);
                __delay_cycles(60000);

                readAddr = (readAddr + FLASH_SENSOR_DATA_SIZE) % FLASH_SENSOR_DATA_AREA_SIZE;
            }
            
            break;
            
        case EV_Get_Record_Data://len(2B) cmd(1B) mode(1B) chk(1B)
#ifdef SUPPORT_MENU
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
#endif // G7_PROJECT
#endif //SUPPORT_MENU

            transmode = pData[3];
            
            recordAddr  = Flash_get_record_addr();
            readAddr = recordAddr.start;
            writeAddr = recordAddr.end;
            
            while ((readAddr != writeAddr)){
                Flash_get_record(readAddr, pData, FLASH_SENSOR_DATA_SIZE);
                
                if(transmode){//涓嶉渶瑕佹墦鍖咃紝浼犲師濮嬫暟鎹�
                    len =  pData[0]+1;
                }
                else{
                    len = Usb_group_package(AC_Send_Record_Data, pData, pData[0]+1);                     
                }

                InterfaceSendImmediately(pData, len);
                __delay_cycles(60000);

                readAddr = (readAddr + FLASH_SENSOR_DATA_SIZE) % FLASH_SENSOR_DATA_AREA_SIZE;
            }

            break;

        case EV_Get_ReadFlash://len(2B) cmd(1B) addr(4B)  size(4B)  mode(1B) chk
            HIBYTE_ZKS(HIWORD_ZKS(addr)) = pData[3];
            LOBYTE_ZKS(HIWORD_ZKS(addr)) = pData[4];
            HIBYTE_ZKS(LOWORD_ZKS(addr)) = pData[5];
            LOBYTE_ZKS(LOWORD_ZKS(addr)) = pData[6];    
            HIBYTE_ZKS(HIWORD_ZKS(datasize)) = pData[7];
            LOBYTE_ZKS(HIWORD_ZKS(datasize)) = pData[8];
            HIBYTE_ZKS(LOWORD_ZKS(datasize)) = pData[9];
            LOBYTE_ZKS(LOWORD_ZKS(datasize)) = pData[10];                
            transmode = pData[11];

            
            size  = 256;

            while(datasize > 0 ){

                if(datasize < 256)
                    size  = datasize;
                
                Flash_read_rawdata(addr, pData, size); 
                
                if(transmode){//涓嶉渶瑕佹墦鍖咃紝浼犲師濮嬫暟鎹�
                    len = size;
                }
                else{
                    len = Usb_group_package(EV_Send_ReadFlash, pData, size);
                }
                
                InterfaceSendImmediately(pData, len);
                __delay_cycles(60000);

                datasize -= size;                 
                addr += size;
            }
            break;
#endif

        case EV_Verify_Code:
            ptr = strstr((char *)pData+1, USB_DECEIVE_CODE);
            if(ptr != NULL)
            {
                pData[0] = 0;
            }
            else
            {
                pData[0] = 1;
            }
            len = Usb_group_package(AC_Ack, pData, 1);
            InterfaceSendImmediately(pData, len);
            break;

		case EV_Get_SIM_CCID:
            pData[0] = 20;
			Nwk_get_simccid(pData+1);
            len = Usb_group_package(EV_Send_SIM_CCID, pData, 21);
            InterfaceSendImmediately(pData, len);
            break;

        default:
            pData[0] = 2;
            len = Usb_group_package(AC_Ack, pData, 1);
            InterfaceSendImmediately(pData, len);
            return -1;
    }
	return 0; // No Error
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
        InterfaceSendImmediately(bUsbBuff, len);
        break;

        case AC_Send_Config:
        memcpy((char *)bUsbBuff, (char *)&g_rSysConfigInfo, sizeof(g_rSysConfigInfo));
        len = Usb_group_package(AC_Send_Config, bUsbBuff, sizeof(g_rSysConfigInfo));
        InterfaceSendImmediately(bUsbBuff, len);
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
        InterfaceSendImmediately(bUsbBuff, len);
        break;

        case AC_Send_Bluetooth_Name:

        break;

    }
}
#endif //SUPPORT_USB
