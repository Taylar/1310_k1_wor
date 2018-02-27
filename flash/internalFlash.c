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

	addrSector	= oppositeAddr / NODE_ADDR_INT_FLASH_BLOCK_NUM;


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
bool InternalFlashSaveNodeAddr(uint32_t nodeAddr, uint32_t *nodeChannel)
{
	if(InternalFlashCheckNodeAddr(nodeAddr)	== false)
	{
		uint32_t oppositeAddr;
		uint32_t addrSector;
		uint32_t addrTemp;

		oppositeAddr = nodeAddr % NODE_DECEIVE_MAX_NUM;

		addrSector	= oppositeAddr / NODE_ADDR_INT_FLASH_BLOCK_NUM;

		addrTemp	= NODE_ADDR_INT_FLASH_POS + addrSector * NODE_ADDR_INT_FLASH_BLOCK_SIZE;

		FlashProgram((uint8_t*)(&nodeAddr), addrTemp + intFlashNodeWritenAddr[addrSector], 4);
		FlashProgram((uint8_t*)(nodeChannel), addrTemp + intFlashNodeWritenAddr[addrSector] + 4, 4);

		intFlashNodeWritenAddr[addrSector] += NODE_ADDR_INT_FLASH_SIZE;
		if((intFlashNodeWritenAddr[addrSector] / NODE_ADDR_INT_FLASH_SIZE) >= NODE_ADDR_INT_FLASH_BLOCK_SIZE)
		{
			// clear the channel num and erase the node addr and channel record
			InternalFlashInit();
			*nodeChannel = 0;
			return false;
		}

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

	addrSector	= oppositeAddr / NODE_ADDR_INT_FLASH_BLOCK_NUM;

	addrTemp	= NODE_ADDR_INT_FLASH_POS + addrSector * NODE_ADDR_INT_FLASH_BLOCK_SIZE; 
	for(i = 0; i < intFlashNodeWritenAddr[addrSector]; i++)
	{
		if((*(uint32_t*)(addrTemp+i*NODE_ADDR_INT_FLASH_SIZE)) == nodeAddr)
			return (*(uint32_t*)(addrTemp+i*NODE_ADDR_INT_FLASH_SIZE + 4));
	}

	return 0xffffffff;
}

const uint8_t configFlag[12] = CONFIG_VALID_FLAG;
//***********************************************************************************
// brief: store the config from the internal flash   
// 
// note: every sector is 4K
// parameter: 
//***********************************************************************************
void InternalFlashStoreConfig(void)
{
	FlashSectorErase(CONFIG_DATA_INTERNAL_ADDR);
	
	FlashProgram((uint8_t *)configFlag, CONFIG_FLAG_INTERNAL_ADDR, 12);
	FlashProgram((uint8_t*)(&g_rSysConfigInfo), CONFIG_DATA_INTERNAL_ADDR, sizeof(ConfigInfo_t));
}

//***********************************************************************************
// brief: load the config from the internal flash   
// 
// parameter: 
//***********************************************************************************
bool InternalFlashLoadConfig(void)
{
	uint8_t i;

	for(i = 0; i < 12; i++)
	{
		if(configFlag[i] != *((uint8_t*)(CONFIG_FLAG_INTERNAL_ADDR+i)))
			return false;
	}
	memcpy((uint8_t*)(&g_rSysConfigInfo), (uint8_t*)CONFIG_DATA_INTERNAL_ADDR, sizeof(ConfigInfo_t));
	return true;
}


//***********************************************************************************
// brief: load the config from the internal flash   
// 
// parameter: 
//***********************************************************************************
void InternalFlashConfigReset(void)
{
	// uint8_t i;

    g_rSysConfigInfo.size = sizeof(ConfigInfo_t);
    g_rSysConfigInfo.swVersion = FW_VERSION;
    g_rSysConfigInfo.DeviceId[0] = (uint8_t)((DECEIVE_ID_DEFAULT>>24)&0xff);
    g_rSysConfigInfo.DeviceId[1] = (uint8_t)((DECEIVE_ID_DEFAULT>>16)&0xff);
    g_rSysConfigInfo.DeviceId[2] = (uint8_t)((DECEIVE_ID_DEFAULT>>8)&0xff);
    g_rSysConfigInfo.DeviceId[3] = (uint8_t)((DECEIVE_ID_DEFAULT)&0xff);;

    g_rSysConfigInfo.customId[0] = (uint8_t)(CUSTOM_ID_DEFAULT >> 8);
    g_rSysConfigInfo.customId[1] = (uint8_t)(CUSTOM_ID_DEFAULT);


    g_rSysConfigInfo.status = 0;

#ifdef		BOARD_S2_2

	g_rSysConfigInfo.module          = MODULE_NWK | MODULE_RADIO;
	g_rSysConfigInfo.serverIpAddr[0] = 114;
	g_rSysConfigInfo.serverIpAddr[1] = 215;
	g_rSysConfigInfo.serverIpAddr[2] = 122;
	g_rSysConfigInfo.serverIpAddr[3] = 32;
	g_rSysConfigInfo.serverIpPort    = 12200;

	g_rSysConfigInfo.batLowVol       = 3600;
	g_rSysConfigInfo.apnuserpwd[0]   = 0;
	g_rSysConfigInfo.hbPeriod        = UPLOAD_PERIOD_DEFAULT;     // unit is sec
#endif

#ifdef		BOARD_S6_6

	g_rSysConfigInfo.module          = MODULE_NWK | MODULE_RADIO;
	g_rSysConfigInfo.serverIpAddr[0] = 114;
	g_rSysConfigInfo.serverIpAddr[1] = 215;
	g_rSysConfigInfo.serverIpAddr[2] = 122;
	g_rSysConfigInfo.serverIpAddr[3] = 32;
	g_rSysConfigInfo.serverIpPort    = 12200;

	g_rSysConfigInfo.batLowVol       = 3600;
	g_rSysConfigInfo.apnuserpwd[0]   = 0;
	g_rSysConfigInfo.hbPeriod        = UPLOAD_PERIOD_DEFAULT;     // unit is sec
#endif


#ifdef		BOARD_S1_2

	g_rSysConfigInfo.batLowVol       = 2500;
	g_rSysConfigInfo.apnuserpwd[0]   = 0;
#endif

/*
    for (i = 0; i < MODULE_SENSOR_MAX; i++) {
        g_rSysConfigInfo.sensorModule[i] = SEN_TYPE_NONE;
        g_rSysConfigInfo.alarmTemp[i].high = ALARM_TEMP_HIGH;
        g_rSysConfigInfo.alarmTemp[i].low = ALARM_TEMP_LOW;
        g_rSysConfigInfo.WarningTemp[i].high = ALARM_TEMP_HIGH;
        g_rSysConfigInfo.WarningTemp[i].low = ALARM_TEMP_LOW;
    }
    
    g_rSysConfigInfo.sensorModule[0] = SEN_TYPE_SHT2X;
*/

	g_rSysConfigInfo.collectPeriod   = UPLOAD_PERIOD_DEFAULT;   //unit is sec
	
	g_rSysConfigInfo.uploadPeriod    = UPLOAD_PERIOD_DEFAULT; // unit is sec
	
	g_rSysConfigInfo.ntpPeriod       = NTC_DEFAULT;    // 
	
	g_rSysConfigInfo.gnssPeriod      = 10;  // 10sec
	
	
}
