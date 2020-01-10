/*
* @Author: zxt
* @Date:   2020-01-10 17:39:17
* @Last Modified by:   zxt
* @Last Modified time: 2020-01-10 19:12:27
*/
#include "../general.h"



// board node
#define HIGH_LEVEL_ENABLE_PIN                      IOID_14		//高压档位
#define MID_LEVEL_ENABLE_PIN                       IOID_27		//中档电压档位
#define LOW_LEVEL_ENABLE_PIN                       IOID_28		//低档电压档位
#define SHOCK_CTR_ENABLE_PIN                       IOID_15		//电击功能总开关使能脚


#define TAMPER_CTR_ENABLE_PIN                      IOID_15		//防拆中断脚




static const uint8_t LED_ID_CONST[ELE_SHOCK_PIN_MAX] =
{
    HIGH_LEVEL_ENABLE_PIN,
    LOW_LEVEL_ENABLE_PIN,
    MID_LEVEL_ENABLE_PIN,
    SHOCK_CTR_ENABLE_PIN,
};



const PIN_Config eleShockPinTable[] = {
    HIGH_LEVEL_ENABLE_PIN | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,       /* LED initially off          */
    MID_LEVEL_ENABLE_PIN | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,       /* LED initially off          */
    LOW_LEVEL_ENABLE_PIN | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,       /* LED initially off          */
    SHOCK_CTR_ENABLE_PIN | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,       /* LED initially off          */
    TAMPER_CTR_ENABLE_PIN | PIN_INPUT_EN | PIN_PULLDOWN | PIN_IRQ_POSEDGE,       /* LED initially off          */
    PIN_TERMINATE
};



static Semaphore_Struct eleShockSemStruct;
static Semaphore_Handle eleShockSemHandle;

static PIN_State   eleShockState;
static PIN_Handle  eleShockHandle;

#define     LED_PERIOD_CLOCK_TIME_MS       10
singleport_drive_t singlePort[ELE_SHOCK_PIN_MAX];

Clock_Struct eleShockProcessClk;     /* not static so you can see in ROV */
Clock_Handle eleShockProcessClkHandle;


PWM_Handle eletricShockPulseHandle = NULL;

#define 	PWM_ELE_SHOCK_PULSE_FRQ			1
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

    PWM_Params_init(&params);
    params.dutyUnits   = PWM_DUTY_US;
    params.dutyValue   = 100L / PWM_ELE_SHOCK_PULSE_FRQ;	//100us 的脉冲
    params.periodUnits = PWM_PERIOD_US;
    params.periodValue = 1000000L/ PWM_ELE_SHOCK_PULSE_FRQ;	 	//1秒的周期
    params.idleLevel   = PWM_IDLE_LOW;
    eletricShockPulseHandle       = PWM_open(Board_PWM0, &params);
}



//***********************************************************************************
//
// eleShockIoInit.
//      
//
//***********************************************************************************
void eleShockIoInit(void)
{
    eleShockHandle = PIN_open(&eleShockState, eleShockPinTable);
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

    PIN_setOutputValue(eleShockHandle, LED_ID_CONST[ledId], state);

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
    PIN_setOutputValue(eleShockHandle, LED_ID_CONST[ledId], !(PIN_getOutputValue(LED_ID_CONST[ledId])));

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
    PIN_setOutputValue(eleShockHandle, LED_ID_CONST[ledId], status);

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
                        PIN_setOutputValue(eleShockHandle, LED_ID_CONST[i], !singlePort[i].state);
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
                            PIN_setOutputValue(eleShockHandle, LED_ID_CONST[i], singlePort[i].state);
                        singlePort[i].periodT1    = singlePort[i].periodT1Set;
                        singlePort[i].periodT2    = singlePort[i].periodT2Set;
                    }
                }
            }
            else
            {
                PIN_setOutputValue(eleShockHandle, LED_ID_CONST[i], !singlePort[i].state);
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


//***********************************************************************************
//
// Led init.
//
//***********************************************************************************
void ElectricShockInit(void)
{
    eleShockIoInit();

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
}


