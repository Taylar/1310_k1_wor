/*
* @Author: justfortest
* @Date:   2018-04-08 16:25:04
* @Last Modified by:   zxt
* @Last Modified time: 2020-06-08 16:52:15
*/
#include "../general.h"


#pragma DATA_SECTION ( PROJECT_INFO_NAME , "PROJ_NAME_SEC");
#pragma DATA_SECTION ( PROJECT_INFO_VERSION , "PROJ_VER_SEC");

const uint8_t PROJECT_INFO_NAME[64]  = PROJECT_NAME;
const uint32_t PROJECT_INFO_VERSION   = FW_VERSION;

uint8_t upgradeSeverIp[64];
uint16_t upgradeSeverPort;
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

	memset(upgradeSeverIp, 0, 64);
	upgradeSeverPort = 0;
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

	memset(upgradeSeverIp, 0, 64);
	upgradeSeverPort = 0;
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
	else if((upgradeInfo.upgradeVer != version) || (fileLength != upgradeInfo.fileLength))
	{
		upgradeInfo.upgradeStep = UPGRADE_LOADING;
		upgradeInfo.upgradeVer  = version;
		upgradeInfo.fileLength  = fileLength;
		upgradeInfo.packNum     = 0;
		upgradeInfo.infoFlag    = UPGRADE_RESULT_NEED_UPDATA;
	}
	else
	{
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

