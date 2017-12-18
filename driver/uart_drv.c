//***********************************************************************************
// Copyright 2017, Zksiot Development Ltd.
// Created by Zhengxuntai, 2017.12.18
// MCU:	MSP430F5529
// OS: TI-RTOS
// Project:
// File name: uart_drv.c
// Description: uart process routine.
//***********************************************************************************
#include "../general.h"

/* UART Board */
#define CC1310_LAUNCHXL_UART_RX               IOID_2          /* RXD */
#define CC1310_LAUNCHXL_UART_TX               IOID_3          /* TXD */
#define CC1310_LAUNCHXL_UART_CTS              IOID_19         /* CTS */
#define CC1310_LAUNCHXL_UART_RTS              IOID_18         /* RTS */

/*
 *  =============================== UART ===============================
 */
#include <ti/drivers/UART.h>
#include <ti/drivers/uart/UARTCC26XX.h>
#include <ti/drivers/power/PowerCC26XX.h>

UARTCC26XX_Object uartCC26XXObjects[CC1310_LAUNCHXL_UARTCOUNT];

const UARTCC26XX_HWAttrsV2 uartCC26XXHWAttrs[CC1310_LAUNCHXL_UARTCOUNT] = {
    {
        .baseAddr       = UART0_BASE,
        .powerMngrId    = PowerCC26XX_PERIPH_UART0,
        .intNum         = INT_UART0_COMB,
        .intPriority    = ~0,
        .swiPriority    = 0,
        .txPin          = CC1310_LAUNCHXL_UART_TX,
        .rxPin          = CC1310_LAUNCHXL_UART_RX,
        .ctsPin         = PIN_UNASSIGNED,
        .rtsPin         = PIN_UNASSIGNED
    }
};

const UART_Config UART_config[CC1310_LAUNCHXL_UARTCOUNT] = {
    {
        .fxnTablePtr = &UARTCC26XX_fxnTable,
        .object      = &uartCC26XXObjects[CC1310_LAUNCHXL_UART0],
        .hwAttrs     = &uartCC26XXHWAttrs[CC1310_LAUNCHXL_UART0]
    },
};

const uint_least8_t UART_count = CC1310_LAUNCHXL_UARTCOUNT;
/***************************************************************************/

static UART_Callback    UartCbRegister[CC1310_LAUNCHXL_UARTCOUNT];
static UART_Handle      uarthandle[CC1310_LAUNCHXL_UARTCOUNT];



static void UartReceiveIsrCb(UART_Handle handle, void *buf, size_t count)
{
    uint8_t         dataBuf;
    UART_read(handle, &dataBuf, 1);
    UartCbRegister[CC1310_LAUNCHXL_UART0](UART_0, &dataBuf, 1);
}

//***********************************************************************************
//
// UART0/1 init, use USCI_A0/1.
//
//***********************************************************************************
void UartHwInit(UART_PORT uartPort, uint32_t baudrate, UART_Callback callback)
{

    if (uartPort >= UART_MAX) {
        return;
    }

    UART_init();

    UART_Params uartParams;

    UART_Params_init(&uartParams);

    uartParams.readMode       = UART_MODE_CALLBACK;
    uartParams.writeMode      = UART_MODE_BLOCKING;
    uartParams.readTimeout    = UART_WAIT_FOREVER;
    uartParams.writeTimeout   = UART_WAIT_FOREVER;
    uartParams.readCallback   = UartReceiveIsrCb;
    uartParams.writeCallback  = NULL;
    uartParams.readReturnMode = UART_RETURN_NEWLINE;
    uartParams.readDataMode   = UART_DATA_BINARY;
    uartParams.writeDataMode  = UART_DATA_TEXT;
    uartParams.readEcho       = UART_ECHO_OFF;
    uartParams.baudRate       = 115200;
    uartParams.dataLength     = UART_LEN_8;
    uartParams.stopBits       = UART_STOP_ONE;
    uartParams.parityType     = UART_PAR_NONE;

    UartCbRegister[uartPort]  = callback;

    uarthandle[uartPort]      = UART_open(CC1310_LAUNCHXL_UART0, &uartParams);
}

void UartClose(UART_PORT uartPort)
{
    if (uartPort >= UART_MAX) {
        return;
    }

    UART_close(uarthandle[uartPort]);
}



void UartSendDatas(UART_PORT uartPort, void *buf, size_t count)
{
    if (uartPort >= UART_MAX) {
        return;
    }

    UART_write(uarthandle[uartPort], buf, count);
}
