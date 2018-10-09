//***********************************************************************************
// Copyright 2017, Zksiot Development Ltd.
// Created by Linjie, 2017.08.08
// MCU:	MSP430F5529
// OS: TI-RTOS
// Project:
// File name: display.c
// Description: Display API function process routine.
//***********************************************************************************
#include "../general.h"
#ifndef SUPPORT_SFKJ_UI
#ifdef SUPPORT_DISP_SCREEN
// const uint8_t font5x8[]= {
//     #include "font\font5x8.txt"
// };

const uint8_t font8x16[]= {
    #include "font\font8x16.txt"
};

const uint8_t font12x24[]= {
    #include "font\font12x24.txt"
};

const uint8_t icon5x8[]= {
    #include "font\icon5x8.txt"
};

const uint8_t icon16x16[]= {
    #include "font\icon16x16.txt"
};

const uint8_t icon9x24[]= {
    #include "font\icon9x24.txt"
};

const uint8_t icon12x24[]= {
    #include "font\icon12x24.txt"
};

const uint8_t icon7x32[]= {
    #include "font\icon7x32.txt"
};

const uint8_t icon14x32[]= {
    #include "font\icon14x32.txt"
};

const uint8_t icon16x32[]= {
    #include "font\icon16x32.txt"
};
//Calendar icon gap
#define CAICON_GAP              1
//Calendar icon digit wide and high
#define CAICON_W                5
#define CAICON_H                8
//Calendar icon sub index
#define CAICON_SUB              ICON_5X8_SUB
//Calendar icon digit index
#define CAICON_DIGIT            ICON_5X8_DIGIT_0
//Calendar icon column index
#define CAICON_COL              ICON_5X8_COL

//Temperature icon gap
#define TPICON_GAP              0
//Temperature icon digit wide and high
#define TPICON_W                16
#define TPICON_H                32
//Temperature icon dot wide and high
#define TPICON_DOT_W            7
#define TPICON_DOT_H            32
//Temperature icon degree Celsius wide and high
#define TPICON_DC_W             14
#define TPICON_DC_H             32
//Temperature icon sub index
#define TPICON_SUB              ICON_16X32_SUB
//Temperature icon digit index
#define TPICON_DIGIT            ICON_16X32_DIGIT_0
//Temperature icon dot index
#define TPICON_DOT              ICON_7X32_DOT
//Temperature icon degree Celsius index
#define TPICON_DC               ICON_14X32_TC

//Humidty icon gap
#define HUICON_GAP              0
//Humidty icon digit wide and high
#define HUICON_W                9
#define HUICON_H                24
//Humidty icon percent wide and high
#define HUICON_PCT_W            12
#define HUICON_PCT_H            24
//Humidty icon sub index
#define HUICON_SUB              ICON_9X24_SUB
//Humidty icon digit index
#define HUICON_DIGIT            ICON_9X24_DIGIT_0
//Humidty icon percent index
#define HUICON_PCT              ICON_12X24_PERCENT

//Status bar icon wide and high
#define SBICON_W                16
#define SBICON_H                16

#define SIGNAL_W                13
#define SIGNAL_H                8
#define STAR_PER_W              8
#define STAR_PER_H              8

#define TEMP0_COL_POS           0
#define TEMP0_ROW_POS           2
#define TEMP1_COL_POS           20
#define LUX_COL_POS             2
#define TEMP1_ROW_POS           2
#define HUMI_COL_POS            89
#define HUMI_ROW_POS            3
#define STATUSB_COL_POS         0
#define STATUSB_ROW_POS         6

//icon start col and row pos
#define CALD_COL_POS            14
#define CALD_ROW_POS            0

#define DISP_INTERVAL_TIME      2000

typedef struct {
    //Display init flag.
    uint8_t init;
    //Display refresh flag.
    uint8_t refresh;
    //Display sensor module number index.
    uint8_t sensorIndex;
    //Display info index.
    uint8_t infoIndex;
} DispObject_t;
typedef struct {
    uint8_t     flag;
    int32_t    valueHH;
    int32_t     valueHL;
} TempHistory_t;
DispObject_t rDispObject;
TempHistory_t rTempHistory;

//***********************************************************************************
//
// Display a character with font type.
//
//***********************************************************************************
static void Disp_character(uint8_t bColStart, uint8_t bPageStart, uint8_t bChar, uint8_t bFont)
{
    uint8_t fgInv;

    if ((bChar >= FONT_ASCII_START) && (bChar <= FONT_ASCII_END)) {
        bChar -= FONT_ASCII_START;
    } else {
        //out of ASCII range
        bChar = FONT_ASCII_OUTRANGE;
    }

    if (bFont & 0x80)
        fgInv = 1;
    else
        fgInv = 0;

    switch (bFont & 0x7f) {
//         case FONT_5X8:
//             bColStart *= 6;
//             bColStart += LCD_FONT5X8_START_COL;
//             Lcd_set_font(5, 8, fgInv);
// //            Lcd_write_character(bColStart, bPageStart, &font5x8[bChar * 5]);
//             // We use font 5x8, there are one space between char, so need clear the space.
//             Lcd_set_font(1, 8, fgInv);
// //            Lcd_write_character(bColStart + 5, bPageStart, &font5x8[0]);
        // break;

        case FONT_8X16:
            bColStart *= 8;
            bColStart += LCD_FONT8X16_START_COL;
//            bPageStart *= 2;
            Lcd_set_font(8, 16, fgInv);
            Lcd_write_character(bColStart, bPageStart, &font8x16[bChar * 16]);
        break;

        // case FONT_12X24:
        //     bColStart *= 12;
        //     bColStart += LCD_FONT12X24_START_COL;
        //     Lcd_set_font(12, 24, fgInv);
        //    // bPageStart = bPageStart * 3 + 2;
        //     Lcd_write_character(bColStart, bPageStart, &font12x24[bChar * 36]);
        // break;

       //  case FONT_16X32:
       //      bColStart *= 16;
       //      Lcd_set_font(16, 32, fgInv);
       //     Lcd_write_character(bColStart, bPageStart, Font16X32Tab[bChar]);
       // break;

       //  case FONT_24X48:
       //      bColStart *= 24;
       //      bColStart += LCD_FONT24X48_START_COL;
       //      Lcd_set_font(24, 48, fgInv);
       //      bPageStart += 5;
       //     Lcd_write_character(bColStart, bPageStart, &font24x48[bChar * 144]);
       // break;
    }
    rDispObject.refresh = 1;
}

//***********************************************************************************
//
// Display icon.
//
//***********************************************************************************
void Disp_icon(uint8_t col, uint8_t row, uint8_t icon, uint8_t light)
{
    if (rDispObject.init == 0)
        return;

    switch (icon) {
        case ICON_5X8_DIGIT_0 ... ICON_5X8_COL:
            if (light) {
                Lcd_write_character(col, row, &icon5x8[(icon - ICON_5X8_DIGIT_0) * FONT_5X8_OFS]);
            } else {
                Lcd_clear_area(col, row);
            }
            break;

        case ICON_16X16_EXTERNAL ... ICON_16X16_FLIGHT://ICON_16X16_BATTERY3:
            if (light) {
                Lcd_write_character(col, row, &icon16x16[(icon - ICON_16X16_EXTERNAL) * FONT_16X16_OFS]);
            } else {
                Lcd_clear_area(col, row);
            }
            break;

        case ICON_9X24_DIGIT_0 ... ICON_9X24_SUB:
            if (light) {
                Lcd_write_character(col, row, &icon9x24[(icon - ICON_9X24_DIGIT_0) * FONT_9X24_OFS]);
            } else {
                Lcd_clear_area(col, row);
            }
            break;

        case ICON_12X24_DIGIT_0 ... ICON_12X24_PERCENT:
            if (light) {
                Lcd_write_character(col, row, &icon12x24[(icon - ICON_12X24_DIGIT_0) * FONT_12X24_OFS]);
            } else {
                Lcd_clear_area(col, row);
            }
            break;

        case ICON_7X32_DOT:
            if (light) {
                Lcd_write_character(col, row, &icon7x32[(icon - ICON_7X32_DOT) * FONT_7X32_OFS]);
            } else {
                Lcd_clear_area(col, row);
            }
            break;

        case ICON_14X32_TC:
            if (light) {
                Lcd_write_character(col, row, &icon14x32[(icon - ICON_14X32_TC) * FONT_14X32_OFS]);
            } else {
                Lcd_clear_area(col, row);
            }
            break;

        case ICON_16X32_DIGIT_0 ... ICON_16X32_SUB:
            if (light) {
                Lcd_write_character(col, row, &icon16x32[(icon - ICON_16X32_DIGIT_0) * FONT_16X32_OFS]);
            } else {
                Lcd_clear_area(col, row);
            }
            break;
    }

    rDispObject.refresh = 1;
}

//***********************************************************************************
//
// Display string message.
//
//***********************************************************************************
void Disp_msg(uint8_t col, uint8_t row, const uint8_t *pChar, E_FONT_TYPE eFont)
{
    if (rDispObject.init == 0)
        return;

    while (*pChar) {
        Disp_character(col, row, *pChar, eFont);
        col++;
        pChar++;
    }
}

//***********************************************************************************
//
// Display digit number.
//
//***********************************************************************************
void Disp_number(uint8_t col, uint8_t row, uint16_t value, uint8_t length, E_FONT_TYPE eFont)
{    
    if (rDispObject.init == 0)
        return;    

    switch (length) {
        case 5:
            Disp_character(col + 4, row, value % 10 + 48, eFont);
            value /= 10;
            /* no break */
        case 4:
            Disp_character(col + 3, row, value % 10 + 48, eFont);
            value /= 10;
            /* no break */
        case 3:
            Disp_character(col + 2, row, value % 10 + 48, eFont);
            value /= 10;
            /* no break */
        case 2:
            Disp_character(col + 1, row, value % 10 + 48, eFont);
            value /= 10;
            /* no break */
        case 1:
            Disp_character(col, row, value % 10 + 48, eFont);
            break;
    }
}

//***********************************************************************************
//
// Display clear screen.
//
//***********************************************************************************
void Disp_clear_all(void)
{    
    if (rDispObject.init == 0)
        return;
    
    Lcd_clear_screen();
    rDispObject.refresh = 1;
}

//***********************************************************************************
//
// Display refresh.
//
//***********************************************************************************
void Disp_refresh(void)
{    
    if (rDispObject.init == 0)
        return;    

    if (rDispObject.refresh) {
        Lcd_refresh();
        rDispObject.refresh = 0;
    }
}

//***********************************************************************************
//
// Display calendar.
//
//***********************************************************************************
static void Disp_calendar(void)
{
    Calendar calendar;
    uint8_t value;
    uint8_t col = CALD_COL_POS, row = CALD_ROW_POS;

    calendar = Rtc_get_calendar();
    calendar.Year       = TransHexToBcd((uint8_t)(calendar.Year - 2000)) + 0x2000;
    calendar.Month      = TransHexToBcd((uint8_t)(calendar.Month));
    calendar.DayOfMonth = TransHexToBcd((uint8_t)(calendar.DayOfMonth));
    calendar.Hours      = TransHexToBcd((uint8_t)(calendar.Hours));
    calendar.Minutes    = TransHexToBcd((uint8_t)(calendar.Minutes));
    calendar.Seconds    = TransHexToBcd((uint8_t)(calendar.Seconds));

	Lcd_set_font(CAICON_W, CAICON_H, 0);
    value = (calendar.Year >> 12) & 0x000f;
    Disp_icon(col, row, CAICON_DIGIT + value, 1);
    value = (calendar.Year >> 8) & 0x000f;
    col += CAICON_W + CAICON_GAP;
    Disp_icon(col, row, CAICON_DIGIT + value, 1);
    value = (calendar.Year >> 4) & 0x000f;
    col += CAICON_W + CAICON_GAP;
    Disp_icon(col, row, CAICON_DIGIT + value, 1);
    value = calendar.Year & 0x000f;
    col += CAICON_W + CAICON_GAP;
    Disp_icon(col, row, CAICON_DIGIT + value, 1);

    col += CAICON_W + CAICON_GAP;
    Disp_icon(col, row, ICON_5X8_SUB, 1);

    value = (calendar.Month >> 4) & 0x0f;
    col += CAICON_W + CAICON_GAP;
    Disp_icon(col, row, CAICON_DIGIT + value, 1);
    value = calendar.Month & 0x0f;
    col += CAICON_W + CAICON_GAP;
    Disp_icon(col, row, CAICON_DIGIT + value, 1);

    col += CAICON_W + CAICON_GAP;
    Disp_icon(col, row, CAICON_SUB, 1);

    value = (calendar.DayOfMonth >> 4) & 0x0f;
    col += CAICON_W + CAICON_GAP;
    Disp_icon(col, row, CAICON_DIGIT + value, 1);
    value = calendar.DayOfMonth & 0x0f;
    col += CAICON_W + CAICON_GAP;
    Disp_icon(col, row, CAICON_DIGIT + value, 1);

    value = (calendar.Hours >> 4) & 0x0f;
    col += 10;
    col += CAICON_W + CAICON_GAP;
    Disp_icon(col, row, CAICON_DIGIT + value, 1);
    value = calendar.Hours & 0x0f;
    col += CAICON_W + CAICON_GAP;
    Disp_icon(col, row, CAICON_DIGIT + value, 1);

    col += CAICON_W + CAICON_GAP;
    Disp_icon(col, row, CAICON_COL, 1);

    value = (calendar.Minutes >> 4) & 0x0f;
    col += CAICON_W + CAICON_GAP;
    Disp_icon(col, row, CAICON_DIGIT + value, 1);
    value = calendar.Minutes & 0x0f;
    col += CAICON_W + CAICON_GAP;
    Disp_icon(col, row, CAICON_DIGIT + value, 1);
}

#ifdef SUPPORT_SENSOR
//***********************************************************************************
//
// Display temperature.
//
//***********************************************************************************
static void Disp_temperature(uint8_t col, uint8_t row, int32_t value, bool deep)
{
    uint8_t thousand,hundreds,integer, decimal;
    Lcd_set_font(128, 8, 0);
    Lcd_clear_area(0,2);
    Lcd_clear_area(0,3);
    Lcd_clear_area(0,4);
    Lcd_clear_area(0,5);
	Lcd_set_font(TPICON_W, TPICON_H, 0);
    
    if ((deep == false && value == TEMPERATURE_OVERLOAD) ||
        (deep == true && value == (DEEP_TEMP_OVERLOAD >> 4))){
        Disp_icon(col, row, TPICON_SUB, 0);
        col += TPICON_W + TPICON_GAP;
        Disp_icon(col, row, TPICON_SUB, 1);
        col += TPICON_W + TPICON_GAP;
        Disp_icon(col, row, TPICON_SUB, 1);
        col += TPICON_W + TPICON_GAP;
    	Lcd_set_font(TPICON_DOT_W, TPICON_DOT_H, 0);
        Disp_icon(col, row, TPICON_DOT, 0);
        col += TPICON_DOT_W + TPICON_GAP;
    	Lcd_set_font(TPICON_W, TPICON_H, 0);
        Disp_icon(col, row, TPICON_DIGIT, 0);
        col += TPICON_W + TPICON_GAP;
    	Lcd_set_font(TPICON_DC_W, TPICON_DC_H, 0);
        Disp_icon(col, row, TPICON_DC, 0);
        return;
    }

    value = (int32_t)round((float)value / 10.0);
    if (value < 0) {
        value = -value;
        Disp_icon(col, row, TPICON_SUB, 1);
    } else {
        Disp_icon(col, row, TPICON_SUB, 0);
    }
    col += TPICON_W + TPICON_GAP;
    thousand = value / 10000;
    if (thousand !=0 ) {
        Disp_icon(col, row, TPICON_DIGIT + thousand, 1);
        col += TPICON_W + TPICON_GAP;

        hundreds = (value % 10000) / 1000;
        Disp_icon(col, row, TPICON_DIGIT + hundreds, 1);
        col += TPICON_W + TPICON_GAP;

        value = (value % 1000);
    } else {
        hundreds = value / 1000;
        if ( hundreds !=0 ){
            Disp_icon(col, row, TPICON_DIGIT + hundreds, 1);
            col += TPICON_W + TPICON_GAP;
            value = value%1000;
        }
    }
    integer = value / 10;
    decimal = value % 10;
    if (integer >= 10) {
        Disp_icon(col, row, TPICON_DIGIT + (integer / 10) % 10, 1);
    } else {
        Disp_icon(col, row, TPICON_DIGIT + (integer / 10) % 10, 0);
    }
    col += TPICON_W + TPICON_GAP;
    Disp_icon(col, row, TPICON_DIGIT + integer % 10, 1);
    col += TPICON_W + TPICON_GAP;
	Lcd_set_font(TPICON_DOT_W, TPICON_DOT_H, 0);
    Disp_icon(col, row, TPICON_DOT, 1);
    col += TPICON_DOT_W + TPICON_GAP;
	Lcd_set_font(TPICON_W, TPICON_H, 0);
    Disp_icon(col, row, TPICON_DIGIT + decimal % 10, 1);
    col += TPICON_W + TPICON_GAP;
	Lcd_set_font(TPICON_DC_W, TPICON_DC_H, 0);
    Disp_icon(col, row, TPICON_DC, 1);
}

//***********************************************************************************
//
// Display humidty.
//
//***********************************************************************************
static void Disp_humidty(uint8_t col, uint8_t row, uint16_t value)
{
	Lcd_set_font(HUICON_W, HUICON_H, 0);
    if (value == HUMIDTY_OVERLOAD) {
        Disp_icon(col, row, HUICON_DIGIT, 0);
        col += HUICON_W + HUICON_GAP;
        Disp_icon(col, row, ICON_9X24_SUB, 1);
        col += HUICON_W + HUICON_GAP;
        Disp_icon(col, row, ICON_9X24_SUB, 1);
        col += HUICON_W + HUICON_GAP;
    	Lcd_set_font(HUICON_PCT_W, HUICON_PCT_H, 0);
        Disp_icon(col, row, HUICON_PCT, 0);
        return;
    }

    value /= 100;
    if (value >= 99) {
        Disp_icon(col, row, HUICON_DIGIT, 0);
        col += HUICON_W + HUICON_GAP;
        Disp_icon(col, row, HUICON_DIGIT + 9, 1);
        col += HUICON_W + HUICON_GAP;
        Disp_icon(col, row, HUICON_DIGIT + 9, 1);
    } else {
        Disp_icon(col, row, HUICON_DIGIT, 0);
        col += HUICON_W + HUICON_GAP;
        if (value >= 10) {
            Disp_icon(col, row, HUICON_DIGIT + (value / 10) % 10, 1);
        } else {
            Disp_icon(col, row, HUICON_DIGIT + (value / 10) % 10, 0);
        }
        col += HUICON_W + HUICON_GAP;
        Disp_icon(col, row, HUICON_DIGIT + value % 10, 1);
    }
    col += HUICON_W + HUICON_GAP;
	Lcd_set_font(HUICON_PCT_W, HUICON_PCT_H, 0);
    Disp_icon(col, row, HUICON_PCT, 1);
}
//***********************************************************************************
//
// Display lux data.
//
//***********************************************************************************
static void Disp_Lux(uint8_t col, uint8_t row, uint32_t value)
{

    uint8_t thousand,hundreds,integer, decimal,flag = 0;
    Lcd_set_font(132, 32, 0);
    Lcd_clear_area(0, 2);
        if(value == 0x00ffffff){

            Lcd_set_font(TPICON_W, TPICON_H, 0);
            Disp_icon(col, row, TPICON_SUB, 0);
            col += TPICON_W + TPICON_GAP;
            Disp_icon(col, row, TPICON_SUB, 1);
            col += TPICON_W + TPICON_GAP;
            Disp_icon(col, row, TPICON_SUB, 1);
            return;
        }

        uint8_t buff[21];
        memset(buff,0x00,21);
        if(value > 999999)
        {
            flag = 1;
            value = value / 1000;
        }

        Lcd_set_font(TPICON_W, TPICON_H, 0);
        value = value / 10;

        col += TPICON_W + TPICON_GAP;
        thousand = value / 10000;
        if(thousand > 9)
        {
            value    = 99999;
            thousand = value / 10000;
        }

        if (thousand !=0 ) {
            Disp_icon(col, row, TPICON_DIGIT + thousand, 1);
            col += TPICON_W + TPICON_GAP;

            hundreds = (value % 10000) / 1000;
            Disp_icon(col, row, TPICON_DIGIT + hundreds, 1);
            col += TPICON_W + TPICON_GAP;

            value = (value % 1000);
        } else {
            hundreds = value / 1000;
            if ( hundreds !=0 ){
                Disp_icon(col, row, TPICON_DIGIT + hundreds, 1);
                col += TPICON_W + TPICON_GAP;
                value = value%1000;
            }
        }
        integer = value / 10;
        decimal = value % 10;
        if (integer >= 10) {
            Disp_icon(col, row, TPICON_DIGIT + (integer / 10) % 10, 1);
        } else {
            if(hundreds !=0){
                Disp_icon(col, row, TPICON_DIGIT + (integer / 10) % 10, 1);
            }else{
                Disp_icon(col, row, TPICON_DIGIT + (integer / 10) % 10, 0);
            }
        }
        col += TPICON_W + TPICON_GAP;
        Disp_icon(col, row, TPICON_DIGIT + integer % 10, 1);
        col += TPICON_W + TPICON_GAP;
        Lcd_set_font(TPICON_DOT_W, TPICON_DOT_H, 0);
        Disp_icon(col, row, TPICON_DOT, 1);
        col += TPICON_DOT_W + TPICON_GAP;
        Lcd_set_font(TPICON_W, TPICON_H, 0);
        Disp_icon(col, row, TPICON_DIGIT + decimal % 10, 1);
        col += TPICON_W + TPICON_GAP;
        col = (col + 7) / 8;
        if(flag){
            sprintf((char *)buff, "Klx");
        }else
        sprintf((char *)buff, "Lx");
        Disp_msg(col, row+2, buff, FONT_8X16);
/*    uint8_t buff[21];

    sprintf((char *)buff, "%ld.%dLx", (uint32_t)(value/100), (uint16_t)round(((value%100))/10));
    Lcd_clear_area(1, 4);
    Disp_msg(1, 2, buff, FONT_12X24);*/

}

#endif  /* SUPPORT_SENSOR */

//***********************************************************************************
//
// Display sensor data.
//
//***********************************************************************************
void Disp_sensor_data(void)
{
#ifdef SUPPORT_SENSOR
    int32_t valueT;
    uint32_t valueL;
    uint16_t valueH;
#endif

    if (rDispObject.init == 0)
        return;

    if (g_bAlarmSensorFlag) {//显示需要报警的senso
        rDispObject.sensorIndex = Alarm_ffs(g_bAlarmSensorFlag);
    }
    
#ifdef SUPPORT_SENSOR
    if (rDispObject.sensorIndex < MODULE_SENSOR_MAX) {    //display  local sensor
        if (Sensor_get_function(rDispObject.sensorIndex) == (SENSOR_TEMP | SENSOR_HUMI)) {
            valueT = Sensor_get_temperatureC(rDispObject.sensorIndex);
            valueH = Sensor_get_humidty(rDispObject.sensorIndex);
            Disp_temperature(TEMP0_COL_POS, TEMP0_ROW_POS, valueT,false);
            Disp_humidty(HUMI_COL_POS, HUMI_ROW_POS, valueH);

        } else if (Sensor_get_function(rDispObject.sensorIndex) == SENSOR_TEMP) {
            valueT = Sensor_get_temperatureC(rDispObject.sensorIndex);
            Disp_temperature(TEMP1_COL_POS, TEMP1_ROW_POS, valueT,false);
        }else if(Sensor_get_function(rDispObject.sensorIndex) == SENSOR_DEEP_TEMP){
            valueT = Sensor_get_deepTemperatureC(rDispObject.sensorIndex);
            Disp_temperature(TEMP1_COL_POS, TEMP1_ROW_POS, valueT>>4, true);
        }else if(Sensor_get_function(rDispObject.sensorIndex) == SENSOR_LIGHT){
            valueL = Sensor_get_lux(rDispObject.sensorIndex);
            Disp_Lux(LUX_COL_POS, TEMP1_ROW_POS,valueL);
        }
        return;
    }
#endif

    
#ifdef SUPPORT_NETGATE_DISP_NODE
    sensordata_mem Sensor = {0,0,0,0};
    uint8_t buff[32];
    uint16_t  cursensorno = 0;

    if(g_rSysConfigInfo.module & MODULE_NWK && 
       g_rSysConfigInfo.module & MODULE_RADIO ) {//is netgate, display  node  sensor
            
        if (g_bAlarmSensorFlag) {

            if (g_rSysConfigInfo.status & STATUS_SENSOR_NAME_ON)
                cursensorno = Flash_load_sensor_codec(g_AlarmSensor.DeviceId); //find sensor no in sensor codec table           

            if (cursensorno){
                sprintf((char*)buff, "%02d#", cursensorno);
                Disp_msg(4, 2, buff, FONT_8X16);
            }
            else {                
                sprintf((char*)buff, "%08lx", g_AlarmSensor.DeviceId);
                Disp_msg(2, 2, buff, FONT_8X16);
            }

            //all  data  saved to tempdeep
            if (g_AlarmSensor.type == SENSOR_DATA_TEMP) {

                TempToDisplayBuff(g_AlarmSensor.value.tempdeep,buff,g_AlarmSensor.index);

                Lcd_set_font(132, 16, 0);
				Lcd_clear_area(2, 4);
                Disp_msg(2, 4, buff, FONT_8X16);    
            }
            else if (g_AlarmSensor.type == SENSOR_DATA_HUMI) {
                sprintf((char*)buff, "%02d%%", (uint16_t)g_AlarmSensor.value.tempdeep/100);
                Lcd_set_font(132, 16, 0);
                Lcd_clear_area(2, 4);
                Disp_msg(2, 4, buff, FONT_8X16);    
            }
            

            return;
        }


        if(get_next_sensor_memory(&Sensor)){

            if (g_rSysConfigInfo.status & STATUS_SENSOR_NAME_ON)
                cursensorno = Flash_load_sensor_codec(Sensor.DeviceId); //find sensor no in sensor codec table             
            
            if (cursensorno){
                sprintf((char*)buff, "%02d#", cursensorno);
                Disp_msg(4, 2, buff, FONT_8X16);
            }
            else {                
                sprintf((char*)buff, "%08lx", Sensor.DeviceId);
                Disp_msg(2, 2, buff, FONT_8X16);
            }
            
            if (Sensor_get_function_by_type(Sensor.type) == (uint32_t)(SENSOR_TEMP | SENSOR_HUMI)) {
                if (Sensor.value.temp != TEMPERATURE_OVERLOAD){//temp valid

                    TempToDisplayBuff((int32_t)Sensor.value.temp,buff,Sensor.index);
                }
                else 
                    sprintf((char*)buff, "--c");
                
                if (Sensor.value.humi != HUMIDTY_OVERLOAD) //humi valid
                    sprintf((char*)(buff + strlen((const char *)buff)), " %02d%%", Sensor.value.humi/100);
                else 
                    sprintf((char*)(buff + strlen((const char *)buff)), " --%%");
                Lcd_set_font(132, 16, 0);
                Lcd_clear_area(2, 4);
                Disp_msg(2, 4, buff, FONT_8X16);                
            } else if (Sensor_get_function_by_type(Sensor.type) == SENSOR_TEMP) {   
            
                if (Sensor.value.temp != TEMPERATURE_OVERLOAD){//temp valid

                    TempToDisplayBuff((int32_t)Sensor.value.temp,buff,Sensor.index);
                }
                else 
                    sprintf((char*)buff, "--c");
                Lcd_set_font(132, 16, 0);
                Lcd_clear_area(2, 4);
                Disp_msg(2, 4, buff, FONT_8X16);                
            } else if (Sensor_get_function_by_type(Sensor.type) == (uint32_t)SENSOR_DEEP_TEMP){
                if (Sensor.value.tempdeep != DEEP_TEMP_OVERLOAD){

                    TempToDisplayBuff(Sensor.value.tempdeep,buff,Sensor.index);
                }
                else
                    sprintf((char*)buff, "--c");
                Lcd_set_font(132, 16, 0);
                Lcd_clear_area(2, 4);
                Disp_msg(2, 4, buff, FONT_8X16);
            } else if(Sensor_get_function_by_type(Sensor.type) == (uint32_t)SENSOR_LIGHT){

                if (Sensor.value.lux != DEEP_TEMP_OVERLOAD){
                    sprintf((char *)buff,  "%ld.%dLx", (uint32_t)((Sensor.value.lux&0x00ffffff)/100),(uint16_t)(LOWORD_ZKS(Sensor.value.lux)%100/10.0));
                }
                else
                    sprintf((char*)buff, "--c");
                Lcd_set_font(132, 16, 0);
                Lcd_clear_area(2, 4);
                Disp_msg(2, 4, buff, FONT_8X16);
            }
#ifdef SUPPORT_UPLOAD_ASSET_INFO
            else if(Sensor.type == SEN_TYPE_ASSET){
                sprintf((char*)buff, "%02x-%02x %02x:%02x", Sensor.value.month, Sensor.value.day, Sensor.value.hour, Sensor.value.minutes);
                Lcd_set_font(132, 16, 0);
                Lcd_clear_area(2, 4);
                Disp_msg(2, 4, buff, FONT_8X16);
            }
#endif // SUPPORT_UPLOAD_ASSET_INFO
        }
    }
        
#endif
    
}

//***********************************************************************************
//
// Display sensor switch.
//
//***********************************************************************************
void Disp_sensor_switch(void)
{
    uint8_t i, num, oldNum;

    if (rDispObject.init == 0)
        return;

    if (rDispObject.infoIndex) {
        rDispObject.infoIndex = 0;
        Disp_clear_all();
        return;
    }

#ifdef SUPPORT_NETGATE_DISP_NODE    
    if(g_rSysConfigInfo.module & MODULE_NWK && 
       g_rSysConfigInfo.module & MODULE_RADIO ) {//is netgate, only display  node  sensor
        //If exist more than 2 type temperature, should switch display.
    	Lcd_set_font(128, 32, 0);
        Lcd_clear_area(0, 2);
        rDispObject.sensorIndex = MODULE_SENSOR_MAX;  //don't display local sensor data

        return;        
    }            
#endif

    oldNum = rDispObject.sensorIndex;
    for (i = 0; i < MODULE_SENSOR_MAX; i++) {
        num = (rDispObject.sensorIndex + i + 1) % MODULE_SENSOR_MAX;
        if ((g_rSysConfigInfo.sensorModule[num] != SEN_TYPE_NONE)&&(g_rSysConfigInfo.sensorModule[num] != SEN_TYPE_GSENSOR)) {

            if((g_rSysConfigInfo.status & STATUS_HIDE_SHT_SENSOR) && (g_rSysConfigInfo.sensorModule[num] == SEN_TYPE_SHT2X)){
                continue;//hide sht20 sensor
            }
            
            rDispObject.sensorIndex = num;
            break;
        }
    }
    if (oldNum != num && g_rSysConfigInfo.sensorModule[oldNum] != g_rSysConfigInfo.sensorModule[num]) {
        //If exist more than 2 type temperature, should switch display.
    	Lcd_set_font(128, 32, 0);
        Lcd_clear_area(0, 2);
    }

}


//***********************************************************************************
//
// Display status bar.
//
//***********************************************************************************
static void Disp_status_bar(void)
{
    uint8_t col = STATUSB_COL_POS, row = STATUSB_ROW_POS;
    uint16_t value;

	Lcd_set_font(SBICON_W, SBICON_H, 0);

#if 1//def SUPPORT_SENSOR
//Display external sensor flag
    if ( (rDispObject.sensorIndex  ==  0 && g_rSysConfigInfo.sensorModule[0] == SEN_TYPE_SHT2X) ||
        (rDispObject.sensorIndex  ==  0 && g_rSysConfigInfo.sensorModule[0] == SEN_TYPE_NONE) ||
        (rDispObject.sensorIndex  ==  2 && g_rSysConfigInfo.sensorModule[0] == SEN_TYPE_OPT3001) ){
        Disp_icon(col, row, ICON_16X16_EXTERNAL, 0);            
    } else {
        Disp_icon(col, row, ICON_16X16_EXTERNAL, 1);
    }
#endif
    col += SBICON_W;

#ifdef SUPPORT_MENU
//Display record flag
    if (Menu_is_record()) {
        Disp_icon(col, row, ICON_16X16_RECORD, 1);
    } else {
        Disp_icon(col, row, ICON_16X16_RECORD, 0);
    }
#endif

//Diaplay Alarm flag
    col += SBICON_W;
    if (g_bAlarmSensorFlag & (1 << rDispObject.sensorIndex))
        Disp_icon(col, row, ICON_16X16_ALARM, 1);
    else
        Disp_icon(col, row, ICON_16X16_ALARM, 0);
    col += SBICON_W * 4;

//Display signal    or flight
#ifdef SUPPORT_FLIGHT_MODE
    if(Flight_mode_isFlightMode() && (!Nwk_is_Active())){
        Disp_icon(col, row, ICON_16X16_FLIGHT, 1);
    }else
#endif
    {
    value = Nwk_get_rssi();
    if (value < 2 || value == 99)
        Disp_icon(col, row, ICON_16X16_SIGNAL0, 0);
    else if (value >= 12)
        Disp_icon(col, row, ICON_16X16_SIGNAL3, 1);
    else if (value >= 8)
        Disp_icon(col, row, ICON_16X16_SIGNAL2, 1);
    else if (value >= 5)
        Disp_icon(col, row, ICON_16X16_SIGNAL1, 1);
    else
        Disp_icon(col, row, ICON_16X16_SIGNAL0, 1);
    }
    col += SBICON_W;

#ifdef SUPPORT_BATTERY
//Display battery
    value = Battery_get_voltage();
    if (value >= BAT_VOLTAGE_L3)
        Disp_icon(col, row, ICON_16X16_BATTERY3, 1);
    else if (value >= BAT_VOLTAGE_L2)
        Disp_icon(col, row, ICON_16X16_BATTERY2, 1);
    else if (value >= BAT_VOLTAGE_L1)
        Disp_icon(col, row, ICON_16X16_BATTERY1, 1);
    else
        Disp_icon(col, row, ICON_16X16_BATTERY0, 1);
#endif
}
//***********************************************************************************
//
// Display info switch.
//
//***********************************************************************************
void Disp_info_switch(void)
{
    uint8_t info_num = 4, sensor_num = 0, i;

    if (rDispObject.init == 0)
        return;

    
    for(i =0; i< MODULE_SENSOR_MAX; ++i){
        if((g_rSysConfigInfo.sensorModule[i] != SEN_TYPE_NONE)&&(g_rSysConfigInfo.sensorModule[i] != SEN_TYPE_GSENSOR)) {
            sensor_num++;
        }
    }

    info_num +=(sensor_num/2);//  info  5 , 6, 7 ,8

        
    if (++rDispObject.infoIndex >= info_num) {
        rDispObject.infoIndex = 0;
    }
    Disp_clear_all();
}

void Disp_info_close(void)
{    
    if (rDispObject.init == 0)
        return;
    

    rDispObject.infoIndex = 0;    
    Disp_clear_all();
}


//***********************************************************************************
//
// Display info.
//
//***********************************************************************************
static void Disp_info(void)
{
    uint8_t temp, buff[21],i,j;
#ifdef SUPPORT_LORA
    uint8_t *gateid;
#endif

    switch (rDispObject.infoIndex) {
        case 1:
            //MAC ID
            sprintf((char *)buff, "DEID: %02x%02x%02x%02x", g_rSysConfigInfo.DeviceId[0], g_rSysConfigInfo.DeviceId[1],
                                                            g_rSysConfigInfo.DeviceId[2], g_rSysConfigInfo.DeviceId[3]);
            Disp_msg(0, 0, buff, FONT_8X16);

            //CUSTOM ID
            sprintf((char *)buff, "CUID: %02x%02x", g_rSysConfigInfo.customId[0], g_rSysConfigInfo.customId[1]);
            Disp_msg(0, 2, buff, FONT_8X16);

			if (g_rSysConfigInfo.module & MODULE_GSM) {
                //SIM CCID
                Disp_msg(0, 4, "CCID: ", FONT_8X16);
                memset(buff, 0 ,21);
                Nwk_get_simccid(buff);
                buff[20] = '\0';
                temp = buff[10];
                buff[10] = '\0';
                Disp_msg(6, 4, buff, FONT_8X16);
                buff[10] = temp;
                Disp_msg(6, 6, &buff[10], FONT_8X16);
			}

			
#ifdef S_C
			
			if (*(uint32_t*)g_rSysConfigInfo.BindGateway  != 0 ){

				sprintf((char *)buff, "BGID: %02x%02x%02x%02x", g_rSysConfigInfo.BindGateway[0], g_rSysConfigInfo.BindGateway[1],
																g_rSysConfigInfo.BindGateway[2], g_rSysConfigInfo.BindGateway[3]);
				Disp_msg(0, 4, buff, FONT_8X16);
			}else{
				Disp_msg(0, 4, "BGID: ", FONT_8X16);
				memset(buff, 0 ,21);
			}
#endif
            break;

        case 2:
            //Collect period.
            sprintf((char *)buff, "CLPD: %lds", g_rSysConfigInfo.collectPeriod);
            Disp_msg(0, 0, buff, FONT_8X16);
            //upload period.
            sprintf((char *)buff, "ULPD: %lds", g_rSysConfigInfo.uploadPeriod);
            Disp_msg(0, 2, buff, FONT_8X16);
#ifdef FLASH_EXTERNAL
            //Collect period.
            memset(buff, 0 ,21);
            sprintf((char *)buff, "RMIT: %ld", Flash_get_unupload_items());
            Lcd_set_font(132, 16, 0);
            Lcd_clear_area(0,4);
            Disp_msg(0, 4, buff, FONT_8X16);
            //upload period.
            memset(buff, 0 ,21);
            sprintf((char *)buff, "RCIT: %ld", Flash_get_record_items());
            Lcd_set_font(132, 16, 0);
            Lcd_clear_area(0,6);
            Disp_msg(0, 6, buff, FONT_8X16);
#endif
            break;

        case 3:
            //Software version.
            sprintf((char *)buff, "FW:  %x.%x.%x", FW_VERSION >> 12, (FW_VERSION >> 8) & 0x0f, FW_VERSION & 0xff);
            Disp_msg(0, 0, buff, FONT_8X16);
           

            for(i =0; i< MODULE_SENSOR_MAX; ++i){
                if(g_rSysConfigInfo.sensorModule[i] != SEN_TYPE_NONE) {
                    if(g_rSysConfigInfo.alarmTemp[i].high == ALARM_TEMP_HIGH && g_rSysConfigInfo.alarmTemp[i].low == ALARM_TEMP_LOW)
                        sprintf((char *)buff, "TA%02d:    ", i);
                    else if(g_rSysConfigInfo.alarmTemp[i].high == ALARM_TEMP_HIGH)
                        sprintf((char *)buff, "TA%02d:%d~ ", i, g_rSysConfigInfo.alarmTemp[i].low/100);
                    else if(g_rSysConfigInfo.alarmTemp[i].low == ALARM_TEMP_LOW)
                        sprintf((char *)buff, "TA%02d: ~%d", i,g_rSysConfigInfo.alarmTemp[i].high/100);
                    else                        
                        sprintf((char *)buff, "TA%02d:%d~%d", i, g_rSysConfigInfo.alarmTemp[i].low/100,g_rSysConfigInfo.alarmTemp[i].high/100);
                    
                    Disp_msg(0, 2*2, buff, FONT_8X16);     
                    
                    if(g_rSysConfigInfo.WarningTemp[i].high == ALARM_TEMP_HIGH && g_rSysConfigInfo.WarningTemp[i].low == ALARM_TEMP_LOW)
                        sprintf((char *)buff, "PA%02d:    ", i);
                    else if(g_rSysConfigInfo.WarningTemp[i].high == ALARM_TEMP_HIGH)
                        sprintf((char *)buff, "PA%02d:%d~ ", i, g_rSysConfigInfo.WarningTemp[i].low/100);
                    else if(g_rSysConfigInfo.WarningTemp[i].low == ALARM_TEMP_LOW)
                        sprintf((char *)buff, "PA%02d: ~%d", i,g_rSysConfigInfo.WarningTemp[i].high/100);
                    else                        
                        sprintf((char *)buff, "PA%02d:%d~%d", i, g_rSysConfigInfo.WarningTemp[i].low/100,g_rSysConfigInfo.WarningTemp[i].high/100);
                    
                    Disp_msg(0, 2*3, buff, FONT_8X16);                    

                    break;//只显示第一个通道的报警和预警信息
                }                
            }
            
            break;
            
        case 4 ... 7:
            
            j= 0;
            
            for(i =0; i< MODULE_SENSOR_MAX; ++i){
                if(g_rSysConfigInfo.sensorModule[i] != SEN_TYPE_NONE) {

                    if(j++ <= ((rDispObject.infoIndex-4)*2))continue;//跳过第一个通道的报警和预警信息

                    if (j > ((rDispObject.infoIndex-4)*2 + 3))break;//只显示第 2 / 3 个通道的报警和预警信息
                    
                    if(g_rSysConfigInfo.alarmTemp[i].high == ALARM_TEMP_HIGH && g_rSysConfigInfo.alarmTemp[i].low == ALARM_TEMP_LOW)
                        sprintf((char *)buff, "TA%02d:    ", i);
                    else if(g_rSysConfigInfo.alarmTemp[i].high == ALARM_TEMP_HIGH)
                        sprintf((char *)buff, "TA%02d:%d~ ", i, g_rSysConfigInfo.alarmTemp[i].low/100);
                    else if(g_rSysConfigInfo.alarmTemp[i].low == ALARM_TEMP_LOW)
                        sprintf((char *)buff, "TA%02d: ~%d", i,g_rSysConfigInfo.alarmTemp[i].high/100);
                    else                        
                        sprintf((char *)buff, "TA%02d:%d~%d", i, g_rSysConfigInfo.alarmTemp[i].low/100,g_rSysConfigInfo.alarmTemp[i].high/100);
                    
                    Disp_msg(0, 4*((((j-1)%2)?1:2)-1), buff, FONT_8X16);                    

                    if(g_rSysConfigInfo.WarningTemp[i].high == ALARM_TEMP_HIGH && g_rSysConfigInfo.WarningTemp[i].low == ALARM_TEMP_LOW)
                        sprintf((char *)buff, "PA%02d:    ", i);
                    else if(g_rSysConfigInfo.WarningTemp[i].high == ALARM_TEMP_HIGH)
                        sprintf((char *)buff, "PA%02d:%d~ ", i, g_rSysConfigInfo.WarningTemp[i].low/100);
                    else if(g_rSysConfigInfo.WarningTemp[i].low == ALARM_TEMP_LOW)
                        sprintf((char *)buff, "PA%02d: ~%d", i,g_rSysConfigInfo.WarningTemp[i].high/100);
                    else                        
                        sprintf((char *)buff, "PA%02d:%d~%d", i, g_rSysConfigInfo.WarningTemp[i].low/100,g_rSysConfigInfo.WarningTemp[i].high/100);
                    
                    Disp_msg(0, 4*((((j-1)%2)?1:2)-1)+2, buff, FONT_8X16);   

                }
                
            }

            break;
    }

}

void TempToDisplayBuff(int32_t value,uint8_t *buff,uint8_t ch)
{

    value = (int32_t)round( value / 10.0);
    if(value < 0){

       sprintf((char*)buff, "%d -%2d.%dc", ch, (uint16_t)(-value/10), (uint16_t)(-value %10));
    }
    else{
       sprintf((char*)buff, "%d %2d.%dc",ch, (uint16_t)(value/10), (uint16_t)(value %10));

    }
}
//***********************************************************************************
//
// Display picture.
//
//***********************************************************************************
void Disp_picture(uint8_t col, uint8_t row, uint8_t wide, uint8_t high, const uint8_t *picture)
{
    if (rDispObject.init == 0)
        return;

	Lcd_set_font(wide, high, 0);
    Lcd_write_character(col, row, picture);
}

//***********************************************************************************
//
// Display process.
//
//***********************************************************************************
void Disp_proc(void)
{
    if (rDispObject.init == 0)
        return;

#ifdef SUPPORT_LORA
    //增加采集器/网关显示注册信息
   if( g_rSysConfigInfo.module & MODULE_RADIO ) {
       if(Lora_get_ntp()!= 0)
           Disp_msg(0, 6, "              ", FONT_8X16);//clear 
   }
#endif


#ifdef SUPPORT_MENU
    if (Menu_is_process()) {
        Menu_show();
    } else
#endif
    if (rDispObject.infoIndex) {
        Disp_info();
    } else {
        Disp_calendar();
#if 1//def SUPPORT_SENSOR
        Disp_sensor_data();
#endif
        Disp_status_bar();

#ifdef SUPPORT_FREQ_FIND
       uint8_t buff[8];
       if(AutoFreqStateRead() == false)
       {
            Disp_msg(3, 6, "Register", FONT_8X16);//display
       }
       else
       {
            sprintf((char *)buff, "ch:%d", (g_rSysConfigInfo.rfBW >> 4));
            Disp_msg(3, 6, "         ", FONT_8X16);
            Disp_msg(3, 6, buff, FONT_8X16);//display
       }
#endif // SUPPORT_FREQ_FIND
    }

#ifdef SUPPORT_LORA
    //增加采集器/网关显示注册信息
   if( g_rSysConfigInfo.module & MODULE_RADIO ) {
      if(Lora_get_ntp()== 0){
          if(*(uint32_t*)g_rSysConfigInfo.DeviceId != 0)
            Disp_msg(0, 6, "Registering...", FONT_8X16);//display
          else
            Disp_msg(0, 6, "NO DEID", FONT_8X16);//display
      }
   }
#endif



}

//***********************************************************************************
//
// Display init.
//
//***********************************************************************************
void Disp_init(void)
{
    rDispObject.init = 0;
    rDispObject.refresh = 0;
}

//***********************************************************************************
//
// Display power on.
//
//***********************************************************************************
bool Disp_poweron(void)
{
    bool ret = false;
    if(!(g_rSysConfigInfo.module & MODULE_LCD))
       return false;//no lcd

    if (rDispObject.init == 0) {
        Lcd_init();
        rDispObject.init = 1;
        rDispObject.infoIndex = 0;
        ret = true;
        
#ifdef SUPPORT_SENSOR
        uint8_t i;
        for (i = 0; i < MODULE_SENSOR_MAX; i++) {
            if (g_rSysConfigInfo.sensorModule[i] != SEN_TYPE_NONE) {
                if((g_rSysConfigInfo.status & STATUS_HIDE_SHT_SENSOR) && (g_rSysConfigInfo.sensorModule[i] == SEN_TYPE_SHT2X)){
                    continue;//hide sht20 sensor
                }
                break;
            }
        }
        rDispObject.sensorIndex = i;
#endif

#ifdef SUPPORT_NETGATE_DISP_NODE    
        if(g_rSysConfigInfo.module & MODULE_NWK && g_rSysConfigInfo.module & MODULE_RADIO ) {//is netgate, display  node  sensor，don't display local sensor data
            rDispObject.sensorIndex = MODULE_SENSOR_MAX;
        }
#endif

    }

#ifdef BOARD_S6_6
    Sys_lcd_start_timing();
#endif
    return ret;
}

//***********************************************************************************
//
// Display power off.
//
//***********************************************************************************
void Disp_poweroff(void)
{
    if(!(g_rSysConfigInfo.module & MODULE_LCD))
       return;//no lcd

#ifdef SUPPORT_MENU
    Menu_exit();
#endif
    Lcd_poweroff();
    rDispObject.init = 0;
    rDispObject.infoIndex = 0;
#ifdef BOARD_S6_6
    Sys_lcd_stop_timing();
#endif
}

#endif  /* SUPPORT_DISP_SCREEN */
#endif
