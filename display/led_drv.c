/*
* @Author: zxt
* @Date:   2017-12-21 17:36:18
* @Last Modified by:   zxt
* @Last Modified time: 2018-01-22 16:33:17
*/
#include "../general.h"

#ifdef SUPPORT_LED
#include "led_drv.h"

// board node
#ifdef BOARD_S1_2

#define LED_R_PIN_NODE                       IOID_25
#define LED_G_PIN_NODE                       IOID_24
#define LED_B_PIN_NODE                       IOID_23
#define LED_GND_PIN_NODE                     IOID_21


static const uint8_t LED_ID_CONST[LED_MAX] =
{
    LED_R_PIN_NODE,
    LED_G_PIN_NODE,
    LED_B_PIN_NODE,
    LED_GND_PIN_NODE,
};



const PIN_Config ledPinTable[] = {
    LED_R_PIN_NODE | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX,       /* LED initially off          */
    LED_G_PIN_NODE | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX,       /* LED initially off          */
    LED_B_PIN_NODE | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX,       /* LED initially off          */
    LED_GND_PIN_NODE | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX,    /* LED GND initially off          */
    PIN_TERMINATE
};

#endif

// board gateway
#ifdef BOARD_S2_2

#define LED_R_PIN_GATEWAY                       IOID_14
#define LED_G_PIN_GATEWAY                       IOID_13
#define LED_B_PIN_GATEWAY                       IOID_15

static const uint8_t LED_ID_CONST[LED_MAX] =
{
    LED_R_PIN_GATEWAY,
    LED_G_PIN_GATEWAY,
    LED_B_PIN_GATEWAY,
};

const PIN_Config ledPinTable[] = {
    
    LED_R_PIN_GATEWAY | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,       /* LED initially off          */
    LED_G_PIN_GATEWAY | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,       /* LED initially off          */
    LED_B_PIN_GATEWAY | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,       /* LED initially off          */
    PIN_TERMINATE
};

#endif


static Semaphore_Struct ledSemStruct;
static Semaphore_Handle ledSemHandle;

static PIN_State   ledState;
static PIN_Handle  ledHandle;

//***********************************************************************************
//
// LedIoInit.
//      
//
//***********************************************************************************
void LedIoInit(void)
{
    ledHandle = PIN_open(&ledState, ledPinTable);
}


//***********************************************************************************
//
// Led control.
//      id:     allow multi led
//      state:  led first state, 0 or 1
//      period: led blink time,  0 means just set led state and no blink
//      times:  led blink times, 0 means just set led state and no blink
//
//***********************************************************************************
void Led_ctrl(uint8_t ledId, uint8_t state, uint32_t period, uint8_t times)
{
    /* Get access to resource */
    Semaphore_pend(ledSemHandle, BIOS_WAIT_FOREVER);

    PIN_setOutputValue(ledHandle, LED_ID_CONST[ledId], state);

    if (period == 0 || times == 0) {
        /* Unlock resource */
        Semaphore_post(ledSemHandle);
        return;
    }

    Task_sleep(period);
    PIN_setOutputValue(ledHandle, LED_ID_CONST[ledId], !state);
    times--;

    while (times--) {
        Task_sleep(period);
        PIN_setOutputValue(ledHandle, LED_ID_CONST[ledId], state);
        Task_sleep(period);
        PIN_setOutputValue(ledHandle, LED_ID_CONST[ledId], !state);
    }

    /* Unlock resource */
    Semaphore_post(ledSemHandle);
}

//***********************************************************************************
//
// Led toggle.
//      id:     allow multi led
//
//***********************************************************************************
void Led_toggle(uint8_t ledId)
{
    /* Get access to resource */
    Semaphore_pend(ledSemHandle, BIOS_WAIT_FOREVER);

//    GPIO_toggleOutputOnPin(rLedHWAttrs[ledId].port, rLedHWAttrs[ledId].pin);
    PIN_setOutputValue(ledHandle, LED_ID_CONST[ledId], !(PIN_getOutputValue(LED_ID_CONST[ledId])));

    /* Unlock resource */
    Semaphore_post(ledSemHandle);
}

//***********************************************************************************
//
// Led toggle.
//      id:     allow multi led
//
//***********************************************************************************
void Led_set(uint8_t ledId, uint8_t status)
{
    /* Get access to resource */
    Semaphore_pend(ledSemHandle, BIOS_WAIT_FOREVER);

//    GPIO_toggleOutputOnPin(rLedHWAttrs[ledId].port, rLedHWAttrs[ledId].pin);
    PIN_setOutputValue(ledHandle, LED_ID_CONST[ledId], status);

    /* Unlock resource */
    Semaphore_post(ledSemHandle);
}



//***********************************************************************************
//
// Led init.
//
//***********************************************************************************
void LedInit(void)
{
    LedIoInit();

    /* Construct a Semaphore object to be use as a resource lock, inital count 1 */
    Semaphore_Params ledSemParams;
    Semaphore_Params_init(&ledSemParams);
    ledSemParams.mode = Semaphore_Mode_BINARY;
    Semaphore_construct(&ledSemStruct, 1, &ledSemParams);
    /* Obtain instance handle */
    ledSemHandle = Semaphore_handle(&ledSemStruct);
}

#else

void Led_ctrl(uint8_t ledId, uint8_t state, uint32_t period, uint8_t times)
{
}

void Led_toggle(uint8_t ledId)
{
}

void LedInit(void)
{
}

#endif  /* SUPPORT_LED */

