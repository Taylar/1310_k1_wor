//***********************************************************************************
// Copyright 2017, Zksiot Development Ltd.
// Created by Linjie, 2017.08.08
// MCU:	MSP430F5529
// OS: TI-RTOS
// Project:
// File name: sht2x.h
// Description: SHT2x humiture sensor process routine head.
//***********************************************************************************

#ifndef __ZKSIOT_SHT3X_H__
#define __ZKSIOT_SHT3X_H__








#ifdef SUPPORT_SHT3X




#define Board_SHT3x_ADDR            (0x8A >> 1)

//-- Enumerations -------------------------------------------------------------
// Sensor Commands
typedef enum{
  CMD_READ_SERIALNBR  = 0x3780, // read serial number
  CMD_READ_STATUS     = 0xF32D, // read status register
  CMD_CLEAR_STATUS    = 0x3041, // clear status register
  CMD_HEATER_ENABLE   = 0x306D, // enabled heater
  CMD_HEATER_DISABLE  = 0x3066, // disable heater
  CMD_SOFT_RESET      = 0x30A2, // soft reset
  CMD_MEAS_CLOCKSTR_H = 0x2C06, // measurement: clock stretching, high repeatability
  CMD_MEAS_CLOCKSTR_M = 0x2C0D, // measurement: clock stretching, medium repeatability
  CMD_MEAS_CLOCKSTR_L = 0x2C10, // measurement: clock stretching, low repeatability
  CMD_MEAS_POLLING_H  = 0x2400, // measurement: polling, high repeatability
  CMD_MEAS_POLLING_M  = 0x240B, // measurement: polling, medium repeatability
  CMD_MEAS_POLLING_L  = 0x2416, // measurement: polling, low repeatability
  CMD_MEAS_PERI_05_H  = 0x2032, // measurement: periodic 0.5 mps, high repeatability
  CMD_MEAS_PERI_05_M  = 0x2024, // measurement: periodic 0.5 mps, medium repeatability
  CMD_MEAS_PERI_05_L  = 0x202F, // measurement: periodic 0.5 mps, low repeatability
  CMD_MEAS_PERI_1_H   = 0x2130, // measurement: periodic 1 mps, high repeatability
  CMD_MEAS_PERI_1_M   = 0x2126, // measurement: periodic 1 mps, medium repeatability
  CMD_MEAS_PERI_1_L   = 0x212D, // measurement: periodic 1 mps, low repeatability
  CMD_MEAS_PERI_2_H   = 0x2236, // measurement: periodic 2 mps, high repeatability
  CMD_MEAS_PERI_2_M   = 0x2220, // measurement: periodic 2 mps, medium repeatability
  CMD_MEAS_PERI_2_L   = 0x222B, // measurement: periodic 2 mps, low repeatability
  CMD_MEAS_PERI_4_H   = 0x2334, // measurement: periodic 4 mps, high repeatability
  CMD_MEAS_PERI_4_M   = 0x2322, // measurement: periodic 4 mps, medium repeatability
  CMD_MEAS_PERI_4_L   = 0x2329, // measurement: periodic 4 mps, low repeatability
  CMD_MEAS_PERI_10_H  = 0x2737, // measurement: periodic 10 mps, high repeatability
  CMD_MEAS_PERI_10_M  = 0x2721, // measurement: periodic 10 mps, medium repeatability
  CMD_MEAS_PERI_10_L  = 0x272A, // measurement: periodic 10 mps, low repeatability
  CMD_FETCH_DATA      = 0xE000, // readout measurements for periodic mode
  CMD_R_AL_LIM_LS     = 0xE102, // read alert limits, low set
  CMD_R_AL_LIM_LC     = 0xE109, // read alert limits, low clear
  CMD_R_AL_LIM_HS     = 0xE11F, // read alert limits, high set
  CMD_R_AL_LIM_HC     = 0xE114, // read alert limits, high clear
  CMD_W_AL_LIM_HS     = 0x611D, // write alert limits, high set
  CMD_W_AL_LIM_HC     = 0x6116, // write alert limits, high clear
  CMD_W_AL_LIM_LC     = 0x610B, // write alert limits, low clear
  CMD_W_AL_LIM_LS     = 0x6100, // write alert limits, low set
  CMD_NO_SLEEP        = 0x303E,
}SHT3X_CMD;



// Measurement Repeatability
typedef enum{
  REPEATAB_HIGH,   // high repeatability
  REPEATAB_MEDIUM, // medium repeatability
  REPEATAB_LOW,    // low repeatability
}etRepeatability;



//-- Typedefs -----------------------------------------------------------------
// Status-Register
typedef union {
  uint16_t u16;
  struct{
    #ifdef LITTLE_ENDIAN  // bit-order is little endian
      uint16_t CrcStatus     : 1; // write data checksum status
      uint16_t CmdStatus     : 1; // command status
      uint16_t Reserve0      : 2; // reserved
      uint16_t ResetDetected : 1; // system reset detected
      uint16_t Reserve1      : 5; // reserved
      uint16_t T_Alert       : 1; // temperature tracking alert
      uint16_t RH_Alert      : 1; // humidity tracking alert
      uint16_t Reserve2      : 1; // reserved
      uint16_t HeaterStatus  : 1; // heater status
      uint16_t Reserve3      : 1; // reserved
      uint16_t AlertPending  : 1; // alert pending status
    #else                 // bit-order is big endian
    u16t AlertPending  : 1;
    u16t Reserve3      : 1;
    u16t HeaterStatus  : 1;
    u16t Reserve2      : 1;
    u16t RH_Alert      : 1;
    u16t T_Alert       : 1;
    u16t Reserve1      : 5;
    u16t ResetDetected : 1;
    u16t Reserve0      : 2;
    u16t CmdStatus     : 1;
    u16t CrcStatus     : 1;
    #endif
  }bit;
} regStatus;



// CRC algorithm
#define POLYNOMIAL                  0x131 //P(x)=x^8+x^5+x^4+1 = 100110001

extern void SHT3x_ResetIoInitial(void);

#endif  /* SUPPORT_SHT3X */



#endif	/* __ZKSIOT_SHT3X_H__ */

