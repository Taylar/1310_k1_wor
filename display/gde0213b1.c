//***********************************************************************************
// Copyright 2017, Zksiot Development Ltd.
// Created by Linjie, 2017.05.17
// MCU:	MSP430F55xx
// Project:
// File name: gde0213b1.c
// Description: gde0213b1 process routine.
//***********************************************************************************
#include "../general.h"

#ifdef EPD_GDE0213B1
#include "gde0213b1.h"

const uint8_t LUTDefault_full[] = {
    0x32,	// command
    0x22, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x11, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1E, 0x1E, 0x1E, 0x1E,
    0x1E, 0x1E, 0x1E, 0x1E, 0x01, 0x00, 0x00, 0x00, 0x00
};

const uint8_t LUTDefault_part[] = {
    0x32,	// command
    0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


//***********************************************************************************
//
// EPD screen spi send byte.
//
//***********************************************************************************
void EPD_waitBusy(void)
{
    uint64_t timerout;

    timerout = Timer_getSysTickms() + 1000;
    while (EPD_isBusy()) {
        if (timerout <= Timer_getSysTickms())
            break;
    }
}

//***********************************************************************************
//
// EPD screen spi send byte.
//
//***********************************************************************************
void EPD_spiSendByte(uint8_t value)
{
    uint8_t i;

    for (i = 0; i < 8; i++) {
        EPD_spiClkCtrl(0);
        if (value & 0x80) {
            EPD_spiSimoCtrl(1);
        } else {
            EPD_spiSimoCtrl(0);
        }
        EPD_spiClkCtrl(1);
        value <<= 1;
    }
}

//***********************************************************************************
//
// EPD screen send cmd.
//
//***********************************************************************************
void EPD_sendCmd(uint8_t cmd)
{
    EPD_spiDcCtrl(0);
    EPD_spiSteCtrl(0);
    EPD_spiSendByte(cmd);
    EPD_spiSteCtrl(1);
}

//***********************************************************************************
//
// EPD screen send data.
//
//***********************************************************************************
void EPD_sendData(uint8_t value)
{
    EPD_spiDcCtrl(1);
    EPD_spiSteCtrl(0);
    EPD_spiSendByte(value);
    EPD_spiSteCtrl(1);
}

//***********************************************************************************
//
// EPD screen send cmd and data.
//
//***********************************************************************************
void EPD_sendCmdData(uint8_t *buff, uint8_t length)
{
    uint8_t i;

    EPD_spiSteCtrl(0);
    EPD_spiDcCtrl(0);
    EPD_spiSendByte(buff[0]);
    EPD_spiDcCtrl(1);
    for (i = 1; i < length; i++) {
        EPD_spiSendByte(buff[i]);
    }
    EPD_spiSteCtrl(1);
}

//***********************************************************************************
//
// EPD screen write data to ram.
//      isSingleData:  1 means only write first data to ram
//                     0 means write buff data to ram
//
//***********************************************************************************
void EPD_writeRam(uint8_t xStart, uint8_t yStart, uint8_t *pData, uint8_t isSingleData)
{
    uint8_t i, j;

    EPD_waitBusy();

    EPD_spiSteCtrl(0);
    EPD_spiDcCtrl(0);
    EPD_spiSendByte(0x24);
    EPD_spiDcCtrl(1);
	for (i = 0; i < yStart; i++) {
		for (j = 0; j < xStart; j++) {
			// check inverse flag bit.
            EPD_spiSendByte(*pData ^ 0xff);
            if (isSingleData == 0)
    			pData++;
		}
	}
    EPD_spiSteCtrl(1);
}

//***********************************************************************************
//
// EPD screen write LUT parameter.
//
//***********************************************************************************
void EPD_writeLut(uint8_t isFull)
{
    if (isFull) {
        EPD_sendCmdData((uint8_t *)LUTDefault_full, sizeof(LUTDefault_full));
    } else {
        EPD_sendCmdData((uint8_t *)LUTDefault_part, sizeof(LUTDefault_part));
    }
}

//***********************************************************************************
//
// EPD set ram area.
//
//***********************************************************************************
void EPD_setRamArea(uint8_t xStart, uint8_t xEnd, uint8_t yStart, uint8_t yEnd)
{
    uint8_t buff[3];

    //Set RAM X - address Start / End position 
    buff[0] = 0x44;
    buff[1] = xStart;
    buff[2] = xEnd;
    EPD_sendCmdData(buff, 3);
    //Set RAM Y - address Start / End position 
    buff[0] = 0x45;
    buff[1] = yStart;
    buff[2] = yEnd;
    EPD_sendCmdData(buff, 3);
}

//***********************************************************************************
//
// EPD set ram area.
//
//***********************************************************************************
void EPD_setRamPointer(uint8_t xStart, uint8_t yStart)
{
    uint8_t buff[2];

    //Set RAM X - address position 
    buff[0] = 0x4e;
    buff[1] = xStart;
    EPD_sendCmdData(buff, 2);
    //Set RAM Y - address position 
    buff[0] = 0x4f;
    buff[1] = yStart;
    EPD_sendCmdData(buff, 2);
}

//***********************************************************************************
//
// EPD screen update.
//
//***********************************************************************************
void EPD_update(uint8_t isFull)
{
    uint8_t buff[2];

    buff[0] = 0x22;
    if (isFull) {
        buff[1] = 0xc7;
    } else {
        buff[1] = 0x04;
    }
    EPD_sendCmdData(buff, 2);
    EPD_sendCmd(0x20);
    EPD_sendCmd(0xff);
}

//***********************************************************************************
//
// EPD screen update.
//
//***********************************************************************************
void EPD_powerCtrl(uint8_t poweron)
{
    uint8_t buff[2];

    buff[0] = 0x22;
    if (poweron) {
        buff[1] = 0xc0;
    } else {
        buff[1] = 0xc3;
    }
    EPD_sendCmdData(buff, 2);
    EPD_sendCmd(0x20);
}

//***********************************************************************************
//
// EPD screen display character.
//
//***********************************************************************************
void EPD_writeCharacter(uint8_t xStart, uint8_t xEnd, uint8_t yStart, uint8_t yEnd, const uint8_t *pChar)
{
    yStart = 0xf9 - yStart;
    yEnd = 0xf9 - yEnd;

    EPD_setRamArea(xStart, xEnd - 1, yStart, yEnd + 1);
    EPD_setRamPointer(xStart, yStart);

    EPD_writeRam(xEnd - xStart, yStart - yEnd, (uint8_t *)pChar, 0);
}

//***********************************************************************************
//
// EPD screen clear area screen.
//
//***********************************************************************************
void EPD_clearArea(uint8_t xStart, uint8_t xEnd, uint8_t yStart, uint8_t yEnd)
{
	uint8_t data;

    yStart = 0xf9 - yStart;
    yEnd = 0xf9 - yEnd;

    EPD_setRamArea(xStart, xEnd - 1, yStart, yEnd + 1);
    EPD_setRamPointer(xStart, yStart);

    data = 0x00;
    EPD_writeRam(xEnd - xStart, yStart - yEnd, &data, 1);
}

//***********************************************************************************
//
// EPD screen clear full screen.
//
//***********************************************************************************
void EPD_clearScreen(void)
{
	uint8_t data;

    EPD_setRamArea(LCD_START_ROW, LCD_END_ROW, LCD_START_COL, LCD_END_COL);
    EPD_setRamPointer(LCD_START_ROW, LCD_START_COL);

    data = 0x00;
    EPD_writeRam(LCD_TOTAL_PAGE, LCD_TOTAL_COL, &data, 1);
}

//***********************************************************************************
//
// EPD screen reset.
//
//***********************************************************************************
void EPD_reset(void)
{
    EPD_resetCtrl(1);
    Delay_ms(100);
    EPD_resetCtrl(0);
    Delay_ms(100);
}

//***********************************************************************************
//
// EPD screen init.
//
//***********************************************************************************
void EPD_init(uint8_t isFull)
{
    uint8_t buff[3];

    EPD_ioInit();

    EPD_reset();

    //Set the number of gate
    buff[0] = 0x01;
    buff[1] = 0xf9;
    buff[2] = 0x00;
    EPD_sendCmdData(buff, 3);
    //Set dummy line period
    buff[0] = 0x3a;
    buff[1] = 0x06;
    EPD_sendCmdData(buff, 2);
    //Set Gate line width
    buff[0] = 0x3b;
    buff[1] = 0x0b;
    EPD_sendCmdData(buff, 2);
    //Define data entry sequence
    buff[0] = 0x11;
    buff[1] = 0x01;
    EPD_sendCmdData(buff, 2);
    //Set RAM X/Y - address Start / End position 
    EPD_setRamArea(0x00, 0x0f, 0xf9, 0x00);
    //Set RAM X/Y - address position
    EPD_setRamPointer(0x00, 0xf9);
    //Write VCOM register
    buff[0] = 0x2c;
    buff[1] = 0x4b;
    EPD_sendCmdData(buff, 2);

    EPD_writeLut(isFull);
    EPD_powerCtrl(1);
}

#endif  /* EPD_GDE0213B1 */

