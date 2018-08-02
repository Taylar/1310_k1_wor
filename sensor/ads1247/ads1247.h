#ifndef __ZKSIOST_ADS1247_H_
#define __ZKSIOST_ADS1247_H_

#ifdef SUPPORT_DEEPTEMP_PT100

//#define USE_HARDWARE_SPI

#define ADS1247_REF0_RES_1K           1000.0     // 欧姆
#define ADS1247_RESOLUTION_RATIO      0x7FFFFF
#define ADS1247_UNIT_REF0            (ADS1247_REF0_RES_1K * 2.0  / ADS1247_RESOLUTION_RATIO) // 单位：欧姆

/*ADS1248 register address*/
#define ADS1247REG_MUX0               0x00
#define ADS1247REG_VBIAS              0x01
#define ADS1247REG_MUX1               0x02
#define ADS1247REG_SYS0               0x03
#define ADS1247REG_OFC0               0x04
#define ADS1247REG_OFC1               0x05
#define ADS1247REG_OFC2               0x06
#define ADS1247REG_FSC0               0x07
#define ADS1247REG_FSC1               0x08
#define ADS1247REG_FSC2               0x09
#define ADS1247REG_IDAC0              0x0A
#define ADS1247REG_IDAC1              0x0B
#define ADS1247REG_GPIOCFG            0x0C
#define ADS1247REG_GPIODIR            0x0D
#define ADS1247REG_GPIODAT            0x0E

/*ADS1248 command*/
#define ADS1247CMD_WAKEUP             0x00 // Exit power-down mode
#define ADS1247CMD_SLEEP              0x02 // Enter power-down mode
#define ADS1247CMD_SYNC               0x04 // Synchronize ADC conversions
#define ADS1247CMD_RESET              0x06 // Reset to default values
#define ADS1247CMD_NOP                0xFF // No operation
#define ADS1247CMD_RDATA              0x12 // Read data once
#define ADS1247CMD_RDATAC             0x14 // Read data continuous mode 0001 010x (14h, 15h
#define ADS1247CMD_SDATAC             0x16 // Stop read data continuous mode
#define ADS1247CMD_RREG               0x20 // Read from register rrrr
#define ADS1247CMD_WREG               0x40 // Write to register rrrr
#define ADS1247CMD_SYSOCAL            0x60 // System offset calibration
#define ADS1247CMD_SYSGCAL            0x61 // System gain calibration
#define ADS1247CMD_SELFOCAL           0x62 // Self offset calibration

/************************************************************************/
/* ads1248 macroinstruction                                             */
/************************************************************************/
// ADS1248的通道号
// about MUX0: Multiplexer Control Register 0
#define  P_AIN0                       0x00
#define  P_AIN1                       0x08
#define  P_AIN2                       0x10
#define  P_AIN3                       0x18
#define  N_AIN0                       0x00
#define  N_AIN1                       0x01
#define  N_AIN2                       0x02
#define  N_AIN3                       0x03

// about MUX1: Multiplexer Control Register 1
#define  CLK_INTER                   0x00
#define  CLK_EXTER                   0x80
#define  REF_INTER_ALWAYS_ON         0x20  // selecte internal reference and always open
#define  REF_INTER_ALWAYS_OFF        0x00  // selecte internal reference and always off
#define  SELT_REF0                   0x00
#define  SELT_REF1                   0x08
#define  SELT_INTER                  0x10

// about SYS0 : System Control Register 0
// ADS1248支持的增益列表
#define ADC_GAIN_1                    0x00
#define ADC_GAIN_2                    0x10
#define ADC_GAIN_4                    0x20
#define ADC_GAIN_8                    0x30
#define ADC_GAIN_16                   0x40
#define ADC_GAIN_32                   0x50
#define ADC_GAIN_64                   0x60
#define ADC_GAIN_128                  0x70
// ADS1246支持的转换速率列表
#define ADC_SPS_5                     0x00
#define ADC_SPS_10                    0x01
#define ADC_SPS_20                    0x02
#define ADC_SPS_40                    0x03
#define ADC_SPS_80                    0x04
#define ADC_SPS_160                   0x05
#define ADC_SPS_320                   0x06
#define ADC_SPS_640                   0x07
#define ADC_SPS_1000                  0x08
#define ADC_SPS_2000                  0x09

// about IDAC0: IDAC Control Register 0
// ADS1248 输出电流
#define ADS1247_IEXC_OUTPUT_0UA        0x00
#define ADS1247_IEXC_OUTPUT_50UA       0x01
#define ADS1247_IEXC_OUTPUT_100UA      0x02
#define ADS1247_IEXC_OUTPUT_250UA      0x03
#define ADS1247_IEXC_OUTPUT_500UA      0x04
#define ADS1247_IEXC_OUTPUT_750UA      0x05
#define ADS1247_IEXC_OUTPUT_1000UA     0x06
#define ADS1247_IEXC_OUTPUT_1500UA     0x07
#define ADS1247_DOUT_MODULE_ONLY_DOUT  0x00
#define ADS1247_DOUT_MODULE_DOUT_READY 0x08

// about IDAC1: IDAC Control Register 1
// ADS1248 电流输出通道
#define ADS1247_IDAC1_OUTPUT_DISCONNET     0xF0
#define ADS1247_IDAC1_OUTPUT_CHANNEL_AN0   0x00
#define ADS1247_IDAC1_OUTPUT_CHANNEL_AN1   0x10
#define ADS1247_IDAC1_OUTPUT_CHANNEL_AN2   0x20
#define ADS1247_IDAC1_OUTPUT_CHANNEL_AN3   0x30
#define ADS1247_IDAC1_OUTPUT_CHANNEL_IEXC1 0x80
#define ADS1247_IDAC1_OUTPUT_CHANNEL_IEXC2 0x90
#define ADS1247_IDAC2_OUTPUT_DISCONNET     0x0F
#define ADS1247_IDAC2_OUTPUT_CHANNEL_AN0   0x00
#define ADS1247_IDAC2_OUTPUT_CHANNEL_AN1   0x01
#define ADS1247_IDAC2_OUTPUT_CHANNEL_AN2   0x02
#define ADS1247_IDAC2_OUTPUT_CHANNEL_AN3   0x03
#define ADS1247_IDAC2_OUTPUT_CHANNEL_IEXC1 0x08
#define ADS1247_IDAC2_OUTPUT_CHANNEL_IEXC2 0x09


/* ADS1248的引脚定义 */
#define ADS1247_POWER_CTR_PIN         IOID_8

#define ADS1247_DRDY_PIN              IOID_21
#define ADS1247_CS_PIN                IOID_28
#define ADS1247_SPI_CLK_PIN           IOID_9
#define ADS1247_SPI_SIMO_PIN          IOID_18
#define ADS1247_SPI_SOMI_PIN          IOID_20

#define READ_ADS1247_DRDY_PIN_LEVEL() PIN_getInputValue(ADS1247_DRDY_PIN)

#define ENABLE_ADS1247_CS()           PIN_setOutputValue(ads1247PinHandle, ADS1247_CS_PIN, 0)
#define DISENABLE_ADS1247_CS()        PIN_setOutputValue(ads1247PinHandle, ADS1247_CS_PIN, 1)

#define ADS_SPI_CLK_LOW()             PIN_setOutputValue(ads1247PinHandle, ADS1247_SPI_CLK_PIN, 0)
#define ADS_SPI_CLK_HIGH()            PIN_setOutputValue(ads1247PinHandle, ADS1247_SPI_CLK_PIN, 1)

#define ADS_SPI_SIMO_LOW()            PIN_setOutputValue(ads1247PinHandle, ADS1247_SPI_SIMO_PIN, 0)
#define ADS_SPI_SIMO_HIGH()           PIN_setOutputValue(ads1247PinHandle, ADS1247_SPI_SIMO_PIN, 1)

#define READ_ADS_SPI_SOMI_PIN_LEVEL() PIN_getInputValue(ADS1247_SPI_SOMI_PIN)

#define ADS_SPI_CLK_DELAY()           __delay_cycles // 500KHz

#define ADS1247_POWER_ON()            PIN_setOutputValue(ads1247PinHandle, ADS1247_POWER_CTR_PIN, 1)
#define ADS1247_POWER_OFF()           PIN_setOutputValue(ads1247PinHandle, ADS1247_POWER_CTR_PIN, 0)


extern uint8_t ads1247PGAGain;

void ADS1247_IOInit(void);
void ADS1247_ConfigInit(void);
void ADS1247_PowerOn(void);
void ADS1247_PowerOff(void);
void ADS1247_WriteCmd(uint8_t cmd);
void ADS1247_WriteReg(uint8_t regAddr, uint8_t dataByte);
uint8_t ADS1247_ReadReg(uint8_t regAddr);
void ADS1247_SetPGAGainAndDataRate(uint8_t covGain, uint8_t covRate);
void ADS1247_SetReference(uint8_t clkSelected, uint8_t interVrefOnOff, uint8_t refSelected);
void ADS1247_SetInputChannel(uint8_t positiveChannel, uint8_t negativeChannel);
void ADS1247_SetOutputCurrentValue(uint8_t IDAC1Chn, uint8_t IDAC2Chn, uint8_t doutModule, uint8_t IValue);
int32_t ADS1247_StartAConversion(void);
float ADS1247_MeasureOneResistance(void);

#endif

#endif /* ZKS_ADS1248_ADS1248_H_ */
