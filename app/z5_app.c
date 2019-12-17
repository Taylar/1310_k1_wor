/*
* @Author: luowen
* @Date:   2019-04-08
* @Last Modified by:   luowen
* @Last Modified time: 2019-04-08
*/
#include "../general.h"



/***** Defines *****/


/***** Type declarations *****/


/***** Variable declarations *****/

uint8_t WakeUpPeriod;


/***** Prototypes *****/



/***** Function definitions *****/
//============================================
// SleepRadioApp
//===============================================
void Z5SleepRadioApp(void){
    RadioDisable();
}


//====================================
// Z5TxLoseAlarm
//====================================
void Z5TxLoseAlarm(void){
    Calendar Getcalendar;
    static uint16_t AlarmSerialNum;
    uint16_t value = 0;
    value =  Battery_get_voltage();

    Getcalendar = Rtc_get_calendar();



    assetInfoValid = 1;
    NodeStartBroadcast();//¨¦¨¨????¡À¨º¦Ì??¡¤?a?¨ª?¡ì??
    assetInfo[0] = HIBYTE_ZKS(AlarmSerialNum);
    assetInfo[1] = LOBYTE_ZKS(AlarmSerialNum);

    if (AlarmSerialNum>= 0xfffe) {
         AlarmSerialNum = 0;
     }
     AlarmSerialNum++;

    assetInfo[2] = TransHexToBcd(Getcalendar.Year -2000); //TransHexToBcd(lastcalendar.Year - 2000);
    assetInfo[3] = TransHexToBcd(Getcalendar.Month);
    assetInfo[4] = TransHexToBcd(Getcalendar.DayOfMonth);
    assetInfo[5] = TransHexToBcd(Getcalendar.Hours);
    assetInfo[6] = TransHexToBcd(Getcalendar.Minutes);
    assetInfo[7] = TransHexToBcd(Getcalendar.Seconds);
    assetInfo[8] = HIBYTE_ZKS(value);;
    assetInfo[9] = LOBYTE_ZKS(value);
    assetInfo[10] = 0;
    assetInfo[11] = SEN_TYPE_ASSET;
    NodeUploadProcess();
    RadioSend();
}

//==============================================
//Z5GotoRxWakeUpOrder
//================================================
void Z5GotoRxWakeUpOrder(void){
    if(deviceMode == DEVICES_WAKEUP_MODE){
       // EasyLink_setCtrl(EasyLink_Ctrl_AsyncRx_TimeOut, 100 * CLOCK_UNIT_MS);
        RadioSetRxMode();  //10s wakeup goto rx comand
    }
}

//========================================
//Z5_wakeup_time_isr
////========================================
void Z5_wakeup_time_isr(void){
   if(++WakeUpPeriod>=10){
       WakeUpPeriod = 0;
#if 1
      Z5GotoRxWakeUpOrder();
#else
        Z5TxLoseAlarm();
#endif



   }


}
//***********************************************************************************
// brief:   
// 
// parameter: 
//***********************************************************************************
