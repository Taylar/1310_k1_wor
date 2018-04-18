//***********************************************************************************
// Copyright 2017, Zksiot Development Ltd.
// Created by Linjie, 2017.08.08
// MCU:	MSP430F5529
// OS: TI-RTOS
// Project:
// File name: display.h
// Description: Display process routine.
//***********************************************************************************
#ifndef __ZKSIOT_DISPLAY_H__
#define __ZKSIOT_DISPLAY_H__

#ifdef SUPPORT_DISP_SCREEN
#include "lcd_drv.h"


#define FONT_5X8_OFS                5
#define FONT_16X16_OFS              32
#define FONT_9X24_OFS               27
#define FONT_12X24_OFS              36
#define FONT_7X32_OFS               28
#define FONT_14X32_OFS              56
#define FONT_16X32_OFS              64

#ifdef SUPPORT_SFKJ_UI
#define FONT_23X24_OFS              69
#define FONT_13X8_OFS               13
#define FONT_8X8_OFS                8
#define FONT_6X16_OFS               12
#define FONT_3X16_OFS               6
#endif
typedef enum {
    FONT_5X8 = 0,
    FONT_8X16,
    FONT_12X24,
    FONT_16X32,
    FONT_24X48,
    FONT_50X100,
    FONT_80X160,
    FONT_96X192,
    FONT_192X96
} E_FONT_TYPE;

#define FONT_ASCII_START		32
#define FONT_ASCII_END			126
#define FONT_ASCII_OUTRANGE		0


typedef enum {
    ICON_5X8_DIGIT_0,
    ICON_5X8_DIGIT_1,
    ICON_5X8_DIGIT_2,
    ICON_5X8_DIGIT_3,
    ICON_5X8_DIGIT_4,
    ICON_5X8_DIGIT_5,
    ICON_5X8_DIGIT_6,
    ICON_5X8_DIGIT_7,
    ICON_5X8_DIGIT_8,
    ICON_5X8_DIGIT_9,
    ICON_5X8_SUB,
    ICON_5X8_COL,

    ICON_16X16_EXTERNAL,
    ICON_16X16_RECORD,
    ICON_16X16_ALARM,
    ICON_16X16_BLUETOOTH,
    ICON_16X16_UPLOAD,
    ICON_16X16_SIGNAL0,
    ICON_16X16_SIGNAL1,
    ICON_16X16_SIGNAL2,
    ICON_16X16_SIGNAL3,
    ICON_16X16_BATTERY0,
    ICON_16X16_BATTERY1,
    ICON_16X16_BATTERY2,
    ICON_16X16_BATTERY3,

    ICON_9X24_DIGIT_0,
    ICON_9X24_DIGIT_1,
    ICON_9X24_DIGIT_2,
    ICON_9X24_DIGIT_3,
    ICON_9X24_DIGIT_4,
    ICON_9X24_DIGIT_5,
    ICON_9X24_DIGIT_6,
    ICON_9X24_DIGIT_7,
    ICON_9X24_DIGIT_8,
    ICON_9X24_DIGIT_9,
    ICON_9X24_SUB,

    ICON_12X24_DIGIT_0,
    ICON_12X24_DIGIT_1,
    ICON_12X24_DIGIT_2,
    ICON_12X24_DIGIT_3,
    ICON_12X24_DIGIT_4,
    ICON_12X24_DIGIT_5,
    ICON_12X24_DIGIT_6,
    ICON_12X24_DIGIT_7,
    ICON_12X24_DIGIT_8,
    ICON_12X24_DIGIT_9,
    ICON_12X24_SUB,
    ICON_12X24_PERCENT,

    ICON_7X32_DOT,

    ICON_14X32_TC,

    ICON_16X32_DIGIT_0,
    ICON_16X32_DIGIT_1,
    ICON_16X32_DIGIT_2,
    ICON_16X32_DIGIT_3,
    ICON_16X32_DIGIT_4,
    ICON_16X32_DIGIT_5,
    ICON_16X32_DIGIT_6,
    ICON_16X32_DIGIT_7,
    ICON_16X32_DIGIT_8,
    ICON_16X32_DIGIT_9,
    ICON_16X32_SUB,
#ifdef  SUPPORT_SFKJ_UI
    ICON_13X8_SIGNAL_0,
    ICON_13X8_SIGNAL_1,
    ICON_13X8_SIGNAL_2,
    ICON_13X8_SIGNAL_3,
    ICON_8X8_PER_0,
    ICON_23X24_TMP_STAT_0,
    ICON_23X24_TMP_STAT_1,

    ICON_6X16_STAR_BAR_0,
    ICON_6X16_STAR_BAR_1,
    ICON_6X16_STAR_BAR_2,
    ICON_6X16_STAR_BAR_3,
    ICON_6X16_STAR_BAR_4,
    ICON_6X16_STAR_BAR_5,
    ICON_6X16_STAR_BAR_6,
    ICON_6X16_STAR_BAR_7,
    ICON_6X16_STAR_BAR_8,
    ICON_6X16_STAR_BAR_9,
	ICON_6X16_STAR_BAR_A,
    ICON_6X16_STAR_BAR_H,
    ICON_6X16_STAR_BAR_L,
    ICON_6X16_STAR_BAR_T,
    ICON_6X16_STAR_BAR_P,
    ICON_6X16_STAR_BAR_DC,

    ICON_3X16_STAR_BAR_MHAO,
    ICON_3X16_STAR_BAR_SUB,
    ICON_3X16_STAR_BAR_DOT,
#endif
    ICON_MAX
} DISP_ICON;

typedef enum {
    DCON_POWERON    = 0,
    DCON_SENSOR,
    DCON_MAX
} DISP_CONTENT;


//extern void Disp_icon(uint8_t icon, uint8_t lite);
extern void Disp_msg(uint8_t x, uint8_t y, const uint8_t *pChar, E_FONT_TYPE eFont);
extern void Disp_number(uint8_t x, uint8_t y, uint16_t value, uint8_t length, E_FONT_TYPE eFont);
extern void Disp_clear_all(void);
extern void Disp_refresh(void);
extern void Disp_sensor_switch(void);
extern void Disp_info_switch(void);
extern void Disp_info_close(void);
extern void Disp_picture(uint8_t col, uint8_t row, uint8_t wide, uint8_t high, const uint8_t *picture);
extern void Disp_proc(void);
extern void Disp_init(void);
extern bool Disp_poweron(void);
extern void Disp_poweroff(void);

#endif  /* SUPPORT_DISP_SCREEN */

#endif	/* __ZKSIOT_DISPLAY_H__ */


