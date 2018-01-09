#include "../general.h"

/***** Defines *****/




/***** Type declarations *****/


/***** Variable declarations *****/
ConfigInfo_t g_rSysConfigInfo;

uint32_t intFlashNodeWritenAddr[NODE_ADDR_INT_FLASH_BLOCK_NUM];




/***** Prototypes *****/




/***** Function definitions *****/

//***********************************************************************************
// brief: erase the node Addr record
// 
// parameter: 
//***********************************************************************************
void InternalFlashInit(void)
{
	
	FlashSectorErase(NODE_ADDR_INT_FLASH_POS);

	memset(intFlashNodeWritenAddr, 0, NODE_ADDR_INT_FLASH_BLOCK_NUM);
}

//***********************************************************************************
// brief: check the node channel if in the internal flash
// 
// parameter: true : save the node channel to internal flash
// 			  false: this node channel has save in the internal flash
//***********************************************************************************
bool InternalFlashCheckNodeAddr(uint32_t nodeAddr)
{
	uint32_t oppositeAddr;
	uint32_t addrSector;
	uint32_t addrTemp;
	uint8_t  i;
	oppositeAddr = nodeAddr % NODE_DECEIVE_MAX_NUM;

	addrSector	= oppositeAddr / NODE_ADDR_INT_FLASH_BLOCK_SIZE;


	addrTemp	= NODE_ADDR_INT_FLASH_POS + addrSector * NODE_ADDR_INT_FLASH_BLOCK_SIZE; 
	for(i = 0; i < intFlashNodeWritenAddr[addrSector]; i++)
	{
		if((*(uint32_t*)(addrTemp+i*NODE_ADDR_INT_FLASH_SIZE)) == nodeAddr)
			return true;
	}
	
	return false;
}

//***********************************************************************************
// brief: save the node addr and the channel to the internal flash
// 
// parameter: true : save the node channel to internal flash
// 			  false: this node channel has save in the internal flash
//***********************************************************************************
bool InternalFlashSaveNodeAddr(uint32_t nodeAddr, uint32_t nodeChannel)
{
	if(InternalFlashCheckNodeAddr(nodeAddr)	== false)
	{
		uint32_t oppositeAddr;
		uint32_t addrSector;
		uint32_t addrTemp;

		oppositeAddr = nodeAddr % NODE_DECEIVE_MAX_NUM;

		addrSector	= oppositeAddr / NODE_ADDR_INT_FLASH_BLOCK_SIZE;

		addrTemp	= NODE_ADDR_INT_FLASH_POS + addrSector * NODE_ADDR_INT_FLASH_BLOCK_SIZE;

		FlashProgram((uint8_t*)(&nodeAddr), addrTemp + intFlashNodeWritenAddr[addrSector], 4);
		FlashProgram((uint8_t*)(&nodeChannel), addrTemp + intFlashNodeWritenAddr[addrSector] + 4, 4);

		intFlashNodeWritenAddr[addrSector] += NODE_ADDR_INT_FLASH_SIZE;
		return true;
	}

	return false;
}


//***********************************************************************************
// brief: read the node addr and the channel to the internal flash
// 
// parameter: 
//***********************************************************************************
uint32_t InternalFlashReadNodeAddr(uint32_t nodeAddr)
{
	uint32_t oppositeAddr;
	uint32_t addrSector;
	uint32_t addrTemp;
	uint8_t  i;
	oppositeAddr = nodeAddr % NODE_DECEIVE_MAX_NUM;

	addrSector	= oppositeAddr / NODE_ADDR_INT_FLASH_BLOCK_SIZE;

	addrTemp	= NODE_ADDR_INT_FLASH_POS + addrSector * NODE_ADDR_INT_FLASH_BLOCK_SIZE; 
	for(i = 0; i < intFlashNodeWritenAddr[addrSector]; i++)
	{
		if((*(uint32_t*)(addrTemp+i*NODE_ADDR_INT_FLASH_SIZE)) == nodeAddr)
			return (*(uint32_t*)(addrTemp+i*NODE_ADDR_INT_FLASH_SIZE + 4));
	}

	return 0xffffffff;
}


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

