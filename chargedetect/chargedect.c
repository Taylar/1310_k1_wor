#include "../general.h"

#ifdef SUPPORT_CHARGE_DECT

static const PIN_Config chargeDectPinConfig[] = {
    CHARGE_DETECT_PIN  | PIN_INPUT_EN | PIN_NOPULL,
    PIN_TERMINATE
};
static PIN_State   chargeDectPinState;
static PIN_Handle  chargeDectPinHandle = NULL;

#if 0
static const PIN_Config chargeRegPinConfig[] = {
    CHARGE_STATUS_PIN | PIN_INPUT_EN | PIN_NOPULL | PIN_IRQ_POSEDGE,
    PIN_TERMINATE
};
static PIN_State   chargeRegPinState;
static PIN_Handle  chargeRegPinHandle = NULL;

//static uint16_t bBatVoltage;
static void ChargeDet_isrFxn(UInt index)
{
    Sys_event_post(SYS_EVT_USB_CHARGE);
}
#endif


//***********************************************************************************
//
// Battery init.
//
//***********************************************************************************
void Charge_detect_init(void)
{
    chargeDectPinHandle = PIN_open(&chargeDectPinState, chargeDectPinConfig);
#if 0
    chargeRegPinHandle = PIN_open(&chargeRegPinState, chargeRegPinConfig);
    PIN_registerIntCb(chargeRegPinHandle, ChargeDet_isrFxn);
#endif

}



//***********************************************************************************
//
// Battery get voltage value.
//
//***********************************************************************************
ChargeStateType Get_Charge_plug(void)
{
    if(GET_PLUG_IN_STATUS()){
#ifndef BOARD_S6_6
        if(GET_CHARGE_STATUS()){
            return(CHARGE_FULL);
        }
        else
#endif // S6_6
        {
            return(CHARGEING);
        }
    }
    else{
       return (NO_CHARGE);
    }
}



#endif  /* SUPPORT_BATTERY */

