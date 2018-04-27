/*
* @Author: zxt
* @Date:   2017-12-21 17:36:18
* @Last Modified by:   zxt
* @Last Modified time: 2018-04-27 10:01:12
*/
#include "../general.h"

/* UART Board */
#ifdef BOARD_S2_2

#define UART_RX_INTERFACE               IOID_11          /* RXD */
#define UART_TX_INTERFACE               IOID_8          /* TXD */

#endif

#ifdef BOARD_S6_6

#define UART_RX_INTERFACE               IOID_13          /* RXD */
#define UART_TX_INTERFACE               IOID_10          /* TXD */

#endif

#if (!defined BOARD_S6_6) && (!defined BOARD_S2_2)

#define UART_RX_INTERFACE               IOID_13          /* RXD */
#define UART_TX_INTERFACE               IOID_10          /* TXD */

#endif

#define UART_RX_GSM                     IOID_5          /* RXD */
#define UART_TX_GSM                     IOID_4          /* TXD */


/*
 *  =============================== UART ===============================
 */
#include <ti/drivers/UART.h>
#include <ti/drivers/uart/UARTCC26XX.h>
#include <ti/drivers/power/PowerCC26XX.h>

UARTCC26XX_Object uartCC26XXObjects[CC1310_LAUNCHXL_UARTCOUNT];

const UARTCC26XX_HWAttrsV2 uartCC26XXHWAttrs_Gsm[CC1310_LAUNCHXL_UARTCOUNT] = {
    {
        .baseAddr       = UART0_BASE,
        .powerMngrId    = PowerCC26XX_PERIPH_UART0,
        .intNum         = INT_UART0_COMB,
        .intPriority    = ~0,
        .swiPriority    = 0,
        .txPin          = UART_TX_GSM,
        .rxPin          = UART_RX_GSM,
        .ctsPin         = PIN_UNASSIGNED,
        .rtsPin         = PIN_UNASSIGNED
    }
};

const UARTCC26XX_HWAttrsV2 uartCC26XXHWAttrs_Interface[CC1310_LAUNCHXL_UARTCOUNT] = {
    {
        .baseAddr       = UART0_BASE,
        .powerMngrId    = PowerCC26XX_PERIPH_UART0,
        .intNum         = INT_UART0_COMB,
        .intPriority    = ~0,
        .swiPriority    = 0,
        .txPin          = UART_TX_INTERFACE,
        .rxPin          = UART_RX_INTERFACE,
        .ctsPin         = PIN_UNASSIGNED,
        .rtsPin         = PIN_UNASSIGNED
    }
};

const PIN_Config uart_pin_interface[] = {
    UART_TX_INTERFACE | PIN_INPUT_EN | PIN_PULLDOWN | PIN_IRQ_POSEDGE,       /* interface uart set as input          */
    UART_RX_INTERFACE | PIN_INPUT_EN | PIN_PULLDOWN | PIN_IRQ_POSEDGE,       /* interface uart set as input          */
    PIN_TERMINATE
};


UART_Config UART_config[CC1310_LAUNCHXL_UARTCOUNT] = {
    {
        .fxnTablePtr = &UARTCC26XX_fxnTable,
        .object      = &uartCC26XXObjects[CC1310_LAUNCHXL_UART0],
        .hwAttrs     = &uartCC26XXHWAttrs_Gsm[CC1310_LAUNCHXL_UART0]
    },
};


const uint_least8_t UART_count = CC1310_LAUNCHXL_UARTCOUNT;


                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        

static UART_Handle      uarthandle[CC1310_LAUNCHXL_UARTCOUNT];




const PIN_Config uart_pin_gsm[] = {
    UART_TX_GSM | PIN_INPUT_EN | PIN_PULLDOWN | PIN_IRQ_DIS,       /* gsm uart set as input          */
    UART_RX_GSM | PIN_INPUT_EN | PIN_PULLDOWN | PIN_IRQ_DIS,       /* gsm uart set as input          */
    PIN_TERMINATE
};



// 
static PIN_State   interfacePortState, gsmPortState;
static PIN_Handle  interfacePortHandle, gsmPortHandle;

/***************************************************************************/


// variable
static void (*Uart0IsrCb)(void);

UartRxData_t     g_rUart1RxData;
UartRxData_t     uart0RxData;
UartRxData_t     uart0TxData;


void uart0Isr(void)
{
    uint32_t               intStatus;
    intStatus       = UARTIntStatus(UART0_BASE, true);
    UARTIntClear(UART0_BASE,intStatus);

    if(intStatus & UART_INT_RX || intStatus & UART_INT_RT)
    {
        while(1)
        if(!(HWREG(UART0_BASE + UART_O_FR) & UART_FR_RXFE))
        {
            // Read and return the next character.
            g_rUart1RxData.buff[g_rUart1RxData.length] = (HWREG(UART0_BASE + UART_O_DR));
            g_rUart1RxData.length++;
            if(g_rUart1RxData.length >= UART_BUFF_SIZE)
                g_rUart1RxData.length = 0;
        }
        else
        {
            break;
        }
        Uart0IsrCb();
    }
}


//***********************************************************************************
//
// UART0/1 init, use USCI_A0/1.
//
//***********************************************************************************
void UartHwInit(UART_PORT uartPort, uint32_t baudrate, UART_CB_T Cb, uint8_t type)
{

    if (uartPort >= UART_MAX) {
        return;
    }


    if(type == UART_INTERFACE)
    {
        if(interfacePortHandle)
        {
            PIN_close(interfacePortHandle);
            interfacePortHandle = NULL;
        }
        UartPortDisable(UART_GSM);

    }

    if(type == UART_GSM)
    {
        if(gsmPortHandle)
        {
            PIN_close(gsmPortHandle);
            gsmPortHandle = NULL;
        }
        UartPortDisable(UART_INTERFACE);
    }

    UartClose(uartPort);

    // UartPortEnable(type);

    if(type == UART_GSM)
    {
        UART_config[uartPort].hwAttrs     = &uartCC26XXHWAttrs_Gsm[uartPort];
    }

    if(type == UART_INTERFACE)
    {
        UART_config[uartPort].hwAttrs     = &uartCC26XXHWAttrs_Interface[uartPort];
    }

    UART_init();

    UART_Params uartParams;

    UART_Params_init(&uartParams);

    uartParams.readMode       = UART_MODE_CALLBACK;
    uartParams.writeMode      = UART_MODE_BLOCKING;
    uartParams.readTimeout    = UART_WAIT_FOREVER;
    uartParams.writeTimeout   = UART_WAIT_FOREVER;
    uartParams.readCallback   = (UART_Callback)uart0Isr;
    uartParams.writeCallback  = NULL;
    uartParams.readReturnMode = UART_RETURN_FULL;
    uartParams.readDataMode   = UART_DATA_BINARY;
    uartParams.writeDataMode  = UART_DATA_BINARY;
    uartParams.readEcho       = UART_ECHO_OFF;
    uartParams.baudRate       = baudrate;
    uartParams.dataLength     = UART_LEN_8;
    uartParams.stopBits       = UART_STOP_ONE;
    uartParams.parityType     = UART_PAR_NONE;

    Uart0IsrCb                = Cb;

    uarthandle[uartPort]      = UART_open(CC1310_LAUNCHXL_UART0, &uartParams);

    // enable the uart int
    UART_read(uarthandle[CC1310_LAUNCHXL_UART0], (uint8_t *)&baudrate, 1);

}

void UartClose(UART_PORT uartPort)
{
    if (uartPort >= UART_MAX) {
        return;
    }

    if(uarthandle[uartPort])
    {
        UART_readCancel(uarthandle[uartPort]);
        UART_close(uarthandle[uartPort]);
        uarthandle[uartPort] = NULL;
    }
}



void UartSendDatas(UART_PORT uartPort, uint8_t *buf, uint8_t count)
{
    if (uartPort >= UART_MAX) {
        return;
    }
    UART_write(uarthandle[uartPort], buf, count);
}


//***********************************************************************************
//
// UART0/1 send burst byte, use USCI_A0/1.
//
//***********************************************************************************
void Uart_send_burst_data(UART_PORT uartPort, uint8_t *pData, uint16_t length)
{
    uint16_t i;
    UARTCC26XX_HWAttrsV2 const     *hwAttrs;

    hwAttrs = uarthandle[uartPort]->hwAttrs;

    if (uartPort >= UART_MAX) {
        return;
    }

    /* Enable TX */
    HWREG(UART0_BASE + UART_O_CTL) |= UART_CTL_TXE;

    for (i = 0; i < length; i++) {
        // wait for tx finish
        while(HWREG(hwAttrs->baseAddr + UART_O_FR) & UART_FR_TXFF);
        // Send the char.
        HWREG(hwAttrs->baseAddr + UART_O_DR) = *(pData+i);

        // wait for tx finish
        while(HWREG(hwAttrs->baseAddr + UART_O_FR) & UART_FR_TXFF);
    }

    // if((length % 2) != 0)
    // {

    //     // Send the char.
    //     HWREG(hwAttrs->baseAddr + UART_O_DR) = 0;
        
    //     // wait for tx finish
    //     while(HWREG(hwAttrs->baseAddr + UART_O_FR) & UART_FR_TXFF);
    // }
    __delay_cycles(800);
    while(HWREG(hwAttrs->baseAddr + UART_O_FR) & UART_FR_BUSY);

    /* Disable UART TX */
    HWREG(UART0_BASE + UART_O_CTL) &= ~(UART_CTL_TXE);
}

//***********************************************************************************
//
// UART0/1 send string, use USCI_A0/1.
//
//***********************************************************************************
void Uart_send_string(UART_PORT uartPort, uint8_t *string)
{
    UARTCC26XX_HWAttrsV2 const     *hwAttrs;
    
    hwAttrs = uarthandle[uartPort]->hwAttrs;

    /* Enable TX */
    HWREG(UART0_BASE + UART_O_CTL) |= UART_CTL_TXE;

    

    while (*string != '\0') {
        // wait for tx finish
        while(HWREG(hwAttrs->baseAddr + UART_O_FR) & UART_FR_TXFF);

        // Send the char.
        HWREG(hwAttrs->baseAddr + UART_O_DR) = *(string++);

        // wait for tx finish
        while(HWREG(hwAttrs->baseAddr + UART_O_FR) & UART_FR_TXFF);
    }

    __delay_cycles(800);
    while(HWREG(hwAttrs->baseAddr + UART_O_FR) & UART_FR_BUSY);

    /* Disable UART TX */
    HWREG(UART0_BASE + UART_O_CTL) &= ~(UART_CTL_TXE);

}



void UartPortDisable(uint8_t type)
{
    if(type == UART_INTERFACE)
    {
        if(interfacePortHandle == NULL)
            interfacePortHandle = PIN_open(&interfacePortState, uart_pin_interface);
    }

    if(type == UART_GSM)
    {
        if(gsmPortHandle == NULL)
            gsmPortHandle = PIN_open(&gsmPortState, uart_pin_gsm);
    }
}


void UartPortEnable(uint8_t type)
{
    if(type == UART_INTERFACE)
    {
        if(interfacePortHandle)
            PIN_close(interfacePortHandle);


        interfacePortHandle = NULL;
    }


    if(type == UART_GSM)
    {
        if(gsmPortHandle)
            PIN_close(gsmPortHandle);
     
        gsmPortHandle       = NULL;
    }

}

