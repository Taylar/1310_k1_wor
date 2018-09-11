/*
* @Author: zxt
* @Date:   2017-12-21 17:36:18
* @Last Modified by:   zxt
* @Last Modified time: 2018-09-07 17:01:22
*/
#include "../general.h"

#ifdef SUPPORT_LED
#include "led_drv.h"

// board node

#define LED_R_PIN_NODE                       IOID_25
#define LED_G_PIN_NODE                       IOID_23
#define LED_B_PIN_NODE                       IOID_24


static const uint8_t LED_ID_CONST[LED_MAX] =
{
    LED_R_PIN_NODE,
    LED_B_PIN_NODE,
    LED_G_PIN_NODE,
};



const PIN_Config ledPinTable[] = {
    LED_R_PIN_NODE | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,       /* LED initially off          */
    LED_G_PIN_NODE | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,       /* LED initially off          */
    LED_B_PIN_NODE | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,       /* LED initially off          */
    PIN_TERMINATE
};



static Semaphore_Struct ledSemStruct;
static Semaphore_Handle ledSemHandle;

static PIN_State   ledState;
static PIN_Handle  ledHandle;

#define     LED_PERIOD_CLOCK_TIME_MS       10
singleport_drive_t singlePort[LED_MAX];

Clock_Struct ledProcessClk;     /* not static so you can see in ROV */
Clock_Handle ledProcessClkHandle;

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
    Led_ctrl2(ledId, state, period, period, times);
}

//***********************************************************************************
//
// Led control.
//      id:     allow multi led
//      state:  led first state, 0 or 1
//      period: led blink time,  0 means just set led state and no blink
//      period2: led off time,  0 means just set led state and no blink
//      times:  led blink times, 0 means just set led state and no blink
//
//***********************************************************************************
void Led_ctrl2(uint8_t ledId, uint8_t state, uint32_t period, uint32_t period2, uint8_t times)
{
    uint8_t i;
    /* Get access to resource */

    period  /= CLOCK_UNIT_MS; 
    period2 /= CLOCK_UNIT_MS;

    if(Clock_isActive(ledProcessClkHandle))
        Clock_stop(ledProcessClkHandle);

    Semaphore_pend(ledSemHandle, BIOS_WAIT_FOREVER);

    PIN_setOutputValue(ledHandle, LED_ID_CONST[ledId], state);

    if (period == 0 || times == 0) {
        /* Unlock resource */

        for(i = 0; i < LED_MAX; i++)
        {
            if(singlePort[i].enable)
                break;
        }
        if(i < LED_MAX)
        {
            Clock_start(ledProcessClkHandle);
        }

        Semaphore_post(ledSemHandle);
        return;
    }

    singlePort[ledId].enable      = true;
    singlePort[ledId].times       = times;
    singlePort[ledId].state       = state;
    singlePort[ledId].periodT1Set = (period >= LED_PERIOD_CLOCK_TIME_MS)?period/LED_PERIOD_CLOCK_TIME_MS:1;
    singlePort[ledId].periodT2Set = (period2 >= LED_PERIOD_CLOCK_TIME_MS)?period2/LED_PERIOD_CLOCK_TIME_MS:1;
    singlePort[ledId].periodT1    = singlePort[ledId].periodT1Set;
    singlePort[ledId].periodT2    = singlePort[ledId].periodT2Set;

    /* Unlock resource */
    Semaphore_post(ledSemHandle);

    Clock_start(ledProcessClkHandle);
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
void Led_clk_cb(UArg arg0)
{
    uint8_t i;
    for(i = 0; i < LED_MAX; i++)
    {
        if(singlePort[i].enable)
        {
            if(singlePort[i].times)
            {
                if(singlePort[i].periodT1)
                {
                    singlePort[i].periodT1--;
                    if(singlePort[i].periodT1 == 0)
                        PIN_setOutputValue(ledHandle, LED_ID_CONST[i], !singlePort[i].state);
                }
                else
                {
                    if(singlePort[i].periodT2)
                    {
                        singlePort[i].periodT2--;
                    }
                    else
                    {
                        singlePort[i].times--;
                        if(singlePort[i].times)
                            PIN_setOutputValue(ledHandle, LED_ID_CONST[i], singlePort[i].state);
                        singlePort[i].periodT1    = singlePort[i].periodT1Set;
                        singlePort[i].periodT2    = singlePort[i].periodT2Set;
                    }
                }
            }
            else
            {
                PIN_setOutputValue(ledHandle, LED_ID_CONST[i], !singlePort[i].state);
                singlePort[i].enable = 0;
            }
        }
    }
    for(i = 0; i < LED_MAX; i++)
    {
        if(singlePort[i].enable == true)
            break;
    }
    if(i >= LED_MAX)
    {
        Clock_stop(ledProcessClkHandle);
    }
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

    Clock_Params clkParams;
    Clock_Params_init(&clkParams);
    clkParams.period    = 1;
    clkParams.startFlag = FALSE;
    Clock_construct(&ledProcessClk, Led_clk_cb, 1, &clkParams);
    ledProcessClkHandle = Clock_handle(&ledProcessClk);
    Clock_setTimeout(ledProcessClkHandle, LED_PERIOD_CLOCK_TIME_MS * CLOCK_UNIT_MS);
    Clock_setPeriod(ledProcessClkHandle, LED_PERIOD_CLOCK_TIME_MS * CLOCK_UNIT_MS);
}

#else

void Led_ctrl(uint8_t ledId, uint8_t state, uint32_t period, uint8_t times)
{
}

extern void Led_ctrl2(uint8_t ledId, uint8_t state, uint32_t period, uint32_t period2, uint8_t times);
{
}

void Led_toggle(uint8_t ledId)
{
}

void LedInit(void)
{
}

#endif  /* SUPPORT_LED */

