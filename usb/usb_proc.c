
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

#define USB_TXRX_TIMEROUT           (100L * 1000 / Clock_tickPeriod)
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
    //濞戝牊浼呴幐鍥︽姢
    buff[length++] = msgId;

    for (i = 0; i < dataLen; i++) {
        buff[length++] = pPacket[i];
    }

    //濞戝牊浼呴梹鍨
    buff[1] = (length - 2) & 0xff;
    buff[0] = ((length - 2) >> 8) & 0xff;

    //閺嶏繝鐛欓惍锟�
    buff[length++] = CheckCode8(&buff[0], length);

    //鏉╂稖顢戞潪顑跨疅
    length = Protocol_escape(&pPacket[1], &buff[0], length);

    //濞戝牊浼呴弽鍥х箶娴ｏ拷
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
 * 闁汇垹褰夌花顒勫礆濠靛棭娼楅柛鏍ㄧ墪閻°劑宕掗妸銉ラ殬闁糕晝鍠愬锟� 濡絾鐗曢悢顒勫箥閸パ冮殬闂侇喛濮ょ粩鑽ょ矚鏉炴壆鍟�
 * 闁告巻鏅濋弫銈咁嚗椤忓棗绠氶悗娑櫭崑宥夋儍閸曨厼顥楅柟顑倻鐟㈤悘蹇曞亾婢ф牠宕犻搹瑙勭暠闁哄嫷鍨伴幆渚�寮垫径瀣珡闁轰胶澧楀畵浣猴拷娑櫭崑锟�
 * 闁告瑯鍨禍鎺旀媼閿涘嫮鏆柛鎴犲劋濠�渚�寮崼婵堟憼闁稿被鍔嶉弳鐔煎箲椤旂偓鐣遍弶鍫濇贡閺咃拷
 * */
/* 闁告瑯浜ｉ浼村礈瀹ュ妗� DID 闁轰胶澧楀畵锟� 闁革拷?Date data
 * 0-----13 <>
 * 婵絾姊诲ú鍧楀箳閵夘煈鍤� 濞戞挴鍋撳☉锟�?FLASH_SENSOR_DATA_SIZE 闁煎搫鍊搁惃锟� FLASH_SENSOR_DATA_SIZE - FLASH_SENSOR_HEAD_DATA_SIZE 濞戞搩浜滈悺褔鎳為崒婵愬殺闁告瑦鐗楀鍌炴⒒?
 **/
#define  FLASH_SENSOR_HEAD_DATA_SIZE 14

//#define FLASH_SENSOR_LAST_SECTOR_SENSOR_DATA_ADDR (FLASH_SENSOR_DATA_AREA_SIZE - FLASH_SECTOR_SIZE)

static uint32_t Flash_sensor_get_sensor_data_readAddr(uint32_t writeAddr, uint8_t *pData)
{
    uint32_t readAddr=0;
    int flag = 0, i;
    // last Sector valid ?
    readAddr = (uint32_t)(readAddr + FLASH_SENSOR_DATA_AREA_SIZE - FLASH_SECTOR_SIZE); // 闁哄牃鍋撻柛姘凹缁斿瓨绋夐鍛暬闁告牞娅ｅ▓鎴烇純閺嵮呮憼闁稿被鍔嬬紞鍛磾?
    memset(pData, 0xff, FLASH_SENSOR_DATA_SIZE);
    Flash_get_record(readAddr, pData, FLASH_SENSOR_DATA_SIZE); // FLASH_SENSOR_HEAD_DATA_SIZE);

    for(i = 0; i <FLASH_SENSOR_DATA_SIZE; i++ ){
        if(0xff != pData[i]){
            flag = 1;
            break;
        }
    }

    if(flag){   // valid // 闁告劖鐟︽晶鏍礌閾忚鐣卞☉鎾愁儎缁斿瓨绋夐鍛暬闁告牞娅ｅ▓鎴烇純閺嵮呮憼闁稿被鍔嬬紞鍛磾?
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

    if(tmpData[4]/* modeYear */ != 0x20){  // just support 20xx xx xx xx xx    闁哄秶鍘х槐锟犳儍閸曨剚顦ч梻鍌氱摠椤旓拷
        *startAddr = writeAddr;
        return -1; //
    }

#ifndef S_G //  闂傚牏鍋熺紞澶愬礂閾忣偄鍓伴柛鎰悁缁楋拷 閺夊牆婀遍弲顐⑽涢埀顒勫蓟?
    //
    uint32_t lastRAddr; // last Record
    // 闁稿繐鐗嗛顔句焊閻愵剚娈堕柟璇″枦缁绘鎮扮仦钘夌伈闁猴拷?濠碘�冲�归悘澶娾柦閳╁啯绠掗柡鍫濐槹閺呫儵寮崼鏇燂紵 闁告濞婇埀顑藉亾闁告垹鍎ら幃宕囨?
    lastRAddr = (readAddr + sumRecordNum*FLASH_SENSOR_DATA_SIZE)%FLASH_SENSOR_DATA_AREA_SIZE;
    // last Record judment
    Flash_get_record(lastRAddr, pData, FLASH_SENSOR_HEAD_DATA_SIZE);
    if( (datecmp(&tmpData[5], &pData[8],5)> 0)) {// No valid data in storage Region
        *startAddr = writeAddr;
        return -1;
    }
    // 濡絾鐗為鍥亹閺囩偛鐏查柡锟�?
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
            (tmpData[3] == pData[5])){//ID 缂佹绠戦幃搴ㄦ儍閸曨噮鍟庡锟�?
#endif
            if(beginAddr == endAddr){ // 闁瑰吋绮庨崒銊э拷鐟版湰閻︼拷
                *startAddr = beginAddr;
                return 0;
            }

            // CMP Y M D H M // 妤狅拷?闁猴拷?闁猴拷?闁猴拷?闁革拷?// 濞戞挸绉撮顔剧矓閹烘洜绠婚悶娑樿嫰椤曨喖袙?
            if( datecmp(&tmpData[5], &pData[8],5) > 0) {//invalid data //halfRecord Date < startDate 闁圭鍋撻梻鍥ｅ亾闁轰胶澧楀畵浣烘導瀹勯偊娼楅柛锔哄妼閹宕℃繝鍥у姤
                beginAddr = halfAddr;
                sumRecordNum = halfRecordNum;
                continue;
            }
            if(datecmp(&tmpData[5], &pData[8],5) < 0) {//invalid data //halfRecord Date > startDate 闁圭鍋撻梻鍥ｅ亾闁轰胶澧楀畵浣烘導瀹勯偊娼楅柛锔哄妼婢х娀宕℃繝鍥у姤
                endAddr = halfAddr;
                sumRecordNum = halfRecordNum;
                continue;
            }

            if(0 ==  datecmp(&tmpData[5], &pData[8],5)) {//valid data //halfRecord Date == startDate 闁圭鍋撻梻鍥ｅ亾闁轰胶澧楀畵渚�骞嶉幆褍鐓�
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
                   return -1;//usb闁哄偆鍘肩槐鎴︽晬鐏炴垝绮绘慨婵勫灪婢х晫鎮扮仦鐑╁亾?
               }

               nextAddr = (uint32_t)((beginAddr + inextRecord*FLASH_SENSOR_DATA_SIZE) % FLASH_SENSOR_DATA_AREA_SIZE);
               Flash_get_record(nextAddr, pData, FLASH_SENSOR_HEAD_DATA_SIZE);
               //Flash_get_record(nextAddr, pData, 6);    // ID

               if((tmpData[0] == pData[2]) &&
                   (tmpData[1] == pData[3]) &&
                   (tmpData[2] == pData[4]) &&
                   (tmpData[3] == pData[5])){//ID 缂佹绠戦幃搴ㄦ儍閸曨噮鍟庡锟�?

                   //Flash_get_record(nextAddr, pData+6, FLASH_SENSOR_HEAD_DATA_SIZE-6);  // TIME

                   // CMP Y M D H M // 妤狅拷?闁猴拷?闁猴拷?闁猴拷?闁革拷?// 濞戞挸绉撮顔剧矓閹烘洜绠婚悶娑樿嫰椤曨喖袙?
                   //invalid data //halfRecord Date < startDate 闁圭鍋撻梻鍥ｅ亾闁轰胶澧楀畵浣烘導瀹勯偊娼楅柛锔哄妼閹宕℃繝鍥у姤
                   if((beginAddr == nextAddr) ||( datecmp(&tmpData[5], &pData[8],5) > 0) ){ // 闁告挸绉村畷鎰枔閸偆姊鹃柡锟�?
                       beginAddr = halfAddr;
                       sumRecordNum = halfRecordNum;
                       goto BEGIN_STEP;
                   }

                   if(datecmp(&tmpData[5], &pData[8],5) < 0) {//invalid data //halfRecord Date > startDate 闁圭鍋撻梻鍥ｅ亾闁轰胶澧楀畵浣烘導瀹勯偊娼楅柛锔哄妼婢х娀宕℃繝鍥у姤
                       endAddr = nextAddr;
                       sumRecordNum = inextRecord;
                       goto BEGIN_STEP;
                   }

                   if(0 ==  datecmp(&tmpData[5], &pData[8],5)) {//valid data //halfRecord Date == startDate 闁圭鍋撻梻鍥ｅ亾闁轰胶澧楀畵渚�骞嶉幆褍鐓�
                       *startAddr = nextAddr;
                       return 0;
                   }
              }
           }
           // 婵炲备鍓濆﹢渚�骞嶉幆褍鐓�
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

    //length = len(2)cmd(1)data(n)  // 蹇欓垾鈷欐噴尾宓氼啔鈹撅拷鈾�锟解埗褉锟芥緵顭嬨儌銇㈩灐顬狀伩锟�
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

    //length = len(2)cmd(1)data(n)  // 蹇欓垾鈷欐噴尾宓氼啔鈹撅拷鈾�锟解埗褉锟芥緵顭嬨儌銇㈩灐顬狀伩锟�
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
    uint16_t len, size, vol;
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
            if (size > sizeof(g_rSysConfigInfo)) {//娑撹桨绨￠崜宥呮倵閸忕厧顔愰敍灞界秼閺傜増鍨ㄩ弮褏娈戝銉ュ徔閸愭瑥鍙嗛弫鐗堝祦閺冭绱濋崣顏勫絿瑜版挸澧犻幓鎰返閻ㄥ嫭鍨ㄧ粙瀣碍闂囷拷鐟曚胶娈戦柈銊ュ瀻閵嗭拷
                size = sizeof(g_rSysConfigInfo);                
            }

            memcpy((char *)&g_rSysConfigInfo, (char *)&pData[3], size);
            g_rSysConfigInfo.size = sizeof(g_rSysConfigInfo);            
            Sys_event_post(SYSTEMAPP_EVT_STORE_SYS_CONFIG);

            pData[0] = 0;
            len = Usb_group_package(AC_Ack, pData, 1);
            InterfaceSendImmediately(pData, len);
#endif // SYS_CONFIG_USE_ID_MODE
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
            InterfaceEnable();
            InterfaceSendImmediately(pData, len);
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
            InterfaceEnable();
            InterfaceSendImmediately(pData, len);
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
            SysCtrlSystemReset();//闁插秴鎯�
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
            
            if(pData[0]<255)//閺堝鏅ラ弫鐗堝祦
    			size = pData[0]+1;
			else
                size = 10;//閺冪姵鏅ラ弫鐗堝祦
            
			len = Usb_group_package(AC_Send_History_Data, pData, size);
            InterfaceSendImmediately(pData, len);
			break;
            
#ifdef FLASH_EXTERNAL
        case EV_Get_Device_Data://len(2B) cmd(1B) deviceid(4B)  startdate(6B):ymdhm 201801011259 enddate(6B)   mode(1B)
#ifdef  BOARD_S6_6

#endif  // BOARD_S6_6
           
            memcpy(tmpData,&pData[3], 16);//deviceid(4B)  startdate(6B) enddate(6B)
            transmode = pData[19];

            writeAddr = Flash_get_sensor_writeaddr();
#if 0
            // 姘撴菠楦ユ皳鎼傞垾濮戭灎顔癸拷鏂單叉枼锟斤拷
            readAddr = (writeAddr + FLASH_SENSOR_DATA_AREA_SIZE - FLASH_SENSOR_DATA_SIZE) % FLASH_SENSOR_DATA_AREA_SIZE;
            // Find first Record date optimize
            /* ALG: CMP YMDHM, NOT CMP DID , FIND FIRST  Date record,鐩查檰椹磋幗閳ユ緵锟� 鑾芥綖椹村繖閳э拷?蹇欒伂鑹欒幗楹撳瀯  */
            while(1) {

                if ((GetUsbState() == USB_UNLINK_STATE))
                    return;//usb蹇欓垾鎾�顒笺儌灏栧亾鑼傚綍鑹楁皳鑱涜墮蹇欓鍨勫繖閳ュゥ顬☆煀鈭ㄦ崡锝傚亾?

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
                    (tmpData[3] == 0xff)) ||//鐠佹儳顦幍锟介張澶嬫殶閹诡噯绱濇稉宥呭灲閺傜挰eviceid
                   ((tmpData[0] == pData[2]) && 
                    (tmpData[1] == pData[3]) && 
                    (tmpData[2] == pData[4]) && 
                    (tmpData[3] == pData[5]))){//ID 缁楋箑鎮庨惃鍕啎婢讹拷
                    
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
            readAddr = Flash_sensor_get_sensor_data_readAddr( writeAddr, pData); // 蹇欒墮閳ユ噴锔癸拷鈶哄枹顭婇煭濮懷�锟解懘锟�
    #ifdef S_G
            // 蹇欒墮闄嬪繖鑹楅垾鈭躲儌顔滎搳ID,蹇欒墬閳モ埗銉傤啘鈭讹腹锟芥枀鐫光斁锟芥枀鐤碉腹锟芥拃顒碱灒鈾�锟界伝锔癸拷鎵冲亾蹇欒墮閳ユ噴锔癸拷鈷欐噴尾宓氼啔鈹㈡崠鐭煀尾浼便儌顕锋祹銉拷梅濂筋嚪瀵傛崡鈹撅拷鈾�锟解埗褉锟芥緵顭嬵灎閮濇５锔瑰亾鎼傚繖鑱墮鑾介簱?蹇欓垾濂ヤ箙銉嬪枴鎳婎灎顑濐優銇㈢硶鍋撶洸璧�?蹇欓垾鏂呯澒鈹撅拷鏂呯柕顬狀儩纬銉傛剝鍠㈩灒鈾�锟界伝锔癸拷鈷欐噴尾锟�?
            if((tmpData[0] == 0xff) &&
                (tmpData[1] == 0xff) &&
                (tmpData[2] == 0xff) &&
                (tmpData[3] == 0xff))
           {
                while(1) {

                    if ((GetUsbState() == USB_UNLINK_STATE))
                        return 0;//usb蹇欓垾鎾�顒笺儌灏栧亾鑼傚綍鑹楁皳鑱涜墮蹇欓鍨勫繖閳ュゥ顬☆煀鈭ㄦ崡锝傚亾?
#ifdef      SUPPORT_WATCHDOG
                    WdtClear();
#endif  

                    Flash_get_record(readAddr, pData, FLASH_SENSOR_HEAD_DATA_SIZE);
                    if((tmpData[4] == 0x20) && (datecmp(&tmpData[5], &pData[8],5) < 0)){//find   date < startdate
                        // 鐚┐閳ユ緷銉拷鍙蜂箙顬狀儩顑炪仮绯曞亾鐩茶祩闄嬫皳闄囨悅鐩叉綖娌ゅ繖楦ユゼ鐚倝鍨勫繖閳ユ枀鐫光斁锟芥枀鐤殿灒鈾�锟斤拷 姘撻垾濂ュ禍銇㈢硶鍋撶洸璧傞檵蹇欒墮閳ユ噴锔癸拷鈶哄枹銉傤厔婧嶃儮锟芥顭嬨仮閾板禍顬狅拷?
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
                writeAddr = (uint32_t)(64000); // 1000 蹇欒伜闅嗙尗搴愰殕姘撻檰?
                #endif
                /* 蹇欓埀顑濇緷味鎹栬秮顕峰瘋鈭躲仮閾版５褉锟斤拷?鑾芥綖椹村繖閳ь儩顬∥叉兓鎾侇灎锟�?+ 鐩叉綖鑹楁皳钘涢垾鐘單叉枼锟解挌蔚鍢庛儍锔癸拷锟�?+ 蹇欑瘬闅嗚幗椴佽墿姘撹棝閳ョ妴尾顔欏緱顬犻儩妫碉腹鍋撴悅蹇欓弗妤煎繖閳ワ拷?姘撴幊闄嗘皳椹磋姦姘撳簮鎷х洸闄嗚仹姘撹棝?姘撳簮鍟寘閳┾懇锟轿斥敗鎾亾鐚▌鑱涚尗鐐夌鑼呰墮閳ь優顬ｂ檧锟界伝锔癸拷鈷欐噴尾锟�?姘撹墮鎺虫皳鑱洪埀锟� */
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
                    // 闁哄牆顦伴弲顧奍D
                    if((0xff != pData[2]) &&
                        (0xff != pData[3]) &&
                        (0xff != pData[4]) &&
                        (0xff != pData[5]))
                    if(((tmpData[0] == 0xff) && 
                        (tmpData[1] == 0xff) && 
                        (tmpData[2] == 0xff) && 
                        (tmpData[3] == 0xff)) ||//鐠佹儳顦幍锟介張澶嬫殶閹诡噯绱濇稉宥呭灲閺傜挰eviceid
                       ((tmpData[0] == pData[2]) && 
                        (tmpData[1] == pData[3]) && 
                        (tmpData[2] == pData[4]) && 
                        (tmpData[3] == pData[5]))){//ID 缁楋箑鎮庨惃鍕啎婢讹拷
                        
                        if((tmpData[10] == 0x20) && (datecmp(&tmpData[11], &pData[8],5)>= 0)){//data <= enddate
                            
                            if(transmode){//娑撳秹娓剁憰浣瑰ⅵ閸栧拑绱濇导鐘插斧婵鏆熼幑锟�
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
    
                
            break;
            
        case EV_Set_DevicePara://len(2B) cmd(1B)  para  chk
            
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
                
                if(transmode){//娑撳秹娓剁憰浣瑰ⅵ閸栧拑绱濇导鐘插斧婵鏆熼幑锟�
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
                
                if(transmode){//娑撳秹娓剁憰浣瑰ⅵ閸栧拑绱濇导鐘插斧婵鏆熼幑锟�
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
                
                if(transmode){//娑撳秹娓剁憰浣瑰ⅵ閸栧拑绱濇导鐘插斧婵鏆熼幑锟�
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


        case EV_Get_Cur_Vol:

            vol = Battery_get_voltage();
            pData[0] = (uint8_t)(vol>>8);
            pData[1] = (uint8_t)(vol);
            len = Usb_group_package(AC_Send_Voltage, pData, 2);
            InterfaceSendImmediately(pData, len);
            break;

        case EV_Get_SensorData:

            break;            
        case EV_Set_DevicesId:
              memcpy((char *)&g_rSysConfigInfo.DeviceId, (char *)&pData[3], 4);
              g_rSysConfigInfo.DeviceId[0] = pData[6];
              g_rSysConfigInfo.DeviceId[1] = pData[5];
              g_rSysConfigInfo.DeviceId[2] = pData[4];
              g_rSysConfigInfo.DeviceId[3] = pData[3];
             len = Usb_group_package(AC_Ack, pData, 1);
             break;
#ifdef S_G
        case EV_Get_Term_Log:
            HIBYTE_ZKS(HIWORD_ZKS(readAddr)) = pData[6];
            LOBYTE_ZKS(HIWORD_ZKS(readAddr)) = pData[5];
            HIBYTE_ZKS(LOWORD_ZKS(readAddr)) = pData[4];
            LOBYTE_ZKS(LOWORD_ZKS(readAddr)) = pData[3];
            readAddr = HexToInt(readAddr);
            logReceiveTimeOut = 0;
            RadioCmdSetWithNoRespon(RADIO_PRO_CMD_REQUES_TERM_LOG, readAddr, NULL);
            break;
#endif //S_G
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

void UsbSend_NodeConfig(USB_TX_MSG_ID msgId, uint8_t* buff, uint8_t bufLen)
{
    uint16_t len;
    switch(msgId)
    {
        case AC_Send_Voltage:
        memcpy((char *)bUsbBuff, buff, bufLen);
        len = Usb_group_package(AC_Send_Voltage, bUsbBuff, bufLen);
        InterfaceSendImmediately(bUsbBuff, len);
        break;

        case AC_Send_SensorData:
        memcpy((char *)bUsbBuff, buff, bufLen);
        len = Usb_group_package(AC_Send_SensorData, bUsbBuff, bufLen);
        InterfaceSendImmediately(bUsbBuff, len);
        break;

        case EV_Send_Term_Log:
        memcpy((char *)bUsbBuff, buff, bufLen);
        len = Usb_group_package(EV_Send_Term_Log, bUsbBuff, bufLen);
        InterfaceSendImmediately(bUsbBuff, len);
        break;        
    }
}

#endif //SUPPORT_USB
