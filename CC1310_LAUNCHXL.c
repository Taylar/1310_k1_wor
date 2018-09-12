/*
 * Copyright (c) 2015-2017, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ====================== CC1310_LAUNCHXL.c ===================================
 *  This file is responsible for setting up the board specific items for the
 *  CC1310_LAUNCHXL board.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <ti/devices/cc13x0/driverlib/ioc.h>
#include <ti/devices/cc13x0/driverlib/udma.h>
#include <ti/devices/cc13x0/inc/hw_ints.h>
#include <ti/devices/cc13x0/inc/hw_memmap.h>

#include "CC1310_LAUNCHXL.h"

/*
 *  =============================== ADCBuf ===============================
 */
#include <ti/drivers/ADCBuf.h>
#include <ti/drivers/adcbuf/ADCBufCC26XX.h>

ADCBufCC26XX_Object adcBufCC26xxObjects[CC1310_LAUNCHXL_ADCBUFCOUNT];

/*
 *  This table converts a virtual adc channel into a dio and internal analogue
 *  input signal. This table is necessary for the functioning of the adcBuf
 *  driver. Comment out unused entries to save flash. Dio and internal signal
 *  pairs are hardwired. Do not remap them in the table. You may reorder entire
 *  entries. The mapping of dio and internal signals is package dependent.
 */
const ADCBufCC26XX_AdcChannelLutEntry ADCBufCC26XX_adcChannelLut[CC1310_LAUNCHXL_ADCBUF0CHANNELCOUNT] = {
    {CC1310_LAUNCHXL_DIO23_ANALOG, ADC_COMPB_IN_AUXIO7},
    {CC1310_LAUNCHXL_DIO24_ANALOG, ADC_COMPB_IN_AUXIO6},
    {CC1310_LAUNCHXL_DIO25_ANALOG, ADC_COMPB_IN_AUXIO5},
    {CC1310_LAUNCHXL_DIO26_ANALOG, ADC_COMPB_IN_AUXIO4},
    {CC1310_LAUNCHXL_DIO27_ANALOG, ADC_COMPB_IN_AUXIO3},
    {CC1310_LAUNCHXL_DIO28_ANALOG, ADC_COMPB_IN_AUXIO2},
    {CC1310_LAUNCHXL_DIO29_ANALOG, ADC_COMPB_IN_AUXIO1},
    {CC1310_LAUNCHXL_DIO30_ANALOG, ADC_COMPB_IN_AUXIO0},
    {PIN_UNASSIGNED, ADC_COMPB_IN_VDDS},
    {PIN_UNASSIGNED, ADC_COMPB_IN_DCOUPL},
    {PIN_UNASSIGNED, ADC_COMPB_IN_VSS},
};

const ADCBufCC26XX_HWAttrs adcBufCC26xxHWAttrs[CC1310_LAUNCHXL_ADCBUFCOUNT] = {
    {
        .intPriority       = ~0,
        .swiPriority       = 0,
        .adcChannelLut     = ADCBufCC26XX_adcChannelLut,
        .gpTimerUnit       = CC1310_LAUNCHXL_GPTIMER0A,
        .gptDMAChannelMask = 1 << UDMA_CHAN_TIMER0_A,
    }
};

const ADCBuf_Config ADCBuf_config[CC1310_LAUNCHXL_ADCBUFCOUNT] = {
    {
        &ADCBufCC26XX_fxnTable,
        &adcBufCC26xxObjects[CC1310_LAUNCHXL_ADCBUF0],
        &adcBufCC26xxHWAttrs[CC1310_LAUNCHXL_ADCBUF0]
    },
};

const uint_least8_t ADCBuf_count = CC1310_LAUNCHXL_ADCBUFCOUNT;

/*
 *  =============================== ADC ===============================
 */
#include <ti/drivers/ADC.h>
#include <ti/drivers/adc/ADCCC26XX.h>


/*
 *  =============================== Crypto ===============================
 */
#include <ti/drivers/crypto/CryptoCC26XX.h>

CryptoCC26XX_Object cryptoCC26XXObjects[CC1310_LAUNCHXL_CRYPTOCOUNT];

const CryptoCC26XX_HWAttrs cryptoCC26XXHWAttrs[CC1310_LAUNCHXL_CRYPTOCOUNT] = {
    {
        .baseAddr       = CRYPTO_BASE,
        .powerMngrId    = PowerCC26XX_PERIPH_CRYPTO,
        .intNum         = INT_CRYPTO_RESULT_AVAIL_IRQ,
        .intPriority    = ~0,
    }
};

const CryptoCC26XX_Config CryptoCC26XX_config[CC1310_LAUNCHXL_CRYPTOCOUNT] = {
    {
         .object  = &cryptoCC26XXObjects[CC1310_LAUNCHXL_CRYPTO0],
         .hwAttrs = &cryptoCC26XXHWAttrs[CC1310_LAUNCHXL_CRYPTO0]
    }
};


/*
 *  =============================== GPIO ===============================
 */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/gpio/GPIOCC26XX.h>

/*
 * Array of Pin configurations
 * NOTE: The order of the pin configurations must coincide with what was
 *       defined in CC1310_LAUNCHXL.h
 * NOTE: Pins not used for interrupts should be placed at the end of the
 *       array. Callback entries can be omitted from callbacks array to
 *       reduce memory usage.
 */
GPIO_PinConfig gpioPinConfigs[] = {
    /* Input pins */
//    GPIOCC26XX_DIO_13 | GPIO_CFG_IN_PU | GPIO_CFG_IN_INT_RISING,  /* Button 0 */
//    GPIOCC26XX_DIO_14 | GPIO_CFG_IN_PU | GPIO_CFG_IN_INT_RISING,  /* Button 1 */
//
//    /* Output pins */
//    GPIOCC26XX_DIO_07 | GPIO_CFG_OUT_STD | GPIO_CFG_OUT_STR_HIGH | GPIO_CFG_OUT_LOW,  /* Green LED */
//    GPIOCC26XX_DIO_06 | GPIO_CFG_OUT_STD | GPIO_CFG_OUT_STR_HIGH | GPIO_CFG_OUT_LOW,  /* Red LED */
//
//    /* SPI Flash CSN */
//    GPIOCC26XX_DIO_20 | GPIO_CFG_OUT_STD | GPIO_CFG_OUT_STR_HIGH | GPIO_CFG_OUT_HIGH,
};

/*
 * Array of callback function pointers
 * NOTE: The order of the pin configurations must coincide with what was
 *       defined in CC1310_LAUNCH.h
 * NOTE: Pins not used for interrupts can be omitted from callbacks array to
 *       reduce memory usage (if placed at end of gpioPinConfigs array).
 */
GPIO_CallbackFxn gpioCallbackFunctions[] = {
    NULL,  /* Button 0 */
    NULL,  /* Button 1 */
};

const GPIOCC26XX_Config GPIOCC26XX_config = {
    .pinConfigs = (GPIO_PinConfig *)gpioPinConfigs,
    .callbacks = (GPIO_CallbackFxn *)gpioCallbackFunctions,
    .numberOfPinConfigs = CC1310_LAUNCHXL_GPIOCOUNT,
    .numberOfCallbacks  = sizeof(gpioCallbackFunctions)/sizeof(GPIO_CallbackFxn),
    .intPriority = (~0)
};

/*
 *  =============================== GPTimer ===============================
 *  Remove unused entries to reduce flash usage both in Board.c and Board.h
 */
#include <ti/drivers/timer/GPTimerCC26XX.h>

GPTimerCC26XX_Object gptimerCC26XXObjects[CC1310_LAUNCHXL_GPTIMERCOUNT];

const GPTimerCC26XX_HWAttrs gptimerCC26xxHWAttrs[CC1310_LAUNCHXL_GPTIMERPARTSCOUNT] = {
    { .baseAddr = GPT0_BASE, .intNum = INT_GPT0A, .intPriority = (~0), .powerMngrId = PowerCC26XX_PERIPH_GPT0, .pinMux = GPT_PIN_0A, },
    { .baseAddr = GPT0_BASE, .intNum = INT_GPT0B, .intPriority = (~0), .powerMngrId = PowerCC26XX_PERIPH_GPT0, .pinMux = GPT_PIN_0B, },
    { .baseAddr = GPT1_BASE, .intNum = INT_GPT1A, .intPriority = (~0), .powerMngrId = PowerCC26XX_PERIPH_GPT1, .pinMux = GPT_PIN_1A, },
    { .baseAddr = GPT1_BASE, .intNum = INT_GPT1B, .intPriority = (~0), .powerMngrId = PowerCC26XX_PERIPH_GPT1, .pinMux = GPT_PIN_1B, },
    { .baseAddr = GPT2_BASE, .intNum = INT_GPT2A, .intPriority = (~0), .powerMngrId = PowerCC26XX_PERIPH_GPT2, .pinMux = GPT_PIN_2A, },
    { .baseAddr = GPT2_BASE, .intNum = INT_GPT2B, .intPriority = (~0), .powerMngrId = PowerCC26XX_PERIPH_GPT2, .pinMux = GPT_PIN_2B, },
    { .baseAddr = GPT3_BASE, .intNum = INT_GPT3A, .intPriority = (~0), .powerMngrId = PowerCC26XX_PERIPH_GPT3, .pinMux = GPT_PIN_3A, },
    { .baseAddr = GPT3_BASE, .intNum = INT_GPT3B, .intPriority = (~0), .powerMngrId = PowerCC26XX_PERIPH_GPT3, .pinMux = GPT_PIN_3B, },
};

const GPTimerCC26XX_Config GPTimerCC26XX_config[CC1310_LAUNCHXL_GPTIMERPARTSCOUNT] = {
    { &gptimerCC26XXObjects[CC1310_LAUNCHXL_GPTIMER0], &gptimerCC26xxHWAttrs[CC1310_LAUNCHXL_GPTIMER0A], GPT_A },
    { &gptimerCC26XXObjects[CC1310_LAUNCHXL_GPTIMER0], &gptimerCC26xxHWAttrs[CC1310_LAUNCHXL_GPTIMER0B], GPT_B },
    { &gptimerCC26XXObjects[CC1310_LAUNCHXL_GPTIMER1], &gptimerCC26xxHWAttrs[CC1310_LAUNCHXL_GPTIMER1A], GPT_A },
    { &gptimerCC26XXObjects[CC1310_LAUNCHXL_GPTIMER1], &gptimerCC26xxHWAttrs[CC1310_LAUNCHXL_GPTIMER1B], GPT_B },
    { &gptimerCC26XXObjects[CC1310_LAUNCHXL_GPTIMER2], &gptimerCC26xxHWAttrs[CC1310_LAUNCHXL_GPTIMER2A], GPT_A },
    { &gptimerCC26XXObjects[CC1310_LAUNCHXL_GPTIMER2], &gptimerCC26xxHWAttrs[CC1310_LAUNCHXL_GPTIMER2B], GPT_B },
    { &gptimerCC26XXObjects[CC1310_LAUNCHXL_GPTIMER3], &gptimerCC26xxHWAttrs[CC1310_LAUNCHXL_GPTIMER3A], GPT_A },
    { &gptimerCC26XXObjects[CC1310_LAUNCHXL_GPTIMER3], &gptimerCC26xxHWAttrs[CC1310_LAUNCHXL_GPTIMER3B], GPT_B },
};

/*
 *  =============================== NVS ===============================
 */
#include <ti/drivers/NVS.h>
#include <ti/drivers/nvs/NVSSPI25X.h>
#include <ti/drivers/nvs/NVSCC26XX.h>

#define NVS_REGIONS_BASE 0x1A000
#define SECTORSIZE       0x1000
#define REGIONSIZE       (SECTORSIZE * 4)

#ifndef Board_EXCLUDE_NVS_INTERNAL_FLASH

/*
 * Reserve flash sectors for NVS driver use by placing an uninitialized byte
 * array at the desired flash address.
 */
#if defined(__TI_COMPILER_VERSION__)

/*
 * Place uninitialized array at NVS_REGIONS_BASE
 */
#pragma LOCATION(flashBuf, NVS_REGIONS_BASE);
#pragma NOINIT(flashBuf);
static char flashBuf[REGIONSIZE];

#elif defined(__IAR_SYSTEMS_ICC__)

/*
 * Place uninitialized array at NVS_REGIONS_BASE
 */
static __no_init char flashBuf[REGIONSIZE] @ NVS_REGIONS_BASE;

#elif defined(__GNUC__)

/*
 * Place the reserved flash buffers in the .nvs section.
 * The nvs section will be placed at address NVS_REGIONS_BASE by
 * the gcc linker cmd file.
 */
__attribute__ ((section (".nvs")))
static char flashBuf[REGIONSIZE];

#endif

/* Allocate objects for NVS and NVS SPI */
NVSCC26XX_Object nvsCC26xxObjects[1];
NVSSPI25X_Object nvsSPI25XObjects[1];

/* Hardware attributes for NVS */
const NVSCC26XX_HWAttrs nvsCC26xxHWAttrs[1] = {
    {
        .regionBase = (void *)flashBuf,
        .regionSize = REGIONSIZE,
    },
};

#endif /* Board_EXCLUDE_NVS_INTERNAL_FLASH */

#ifndef Board_EXCLUDE_NVS_EXTERNAL_FLASH

#define SPISECTORSIZE    0x1000
#define SPIREGIONSIZE    (SPISECTORSIZE * 32)
#define VERIFYBUFSIZE    64

static uint8_t verifyBuf[VERIFYBUFSIZE];

/* Allocate objects for NVS External Regions */
NVSSPI25X_Object nvsSPI25XObjects[1];

/* Hardware attributes for NVS External Regions */
const NVSSPI25X_HWAttrs nvsSPI25XHWAttrs[1] = {
    {
        .regionBaseOffset = 0,
        .regionSize = SPIREGIONSIZE,
        .sectorSize = SPISECTORSIZE,
        .verifyBuf = verifyBuf,
        .verifyBufSize = VERIFYBUFSIZE,
        .spiHandle = NULL,
        .spiIndex = 0,
        .spiBitRate = 4000000,
        .spiCsnGpioIndex = CC1310_LAUNCHXL_GPIO_SPI_FLASH_CS,
    },
};

#endif /* Board_EXCLUDE_NVS_EXTERNAL_FLASH */
/* NVS Region index 0 and 1 refer to NVS and NVS SPI respectively */
const NVS_Config NVS_config[CC1310_LAUNCHXL_NVSCOUNT] = {
#ifndef Board_EXCLUDE_NVS_INTERNAL_FLASH
    {
        .fxnTablePtr = &NVSCC26XX_fxnTable,
        .object = &nvsCC26xxObjects[0],
        .hwAttrs = &nvsCC26xxHWAttrs[0],
    },
#endif
#ifndef Board_EXCLUDE_NVS_EXTERNAL_FLASH
    {
        .fxnTablePtr = &NVSSPI25X_fxnTable,
        .object = &nvsSPI25XObjects[0],
        .hwAttrs = &nvsSPI25XHWAttrs[0],
    },
#endif
};

const uint_least8_t NVS_count = CC1310_LAUNCHXL_NVSCOUNT;

/*
 *  =============================== PIN ===============================
 */
#include <ti/drivers/PIN.h>
#include <ti/drivers/pin/PINCC26XX.h>

const PIN_Config BoardGpioInitTable[] = {

    PIN_TERMINATE
};

const PINCC26XX_HWAttrs PINCC26XX_hwAttrs = {
    .intPriority = ~0,
    .swiPriority = 0
};

/*
 *  =============================== Power ===============================
 */
#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26XX.h>

const PowerCC26XX_Config PowerCC26XX_config = {
    .policyInitFxn      = NULL,
    .policyFxn          = &PowerCC26XX_standbyPolicy,
    .calibrateFxn       = &PowerCC26XX_calibrate,
    .enablePolicy       = true,
    .calibrateRCOSC_LF  = true,
    .calibrateRCOSC_HF  = true,
};

/*
 *  =============================== PWM ===============================
 *  Remove unused entries to reduce flash usage both in Board.c and Board.h
 */


/*
 *  =============================== RF Driver ===============================
 */
#include <ti/drivers/rf/RF.h>

const RFCC26XX_HWAttrsV2 RFCC26XX_hwAttrs = {
    .hwiPriority        = ~0,       /* Lowest HWI priority */
    .swiPriority        = 0,        /* Lowest SWI priority */
    .xoscHfAlwaysNeeded = true,     /* Keep XOSC dependency while in stanby */
    .globalCallback     = NULL,     /* No board specific callback */
    .globalEventMask    = 0         /* No events subscribed to */
};



/*
 *  =============================== UDMA ===============================
 */
#include <ti/drivers/dma/UDMACC26XX.h>

UDMACC26XX_Object udmaObjects[CC1310_LAUNCHXL_UDMACOUNT];

const UDMACC26XX_HWAttrs udmaHWAttrs[CC1310_LAUNCHXL_UDMACOUNT] = {
    {
        .baseAddr    = UDMA0_BASE,
        .powerMngrId = PowerCC26XX_PERIPH_UDMA,
        .intNum      = INT_DMA_ERR,
        .intPriority = ~0
    }
};

const UDMACC26XX_Config UDMACC26XX_config[CC1310_LAUNCHXL_UDMACOUNT] = {
    {
         .object  = &udmaObjects[CC1310_LAUNCHXL_UDMA0],
         .hwAttrs = &udmaHWAttrs[CC1310_LAUNCHXL_UDMA0]
    },
};



/*
 *  ======== CC1310_LAUNCHXL_initGeneral ========
 */
void CC1310_LAUNCHXL_initGeneral(void)
{
    Power_init();

    if (PIN_init(BoardGpioInitTable) != PIN_SUCCESS) {
        /* Error with PIN_init */
        while (1);
    }
}

/*
 *  ======== CC1310_LAUNCHXL_sendExtFlashByte ========
 */
void CC1310_LAUNCHXL_sendExtFlashByte(PIN_Handle pinHandle, uint8_t byte)
{
    PIN_setOutputValue(pinHandle, CC1310_LAUNCHXL_SPI_FLASH_CS, 0);
    uint8_t i;
    for(i = 0; i < 8; i++){
        PIN_setOutputValue(pinHandle, CC1310_LAUNCHXL_SPI0_CLK, 0);
        PIN_setOutputValue(pinHandle, CC1310_LAUNCHXL_SPI0_MOSI, (byte >> (7 - i)) & 0x01);
        PIN_setOutputValue(pinHandle, CC1310_LAUNCHXL_SPI0_CLK, 1);
    }
    PIN_setOutputValue(pinHandle, CC1310_LAUNCHXL_SPI_FLASH_CS, 1);
}

/*
 *  ======== CC1310_LAUNCHXL_shutDownExtFlash ========
 */
void CC1310_LAUNCHXL_shutDownExtFlash(void)
{
    PIN_Config extFlashPinTable[] = {
        CC1310_LAUNCHXL_SPI_FLASH_CS | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_INPUT_DIS | PIN_DRVSTR_MED,
        CC1310_LAUNCHXL_SPI0_CLK | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_INPUT_DIS | PIN_DRVSTR_MED,
        CC1310_LAUNCHXL_SPI0_MOSI | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_INPUT_DIS | PIN_DRVSTR_MED,
        CC1310_LAUNCHXL_SPI0_MISO | PIN_INPUT_EN | PIN_PULLDOWN,
        PIN_TERMINATE
    };
    PIN_State extFlashPinState;
    PIN_Handle extFlashPinHandle = PIN_open(&extFlashPinState, extFlashPinTable);

    uint8_t extFlashStartup = 0xAB;
    uint8_t extFlashShutdown = 0xB9;

    CC1310_LAUNCHXL_sendExtFlashByte(extFlashPinHandle, extFlashStartup);
    CC1310_LAUNCHXL_sendExtFlashByte(extFlashPinHandle, extFlashShutdown);

    PIN_close(extFlashPinHandle);
}
