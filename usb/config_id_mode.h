//***********************************************************************************
// Copyright 2017-2018, Zksiot Development Ltd.
// Created by yuanGuo, 2018.07.10
// MCU:	MSP430F5529
// OS: TI-RTOS
// Project:
// File name: config_id_mode.h
// Description: used for configure system setting with id mode.
//***********************************************************************************

#ifndef __ZKSIOT_CONFIG_ID_MODE_H__
#define __ZKSIOT_CONFIG_ID_MODE_H__

#include "../general.h"

#ifdef USE_NEW_SYS_CONFIG
#define SYS_CONFIG_USE_ID_MODE 1
#endif

#include "config_id_mode_type.h"

typedef uint16_t (*send_data_callback_func)(uint8_t *pData, uint16_t len);

extern uint16_t sys_Get_Config_Id(uint8_t* pData, uint16_t buf_len, uint8_t id_start, uint8_t id_end, send_data_callback_func send_data_callback);

// return :position witch place parsed
extern uint16_t sys_Set_Config_Id(uint8_t* pData, uint16_t all_data_len);

#endif	/* __ZKSIOT_CONFIG_ID_MODE_H__ */

