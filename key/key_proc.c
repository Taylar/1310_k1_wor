/*
* @Author: zxt
* @Date:   2017-12-21 17:36:18
* @Last Modified by:   zxt
* @Last Modified time: 2018-01-16 20:30:16
*/

#include "../general.h"

static Clock_Struct keyClkStruct;
static Clock_Handle keyClkHandle;

static KeyTask_t rKeyTask;


#define Board_BUTTON0_NODE                       IOID_4
#define Board_BUTTON0_GATEWAY                    IOID_1




const PIN_Config keyPinTable_node[] = {
    Board_BUTTON0_NODE | PIN_INPUT_EN | PIN_PULLUP | PIN_IRQ_NEGEDGE,       /* key isr enable          */
    PIN_TERMINATE
};

const PIN_Config keyPinTable_gateway[] = {
    Board_BUTTON0_GATEWAY | PIN_INPUT_EN | PIN_PULLUP | PIN_IRQ_NEGEDGE,       /* key isr enable          */
    PIN_TERMINATE
};

static PIN_State   keyState;
static PIN_Handle  keyHandle;



typedef void (*AppKeyIsrCb_t)(void);

static AppKeyIsrCb_t   AppKeyIsrCb[KEY_ACTION_MAX];


static PIN_Id boardButton0Pin;

//***********************************************************************************
//
// key Io Init.
//      
//
//***********************************************************************************
void KeyIoInit(PIN_IntCb pCb)
{
    if(devicesType == DEVICES_TYPE_GATEWAY)
    {
        boardButton0Pin = Board_BUTTON0_GATEWAY;
        keyHandle = PIN_open(&keyState, keyPinTable_gateway);
    }
    else
    {
        boardButton0Pin = Board_BUTTON0_NODE;
        keyHandle = PIN_open(&keyState, keyPinTable_node);
    }
    PIN_registerIntCb(keyHandle, pCb);
}


//***********************************************************************************
//
// Key stop scan.
//
//***********************************************************************************
static void KeyScanStop(void)
{
    rKeyTask.holdTime        = 0;
    rKeyTask.holdPress       = 0;
    Clock_stop(keyClkHandle);
}

//***********************************************************************************
//
// Key scan callback function, use 10ms clock.
//
//***********************************************************************************
static void KeyScanFxn(UArg arg0)
{
    if (PIN_getInputValue(boardButton0Pin) == KEY_PRESSED)
    {
        if(rKeyTask.holdPress == 0)
        {
            rKeyTask.holdPress       = 1;
            rKeyTask.holdTime        = 0;
        }
        else
        {
            rKeyTask.holdTime++;
            if(rKeyTask.holdTime > TIME_KEY0_LONG)
            {
                KeyScanStop();
                if(AppKeyIsrCb[KEY_0_LONG_PRESS])
                    AppKeyIsrCb[KEY_0_LONG_PRESS]();
            }
        }
    }
    else
    {
        rKeyTask.holdTime = 0;
        KeyScanStop();
        if(AppKeyIsrCb[KEY_0_SHORT_PRESS])
            AppKeyIsrCb[KEY_0_SHORT_PRESS]();
    }
}

//***********************************************************************************
//
// Key gpio hwi callback function.
//
//***********************************************************************************
static void KeyIsrFxn(UInt index)
{
    if (Clock_isActive(keyClkHandle) == FALSE)
        Clock_start(keyClkHandle);
}

//***********************************************************************************
//
// Key init.
//
//***********************************************************************************
void KeyInit(void)
{
    uint8_t i;

    rKeyTask.holdTime = 0;
    rKeyTask.doublePressTime = 0;
    rKeyTask.shortPress = 0;
    rKeyTask.doublePress = 0;

    /* Construct a 10ms periodic Clock Instance to scan key */
    Clock_Params clkParams;
    Clock_Params_init(&clkParams);
    clkParams.period = 10 * CLOCK_UNIT_MS;
    clkParams.startFlag = FALSE;
    Clock_construct(&keyClkStruct, (Clock_FuncPtr)KeyScanFxn, 0, &clkParams);
    /* Obtain clock instance handle */
    keyClkHandle = Clock_handle(&keyClkStruct);

    for(i = 0; i < KEY_ACTION_MAX; i++)
    {
        AppKeyIsrCb[i] = NULL;
    }

    /* install Button callback */
    KeyIoInit((PIN_IntCb)KeyIsrFxn);
}

void KeyRegister(void (*Cb)(void), KEY_ACTION action)
{
    if(action < KEY_ACTION_MAX)
        AppKeyIsrCb[action] = Cb;
}

