/*
* @Author: zxt
* @Date:   2018-01-27 11:44:25
* @Last Modified by:   zxt
* @Last Modified time: 2018-07-25 18:00:21
*/
#include "../general.h"

#define		WDT_TOUT_MS				10000


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

	Watchdog_init();
	Watchdog_Params_init(&params);
    params.callbackFxn = cb;
    params.resetMode = Watchdog_RESET_ON;
    watchdogHandle = Watchdog_open(Board_WATCHDOG0, &params);
}


void WdtClear(void)
{
	Watchdog_clear(watchdogHandle);
}
