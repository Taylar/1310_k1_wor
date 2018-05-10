#ifndef			__INTERNAL_FLASH_H__
#define			__INTERNAL_FLASH_H__







/***** Defines *****/
// g_rSysConfigInfo
#define 	CONFIG_VALID_FLAG					"valid config"
#define 	CONFIG_FLAG_INTERNAL_ADDR			(0x1d000)  // start 116k position
#define 	CONFIG_DATA_INTERNAL_ADDR			(0x1d000 + 0x20)  // 



//External flash sensor data pointer store position
#define NODE_ADDR_INT_FLASH_POS            			(0X1e000)            // base start 120K position
//External flash sensor data pointer size
#define NODE_ADDR_INT_FLASH_SIZE                    (8)
//External flash sensor data pointer number
#define NODE_ADDR_INT_FLASH_BLOCK_NUM               (32)
//External flash sensor data pointer number
#define NODE_ADDR_INT_FLASH_BLOCK_SIZE              (128)
//External flash sensor data pointer position offset
#define NODE_ADDR_INT_FLASH_AREA_SIZE			    (NODE_ADDR_FLASH_BLOCK_SIZE * NODE_ADDR_FLASH_BLOCK_NUM * NODE_ADDR_FLASH_SIZE)


#define NODE_DECEIVE_MAX_NUM						(200)






/***** Prototypes *****/


extern void Sys_config_reset(void);

extern void Flash_store_config(void);

extern bool Flash_load_config(void);

void InternalFlashInit(void);

bool InternalFlashCheckNodeAddr(uint32_t nodeAddr);

bool InternalFlashSaveNodeAddr(uint32_t nodeAddr, uint32_t *nodeChannel);

uint32_t InternalFlashReadNodeAddr(uint32_t nodeAddr);


#endif			// __INTERNAL_FLASH_H__
