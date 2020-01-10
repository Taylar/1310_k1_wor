#include "../general.h"

#ifdef SUPPORT_CHARGE_DECT

static const PIN_Config chargeDectPinConfig[] = {
    CHARGE_DETECT_PIN  | PIN_INPUT_EN | PIN_NOPULL,
    PIN_TERMINATE
};
PIN_State   chargeDectPinState;
PIN_Handle  chargeDectPinHandle = NULL;



//***********************************************************************************
//
// Battery init.
//
//***********************************************************************************
void Charge_detect_init(void)
{
    chargeDectPinHandle = PIN_open(&chargeDectPinState, chargeDectPinConfig);

}







#endif  /* SUPPORT_BATTERY */

