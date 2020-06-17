#include "../general.h"
//***********************************************************************************
// Copyright 2017, Zksiot Development Ltd.
// Created by Linjie, 2017.08.08
// MCU: MSP430F5529
// OS: TI-RTOS
// Project:
// File name: key_proc.c
// Description: key process routine.
//***********************************************************************************
#include "../general.h"




#ifdef BOARD_B1S
#define Board_BUTTON0                            IOID_1
#endif //BOARD_B1S
#if 1
static PIN_State   keyState;
static PIN_Handle  keyHandle;

#define Board_BUTTON_POWER_EN       IOID_1
#define Board_BUTTON_POWER_KEY_INIT IOID_24

const PIN_Config keyPinTable1[] = {
   Board_BUTTON_POWER_KEY_INIT | PIN_INPUT_EN | PIN_PULLUP | PIN_IRQ_NEGEDGE,       /* key isr enable          */

};


static PIN_State   PowerPinState;
static PIN_Handle  PowerPinHandle;

const PIN_Config POWER_ENTABLE[] = {
       Board_BUTTON_POWER_EN  | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX,       /* LED initially off          */
       PIN_TERMINATE
};

static Clock_Struct keyClkStruct;
static Clock_Handle keyClkHandle;

static KeyTask_t rKeyTask;

void gpio_power_en_config(void)
{
    PowerPinHandle = PIN_open(&PowerPinState, POWER_ENTABLE);
    PIN_setOutputValue(PowerPinHandle, Board_BUTTON_POWER_EN, 1);
}


static const uint8_t key_pin_id[KEY_MAX_NUM] =
{
 //Board_BUTTON_POWER_KEY_INIT,
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
    keyHandle = PIN_open(&keyState, keyPinTable1);
    PIN_registerIntCb(keyHandle, pCb);
}



//***********************************************************************************
//
// Key stop scan.
//
//***********************************************************************************
static void Key_scan_stop(void)
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
static void Key_scanFxn(UArg arg0)
{
    if(KeyReadState1((KEY_NUM_E)rKeyTask.keyNum) == KEY_PRESSED)
    {
#ifdef  SUPPORT_DOUBLE_CLICK
        if(rKeyTask.doublePress)
        {
            Key_scan_stop();
            if(AppKeyIsrCb[KEY_0_DOUBLE_PRESS + rKeyTask.keyNum * KEY_ACTION_TYPE_MAX])
                AppKeyIsrCb[KEY_0_DOUBLE_PRESS + rKeyTask.keyNum * KEY_ACTION_TYPE_MAX]();
        }
#endif  // SUPPORT_DOUBLE_CLICK

        if(rKeyTask.holdPress == 0)
        {
            rKeyTask.holdPress       = 1;
            rKeyTask.holdTime        = 0;
        }
        else
        {
            rKeyTask.holdTime++;
            if(rKeyTask.holdTime >= KEY_LONG_TIME[rKeyTask.keyNum])
            {
                Key_scan_stop();
                if(AppKeyIsrCb[KEY_0_LONG_PRESS + rKeyTask.keyNum * KEY_ACTION_TYPE_MAX])
                    AppKeyIsrCb[KEY_0_LONG_PRESS + rKeyTask.keyNum * KEY_ACTION_TYPE_MAX]();
            }
        }
    }
    else
    {
        if(rKeyTask.doublePress)
        {
#ifdef  SUPPORT_DOUBLE_CLICK
            rKeyTask.doublePressTime++;

            if(rKeyTask.doublePressTime > TIME_KEY_DOUBLE)
            {
                Key_scan_stop();
                if(AppKeyIsrCb[KEY_0_SHORT_PRESS + rKeyTask.keyNum * KEY_ACTION_TYPE_MAX])
                    AppKeyIsrCb[KEY_0_SHORT_PRESS + rKeyTask.keyNum * KEY_ACTION_TYPE_MAX]();
            }
#else
            if((AppKeyIsrCb[KEY_0_SHORT_PRESS + rKeyTask.keyNum * KEY_ACTION_TYPE_MAX]) && (rKeyTask.holdTime > TIME_KEY_NEW))
                AppKeyIsrCb[KEY_0_SHORT_PRESS + rKeyTask.keyNum * KEY_ACTION_TYPE_MAX]();
            Key_scan_stop();
#endif  // SUPPORT_DOUBLE_CLICK
        }
        else
        {
            rKeyTask.doublePress     = 1;
            rKeyTask.doublePressTime = 0;
#ifdef  SUPPORT_DOUBLE_CLICK
            rKeyTask.holdTime        = 0;
#endif  // SUPPORT_DOUBLE_CLICK
        }
    }
}

//***********************************************************************************
//
// Key gpio hwi callback function.
//
//***********************************************************************************
static void Key0_isrFxn(UInt index)
{
    if (Clock_isActive(keyClkHandle) == FALSE)
    {
        rKeyTask.keyNum = KEY0_NUM;
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
    rKeyTask.holdTime        = 0;
    rKeyTask.doublePressTime = 0;
    rKeyTask.shortPress      = 0;
    rKeyTask.doublePress     = 0;
    //rKeyTask.code            = _VK_NULL;

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
    KeyIoInit1((PIN_IntCb)Key0_isrFxn);

    KeyRegister1(Boutton0ShortPressCb, KEY_0_SHORT_PRESS);
    KeyRegister1(Boutton0LongPressCb, KEY_0_LONG_PRESS);
    KeyRegister1(Boutton0DoublePressCb, KEY_0_DOUBLE_PRESS);

    gpio_power_en_config();
}

//***********************************************************************************
//
// Key process task function.
//
//***********************************************************************************
uint8_t power_Key_get(void)
{
    return 1;//rKeyTask.code;
}

//***********************************************************************************
//
// Key read state.
//
//***********************************************************************************
uint8_t KeyReadState1(KEY_NUM_E key)
{
    if(key < KEY_MAX_NUM)
        return(PIN_getInputValue(key_pin_id[key]));

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
   // rKeyTask.code = _VK_SELECT;
    //Sys_event_post(SYS_EVT_KEY);
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
    //rKeyTask.code = _VK_DOUBLE_KEY;
   // Sys_event_post(SYS_EVT_KEY);
}
#endif
