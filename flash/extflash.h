#ifndef         __EXTFLASH_H__
#define         __EXTFLASH_H__


#ifdef FLASH_EXTERNAL
#define SUPPORT_FLASH_LOG

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
/*---      |-- FlashSysInfo ----4KB ---*/
/*---      |-- LORA_CHN   ----4KB ---*/
/*---      |-- Flash log--- ----8KB ---*/
/*---      |-- sensor_code-----4KB ---*/
/*---      |-- other             ----140KB ---*/
/*---      |-- upgrade data ----352KB ---*/

/*---    SENSORPTR  ----512KB---*/
/*---    SENSORDATA  -----XMB---*/
/*---    OTHER ------------XMB---*/
/**************flash area  end**************/


#if (defined BOARD_S6_6) || (defined BOARD_B2S)

/*---    MISC   ----------512KB ---*/
//External flash system info store position
#define FLASH_SYS_POS_OFFSET            0x0
#define FLASH_SYS_POS                   0   
//External flash system info length
#define FLASH_SYS_LENGTH                sizeof(FlashSysInfo_t)
//External flash system info area size
#define FLASH_SYS_AREA_SIZE             FLASH_SECTOR_SIZE * 1                           //4KB


//External flash system config info store position
#define FLASH_SYS_CONFIG_INFO_POS                   (FLASH_SYS_POS + FLASH_SYS_AREA_SIZE) 
//External flash system config data store position
#define FLASH_SYS_CONFIG_DATA_POS                   (FLASH_SYS_CONFIG_INFO_POS + 0X20)   
//External flash system config info length
#define FLASH_SYS_CONFIG_LENGTH                sizeof(ConfigInfo_t)
//External flash system config info area size
#define FLASH_SYS_CONFIG_AREA_SIZE             FLASH_SECTOR_SIZE * 1                           //4KB

//External flash system config info store position
#define FLASH_SYS_CONFIG_BAK_INFO_POS                   (FLASH_SYS_CONFIG_INFO_POS + FLASH_SYS_CONFIG_AREA_SIZE) 
//External flash system config data store position
#define FLASH_SYS_CONFIG_BAK_DATA_POS                   (FLASH_SYS_CONFIG_BAK_INFO_POS + 0X20)  
//External flash system config info length
#define FLASH_SYS_CONFIG_BAK_AREA_SIZE             FLASH_SECTOR_SIZE * 1                           //4KB


#define FLASH_LOG_POS               (FLASH_SYS_CONFIG_BAK_INFO_POS + FLASH_SYS_CONFIG_BAK_AREA_SIZE)
#define FLASH_LOG_SIZE              (32)
#ifdef SUPPORT_FLASH_LOG
#define FLASH_LOG_AREA_SIZE         (FLASH_SECTOR_SIZE*2)           //8KB
#else
#define FLASH_LOG_AREA_SIZE         (0)
#endif
#define FLASH_LOG_NUMBER            (FLASH_LOG_AREA_SIZE/FLASH_LOG_SIZE)



#define FLASH_ALARM_RECODRD_POS               (FLASH_LOG_POS + FLASH_LOG_AREA_SIZE)
#define FLASH_ALARM_RECODRD_SIZE              (16)
#ifdef SUPPORT_ALARM_RECORD_QURERY
#define FLASH_ALARM_RECODRD_AREA_SIZE         (FLASH_SECTOR_SIZE*2)           //8KB
#else
#define FLASH_ALARM_RECODRD_AREA_SIZE         (0)
#endif
#define FLASH_ALARM_RECODRD_NUMBER            (FLASH_ALARM_RECODRD_AREA_SIZE/FLASH_ALARM_RECODRD_SIZE)



#define FLASH_SENSOR_CODEC_POS            (FLASH_ALARM_RECODRD_POS + FLASH_ALARM_RECODRD_AREA_SIZE)

#define FLASH_SENSOR_CODEC_SIZE           (4)
#define FLASH_SENSOR_CODEC_NUM            (1024L)
#ifdef SUPPORT_NETGATE_DISP_NODE
#define FLASH_SENSOR_CODEC_AREA_SIZE      (INTERNAL_SEGMEMT_SIZE*FLASH_SENSOR_CODEC_NUM)  //4KB
#else
#define FLASH_SENSOR_CODEC_AREA_SIZE      (0)
#endif


// ************upgrade data ***********************
#define FLASH_UPGRADE_INFO_POS           (352 * 1024L)  // 

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
#define FLASH_SENSOR_PTR_NUMBER         32*1024L  // 1分钟1条 60*24*365*500(台) =262800000 = 2628*10万次/年  12年寿命=32*1024  6年寿命 16*1024， 
//External flash sensor data pointer position offset
#define FLASH_SENSOR_PTR_AREA_SIZE      (FLASH_SENSOR_PTR_SIZE * FLASH_SENSOR_PTR_NUMBER)  //512 KB

/*---    SENSORDATA  -----XMB---*/
//External flash sensor data store position
#define FLASH_SENSOR_DATA_POS           (1*1024*1024L)// 1MB
//#define FLASH_SENSOR_DATA_32BYTE

//External flash sensor data size
#ifdef FLASH_SENSOR_DATA_32BYTE
#define FLASH_SENSOR_DATA_SIZE          32  //32 支持2通道
#else
#define FLASH_SENSOR_DATA_SIZE          64  //64 支持8通道,
#endif
//External flash sensor data number

#ifdef FLASH_SIZE_128M
#define FLASH_SENSOR_DATA_NUMBER        (127*(1024/FLASH_SENSOR_DATA_SIZE)* 1024L)    //127MB
#else //FLASH_SIZE_16M
#define FLASH_SENSOR_DATA_NUMBER        (8*(1024/FLASH_SENSOR_DATA_SIZE)* 1024L)    //8MB
#endif

//External flash sensor data position offset
#define FLASH_SENSOR_DATA_AREA_SIZE     (FLASH_SENSOR_DATA_SIZE * FLASH_SENSOR_DATA_NUMBER)


/*---    OTHER ------------XMB---*/
#ifdef SUPPORT_GSENSOR
#define FLASH_G_SENSOR_PTR_POS            (FLASH_SENSOR_DATA_POS + FLASH_SENSOR_DATA_AREA_SIZE)
#define FLASH_G_SENSOR_PTR_SIZE           16
#define FLASH_G_SENSOR_PTR_NUMBER         1024L    //16KB
#define FLASH_G_SENSOR_PTR_AREA_SIZE      (FLASH_G_SENSOR_PTR_SIZE * FLASH_G_SENSOR_PTR_NUMBER)

#define FLASH_G_SENSOR_DATA_POS           (FLASH_G_SENSOR_PTR_POS + FLASH_G_SENSOR_PTR_AREA_SIZE)
#define FLASH_G_SENSOR_DATA_SIZE          32
#define FLASH_G_SENSOR_DATA_NUMBER        (8*8* 1024L)// 2MB
#define FLASH_G_SENSOR_DATA_AREA_SIZE     (FLASH_G_SENSOR_DATA_SIZE * FLASH_G_SENSOR_DATA_NUMBER)
#else
#define FLASH_G_SENSOR_PTR_POS            (FLASH_SENSOR_DATA_POS + FLASH_SENSOR_DATA_AREA_SIZE)
#define FLASH_G_SENSOR_PTR_AREA_SIZE      0

#define FLASH_G_SENSOR_DATA_POS           (FLASH_G_SENSOR_PTR_POS + FLASH_G_SENSOR_PTR_AREA_SIZE)
#define FLASH_G_SENSOR_DATA_AREA_SIZE     0
#endif

#ifdef SUPPORT_GPS
#define FLASH_GPS_PTR_POS            (FLASH_G_SENSOR_DATA_POS + FLASH_G_SENSOR_DATA_AREA_SIZE)
#define FLASH_GPS_PTR_SIZE           16
#define FLASH_GPS_PTR_NUMBER         1024L   //16KB
#define FLASH_GPS_PTR_AREA_SIZE      (FLASH_GPS_PTR_SIZE * FLASH_GPS_PTR_NUMBER)

#define FLASH_GPS_DATA_POS           (FLASH_GPS_PTR_POS + FLASH_GPS_PTR_AREA_SIZE)
#define FLASH_GPS_DATA_SIZE          32
#define FLASH_GPS_DATA_NUMBER        (8*8* 1024L)  //2MB
#define FLASH_GPS_DATA_AREA_SIZE     (FLASH_GPS_DATA_SIZE * FLASH_GPS_DATA_NUMBER)
#else
#define FLASH_GPS_PTR_POS            (FLASH_G_SENSOR_DATA_POS + FLASH_G_SENSOR_DATA_AREA_SIZE)
#define FLASH_GPS_PTR_AREA_SIZE      0

#define FLASH_GPS_DATA_POS           (FLASH_GPS_PTR_POS + FLASH_GPS_PTR_AREA_SIZE)
#define FLASH_GPS_DATA_AREA_SIZE     0
#endif

#ifdef SUPPORT_DEVICED_STATE_UPLOAD
#define FLASH_DEVICED_STATE_PTR_POS            (FLASH_GPS_DATA_POS + FLASH_GPS_DATA_AREA_SIZE)
#define FLASH_DEVICED_STATE_PTR_SIZE           16
#define FLASH_DEVICED_STATE_PTR_NUMBER         512L   //16KB
#define FLASH_DEVICED_STATE_PTR_AREA_SIZE      (FLASH_DEVICED_STATE_PTR_SIZE * FLASH_DEVICED_STATE_PTR_NUMBER)

#define FLASH_DEVICED_STATE_DATA_POS           (FLASH_DEVICED_STATE_PTR_POS + FLASH_DEVICED_STATE_PTR_AREA_SIZE)
#define FLASH_DEVICED_STATE_DATA_SIZE          16
#define FLASH_DEVICED_STATE_DATA_NUMBER        (1024L)  //2MB
#define FLASH_DEVICED_STATE_DATA_AREA_SIZE     (FLASH_DEVICED_STATE_DATA_SIZE * FLASH_DEVICED_STATE_DATA_NUMBER)
#else
#define FLASH_DEVICED_STATE_PTR_POS            (FLASH_GPS_DATA_POS + FLASH_GPS_DATA_AREA_SIZE)
#define FLASH_DEVICED_STATE_PTR_AREA_SIZE      0

#define FLASH_DEVICED_STATE_DATA_POS           (FLASH_DEVICED_STATE_PTR_POS + FLASH_DEVICED_STATE_PTR_AREA_SIZE)
#define FLASH_DEVICED_STATE_DATA_AREA_SIZE     0
#endif

#ifdef SUPPORT_STORE_ID_IN_EXTFLASH
#define FLASH_NODEID_STORE_POS                  (FLASH_DEVICED_STATE_DATA_POS + FLASH_DEVICED_STATE_DATA_AREA_SIZE)
#define FLASH_NODEID_STORE_SIZE                 4
#define FLASH_NODEID_STORE_NUMBER               2048
#define FLASH_NODEID_STORE_AREA_SIZE            (FLASH_NODEID_STORE_SIZE * FLASH_NODEID_STORE_NUMBER)
#else
#define FLASH_NODEID_STORE_POS                  (FLASH_DEVICED_STATE_DATA_POS + FLASH_DEVICED_STATE_DATA_AREA_SIZE)
#define FLASH_NODEID_STORE_AREA_SIZE            0

#endif // SUPPORT_STORE_ID_IN_EXTFLASH


#endif  //(defined BOARD_S6_6) || (defined BOARD_S2_2)

#ifdef BOARD_S3
// ************upgrade data ***********************
/*---    MISC   ----------512KB ---*/
//External flash system info store position
#define FLASH_SYS_POS_OFFSET            0x0
#define FLASH_SYS_POS                   0   
//External flash system info length
#define FLASH_SYS_LENGTH                sizeof(FlashSysInfo_t)
//External flash system info area size
#define FLASH_SYS_AREA_SIZE             FLASH_SECTOR_SIZE * 1                           //4KB


//External flash system config info store position
#define FLASH_SYS_CONFIG_INFO_POS                   (FLASH_SYS_POS + FLASH_SYS_AREA_SIZE) 
//External flash system config data store position
#define FLASH_SYS_CONFIG_DATA_POS                   (FLASH_SYS_CONFIG_INFO_POS + 0X20)   
//External flash system config info length
#define FLASH_SYS_CONFIG_LENGTH                sizeof(ConfigInfo_t)
//External flash system config info area size
#define FLASH_SYS_CONFIG_AREA_SIZE             FLASH_SECTOR_SIZE * 1                           //4KB

#define FLASH_LOG_POS               (FLASH_SYS_CONFIG_INFO_POS + FLASH_SYS_CONFIG_AREA_SIZE)
#define FLASH_LOG_SIZE              (32)
#ifdef SUPPORT_FLASH_LOG
#define FLASH_LOG_AREA_SIZE         (FLASH_SECTOR_SIZE*2)           //8KB
#else
#define FLASH_LOG_AREA_SIZE         (0)
#endif
#define FLASH_LOG_NUMBER            (FLASH_LOG_AREA_SIZE/FLASH_LOG_SIZE)


#define FLASH_UPGRADE_INFO_POS           (FLASH_SECTOR_SIZE*4)// start addr: 16K

#define FLASH_UPGRADE_INFO_LENGTH        (sizeof(upgrade_flag_t))

#define FLASH_UPGRADE_INFO_AREA_SIZE     (FLASH_SECTOR_SIZE) //4 KB


#define FLASH_UPGRADE_DATA_POS           (FLASH_UPGRADE_INFO_POS + FLASH_UPGRADE_INFO_AREA_SIZE)// 

#define FLASH_UPGRADE_DATA_AREA_SIZE     (132 * 1024L) //132 KB



//External flash system config info store position
#define FLASH_SYS_CONFIG_BAK_INFO_POS                   (FLASH_UPGRADE_INFO_POS + FLASH_UPGRADE_DATA_AREA_SIZE) 
//External flash system config data store position
#define FLASH_SYS_CONFIG_BAK_DATA_POS                   (FLASH_SYS_CONFIG_BAK_INFO_POS + 0X20)  
//External flash system config info length
#define FLASH_SYS_CONFIG_BAK_AREA_SIZE             FLASH_SECTOR_SIZE * 1                           //4KB

//External flash sensor data pointer store position
#define FLASH_SENSOR_PTR_POS            (FLASH_SYS_CONFIG_BAK_INFO_POS + FLASH_SYS_CONFIG_BAK_AREA_SIZE)
//External flash sensor data pointer size
#define FLASH_SENSOR_PTR_SIZE           16
//External flash sensor data pointer number
#define FLASH_SENSOR_PTR_NUMBER         (11264L)           // 176*1024
//External flash sensor data pointer position offset
#define FLASH_SENSOR_PTR_AREA_SIZE      (FLASH_SENSOR_PTR_SIZE * FLASH_SENSOR_PTR_NUMBER)

//External flash sensor data store position
#define FLASH_SENSOR_DATA_POS           (FLASH_SENSOR_PTR_POS + FLASH_SENSOR_PTR_AREA_SIZE)
//External flash sensor data size
#define FLASH_SENSOR_DATA_SIZE          16
//External flash sensor data number
#define FLASH_SENSOR_DATA_NUMBER        (11520L)         // the max store data   //180 *1024
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

#ifdef SUPPORT_DEVICED_STATE_UPLOAD
typedef enum {
   TYPE_NONE = 0,
   TYPE_POWER_ON,
   TYPE_POWER_DOWN,
   TYPE_RECORD_START,
   TYPE_RECORD_STOP,
   TYPE_FLIGHT_MODE_START,
   TYPE_FLIGHT_MODE_STOP,
   TYPE_POWER_RESTART,
   TYPE_BT_PRINT_START,
   TYPE_BT_PRINT_END,
}DevicesSysStateType;
#endif
extern void Flash_init(void);
extern void Flash_reset_all(void);
extern uint32_t Flash_get_sensor_writeaddr(void);
extern uint32_t Flash_get_sensor_readaddr(void);
extern void Flash_read_rawdata(uint32_t flashAddr, uint8_t *pData, uint16_t length);
extern ErrorStatus Flash_load_sensor_data(uint8_t *pData, uint16_t length);
extern ErrorStatus Flash_load_sensor_data_history(uint8_t *pData, uint16_t length, uint32_t number);
extern void Flash_store_sensor_data(uint8_t *pData, uint16_t length);
extern void Flash_recovery_last_sensor_data(void);
extern void Flash_moveto_next_sensor_data(void);
extern ErrorStatus Flash_load_sensor_data_by_offset(uint8_t *pData, uint16_t length, uint32_t offset);
extern void Flash_moveto_offset_sensor_data(uint8_t offset);
extern void Flash_store_record_addr(uint8_t startOrEnd);
extern FlashPrintRecordAddr_t Flash_get_record_addr(void);
FlashPrintRecordAddr_t Flash_get_current_record_addr(void);
extern void Flash_get_record(uint32_t addr, uint8_t *pData, uint16_t length);
extern uint32_t Flash_get_unupload_items(void);
extern uint32_t Flash_get_record_items(void);


extern uint16_t Flash_load_sensor_codec( uint32_t deviceid);
extern void Flash_store_sensor_codec(uint16_t no, uint32_t deviceid);

extern bool Flash_store_sensor_data_async(uint8_t *pData, uint16_t length);
extern void Flash_store_sensor_data_sync();

extern void Flash_recovery_last_g_sensor_data(void);
extern void Flash_store_g_sensor_data(uint8_t *pData, uint16_t length);
extern ErrorStatus Flash_load_g_sensor_data(uint8_t *pData, uint16_t length);

extern void Flash_store_gps_data(uint8_t *pData, uint16_t length);
extern ErrorStatus Flash_load_gps_data_history(uint8_t *pData, uint16_t length, uint16_t number);
extern void Flash_moveto_next_gps_data(void);
extern void Flash_recovery_last_gps_data(void);
ErrorStatus Flash_load_gps_data(uint8_t *pData, uint16_t length);
#ifdef SUPPORT_FLASH_LOG
extern void Flash_log(uint8_t *log);
#else
#define  Flash_log(x)   //
#endif

#ifdef SUPPORT_ALARM_RECORD_QURERY
uint8_t Flash_get_alarm_record_items(void);
ErrorStatus Flash_load_alarm_record_by_offset(uint8_t *buff, uint8_t length,uint8_t offset);
void Flash_store_alarm_record(uint8_t *buff, uint8_t len);
#endif

#ifdef SUPPORT_DEVICED_STATE_UPLOAD
void Flash_store_deviced_state_data(uint8_t *pData, uint16_t length);
void Flash_moveto_next_deviced_state_data(void);
uint32_t Flash_get_deviced_state_items(void);
ErrorStatus Flash_load_deviced_state_data(uint8_t *pData, uint16_t length);
void Flash_store_devices_state(uint8_t StateType);
ErrorStatus Flash_load_alarm_data_by_offset(uint8_t *pData, uint16_t length, uint8_t offset);
#endif
void Flash_store_upgrade_data(uint32_t addr, uint8_t *pData, uint16_t length);

ErrorStatus Flash_load_upgrade_data(uint32_t addr, uint8_t *pData, uint16_t length);

void Flash_store_upgrade_info(uint8_t *pData, uint16_t length);

ErrorStatus Flash_load_upgrade_info(uint8_t *pData, uint16_t length);



void Flash_store_config(void);

bool Flash_load_config(void);

void Sys_config_reset(void);

void Flash_store_nodeid(uint8_t *pData, uint32_t offset);

void Flash_load_nodeid(uint8_t *pData, uint32_t offset);
#endif  /* FLASH_EXTERNAL */


#endif          // __EXTFLASH_H__
