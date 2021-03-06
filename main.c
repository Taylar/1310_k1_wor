/*
 * Copyright (c) 2015-2016, Texas Instruments Incorporated
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

/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/System.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/display/Display.h>
#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26XX.h>

/* Board Header files */
#include "Board.h"

#define VARIABLES_DEFINE

#include "general.h"




/*
 *  ======== main ========
 */
int main(void)
{
    __delay_cycles(10000);
    /* Call driver init functions. */
    Board_initGeneral();

    SysAppTaskCreate();


    InterfaceTaskCreate();

#ifndef S_A//涓�浣撴満
    RadioAppTaskCreate();
#endif //S_A
    // test for 32K s
#ifdef FACTOR_RADIO_32K_TEST
    IOCPortConfigureSet(IOID_26, IOC_PORT_AON_CLK32K, IOC_STD_OUTPUT);
    AONIOC32kHzOutputEnable();
    while(1);
#endif //FACTOR_RADIO_32K_TEST
    /* Start BIOS */
    BIOS_start();

    return (0);
}


xdc_Void myErrorFxn(xdc_runtime_Error_Block* p)
{
    // UInt hwiKey;

    // hwiKey = Hwi_enable();
    // Flash_log("UNO\n");

    // g_rSysConfigInfo.sysState.lora_list_fulls++;
    // Flash_store_config();
    // Hwi_restore(hwiKey);
    while(1)
        SysCtrlSystemReset();
}
