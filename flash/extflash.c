#include "../general.h"


#define FLASH_EXTERNAL_SELFTEST_ADDR                (8*1024+8)



// board node
#ifdef BOARD_S3

#define FLASH_POWER_PIN         IOID_8
#define FLASH_SPI_CS_PIN        IOID_9
#define FLASH_WP_PIN            IOID_11
#define FLASH_HOLD_PIN          IOID_5

#define FLASH_SPI_SIMO          IOID_10
#define FLASH_SPI_CLK           IOID_7
#define FLASH_SPI_SOMI          IOID_6

static PIN_Handle  FLASH_SPI_COM_PinHandle = NULL;
static PIN_State   FLASH_SPI_COM_State;

const PIN_Config extFlashComPinTable[] = {
    FLASH_SPI_SIMO | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    FLASH_SPI_SOMI | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    FLASH_SPI_CLK | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    PIN_TERMINATE
};

const PIN_Config extFlashPinTable[] = {
    FLASH_POWER_PIN | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX,      /*          */
    FLASH_SPI_CS_PIN | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX,    /*          */
    FLASH_WP_PIN | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX,       /*          */
    FLASH_HOLD_PIN | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX,    /*          */
    PIN_TERMINATE
};


#define Flash_spi_enable()      do { \
                                       if (FLASH_SPI_COM_PinHandle) { \
                                            PIN_close(FLASH_SPI_COM_PinHandle); \
                                        } \
                                        Spi_open(); \
                                       PIN_setOutputValue(extFlashPinHandle, FLASH_SPI_CS_PIN, 0); \
                                } while(0)
#define Flash_spi_disable()     do { \
                                       Spi_close(); \
                                       PIN_setOutputValue(extFlashPinHandle, FLASH_SPI_CS_PIN, 1); \
                                       FLASH_SPI_COM_PinHandle = PIN_open(&FLASH_SPI_COM_State, extFlashComPinTable); \
                                   }while(0)
#endif

// board gateway
#ifdef BOARD_B2S

#define FLASH_SPI_CS_PIN        IOID_24
#define FLASH_SPI_SIMO          IOID_27
#define FLASH_SPI_CLK           IOID_26
#define FLASH_SPI_SOMI          IOID_25

static PIN_Handle  FLASH_SPI_COM_PinHandle = NULL;
static PIN_State   FLASH_SPI_COM_State;

const PIN_Config extFlashPinTable[] = {
    FLASH_SPI_CS_PIN | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    PIN_TERMINATE
};

const PIN_Config extFlashComPinTable[] = {
    FLASH_SPI_SIMO | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    FLASH_SPI_SOMI | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    FLASH_SPI_CLK | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    PIN_TERMINATE
};

#define Flash_spi_enable()      do { \
                                       if (FLASH_SPI_COM_PinHandle) { \
                                            PIN_close(FLASH_SPI_COM_PinHandle); \
                                        } \
                                        Spi_open(); \
                                       PIN_setOutputValue(extFlashPinHandle, FLASH_SPI_CS_PIN, 0); \
                                } while(0)
#define Flash_spi_disable()     do { \
                                       Spi_close(); \
                                       PIN_setOutputValue(extFlashPinHandle, FLASH_SPI_CS_PIN, 1); \
                                       FLASH_SPI_COM_PinHandle = PIN_open(&FLASH_SPI_COM_State, extFlashComPinTable); \
                                   }while(0)


#endif


// board S6_6
#ifdef BOARD_S6_6


#define FLASH_SPI_CS_PIN        IOID_9
#define FLASH_WP_PIN            IOID_11
#define FLASH_HOLD_PIN          IOID_5


const PIN_Config extFlashPinTable[] = {
    FLASH_SPI_CS_PIN | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX,       /* LED initially off          */
    FLASH_WP_PIN | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX,       /*          */
#ifdef BOARD_CONFIG_DECEIVE
    FLASH_HOLD_PIN | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX,       /*          */
#endif
    PIN_TERMINATE
};

#define Flash_spi_enable()      PIN_setOutputValue(extFlashPinHandle, FLASH_SPI_CS_PIN, 0)
#define Flash_spi_disable()     PIN_setOutputValue(extFlashPinHandle, FLASH_SPI_CS_PIN, 1)

#endif




static PIN_State   extFlashPinState;
static PIN_Handle  extFlashPinHandle;





//QueueDef rFlashGnssQueue;
static FlashSensorData_t rFlashSensorData;


//***********************************************************************************
#ifdef SUPPORT_ALARM_RECORD_QURERY
//static FlashSensorData_t rFlashAlarmInfo;
#endif

#ifdef SUPPORT_DEVICED_STATE_UPLOAD
static FlashSensorData_t rFlashSysRunState;
#endif
bool flash_inited = 0;

#ifdef FLASH_W25Q256FV
//***********************************************************************************
//
// Flash external address mode select 
//
//***********************************************************************************
static void Flash_extended_address_mode(uint8_t is4Byte)
{
    uint8_t buff[1];

    Flash_spi_enable();
    if (is4Byte)
        buff[0] = FLASHCMD_4BYTE_ADDR_MODE;
    else
        buff[0] = FLASHCMD_3BYTE_ADDR_MODE;
    Spi_write(buff, 1);
    Flash_spi_disable();
}

#endif

//***********************************************************************************
//
// Flash external read status register: 
//      return:     status value
//
//***********************************************************************************
static uint8_t Flash_external_read_status(void)
{
    uint8_t buff[1];

    Flash_spi_enable();
    buff[0] = FLASHCMD_R_STATUS;
    Spi_write(buff, 1);
    Spi_read(buff, 1);
    Flash_spi_disable();

    return buff[0];
}

//***********************************************************************************
//
// Flash external write enable cmd 
//
//***********************************************************************************
static void Flash_external_write_enable(void)
{
    uint8_t buff[1];

    Flash_spi_enable();
    buff[0] = FLASHCMD_W_ENABLE;
    Spi_write(buff, 1);
    Flash_spi_disable();
}

//***********************************************************************************
//
// Flash external page program: 
//      flashAddr:  Flash address
//      pData:      Data buff ptr
//      length:     Data buff length, 0 < length <= 256
//
//***********************************************************************************
static void Flash_external_page_program(uint32_t flashAddr, uint8_t *pData, uint16_t length)
{
    uint8_t buff[5];
    uint16_t pageRemainBytes;

    buff[0] = FLASHCMD_WRITE;
    
#ifdef FLASH_W25Q256FV
    buff[1] = HIBYTE_ZKS(HIWORD_ZKS(flashAddr));
    buff[2] = LOBYTE_ZKS(HIWORD_ZKS(flashAddr));
    buff[3] = HIBYTE_ZKS(LOWORD_ZKS(flashAddr));
    buff[4] = LOBYTE_ZKS(LOWORD_ZKS(flashAddr));
    // page limit
    pageRemainBytes = PAGE_BYTES - buff[4];
#else
    buff[1] = LOBYTE_ZKS(HIWORD_ZKS(flashAddr));
    buff[2] = HIBYTE_ZKS(LOWORD_ZKS(flashAddr));
    buff[3] = LOBYTE_ZKS(LOWORD_ZKS(flashAddr));
    // page limit
    pageRemainBytes = PAGE_BYTES - buff[3];
#endif

    if (pageRemainBytes < length)
        length = pageRemainBytes;

    // wait chip idle
    while (Flash_external_read_status() & WIP_BIT)
        __delay_cycles(3000);    // 0.5ms
        // Task_sleep(1);

    // Write enable
    do {
        Flash_external_write_enable();
        __delay_cycles(1625);    // 0.5ms
    } while (!(Flash_external_read_status() & WEL_BIT));

    Flash_spi_enable();
    
#ifdef FLASH_W25Q256FV
    Spi_write(buff, 5);
#else
    Spi_write(buff, 4);
#endif

    Spi_write(pData, length);
    Flash_spi_disable();
}

//***********************************************************************************
//
// Flash external erase: 
//      flashAddr:  Flash address
//      eraseMode:  FLASH_EXT_SEGMENT_ERASE
//                  FLASH_EXT_BANK_ERASE
//                  FLASH_EXT_MASS_ERASE
//
//***********************************************************************************
static void Flash_external_erase(uint32_t flashAddr, uint8_t eraseMode)
{
    uint8_t buff[5];

    if (eraseMode == FLASH_EXT_SECTOR_ERASE) {
        buff[0] = FLASHCMD_SECTOR_ERASE;
    } else if (eraseMode == FLASH_EXT_BLOCK_ERASE) {
        buff[0] = FLASHCMD_BLOCK_ERASE;
    } else if (eraseMode == FLASH_EXT_CHIP_ERASE) {
        buff[0] = FLASHCMD_CHIP_ERASE;
    } else {
        return;
    }
    
#ifdef FLASH_W25Q256FV
    buff[1] = HIBYTE_ZKS(HIWORD_ZKS(flashAddr));
    buff[2] = LOBYTE_ZKS(HIWORD_ZKS(flashAddr));
    buff[3] = HIBYTE_ZKS(LOWORD_ZKS(flashAddr));
    buff[4] = LOBYTE_ZKS(LOWORD_ZKS(flashAddr));
#else
    buff[1] = LOBYTE_ZKS(HIWORD_ZKS(flashAddr));
    buff[2] = HIBYTE_ZKS(LOWORD_ZKS(flashAddr));
    buff[3] = LOBYTE_ZKS(LOWORD_ZKS(flashAddr));
#endif
    // wait chip idle
    while (Flash_external_read_status() & WIP_BIT)
        Task_sleep(1);

    // Write enable
    do {
        Flash_external_write_enable();
        __delay_cycles(1625);    // 0.5us
    } while (!(Flash_external_read_status() & WEL_BIT));

    Flash_spi_enable();
    if (eraseMode == FLASH_EXT_CHIP_ERASE) {
        Spi_write(buff, 1);
    } else {    
    #ifdef FLASH_W25Q256FV
        Spi_write(buff, 5);
    #else
        Spi_write(buff, 4);
    #endif
    }
    Flash_spi_disable();
}

//***********************************************************************************
//
// Flash external write: 
//      flashAddr:  Flash address
//      pData:      Data buff ptr
//      length:     Data buff length
//
//***********************************************************************************
static void Flash_external_write(uint32_t flashAddr, uint8_t *pData, uint16_t length)
{
    uint16_t pageRemainBytes;

    // page limit
    pageRemainBytes = PAGE_BYTES - LOBYTE_ZKS(LOWORD_ZKS(flashAddr));
    if (pageRemainBytes >= length) {
        Flash_external_page_program(flashAddr, pData, length);
    } else {
        Flash_external_page_program(flashAddr, pData, pageRemainBytes);
        flashAddr += pageRemainBytes;
        length -= pageRemainBytes;
        pData += pageRemainBytes;
        while (length > PAGE_BYTES) {
            Flash_external_page_program(flashAddr, pData, PAGE_BYTES);
            flashAddr += PAGE_BYTES;
            length -= PAGE_BYTES;
            pData += PAGE_BYTES;
        }
        Flash_external_page_program(flashAddr, pData, length);
    }
}

//***********************************************************************************
//
// Flash external read: 
//      flashAddr:  Flash address
//      pData:      Data buff ptr
//      length:     Data buff length
//
//***********************************************************************************
static void Flash_external_read(uint32_t flashAddr, uint8_t *pData, uint16_t length)
{
    uint8_t buff[5];
    
    buff[0] = FLASHCMD_READ;
    
#ifdef FLASH_W25Q256FV
    buff[1] = HIBYTE_ZKS(HIWORD_ZKS(flashAddr));
    buff[2] = LOBYTE_ZKS(HIWORD_ZKS(flashAddr));
    buff[3] = HIBYTE_ZKS(LOWORD_ZKS(flashAddr));
    buff[4] = LOBYTE_ZKS(LOWORD_ZKS(flashAddr));
#else
    buff[1] = LOBYTE_ZKS(HIWORD_ZKS(flashAddr));
    buff[2] = HIBYTE_ZKS(LOWORD_ZKS(flashAddr));
    buff[3] = LOBYTE_ZKS(LOWORD_ZKS(flashAddr));
#endif
    // wait chip idle
    while (Flash_external_read_status() & WIP_BIT)
        __delay_cycles(3000);    // 0.5ms

    Flash_spi_enable();
#ifdef FLASH_W25Q256FV    
    Spi_write(buff, 5);
#else
    Spi_write(buff, 4);
#endif
    Spi_read(pData, length);
    Flash_spi_disable();
}


static void Flash_load_sensor_ptr(void);

#ifdef SUPPORT_DEVICED_STATE_UPLOAD
static void Flash_load_deviced_state_ptr(void);
#endif

#ifdef SUPPORT_ALARM_RECORD_QURERY
static void Flash_load_alarm_record_ptr(void);
#endif

//***********************************************************************************
//
// Flash reset data area.
//
//***********************************************************************************
static void Flash_reset_data(void)
{
    FlashSysInfo_t sysInfo;

    Flash_external_erase(FLASH_SYS_POS, FLASH_EXT_SECTOR_ERASE);
    Flash_external_erase(FLASH_SENSOR_PTR_POS, FLASH_EXT_SECTOR_ERASE);
    Flash_external_erase(FLASH_SENSOR_DATA_POS, FLASH_EXT_SECTOR_ERASE);
    Flash_external_erase(FLASH_SENSOR_DATA_POS + FLASH_SENSOR_DATA_AREA_SIZE - FLASH_SECTOR_SIZE, FLASH_EXT_SECTOR_ERASE);



#ifdef SUPPORT_FLASH_LOG    
    Flash_external_erase(FLASH_LOG_POS, FLASH_EXT_SECTOR_ERASE);
#endif

    sysInfo.swVersion = FW_VERSION;
    sysInfo.printRecordAddr.start = 0xffffffff;//娌℃湁寮�濮嬭锟�?
    sysInfo.printRecordAddr.end =   0xffffffff;
    Flash_external_write(FLASH_SYS_POS, (uint8_t *)&sysInfo, FLASH_SYS_LENGTH);

    rFlashSensorData.ptrDataAddr = 0;
    rFlashSensorData.ptrData.head = FLASH_PTRDATA_VALID;
    rFlashSensorData.ptrData.frontAddr = 0;
    rFlashSensorData.ptrData.rearAddr = 0;
    Flash_external_write(FLASH_SENSOR_PTR_POS, (uint8_t *)&rFlashSensorData.ptrData, sizeof(FlashPointerData_t));


#ifdef SUPPORT_DEVICED_STATE_UPLOAD
    rFlashSysRunState.ptrDataAddr = 0;
    rFlashSysRunState.ptrData.head = FLASH_PTRDATA_VALID;
    rFlashSysRunState.ptrData.frontAddr = 0;
    rFlashSysRunState.ptrData.rearAddr = 0;
    Flash_external_write(FLASH_DEVICED_STATE_PTR_POS, (uint8_t *)&rFlashSysRunState.ptrData, sizeof(FlashPointerData_t));
#endif

#ifdef SUPPORT_ALARM_RECORD_QURERY
    Flash_external_erase(FLASH_ALARM_RECODRD_POS, FLASH_EXT_SECTOR_ERASE);
    Flash_external_erase(FLASH_ALARM_RECODRD_POS + FLASH_EXT_SECTOR_ERASE, FLASH_EXT_SECTOR_ERASE);


#endif
}

#ifdef SUPPORT_START_LOGO
uint8_t logo_x = 0; 
#define LOGO_X_MAX  128
#endif
//***********************************************************************************
//
// Flash init.
//
//***********************************************************************************
void Flash_init(void)
{
    FlashSysInfo_t sysInfo;

#ifdef SUPPORT_START_LOGO
    Disp_poweron();
    Lcd_set_font(1, 8, 1);
    Lcd_clear_area(logo_x++, 4);
#endif
    extFlashPinHandle = PIN_open(&extFlashPinState, extFlashPinTable);

    // Time delay before write instruction.
    Task_sleep(10 * CLOCK_UNIT_MS);

	Semaphore_pend(spiSemHandle, BIOS_WAIT_FOREVER);
#ifdef FLASH_W25Q256FV
    Flash_extended_address_mode(1);
#endif
    Semaphore_post(spiSemHandle);

#ifndef   BOARD_CONFIG_DECEIVE
    // Init the config
    if(Flash_load_config() == false)
    {
        Sys_config_reset();
        Flash_store_config();
    }
#endif
    
    Semaphore_pend(spiSemHandle, BIOS_WAIT_FOREVER);
    sysInfo.printRecordAddr.start = 0xffffffff;
    sysInfo.printRecordAddr.end = 0xffffffff;
    Flash_external_read(FLASH_SYS_POS, (uint8_t *)&sysInfo, FLASH_SYS_LENGTH);
    Semaphore_post(spiSemHandle);

    if (sysInfo.swVersion != FW_VERSION) {
        Flash_reset_data();
        g_rSysConfigInfo.swVersion = FW_VERSION;
        Flash_store_config();
    }

    Semaphore_pend(spiSemHandle, BIOS_WAIT_FOREVER);
    Flash_load_sensor_ptr();
#ifdef SUPPORT_DEVICED_STATE_UPLOAD
    Flash_load_deviced_state_ptr();
#endif

#ifdef SUPPORT_ALARM_RECORD_QURERY
    Flash_load_alarm_record_ptr();
#endif
    Semaphore_post(spiSemHandle);
    flash_inited = 1;
    
#ifdef SUPPORT_START_LOGO    
    Lcd_set_font(LOGO_X_MAX, 8, 1);
    Lcd_clear_area(0, 4);        
#endif

    uint8_t ret = 0;
    if(g_rSysConfigInfo.batLowVol > BAT_VOLTAGE_L2) {//淇閰嶇疆璇ユ暟鎹�
        g_rSysConfigInfo.batLowVol = BAT_VOLTAGE_L2;
        ret =  1;

    }

    if(g_rSysConfigInfo.batLowVol < BAT_VOLTAGE_LOW) {//淇閰嶇疆璇ユ暟鎹�
        g_rSysConfigInfo.batLowVol = BAT_VOLTAGE_LOW;
        ret = 1;
    }

    if (ret) {
        Flash_store_config();
    }

#if  defined(SUPPORT_BOARD_OLD_S1) || defined(SUPPORT_BOARD_OLD_S2S_1)
    uint8_t i;
    for (i = 0; i < MODULE_SENSOR_MAX; i++) {
        g_rSysConfigInfo.sensorModule[i]     = SEN_TYPE_NONE;
        g_rSysConfigInfo.alarmTemp[i].high   = ALARM_TEMP_HIGH;
        g_rSysConfigInfo.alarmTemp[i].low    = ALARM_TEMP_LOW;
        g_rSysConfigInfo.WarningTemp[i].high = ALARM_TEMP_HIGH;
        g_rSysConfigInfo.WarningTemp[i].low  = ALARM_TEMP_LOW;
    }

#ifdef SUPPORT_BOARD_OLD_S1
    g_rSysConfigInfo.sensorModule[0] = SEN_TYPE_SHT2X;
#endif //SUPPORT_BOARD_OLD_S1

#ifdef SUPPORT_BOARD_OLD_S2S_1
    g_rSysConfigInfo.sensorModule[0] = SEN_TYPE_DEEPTEMP;
#endif //SUPPORT_BOARD_OLD_S2S_1

    /* 当设为mast时为旧的S1工作模式为模式1，其它模式2*/
    if (g_rSysConfigInfo.rfStatus & STATUS_1310_MASTER) {
        OldS1nodeAPP_setWorkMode(S1_OPERATING_MODE1);
    } else {
        OldS1nodeAPP_setWorkMode(S1_OPERATING_MODE2);
    }
#endif // (defined SUPPORT_BOARD_OLD_S1) || (defined SUPPORT_BOARD_OLD_S2S_1)

#if  defined(BOARD_B2S) && defined(S_C)
    if ((uint8_t)g_rSysConfigInfo.deepTempAdjust == 0xff) {
        g_rSysConfigInfo.deepTempAdjust = 0;
    }
#endif
}

void Flash_reset_all(void)
{
	Semaphore_pend(spiSemHandle, BIOS_WAIT_FOREVER);
    Flash_reset_data();
    Semaphore_post(spiSemHandle);
    
    //Flash_init();
}
void Flash_read_rawdata(uint32_t flashAddr, uint8_t *pData, uint16_t length)
{
	Semaphore_pend(spiSemHandle, BIOS_WAIT_FOREVER);
    Flash_external_read( flashAddr,  pData,  length);
	Semaphore_post(spiSemHandle);
}

//***********************************************************************************
//
// Flash store sensor data pointer.
//
//***********************************************************************************
static void Flash_store_sensor_ptr(void)
{
    //Abolish current ptrData in flash.
    rFlashSensorData.ptrData.head = FLASH_PTRDATA_INVALID;
    Flash_external_write(rFlashSensorData.ptrDataAddr + FLASH_SENSOR_PTR_POS, (uint8_t *)&rFlashSensorData.ptrData.head, sizeof(rFlashSensorData.ptrData.head));

    //Go to next ptrData position.
    rFlashSensorData.ptrDataAddr += FLASH_SENSOR_PTR_SIZE;
    rFlashSensorData.ptrDataAddr %= (FLASH_SENSOR_PTR_SIZE * FLASH_SENSOR_PTR_NUMBER);

    //If the position is the first byte of a sector, clear the sector.
    if ((rFlashSensorData.ptrDataAddr % (FLASH_SECTOR_SIZE)) == 0) {
        Flash_external_erase(rFlashSensorData.ptrDataAddr + FLASH_SENSOR_PTR_POS, FLASH_EXT_SECTOR_ERASE);
    }

    //Store new ptrData to flash.
    rFlashSensorData.ptrData.head = FLASH_PTRDATA_VALID;
    Flash_external_write(rFlashSensorData.ptrDataAddr + FLASH_SENSOR_PTR_POS, (uint8_t *)&rFlashSensorData.ptrData, sizeof(FlashPointerData_t));
}


//***********************************************************************************
//
// Flash load sensor data pointer.
//
//***********************************************************************************
static void Flash_load_sensor_ptr(void)
{
    uint8_t ret;
    uint32_t i;

    ret = ES_ERROR;
    rFlashSensorData.ptrDataAddr = 0;
    for (i = 0; i < FLASH_SENSOR_PTR_NUMBER; i++) {
        Flash_external_read(rFlashSensorData.ptrDataAddr + FLASH_SENSOR_PTR_POS, (uint8_t *)&rFlashSensorData.ptrData, sizeof(FlashPointerData_t));
        if (rFlashSensorData.ptrData.head == FLASH_PTRDATA_VALID) {
            ret = ES_SUCCESS;
            break;
        }        
        rFlashSensorData.ptrDataAddr += FLASH_SENSOR_PTR_SIZE;

        
    #ifdef SUPPORT_START_LOGO
        if( i% 100 == 0){
            Lcd_clear_area(logo_x++, 4);        
            logo_x = logo_x % LOGO_X_MAX;
            if(logo_x == 0) {
                Lcd_set_font(128, 32, 0);
                Lcd_clear_area(0, 2);
                Lcd_set_font(1, 8, 1);
            }
        }
    #endif

    }

    if (ret == ES_ERROR) {
        i = 0;
        while (i < FLASH_SENSOR_PTR_SIZE * FLASH_SENSOR_PTR_NUMBER) {
            Flash_external_erase(FLASH_SENSOR_PTR_POS + i, FLASH_EXT_SECTOR_ERASE);
            i += FLASH_SECTOR_SIZE;
        }
        rFlashSensorData.ptrDataAddr = 0;
        rFlashSensorData.ptrData.head = FLASH_PTRDATA_VALID;
        rFlashSensorData.ptrData.frontAddr = 0;
        rFlashSensorData.ptrData.rearAddr = 0;
        Flash_external_write(FLASH_SENSOR_PTR_POS, (uint8_t *)&rFlashSensorData.ptrData, sizeof(FlashPointerData_t));
    }
}

uint32_t Flash_get_sensor_writeaddr(void)
{
    uint32_t addr;

   Semaphore_pend(spiSemHandle, BIOS_WAIT_FOREVER);
   addr = rFlashSensorData.ptrData.rearAddr;   
   Semaphore_post(spiSemHandle);
   return addr;
}

uint32_t Flash_get_sensor_readaddr(void)
{
    uint32_t addr;

   Semaphore_pend(spiSemHandle, BIOS_WAIT_FOREVER);
   addr = rFlashSensorData.ptrData.frontAddr;   
   Semaphore_post(spiSemHandle);
   return addr;
}

//***********************************************************************************
//
// Flash store one sensor data.
//
//***********************************************************************************
void Flash_store_sensor_data(uint8_t *pData, uint16_t length)
{
    uint32_t addr;

    if (length > FLASH_SENSOR_DATA_SIZE)
        return;

	Semaphore_pend(spiSemHandle, BIOS_WAIT_FOREVER);
    addr = (rFlashSensorData.ptrData.rearAddr + FLASH_SENSOR_DATA_SIZE) % (FLASH_SENSOR_DATA_SIZE * FLASH_SENSOR_DATA_NUMBER);
    if (addr == rFlashSensorData.ptrData.frontAddr) {
        //Data queue full, drop one object data, Data queue frontAddr increase.
        rFlashSensorData.ptrData.frontAddr += FLASH_SENSOR_DATA_SIZE;
        rFlashSensorData.ptrData.frontAddr %= (FLASH_SENSOR_DATA_SIZE * FLASH_SENSOR_DATA_NUMBER);
    }

    //If the position is the first byte of a sector, clear the sector.
    if ((rFlashSensorData.ptrData.rearAddr % (FLASH_SECTOR_SIZE)) == 0) {
        //If frontAddr in the sector which need clear.
        if ((rFlashSensorData.ptrData.frontAddr > rFlashSensorData.ptrData.rearAddr)
            && (rFlashSensorData.ptrData.frontAddr < rFlashSensorData.ptrData.rearAddr + FLASH_SECTOR_SIZE)) {
            //Data queue frontAddr point to next sector first byte.
            rFlashSensorData.ptrData.frontAddr = rFlashSensorData.ptrData.rearAddr + FLASH_SECTOR_SIZE;
            rFlashSensorData.ptrData.frontAddr %= (FLASH_SENSOR_DATA_SIZE * FLASH_SENSOR_DATA_NUMBER);
        }
        Flash_external_erase(rFlashSensorData.ptrData.rearAddr + FLASH_SENSOR_DATA_POS, FLASH_EXT_SECTOR_ERASE);
    }


    //Data queue not empty, dequeue data.
    Flash_external_write(rFlashSensorData.ptrData.rearAddr + FLASH_SENSOR_DATA_POS, pData, length);
    //Data queue rearAddr increase.
    rFlashSensorData.ptrData.rearAddr += FLASH_SENSOR_DATA_SIZE;
    rFlashSensorData.ptrData.rearAddr %= (FLASH_SENSOR_DATA_SIZE * FLASH_SENSOR_DATA_NUMBER);

    //Store sensor ptrData.
    Flash_store_sensor_ptr();
	Semaphore_post(spiSemHandle);
}


//***********************************************************************************
//
// Flash load sensor data.
//
//***********************************************************************************
ErrorStatus Flash_load_sensor_data(uint8_t *pData, uint16_t length)
{
    if (length > FLASH_SENSOR_DATA_SIZE)
        return ES_ERROR;

	Semaphore_pend(spiSemHandle, BIOS_WAIT_FOREVER);
    if (rFlashSensorData.ptrData.rearAddr == rFlashSensorData.ptrData.frontAddr) {
        //Data queue empty.
    	Semaphore_post(spiSemHandle);
        return ES_ERROR;
    }

    //Data queue not empty, dequeue data.
    Flash_external_read(rFlashSensorData.ptrData.frontAddr + FLASH_SENSOR_DATA_POS, pData, length);
    //Data queue front pointer increase.
    //rFlashSensorData.ptrData.frontAddr += FLASH_SENSOR_DATA_SIZE;
    //rFlashSensorData.ptrData.frontAddr %= (FLASH_SENSOR_DATA_SIZE * FLASH_SENSOR_DATA_NUMBER);

    //Store sensor ptrData.
    //Flash_store_sensor_ptr();
	Semaphore_post(spiSemHandle);

    return ES_SUCCESS;
}



//***********************************************************************************
//
// Flash load sensor data by offset.
//
//***********************************************************************************
ErrorStatus Flash_load_sensor_data_by_offset(uint8_t *pData, uint16_t length, uint8_t offset)
{
    uint32_t LoadAddr;

    if (length > FLASH_SENSOR_DATA_SIZE)
        return ES_ERROR;

	Semaphore_pend(spiSemHandle, BIOS_WAIT_FOREVER);

	LoadAddr = rFlashSensorData.ptrData.frontAddr + offset*FLASH_SENSOR_DATA_SIZE;
    LoadAddr %= (FLASH_SENSOR_DATA_SIZE * FLASH_SENSOR_DATA_NUMBER);

    if (rFlashSensorData.ptrData.rearAddr == LoadAddr) {
        //Data queue empty.
    	Semaphore_post(spiSemHandle);
        return ES_ERROR;
    }


    
    //Data queue not empty, dequeue data.
    Flash_external_read(LoadAddr + FLASH_SENSOR_DATA_POS, pData, length);
    
	Semaphore_post(spiSemHandle);

    return ES_SUCCESS;
}


//***********************************************************************************
//
// Flash moveto offset  sensor data.
//
//***********************************************************************************
void Flash_moveto_offset_sensor_data(uint8_t offset)
{
	Semaphore_pend(spiSemHandle, BIOS_WAIT_FOREVER);

    //Data queue front pointer increase.
    rFlashSensorData.ptrData.frontAddr += offset*FLASH_SENSOR_DATA_SIZE;
    rFlashSensorData.ptrData.frontAddr %= (FLASH_SENSOR_DATA_SIZE * FLASH_SENSOR_DATA_NUMBER);

    //Store sensor ptrData.
    Flash_store_sensor_ptr();
	Semaphore_post(spiSemHandle);
}

//***********************************************************************************
//
// Flash moveto next last sensor data.
//
//***********************************************************************************
void Flash_moveto_next_sensor_data(void)
{
	Semaphore_pend(spiSemHandle, BIOS_WAIT_FOREVER);

    //Data queue front pointer increase.
    rFlashSensorData.ptrData.frontAddr += FLASH_SENSOR_DATA_SIZE;
    rFlashSensorData.ptrData.frontAddr %= (FLASH_SENSOR_DATA_SIZE * FLASH_SENSOR_DATA_NUMBER);

    //Store sensor ptrData.
    Flash_store_sensor_ptr();
	Semaphore_post(spiSemHandle);
}


//***********************************************************************************
//
// Flash recovery last sensor data.
//
//***********************************************************************************
void Flash_recovery_last_sensor_data(void)
{
	Semaphore_pend(spiSemHandle, BIOS_WAIT_FOREVER);
    //Data queue front pointer decrease.
    rFlashSensorData.ptrData.frontAddr += (FLASH_SENSOR_DATA_SIZE * FLASH_SENSOR_DATA_NUMBER);
    rFlashSensorData.ptrData.frontAddr -= FLASH_SENSOR_DATA_SIZE;
    rFlashSensorData.ptrData.frontAddr %= (FLASH_SENSOR_DATA_SIZE * FLASH_SENSOR_DATA_NUMBER);

    //Store sensor ptrData.
    Flash_store_sensor_ptr();
	Semaphore_post(spiSemHandle);

}

//***********************************************************************************
//
// Flash load sensor history data.
//
//***********************************************************************************
ErrorStatus Flash_load_sensor_data_history(uint8_t *pData, uint16_t length, uint32_t number)
{
    uint32_t historyAddr;

    if (length > FLASH_SENSOR_DATA_SIZE)
        length = FLASH_SENSOR_DATA_SIZE;

	Semaphore_pend(spiSemHandle, BIOS_WAIT_FOREVER);
	#if 0
	historyAddr = (rFlashSensorData.ptrData.frontAddr + (FLASH_SENSOR_DATA_SIZE * FLASH_SENSOR_DATA_NUMBER) - number*FLASH_SENSOR_DATA_SIZE)
					% (FLASH_SENSOR_DATA_SIZE * FLASH_SENSOR_DATA_NUMBER);
	#else
	historyAddr =  (number % FLASH_SENSOR_DATA_NUMBER)*FLASH_SENSOR_DATA_SIZE;
	#endif
    Flash_external_read(historyAddr + FLASH_SENSOR_DATA_POS, pData, length);
	Semaphore_post(spiSemHandle);

    return ES_SUCCESS;
}

//***********************************************************************************
//
// Flash store record address.
//      startOrEnd:  0 is end record flash address
//                   1 is start record flash address
//
//***********************************************************************************
void Flash_store_record_addr(uint8_t startOrEnd)
{
    FlashSysInfo_t sysInfo;

	Semaphore_pend(spiSemHandle, BIOS_WAIT_FOREVER);
    Flash_external_read(FLASH_SYS_POS, (uint8_t *)&sysInfo, FLASH_SYS_LENGTH);
    if (startOrEnd) {
        sysInfo.printRecordAddr.start = rFlashSensorData.ptrData.rearAddr;
        sysInfo.printRecordAddr.end = rFlashSensorData.ptrData.rearAddr;
    } else {
        sysInfo.printRecordAddr.end = rFlashSensorData.ptrData.rearAddr;
        if(sysInfo.printRecordAddr.start == sysInfo.printRecordAddr.end)
        {
            sysInfo.printRecordAddr.start = 0xffffffff;
            sysInfo.printRecordAddr.end   = 0xffffffff;
        }
    }
    Flash_external_erase(FLASH_SYS_POS, FLASH_EXT_SECTOR_ERASE);
    Flash_external_write(FLASH_SYS_POS, (uint8_t *)&sysInfo, FLASH_SYS_LENGTH);
	Semaphore_post(spiSemHandle);
}

//***********************************************************************************
//
// Flash get record address.
//
//***********************************************************************************
FlashPrintRecordAddr_t Flash_get_record_addr(void)
{
    FlashSysInfo_t sysInfo;

	Semaphore_pend(spiSemHandle, BIOS_WAIT_FOREVER);
    Flash_external_read(FLASH_SYS_POS, (uint8_t *)&sysInfo, FLASH_SYS_LENGTH);
	Semaphore_post(spiSemHandle);

    return sysInfo.printRecordAddr;
}

//***********************************************************************************
//
//***********************************************************************************
//
// Flash get System run state data and set alarm recode.
//
//***********************************************************************************
#ifdef SUPPORT_DEVICED_STATE_UPLOAD
//***********************************************************************************
//
// Flash store sensor data pointer.
//
//***********************************************************************************
static void Flash_store_deviced_state_ptr(void)
{
    //Abolish current ptrData in flash.
    rFlashSysRunState.ptrData.head = FLASH_PTRDATA_INVALID;
    Flash_external_write(rFlashSysRunState.ptrDataAddr + FLASH_DEVICED_STATE_PTR_POS, (uint8_t *)&rFlashSysRunState.ptrData.head, sizeof(rFlashSysRunState.ptrData.head));

    //Go to next ptrData position.
    rFlashSysRunState.ptrDataAddr += FLASH_DEVICED_STATE_PTR_SIZE;
    rFlashSysRunState.ptrDataAddr %= (FLASH_DEVICED_STATE_PTR_SIZE * FLASH_DEVICED_STATE_PTR_NUMBER);

    //If the position is the first byte of a sector, clear the sector.
    if ((rFlashSysRunState.ptrDataAddr % (FLASH_SECTOR_SIZE)) == 0) {
        Flash_external_erase(rFlashSysRunState.ptrDataAddr + FLASH_DEVICED_STATE_PTR_POS, FLASH_EXT_SECTOR_ERASE);
    }

    //Store new ptrData to flash.
    rFlashSysRunState.ptrData.head = FLASH_PTRDATA_VALID;
    Flash_external_write(rFlashSysRunState.ptrDataAddr + FLASH_DEVICED_STATE_PTR_POS, (uint8_t *)&rFlashSysRunState.ptrData, sizeof(FlashPointerData_t));
}


//***********************************************************************************
//
// Flash load sensor data pointer.
//
//***********************************************************************************
static void Flash_load_deviced_state_ptr(void)
{
    uint8_t ret;
    uint32_t i;

    ret = ES_ERROR;
    rFlashSysRunState.ptrDataAddr = 0;
    for (i = 0; i < FLASH_DEVICED_STATE_PTR_NUMBER; i++) {
        Flash_external_read(rFlashSysRunState.ptrDataAddr + FLASH_DEVICED_STATE_PTR_POS, (uint8_t *)&rFlashSysRunState.ptrData, sizeof(FlashPointerData_t));
        if (rFlashSysRunState.ptrData.head == FLASH_PTRDATA_VALID) {
            ret = ES_SUCCESS;
            break;
        }
        rFlashSysRunState.ptrDataAddr += FLASH_DEVICED_STATE_PTR_SIZE;
    }


    if (ret == ES_ERROR) {
        i = 0;
        while (i < FLASH_DEVICED_STATE_PTR_SIZE * FLASH_DEVICED_STATE_PTR_NUMBER) {
            Flash_external_erase(FLASH_DEVICED_STATE_PTR_POS + i, FLASH_EXT_SECTOR_ERASE);
            i += FLASH_SECTOR_SIZE;
        }
        rFlashSysRunState.ptrDataAddr = 0;
        rFlashSysRunState.ptrData.head = FLASH_PTRDATA_VALID;
        rFlashSysRunState.ptrData.frontAddr = 0;
        rFlashSysRunState.ptrData.rearAddr = 0;
        Flash_external_write(FLASH_DEVICED_STATE_PTR_POS, (uint8_t *)&rFlashSysRunState.ptrData, sizeof(FlashPointerData_t));
    }
}


//***********************************************************************************
//
// Flash store one sensor data.
//
//***********************************************************************************
void Flash_store_deviced_state_data(uint8_t *pData, uint16_t length)
{
    uint32_t addr;

    if (length > FLASH_DEVICED_STATE_DATA_SIZE)
        return;

    Semaphore_pend(spiSemHandle, BIOS_WAIT_FOREVER);
    addr = (rFlashSysRunState.ptrData.rearAddr + FLASH_DEVICED_STATE_DATA_SIZE) % (FLASH_DEVICED_STATE_DATA_SIZE * FLASH_DEVICED_STATE_DATA_NUMBER);
    if (addr == rFlashSysRunState.ptrData.frontAddr) {
        //Data queue full, drop one object data, Data queue frontAddr increase.
        rFlashSysRunState.ptrData.frontAddr += FLASH_DEVICED_STATE_DATA_SIZE;
        rFlashSysRunState.ptrData.frontAddr %= (FLASH_DEVICED_STATE_DATA_SIZE * FLASH_DEVICED_STATE_DATA_NUMBER);
    }

    //If the position is the first byte of a sector, clear the sector.
    if ((rFlashSysRunState.ptrData.rearAddr % (FLASH_SECTOR_SIZE)) == 0) {
        //If frontAddr in the sector which need clear.
        if ((rFlashSysRunState.ptrData.frontAddr > rFlashSysRunState.ptrData.rearAddr)
            && (rFlashSysRunState.ptrData.frontAddr < rFlashSysRunState.ptrData.rearAddr + FLASH_SECTOR_SIZE)) {
            //Data queue frontAddr point to next sector first byte.
            rFlashSysRunState.ptrData.frontAddr = rFlashSysRunState.ptrData.rearAddr + FLASH_SECTOR_SIZE;
            rFlashSysRunState.ptrData.frontAddr %= (FLASH_DEVICED_STATE_DATA_SIZE * FLASH_DEVICED_STATE_DATA_NUMBER);
        }
        Flash_external_erase(rFlashSysRunState.ptrData.rearAddr + FLASH_DEVICED_STATE_DATA_POS, FLASH_EXT_SECTOR_ERASE);
    }


    //Data queue not empty, dequeue data.
    Flash_external_write(rFlashSysRunState.ptrData.rearAddr + FLASH_DEVICED_STATE_DATA_POS, pData, length);
    //Data queue rearAddr increase.
    rFlashSysRunState.ptrData.rearAddr += FLASH_DEVICED_STATE_DATA_SIZE;
    rFlashSysRunState.ptrData.rearAddr %= (FLASH_DEVICED_STATE_DATA_SIZE * FLASH_DEVICED_STATE_DATA_NUMBER);

    //Store sensor ptrData.
    Flash_store_deviced_state_ptr();
    Semaphore_post(spiSemHandle);
}


//***********************************************************************************
//
// Flash store one sensor data.
//
//***********************************************************************************
void Flash_store_devices_state(uint8_t StateType){
    //采集时间
    Calendar calendar;
    uint8_t  index = 1;
    uint8_t buff[FLASH_DEVICED_STATE_DATA_SIZE];

        
    buff[index ++] = StateType;
    calendar       = Rtc_get_calendar();
    buff[index ++] = 0x20;
    buff[index ++] = TransHexToBcd(calendar.Year - CALENDAR_BASE_YEAR);
    buff[index ++] = TransHexToBcd(calendar.Month);
    buff[index ++] = TransHexToBcd(calendar.DayOfMonth);
    buff[index ++] = TransHexToBcd(calendar.Hours);
    buff[index ++] = TransHexToBcd(calendar.Minutes);
    buff[index ++] = TransHexToBcd(calendar.Seconds);

#ifdef SUPPORT_BLUETOOTH_PRINT
    uint32_t num;
    if(StateType == TYPE_BT_PRINT_END){
        num = Btp_GetPrintNum();
        buff[index ++]  =  HIBYTE(HIWORD(num));
        buff[index ++]  =  LOBYTE(HIWORD(num));
        buff[index ++]  =  HIBYTE(LOWORD(num));
        buff[index ++]  =  LOBYTE(LOWORD(num));
    }
#endif
    buff[index]     =  0;
    buff[0]  = index;

    Flash_store_deviced_state_data((uint8_t *)buff,FLASH_DEVICED_STATE_DATA_SIZE);

}

//***********************************************************************************
//
// Flash load sensor data.
//
//***********************************************************************************
ErrorStatus Flash_load_deviced_state_data(uint8_t *pData, uint16_t length)
{
    if (length > FLASH_DEVICED_STATE_DATA_SIZE)
        return ES_ERROR;

    Semaphore_pend(spiSemHandle, BIOS_WAIT_FOREVER);
    if (rFlashSysRunState.ptrData.rearAddr == rFlashSysRunState.ptrData.frontAddr) {
        //Data queue empty.
        Semaphore_post(spiSemHandle);
        return ES_ERROR;
    }

    //Data queue not empty, dequeue data.
    Flash_external_read(rFlashSysRunState.ptrData.frontAddr + FLASH_DEVICED_STATE_DATA_POS, pData, length);
    //Data queue front pointer increase.
    //rFlashAlarmInfo.ptrData.frontAddr += FLASH_G7_ALARM_DATA_SIZE;
    //rFlashAlarmInfo.ptrData.frontAddr %= (FLASH_G7_ALARM_DATA_SIZE * FLASH_G7_ALARM_DATA_NUMBER);

    //Store sensor ptrData.
    //Flash_store_g7_alarm_ptr();
    Semaphore_post(spiSemHandle);

    return ES_SUCCESS;
}


//***********************************************************************************
//
// Flash moveto next last sensor data.
//
//***********************************************************************************
void Flash_moveto_next_deviced_state_data(void)
{
    Semaphore_pend(spiSemHandle, BIOS_WAIT_FOREVER);

    //Data queue front pointer increase.
    rFlashSysRunState.ptrData.frontAddr += FLASH_DEVICED_STATE_DATA_SIZE;
    rFlashSysRunState.ptrData.frontAddr %= (FLASH_DEVICED_STATE_DATA_SIZE * FLASH_DEVICED_STATE_DATA_NUMBER);

    //Store sensor ptrData.
    Flash_store_deviced_state_ptr();
    Semaphore_post(spiSemHandle);
}


//***********************************************************************************
//
// Flash get un-upload items.
//
//***********************************************************************************
uint32_t Flash_get_deviced_state_items(void)
{
    uint32_t  num;

    Semaphore_pend(spiSemHandle, BIOS_WAIT_FOREVER);

    if (rFlashSysRunState.ptrData.frontAddr > rFlashSysRunState.ptrData.rearAddr)
        num =  FLASH_DEVICED_STATE_DATA_NUMBER - (rFlashSysRunState.ptrData.frontAddr - rFlashSysRunState.ptrData.rearAddr) / FLASH_DEVICED_STATE_DATA_SIZE;
    else
        num =  (rFlashSysRunState.ptrData.rearAddr - rFlashSysRunState.ptrData.frontAddr) / FLASH_DEVICED_STATE_DATA_SIZE;

    Semaphore_post(spiSemHandle);

    return num;
}


#endif
//***********************************************************************************
//
// Flash get one record data.
//
//***********************************************************************************
void Flash_get_record(uint32_t addr, uint8_t *pData, uint16_t length)
{
	Semaphore_pend(spiSemHandle, BIOS_WAIT_FOREVER);
    Flash_external_read(addr + FLASH_SENSOR_DATA_POS, pData, length);
	Semaphore_post(spiSemHandle);
}

//***********************************************************************************
//
// Flash get un-upload items.
//
//***********************************************************************************
uint32_t Flash_get_unupload_items(void)
{
    uint32_t  num;
    
	Semaphore_pend(spiSemHandle, BIOS_WAIT_FOREVER);

    if (rFlashSensorData.ptrData.frontAddr > rFlashSensorData.ptrData.rearAddr)
        num =  FLASH_SENSOR_DATA_NUMBER - (rFlashSensorData.ptrData.frontAddr - rFlashSensorData.ptrData.rearAddr) / FLASH_SENSOR_DATA_SIZE;
    else
        num =  (rFlashSensorData.ptrData.rearAddr - rFlashSensorData.ptrData.frontAddr) / FLASH_SENSOR_DATA_SIZE;
    
	Semaphore_post(spiSemHandle);

    return num;
}

//***********************************************************************************
//
// Flash get record item numbers.
//
//***********************************************************************************
uint32_t Flash_get_record_items(void)
{
    FlashSysInfo_t sysInfo;

	Semaphore_pend(spiSemHandle, BIOS_WAIT_FOREVER);
    Flash_external_read(FLASH_SYS_POS, (uint8_t *)&sysInfo, FLASH_SYS_LENGTH);
	Semaphore_post(spiSemHandle);

    if ((sysInfo.printRecordAddr.start == sysInfo.printRecordAddr.end) &&
		sysInfo.printRecordAddr.start != 0xffffffff)
        sysInfo.printRecordAddr.end = rFlashSensorData.ptrData.rearAddr;

    if (sysInfo.printRecordAddr.start > sysInfo.printRecordAddr.end)
        return FLASH_SENSOR_DATA_NUMBER - (sysInfo.printRecordAddr.start - sysInfo.printRecordAddr.end) / FLASH_SENSOR_DATA_SIZE;

    return (sysInfo.printRecordAddr.end - sysInfo.printRecordAddr.start) / FLASH_SENSOR_DATA_SIZE;
}


#ifdef SUPPORT_NETGATE_DISP_NODE

//***********************************************************************************
//
// Flash load sensor codec 
//
//***********************************************************************************
uint16_t Flash_load_sensor_codec( uint32_t deviceid)
{
    uint32_t id;
    uint16_t i;
    
    Semaphore_pend(spiSemHandle, BIOS_WAIT_FOREVER);
    
    for (i = 0; i < FLASH_SENSOR_CODEC_NUM; ++i) {
        Flash_external_read(FLASH_SENSOR_CODEC_POS + i*FLASH_SENSOR_CODEC_SIZE, (uint8_t*)&id, FLASH_SENSOR_CODEC_SIZE);

        if(deviceid == id){            
            Semaphore_post(spiSemHandle);
            return i;
        }
    }    

    Semaphore_post(spiSemHandle);
    return 0;
}

//***********************************************************************************
//
// Flash store sensor codec.
//
//***********************************************************************************
void Flash_store_sensor_codec(uint16_t no, uint32_t deviceid)
{
#if 0//脦麓脢碌脧脰
    uint8_t tmp[FLASH_SECTOR_SIZE];
    Semaphore_pend(spiSemHandle, BIOS_WAIT_FOREVER);

    Flash_external_read(FLASH_SENSOR_CODEC_POS, tmp, FLASH_SECTOR_SIZE);
    
    Flash_external_erase(FLASH_SENSOR_CODEC_POS, FLASH_EXT_SECTOR_ERASE);

    *(uint32_t*)&tmp[no*FLASH_SENSOR_CODEC_SIZE] = deviceid;
    
    Flash_external_write(FLASH_SENSOR_CODEC_POS, tmp, FLASH_SECTOR_SIZE);

    Semaphore_post(spiSemHandle);
#endif    
}
#endif

static void Flash_external_erase2(uint32_t flashAddr, uint8_t eraseMode);

#ifdef SUPPORT_FLASH_LOG
void Flash_log(uint8_t *log)
{
    static uint32_t log_pos = FLASH_LOG_AREA_SIZE;
    uint8_t startsec, endsec, i = 0;
    uint8_t buff[FLASH_LOG_SIZE];
    Calendar currentTime;
    uint8_t len = strlen((const char *)log);
    
    #define TIME_LEN  16

    if(!flash_inited)return;

    
    Semaphore_pend(spiSemHandle, BIOS_WAIT_FOREVER);

    if(log_pos >= FLASH_LOG_AREA_SIZE){//find writeable position
        
        log_pos = 0;

        while(log_pos < FLASH_LOG_AREA_SIZE){
            
            Flash_external_read(FLASH_LOG_POS + log_pos, (uint8_t*)&buff, FLASH_LOG_SIZE);

            for(i = 0; i < FLASH_LOG_SIZE; ++i){

                if(buff[i] == 0xff){//there is  no data, as write position.
                    break;        
                }       
            }

            if( i < FLASH_LOG_SIZE ){//there is  no data, as write position.
                log_pos += i;
                break;
            }        
            else{
                log_pos += FLASH_LOG_SIZE;
            }
        }

        if(log_pos >= FLASH_LOG_AREA_SIZE){//there is  no space , move to first section.           
            log_pos = 0;
        }        
    }


    #if 1//use time 
    
    if((log_pos + TIME_LEN) >= FLASH_LOG_AREA_SIZE){//there is  no enough space , move to first section.          
        log_pos = 0;
    }

    startsec = log_pos / FLASH_SECTOR_SIZE;
    endsec = (log_pos + TIME_LEN) / FLASH_SECTOR_SIZE;

    if((log_pos % FLASH_SECTOR_SIZE) == 0){//first write in the section, earse first
        //earse...
        Flash_external_erase2(FLASH_LOG_POS + startsec*FLASH_SECTOR_SIZE, FLASH_EXT_SECTOR_ERASE);
    }
        
    while(startsec++ < endsec){//erase all need  section
         //earse...
        Flash_external_erase2(FLASH_LOG_POS + startsec*FLASH_SECTOR_SIZE, FLASH_EXT_SECTOR_ERASE);
    }
    
    currentTime            = Rtc_get_calendar();        
    currentTime.Year       = TransHexToBcd(currentTime.Year%100)+0x2000;
    currentTime.Month      = TransHexToBcd(currentTime.Month);
    currentTime.DayOfMonth = TransHexToBcd(currentTime.DayOfMonth);
    currentTime.Hours      = TransHexToBcd(currentTime.Hours);
    currentTime.Minutes    = TransHexToBcd(currentTime.Minutes);
    currentTime.Seconds    = TransHexToBcd(currentTime.Seconds);
    sprintf((char*)buff, "T%04x%02x%02x%02x%02x%02x:",currentTime.Year,
                                               currentTime.Month, 
                                               currentTime.DayOfMonth,
                                               currentTime.Hours,
                                               currentTime.Minutes,
                                               currentTime.Seconds);
    Flash_external_write(FLASH_LOG_POS + log_pos, buff, TIME_LEN);
    log_pos+= TIME_LEN;    

    #endif

    if(len >= 255) len = 255;//max length of string  no more 255
       
    if((log_pos + len) >= FLASH_LOG_AREA_SIZE){//there is  no enough space , move to  first section.  
        log_pos = 0;
    }

    startsec = log_pos / FLASH_SECTOR_SIZE;
    endsec = (log_pos + len) / FLASH_SECTOR_SIZE;

    if((log_pos % FLASH_SECTOR_SIZE) == 0){//first write in the section, earse first
        //earse...
        Flash_external_erase2(FLASH_LOG_POS + startsec*FLASH_SECTOR_SIZE, FLASH_EXT_SECTOR_ERASE);
    }
    
    while(startsec++ < endsec){//erase all need  section
         //earse...
        Flash_external_erase2(FLASH_LOG_POS + startsec*FLASH_SECTOR_SIZE, FLASH_EXT_SECTOR_ERASE);
    }
        
    Flash_external_write(FLASH_LOG_POS + log_pos, log, len);
    log_pos += len;

    
    Semaphore_post(spiSemHandle);
}
#endif 
   
//***********************************************************************************
//
// Flash load alarm reocord data.
//
//***********************************************************************************
#ifdef SUPPORT_ALARM_RECORD_QURERY
static uint32_t alarm_pos = FLASH_ALARM_RECODRD_POS;

static void Flash_load_alarm_record_ptr(void)
{
        uint8_t flag = 0xff;
        //uint8_t temp[16] ={0};
        //uint8_t index ;


        Flash_external_read(alarm_pos, &flag, 1);
        //Flash_external_erase(FLASH_ALARM_RECODRD_POS, FLASH_EXT_SECTOR_ERASE);
        while(flag != 0xff){
            alarm_pos+= FLASH_ALARM_RECODRD_SIZE;
            Flash_external_read(alarm_pos, &flag, 1);
            if(alarm_pos == FLASH_ALARM_RECODRD_POS +FLASH_ALARM_RECODRD_AREA_SIZE)
                break;
        }

}
//***********************************************************************************
//
//Flash store alarm record.
//
//***********************************************************************************
void Flash_store_alarm_record(uint8_t *buff, uint8_t len)
{

    uint8_t temp[16] ={0};
    uint8_t index ;
    if(!flash_inited)return;

    if(len > FLASH_ALARM_RECODRD_SIZE)
        len = FLASH_ALARM_RECODRD_SIZE;

    Semaphore_pend(spiSemHandle, BIOS_WAIT_FOREVER);

    if(alarm_pos == (FLASH_ALARM_RECODRD_POS + FLASH_ALARM_RECODRD_AREA_SIZE)){

        Flash_external_erase(FLASH_ALARM_RECODRD_POS, FLASH_EXT_SECTOR_ERASE);

        for(index = 1 ;index < ALARM_RECORD_QURERY_MAX_ITEM; index ++){

           Flash_external_read(alarm_pos - FLASH_ALARM_RECODRD_SIZE*index, temp, FLASH_ALARM_RECODRD_SIZE);
           Flash_external_write(FLASH_ALARM_RECODRD_POS + (index -1)*FLASH_ALARM_RECODRD_SIZE,temp,FLASH_ALARM_RECODRD_SIZE);

        }
        Flash_external_erase(FLASH_ALARM_RECODRD_POS + FLASH_EXT_SECTOR_ERASE, FLASH_EXT_SECTOR_ERASE);

        alarm_pos = FLASH_ALARM_RECODRD_POS + FLASH_ALARM_RECODRD_SIZE*(ALARM_RECORD_QURERY_MAX_ITEM - 1);
    }

    Flash_external_write(alarm_pos, buff, len);


    alarm_pos+= FLASH_ALARM_RECODRD_SIZE;

    Semaphore_post(spiSemHandle);
}

//***********************************************************************************
//
// Flash loard alarm reocord data.
//
//***********************************************************************************
ErrorStatus Flash_load_alarm_record_by_offset(uint8_t *buff, uint8_t length,uint8_t offset)
{
    uint32_t LoadAddr;
    if ((length > FLASH_ALARM_RECODRD_SIZE) || (offset > 10) )
        return ES_ERROR;



    LoadAddr = alarm_pos - offset*FLASH_ALARM_RECODRD_SIZE;
    if(LoadAddr < FLASH_ALARM_RECODRD_POS )
        return ES_ERROR;

    //Data queue not empty, dequeue data.
    Semaphore_pend(spiSemHandle, BIOS_WAIT_FOREVER);
    Flash_external_read(LoadAddr, buff, FLASH_ALARM_RECODRD_SIZE);

    Semaphore_post(spiSemHandle);

    return ES_SUCCESS;
}

//***********************************************************************************
//
// Flash get alarm reocord data items.
//
//***********************************************************************************
uint8_t Flash_get_alarm_record_items(void){
    uint8_t num;
    if((alarm_pos - FLASH_ALARM_RECODRD_POS)/FLASH_ALARM_RECODRD_SIZE >= ALARM_RECORD_QURERY_MAX_ITEM){

        num = ALARM_RECORD_QURERY_MAX_ITEM;
    }else{

        num = (alarm_pos - FLASH_ALARM_RECODRD_POS)/FLASH_ALARM_RECODRD_SIZE;
    }

    return num;
}
#endif

//***********************************************************************************
//
// Flash store upgrade data.
//
//***********************************************************************************
void Flash_store_upgrade_data(uint32_t addr, uint8_t *pData, uint16_t length)
{
    Semaphore_pend(spiSemHandle, BIOS_WAIT_FOREVER);

    //If the position is the first byte of a sector, clear the sector.
    if ((addr % (FLASH_SECTOR_SIZE)) == 0) {
        //If frontAddr in the sector which need clear.
        
        Flash_external_erase(addr + FLASH_UPGRADE_DATA_POS, FLASH_EXT_SECTOR_ERASE);
    }

    // Check if the write length is greater than the remaining space in the sector
    if(length > (FLASH_SECTOR_SIZE - (addr % FLASH_SECTOR_SIZE))){
        // Cross-sector erase the required space behind
        uint32_t i;
        for(i=0; i < length; i+=FLASH_SECTOR_SIZE){
            Flash_external_erase(addr+i+FLASH_SECTOR_SIZE + FLASH_UPGRADE_DATA_POS, FLASH_EXT_SECTOR_ERASE);
        }
    }
    //
    Flash_external_write(addr + FLASH_UPGRADE_DATA_POS, pData, length);

    Semaphore_post(spiSemHandle);
}


//***********************************************************************************
//
// Flash load upgrade data.
//
//***********************************************************************************
ErrorStatus Flash_load_upgrade_data(uint32_t addr, uint8_t *pData, uint16_t length)
{
    Semaphore_pend(spiSemHandle, BIOS_WAIT_FOREVER);

    //
    Flash_external_read(addr + FLASH_UPGRADE_DATA_POS, pData, length);

    Semaphore_post(spiSemHandle);

    return ES_SUCCESS;
}


//***********************************************************************************
//
// Flash store upgrade info.
//
//***********************************************************************************
void Flash_store_upgrade_info(uint8_t *pData, uint16_t length)
{
    Semaphore_pend(spiSemHandle, BIOS_WAIT_FOREVER);

        
    Flash_external_erase(FLASH_UPGRADE_INFO_POS, FLASH_EXT_SECTOR_ERASE);
    //
    Flash_external_write(FLASH_UPGRADE_INFO_POS, pData, length);

    Semaphore_post(spiSemHandle);
}


//***********************************************************************************
//
// Flash load upgrade info.
//
//***********************************************************************************
ErrorStatus Flash_load_upgrade_info(uint8_t *pData, uint16_t length)
{
    Semaphore_pend(spiSemHandle, BIOS_WAIT_FOREVER);

    //
    Flash_external_read(FLASH_UPGRADE_INFO_POS, pData, length);

    Semaphore_post(spiSemHandle);

    return ES_SUCCESS;
}


//***********************************************************************************
//
// Flash external erase: 
//      flashAddr:  Flash address
//      eraseMode:  FLASH_EXT_SEGMENT_ERASE
//                  FLASH_EXT_BANK_ERASE
//                  FLASH_EXT_MASS_ERASE
//
//***********************************************************************************
static void Flash_external_erase2(uint32_t flashAddr, uint8_t eraseMode)
{
    uint8_t buff[5];

    if (eraseMode == FLASH_EXT_SECTOR_ERASE) {
        buff[0] = FLASHCMD_SECTOR_ERASE;
    } else if (eraseMode == FLASH_EXT_BLOCK_ERASE) {
        buff[0] = FLASHCMD_BLOCK_ERASE;
    } else if (eraseMode == FLASH_EXT_CHIP_ERASE) {
        buff[0] = FLASHCMD_CHIP_ERASE;
    } else {
        return;
    }
    
#ifdef FLASH_W25Q256FV
    buff[1] = HIBYTE_ZKS(HIWORD_ZKS(flashAddr));
    buff[2] = LOBYTE_ZKS(HIWORD_ZKS(flashAddr));
    buff[3] = HIBYTE_ZKS(LOWORD_ZKS(flashAddr));
    buff[4] = LOBYTE_ZKS(LOWORD_ZKS(flashAddr));
#else
    buff[1] = LOBYTE_ZKS(HIWORD_ZKS(flashAddr));
    buff[2] = HIBYTE_ZKS(LOWORD_ZKS(flashAddr));
    buff[3] = LOBYTE_ZKS(LOWORD_ZKS(flashAddr));
#endif
    // wait chip idle
    while (Flash_external_read_status() & WIP_BIT)
        __delay_cycles(3000);    // 0.5us
        // Task_sleep(1);

    // Write enable
    do {
        Flash_external_write_enable();
        __delay_cycles(1625);    // 0.5us
    } while (!(Flash_external_read_status() & WEL_BIT));

    Flash_spi_enable();
    if (eraseMode == FLASH_EXT_CHIP_ERASE) {
        Spi_write(buff, 1);
    } else {    
    #ifdef FLASH_W25Q256FV
        Spi_write(buff, 5);
    #else
        Spi_write(buff, 4);
    #endif
    }
    Flash_spi_disable();
}
#define     CONFIG_VALID_FLAG                   "valid config"

const uint8_t configFlag[12] = CONFIG_VALID_FLAG;
//***********************************************************************************
// brief: store the config from the ext flash   
// 
// note: every sector is 4K
// parameter: 
//***********************************************************************************
void Flash_store_config(void)
{
    Semaphore_pend(spiSemHandle, BIOS_WAIT_FOREVER);

    Flash_external_erase2(FLASH_SYS_CONFIG_DATA_POS, FLASH_EXT_SECTOR_ERASE);
    Flash_external_write(FLASH_SYS_CONFIG_DATA_POS, (uint8_t*)(&g_rSysConfigInfo), sizeof(ConfigInfo_t));


    Flash_external_erase2(FLASH_SYS_CONFIG_BAK_DATA_POS, FLASH_EXT_SECTOR_ERASE);
    Flash_external_write(FLASH_SYS_CONFIG_BAK_DATA_POS, (uint8_t*)(&g_rSysConfigInfo), sizeof(ConfigInfo_t));

    Semaphore_post(spiSemHandle);
}

//***********************************************************************************
// brief: load the config from the ext flash   
// 
// parameter: 
//***********************************************************************************
bool Flash_load_config(void)
{
    Semaphore_pend(spiSemHandle, BIOS_WAIT_FOREVER);
    Flash_external_read(FLASH_SYS_CONFIG_DATA_POS, (uint8_t*)&g_rSysConfigInfo, sizeof(ConfigInfo_t));
    Semaphore_post(spiSemHandle);

    if(g_rSysConfigInfo.size == 0 || g_rSysConfigInfo.size == 0xffff)
    {
        goto ReadBakConfig;
    }
    return true;

ReadBakConfig:

    Semaphore_pend(spiSemHandle, BIOS_WAIT_FOREVER);
    Flash_external_read(FLASH_SYS_CONFIG_BAK_DATA_POS, (uint8_t*)&g_rSysConfigInfo, sizeof(ConfigInfo_t));
    Semaphore_post(spiSemHandle);
    if(g_rSysConfigInfo.size == 0 || g_rSysConfigInfo.size == 0xffff)
        return false;
    else
        return true;
}


//***********************************************************************************
// brief: reset the config data  
// 
// parameter: 
//***********************************************************************************
void Sys_config_reset(void)
{
    uint8_t i;

    g_rSysConfigInfo.size = sizeof(ConfigInfo_t);
    g_rSysConfigInfo.swVersion = FW_VERSION;
    g_rSysConfigInfo.DeviceId[0] = (uint8_t)((DECEIVE_ID_DEFAULT>>24)&0xff);
    g_rSysConfigInfo.DeviceId[1] = (uint8_t)((DECEIVE_ID_DEFAULT>>16)&0xff);
    g_rSysConfigInfo.DeviceId[2] = (uint8_t)((DECEIVE_ID_DEFAULT>>8)&0xff);
    g_rSysConfigInfo.DeviceId[3] = (uint8_t)((DECEIVE_ID_DEFAULT)&0xff);;

    g_rSysConfigInfo.customId[0] = (uint8_t)(CUSTOM_ID_DEFAULT >> 8);
    g_rSysConfigInfo.customId[1] = (uint8_t)(CUSTOM_ID_DEFAULT);

    g_rSysConfigInfo.status = 0;

    for (i = 0; i < MODULE_SENSOR_MAX; i++) {
        g_rSysConfigInfo.sensorModule[i]     = SEN_TYPE_NONE;
        g_rSysConfigInfo.alarmTemp[i].high   = ALARM_TEMP_HIGH;
        g_rSysConfigInfo.alarmTemp[i].low    = ALARM_TEMP_LOW;
        g_rSysConfigInfo.WarningTemp[i].high = ALARM_TEMP_HIGH;
        g_rSysConfigInfo.WarningTemp[i].low  = ALARM_TEMP_LOW;
    }

#ifdef      BOARD_B2S

    g_rSysConfigInfo.module          = MODULE_NWK | MODULE_RADIO;
    g_rSysConfigInfo.serverIpAddr[0] = 114;
    g_rSysConfigInfo.serverIpAddr[1] = 215;
    g_rSysConfigInfo.serverIpAddr[2] = 122;
    g_rSysConfigInfo.serverIpAddr[3] = 32;
    g_rSysConfigInfo.serverIpPort    = 12200;

    g_rSysConfigInfo.batLowVol       = BAT_VOLTAGE_LOW;
    g_rSysConfigInfo.apnuserpwd[0]   = 0;
    g_rSysConfigInfo.hbPeriod        = UPLOAD_PERIOD_DEFAULT;     // unit is sec
    g_rSysConfigInfo.rfStatus       |= STATUS_1310_MASTER;
#endif

#ifdef      BOARD_S6_6
    #ifdef S_G //缃戝叧
    g_rSysConfigInfo.module          = MODULE_NWK | MODULE_RADIO | MODULE_LCD;
    g_rSysConfigInfo.serverIpAddr[0] = 114;
    g_rSysConfigInfo.serverIpAddr[1] = 215;
    g_rSysConfigInfo.serverIpAddr[2] = 122;
    g_rSysConfigInfo.serverIpAddr[3] = 32;
    g_rSysConfigInfo.serverIpPort    = 12200;
    g_rSysConfigInfo.rfStatus       |= STATUS_1310_MASTER;
    #endif //S_G

    #ifdef S_A//涓�浣撴満
    g_rSysConfigInfo.sensorModule[0] = SEN_TYPE_SHT2X;
    #endif //S_A
    
    g_rSysConfigInfo.batLowVol       = BAT_VOLTAGE_LOW;
    g_rSysConfigInfo.apnuserpwd[0]   = 0;
    g_rSysConfigInfo.hbPeriod        = UPLOAD_PERIOD_DEFAULT;     // unit is sec
    // g_rSysConfigInfo.sensorModule[0] = SEN_TYPE_SHT2X;
    // g_rSysConfigInfo.sensorModule[1] = SEN_TYPE_OPT3001;
#endif


#ifdef      BOARD_S3

    g_rSysConfigInfo.module          = MODULE_RADIO;
    g_rSysConfigInfo.batLowVol       = BAT_VOLTAGE_LOW;
    g_rSysConfigInfo.apnuserpwd[0]   = 0;
    g_rSysConfigInfo.rfStatus        = 0;
    g_rSysConfigInfo.sensorModule[0] = SEN_TYPE_SHT2X;
#endif


    g_rSysConfigInfo.collectPeriod   = COLLECT_PERIOD_DEFAULT;   //unit is sec
    
    g_rSysConfigInfo.uploadPeriod    = UPLOAD_PERIOD_DEFAULT; // unit is sec
    
    g_rSysConfigInfo.ntpPeriod       = NTC_DEFAULT;    // 
    

    
    g_rSysConfigInfo.rtc.Seconds = 1;
    g_rSysConfigInfo.rtc.Minutes = 1;
    g_rSysConfigInfo.rtc.Hours   = 1;
    g_rSysConfigInfo.rtc.DayOfMonth = 28;
    g_rSysConfigInfo.rtc.Month  = 6;
    g_rSysConfigInfo.rtc.Year   = 2018;
}



//***********************************************************************************
//
// Flash external testSelf: 
//  
//***********************************************************************************
static const uint8_t test[16] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};

ErrorStatus Flash_external_Selftest(void)
{
    uint8_t zxtTest[16], i;
    Flash_external_erase(FLASH_EXTERNAL_SELFTEST_ADDR, FLASH_EXT_SECTOR_ERASE);
    Flash_external_read(FLASH_EXTERNAL_SELFTEST_ADDR, zxtTest, 16);
    for(i = 0; i < 16; i++)
    {
       //System_printf("%d, ", zxtTest[i]);
        if(zxtTest[i] != 0xff)
            return ES_ERROR;
    }
    Flash_external_write(FLASH_EXTERNAL_SELFTEST_ADDR, (uint8_t *)test, 16);
    Flash_external_read(FLASH_EXTERNAL_SELFTEST_ADDR, zxtTest, 16);
    for(i = 0; i < 16; i++)
    {
        //System_printf("%d, ", zxtTest[i]);

        if(zxtTest[i] != test[i])
            return ES_ERROR;
    }
    //System_printf("\n ");
    return ES_SUCCESS;
} 

#ifdef SUPPORT_STORE_ID_IN_EXTFLASH
//***********************************************************************************
//
// Flash store the node id in extflash.
//
//***********************************************************************************
void Flash_store_nodeid(uint8_t *pData, uint32_t offset)
{
    Semaphore_pend(spiSemHandle, BIOS_WAIT_FOREVER);
    if(offset >= FLASH_NODEID_STORE_NUMBER)
        return;
    offset = offset * FLASH_NODEID_STORE_SIZE;
    //If the position is the first byte of a sector, clear the sector.
    if ((offset % (FLASH_SECTOR_SIZE)) == 0) {
        Flash_external_erase(FLASH_NODEID_STORE_POS + offset , FLASH_EXT_SECTOR_ERASE);
    }

    Flash_external_write(FLASH_NODEID_STORE_POS + offset, pData, FLASH_NODEID_STORE_SIZE);

    Semaphore_post(spiSemHandle);
}


//***********************************************************************************
//
// Flash load upgrade info.
//
//***********************************************************************************
void Flash_load_nodeid(uint8_t *pData, uint32_t offset)
{
    Semaphore_pend(spiSemHandle, BIOS_WAIT_FOREVER);

    //
    Flash_external_read(FLASH_NODEID_STORE_POS + (offset * FLASH_NODEID_STORE_SIZE), pData, FLASH_NODEID_STORE_SIZE);

    Semaphore_post(spiSemHandle);
}
#endif  //SUPPORT_STORE_ID_IN_EXTFLASH
