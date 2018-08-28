//***********************************************************************************
// Copyright 2017, Zksiot Development Ltd.
// Created by Linjie, 2017.08.08
// MCU:	MSP430F5529
// OS: TI-RTOS
// Project:
// File name: lcd_drv.c
// Description: lcd process routine.
//***********************************************************************************
#include "../general.h"

#ifdef SUPPORT_DISP_SCREEN
#include "lcd_drv.h"

#define LCD_POWER_PIN               IOID_20
#define LCD_DATA_PIN                IOID_27
#define LCD_CLK_PIN                 IOID_8
#define LCD_CS_PIN                  IOID_21
#define LCD_DATA_C_PIN              IOID_29

#define LCD_RESET_PIN               IOID_28


const PIN_Config lcdPinTable[] = {
    LCD_POWER_PIN   | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW  | PIN_PUSHPULL | PIN_DRVSTR_MAX,      /*          */
    LCD_DATA_PIN    | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW  | PIN_PUSHPULL | PIN_DRVSTR_MAX,    /*          */
    LCD_CLK_PIN     | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH  | PIN_PUSHPULL | PIN_DRVSTR_MAX,       /* LED initially off          */
    LCD_CS_PIN      | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH  | PIN_PUSHPULL | PIN_DRVSTR_MAX,    /*          */
    LCD_DATA_C_PIN  | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW  | PIN_PUSHPULL | PIN_DRVSTR_MAX,       /* LED initially off          */
    LCD_RESET_PIN   | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH  | PIN_PUSHPULL | PIN_DRVSTR_MAX,       /* LED initially off          */
    PIN_TERMINATE
};





#define Lcd_power_ctrl(on)          (!on) ? (PIN_setOutputValue(lcdPinHandle, LCD_POWER_PIN, 1)) : (PIN_setOutputValue(lcdPinHandle, LCD_POWER_PIN, 0))

#ifdef LCD_ST7567A
//Use software spi
#define Lcd_spiDatCtrl(on)          (on) ? (PIN_setOutputValue(lcdPinHandle, LCD_DATA_PIN, 1)) : (PIN_setOutputValue(lcdPinHandle, LCD_DATA_PIN, 0))
#define Lcd_spiClkCtrl(on)          (on) ? (PIN_setOutputValue(lcdPinHandle, LCD_CLK_PIN, 1)) : (PIN_setOutputValue(lcdPinHandle, LCD_CLK_PIN, 0))
#define Lcd_spiCsCtrl(on)           (on) ? (PIN_setOutputValue(lcdPinHandle, LCD_CS_PIN, 1)) : (PIN_setOutputValue(lcdPinHandle, LCD_CS_PIN, 0))
#define Lcd_spiDcCtrl(on)           (on) ? (PIN_setOutputValue(lcdPinHandle, LCD_DATA_C_PIN, 1)) : (PIN_setOutputValue(lcdPinHandle, LCD_DATA_C_PIN, 0))

#define Lcd_reset_ctrl(on)          (!on) ? (PIN_setOutputValue(lcdPinHandle, LCD_RESET_PIN, 1)) : (PIN_setOutputValue(lcdPinHandle, LCD_RESET_PIN, 0))


static PIN_State   lcdPinState;
static PIN_Handle  lcdPinHandle = NULL;

#endif


static uint8_t fgLcdFontInverse;
static uint8_t bLcdFontPages;       // Font Pages = Rows / 8
static uint8_t bLcdFontCols;        // Font Cols


#ifdef LCD_ST7567A

//***********************************************************************************
//
// LCD enable the power.
//
//***********************************************************************************
void Lcd_io_init(void)
{
    if(lcdPinHandle == NULL)
        lcdPinHandle = PIN_open(&lcdPinState, lcdPinTable);
}


//***********************************************************************************
//
// LCD disable the power the all the pin output low.
//
//***********************************************************************************
void Lcd_io_input(void)
{
    Lcd_spiDatCtrl(0);
    Lcd_spiClkCtrl(0);
    Lcd_spiDcCtrl(0);
    Lcd_spiCsCtrl(0);
}

//***********************************************************************************
//
// LCD screen spi send byte.
//
//***********************************************************************************
static void Lcd_spiSendByte(uint8_t value)
{
    uint8_t i;

    for (i = 0; i < 8; i++) {
        Lcd_spiClkCtrl(0);
        if (value & 0x80) {
            Lcd_spiDatCtrl(1);
        } else {
            Lcd_spiDatCtrl(0);
        }
        Lcd_spiClkCtrl(1);
        value <<= 1;
    }
}

//***********************************************************************************
//
// LCD screen send cmd.
//
//***********************************************************************************
void Lcd_send_cmd(uint8_t cmd)
{
    UInt key;

    key = Hwi_disable();
    Lcd_spiDcCtrl(0);
    Lcd_spiCsCtrl(0);
    Lcd_spiSendByte(cmd);
    Lcd_spiCsCtrl(1);
    Hwi_restore(key);
}

//***********************************************************************************
//
// LCD screen send data.
//
//***********************************************************************************
void Lcd_send_data(uint8_t value)
{
    UInt key;

    key = Hwi_disable();
    Lcd_spiDcCtrl(1);
    Lcd_spiCsCtrl(0);
    Lcd_spiSendByte(value);
    Lcd_spiCsCtrl(1);
    Hwi_restore(key);
}

//***********************************************************************************
//
// LCD screen reset.
//
//***********************************************************************************
static void Lcd_reset(void)
{
    Lcd_reset_ctrl(1);
    Task_sleep(1 * CLOCK_UNIT_MS);
    Lcd_reset_ctrl(0);
    Task_sleep(1 * CLOCK_UNIT_MS);
}

#endif

//***********************************************************************************
//
// LCD set font parameters, Font Cols, Font Pages = Rows / 8, display inverse.
//
//***********************************************************************************
void Lcd_set_font(uint8_t bFontCols, uint8_t bFontRows, uint8_t fgInverse)
{
	bLcdFontPages = (bFontRows + 7) / 8;
	bLcdFontCols = bFontCols;
	fgLcdFontInverse = fgInverse;
}

//***********************************************************************************
//
// LCD init.
//
//***********************************************************************************
void Lcd_init(void)
{
    Lcd_io_init();
    Lcd_power_ctrl(1);

#ifdef LCD_ST7567A
    Task_sleep(1 * CLOCK_UNIT_MS);
    Lcd_reset();

    Lcd_send_cmd(0x40);
    Lcd_send_cmd(0xa2);
//    Lcd_send_cmd(0xa6);
    Lcd_send_cmd(0xa0);
    Lcd_send_cmd(0xc8);
//    Lcd_send_cmd(0x2c);
//    Delay_ms(200);
//    Lcd_send_cmd(0x2e);
//    Delay_ms(100);
    Lcd_send_cmd(0x2f);
//    Delay_ms(100);
    Lcd_send_cmd(0x25);
    Lcd_send_cmd(0x81);
    Lcd_send_cmd(0x17);

    Lcd_send_cmd(0xf8);
    Lcd_send_cmd(0x00);

	Lcd_clear_screen();
	Lcd_refresh();

    Lcd_send_cmd(0xaf);
#endif

#ifdef EPD_GDE0213B1
    EPD_init(1);
    EPD_clearScreen();
    EPD_update(1);
/*    Delay_ms(4000);
    EPD_init(0);
    EPD_clearScreen();
    EPD_update(0);
    EPD_clearScreen();
    Delay_ms(300);*/
#endif

}

//***********************************************************************************
//
// LCD write character.
//
//***********************************************************************************
void Lcd_write_character(uint8_t xStart, uint8_t yStart, const uint8_t *pChar)
{
#ifdef LCD_ST7567A
    uint8_t bLowCol, bHiCol;
    uint8_t page, col;

    // Offset from left edge of display.
    xStart += LCD_START_COL;

    // Add the command to page address.
    bLowCol = xStart & 0x0f;
    bHiCol = (xStart >> 4) & 0x0f;

    for (page = 0; page < bLcdFontPages; page++) {
        Lcd_send_cmd(LCD_CMD_PSA + yStart);			// set the page start address
        Lcd_send_cmd(LCD_CMD_LOWER_SCA + bLowCol);		// set the lower start column address
        Lcd_send_cmd(LCD_CMD_UPPER_SCA + bHiCol);		// set the upper start column address

        for (col = 0; col < bLcdFontCols; col++) {
            // check inverse flag bit.
            if (fgLcdFontInverse)
                Lcd_send_data(*pChar ^ 0xff);
            else
                Lcd_send_data(*pChar);
            pChar++;
        }
        yStart++;
    }
#endif

#ifdef EPD_GDE0213B1
    EPD_writeCharacter(yStart, yStart + bLcdFontPages, xStart, xStart + bLcdFontCols, pChar);
#endif
}

//***********************************************************************************
//
// LCD clear area screen.
//
//***********************************************************************************
void Lcd_clear_area(uint8_t xStart, uint8_t yStart)
{
#ifdef LCD_ST7567A
    uint8_t bLowCol, bHiCol;
    uint8_t page, col;

    // Offset from left edge of display.
    xStart += LCD_START_COL;

    // Add the command to page address.
    bLowCol = xStart & 0x0f;
    bHiCol = (xStart >> 4) & 0x0f;

    for (page = 0; page < bLcdFontPages; page++) {
        Lcd_send_cmd(LCD_CMD_PSA + yStart);			// set the page start address
        Lcd_send_cmd(LCD_CMD_LOWER_SCA + bLowCol);		// set the lower start column address
        Lcd_send_cmd(LCD_CMD_UPPER_SCA + bHiCol);		// set the upper start column address

        for (col = 0; col < bLcdFontCols; col++) {
            // check inverse flag bit.
            if (fgLcdFontInverse)
                Lcd_send_data(0xff);
            else
                Lcd_send_data(0);
        }
        yStart++;
    }
#endif

#ifdef EPD_GDE0213B1
    EPD_clearArea(yStart, yStart + bLcdFontPages, xStart, xStart + bLcdFontCols);
#endif
}

//***********************************************************************************
//
// LCD clear page.
//
//***********************************************************************************
void Lcd_clear_page(uint8_t page)
{
	uint8_t col;

	Lcd_send_cmd(LCD_CMD_PSA + page);					// set the page start address
	Lcd_send_cmd(LCD_CMD_LOWER_SCA + LCD_START_COL);	// set the lower start column address
	Lcd_send_cmd(LCD_CMD_UPPER_SCA);					// set the upper start column address
	for(col = 0; col < LCD_TOTAL_COL; col++)
		Lcd_send_data(0x00);
}

//***********************************************************************************
//
// LCD clear full screen.
//
//***********************************************************************************
void Lcd_clear_screen(void)
{
#ifdef LCD_ST7567A
	uint8_t page, col;

	for(page = 0; page < LCD_MAX_PAGE; page++) {
		Lcd_send_cmd(LCD_CMD_PSA + page);					// set the page start address
		Lcd_send_cmd(LCD_CMD_LOWER_SCA + LCD_START_COL);	// set the lower start column address
		Lcd_send_cmd(LCD_CMD_UPPER_SCA);					// set the upper start column address
		for(col = 0; col < LCD_TOTAL_COL; col++)
			Lcd_send_data(0x00);
	}
#endif

#ifdef EPD_GDE0213B1
    EPD_clearScreen();
#endif
}

//***********************************************************************************
//
// LCD refresh.
//
//***********************************************************************************
void Lcd_refresh(void)
{
#ifdef EPD_GDE0213B1
    EPD_update(0);
#endif
}

//***********************************************************************************
//
// LCD power on.
//
//***********************************************************************************
void Lcd_poweron(void)
{
    Lcd_io_init();
    Lcd_power_ctrl(1);
#ifdef EPD_GDE0213B1
    EPD_init(0);
    EPD_clearScreen();
    EPD_update(0);
    EPD_clearScreen();
#endif
}

//***********************************************************************************
//
// LCD power off.
//
//***********************************************************************************
void Lcd_poweroff(void)
{
    Lcd_reset_ctrl(1);
    Lcd_io_input();
    Lcd_power_ctrl(0);
}

#endif  /* SUPPORT_DISP_SCREEN */

