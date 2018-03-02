/*
* @Author: zxt
* @Date:   2018-01-10 20:26:17
* @Last Modified by:   zxt
* @Last Modified time: 2018-03-02 10:24:53
*/
#include "../general.h"
#include "../app/usb_prot.h"
#include "../radio_app/radio_app.h"
#include "../interface_app/interface.h"


uint8_t usbBuff[UART_BUFF_SIZE];


//***********************************************************************************
//
// Usb protocol group package.
//
//***********************************************************************************
static uint16_t Usb_group_package(USB_TX_MSG_ID msgId, uint8_t *pPacket, uint8_t dataLen)
{
    uint8_t i, buff[USB_BUFF_LENGTH];
    uint16_t length;

    length = 2;
    //娑堟伅鎸囦护
    buff[length++] = msgId;

    for (i = 0; i < dataLen; i++) {
        buff[length++] = pPacket[i];
    }

    //娑堟伅闀垮害
    buff[0] = (length - 2) & 0xff;
    buff[1] = ((length - 2) >> 8) & 0xff;

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


void UsbSend(USB_TX_MSG_ID msgId)
{
    uint16_t len;
    Calendar calendar;

    switch(msgId)
    {
        case AC_Ack:
        usbBuff[0] = 0;
        len = Usb_group_package(AC_Ack, usbBuff, 1);
        InterfaceSend(usbBuff, len);
        break;

        case AC_Send_Config:
        memcpy((char *)usbBuff, (char *)&g_rSysConfigInfo, sizeof(g_rSysConfigInfo));
        len = Usb_group_package(AC_Send_Config, usbBuff, sizeof(g_rSysConfigInfo));
        InterfaceSend(usbBuff, len);
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

        memcpy((char *)usbBuff, (char *)&calendar, sizeof(Calendar));
        len = Usb_group_package(AC_Send_Calendar, usbBuff, sizeof(Calendar));
        InterfaceSend(usbBuff, len);
        break;

        case AC_Send_Bluetooth_Name:

        break;

        case EV_Send_DataRecord:

        break;

    }
}


//***********************************************************************************
//
// USB received data parse. For new protocl.
//
//***********************************************************************************
void Usb_data_parse(uint8_t *pData, uint16_t length)
{
    // ErrorStatus ret;
    uint16_t len, size;
    Calendar calendar;

    //Check package head and tail
    if (pData[0] != PROTOCOL_TOKEN || pData[length - 1] != PROTOCOL_TOKEN)
        return; //Not valid package.

    //Recover transferred meaning
    len = Protocol_recover_escape(&pData[0], &pData[1], length - 2);
    if (len <= 1)
        return;

    //package check code
    // if (pData[len - 1] != CheckCode8(pData, len - 1))
    //     return;

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

        case EV_Set_config:
            size = (pData[0] << 8) + pData[1] - 1;
            if (size > sizeof(g_rSysConfigInfo)) {//涓轰簡鍓嶅悗鍏煎锛屽綋鏂版垨鏃х殑宸ュ叿鍐欏叆鏁版嵁鏃讹紝鍙彇褰撳墠鎻愪緵鐨勬垨绋嬪簭闇�瑕佺殑閮ㄥ垎銆�
                size = sizeof(g_rSysConfigInfo);                
            }
            memcpy((char*)&g_rSysConfigInfo, (char *)&pData[3], size);
            g_rSysConfigInfo.size = sizeof(g_rSysConfigInfo);
            
#ifdef FLASH_INTERNAL
            InternalFlashStoreConfig();
#endif
            pData[0] = 0;
            len = Usb_group_package(AC_Ack, pData, 1);
            InterfaceSend(pData, len);

            if(deviceMode == DEVICES_CONFIG_MODE)
            {
                ConcenterRadioSendParaSet(GetRadioSrcAddr(), GetRadioDstAddr());
#ifdef  BOARD_CONFIG_DECEIVE
                memset((char *)&g_rSysConfigInfo, 0, sizeof(g_rSysConfigInfo));
#endif
                g_rSysConfigInfo.size = sizeof(g_rSysConfigInfo);
            }
            break;

        case EV_Get_APN:
            //__no_operation();
            break;

        case EV_Set_APN:
            //__no_operation();
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
// #ifdef SUPPORT_BLUETOOTH_PRINT
//             Btp_poweron();
//             ret = Btp_get_device_name(pData);
//             Btp_poweroff();
//             if (ret == ES_ERROR) {
//                 pData[0] = 1;
//                 len = Usb_group_package(AC_Ack, pData, 1);
//             } else {
//                 len = Usb_group_package(AC_Send_Bluetooth_Name, pData, strlen((char *)pData));
//             }
//             InterfaceSend(pData, len);
// #endif
            break;

        case EV_Set_Bluetooth_Name:
// #ifdef SUPPORT_BLUETOOTH_PRINT
//             Btp_poweron();
//             pData[len - 1] = '\0';
//             if (Btp_set_device_name(&pData[3]) == ES_ERROR)
//                 pData[0] = 1;
//             else
//                 pData[0] = 0;
//             Btp_poweroff();
//             len = Usb_group_package(AC_Ack, pData, 1);
//             InterfaceSend(pData, len);
// #endif
            break;

		case EV_Usb_Upgrade:
// #ifdef SUPPORT_USB_UPGRADE
//             pData[0] = 2;
//             len = Usb_group_package(AC_Ack, pData, 1);
//             InterfaceSend(pData, len);

//             bslFxn();
// #endif
			break;

#ifdef FLASH_EXTERNAL
		case EV_Get_DataRecord:
			size = (pData[3] << 8) + pData[4];
			Flash_load_sensor_data_history(pData, USB_BUFF_LENGTH, size);
			size = pData[0]+1;
			if(size > 100)size = 100;
			len = Usb_group_package(EV_Send_DataRecord, pData, size);
			InterfaceSend(pData, len);
			break;
#endif

        default:
            pData[0] = 2;
            len = Usb_group_package(AC_Ack, pData, 1);
            InterfaceSend(pData, len);
            break;
    }
}
