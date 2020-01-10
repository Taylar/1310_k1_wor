/*
* @Author: zxt
* @Date:   2017-12-21 17:36:18
* @Last Modified by:   zxt
* @Last Modified time: 2020-01-10 14:38:28
*/

#include "../general.h"
#define     KEY_IC_ADDR     0x58
#define     KEY_IC_P0_INVALID   0X0F
#define     KEY_IC_P1_INVALID   0XF0

static Clock_Struct keyClkStruct;
static Clock_Handle keyClkHandle;

static KeyTask_t rKeyTask;


// node board
#ifdef BOARD_S3
#define Board_BUTTON_INT                            IOID_4

const PIN_Config keyPinTable[] = {
    Board_BUTTON_INT | PIN_INPUT_EN | PIN_PULLUP | PIN_IRQ_NEGEDGE,       /* key isr enable          */
    PIN_TERMINATE
};

#endif

// gateway board

// S6_6 board
#ifdef BOARD_S6_6
#define Board_BUTTON_INT                    IOID_14
#define Board_BUTTON_RES                    IOID_4

const PIN_Config keyPinTable[] = {
    Board_BUTTON_INT | PIN_INPUT_EN | PIN_PULLUP | PIN_IRQ_NEGEDGE,       /* key isr enable          */
    PIN_TERMINATE
};

const PIN_Config keyResPinTable[] = {
    Board_BUTTON_RES | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX,       /* LED initially off          */
    PIN_TERMINATE
};



static PIN_State   key1State;
static PIN_Handle  keyResHandle;
#endif



static PIN_State   keyState;
static PIN_Handle  keyHandle;

static const uint8_t key_pin_id[KEY_MAX_NUM] = 
{
    Board_BUTTON_INT,
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
void Key1IoInit(void)
{
    keyResHandle = PIN_open(&key1State, keyResPinTable);
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
    rKeyTask.scanPort        = 0;
    rKeyTask.scanState       = 0;

    Clock_stop(keyClkHandle);
}

#ifdef BOARD_S6_6

KEY_CODE_E Key_get(void)
{
    return rKeyTask.keyCode;
}


void KeySetP0InP1Out(void)
{
    I2C_bytewrite(KEY_IC_ADDR << 1, 0x04, 0XFF); //设置P0口作为输入
    I2C_bytewrite(KEY_IC_ADDR << 1, 0x05, 0X00); //设置P1口作为输出


    I2C_bytewrite(KEY_IC_ADDR << 1, 0x03, 0X00); //设置P1口输出低
}

void KeySetP1InP0Out(void)
{
    I2C_bytewrite(KEY_IC_ADDR << 1, 0x04, 0X00); //设置P0口作为输出
    I2C_bytewrite(KEY_IC_ADDR << 1, 0x05, 0XFF); //设置P1口作为输入


    I2C_bytewrite(KEY_IC_ADDR << 1, 0x02, 0X00); //设置P0口输出低

}


static void KeyIcInit(void)
{
    uint8_t controlReg;
    PIN_setOutputValue(keyResHandle, Board_BUTTON_RES, 1);
    Task_sleep(10 * CLOCK_UNIT_MS);

    I2C_bytewrite(KEY_IC_ADDR << 1, 0x12, 0XFF); //设置P0口为gpio模式
    I2C_bytewrite(KEY_IC_ADDR << 1, 0x13, 0XFF); //设置P1口为gpio模式

    controlReg = I2C_byteread(KEY_IC_ADDR << 1, 0x11);
    I2C_bytewrite(KEY_IC_ADDR << 1, 0x11, controlReg | (0x01 << 4)); //设置P0口为推挽式输出
    controlReg = I2C_byteread(KEY_IC_ADDR << 1, 0x11);

    if(controlReg == (0x01 << 4))
        KeySetP0InP1Out();
}

uint8_t KeyReadP0(void)
{
    uint8_t portData;
    portData = I2C_byteread(KEY_IC_ADDR << 1, 0x00) >> 3;
    return (portData&0x0f);
}

uint8_t KeyReadP1(void)
{
    uint8_t portData;
    portData = I2C_byteread(KEY_IC_ADDR << 1, 0x01) << 3;
    return (portData&0xf0);
}

void KeyScanCbEvent(UArg arg0)
{
    Sys_event_post(SYS_EVT_KEY_SCAN);
}

void KeyScanFxn(void)
{
    uint8_t portState;
    if(rKeyTask.scanPort == 0){
        portState = KeyReadP0();
        if(portState == KEY_IC_P0_INVALID){
            KeyScanStop();
            return; 
        }
        rKeyTask.scanState = portState;
        rKeyTask.scanPort = 1;
        KeySetP1InP0Out();
    }else if(rKeyTask.scanPort){
        portState = KeyReadP1();
        if(portState == KEY_IC_P1_INVALID){
            KeyScanStop();
            return; 
        }
        rKeyTask.scanState |= portState;
        rKeyTask.keyCode = rKeyTask.scanState;
        KeySetP0InP1Out();
        // 延迟一段时间，等待中断产生，并读取IO口状态，清除中断标志
        Task_sleep(2 * CLOCK_UNIT_MS);
        KeyReadP0();
        KeyReadP1();
        KeyScanStop();
        Sys_event_post(SYSTEMAPP_EVT_KEY);
    }
}
#else
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

#endif //BOARD_S6_6
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
// Key init.
//
//***********************************************************************************
void KeyInit(void)
{
    uint8_t i;

    rKeyTask.holdTime        = 0;
    rKeyTask.doublePressTime = 0;
    rKeyTask.shortPress      = 0;
    rKeyTask.doublePress     = 0;
    rKeyTask.scanPort        = 0;
    rKeyTask.scanState       = 0;
    rKeyTask.keyCode         = _VK_NULL;

    /* Construct a 10ms periodic Clock Instance to scan key */
    Clock_Params clkParams;
    Clock_Params_init(&clkParams);
    clkParams.period = 10 * CLOCK_UNIT_MS;
    clkParams.startFlag = FALSE;
#ifdef BOARD_S6_6
    Clock_construct(&keyClkStruct, (Clock_FuncPtr)KeyScanCbEvent, 0, &clkParams);
#else
    Clock_construct(&keyClkStruct, (Clock_FuncPtr)KeyScanFxn, 0, &clkParams);
#endif
    /* Obtain clock instance handle */
    keyClkHandle = Clock_handle(&keyClkStruct);

    for(i = 0; i < KEY_ACTION_MAX; i++)
    {
        AppKeyIsrCb[i] = NULL;
    }

    KeyIoInit((PIN_IntCb)KeyIsrFxn);
#ifdef  BOARD_S6_6
    Key1IoInit();
    Task_sleep(2 * CLOCK_UNIT_MS);
    KeyIcInit();

    // 检测是否有按键按下
    if(KeyReadP0() != 0x0f)
        KeyIsrFxn(0);
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
