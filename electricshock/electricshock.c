/*
* @Author: justfortest
* @Date:   2020-01-10 17:39:17
* @Last Modified by:   zxt
* @Last Modified time: 2020-08-10 15:24:46
*/
#include "../general.h"



// board node
#define HIGH_LEVEL_ENABLE_PIN                       IOID_14     //高压档位
#define MID_LEVEL_ENABLE_PIN                        IOID_27     //中档电压档位
#define LOW_LEVEL_ENABLE_PIN                        IOID_28     //低档电压档位
#define SHOCK_CTR_ENABLE_PIN                        IOID_15     //电击功能总开关使能脚

#define MOTO_ENABLE_PIN                             IOID_2      //马达使能脚


#define PREVENTIVE_INSERT_ENABLE_PIN                IOID_20      //防塞检测开启
#define PREVENTIVE_INSERT_ENABLE_PIN2               IOID_23      //防塞检测开启

#define MOTO_INT_PIN                                IOID_1      //马达中断脚

#define DESTROY_INT_PIN                             IOID_3      //防拆中断脚


const PIN_Config motoIntPinTable[] = {
    MOTO_INT_PIN | PIN_INPUT_EN | PIN_PULLUP | PIN_IRQ_NEGEDGE,       /* 马达中断          */
    PIN_TERMINATE
};

const PIN_Config destroyIntPinTable[] = {
    DESTROY_INT_PIN | PIN_INPUT_EN | PIN_PULLDOWN | PIN_IRQ_POSEDGE,       /* 防拆中断          */
    PIN_TERMINATE
};


static const uint8_t ELE_CTR_ID_CONST[ELE_SHOCK_PIN_MAX] =
{
    HIGH_LEVEL_ENABLE_PIN,
    MID_LEVEL_ENABLE_PIN,
    LOW_LEVEL_ENABLE_PIN,
    SHOCK_CTR_ENABLE_PIN,
    MOTO_ENABLE_PIN,
    PREVENTIVE_INSERT_ENABLE_PIN,
    PREVENTIVE_INSERT_ENABLE_PIN2,
};



const PIN_Config eleShockPinTable[] = {
    HIGH_LEVEL_ENABLE_PIN | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,       /* 高压档位         */
    MID_LEVEL_ENABLE_PIN | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,       /* 中档电压档位         */
    LOW_LEVEL_ENABLE_PIN | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,       /* 低档电压档位         */
    SHOCK_CTR_ENABLE_PIN | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX,       /* 电击功能总开关使能脚          */
    MOTO_ENABLE_PIN | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,       /* 马达使能脚         */
    PREVENTIVE_INSERT_ENABLE_PIN | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,       /* 防塞检测开启          */
    PREVENTIVE_INSERT_ENABLE_PIN2 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,       /* 防塞检测开启          */
    PIN_TERMINATE
};


static PIN_State   motoIntState, destroyIntState;
static PIN_Handle  motoIntHandle, destroyIntHandle;


static Semaphore_Struct eleShockSemStruct;
static Semaphore_Handle eleShockSemHandle;

static PIN_State   eleShockState;
static PIN_Handle  eleShockHandle;

Clock_Struct pulseClkStruct;
Clock_Handle pulseClkHandle;



#define     LED_PERIOD_CLOCK_TIME_MS       10
singleport_drive_t singlePort[ELE_SHOCK_PIN_MAX];

Clock_Struct eleShockProcessClk;     /* not static so you can see in ROV */
Clock_Handle eleShockProcessClkHandle;


PWM_Handle eletricShockPulseHandle = NULL;

#define     PWM_ELE_SHOCK_PULSE_FRQ         33

uint16_t    pulseTimes_sec = 0;


void PulseFxn(UArg arg0)
{
    pulseTimes_sec--;
    if(pulseTimes_sec == 0){
        ElectricShockPowerDisable();
        EletricShockPulseDisable();
        Clock_stop(pulseClkHandle);
    }
}
//***********************************************************************************
//
// System buzzer enable.
//
//***********************************************************************************
void EletricShockPulseEnable(void)
{
    PWM_start(eletricShockPulseHandle);
}


//***********************************************************************************
//
// System buzzer disable.
//
//***********************************************************************************
void EletricShockPulseDisable(void)
{
    PWM_stop(eletricShockPulseHandle);
}

//***********************************************************************************
//
// System buzzer init.
//
//***********************************************************************************
void EletricShockPulseInit(void)
{
    PWM_Params params;
    Clock_Params clkParams;

    Clock_Params_init(&clkParams);
    clkParams.period = 1000 * CLOCK_UNIT_MS;
    clkParams.startFlag = FALSE;
    Clock_construct(&pulseClkStruct, (Clock_FuncPtr)PulseFxn, 1000 * CLOCK_UNIT_MS, &clkParams);
    /* Obtain clock instance handle */
    pulseClkHandle = Clock_handle(&pulseClkStruct);


    PWM_Params_init(&params);
    params.dutyUnits   = PWM_DUTY_US;
    params.dutyValue   = 1000000L / PWM_ELE_SHOCK_PULSE_FRQ / 230;//230:130us 300:100us 的脉冲
    params.periodUnits = PWM_PERIOD_US;
    params.periodValue = 1000000L/ PWM_ELE_SHOCK_PULSE_FRQ;     //1秒的周期
    params.idleLevel   = PWM_IDLE_LOW;
    eletricShockPulseHandle       = PWM_open(Board_PWM0, &params);
}


void EletricPulseSetTime_S(uint16_t keepTime_S)
{
    pulseTimes_sec = keepTime_S;

    if(electricshockEnable == 0){
        EletricShockPulseDisable();
        Clock_stop(pulseClkHandle);
        return;
    }
    if(pulseTimes_sec == 0){
        EletricShockPulseDisable();
        Clock_stop(pulseClkHandle);
    }
    else{
        Clock_start(pulseClkHandle);
        ElectricShockPowerEnable();
        EletricShockPulseEnable();
    }
}

#define     INSERT_DECTECT_VALUE        1430
ADC_Handle   preventInsertHandle;
uint8_t     insertOccur = 0;


void ElecPreventInsertInit(void)
{
#ifdef BOARD_S3
    ADC_Params   params;

    ADC_Params_init(&params);

    if(preventInsertHandle == NULL)
        preventInsertHandle = ADC_open(PREVENTIVE_INSERT_ADC, &params);
#endif //BOARD_S3
}
static uint32_t volvalue;
void ElecPreventInsertMeasure(void)
{
    uint16_t temp;

    // eleShock_set(ELE_PREVENT_INSERT_ENABLE, 1);
    // eleShock_set(ELE_PREVENT_INSERT2_ENABLE, 1);
    Task_sleep(100 * CLOCK_UNIT_MS);
    ADC_convert(preventInsertHandle, &temp);
    volvalue    = ADC_convertToMicroVolts(preventInsertHandle, temp);
    if((volvalue/1000) > INSERT_DECTECT_VALUE)
        insertOccur = 1;
    else
        insertOccur = 0;

    if(insertOccur == 0){
        eleShock_set(ELE_PREVENT_INSERT_ENABLE, 0);
        eleShock_set(ELE_PREVENT_INSERT2_ENABLE, 0);
    }
}


uint8_t ElecPreventInsertState(void)
{
    return insertOccur;
}


//***********************************************************************************
//
// eletric shock control.
//      id:     allow multi led
//      state:  led first state, 0 or 1
//      period: led blink time,  0 means just set led state and no blink
//      times:  led blink times, 0 means just set led state and no blink
//
//***********************************************************************************
void eleShock_ctrl(uint8_t ledId, uint8_t state, uint32_t period, uint8_t times)
{
    eleShock_ctrl2(ledId, state, period, period, times);
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
void eleShock_ctrl2(uint8_t ledId, uint8_t state, uint32_t period, uint32_t period2, uint8_t times)
{
    uint8_t i;
    /* Get access to resource */

    period  /= CLOCK_UNIT_MS; 
    period2 /= CLOCK_UNIT_MS;

    if(Clock_isActive(eleShockProcessClkHandle))
        Clock_stop(eleShockProcessClkHandle);

    Semaphore_pend(eleShockSemHandle, BIOS_WAIT_FOREVER);

    PIN_setOutputValue(eleShockHandle, ELE_CTR_ID_CONST[ledId], state);

    if (period == 0 || times == 0) {
        /* Unlock resource */

        for(i = 0; i < ELE_SHOCK_PIN_MAX; i++)
        {
            if(singlePort[i].enable)
                break;
        }
        if(i < ELE_SHOCK_PIN_MAX)
        {
            Clock_start(eleShockProcessClkHandle);
        }

        Semaphore_post(eleShockSemHandle);
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
    Semaphore_post(eleShockSemHandle);

    Clock_start(eleShockProcessClkHandle);
}

//***********************************************************************************
//
// Led toggle.
//      id:     allow multi led
//
//***********************************************************************************
void eleShock_toggle(uint8_t ledId)
{
    /* Get access to resource */
    Semaphore_pend(eleShockSemHandle, BIOS_WAIT_FOREVER);

//    GPIO_toggleOutputOnPin(rLedHWAttrs[ledId].port, rLedHWAttrs[ledId].pin);
    PIN_setOutputValue(eleShockHandle, ELE_CTR_ID_CONST[ledId], !(PIN_getOutputValue(ELE_CTR_ID_CONST[ledId])));

    /* Unlock resource */
    Semaphore_post(eleShockSemHandle);
}

//***********************************************************************************
//
// Led toggle.
//      id:     allow multi led
//
//***********************************************************************************
void eleShock_set(uint8_t ledId, uint8_t status)
{
    /* Get access to resource */
    Semaphore_pend(eleShockSemHandle, BIOS_WAIT_FOREVER);

//    GPIO_toggleOutputOnPin(rLedHWAttrs[ledId].port, rLedHWAttrs[ledId].pin);
    PIN_setOutputValue(eleShockHandle, ELE_CTR_ID_CONST[ledId], status);

    /* Unlock resource */
    Semaphore_post(eleShockSemHandle);
}

//***********************************************************************************
//
// Led init.
//
//***********************************************************************************
void eleShock_clk_cb(UArg arg0)
{
    uint8_t i;
    for(i = 0; i < ELE_SHOCK_PIN_MAX; i++)
    {
        if(singlePort[i].enable)
        {
            if(singlePort[i].times)
            {
                if(singlePort[i].periodT1)
                {
                    singlePort[i].periodT1--;
                    if(singlePort[i].periodT1 == 0)
                        PIN_setOutputValue(eleShockHandle, ELE_CTR_ID_CONST[i], !singlePort[i].state);
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
                            PIN_setOutputValue(eleShockHandle, ELE_CTR_ID_CONST[i], singlePort[i].state);
                        singlePort[i].periodT1    = singlePort[i].periodT1Set;
                        singlePort[i].periodT2    = singlePort[i].periodT2Set;
                    }
                }
            }
            else
            {
                PIN_setOutputValue(eleShockHandle, ELE_CTR_ID_CONST[i], !singlePort[i].state);
                singlePort[i].enable = 0;
            }
        }
    }
    for(i = 0; i < ELE_SHOCK_PIN_MAX; i++)
    {
        if(singlePort[i].enable == true)
            break;
    }
    if(i >= ELE_SHOCK_PIN_MAX)
    {
        Clock_stop(eleShockProcessClkHandle);
    }
}

static void MotoIsrFxn(PIN_Handle handle, PIN_Id pinId)
{
    Sys_event_post(SYS_EVT_MOTO_INT_REC);
}

static void DestroyIsrFxn(PIN_Handle handle, PIN_Id pinId)
{
    Sys_event_post(SYS_EVT_ELE_SHOCK_DESTROY);
}

uint8_t DestroyPinRead(void)
{
    return PIN_getInputValue(DESTROY_INT_PIN);
}

//***********************************************************************************
//
// Led init.
//
//***********************************************************************************
void ElectricShockInit(void)
{
    eleShockHandle = PIN_open(&eleShockState, eleShockPinTable);

    motoIntHandle = PIN_open(&motoIntState, motoIntPinTable);
    PIN_registerIntCb(motoIntHandle, MotoIsrFxn);

    destroyIntHandle = PIN_open(&destroyIntState, destroyIntPinTable);
    PIN_registerIntCb(destroyIntHandle, DestroyIsrFxn);

    // 电击脉冲初始化
    EletricShockPulseInit();

    // 防塞检测初始化
    ElecPreventInsertInit();

    /* Construct a Semaphore object to be use as a resource lock, inital count 1 */
    Semaphore_Params ledSemParams;
    Semaphore_Params_init(&ledSemParams);
    ledSemParams.mode = Semaphore_Mode_BINARY;
    Semaphore_construct(&eleShockSemStruct, 1, &ledSemParams);
    /* Obtain instance handle */
    eleShockSemHandle = Semaphore_handle(&eleShockSemStruct);

    Clock_Params clkParams;
    Clock_Params_init(&clkParams);
    clkParams.period    = 1;
    clkParams.startFlag = FALSE;
    Clock_construct(&eleShockProcessClk, eleShock_clk_cb, 1, &clkParams);
    eleShockProcessClkHandle = Clock_handle(&eleShockProcessClk);
    Clock_setTimeout(eleShockProcessClkHandle, LED_PERIOD_CLOCK_TIME_MS * CLOCK_UNIT_MS);
    Clock_setPeriod(eleShockProcessClkHandle, LED_PERIOD_CLOCK_TIME_MS * CLOCK_UNIT_MS);

    destroyEleShock = 1;

    if(g_rSysConfigInfo.electricLevel > ELECTRIC_HIGH_LEVEL)
        g_rSysConfigInfo.electricLevel = ELECTRIC_HIGH_LEVEL;
    ElectricShockLevelSet(g_rSysConfigInfo.electricLevel);
    
    // ElectricShockLevelSet(g_rSysConfigInfo.electricLevel);
    if(g_rSysConfigInfo.electricFunc & ELE_FUNC_ENABLE_SHOCK){
        electricshockEnable = 1;
        // ElectricShockPowerEnable();
    }
    else{
        electricshockEnable = 0;
        ElectricShockPowerDisable();
    }
}


// 设置电击强度
void ElectricShockLevelSet(uint8_t level)
{
    switch(level)
    {
        case ELECTRIC_LOW_LEVEL:
        eleShock_set(ELE_SHOCK_HIGH, 0);
        eleShock_set(ELE_SHOCK_MID, 0);
        eleShock_set(ELE_SHOCK_LOW, 1);
        break;


        case ELECTRIC_MID_LEVEL:
        eleShock_set(ELE_SHOCK_HIGH, 0);
        eleShock_set(ELE_SHOCK_LOW, 0);
        eleShock_set(ELE_SHOCK_MID, 1);
        break;


        case ELECTRIC_HIGH_LEVEL:
        eleShock_set(ELE_SHOCK_MID, 0);
        eleShock_set(ELE_SHOCK_LOW, 0);
        eleShock_set(ELE_SHOCK_HIGH, 1);
        break;
    }
}


void ElectricShockPowerEnable(void)
{
    eleShock_set(ELE_SHOCK_POWER_ENABLE, 0);
}


void ElectricShockPowerDisable(void)
{
    eleShock_set(ELE_SHOCK_POWER_ENABLE, 1);
}


