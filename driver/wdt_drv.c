/*
* @Author: zxt
* @Date:   2018-01-27 11:44:25
* @Last Modified by:   zxt
* @Last Modified time: 2018-08-09 16:45:22
*/
#include "../general.h"

#define		WDT_TOUT_MS				15000

#ifdef SUPPORT_WATCHDOG
Clock_Struct watchdogClkStruct;
Clock_Handle watchdogClkHandle;
#define WATCHDAG_FEED_TIME          10 * CLOCK_UNIT_S
#endif

//***********************************************************************************
//
// System feeding dog
//
//***********************************************************************************
static void Sys_watchDogFxn(UArg arg0)
{
    Sys_event_post(SYS_FEED_WATCHDOG);
}

/*
 *  =============================== Watchdog ===============================
 */

WatchdogCC26XX_Object watchdogCC26XXObjects[CC1310_LAUNCHXL_WATCHDOGCOUNT];

const WatchdogCC26XX_HWAttrs watchdogCC26XXHWAttrs[CC1310_LAUNCHXL_WATCHDOGCOUNT] = {
    {
        .baseAddr    = WDT_BASE,
        .reloadValue = WDT_TOUT_MS /* Reload value in milliseconds */
    },
};

const Watchdog_Config Watchdog_config[CC1310_LAUNCHXL_WATCHDOGCOUNT] = {
    {
        .fxnTablePtr = &WatchdogCC26XX_fxnTable,
        .object      = &watchdogCC26XXObjects[CC1310_LAUNCHXL_WATCHDOG0],
        .hwAttrs     = &watchdogCC26XXHWAttrs[CC1310_LAUNCHXL_WATCHDOG0]
    },
};

const uint_least8_t Watchdog_count = CC1310_LAUNCHXL_WATCHDOGCOUNT;


/***** Variable declarations *****/
Watchdog_Handle watchdogHandle;


void WdtInit(Watchdog_Callback cb)
{
	Watchdog_Params params;
	Clock_Params clkParams;

	if (NULL == watchdogHandle &&  NULL == watchdogClkHandle)
	{
        /* Construct a 8s periodic Clock Instance to feed watchdog */
        Clock_Params_init(&clkParams);
        clkParams.period = WATCHDAG_FEED_TIME;
        clkParams.startFlag = TRUE;
        Clock_construct(&watchdogClkStruct, (Clock_FuncPtr)Sys_watchDogFxn, WATCHDAG_FEED_TIME, &clkParams);
        watchdogClkHandle = Clock_handle(&watchdogClkStruct);

	    Watchdog_init();
	    Watchdog_Params_init(&params);
        params.callbackFxn = cb;
        params.resetMode = Watchdog_RESET_ON;
        watchdogHandle = Watchdog_open(Board_WATCHDOG0, &params);
	}
}


void WdtClear(void)
{
	Watchdog_clear(watchdogHandle);
}
