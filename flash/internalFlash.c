#include "../general.h"

/***** Defines *****/
#define 	CONFIG_ADDR_INTERNAL_ADDR			(1024*120)  // start 120k position




/***** Type declarations *****/


/***** Variable declarations *****/
ConfigInfo_t g_rSysConfigInfo;



/***** Prototypes *****/


/***** Function definitions *****/
//***********************************************************************************
// brief: store the config from the internal flash   
// 
// note: every sector is 4K
// parameter: 
//***********************************************************************************
void InternalFlashStoreConfig(void)
{
	FlashSectorErase(CONFIG_ADDR_INTERNAL_ADDR);
	FlashProgram((uint8_t*)(&g_rSysConfigInfo), CONFIG_ADDR_INTERNAL_ADDR, sizeof(ConfigInfo_t));
}




//***********************************************************************************
// brief: load the config from the internal flash   
// 
// parameter: 
//***********************************************************************************
void InternalFlashLoadConfig(void)
{
	memcpy((uint8_t*)(&g_rSysConfigInfo), (uint8_t*)CONFIG_ADDR_INTERNAL_ADDR, sizeof(ConfigInfo_t));
}
