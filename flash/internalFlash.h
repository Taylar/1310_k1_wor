#ifndef			__INTERNAL_FLASH_H__
#define			__INTERNAL_FLASH_H__







/***** Defines *****/
// g_rSysConfigInfo
#define 	CONFIG_VALID_FLAG					"valid config"
#define 	CONFIG_FLAG_INTERNAL_ADDR			(0x1e000)  // start 116k position
#define 	CONFIG_DATA_INTERNAL_ADDR			(0x1e000 + 0x20)  // 








/***** Prototypes *****/


extern void Sys_config_reset(void);

extern void Flash_store_config(void);

extern bool Flash_load_config(void);


#endif			// __INTERNAL_FLASH_H__
