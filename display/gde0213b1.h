//***********************************************************************************
// Copyright 2017, Zksiot Development Ltd.
// Created by Linjie, 2017.05.17
// MCU:	MSP430F55xx
// Project:
// File name: gde0213b1.h
// Description: gde0213b1 process routine head.
//***********************************************************************************

#ifndef __ZKSIOT_GDE0213B1_H__
#define __ZKSIOT_GDE0213B1_H__

#ifdef EPD_GDE0213B1

#define EPD_SPI_HW_PORT                 PJOUT
#define EPD_SPI_PORT                    GPIO_PORT_PJ
#define EPD_SPI_SIMO_PIN                GPIO_PIN0
#define EPD_SPI_CLK_PIN                 GPIO_PIN1
#define EPD_SPI_CS_PIN                  GPIO_PIN2
#define EPD_SPI_DC_PIN                  GPIO_PIN3
#define EPD_RESET_PORT                  GPIO_PORT_P6
#define EPD_RESET_PIN                   GPIO_PIN0
#define EPD_BUSY_PORT                   GPIO_PORT_P6
#define EPD_BUSY_PIN                    GPIO_PIN1

//Use software spi
#if 0
#define EPD_spiSimoCtrl(on)         (on) ? (GPIO_setOutputHighOnPin(GPIO_PORT_PJ, GPIO_PIN0)) : (GPIO_setOutputLowOnPin(GPIO_PORT_PJ, GPIO_PIN0))
#define EPD_spiClkCtrl(on)          (on) ? (GPIO_setOutputHighOnPin(GPIO_PORT_PJ, GPIO_PIN1)) : (GPIO_setOutputLowOnPin(GPIO_PORT_PJ, GPIO_PIN1))
#define EPD_spiSteCtrl(on)          (on) ? (GPIO_setOutputHighOnPin(GPIO_PORT_PJ, GPIO_PIN2)) : (GPIO_setOutputLowOnPin(GPIO_PORT_PJ, GPIO_PIN2))
#define EPD_spiDcCtrl(on)           (on) ? (GPIO_setOutputHighOnPin(GPIO_PORT_PJ, GPIO_PIN3)) : (GPIO_setOutputLowOnPin(GPIO_PORT_PJ, GPIO_PIN3))
#else
#define EPD_spiSimoCtrl(on)         (on) ? (EPD_SPI_HW_PORT |= EPD_SPI_SIMO_PIN) : (EPD_SPI_HW_PORT &= ~EPD_SPI_SIMO_PIN)
#define EPD_spiClkCtrl(on)          (on) ? (EPD_SPI_HW_PORT |= EPD_SPI_CLK_PIN) : (EPD_SPI_HW_PORT &= ~EPD_SPI_CLK_PIN)
#define EPD_spiSteCtrl(on)          (on) ? (EPD_SPI_HW_PORT |= EPD_SPI_CS_PIN) : (EPD_SPI_HW_PORT &= ~EPD_SPI_CS_PIN)
#define EPD_spiDcCtrl(on)           (on) ? (EPD_SPI_HW_PORT |= EPD_SPI_DC_PIN) : (EPD_SPI_HW_PORT &= ~EPD_SPI_DC_PIN)
#endif

#define EPD_resetCtrl(on)           (!on) ? (GPIO_setOutputHighOnPin(EPD_RESET_PORT, EPD_RESET_PIN)) : (GPIO_setOutputLowOnPin(EPD_RESET_PORT, EPD_RESET_PIN))
#define EPD_isBusy()                GPIO_getInputPinValue(EPD_BUSY_PORT, GPIO_PIN1)

#define EPD_ioInit()            { \
                                    GPIO_setOutputHighOnPin(EPD_SPI_PORT, EPD_SPI_SIMO_PIN + EPD_SPI_CLK_PIN + EPD_SPI_CS_PIN + EPD_SPI_DC_PIN); \
                                    GPIO_setAsOutputPin(EPD_SPI_PORT, EPD_SPI_SIMO_PIN + EPD_SPI_CLK_PIN + EPD_SPI_CS_PIN + EPD_SPI_DC_PIN); \
                                    GPIO_setOutputLowOnPin(EPD_RESET_PORT, EPD_RESET_PIN); \
                                    GPIO_setAsOutputPin(EPD_RESET_PORT, EPD_RESET_PIN); \
                                    GPIO_setAsInputPin(EPD_BUSY_PORT, EPD_BUSY_PIN); \
                                }


extern void EPD_update(uint8_t isFull);
extern void EPD_writeCharacter(uint8_t xStart, uint8_t xEnd, uint8_t yStart, uint8_t yEnd, const uint8_t *pChar);
extern void EPD_clearArea(uint8_t xStart, uint8_t xEnd, uint8_t yStart, uint8_t yEnd);
extern void EPD_clearScreen(void);
extern void EPD_init(uint8_t isFull);

#endif  /* EPD_GDE0213B1 */

#endif	/* __ZKSIOT_GDE0213B1_H__ */

