#ifndef         __EXTFLASH_H__
#define         __EXTFLASH_H__



#define FLASH_EXT_SECTOR_ERASE          0
#define FLASH_EXT_BLOCK_ERASE           1
#define FLASH_EXT_CHIP_ERASE            2


//Flash command
//Flash status cmd
#define FLASHCMD_W_STATUS               0x01
#define FLASHCMD_R_STATUS               0x05
//Flash write enable/disable cmd
#define FLASHCMD_W_DISABLE              0x04
#define FLASHCMD_W_ENABLE               0x06
//Flash page program & read cmd
#define FLASHCMD_WRITE                  0x02
#define FLASHCMD_READ                   0x03
//Flash erase cmd
#define FLASHCMD_SECTOR_ERASE           0x20
#define FLASHCMD_BLOCK_ERASE            0xD8
#define FLASHCMD_CHIP_ERASE             0xC7

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

//External flash system info store position
#define FLASH_SYS_POS                   0
//External flash system info length
#define FLASH_SYS_LENGTH                sizeof(FlashSysInfo_t)
//External flash system info area size
#define FLASH_SYS_AREA_SIZE             FLASH_SECTOR_SIZE * 1

#ifdef BOARD_S1_2

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

#if (defined BOARD_S6_6) || (defined BOARD_S2_2)

//External flash sensor data pointer store position
#define FLASH_SENSOR_PTR_POS            (FLASH_SYS_POS + FLASH_SYS_AREA_SIZE)
//External flash sensor data pointer size
#define FLASH_SENSOR_PTR_SIZE           16
//External flash sensor data pointer number
#define FLASH_SENSOR_PTR_NUMBER         (698624L)           // 2729*4096
//External flash sensor data pointer position offset
#define FLASH_SENSOR_PTR_AREA_SIZE      (FLASH_SENSOR_PTR_SIZE * FLASH_SENSOR_PTR_NUMBER)

//External flash sensor data store position
#define FLASH_SENSOR_DATA_POS           (FLASH_SENSOR_PTR_POS + FLASH_SENSOR_PTR_AREA_SIZE)
//External flash sensor data size
#define FLASH_SENSOR_DATA_SIZE          32
//External flash sensor data number
#define FLASH_SENSOR_DATA_NUMBER        (698624L)         // the max store data   //40 *4096
//External flash sensor data position offset
#define FLASH_SENSOR_DATA_AREA_SIZE     (FLASH_SENSOR_DATA_SIZE * FLASH_SENSOR_DATA_NUMBER)


#endif



//External flash NODE_PARA data pointer store position
#define FLASH_NODE_PARA_PTR_POS              (FLASH_SENSOR_DATA_POS + FLASH_SENSOR_DATA_AREA_SIZE)
//External flash NODE_PARA data pointer size
#define FLASH_NODE_PARA_PTR_SIZE             8
//External flash NODE_PARA data pointer number
#define FLASH_NODE_PARA_PTR_NUMBER           1024           // 4096 * 2 
//External flash NODE_PARA data pointer position offset
#define FLASH_NODE_PARA_PTR_AREA_SIZE        (FLASH_GNSS_PTR_SIZE * FLASH_GNSS_PTR_NUMBER)






#ifdef SUPPORT_GNSS
//External flash GNSS data pointer store position
#define FLASH_GNSS_PTR_POS              (FLASH_SENSOR_DATA_POS + FLASH_SENSOR_DATA_AREA_SIZE)
//External flash GNSS data pointer size
#define FLASH_GNSS_PTR_SIZE             16
//External flash GNSS data pointer number
#define FLASH_GNSS_PTR_NUMBER           1024
//External flash GNSS data pointer position offset
#define FLASH_GNSS_PTR_AREA_SIZE        (FLASH_GNSS_PTR_SIZE * FLASH_GNSS_PTR_NUMBER)
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
extern ErrorStatus Flash_load_sensor_data(uint8_t *pData, uint16_t length, uint8_t offsetUnit);
extern ErrorStatus Flash_load_sensor_data_history(uint8_t *pData, uint16_t length, uint16_t number);
extern void Flash_store_sensor_data(uint8_t *pData, uint16_t length);
extern void Flash_recovery_last_sensor_data(void);
extern void Flash_store_record_addr(uint8_t startOrEnd);
extern FlashPrintRecordAddr_t Flash_get_record_addr(void);
extern void Flash_get_record(uint32_t addr, uint8_t *pData, uint16_t length);
extern uint32_t Flash_get_unupload_items(void);
extern uint32_t Flash_get_record_items(void);
extern ErrorStatus  Falsh_prtpoint_forward(void);

extern ErrorStatus Flash_external_Selftest(void);

#endif          // __EXTFLASH_H__
