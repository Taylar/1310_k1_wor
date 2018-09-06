/*
* @Author: zxt
* @Date:   2018-04-08 16:25:04
* @Last Modified by:   zxt
* @Last Modified time: 2018-05-25 11:00:51
*/
#include "../general.h"


#pragma DATA_SECTION ( PROJECT_INFO_NAME , "PROJ_NAME_SEC");
#pragma DATA_SECTION ( PROJECT_INFO_VERSION , "PROJ_VER_SEC");

const uint8_t PROJECT_INFO_NAME[64]  = PROJECT_NAME;
const uint32_t PROJECT_INFO_VERSION   = FW_VERSION;

#ifdef SUPPORT_NETWORK

#define				CODE_DATA_LENGTH		64



typedef enum {
	UPGRADE_START = 0, 
	UPGRADE_LOADING,
	UPGRADE_LOADING_COMPLETE,
} UPGRADE_PROCESS_E;

typedef struct
{
    uint8_t		upgradeStep;
    uint16_t	upgradeVer;	
    uint16_t	packNum;	
    uint32_t	fileLength;	
    uint32_t	crc;
    uint32_t	monitorTimer;
    uint32_t	packTimeout;
    uint8_t 	infoFlag;
}upgrade_info_t;



upgrade_info_t upgradeInfo;
const UpgradeBL_t UpgradeBootLoader =  (UpgradeBL_t)BOOT_LOADER_ADDR;

//***********************************************************************************
//
// upgrade init.
//
//***********************************************************************************
void UpgradeInit(void)
{
    upgradeInfo.upgradeStep  = (uint8_t)PROJECT_INFO_NAME[0];
    upgradeInfo.upgradeStep  = (uint8_t)PROJECT_INFO_VERSION;

	upgradeInfo.upgradeStep  = UPGRADE_START;
	upgradeInfo.upgradeVer   = 0;
	upgradeInfo.packNum      = 0;
	upgradeInfo.fileLength   = 0;
	upgradeInfo.monitorTimer = 0;
	upgradeInfo.packTimeout  = 0;
	upgradeInfo.infoFlag	 = UPGRADE_RESULT_UNREC_VER;

}

//***********************************************************************************
//
// upgrade init.
//
//***********************************************************************************
void UpgradeCancel(void)
{

	upgradeInfo.upgradeStep  = UPGRADE_START;
	upgradeInfo.upgradeVer   = 0;
	upgradeInfo.packNum      = 0;
	upgradeInfo.fileLength   = 0;
	upgradeInfo.monitorTimer = 0;
	upgradeInfo.packTimeout  = 0;
	upgradeInfo.infoFlag	 = UPGRADE_RESULT_UNREC_VER;
}



//***********************************************************************************
//
// get the updata version.
//
//***********************************************************************************
uint16_t UpgradeGetVersion(void)
{
	return upgradeInfo.upgradeVer;
}

//***********************************************************************************
//
// get the next package num.
//
//***********************************************************************************
uint16_t UpgradeGetNextPackNum(void)
{
	return upgradeInfo.packNum;
}


//***********************************************************************************
//
// upgrade init.
//
//***********************************************************************************
UPGRADE_RESULT_E UpgradeLoading(uint16_t version, uint16_t packNum, uint16_t length, uint8_t *data)
{
	uint16_t crc2;
	uint8_t buff[64];
	uint32_t addr, readLen;
	upgrade_flag_t upgradeFlag;

	if(upgradeInfo.upgradeStep == UPGRADE_START)
		return UPGRADE_RESULT_ERR;

	if(upgradeInfo.upgradeStep == UPGRADE_LOADING_COMPLETE)
		return UPGRADE_RESULT_LOADING_COMPLETE;

	if(packNum != upgradeInfo.packNum)
		return UPGRADE_RESULT_PACKNUM_ERR;

	addr = (uint32_t)packNum * NWK_UPGRADE_PACKAGE_LENGTH;

	Flash_store_upgrade_data(addr, data, length);
	upgradeInfo.packNum++;

	if(((uint32_t)upgradeInfo.packNum * NWK_UPGRADE_PACKAGE_LENGTH) >= upgradeInfo.fileLength)
	{
		// check the crc
		addr 	= 0;
		readLen = upgradeInfo.fileLength;  //1d896
		Flash_load_upgrade_data(addr, buff, 64);
		upgradeInfo.crc = ((uint16_t)buff[1] << 8) + buff[0];
		readLen = readLen - 128;
		addr += 128;
		SetContinueCRC16();
		while(readLen)
		{
			if(readLen >= CODE_DATA_LENGTH)
			{
				Flash_load_upgrade_data(addr, buff, CODE_DATA_LENGTH);
				addr += CODE_DATA_LENGTH;
				crc2 = ContinueCRC16(buff, CODE_DATA_LENGTH);
				readLen = readLen - CODE_DATA_LENGTH;
			}
			else
			{
				Flash_load_upgrade_data(addr, buff, readLen);
				crc2 = ContinueCRC16(buff, readLen);
				readLen = 0;
			}
		}
		if(upgradeInfo.crc != crc2)
		{
			upgradeInfo.upgradeStep = UPGRADE_START;
			UpgradeCancel();
			return UPGRADE_RESULT_CRC_ERR;
		}

		memset(&upgradeFlag, 0xff, sizeof(upgrade_flag_t));
		strcpy((char*)upgradeFlag.validFlag, UPGRADE_FLAG);
		upgradeFlag.waiteUpgrade = 0x01;
		upgradeFlag.complete     = 0xff;
		upgradeFlag.crc          = upgradeInfo.crc;
		upgradeFlag.fileLength   = upgradeInfo.fileLength;

		Flash_store_upgrade_info((uint8_t*)&upgradeFlag,sizeof(upgrade_flag_t));
		memset(&upgradeFlag, 0, sizeof(upgrade_flag_t));
		Flash_load_upgrade_info((uint8_t*)&upgradeFlag,sizeof(upgrade_flag_t));
		

		upgradeInfo.upgradeStep = UPGRADE_LOADING_COMPLETE;
		return UPGRADE_RESULT_LOADING_COMPLETE;
	}
	else
	{
		return UPGRADE_RESULT_NEXT_PACKAGE;
	}
}



//***********************************************************************************
//
// upgrade timer monitor.
//
//***********************************************************************************
uint16_t UpgradeMonitor(void)
{
	upgradeInfo.monitorTimer++;
	upgradeInfo.packTimeout++;

	if((upgradeInfo.packTimeout >= UPGRADE_PACKAGE_TIMEOUT) || (upgradeInfo.monitorTimer >= UPGRADE_TIMEOUT))
	{
	    return 1;
	}

	return 0;
}

//***********************************************************************************
//
// upgrade timer monitor.
//
//***********************************************************************************
void UpgradePackTimeoutReset(void)
{
	upgradeInfo.packTimeout = 0;
}


//***********************************************************************************
//
// upgrade timer monitor.
//
//***********************************************************************************
void UpgradeMonitorReset(void)
{
	upgradeInfo.monitorTimer = 0;
}


//***********************************************************************************
//
// upgrade time get
//
//***********************************************************************************
uint32_t UpgradeMonitorGet(void)
{
	return upgradeInfo.monitorTimer;
}


//***********************************************************************************
//
// when receive the version info, set the infoFlag
//
//***********************************************************************************
void SetUpgradeInfo(uint16_t version, uint32_t fileLength)
{
	if(version == FW_VERSION)
	{
		upgradeInfo.infoFlag = UPGRADE_RESULT_NEEDNOT_UPDATA;
	}
	if((upgradeInfo.upgradeVer != version) || (fileLength != upgradeInfo.fileLength))
	{
		upgradeInfo.upgradeStep = UPGRADE_LOADING;
		upgradeInfo.upgradeVer  = version;
		upgradeInfo.fileLength  = fileLength;
		upgradeInfo.packNum     = 0;
		upgradeInfo.infoFlag    = UPGRADE_RESULT_NEED_UPDATA;
	}

}


//***********************************************************************************
//
// when request the version info, clear the infoFlag
//
//***********************************************************************************
void ClearUpgradeInfo(void)
{
	upgradeInfo.infoFlag	 = UPGRADE_RESULT_UNREC_VER;
}


//***********************************************************************************
//
// read the infoFlag
//
//***********************************************************************************
uint8_t ReadUpgradeInfoFlag(void)
{
	return upgradeInfo.infoFlag;
}
#else
void UpgradeCancel(void){}
#endif

