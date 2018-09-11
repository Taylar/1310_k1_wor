/*
* @Author: zxt
* @Date:   2017-12-21 17:36:18
* @Last Modified by:   zxt
* @Last Modified time: 2018-09-05 17:28:26
*/

#include "../general.h"

static Clock_Struct keyClkStruct;
static Clock_Handle keyClkHandle;

static KeyTask_t rKeyTask;


// node board
#ifdef BOARD_S3_2
#define Board_BUTTON0                            IOID_4

const PIN_Config keyPinTable[] = {
    Board_BUTTON0 | PIN_INPUT_EN | PIN_PULLUP | PIN_IRQ_NEGEDGE,       /* key isr enable          */
    PIN_TERMINATE
};

#endif

// gateway board
#ifdef BOARD_B2_2
#define Board_BUTTON0                    IOID_1

const PIN_Config keyPinTable[] = {
    Board_BUTTON0 | PIN_INPUT_EN | PIN_PULLUP | PIN_IRQ_NEGEDGE,       /* key isr enable          */
    PIN_TERMINATE
};

#endif

// S6_6 board
#ifdef BOARD_S6_6
#define Board_BUTTON1                    IOID_4
#define Board_BUTTON0                    IOID_14

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

static const uint8_t key_pin_id[KEY_MAX_NUM] = 
{
    Board_BUTTON0,

#ifdef BOARD_S6_6
    Board_BUTTON1,
#endif
};


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
    rKeyTask.doublePressTime = 0;
    rKeyTask.holdPress       = 0;
    rKeyTask.doublePress     = 0;

    Clock_stop(keyClkHandle);
}

//***********************************************************************************
//
// Key scan callback function, use 10ms clock.
//
//***********************************************************************************
static void KeyScanFxn(UArg arg0)
{
    if(KeyReadState((KEY_NUM_E)rKeyTask.keyNum) == KEY_PRESSED)
    {
        if(rKeyTask.doublePress)
        {
            KeyScanStop();
            if(AppKeyIsrCb[KEY_0_DOUBLE_PRESS + rKeyTask.keyNum * KEY_ACTION_TYPE_MAX])
                AppKeyIsrCb[KEY_0_DOUBLE_PRESS + rKeyTask.keyNum * KEY_ACTION_TYPE_MAX]();
        }

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
                if(AppKeyIsrCb[KEY_0_LONG_PRESS + rKeyTask.keyNum * KEY_ACTION_TYPE_MAX])
                    AppKeyIsrCb[KEY_0_LONG_PRESS + rKeyTask.keyNum * KEY_ACTION_TYPE_MAX]();
            }
        }
    }
    else
    {
        if(rKeyTask.doublePress)
        {
#ifdef  SUPPORT_DOUBLE_PRESS
            rKeyTask.doublePressTime++;

            if(rKeyTask.doublePressTime > TIME_KEY_DOUBLE)
            {
                KeyScanStop();    
                if(AppKeyIsrCb[KEY_0_SHORT_PRESS + rKeyTask.keyNum * KEY_ACTION_TYPE_MAX])
                    AppKeyIsrCb[KEY_0_SHORT_PRESS + rKeyTask.keyNum * KEY_ACTION_TYPE_MAX]();
            }
#else
            KeyScanStop();    
            if(AppKeyIsrCb[KEY_0_SHORT_PRESS + rKeyTask.keyNum * KEY_ACTION_TYPE_MAX])
                AppKeyIsrCb[KEY_0_SHORT_PRESS + rKeyTask.keyNum * KEY_ACTION_TYPE_MAX]();
#endif  // SUPPORT_DOUBLE_PRESS
        }
        else
        {
            rKeyTask.doublePress     = 1;
            rKeyTask.doublePressTime = 0;
            rKeyTask.holdTime        = 0;
        }
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
    {
        rKeyTask.keyNum = KEY0_NUM;
        Clock_start(keyClkHandle);
    }
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
    {
        rKeyTask.keyNum = KEY1_NUM;
        Clock_start(keyClkHandle);
    }
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

uint8_t KeyReadState(KEY_NUM_E key)
{
    if(key < KEY_MAX_NUM)
        return(PIN_getInputValue(key_pin_id[key]));

    return KEY_RELEASE;
}
