#include "../general.h"


#define FLASH_EXTERNAL_SELFTEST_ADDR                (8*1024+8)



// board node
#ifdef BOARD_S1_2

#define FLASH_POWER_PIN         IOID_8
#define FLASH_SPI_CS_PIN        IOID_9
#define FLASH_WP_PIN            IOID_11
#define FLASH_HOLD_PIN          IOID_5

const PIN_Config extFlashPinTable[] = {
    FLASH_POWER_PIN | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX,      /*          */
    FLASH_SPI_CS_PIN | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX,    /*          */
    FLASH_WP_PIN | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX,       /*          */
    FLASH_HOLD_PIN | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX,    /*          */
    PIN_TERMINATE
};


#define Flash_spi_enable()      PIN_setOutputValue(extFlashPinHandle, FLASH_SPI_CS_PIN, 0)
#define Flash_spi_disable()     PIN_setOutputValue(extFlashPinHandle, FLASH_SPI_CS_PIN, 1)

#endif

// board gateway
#ifdef BOARD_S2_2

#define FLASH_SPI_CS_PIN        IOID_24
#define FLASH_WP_PIN            IOID_19
#define FLASH_HOLD_PIN          IOID_18



const PIN_Config extFlashPinTable[] = {
    FLASH_SPI_CS_PIN | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX,       /* LED initially off          */
    FLASH_WP_PIN | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX,       /*          */
    FLASH_HOLD_PIN | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX,    /*          */
    PIN_TERMINATE
};

#define Flash_spi_enable()      PIN_setOutputValue(extFlashPinHandle, FLASH_SPI_CS_PIN, 0)
#define Flash_spi_disable()     PIN_setOutputValue(extFlashPinHandle, FLASH_SPI_CS_PIN, 1)

#endif


// board S6_6
#ifdef BOARD_S6_6


#define FLASH_SPI_CS_PIN        IOID_24


const PIN_Config extFlashPinTable[] = {
    FLASH_SPI_CS_PIN | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX,       /* LED initially off          */
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
    buff[1] = HIBYTE(HIWORD(flashAddr));
    buff[2] = LOBYTE(HIWORD(flashAddr));
    buff[3] = HIBYTE(LOWORD(flashAddr));
    buff[4] = LOBYTE(LOWORD(flashAddr));
    // page limit
    pageRemainBytes = PAGE_BYTES - buff[4];
#else
    buff[1] = LOBYTE(HIWORD(flashAddr));
    buff[2] = HIBYTE(LOWORD(flashAddr));
    buff[3] = LOBYTE(LOWORD(flashAddr));
    // page limit
    pageRemainBytes = PAGE_BYTES - buff[3];
#endif

    if (pageRemainBytes < length)
        length = pageRemainBytes;

    // wait chip idle
    while (Flash_external_read_status() & WIP_BIT)
        ;

    // Write enable
    do {
        Flash_external_write_enable();
        __delay_cycles(120);    // 10us
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
    buff[1] = HIBYTE(HIWORD(flashAddr));
    buff[2] = LOBYTE(HIWORD(flashAddr));
    buff[3] = HIBYTE(LOWORD(flashAddr));
    buff[4] = LOBYTE(LOWORD(flashAddr));
#else
    buff[1] = LOBYTE(HIWORD(flashAddr));
    buff[2] = HIBYTE(LOWORD(flashAddr));
    buff[3] = LOBYTE(LOWORD(flashAddr));
#endif
    // wait chip idle
    while (Flash_external_read_status() & WIP_BIT)
        ;

    // Write enable
    do {
        Flash_external_write_enable();
        __delay_cycles(120);    // 10us
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
    pageRemainBytes = PAGE_BYTES - LOBYTE(LOWORD(flashAddr));
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
    buff[1] = HIBYTE(HIWORD(flashAddr));
    buff[2] = LOBYTE(HIWORD(flashAddr));
    buff[3] = HIBYTE(LOWORD(flashAddr));
    buff[4] = LOBYTE(LOWORD(flashAddr));
#else
    buff[1] = LOBYTE(HIWORD(flashAddr));
    buff[2] = HIBYTE(LOWORD(flashAddr));
    buff[3] = LOBYTE(LOWORD(flashAddr));
#endif
    // wait chip idle
    while (Flash_external_read_status() & WIP_BIT)
        ;

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




    sysInfo.swVersion = FW_VERSION;
    sysInfo.printRecordAddr.start = 0xffffffff;//没有开始记录
    sysInfo.printRecordAddr.end =   0xffffffff;
    Flash_external_write(FLASH_SYS_POS, (uint8_t *)&sysInfo, FLASH_SYS_LENGTH);

    rFlashSensorData.ptrDataAddr = 0;
    rFlashSensorData.ptrData.head = FLASH_PTRDATA_VALID;
    rFlashSensorData.ptrData.frontAddr = 0;
    rFlashSensorData.ptrData.rearAddr = 0;
    Flash_external_write(FLASH_SENSOR_PTR_POS, (uint8_t *)&rFlashSensorData.ptrData, sizeof(FlashPointerData_t));



}


//***********************************************************************************
//
// Flash init.
//
//***********************************************************************************
void Flash_init(void)
{
    FlashSysInfo_t sysInfo;


    extFlashPinHandle = PIN_open(&extFlashPinState, extFlashPinTable);

    // Time delay before write instruction.
    Task_sleep(10 * CLOCK_UNIT_MS);

	Semaphore_pend(spiSemHandle, BIOS_WAIT_FOREVER);
#ifdef FLASH_W25Q256FV
    Flash_extended_address_mode(1);
#endif
    sysInfo.printRecordAddr.start = 0xffffffff;
    sysInfo.printRecordAddr.end = 0xffffffff;
    Flash_external_read(FLASH_SYS_POS, (uint8_t *)&sysInfo, FLASH_SYS_LENGTH);
    if (sysInfo.swVersion != FW_VERSION) {
        Flash_reset_data();
    }

    Flash_load_sensor_ptr();
    
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
// Flash get record data.
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
       System_printf("%d, ", zxtTest[i]);
        if(zxtTest[i] != 0xff)
            return ES_ERROR;
    }
    Flash_external_write(FLASH_EXTERNAL_SELFTEST_ADDR, (uint8_t *)test, 16);
    Flash_external_read(FLASH_EXTERNAL_SELFTEST_ADDR, zxtTest, 16);
    for(i = 0; i < 16; i++)
    {
        System_printf("%d, ", zxtTest[i]);

        if(zxtTest[i] != test[i])
            return ES_ERROR;
    }
    System_printf("\n ");
    return ES_SUCCESS;
} 
