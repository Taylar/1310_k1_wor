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
#ifdef SUPPORT_DISP_SCREEN
// const uint8_t font5x8[]= {
//     #include "font\font5x8.txt"
// };

const uint8_t font8x16[]= {
    #include "font\font8X16_.txt"
};

const uint8_t font8x24[]= {
    #include "font\font8X24.txt"
};

const uint8_t font6x8[]= {
    #include "font\font6X8.txt"
};

// const uint8_t font12x24[]= {
//     #include "font\font12x24.txt"
// };

const uint8_t icon5x8[]= {
    #include "font\icon5x8.txt"
};

const uint8_t icon16x16[]= {
    #include "font\icon16x16.txt"
};



const uint8_t font12x24[]= {
    #include "font\font12x24.txt"
};

const uint8_t icon12x24[]= {
    #include "font\icon12x24.txt"
};


const uint8_t icon16x32[]= {
    #include "font\icon16x32.txt"
};
const uint8_t menu72x24[]= {
    #include "font\menu72x24.txt"
};
const uint8_t menu36x24[]= {
    #include "font\menu36x24.txt"
};

const uint8_t menu8x24[]= {
    #include "font\menu8x24.txt"
};
const uint8_t menu32x24[]= {
    #include "font\menu32x24.txt"
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
#ifdef S_G
#define CALD_COL_POS            2
#define CALD_ROW_POS            0
#else
#define CALD_COL_POS            14
#define CALD_ROW_POS            0
#endif
#define DISP_INTERVAL_TIME      2000


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
        fgInv = 0;
    else
        fgInv = 1;

    switch (bFont & 0x7f) {
         case FONT_5X8:
             bColStart *= 3;
             bColStart += LCD_FONT5X8_START_COL;
             Lcd_set_font(6, 8, fgInv);
             Lcd_write_character(bColStart, bPageStart, &font6x8[bChar * 8]);
             // We use font 5x8, there are one space between char, so need clear the space.
             //Lcd_set_font(1, 8, fgInv);
             //Lcd_write_character(bColStart + 5, bPageStart, &font5x8[0]);
            break;

        case FONT_8X16:
            //bColStart *= 4;
            bColStart += LCD_FONT8X16_START_COL;
//            bPageStart *= 2;
            Lcd_set_font(8, 16, fgInv);
            Lcd_write_character(bColStart, bPageStart, &font8x16[bChar * 16]);
        break;
        case FONT_8X24:
            //bColStart *= 8;
            bColStart += LCD_FONT8X16_START_COL;
//            bPageStart *= 2;
            Lcd_set_font(8, 24, fgInv);
            Lcd_write_character(bColStart, bPageStart, &font8x24[bChar * 24]);
        break;
        case FONT_12X24:
             //bColStart *= 6;
             bColStart += LCD_FONT12X24_START_COL;
             Lcd_set_font(12, 24, fgInv);
            // bPageStart = bPageStart * 3 + 2;
             Lcd_write_character(bColStart, bPageStart, &font12x24[bChar * 48]);
         break;

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
#ifdef S_G
        case ICON_16X16_EXTERNAL ... ICON_16X16_UPLOAD://ICON_16X16_BATTERY3:
            if (light) {
                Lcd_write_character(col, row, &icon16x16[(icon - ICON_16X16_EXTERNAL) * FONT_16X16_OFS]);
            } else {
                Lcd_clear_area(col, row);
            }
            break;
#else
        case ICON_16X16_EXTERNAL ... ICON_16X16_FLIGHT://ICON_16X16_BATTERY3:
            if (light) {
                Lcd_write_character(col, row, &icon16x16[(icon - ICON_16X16_EXTERNAL) * FONT_16X16_OFS]);
            } else {
                Lcd_clear_area(col, row);
            }
            break;
#endif

        case ICON_12X24_DIGIT_0 ... ICON_12X24_PERCENT:
            if (light) {
                Lcd_write_character(col, row, &icon12x24[(icon - ICON_12X24_DIGIT_0) * FONT_12X24_OFS]);
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
        case ICON_72X24_ADD_ARR ... ICON_72X24_DESTORYED:
            if (light) {
                Lcd_write_character(col, row, &menu72x24[(icon - ICON_72X24_ADD_ARR) * FONT_72X24_OFS]);
            } else {
                Lcd_clear_area(col, row);
            }
            break;
        case ICON_36X24_COMPLETE...ICON_36X24_VBAT:
            if (light) {
                Lcd_write_character(col, row, &menu36x24[(icon - ICON_36X24_COMPLETE) * FONT_36X24_OFS]);
            } else {
                Lcd_clear_area(col, row);
            }
             break;
        case ICON_32X24_COMPLETE...ICON_32X24_CLEAR:
                if (light) {
                    Lcd_write_character(col, row, &menu32x24[(icon - ICON_32X24_COMPLETE) * FONT_32X24_OFS]);
                } else {
                    Lcd_clear_area(col, row);
                }
             break;
        case ICON_8X24_ARROW...ICON_8X24_DISPLAY_CLEAR:
            if (light) {
                Lcd_write_character(col, row, &menu8x24[(icon - ICON_8X24_ARROW) * FONT_8X24_OFS]);
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
void Disp_msg(uint16_t col, uint16_t row, const uint8_t *pChar, E_FONT_TYPE eFont)
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
// Display sensor data.
//
//***********************************************************************************
#ifdef SUPPORT_NETGATE_DISP_NODE

#define  DEVIDCOL               8
#define  DEVIDROW               6
uint32_t starBarDeviceid = 0x00;

#endif

void Disp_sensor_data(void)
{

}

//***********************************************************************************
//
// Display sensor switch.
//
//***********************************************************************************
void Disp_sensor_switch(void)
{




}

void Disp_sensor_set(uint8_t index)
{
	rDispObject.sensorIndex = index;
}
//***********************************************************************************
//
// Display status bar.
//
//***********************************************************************************





//***********************************************************************************
//
// Display info switch.
//
//***********************************************************************************
void Disp_info_switch(void)
{

}

void Disp_info_reset(void)
{

}

void Disp_info_exit(void)
{

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
#ifdef RECORD_DISPLAY
extern void Disp_info_record(void);
#endif

void Disp_info(void)
{

}

void TempToDisplayBuff(int32_t value,uint8_t *buff,uint8_t ch)
{

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
void display_star_mssage(void)
{
    uint8_t time_buff[20]={0};
    Calendar calendar;
    uint8_t batValue = 0;
    batValue = ((Battery_get_voltage()-BAT_VOLTAGE_LOW )*100)/ (BAT_VOLTAGE_FULL-BAT_VOLTAGE_LOW);
    calendar = Rtc_get_calendar();
    //sprintf(time_buff,"%02d",(calendar.Year-2000));
    //time_buff[2] ='-';
    sprintf((char*)time_buff,"%02d",calendar.Month);
    time_buff[2] ='-';
    sprintf((char*)(time_buff+3),"%02d",calendar.DayOfMonth);
    time_buff[5] =' ';
    sprintf((char*)(time_buff+6),"%02d",calendar.Hours);
    time_buff[8] =':';
    sprintf((char*)(time_buff+9),"%02d",calendar.Minutes);

    if(batValue > 99)
        batValue = 99;
    if(batValue < 0)
        batValue = 0;
    sprintf(time_buff+11," %02d%%",batValue);

    //time_buff[11] = ' ';
    //time_buff[12] = '9';
    //time_buff[13] = '8';
    //time_buff[14] = '%';
    Disp_msg(0,0,time_buff,FONT_8X24);
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
    //Lcd_clear_screen();

    display_star_mssage();

#if 0
    if(gatewayConfigTime)
    {
        Disp_clear_all();
        Disp_msg(2, 3, "Pair Mode", FONT_8X16);//display
        return;
    }

#ifdef SUPPORT_LORA
    //增加采集?网关显示注册信息
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


       if(!(g_rSysConfigInfo.rfStatus & STATUS_LORA_CHANGE_FREQ))
       {
           if(AutoFreqStateRead() == false)
           {
                Disp_msg(3, 6, "Register", FONT_8X16);//display
           }
       }

    }

#ifdef SUPPORT_LORA
    //增加采集?网关显示注册信息
   if( g_rSysConfigInfo.module & MODULE_RADIO ) {
      if(Lora_get_ntp()== 0){
          if(*(uint32_t*)g_rSysConfigInfo.DeviceId != 0)
            Disp_msg(0, 6, "Registering...", FONT_8X16);//display
          else
            Disp_msg(0, 6, "NO DEID", FONT_8X16);//display
      }
   }
#endif
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
        gpio_power_en_config();
        Lcd_init();
        rDispObject.init = 1;
        rDispObject.infoIndex = 0;
        ret = true;
        
#ifdef SUPPORT_SENSOR
        uint8_t i;
        for (i = 0; i < MODULE_SENSOR_MAX; i++) {
            if ((g_rSysConfigInfo.sensorModule[i] != SEN_TYPE_NONE) && 
			   (g_rSysConfigInfo.sensorModule[i] != SEN_TYPE_GSENSOR)) {
                if((g_rSysConfigInfo.status & STATUS_HIDE_SHT_SENSOR) && (g_rSysConfigInfo.sensorModule[i] == SEN_TYPE_SHT2X)){
                    continue;//hide sht20 sensor
                }								
				rDispObject.sensorIndex = i;//remember the current sensor index.				
				if(g_rSysConfigInfo.sensorModule[i] != SEN_TYPE_SHT2X )				
	                break;//remember the first sensor index,whitch is not sht20
		    }
        }        
        
#endif

#ifdef SUPPORT_NETGATE_DISP_NODE    
        if(g_rSysConfigInfo.module & MODULE_NWK && g_rSysConfigInfo.module & MODULE_RADIO ) {//is netgate, display  node  sensor??don't display local sensor data
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
