/*
* @Author: zxt
* @Date:   2018-01-08 16:46:40
* @Last Modified by:   zxt
* @Last Modified time: 2018-06-05 15:00:38
*/

#include "../general.h"

#if (defined BOARD_S2_2) || (defined BOARD_S6_6)
/***** Defines *****/
#define RADIO_CSD_PIN                       IOID_7      // 0:sleep    1:wake up
#define RADIO_CTX_PIN                       IOID_6      // 0:Rx       1:Tx



/***** Variable declarations *****/

const PIN_Config radioPinTable_gateway[] = {
    RADIO_CSD_PIN | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,       /* LED initially off          */
    RADIO_CTX_PIN | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX,       /* LED initially off          */
    PIN_TERMINATE
};



static PIN_State   radioPinState;
static PIN_Handle  radioPinHandle;


/***** Function definitions *****/


//***********************************************************************************
// brief:   Init the radio front hardware
// 
// parameter: 
//***********************************************************************************
void RadioFrontInit(void)
{
    radioPinHandle = PIN_open(&radioPinState, radioPinTable_gateway);

    // Map RFC_GPO0 to IO 23
    PINCC26XX_setMux(radioPinHandle, RADIO_CTX_PIN, PINCC26XX_MUX_RFC_GPO0);
    // Map IO 26 to RFC_GPI1
    PINCC26XX_setMux(radioPinHandle, RADIO_CSD_PIN, PINCC26XX_MUX_RFC_GPI1);
}


//***********************************************************************************
// brief:   enable the raido front tx
// 
// parameter: 
//***********************************************************************************
void RadioFrontTxEnable(void)
{
    PIN_setOutputValue(radioPinHandle, RADIO_CSD_PIN, 1);
    PIN_setOutputValue(radioPinHandle, RADIO_CTX_PIN, 1);
}


//***********************************************************************************
// brief:   enable the raido front rx
// 
// parameter: 
//***********************************************************************************
void RadioFrontRxEnable(void)
{
    PIN_setOutputValue(radioPinHandle, RADIO_CSD_PIN, 1);
    PIN_setOutputValue(radioPinHandle, RADIO_CTX_PIN, 0);
}


//***********************************************************************************
// brief:   disable the raido front
// 
// parameter: 
//***********************************************************************************
void RadioFrontDisable(void)
{
    PIN_setOutputValue(radioPinHandle, RADIO_CSD_PIN, 0);
    PIN_setOutputValue(radioPinHandle, RADIO_CTX_PIN, 0);
}

#else

void RadioFrontInit(void)
{
    
}

void RadioFrontTxEnable(void)
{
    
}

void RadioFrontRxEnable(void)
{
    
}

void RadioFrontDisable(void)
{
    
}


#endif
