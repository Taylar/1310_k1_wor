/*
* @Author: zxt
* @Date:   2018-03-09 11:13:28
* @Last Modified by:   zxt
* @Last Modified time: 2018-03-12 10:27:13
*/
#include "../general.h"



/***** Defines *****/


/***** Type declarations *****/


/***** Variable declarations *****/
uint32_t configModeTimeCnt;          // the unit is sec



/***** Prototypes *****/



/***** Function definitions *****/



//***********************************************************************************
// brief:   
// 
// parameter: 
//***********************************************************************************
void S1HwInit(void)
{
    KeyInit();

    KeyRegister(SystemKeyEventPostIsr, KEY_0_SHORT_PRESS);

    KeyRegister(SystemLongKeyEventPostIsr, KEY_0_LONG_PRESS);

    Spi_init();

    I2c_init();

    Flash_init();

    SHT2X_FxnTable.initFxn(SHT2X_I2C_CH0);

    configModeTimeCnt = 0;
}




//***********************************************************************************
// brief:the node short key application
// 
// parameter: 
//***********************************************************************************
void S1ShortKeyApp(void)
{
    switch(deviceMode)
    {
        case DEVICES_ON_MODE:
        case DEVICES_CONFIG_MODE:
        // enter DEVICES_CONFIG_MODE, clear radio tx buf and send the config parameter to config deceive
        NodeStrategyReset();
        deviceMode                      = DEVICES_CONFIG_MODE;
        configModeTimeCnt = 0;
        NodeUploadStop();
        NodeUploadFailProcess();
        NodeStrategyBusySet(false);
        RadioModeSet(RADIOMODE_RECEIVEPORT);
        SetRadioDstAddr(CONFIG_DECEIVE_ID_DEFAULT);

        ClearRadioSendBuf();
        NodeRadioSendConfig();


        Led_ctrl(LED_B, 0, 500 * CLOCK_UNIT_MS, 1);
        break;

        case DEVICES_OFF_MODE:
        Led_ctrl(LED_R, 0, 500 * CLOCK_UNIT_MS, 1);
        break;
    }
}

//***********************************************************************************
// brief:the node long key application
// 
// parameter: 
//***********************************************************************************
void S1LongKeyApp(void)
{
    switch(deviceMode)
    {
        case DEVICES_ON_MODE:
        case DEVICES_CONFIG_MODE:
        NodeSleep();
        Led_ctrl(LED_R, 0, 250 * CLOCK_UNIT_MS, 6);
        SysCtrlSystemReset();
        break;

        case DEVICES_OFF_MODE:
        Led_ctrl(LED_B, 0, 250 * CLOCK_UNIT_MS, 6);
        NodeWakeup();
        break;
    }
}



void S1AppRtcProcess(void)
{
	if(deviceMode == DEVICES_CONFIG_MODE)
    {
        configModeTimeCnt++;
        if(configModeTimeCnt >= 60)
        {
            ClearRadioSendBuf();
            RadioModeSet(RADIOMODE_SENDPORT);
            NodeStartBroadcast();
            NodeStrategyBusySet(true);

            NodeBroadcasting();
            deviceMode = DEVICES_ON_MODE;
        }
    }
}
