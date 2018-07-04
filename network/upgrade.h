#ifndef			UPGRADE_H
#define			UPGRADE_H

#define			UPGRADE_TIMEOUT						(30*60L)
#define 		UPGRADE_PACKAGE_TIMEOUT 			(30)


#define 		UPGRADE_FLAG				"valid info"



#ifdef 			BOARD_S6_6


#ifdef 			BOARD_CONFIG_DECEIVE
#define 		PROJECT_NAME				"ZKS_S6_6_CONFIG"
#else
#define 		PROJECT_NAME				"ZKS_S6_6"
#endif	// BOARD_CONFIG_DECEIVE


#endif	// BOARD_S6_6


#ifdef 			BOARD_S1_2

#ifdef          SUPPORT_BOARD_OLD_S1
#define         PROJECT_NAME                "ZKS_S1_1"
#else
#define 		PROJECT_NAME				"ZKS_S3_1"
#endif

#endif	// BOARD_S1_2

#ifndef		PROJECT_NAME
#define 	PROJECT_NAME 			"ZKS"
#endif		// PROJECT_NAME


#if             NWK_MSG_SIZE >= 300
#define 		NWK_UPGRADE_PACKAGE_LENGTH			(256)
#elif		NWK_MSG_SIZE >= 180
#define 		NWK_UPGRADE_PACKAGE_LENGTH			(128)
#elif		NWK_MSG_SIZE >= 100
#define 		NWK_UPGRADE_PACKAGE_LENGTH			(64)
#elif		NWK_MSG_SIZE >= 64
#define 		NWK_UPGRADE_PACKAGE_LENGTH			(32)
#endif

#define 		UPGRADE_FILE_INFO_LENGTH			128

#define			BOOT_LOADER_ADDR					0x1000
typedef void (*UpgradeBL_t) (void);

extern const UpgradeBL_t UpgradeBootLoader;

typedef enum {
	UPGRADE_RESULT_LOADING_COMPLETE = 0,
	UPGRADE_RESULT_NEXT_PACKAGE,
	UPGRADE_RESULT_PACKNUM_ERR,
	UPGRADE_RESULT_CRC_ERR,
	UPGRADE_RESULT_ERR,
	UPGRADE_RESULT_NEED_UPDATA,
	UPGRADE_RESULT_NEEDNOT_UPDATA,
} UPGRADE_RESULT_E;



typedef struct
{
    uint8_t		validFlag[sizeof(UPGRADE_FLAG)];	
    uint8_t		waiteUpgrade;		// if program need to upgrade 0: needn't;   1: need
    uint8_t		complete;			// test this flag when upgrade complete, and then clear it;     1:updata complete; 0: donot occur upgrade
    uint16_t	crc;
    uint32_t	fileLength;			// the len is the file length that include UPGRADE_FILE_INFO_LENGTH;
    bool     	endFlag;
}upgrade_flag_t;


void UpgradeInit(void);

void UpgradeCancel(void);

uint16_t UpgradeSetInfo(uint16_t version, uint32_t fileLength);

uint16_t UpgradeGetVersion(void);

uint16_t UpgradeGetNextPackNum(void);

UPGRADE_RESULT_E UpgradeLoading(uint16_t version, uint16_t packNum, uint16_t length, uint8_t *data);

uint16_t UpgradeMonitor(void);

void UpgradePackTimeoutReset(void);

void UpgradeMonitorReset(void);

uint32_t UpgradeMonitorGet(void);

extern const uint8_t PROJECT_INFO_NAME[64];
extern const uint32_t PROJECT_INFO_VERSION;


#endif 			// UPGRADE_H
