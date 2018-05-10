#ifndef         __EXTFLASH_H__
#define         __EXTFLASH_H__



#define FLASH_EXT_SECTOR_ERASE          0
#define FLASH_EXT_BLOCK_ERASE           1
#define FLASH_EXT_CHIP_ERASE            2

#ifdef FLASH_W25Q256FV
#define FLASHCMD_4BYTE_ADDR_MODE        0xB7
#define FLASHCMD_3BYTE_ADDR_MODE        0xE9
#endif

//Flash command
//Flash status cmd
#define FLASHCMD_W_STATUS               0x01
#define FLASHCMD_R_STATUS               0x05
//Flash write enable/disable cmd
#define FLASHCMD_W_DISABLE              0x04
#define FLASHCMD_W_ENABLE               0x06
//Flash page program & read cmd

#ifdef MX66L1G45G

#define FLASHCMD_WRITE                  0x12
#define FLASHCMD_READ                   0x13

//Flash erase cmd
#define FLASHCMD_SECTOR_ERASE           0x21
#define FLASHCMD_BLOCK_ERASE            0xDC
#define FLASHCMD_CHIP_ERASE             0xC7

#else
#define FLASHCMD_WRITE                  0x02
#define FLASHCMD_READ                   0x03

//Flash erase cmd
#define FLASHCMD_SECTOR_ERASE           0x20
#define FLASHCMD_BLOCK_ERASE            0xD8
#define FLASHCMD_CHIP_ERASE             0xC7

#endif


#define FLASH_STATUS_REG1               1
#define FLASH_STATUS_REG2               2
#define FLASH_STATUS_REG3               3

// Flash status write in progress bit WIP
#define WIP_BIT                         0x01
// Flash status write enable latch bit WEL
#define WEL_BIT                         0x02

// Flash page byte
#define PAGE_BYTES                      256


//External flash sector size
#define FLASH_SECTOR_SIZE               0x1000

/***************flash area start************/
/*---    MISC   ----------512KB ---*/
/*---    SENSORPTR  ----512KB---*/
/*---    SENSORDATA  -----XMB---*/
/*---    OTHER ------------XMB---*/
/**************flash area  end**************/



/*---    MISC   ----------512KB ---*/
//External flash system info store position
#define FLASH_SYS_POS                   0   
//External flash system info length
#define FLASH_SYS_LENGTH                sizeof(FlashSysInfo_t)
//External flash system info area size
#define FLASH_SYS_AREA_SIZE             FLASH_SECTOR_SIZE * 1                           //4KB


#if (defined BOARD_S6_6) || (defined BOARD_S2_2)

// ************upgrade data ***********************
#define FLASH_UPGRADE_INFO_POS           (352 * 1024L)// 

#define FLASH_UPGRADE_INFO_LENGTH        (sizeof(upgrade_flag_t))

#define FLASH_UPGRADE_INFO_AREA_SIZE     (FLASH_SECTOR_SIZE) //4 KB


#define FLASH_UPGRADE_DATA_POS           (FLASH_UPGRADE_INFO_POS + FLASH_UPGRADE_INFO_AREA_SIZE)// 

#define FLASH_UPGRADE_DATA_AREA_SIZE     (132 * 1024L) //132 KB




/*---    SENSORPTR  ----512KB---*/
//External flash sensor data pointer store position
#define FLASH_SENSOR_PTR_POS            (512*1024L)// 512KB
//External flash sensor data pointer size
#define FLASH_SENSOR_PTR_SIZE           16
//External flash sensor data pointer number
#define FLASH_SENSOR_PTR_NUMBER         32*1024L  // 1¡¤??¨®1¨¬? 60*24*365*500(¨¬¡§) =262800000 = 2628*10¨ª¨°¡ä?/?¨º  12?¨º¨º¨´?¨¹=32*1024  6?¨º¨º¨´?¨¹ 16*1024¡ê? 
//External flash sensor data pointer position offset
#define FLASH_SENSOR_PTR_AREA_SIZE      (FLASH_SENSOR_PTR_SIZE * FLASH_SENSOR_PTR_NUMBER)  //512 KB

/*---    SENSORDATA  -----XMB---*/
//External flash sensor data store position
#define FLASH_SENSOR_DATA_POS           (1*1024*1024L)// 1MB
//External flash sensor data size
#ifdef FLASH_SENSOR_DATA_32BYTE
#define FLASH_SENSOR_DATA_SIZE          32  //32 ?¡ì3?2¨ª¡§¦Ì¨¤
#else
#define FLASH_SENSOR_DATA_SIZE          64  //64 ?¡ì3?8¨ª¡§¦Ì¨¤,
#endif
//External flash sensor data number

#ifdef FLASH_SIZE_128M
#define FLASH_SENSOR_DATA_NUMBER        (127*(1024/FLASH_SENSOR_DATA_SIZE)* 1024L)    //127MB
#else //FLASH_SIZE_16M
#define FLASH_SENSOR_DATA_NUMBER        (8*(1024/FLASH_SENSOR_DATA_SIZE)* 1024L)    //8MB
#endif

//External flash sensor data position offset
#define FLASH_SENSOR_DATA_AREA_SIZE     (FLASH_SENSOR_DATA_SIZE * FLASH_SENSOR_DATA_NUMBER)

#endif


#ifdef BOARD_S1_2
// ************upgrade data ***********************
#define FLASH_UPGRADE_INFO_POS           (FLASH_SYS_POS + FLASH_SYS_AREA_SIZE)// 

#define FLASH_UPGRADE_INFO_LENGTH        (sizeof(upgrade_flag_t))

#define FLASH_UPGRADE_INFO_AREA_SIZE     (FLASH_SECTOR_SIZE) //4 KB


#define FLASH_UPGRADE_DATA_POS           (FLASH_UPGRADE_INFO_POS + FLASH_UPGRADE_INFO_AREA_SIZE)// 

#define FLASH_UPGRADE_DATA_AREA_SIZE     (132 * 1024L) //132 KB


//External flash sensor data pointer store position
#define FLASH_SENSOR_PTR_POS            (FLASH_SYS_POS + FLASH_SYS_AREA_SIZE)
//External flash sensor data pointer size
#define FLASH_SENSOR_PTR_SIZE           16
//External flash sensor data pointer number
#define FLASH_SENSOR_PTR_NUMBER         (5120L)           // 20*4096
//External flash sensor data pointer position offset
#define FLASH_SENSOR_PTR_AREA_SIZE      (FLASH_SENSOR_PTR_SIZE * FLASH_SENSOR_PTR_NUMBER)

//External flash sensor data store position
#define FLASH_SENSOR_DATA_POS           (FLASH_SENSOR_PTR_POS + FLASH_SENSOR_PTR_AREA_SIZE)
//External flash sensor data size
#define FLASH_SENSOR_DATA_SIZE          32
//External flash sensor data number
#define FLASH_SENSOR_DATA_NUMBER        (5120L)         // the max store data   //40 *4096
//External flash sensor data position offset
#define FLASH_SENSOR_DATA_AREA_SIZE     (FLASH_SENSOR_DATA_SIZE * FLASH_SENSOR_DATA_NUMBER)


#endif


//FlashPointerData_t.head, means ptrData is valid data.
#define FLASH_PTRDATA_VALID             0xFAF5
//FlashPointerData_t.head, means ptrData is invalid data.
#define FLASH_PTRDATA_INVALID           0xF0A0

//Flash ptrData type, now FlashPointerData_t occupy 16 byte in flash.
typedef struct {
    //FLASH_PTRDATA_VALID means valid data, FLASH_PTRDATA_INVALID means invalid data.
    uint16_t head;
    //Data queue front address
    uint32_t frontAddr;
    //Data queue rear address
    uint32_t rearAddr;
} FlashPointerData_t;

typedef struct {
    //Current ptrData address
    uint32_t ptrDataAddr;
    //ptrData store type
    FlashPointerData_t ptrData;
} FlashSensorData_t;

typedef struct {
    //Start record address
    uint32_t start;
    //End record address
    uint32_t end;
} FlashPrintRecordAddr_t;

typedef struct {
    //Start record address
    uint16_t swVersion;
    //End record address
    FlashPrintRecordAddr_t printRecordAddr;
} FlashSysInfo_t;


extern void Flash_init(void);
extern uint32_t Flash_get_sensor_writeaddr(void);
extern uint32_t Flash_get_sensor_readaddr(void);
extern void Flash_read_rawdata(uint32_t flashAddr, uint8_t *pData, uint16_t length);
extern ErrorStatus Flash_load_sensor_data(uint8_t *pData, uint16_t length);
extern ErrorStatus Flash_load_sensor_data_history(uint8_t *pData, uint16_t length, uint32_t number);
extern void Flash_store_sensor_data(uint8_t *pData, uint16_t length);
extern void Flash_recovery_last_sensor_data(void);
extern void Flash_moveto_next_sensor_data(void);
extern ErrorStatus Flash_load_sensor_data_by_offset(uint8_t *pData, uint16_t length, uint8_t offset);
extern void Flash_moveto_offset_sensor_data(uint8_t offset);
extern void Flash_store_record_addr(uint8_t startOrEnd);
extern FlashPrintRecordAddr_t Flash_get_record_addr(void);
extern void Flash_get_record(uint32_t addr, uint8_t *pData, uint16_t length);
extern uint32_t Flash_get_unupload_items(void);
extern uint32_t Flash_get_record_items(void);


extern uint16_t Flash_load_sensor_codec( uint32_t deviceid);
extern void Flash_store_sensor_codec(uint16_t no, uint32_t deviceid);

extern bool Flash_store_sensor_data_async(uint8_t *pData, uint16_t length);
extern void Flash_store_sensor_data_sync();


void Flash_store_upgrade_data(uint32_t addr, uint8_t *pData, uint16_t length);

ErrorStatus Flash_load_upgrade_data(uint32_t addr, uint8_t *pData, uint16_t length);

void Flash_store_upgrade_info(uint8_t *pData, uint16_t length);

ErrorStatus Flash_load_upgrade_info(uint8_t *pData, uint16_t length);


#endif          // __EXTFLASH_H__
