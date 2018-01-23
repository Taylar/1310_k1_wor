//***********************************************************************************
// Copyright 2017, Zksiot Development Ltd.
// Created by Linjie, 2017.08.08
// MCU:	MSP430F5529
// OS: TI-RTOS
// Project:
// File name: lcd_drv.h
// Description: lcd process routine head.
//***********************************************************************************

#ifndef __ZKSIOT_LCD_DRV_H__
#define __ZKSIOT_LCD_DRV_H__

#ifdef SUPPORT_DISP_SCREEN

#ifdef EPD_GDE0213B1
#include "gde0213b1.h"
#endif

#ifdef EPD_GDE0213B1
#define LCD_START_COL				249
#define LCD_END_COL 				0
#define LCD_START_ROW				0
#define LCD_END_ROW 				15

#define LCD_MAX_PAGE				16
#define LCD_PAGE_SIZE				8
#define LCD_TOTAL_PAGE				16
#define LCD_TOTAL_COL				250
#endif

#ifdef LCD_ST7567A
#define LCD_START_COL				0
#define LCD_START_ROW				0

#define LCD_MAX_PAGE				8
#define LCD_PAGE_SIZE				8
#define LCD_TOTAL_PAGE				8
#define LCD_TOTAL_COL				128

#define LCD_CMD_PSA                 0xb0
#define LCD_CMD_LOWER_SCA           0x00
#define LCD_CMD_UPPER_SCA           0x10

#endif

#define LCD_FONT12X24_START_COL		4
#define LCD_FONT5X8_START_COL		4
#define LCD_FONT8X16_START_COL		0
#define LCD_FONT24X48_START_COL     0


extern void Lcd_set_font(uint8_t bFontCols, uint8_t bFontRows, uint8_t fgInverse);
extern void Lcd_init(void);
extern void Lcd_write_character(uint8_t xStart, uint8_t yStart, const uint8_t *pChar);
extern void Lcd_clear_area(uint8_t xStart, uint8_t yStart);
extern void Lcd_clear_page(uint8_t page);
extern void Lcd_clear_screen(void);
extern void Lcd_refresh(void);
extern void Lcd_poweron(void);
extern void Lcd_poweroff(void);

#endif  /* SUPPORT_DISP_SCREEN */

#endif	/* __ZKSIOT_LCD_DRV_H__ */

