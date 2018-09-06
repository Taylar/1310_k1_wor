//***********************************************************************************
// Copyright 2017-2018, Zksiot Development Ltd.
// Created by yuanGuo, 2018.07.05
// MCU: MSP430F5529
// OS: TI-RTOS
// Project:
// File name: sys_config.c
// Description: system configure setting file .c file.
//***********************************************************************************

#include "../general.h"
#include "sys_config.h"
#include "extflash.h"

#ifdef USE_NEW_SYS_CONFIG
#define USE_ID_MODE_STORE
#endif

#ifdef USE_ID_MODE_STORE
#include "../usb/config_id_mode.h"
/*************** system config area  define ************/
/*---      |-- FlashSysConfigA ----4KB ---*/
/*---      |-- FlashSysConfigB ----4KB ---*/
// |--CRC(2B)--|--data:length(2B)--|------data(lengthB)------|
/*************** system config  define end ************/

typedef enum _config_area{
    CONFIG_AREA_A,
    CONFIG_AREA_B
}CONFIG_AREA_T;

#define DATA_CRC_ADDR_OFFSET 0
#define DATA_LENGTH_ADDR_OFFSET 2
#define DATA_ADDR_OFFSET    4

static void sys_config_read_config_crc_length(CONFIG_AREA_T config_area, uint16_t *crc, uint16_t *length);
static void sys_config_write_config_crc_length(CONFIG_AREA_T config_area, uint16_t *crc, uint16_t *length);

uint16_t sys_config_area_data_caluc_crc(CONFIG_AREA_T config_area, uint16_t length);
void sys_config_mirror_configA_and_configB(CONFIG_AREA_T from_area, CONFIG_AREA_T to_area, uint16_t length);

void sys_config_write_ID_mode_parser(CONFIG_AREA_T config_area);
void sys_config_read_ID_mode_parser(CONFIG_AREA_T config_area, uint16_t data_length);


static void sys_config_read_data(CONFIG_AREA_T config_area, uint16_t offset, uint8_t*pData, uint16_t length)
{
    uint32_t read_addr_base = 0;
    read_addr_base = sys_config_get_base_addr(config_area);

    Semaphore_pend(spiSemHandle, BIOS_WAIT_FOREVER);
    Flash_external_read(read_addr_base+DATA_ADDR_OFFSET + offset, (uint8_t *)pData, length);
    Semaphore_post(spiSemHandle);

}

static uint32_t sys_config_get_base_addr(CONFIG_AREA_T config_area)
{
    uint32_t read_addr_base = 0;

    if(CONFIG_AREA_A == config_area){
        read_addr_base = FLASH_SYS_CONFIG_A_POS;
    }
    if(CONFIG_AREA_B == config_area){
        read_addr_base = FLASH_SYS_CONFIG_B_POS;
    }

    return read_addr_base;
}

static void sys_config_read_config_crc_length(CONFIG_AREA_T config_area, uint16_t *crc, uint16_t *length)
{
    uint32_t read_addr_base = 0;
    read_addr_base = sys_config_get_base_addr(config_area);

    Semaphore_pend(spiSemHandle, BIOS_WAIT_FOREVER);
    Flash_external_read(read_addr_base, (uint8_t *)crc, sizeof(uint16_t));
    Flash_external_read(read_addr_base+DATA_LENGTH_ADDR_OFFSET, (uint8_t *)length, sizeof(uint16_t));
    Semaphore_post(spiSemHandle);

}

// before use this functin, make sure the area must erase before
static void sys_config_write_config_crc_length(CONFIG_AREA_T config_area, uint16_t *crc, uint16_t *length)
{
    uint32_t read_addr_base = 0;
    read_addr_base = sys_config_get_base_addr(config_area);

    Semaphore_pend(spiSemHandle, BIOS_WAIT_FOREVER);
    Flash_external_write(read_addr_base, (uint8_t *)crc, sizeof(uint16_t));
    Flash_external_write(read_addr_base+DATA_LENGTH_ADDR_OFFSET, (uint8_t *)length, sizeof(uint16_t));
    Semaphore_post(spiSemHandle);

}

uint16_t sys_config_area_data_caluc_crc(CONFIG_AREA_T config_area, uint16_t length)
{
    uint16_t crc = 0;
    uint32_t read_addr_base = 0;
    uint8_t data = 0;
    uint16_t i = 0;

    read_addr_base = sys_config_get_base_addr(config_area);

    SetContinueCRC16();
    Semaphore_pend(spiSemHandle, BIOS_WAIT_FOREVER);
    for(i=0; i < length; i++){
        // one byte by one byte // sometimes need optimize
        Flash_external_read(read_addr_base+DATA_ADDR_OFFSET + i, (uint8_t *)&data, sizeof(uint8_t));
        crc = ContinueCRC16((uint8_t *)&data, sizeof(uint8_t));
    }
    Semaphore_post(spiSemHandle);

    return crc;
}
void sys_config_mirror_configA_and_configB(CONFIG_AREA_T from_area, CONFIG_AREA_T to_area, uint16_t length)
{
    uint8_t data = 0;
    uint16_t i = 0;
    uint32_t from_addr_base = 0;
    uint32_t to_addr_base = 0;

    from_addr_base = sys_config_get_base_addr(from_area);
    to_addr_base = sys_config_get_base_addr(to_area);

    Semaphore_pend(spiSemHandle, BIOS_WAIT_FOREVER);
    Flash_external_erase(to_addr_base, FLASH_EXT_SECTOR_ERASE);

    for(i=0; i < length; i++){
        // R/W one byte by byte
        Flash_external_read(from_addr_base + i, (uint8_t *)&data, sizeof(uint8_t));
        Flash_external_write(to_addr_base + i, (uint8_t *)&data, sizeof(uint8_t));
    }
    Semaphore_post(spiSemHandle);
}

static uint16_t config_store_pos_offset = 0;
static uint32_t config_area_base_addr = 0;

static void config_area_clear_set(CONFIG_AREA_T config_area)
{
    config_store_pos_offset = 0;
    config_area_base_addr = sys_config_get_base_addr(config_area); // FLASH_SYS_CONFIG_A_POS; // default

    Semaphore_pend(spiSemHandle, BIOS_WAIT_FOREVER);
    Flash_external_erase(config_area_base_addr, FLASH_EXT_SECTOR_ERASE);
    Semaphore_post(spiSemHandle);
}

// ret: eq send_len
static uint16_t send_data_flash_config_area_callback(uint8_t* pData, uint16_t buf_len)
{
    Semaphore_pend(spiSemHandle, BIOS_WAIT_FOREVER);
    Flash_external_write(config_area_base_addr + DATA_ADDR_OFFSET + config_store_pos_offset, (uint8_t *)pData, buf_len);
    Semaphore_post(spiSemHandle);

    config_store_pos_offset += buf_len;
    return buf_len;
}
void sys_config_read_ID_mode_parser(CONFIG_AREA_T config_area, uint16_t data_length)
{
    uint8_t pData[128];
    uint16_t offset=0;
    uint8_t pos=0;
    uint8_t id_type, id_length;

    while(offset < data_length)
    {
        // read id
        sys_config_read_data(config_area, offset++, pData+ pos, 1);
        id_type = pData[pos];
        pos++;
        sys_config_read_data(config_area, offset++, pData+ pos, 1);
        id_length = pData[pos];

#ifdef PRINT_DEBUG
        System_printf("id_type:%d, id_length:%d\n", id_type, id_length);
        System_flush();
#endif

        if(id_length > 128-1){    // 错误ID-length
            return;
        }

        if(id_length > 128-pos ){   // 剩余 buffer 不够存储 接下来的数据
            pos -= 1;
            sys_Set_Config_Id( pData, pos);
            offset -= 2;
            pos = 0;
            continue;
        }

        pos++;
        if(id_length>0){
            sys_config_read_data(config_area, offset, pData + pos, id_length);

#ifdef PRINT_DEBUG
            if(id_type == 0x05){
                System_printf("id_type:%d, data:%x\n", id_type, *((uint16_t*)pData + pos));
                System_flush();
            }
#endif

            offset += id_length;
            pos += id_length;
        }

        if(pos > 64){
            sys_Set_Config_Id( pData, pos);
            pos = 0;
        }
    }
    if(pos > 0){
        sys_Set_Config_Id( pData, pos);
        pos = 0;
    }
}

void sys_config_write_ID_mode_parser(CONFIG_AREA_T config_area)
{
    uint8_t pData[150];
    uint16_t buf_len =128;

    uint16_t crc=0, data_length=0;
    uint8_t id_start, id_end;

    id_start = ID_MODE_START_ID;
    id_end = ID_MODE_END_ID;

    config_area_clear_set(config_area); // clear and erase store region

    data_length = sys_Get_Config_Id(pData,  buf_len,  id_start,  id_end , send_data_flash_config_area_callback);

    crc = sys_config_area_data_caluc_crc(config_area, data_length);
    sys_config_write_config_crc_length(config_area, &crc, &data_length);
}

#ifdef USE_NEW_SYS_CONFIG
//***********************************************************************************
// Flash load config parameters.
//***********************************************************************************
void Flash_load_config(void)
{
    uint16_t crcA,crcB,lengthA, lengthB;
    uint16_t crc;

    uint8_t verify_a=0,verify_b=0;

    sys_config_read_config_crc_length(CONFIG_AREA_A, &crcA, &lengthA);

    if((0 != crcA) &&(0xffff != crcA) && (0 != lengthA)){
        crc = sys_config_area_data_caluc_crc(CONFIG_AREA_A, lengthA);
        if((crcA == crc) && (crcA != 0) &&(crc != 0)){
            verify_a = 1;
        }
    }

    sys_config_read_config_crc_length(CONFIG_AREA_B, &crcB, &lengthB);
    if((0 != crcB)&&(0xffff != crcB) && (0 != lengthB)){
        crc = sys_config_area_data_caluc_crc(CONFIG_AREA_B, lengthB);
        if((crcB == crc)&& (crcB != 0) &&(crc != 0)){
            verify_b = 1;
        }
    }

    if(verify_a && verify_b){   // get all good configure
        // parser A or B
        sys_config_read_ID_mode_parser( CONFIG_AREA_A, lengthA);
        return ;
    }

    if((0 == verify_a) &&(0==verify_b)){   // 都没有有效的存储 // 清空配置 返回，其他位置对 swVersion
        // no parser
        memset( (uint8_t *)&g_rSysConfigInfo, 0x0, sizeof(g_rSysConfigInfo));
        return;
    }

    if(verify_a){

        if(lengthA > 0){
        // parser A
        sys_config_read_ID_mode_parser( CONFIG_AREA_A, lengthA);
        // Record BACKUP B
        sys_config_mirror_configA_and_configB(CONFIG_AREA_A, CONFIG_AREA_B, lengthA+4); // COPY WITH crc/length
        }
    }else{ // b valid // recover a
        if(lengthB > 0){
        // parser B
        sys_config_read_ID_mode_parser( CONFIG_AREA_B, lengthB);
        // STORE CONFIG A
        sys_config_mirror_configA_and_configB(CONFIG_AREA_B, CONFIG_AREA_A, lengthB+4); // COPY WITH crc/length
        }
    }
}

//***********************************************************************************
// Flash store config parameters.
//***********************************************************************************
void Flash_store_config(void)
{
    uint16_t crcA=0,lengthA=0;

    sys_config_write_ID_mode_parser(CONFIG_AREA_A);

    sys_config_read_config_crc_length(CONFIG_AREA_A, &crcA, &lengthA);

    // Record BACKUP B
    sys_config_mirror_configA_and_configB(CONFIG_AREA_A, CONFIG_AREA_B, lengthA+4); // COPY WITH crc/length
}
#endif

#else

#ifdef USE_NEW_SYS_CONFIG
//***********************************************************************************
//
// Flash load config parameters.
//
//***********************************************************************************
void Flash_load_config(void)
{
    //Flash_internal_read_byte(FLASH_SYS_CONFIG_POS, (uint8_t *)&g_rSysConfigInfo, sizeof(g_rSysConfigInfo));
    uint16_t crc, crc_verify_a, crc_verify_b;
    uint8_t verify_a=0,verify_b=0;

    Semaphore_pend(spiSemHandle, BIOS_WAIT_FOREVER);

    // A configure
    Flash_external_read(FLASH_SYS_CONFIG_A_POS, (uint8_t *)&g_rSysConfigInfo, sizeof(g_rSysConfigInfo));
    Flash_external_read(FLASH_SYS_CONFIG_A_POS+SYS_CONFIG_VERIFY_ADDR, (uint8_t *)&crc_verify_a, sizeof(crc_verify_a));
    SetContinueCRC16();
    crc = ContinueCRC16((uint8_t *)&g_rSysConfigInfo, sizeof(g_rSysConfigInfo));
    if(crc_verify_a == crc){
        verify_a = 1;
    }
    // B configure
    Flash_external_read(FLASH_SYS_CONFIG_B_POS, (uint8_t *)&g_rSysConfigInfo, sizeof(g_rSysConfigInfo));
    Flash_external_read(FLASH_SYS_CONFIG_B_POS+SYS_CONFIG_VERIFY_ADDR, (uint8_t *)&crc_verify_b, sizeof(crc_verify_b));
    SetContinueCRC16();
    crc = ContinueCRC16((uint8_t *)&g_rSysConfigInfo, sizeof(g_rSysConfigInfo));
    if(crc_verify_b == crc){
        verify_b = 1;
    }
    Semaphore_post(spiSemHandle);

    if(verify_a && verify_b){   // get all good configure
        return ;
    }

    if((0 == verify_a) &&(0==verify_b)){   // 都没有有效的存储 // 清空配置 返回，其他位置对 swVersion
        memset( (uint8_t *)&g_rSysConfigInfo, 0x0, sizeof(g_rSysConfigInfo));
        return;
    }

    if(verify_a){
        Semaphore_pend(spiSemHandle, BIOS_WAIT_FOREVER);

        // Read CONFIG A
        Flash_external_read(FLASH_SYS_CONFIG_A_POS, (uint8_t *)&g_rSysConfigInfo, sizeof(g_rSysConfigInfo));
        Flash_external_read(FLASH_SYS_CONFIG_A_POS+SYS_CONFIG_VERIFY_ADDR, (uint8_t *)&crc_verify_a, sizeof(crc_verify_a));

        // Record BACKUP B
        Flash_external_erase(FLASH_SYS_CONFIG_B_POS, FLASH_EXT_SECTOR_ERASE);
        Flash_external_write(FLASH_SYS_CONFIG_B_POS, (uint8_t *)&g_rSysConfigInfo, sizeof(g_rSysConfigInfo));
        Flash_external_write(FLASH_SYS_CONFIG_B_POS+SYS_CONFIG_VERIFY_ADDR, (uint8_t *)&crc_verify_a, sizeof(crc_verify_a));

        Semaphore_post(spiSemHandle);
    }else{ // b valid // recover a

        Semaphore_pend(spiSemHandle, BIOS_WAIT_FOREVER);
        // STORE CONFIG A
        Flash_external_erase(FLASH_SYS_CONFIG_A_POS, FLASH_EXT_SECTOR_ERASE);
        // write data and crc
        Flash_external_write(FLASH_SYS_CONFIG_A_POS, (uint8_t *)&g_rSysConfigInfo, sizeof(g_rSysConfigInfo));
        Flash_external_write(FLASH_SYS_CONFIG_A_POS+SYS_CONFIG_VERIFY_ADDR, (uint8_t *)&crc_verify_b, sizeof(crc_verify_b));

        Semaphore_post(spiSemHandle);
    }
}

//***********************************************************************************
//
// Flash store config parameters.
//
//***********************************************************************************
void Flash_store_config(void)
{
    uint16_t crc;
    //Flash_internal_write_byte(FLASH_SYS_CONFIG_POS, (uint8_t *)&g_rSysConfigInfo, sizeof(g_rSysConfigInfo));
    SetContinueCRC16();
    crc = ContinueCRC16((uint8_t *)&g_rSysConfigInfo, sizeof(g_rSysConfigInfo));

    Semaphore_pend(spiSemHandle, BIOS_WAIT_FOREVER);
    // STORE CONFIG A
    Flash_external_erase(FLASH_SYS_CONFIG_A_POS, FLASH_EXT_SECTOR_ERASE);
    // write data and crc
    Flash_external_write(FLASH_SYS_CONFIG_A_POS, (uint8_t *)&g_rSysConfigInfo, sizeof(g_rSysConfigInfo));
    Flash_external_write(FLASH_SYS_CONFIG_A_POS+SYS_CONFIG_VERIFY_ADDR, (uint8_t *)&crc, sizeof(crc));

    // BACKUP B
    Flash_external_erase(FLASH_SYS_CONFIG_B_POS, FLASH_EXT_SECTOR_ERASE);
    Flash_external_write(FLASH_SYS_CONFIG_B_POS, (uint8_t *)&g_rSysConfigInfo, sizeof(g_rSysConfigInfo));
    Flash_external_write(FLASH_SYS_CONFIG_B_POS+SYS_CONFIG_VERIFY_ADDR, (uint8_t *)&crc, sizeof(crc));
    Semaphore_post(spiSemHandle);
}
#endif  /* USE_NEW_SYS_CONFIG */

#endif  /* USE_ID_MODE_STORE */

