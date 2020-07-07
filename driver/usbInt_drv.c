/*
* @Author: justfortest
* @Date:   2018-03-01 16:50:29
* @Last Modified by:   zxt
* @Last Modified time: 2020-07-07 09:24:46
*/
#include "../general.h"

#ifdef BOARD_S3
#define USB_INT_PIN                            IOID_4
#else
#define USB_INT_PIN                            IOID_11
#endif
void (*UsbIntIsrCb)(void);

static Clock_Struct usbIntClkStruct;
static Clock_Handle UsbIntClkHandle;



static PIN_State   usbIntPinState;
static PIN_Handle  usbIntPinHandle;


static uint8_t usbIntState;

const PIN_Config usbIntPinTable[] = {
    USB_INT_PIN | PIN_INPUT_EN | PIN_PULLUP | PIN_IRQ_BOTHEDGES,       /* key isr enable          */
    PIN_TERMINATE
};


//***********************************************************************************
// brief:   
// 
// parameter: 
//***********************************************************************************
void UsbIntIoInit(PIN_IntCb pCb)
{
    usbIntPinHandle = PIN_open(&usbIntPinState, usbIntPinTable);
    PIN_registerIntCb(usbIntPinHandle, pCb);
}


//***********************************************************************************
// brief:   
// 
// parameter: 
//***********************************************************************************
static void UsbIntIsrFxn(UInt index)
{
    if (Clock_isActive(UsbIntClkHandle) == FALSE)
    {
    	Clock_setTimeout(UsbIntClkHandle, 500 * CLOCK_UNIT_MS);
        Clock_start(UsbIntClkHandle);
    }
}




//***********************************************************************************
// brief:
//
// parameter:
//***********************************************************************************
void UsbIntScanFxn(void)
{
	usbIntState = PIN_getInputValue(USB_INT_PIN);
	if(UsbIntIsrCb)
		UsbIntIsrCb();
}

uint8_t GetUsbState(void)
{
	return usbIntState;
}

//***********************************************************************************
// brief:   
// 
// parameter: 
//***********************************************************************************
void UsbIntInit(void (*Cb)(void))
{

    /* Construct a 500ms single Clock Instance to scan key */
    Clock_Params clkParams;
    Clock_Params_init(&clkParams);
    clkParams.startFlag = FALSE;
    Clock_construct(&usbIntClkStruct, (Clock_FuncPtr)UsbIntScanFxn, 500 * CLOCK_UNIT_MS, &clkParams);
    /* Obtain clock instance handle */
    UsbIntClkHandle = Clock_handle(&usbIntClkStruct);

    /* install Button callback */
    UsbIntIoInit((PIN_IntCb)UsbIntIsrFxn);

    UsbIntIsrCb     = Cb;

    // updata the usb state
    UsbIntScanFxn();
}

//***********************************************************************************
//
// Battery get voltage value.
//
//***********************************************************************************
ChargeStateType Get_Charge_plug(void)
{
    if(GetUsbState()){
        return(CHARGEING);
    }
    else{
       return (NO_CHARGE);
    }
}

