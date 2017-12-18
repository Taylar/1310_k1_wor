//***********************************************************************************
// Copyright 2017, Zksiot Development Ltd.
// Created by Linjie, 2017.08.08
// MCU:	MSP430F5529
// OS: TI-RTOS
// Project:
// File name: uart_drv.h
// Description: uart process routine head.
//***********************************************************************************

#ifndef __ZKSIOT_UART_DRV_H__
#define __ZKSIOT_UART_DRV_H__


/*!
 *  @def    CC1310_LAUNCHXL_UARTName
 *  @brief  Enum of UARTs
 */
typedef enum CC1310_LAUNCHXL_UARTName {
    CC1310_LAUNCHXL_UART0 = 0,

    CC1310_LAUNCHXL_UARTCOUNT
} CC1310_LAUNCHXL_UARTName;
/*****************************************************/


#define UART_RX_BUFF_SIZE           128


typedef enum {
    UART_0,
    UART_MAX
} UART_PORT;

typedef struct {
    uint8_t buff[UART_RX_BUFF_SIZE];
    uint8_t length;
} UartRxData_t;


void UartHwInit(UART_PORT uartPort, uint32_t baudrate, UART_Callback callback);

void UartClose(UART_PORT uartPort);

void UartSendDatas(UART_PORT uartPort, void *buf, size_t count);

#endif	/* __ZKSIOT_UART_DRV_H__ */

