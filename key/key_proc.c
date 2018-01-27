/*
* @Author: zxt
* @Date:   2017-12-21 17:36:18
* @Last Modified by:   zxt
* @Last Modified time: 2018-01-27 14:17:43
*/

#include "../general.h"

static Clock_Struct keyClkStruct;
static Clock_Handle keyClkHandle;

static KeyTask_t rKeyTask;

enum{
    KEY0,
    KEY1,
};

// node board
#ifdef BOARD_S1_2
#define Board_BUTTON0                            IOID_4

const PIN_Config keyPinTable[] = {
    Board_BUTTON0 | PIN_INPUT_EN | PIN_PULLUP | PIN_IRQ_NEGEDGE,       /* key isr enable          */
    PIN_TERMINATE
};

#endif

// gateway board
#ifdef BOARD_S2_2
#define Board_BUTTON0                    IOID_1

const PIN_Config keyPinTable[] = {
    Board_BUTTON0 | PIN_INPUT_EN | PIN_PULLUP | PIN_IRQ_NEGEDGE,       /* key isr enable          */
    PIN_TERMINATE
};

#endif

// S6_6 board
#ifdef BOARD_S6_6
#define Board_BUTTON1                    IOID_1
#define Board_BUTTON0                    IOID_18

const PIN_Config keyPinTable[] = {
    Board_BUTTON0 | PIN_INPUT_EN | PIN_PULLUP | PIN_IRQ_NEGEDGE,       /* key isr enable          */
    PIN_TERMINATE
};

const PIN_Config key1PinTable[] = {
    Board_BUTTON1 | PIN_INPUT_EN | PIN_PULLUP | PIN_IRQ_NEGEDGE,       /* key isr enable          */
    PIN_TERMINATE
};

static PIN_State   key1State;
static PIN_Handle  key1Handle;
#endif



static PIN_State   keyState;
static PIN_Handle  keyHandle;




typedef void (*AppKeyIsrCb_t)(void);

static AppKeyIsrCb_t   AppKeyIsrCb[KEY_ACTION_MAX];


//***********************************************************************************
//
// key Io Init.
//      
//
//***********************************************************************************
void KeyIoInit(PIN_IntCb pCb)
{
    keyHandle = PIN_open(&keyState, keyPinTable);
    PIN_registerIntCb(keyHandle, pCb);
}

//***********************************************************************************
//
// key Io Init.
//      
//
//***********************************************************************************
#ifdef BOARD_S6_6
void Key1IoInit(PIN_IntCb pCb)
{
    key1Handle = PIN_open(&key1State, key1PinTable);
    PIN_registerIntCb(key1Handle, pCb);
}
#endif

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
    switch(rKeyTask.keyNum)
    {
        case KEY0:
        if(PIN_getInputValue(Board_BUTTON0) == KEY_PRESSED)
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
            if(rKeyTask.holdTime > TIME_KEY_NEW)
            {
            rKeyTask.holdTime = 0;
                if(AppKeyIsrCb[KEY_0_SHORT_PRESS])
                    AppKeyIsrCb[KEY_0_SHORT_PRESS]();
            }
            rKeyTask.holdTime = 0;
            KeyScanStop();
            
        }
        break;

        case KEY1:
#ifdef BOARD_S6_6
        if(PIN_getInputValue(Board_BUTTON1) == KEY_PRESSED)
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
                    if(AppKeyIsrCb[KEY_1_LONG_PRESS])
                        AppKeyIsrCb[KEY_1_LONG_PRESS]();
                }
            }
        }
        else
        {
            if(rKeyTask.holdTime > TIME_KEY_NEW)
            {
                rKeyTask.holdTime = 0;
                if(AppKeyIsrCb[KEY_1_SHORT_PRESS])
                    AppKeyIsrCb[KEY_1_SHORT_PRESS]();
            }
            rKeyTask.holdTime = 0;
            KeyScanStop();
        }
#endif
        break;

        
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
// Key gpio hwi callback function.
//
//***********************************************************************************
#ifdef  BOARD_S6_6
static void Key1IsrFxn(UInt index)
{

    if (Clock_isActive(keyClkHandle) == FALSE)
        Clock_start(keyClkHandle);
}
#endif
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
#ifdef  BOARD_S6_6
    Key1IoInit((PIN_IntCb)Key1IsrFxn);
#endif

}

//***********************************************************************************
//
// Key register the isr cb.
//
//***********************************************************************************
void KeyRegister(void (*Cb)(void), KEY_ACTION action)
{
    if(action < KEY_ACTION_MAX)
        AppKeyIsrCb[action] = Cb;
}

//***********************************************************************************
//
// Key read state.
//
//***********************************************************************************

uint8_t KeyReadState(KEY_ACTION action)
{
    switch(action)
    {
        case KEY_0_SHORT_PRESS:
        case KEY_0_LONG_PRESS:

        return PIN_getInputValue(Board_BUTTON0);

#ifdef BOARD_S6_6
        case KEY_1_SHORT_PRESS:
        case KEY_1_LONG_PRESS:

        return PIN_getInputValue(Board_BUTTON1);
#endif

    }
    return KEY_RELEASE;
}
