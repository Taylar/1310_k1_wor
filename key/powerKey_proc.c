#include "../general.h"
//***********************************************************************************
// Copyright 2017, Zksiot Development Ltd.
// Created by Linjie, 2017.08.08
// MCU: MSP430F5529
// OS: TI-RTOS
// Project:
// File name: key_proc.c
// Description: key process routine.

#ifdef BOARD_B1S
#define Board_BUTTON0                            IOID_1
#endif //BOARD_B1S
#if 1
static PIN_State   PwrkeyState;
static PIN_Handle  PwrkeyHandle;

#define Board_BUTTON_POWER_EN       IOID_1
#define Board_BUTTON_POWER_INIT IOID_24

const PIN_Config keyPinTable1[] = {
     Board_BUTTON_POWER_INIT | PIN_INPUT_EN | PIN_PULLDOWN | PIN_IRQ_POSEDGE,       /* key isr enable          */
     PIN_TERMINATE
};


static PIN_State   PowerPinState;
static PIN_Handle  PowerPinHandle;

const PIN_Config POWER_ENTABLE[] = {
       Board_BUTTON_POWER_EN  | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX,       /* LED initially off          */
       PIN_TERMINATE
};

static Clock_Struct keyClkStruct;
static Clock_Handle keyClkHandle;

static KeyTask_t rKeyTaskPower;

void gpio_power_en_config(void)
{
    PowerPinHandle = PIN_open(&PowerPinState, POWER_ENTABLE);
    PIN_setOutputValue(PowerPinHandle, Board_BUTTON_POWER_EN, 1);
}


static const uint8_t key_pin_id_X[KEY_MAX_NUM] =
{
 Board_BUTTON_POWER_INIT,
};

static const uint16_t KEY_LONG_TIME[KEY_MAX_NUM] =
{
    TIME_KEY0_LONG,
};


typedef void (*AppKeyIsrCb_t)(void);

static AppKeyIsrCb_t   AppKeyIsrCb[KEY_ACTION_MAX];

//***********************************************************************************
//
// key Io Init.
//
//
//***********************************************************************************
void KeyIoInit1(PIN_IntCb pCb)
{
    PwrkeyHandle = PIN_open(&PwrkeyState, keyPinTable1);
    PIN_registerIntCb(PwrkeyHandle, pCb);
}



//***********************************************************************************
//
// Key stop scan.
//
//***********************************************************************************
static void Key_scan_stop(void)
{
    rKeyTaskPower.holdTime        = 0;
    rKeyTaskPower.doublePressTime = 0;
    rKeyTaskPower.holdPress       = 0;
    rKeyTaskPower.doublePress     = 0;

    Clock_stop(keyClkHandle);
}

void KeyRegister1(void (*Cb)(void), KEY_ACTION action);
//***********************************************************************************
//
// Key scan callback function, use 10ms clock.
//
//***********************************************************************************
static void Key_scanFxn(UArg arg0)
{
    if(!PowerKeyReadState((KEY_NUM_E)rKeyTaskPower.keyNum) == KEY_PRESSED)
    {
#ifdef  SUPPORT_DOUBLE_CLICK
        if(rKeyTaskPower.doublePress)
        {
            Key_scan_stop();
            if(AppKeyIsrCb[KEY_0_DOUBLE_PRESS + rKeyTaskPower.keyNum * KEY_ACTION_TYPE_MAX])
                AppKeyIsrCb[KEY_0_DOUBLE_PRESS + rKeyTaskPower.keyNum * KEY_ACTION_TYPE_MAX]();
        }
#endif  // SUPPORT_DOUBLE_CLICK

        if(rKeyTaskPower.holdPress == 0)
        {
            rKeyTaskPower.holdPress       = 1;
            rKeyTaskPower.holdTime        = 0;
        }
        else
        {
            rKeyTaskPower.holdTime++;
            if(rKeyTaskPower.holdTime >= KEY_LONG_TIME[rKeyTaskPower.keyNum])
            {
                Key_scan_stop();
                if(AppKeyIsrCb[KEY_0_LONG_PRESS + rKeyTaskPower.keyNum * KEY_ACTION_TYPE_MAX])
                    AppKeyIsrCb[KEY_0_LONG_PRESS + rKeyTaskPower.keyNum * KEY_ACTION_TYPE_MAX]();
            }
        }
    }
    else
    {
        if(rKeyTaskPower.doublePress)
        {
#ifdef  SUPPORT_DOUBLE_CLICK
            rKeyTaskPower.doublePressTime++;

            if(rKeyTaskPower.doublePressTime > TIME_KEY_DOUBLE)
            {
                Key_scan_stop();
                if(AppKeyIsrCb[KEY_0_SHORT_PRESS + rKeyTaskPower.keyNum * KEY_ACTION_TYPE_MAX])
                    AppKeyIsrCb[KEY_0_SHORT_PRESS + rKeyTaskPower.keyNum * KEY_ACTION_TYPE_MAX]();
            }
#else
            if((AppKeyIsrCb[KEY_0_SHORT_PRESS + rKeyTaskPower.keyNum * KEY_ACTION_TYPE_MAX]) && (rKeyTaskPower.holdTime > TIME_KEY_NEW))
                AppKeyIsrCb[KEY_0_SHORT_PRESS + rKeyTaskPower.keyNum * KEY_ACTION_TYPE_MAX]();
            Key_scan_stop();
#endif  // SUPPORT_DOUBLE_CLICK
        }
        else
        {
            rKeyTaskPower.doublePress     = 1;
            rKeyTaskPower.doublePressTime = 0;
#ifdef  SUPPORT_DOUBLE_CLICK
            rKeyTaskPower.holdTime        = 0;
#endif  // SUPPORT_DOUBLE_CLICK
        }
    }
}

//***********************************************************************************
//
// Key gpio hwi callback function.
//
//***********************************************************************************
static void KeyPower_isrFxn(UInt index)
{
    if (Clock_isActive(keyClkHandle) == FALSE)
    {
        rKeyTaskPower.keyNum = KEY0_NUM;
        Clock_start(keyClkHandle);
    }
}


void Boutton0ShortPressCb(void);
void Boutton0LongPressCb(void);
void Boutton0DoublePressCb(void);

//***********************************************************************************
//
// Key init.
//
//***********************************************************************************
void power_Key_init(void)
{
    uint8_t i;
    rKeyTaskPower.holdTime        = 0;
    rKeyTaskPower.doublePressTime = 0;
    rKeyTaskPower.shortPress      = 0;
    rKeyTaskPower.doublePress     = 0;
    rKeyTaskPower.code            = _VK_NULL;

    /* Construct a 10ms periodic Clock Instance to scan key */
    Clock_Params clkParams;
    Clock_Params_init(&clkParams);
    clkParams.period = 10 * CLOCK_UNIT_MS;
    clkParams.startFlag = FALSE;
    Clock_construct(&keyClkStruct, (Clock_FuncPtr)Key_scanFxn, 0, &clkParams);
    /* Obtain clock instance handle */
    keyClkHandle = Clock_handle(&keyClkStruct);

    for(i = 0; i < KEY_ACTION_MAX; i++)
    {
        AppKeyIsrCb[i] = NULL;
    }
    /* install Button callback */
    KeyIoInit1((PIN_IntCb)KeyPower_isrFxn);

    KeyRegister1(Boutton0ShortPressCb, KEY_0_SHORT_PRESS);
    //KeyRegister1(Boutton0LongPressCb, KEY_0_LONG_PRESS);
    //KeyRegister1(Boutton0DoublePressCb, KEY_0_DOUBLE_PRESS);

    //gpio_power_en_config();
}

//***********************************************************************************
//
// Key process task function.
//
//***********************************************************************************
uint8_t power_Key_get(void)
{
    return rKeyTaskPower.code;
}

//***********************************************************************************
//
// Key read state.
//
//***********************************************************************************

uint8_t PowerKeyReadState(KEY_NUM_E key)
{
    if(key < KEY_MAX_NUM)
        return(PIN_getInputValue(key_pin_id_X[key]));

    return KEY_RELEASE;
}


//***********************************************************************************
//
// Key register the isr cb.
//
//***********************************************************************************
void KeyRegister1(void (*Cb)(void), KEY_ACTION action)
{
    if(action < KEY_ACTION_MAX)
        AppKeyIsrCb[action] = Cb;
}


//***********************************************************************************
//
// button 0 short press cb;
//
//***********************************************************************************
void Boutton0ShortPressCb(void)
{
     rKeyTaskPower.code = _VK_SELECT;
     Sys_event_post(SYS_EVT_KEY);
#ifdef LORA_OOK_TEST
    g_rSysConfigInfo.rfPA++;
    if(g_rSysConfigInfo.rfPA > 15)
        g_rSysConfigInfo.rfPA = 0;
#endif //LORA_OOK_TEST
}


//***********************************************************************************
//
// button 0 long press cb;
//
//***********************************************************************************
void Boutton0LongPressCb(void)
{
    //rKeyTask.code = _VK_POWER;
    //Sys_event_post(SYS_EVT_KEY);
}

//***********************************************************************************
//
// button 0 long press cb;
//
//***********************************************************************************
void Boutton0DoublePressCb(void)
{
    //rKeyTaskPower.code = _VK_DOUBLE_KEY;
   // Sys_event_post(SYS_EVT_KEY);
}
#endif
