//***********************************************************************************
// Copyright 2017, Zksiot Development Ltd.
// Created by Linjie, 2017.08.08
// MCU:	MSP430F5529
// OS: TI-RTOS
// Project:
// File name: spi_drv.h
// Description: spi process routine head.
//***********************************************************************************

#ifndef __ZKSIOT_SPI_DRV_H__
#define __ZKSIOT_SPI_DRV_H__

/*!
 *  @def    CC1310_LAUNCHXL_SPIName
 *  @brief  Enum of SPI names
 */
typedef enum CC1310_LAUNCHXL_SPIName {
    CC1310_LAUNCHXL_SPI0 = 0,
    CC1310_LAUNCHXL_SPI1,

    CC1310_LAUNCHXL_SPICOUNT
} CC1310_LAUNCHXL_SPIName;


extern Semaphore_Handle spiSemHandle;


#define SPI_4MHz        4000000

extern ErrorStatus Spi_init(void);
extern ErrorStatus Spi_write(uint8_t *pData, uint16_t length);
extern ErrorStatus Spi_read(uint8_t *pData, uint16_t length);
extern ErrorStatus Spi_writeRead(uint8_t *pTxBuff, uint8_t *pRxBuff, uint16_t length);

#endif	/* __ZKSIOT_SPI_DRV_H__ */

