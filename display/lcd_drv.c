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

#define LCD_POWER_PIN               IOID_25
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





#define Lcd_power_ctrl(on)          (on) ? (PIN_setOutputValue(lcdPinHandle, LCD_POWER_PIN, 1)) : (PIN_setOutputValue(lcdPinHandle, LCD_POWER_PIN, 0))

#if  defined(LCD_ST7567A) || defined(OLED_LX12864K1)
//Use software spi
#define Lcd_spiDatCtrl(on)          (on) ? (PIN_setOutputValue(lcdPinHandle, LCD_DATA_PIN, 1)) : (PIN_setOutputValue(lcdPinHandle, LCD_DATA_PIN, 0))
#define Lcd_spiClkCtrl(on)          (on) ? (PIN_setOutputValue(lcdPinHandle, LCD_CLK_PIN, 1)) : (PIN_setOutputValue(lcdPinHandle, LCD_CLK_PIN, 0))
#define Lcd_spiCsCtrl(on)           (on) ? (PIN_setOutputValue(lcdPinHandle, LCD_CS_PIN, 1)) : (PIN_setOutputValue(lcdPinHandle, LCD_CS_PIN, 0))
#define Lcd_spiDcCtrl(on)           (on) ? (PIN_setOutputValue(lcdPinHandle, LCD_DATA_C_PIN, 1)) : (PIN_setOutputValue(lcdPinHandle, LCD_DATA_C_PIN, 0))

#define Lcd_reset_ctrl(on)          (!on) ? (PIN_setOutputValue(lcdPinHandle, LCD_RESET_PIN, 1)) : (PIN_setOutputValue(lcdPinHandle, LCD_RESET_PIN, 0))


static PIN_State   lcdPinState;
static PIN_Handle  lcdPinHandle = NULL;

#endif  // defined(LCD_ST7567A) || defined(OLED_LX12864K1)


static uint8_t fgLcdFontInverse;
static uint8_t bLcdFontPages;       // Font Pages = Rows / 8
static uint8_t bLcdFontCols;        // Font Cols

const uint8_t oledComTable[] = {
    0,1,2,3,4,5,6,7,
};

#if defined(LCD_ST7567A) || defined(OLED_LX12864K1)

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
    Lcd_reset_ctrl(0);
    Task_sleep(50 * CLOCK_UNIT_MS);
    Lcd_reset_ctrl(1);
    Task_sleep(200 * CLOCK_UNIT_MS);
    Lcd_reset_ctrl(0);
    Task_sleep(200 * CLOCK_UNIT_MS);
}

#endif



//***********************************************************************************
//
// LCD init.
//
//***********************************************************************************
void All_Screen(void);
void cleanDDR(void)
{
int i,j;
 Lcd_send_cmd(0x15);//Set column address
 Lcd_send_cmd(0x00);//Column Start Address
 Lcd_send_cmd(0x7f);//Column End Address
 Lcd_send_cmd(0x75);//Set row address
 Lcd_send_cmd(0x00);//Row Start Address
 Lcd_send_cmd(0x7f);//Row End Address
 for(i=0;i<128;i++)
 {
 for(j=0;j<64;j++)
 {
     Lcd_send_data(0x00);
 }
 }
}
void ALLDDR(void)
{
int i,j;
 Lcd_send_cmd(0x15);//Set column address
 Lcd_send_cmd(0x00);//Column Start Address
 Lcd_send_cmd(0x7f);//Column End Address
 Lcd_send_cmd(0x75);//Set row address
 Lcd_send_cmd(0x00);//Row Start Address
 Lcd_send_cmd(0x7f);//Row End Address
 for(i=0;i<128;i++)
 {

     for(j=0;j<64;j++)
     {
         //if(i%2)
         Lcd_send_data(0xFF);
        // else
        // Lcd_send_data(0x00);
     }
 }
}
void Lcd_init(void)
{
    Lcd_io_init();
    Task_sleep(1 * CLOCK_UNIT_MS);
    Lcd_power_ctrl(1);
    Task_sleep(1 * CLOCK_UNIT_MS);
    Lcd_reset();

#if 1
    Lcd_send_cmd(0xAE);    /*display off*/

          Lcd_send_cmd(0xa0);    /*set re-map*/
          Lcd_send_cmd(0x42);     /*上0x42 下0x53*/

          Lcd_send_cmd(0xa1);    /*set display start line*/
          Lcd_send_cmd(0x00);

          Lcd_send_cmd(0xa2);    /*set display offset*/
          Lcd_send_cmd(0x00);    /*上0x60  下0x20*/

          Lcd_send_cmd(0xa4);    /*normal display*/

          Lcd_send_cmd(0xa8);    /*set multiplex ratio*/
          Lcd_send_cmd(0x7f);

          Lcd_send_cmd(0xab);    /*function selection A*/
          Lcd_send_cmd(0x01);    /*enable internal VDD regulator*/

          Lcd_send_cmd(0x81);    /*set contrast*/
          Lcd_send_cmd(0x49);

          Lcd_send_cmd(0xb1);    /*set phase length*/
          Lcd_send_cmd(0x32);

          Lcd_send_cmd(0xb3);    /*set front clock divider/oscillator frequency*/
          Lcd_send_cmd(0x51);

          Lcd_send_cmd(0xb4);    /*For brightness enhancement*/
          Lcd_send_cmd(0xb5);

          Lcd_send_cmd(0xb6);    /*Set Second pre-charge Period*/
          Lcd_send_cmd(0X0d);

          Lcd_send_cmd(0xbc);    /*Set Pre-charge voltage*/
          Lcd_send_cmd(0x07);

          Lcd_send_cmd(0xbe);    /*set vcomh*/
          Lcd_send_cmd(0x07);

          Lcd_send_cmd(0xd5);    /*Function Selection B*/
          Lcd_send_cmd(0x02);    /*Enable second pre-charge*/
          cleanDDR();
          Lcd_send_cmd(0xAF);    /*display ON*/
#else
          Lcd_send_cmd(0xae);//Set display off
          Lcd_send_cmd(0xa0);//Set re-map
          Lcd_send_cmd(0x42);
          Lcd_send_cmd(0xa1);//Set display start line
          Lcd_send_cmd(0x00);
          Lcd_send_cmd(0xa2);//Set display offset
          Lcd_send_cmd(0x00);
          Lcd_send_cmd(0xa4);//Normal Display
          Lcd_send_cmd(0xa8);//Set multiplex ratio
          Lcd_send_cmd(0x7f);//128MUX
          Lcd_send_cmd(0xab);//Function Selection A
          Lcd_send_cmd(0x01);//Enable internal VDD regulator
          Lcd_send_cmd(0x81);//Set contrast
          Lcd_send_cmd(0x77); //normal mode (type brightness)
          Lcd_send_cmd(0xb1);//Set Phase Length
          Lcd_send_cmd(0x31);
          Lcd_send_cmd(0xb3);//Set Front Clock Divider /Oscillator Frequency
          Lcd_send_cmd(0xb1);//105Hz
          Lcd_send_cmd(0xb4); //For brightness enhancement
          Lcd_send_cmd(0xb5);
          Lcd_send_cmd(0xb6);//Set Second pre-charge Period
          Lcd_send_cmd(0x0d);
          Lcd_send_cmd(0xbc);//Set Pre-charge voltage
          Lcd_send_cmd(0x07);
          Lcd_send_cmd(0xbe);//Set VCOMH
          Lcd_send_cmd(0x07);
          Lcd_send_cmd(0xd5);//Function Selection B
          Lcd_send_cmd(0x02);//Enable second pre-charge
          cleanDDR();// clear the whole DDRAM
          Lcd_send_cmd(0xaf);//Display on
#endif
//	  ALLDDR();
//      cleanDDR();
}
//***********************************************************************************
//
// LCD set font parameters, Font Cols, Font Pages = Rows / 8, display inverse.
//
//***********************************************************************************
static uint8_t retpChar[256] = {0};
static uint8_t W_th = 0,H_th= 0;
static uint8_t inXstart = 0,inYstart = 0;
void Lcd_set_font(uint8_t bFontCols, uint8_t bFontRows, uint8_t fgInverse)
{
    W_th = bFontCols;
    H_th = bFontRows;
    bLcdFontPages = (bFontRows + 7) / 8;
    bLcdFontCols = bFontCols;
    fgLcdFontInverse = fgInverse;
}
//***********************************************************************************
//
// LCD write character.
//
//***********************************************************************************

uint8_t bitChange(uint8_t *byte)
{
  uint8_t ret = 0,i = 8;


      for(i=0;i<8;i++)
      {
          ret=((*byte>>i)&0x01)|ret;
          if(i<7)
              ret=ret<<1;
      }
      return ret;
}
void buffcharChange(uint8_t *pChar,uint8_t w,int8_t h,uint8_t xStart,uint8_t yStart)
{
  uint8_t i = 0,j = 0,W_bytes = 0;
  uint8_t bytefornt = 0,byteEnd = 0;
  uint8_t temp = 0;
  memset(retpChar,0x00,256);
  W_bytes = w/8;

  if(w%8 != 0)
  {

      W_bytes++;

  }
  //else
  {
      for(i = 0 ; i < h;i++)
      {
          for(j = 0 ; j <W_bytes;j++)
          {
              retpChar[(W_bytes)*i + j] = *(pChar+((h-i-1)*(W_bytes)+j));
          }
      }
  }
  for(i = 0 ; i < h ; i++)
  {
      if(W_bytes%2 != 0)
      {
          bytefornt = retpChar[i*W_bytes + W_bytes/2];
          temp = bitChange(&bytefornt);
          retpChar[i*W_bytes + W_bytes/2] = temp;
      }
     for(j = 0 ; j < W_bytes/2;j++)
     {
         bytefornt = retpChar[i*W_bytes + j];
         byteEnd   =  retpChar[i*W_bytes + (W_bytes-j-1)];
         temp = bitChange(&byteEnd);
         retpChar[i*W_bytes + j] =temp;
         temp = bitChange(&bytefornt);
         retpChar[i*W_bytes + (W_bytes-j-1)] = temp;
     }


  }

  //inXstart = 127 - W_th*(xStart+1);
  //inYstart = 127 - (H_th*(yStart+1));

}
void Lcd_write_character(uint8_t xStart, uint8_t yStart, const uint8_t *pChar)
{
#ifdef LCD_ST7567A
    uint8_t page, col,nextrow;
    uint8_t nextCharFlag,dotValue_;
    uint8_t *tempChar;
    // Offset from left edge of display.
    xStart += LCD_START_COL;

    // Add the command to page address.
    //bLowCol = xStart & 0x0f;
    //bHiCol = (xStart >> 4) & 0x0f;

#if 0
    for (page = 0; page < bLcdFontPages; page++) {
        //Lcd_send_cmd(LCD_CMD_PSA + yStart);			// set the page start address
        //Lcd_send_cmd(LCD_CMD_LOWER_SCA + bLowCol);    // set the lower start column address
        //Lcd_send_cmd(LCD_CMD_UPPER_SCA + bHiCol);		// set the upper start column address
        Lcd_send_cmd(0x15);//Set column address
        Lcd_send_cmd(xStart);//Column Start Address
        Lcd_send_cmd(xStart+bLcdFontCols);//Column End Address
        Lcd_send_cmd(0x75);//Set row address
        Lcd_send_cmd(bLcdFontPages*yStart*8);//Row Start Address
        Lcd_send_cmd(bLcdFontPages*yStart*8 + page*8);//Row End Address

        for (col = 0; col < bLcdFontCols; col++) {
            // check inverse flag bit.
            if (fgLcdFontInverse)
            {
                Lcd_send_data(*pChar ^ 0xff);
            }
            else
                Lcd_send_data(*pChar);
            pChar++;
        }
        yStart++;
    }
#else
    //ALLDDR();
    //Lcd_clear_screen();
    inXstart =  (128-((xStart+1)*W_th))/2;
    inYstart = yStart;
    //inXstart = 0;
    buffcharChange(pChar,W_th,H_th,xStart,yStart);

    //inXstart = 128-(W_th+xStart*W_th);
    nextCharFlag = 0;
    nextrow =128-(bLcdFontPages*(inYstart+1)*8)-H_th;


    Lcd_send_cmd(0x15);//Set column address
    Lcd_send_cmd(inXstart);//Column Start Address
    Lcd_send_cmd(inXstart+bLcdFontCols/2-1);//Column End Address

    Lcd_send_cmd(0x75);//Set row address
    Lcd_send_cmd(nextrow);//Row Start Address
    Lcd_send_cmd(nextrow+bLcdFontPages*8-1);//Row End Address
    tempChar = retpChar;
    for (page = 0; page < bLcdFontPages*8; page++)
    {

        for (col = 0; col < bLcdFontCols/2; col++)
        {
            // check inverse flag bit.
            if (fgLcdFontInverse)
            {
                if((*tempChar)&(1<<nextCharFlag))
                {
                    dotValue_ = 0xf0;
                }
                else
                {
                    dotValue_ = 0x00;
                }

                if((*tempChar)&(1<<(nextCharFlag+1)))
                {
                    dotValue_ |= 0x0f;
                }
                else
                {
                    dotValue_ |= 0x00;
                }

                //Lcd_send_data(dotValue_);
                Lcd_send_data(dotValue_);
            }
            else
                Lcd_send_data(0x00);

            nextCharFlag+=2;
            if(nextCharFlag==8)
            {
                nextCharFlag=0;
                tempChar = tempChar +1;
            }

        }
        if((bLcdFontCols%8)!=0)
        {
            nextCharFlag=0;
            tempChar++;
        }
    }
#endif

#endif

#ifdef EPD_GDE0213B1
    EPD_writeCharacter(yStart, yStart + bLcdFontPages, xStart, xStart + bLcdFontCols, pChar);
#endif

#ifdef OLED_LX12864K1
    uint8_t bLowCol, bHiCol;
    uint8_t page, col;

    // Offset from left edge of display.
    xStart += LCD_START_COL;

    // Add the command to page address.
    bLowCol = xStart & 0x0f;
    bHiCol = (xStart >> 4) & 0x0f;

    for (page = 0; page < bLcdFontPages; page++) {
        Lcd_send_cmd(LCD_CMD_PSA + oledComTable[yStart & 0x07]);         // set the page start address
        Lcd_send_cmd(0x01 + bLowCol);      // set the lower start column address
        Lcd_send_cmd(LCD_CMD_UPPER_SCA + bHiCol);       // set the upper start column address

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

    for (page = 0; page < bLcdFontPages*8; page++) {

        Lcd_send_cmd(0x75);//Set row address
        Lcd_send_cmd(bLcdFontPages*yStart*8+page);//Row Start Address
        Lcd_send_cmd(bLcdFontPages*yStart*8+page);//Row End Address

        for (col = 0; col < bLcdFontCols; col++)
        {

            // check inverse flag bit.
            Lcd_send_cmd(0x15);//Set column address
            Lcd_send_cmd(xStart+col);//Column Start Address
            Lcd_send_cmd(xStart+bLcdFontCols);//Column End Address

            Lcd_send_data(0x00);


        }
        yStart++;
    }
#endif

#ifdef EPD_GDE0213B1
    EPD_clearArea(yStart, yStart + bLcdFontPages, xStart, xStart + bLcdFontCols);
#endif

#ifdef OLED_LX12864K1
    uint8_t bLowCol, bHiCol;
    uint8_t page, col;

    // Offset from left edge of display.
    xStart += LCD_START_COL;

    // Add the command to page address.
    bLowCol = xStart & 0x0f;
    bHiCol = (xStart >> 4) & 0x0f;

    for (page = 0; page < bLcdFontPages; page++) {
        Lcd_send_cmd(LCD_CMD_PSA + oledComTable[yStart & 0x07]);         // set the page start address
        Lcd_send_cmd(LCD_CMD_LOWER_SCA + bLowCol);      // set the lower start column address
        Lcd_send_cmd(LCD_CMD_UPPER_SCA + bHiCol);       // set the upper start column address

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
    uint8_t i,j;
    Lcd_send_cmd(0x15);//Set column address
    Lcd_send_cmd(0x00);//Column Start Address
    Lcd_send_cmd(0x7f);//Column End Address
    Lcd_send_cmd(0x75);//Set row address
    Lcd_send_cmd(0x00);//Row Start Address
    Lcd_send_cmd(0x6f);//Row End Address
    for(i=0;i<128;i++)
    {
        for(j=0;j<64;j++)
        {
            //if(i%2)
            Lcd_send_data(0x00);
           // else
           // Lcd_send_data(0x00);
        }
    }
    display_star_mssage();
#endif

#ifdef EPD_GDE0213B1
    EPD_clearScreen();
#endif

#ifdef OLED_LX12864K1
    uint8_t page, col;

    for(page = 0; page < LCD_MAX_PAGE; page++) {
        Lcd_send_cmd(LCD_CMD_PSA + oledComTable[page& 0x07]);                   // set the page start address
        Lcd_send_cmd(LCD_CMD_LOWER_SCA + LCD_START_COL);    // set the lower start column address
        Lcd_send_cmd(LCD_CMD_UPPER_SCA);                    // set the upper start column address
        for(col = 0; col < LCD_TOTAL_COL; col++)
            Lcd_send_data(0x00);
    }
#endif  // OLED_LX12864K1
}
void All_Screen(void)
{
     uint8_t i,j;
     Lcd_send_cmd(0x15);//Set column address
     Lcd_send_cmd(0x00);//Column Start Address
     Lcd_send_cmd(0x7f);//Column End Address
     Lcd_send_cmd(0x75);//Set row address
     Lcd_send_cmd(0x00);//Row Start Address
     Lcd_send_cmd(0x7f);//Row End Address
     for(i=0;i<128;i++)
     {

         for(j=0;j<64;j++)
         {
             //if(i%2)
             Lcd_send_data(0xff);
            // else
            // Lcd_send_data(0x00);
         }
     }
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

void display_char_A()
{


}
#endif  /* SUPPORT_DISP_SCREEN */

