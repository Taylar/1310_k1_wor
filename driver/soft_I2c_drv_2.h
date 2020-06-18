/************************************************************************
/ Copyright 2015, ShenZhen Shanling Development Ltd.
/ Created by Linjie, 2015.06.02
/ MCU:	PIC18F66J11
/ Project: ShanLing PCS2.2
/ File name: 	I2c_drv.h
/ Description:	I2C head.
************************************************************************/

#ifndef _I2C_DRV_2_H_
#define _I2C_DRV_2_H_

#define SOFT_I2C_SCL_PIN_2             IOID_3
#define SOFT_I2C_SDA_PIN_2             IOID_2





#define I2C_SDA_2				PIN_getInputValue(SOFT_I2C_SDA_PIN_2)
#define I2C_SCL_2				PIN_getInputValue(SOFT_I2C_SCL_PIN_2)

#define I2C_BUS_OUTPUT_2		if(softI2cSdaPinHandle_2 != NULL)	\
							{	\
						    	PIN_close(softI2cSdaPinHandle_2);	\
							}	\
						    softI2cSdaPinHandle_2 = PIN_open(&softI2cSdaPinState_2, soft_i2c_sda_output_config_2);


#define I2C_SCL_OUTPUT_2		if(softI2cSclPinHandle_2 != NULL)	\
							{	\
						    	PIN_close(softI2cSclPinHandle_2);	\
							}	\
						    softI2cSclPinHandle_2 = PIN_open(&softI2cSclPinState_2, soft_i2c_scl_output_config_2);

#define I2C_SDA_INPUT_2		if(softI2cSdaPinHandle_2 != NULL)	\
							{	\
						    	PIN_close(softI2cSdaPinHandle_2);	\
							}	\
						    softI2cSdaPinHandle_2 = PIN_open(&softI2cSdaPinState_2, soft_i2c_sda_input_config_2);

#define I2C_SCL_INPUT_2		if(softI2cSclPinHandle_2 != NULL)	\
							{	\
						    	PIN_close(softI2cSclPinHandle_2);	\
							}	\
						    softI2cSclPinHandle_2 = PIN_open(&softI2cSclPinState_2, soft_i2c_scl_input_config_2);

#define I2C_SDA_SET_2		PIN_setOutputValue(softI2cSdaPinHandle_2, SOFT_I2C_SDA_PIN_2, 1)
#define I2C_SDA_CLR_2		PIN_setOutputValue(softI2cSdaPinHandle_2, SOFT_I2C_SDA_PIN_2, 0)
#define I2C_SCL_SET_2		PIN_setOutputValue(softI2cSclPinHandle_2, SOFT_I2C_SCL_PIN_2, 1)
#define I2C_SCL_CLR_2		PIN_setOutputValue(softI2cSclPinHandle_2, SOFT_I2C_SCL_PIN_2, 0)

#define I2C_RW_TRY_2			3

#define I2C_ARRAY_OPERATION

void I2C_start_2();
void I2C_send_byte_2(uint8_t Value);
#ifdef I2C_ARRAY_OPERATION
void I2C_ack_send_2();
void I2C_noack_send_2();
#endif
Bool I2C_ack_receive_2();
uint8_t I2C_receive_byte_2();
void I2C_stop_2();


void I2C_bytewrite_2(uint8_t DeviceID, int16_t Addr, uint8_t Value);
uint8_t I2C_byteread_2(uint8_t DeviceID, int16_t Addr);

#ifdef I2C_ARRAY_OPERATION
void I2C_arraywrite_2(uint8_t DeviceID, int16_t Addr, uint8_t *array, uint8_t n);
void I2C_arrayread_2(uint8_t DeviceID, int16_t Addr, uint8_t *array, uint8_t n);
#endif

extern ErrorStatus I2c_init_2(void);
extern ErrorStatus I2c_write_2(uint8_t devAddr, uint8_t *pBuff, uint8_t n);
extern ErrorStatus I2c_read_2(uint8_t devAddr, uint8_t *pBuff, uint8_t n);
extern ErrorStatus I2c_regRead_2(uint8_t devAddr, uint8_t regAddr, uint8_t *pBuff, uint8_t n);


#endif	/*_I2C_DRV_H_*/
