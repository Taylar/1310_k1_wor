//***********************************************************************************
// Copyright 2017, Zksiot Development Ltd.
// Created by Linjie, 2017.08.08
// MCU:	MSP430F5529
// OS: TI-RTOS
// Project:
// File name: spi_drv.c
// Description: spi process routine.
//***********************************************************************************
#include "../general.h"

/*
 *  =============================== SPI board pin define ===============================
 */





/*
 *  =============================== SPI DMA ===============================
 */
#include <ti/drivers/SPI.h>
#include <ti/drivers/spi/SPICC26XXDMA.h>

SPICC26XXDMA_Object spiCC26XXDMAObjects[CC1310_LAUNCHXL_SPICOUNT];

#ifdef BOARD_S1_2

/* SPI Board */
#define CC1310_LAUNCHXL_SPI0_MISO_NODE             IOID_10
#define CC1310_LAUNCHXL_SPI0_MOSI_NODE             IOID_7
#define CC1310_LAUNCHXL_SPI0_CLK_NODE              IOID_6
#define CC1310_LAUNCHXL_SPI0_CSN_NODE              PIN_UNASSIGNED
#define CC1310_LAUNCHXL_SPI1_MISO_NODE             PIN_UNASSIGNED
#define CC1310_LAUNCHXL_SPI1_MOSI_NODE             PIN_UNASSIGNED
#define CC1310_LAUNCHXL_SPI1_CLK_NODE              PIN_UNASSIGNED
#define CC1310_LAUNCHXL_SPI1_CSN_NODE              PIN_UNASSIGNED

const SPICC26XXDMA_HWAttrsV1 spiCC26XXDMAHWAttrs[CC1310_LAUNCHXL_SPICOUNT] = {
    {
        .baseAddr           = SSI0_BASE,
        .intNum             = INT_SSI0_COMB,
        .intPriority        = ~0,
        .swiPriority        = 0,
        .powerMngrId        = PowerCC26XX_PERIPH_SSI0,
        .defaultTxBufValue  = 0,
        .rxChannelBitMask   = 1<<UDMA_CHAN_SSI0_RX,
        .txChannelBitMask   = 1<<UDMA_CHAN_SSI0_TX,
        .mosiPin            = CC1310_LAUNCHXL_SPI0_MOSI_NODE,
        .misoPin            = CC1310_LAUNCHXL_SPI0_MISO_NODE,
        .clkPin             = CC1310_LAUNCHXL_SPI0_CLK_NODE,
        .csnPin             = CC1310_LAUNCHXL_SPI0_CSN_NODE
    },
    {
        .baseAddr           = SSI1_BASE,
        .intNum             = INT_SSI1_COMB,
        .intPriority        = ~0,
        .swiPriority        = 0,
        .powerMngrId        = PowerCC26XX_PERIPH_SSI1,
        .defaultTxBufValue  = 0,
        .rxChannelBitMask   = 1<<UDMA_CHAN_SSI1_RX,
        .txChannelBitMask   = 1<<UDMA_CHAN_SSI1_TX,
        .mosiPin            = CC1310_LAUNCHXL_SPI1_MOSI_NODE,
        .misoPin            = CC1310_LAUNCHXL_SPI1_MISO_NODE,
        .clkPin             = CC1310_LAUNCHXL_SPI1_CLK_NODE,
        .csnPin             = CC1310_LAUNCHXL_SPI1_CSN_NODE
    }
};
#endif



#if (defined BOARD_S2_2) || (defined BOARD_S6_6)


#define CC1310_LAUNCHXL_SPI0_MISO_GATEWAY             IOID_25
#define CC1310_LAUNCHXL_SPI0_MOSI_GATEWAY             IOID_27
#define CC1310_LAUNCHXL_SPI0_CLK_GATEWAY              IOID_26
#define CC1310_LAUNCHXL_SPI0_CSN_GATEWAY              PIN_UNASSIGNED
#define CC1310_LAUNCHXL_SPI1_MISO_GATEWAY             PIN_UNASSIGNED
#define CC1310_LAUNCHXL_SPI1_MOSI_GATEWAY             PIN_UNASSIGNED
#define CC1310_LAUNCHXL_SPI1_CLK_GATEWAY              PIN_UNASSIGNED
#define CC1310_LAUNCHXL_SPI1_CSN_GATEWAY              PIN_UNASSIGNED


const SPICC26XXDMA_HWAttrsV1 spiCC26XXDMAHWAttrs[CC1310_LAUNCHXL_SPICOUNT] = {
    {
        .baseAddr           = SSI0_BASE,
        .intNum             = INT_SSI0_COMB,
        .intPriority        = ~0,
        .swiPriority        = 0,
        .powerMngrId        = PowerCC26XX_PERIPH_SSI0,
        .defaultTxBufValue  = 0,
        .rxChannelBitMask   = 1<<UDMA_CHAN_SSI0_RX,
        .txChannelBitMask   = 1<<UDMA_CHAN_SSI0_TX,
        .mosiPin            = CC1310_LAUNCHXL_SPI0_MOSI_GATEWAY,
        .misoPin            = CC1310_LAUNCHXL_SPI0_MISO_GATEWAY,
        .clkPin             = CC1310_LAUNCHXL_SPI0_CLK_GATEWAY,
        .csnPin             = CC1310_LAUNCHXL_SPI0_CSN_GATEWAY
    },
    {
        .baseAddr           = SSI1_BASE,
        .intNum             = INT_SSI1_COMB,
        .intPriority        = ~0,
        .swiPriority        = 0,
        .powerMngrId        = PowerCC26XX_PERIPH_SSI1,
        .defaultTxBufValue  = 0,
        .rxChannelBitMask   = 1<<UDMA_CHAN_SSI1_RX,
        .txChannelBitMask   = 1<<UDMA_CHAN_SSI1_TX,
        .mosiPin            = CC1310_LAUNCHXL_SPI1_MOSI_GATEWAY,
        .misoPin            = CC1310_LAUNCHXL_SPI1_MISO_GATEWAY,
        .clkPin             = CC1310_LAUNCHXL_SPI1_CLK_GATEWAY,
        .csnPin             = CC1310_LAUNCHXL_SPI1_CSN_GATEWAY
    }
};
#endif


const SPI_Config SPI_config[CC1310_LAUNCHXL_SPICOUNT] = {
    {
         .fxnTablePtr = &SPICC26XXDMA_fxnTable,
         .object      = &spiCC26XXDMAObjects[CC1310_LAUNCHXL_SPI0],
         .hwAttrs     = &spiCC26XXDMAHWAttrs[CC1310_LAUNCHXL_SPI0]
    },
    {
         .fxnTablePtr = &SPICC26XXDMA_fxnTable,
         .object      = &spiCC26XXDMAObjects[CC1310_LAUNCHXL_SPI1],
         .hwAttrs     = &spiCC26XXDMAHWAttrs[CC1310_LAUNCHXL_SPI1]
    },
};

const uint_least8_t SPI_count = CC1310_LAUNCHXL_SPICOUNT;


Semaphore_Struct spiSemStruct;
Semaphore_Handle spiSemHandle;


//Master spi handle.
SPI_Handle mSpiHandle;

//***********************************************************************************
//
// SPI master init.
//
//***********************************************************************************
ErrorStatus Spi_init(void)
{
    SPI_Params mSpiParams;


    SPI_init();

    SPI_Params_init(&mSpiParams);
    mSpiParams.mode = SPI_MASTER;
    mSpiParams.bitRate = SPI_4MHz;
    mSpiHandle = SPI_open(CC1310_LAUNCHXL_SPI0, &mSpiParams);
    if (mSpiHandle == NULL) {
        System_printf("SPI Master was not opened\n");
        return ES_ERROR;
    }

    Semaphore_Params semParams;
    Semaphore_Params_init(&semParams);
    Semaphore_construct(&spiSemStruct, 1, &semParams);
    spiSemHandle = Semaphore_handle(&spiSemStruct);

    return ES_SUCCESS;
}

//***********************************************************************************
//
// SPI master transmit a byte data.
//
//***********************************************************************************
ErrorStatus Spi_write(uint8_t *pData, uint16_t length)
{
    SPI_Transaction mSpiTransaction;

    mSpiTransaction.count = length;
    mSpiTransaction.txBuf = pData;
    mSpiTransaction.rxBuf = NULL;
    if (!SPI_transfer(mSpiHandle, &mSpiTransaction)) {
        System_printf("Bad SPI master write transfer!\n");
        return ES_ERROR;
    }

    return ES_SUCCESS;
}

//***********************************************************************************
//
// SPI master receive a byte data.
//
//***********************************************************************************
ErrorStatus Spi_read(uint8_t *pData, uint16_t length)
{
    SPI_Transaction mSpiTransaction;

    mSpiTransaction.count = length;
    mSpiTransaction.txBuf = NULL;
    mSpiTransaction.rxBuf = pData;
    if (!SPI_transfer(mSpiHandle, &mSpiTransaction)) {
        System_printf("Bad SPI master read transfer!\n");
        return ES_ERROR;
    }

    return ES_SUCCESS;
}

//***********************************************************************************
//
// SPI master receive a byte data.
//
//***********************************************************************************
ErrorStatus Spi_writeRead(uint8_t *pTxBuff, uint8_t *pRxBuff, uint16_t length)
{
    SPI_Transaction mSpiTransaction;

    mSpiTransaction.count = length;
    mSpiTransaction.txBuf = pTxBuff;
    mSpiTransaction.rxBuf = pRxBuff;
    if (!SPI_transfer(mSpiHandle, &mSpiTransaction)) {
        System_printf("Bad SPI master read transfer!\n");
        return ES_ERROR;
    }

    return ES_SUCCESS;
}

