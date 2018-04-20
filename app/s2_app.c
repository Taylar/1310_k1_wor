/*
* @Author: zxt
* @Date:   2018-03-09 11:14:22
* @Last Modified by:   zxt
* @Last Modified time: 2018-04-20 14:27:33
*/
#include "../general.h"



/***** Defines *****/


/***** Type declarations *****/


/***** Variable declarations *****/



/***** Prototypes *****/



/***** Function definitions *****/


//***********************************************************************************
// brief:   
// 
// parameter: 
//***********************************************************************************
void S2HwInit(void)
{
    LedInit();
    
	KeyInit();
    KeyRegister(SystemKeyEventPostIsr, KEY_0_SHORT_PRESS);
    KeyRegister(SystemLongKeyEventPostIsr, KEY_0_LONG_PRESS);

    Spi_init();

    Flash_init();

    AdcDriverInit();
    Battery_init();
    Battery_voltage_measure();
}

//***********************************************************************************
// brief:the Concenter short key application
// 
// parameter: 
//***********************************************************************************
void S2ShortKeyApp(void)
{
    switch(deviceMode)
    {
        case DEVICES_ON_MODE:
        Led_ctrl(LED_B, 1, 500 * CLOCK_UNIT_MS, 1);
        break;

        case DEVICES_OFF_MODE:
        Led_ctrl(LED_R, 1, 500 * CLOCK_UNIT_MS, 1);
        break;
    }
}

//***********************************************************************************
// brief:the Concenter long key application
// 
// parameter: 
//***********************************************************************************
void S2LongKeyApp(void)
{
    switch(deviceMode)
    {
        case DEVICES_ON_MODE:
        ConcenterSleep();
        Led_ctrl(LED_R, 1, 250 * CLOCK_UNIT_MS, 6);
        break;

        case DEVICES_OFF_MODE:
        Led_ctrl(LED_B, 1, 250 * CLOCK_UNIT_MS, 6);
        ConcenterWakeup();
        break;
    }
}
