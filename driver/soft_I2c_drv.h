/************************************************************************
/ Copyright 2015, ShenZhen Shanling Development Ltd.
/ Created by Linjie, 2015.06.02
/ MCU:	PIC18F66J11
/ Project: ShanLing PCS2.2
/ File name: 	I2c_drv.h
/ Description:	I2C head.
************************************************************************/

#ifndef _I2C_DRV_H_
#define _I2C_DRV_H_

#define SOFT_I2C_SCL_PIN             IOID_19
#define SOFT_I2C_SDA_PIN             IOID_18




#define I2C_SDA				PIN_getInputValue(SOFT_I2C_SDA_PIN)
#define I2C_SCL				PIN_getInputValue(SOFT_I2C_SCL_PIN)

#define I2C_BUS_OUTPUT		if(softI2cSdaPinHandle != NULL)	\
							{	\
						    	PIN_close(softI2cSdaPinHandle);	\
							}	\
						    softI2cSdaPinHandle = PIN_open(&softI2cSdaPinState, soft_i2c_sda_output_config);


#define I2C_SCL_OUTPUT		if(softI2cSclPinHandle != NULL)	\
							{	\
						    	PIN_close(softI2cSclPinHandle);	\
							}	\
						    softI2cSclPinHandle = PIN_open(&softI2cSclPinState, soft_i2c_scl_output_config);

#define I2C_SDA_INPUT		if(softI2cSdaPinHandle != NULL)	\
							{	\
						    	PIN_close(softI2cSdaPinHandle);	\
							}	\
						    softI2cSdaPinHandle = PIN_open(&softI2cSdaPinState, soft_i2c_sda_input_config);

#define I2C_SCL_INPUT		if(softI2cSclPinHandle != NULL)	\
							{	\
						    	PIN_close(softI2cSclPinHandle);	\
							}	\
						    softI2cSclPinHandle = PIN_open(&softI2cSclPinState, soft_i2c_scl_input_config);

#define I2C_SDA_SET		PIN_setOutputValue(softI2cSdaPinHandle, SOFT_I2C_SDA_PIN, 1)
#define I2C_SDA_CLR		PIN_setOutputValue(softI2cSdaPinHandle, SOFT_I2C_SDA_PIN, 0)
#define I2C_SCL_SET		PIN_setOutputValue(softI2cSclPinHandle, SOFT_I2C_SCL_PIN, 1)
#define I2C_SCL_CLR		PIN_setOutputValue(softI2cSclPinHandle, SOFT_I2C_SCL_PIN, 0)

#define I2C_RW_TRY			3

#define I2C_ARRAY_OPERATION

void I2C_start();
void I2C_send_byte(uint8_t Value);
#ifdef I2C_ARRAY_OPERATION
void I2C_ack_send();
void I2C_noack_send();
#endif
Bool I2C_ack_receive();
uint8_t I2C_receive_byte();
void I2C_stop();


void I2C_bytewrite(uint8_t DeviceID, int16_t Addr, uint8_t Value);
uint8_t I2C_byteread(uint8_t DeviceID, int16_t Addr);

#ifdef I2C_ARRAY_OPERATION
void I2C_arraywrite(uint8_t DeviceID, int16_t Addr, uint8_t *array, uint8_t n);
void I2C_arrayread(uint8_t DeviceID, int16_t Addr, uint8_t *array, uint8_t n);
#endif

extern ErrorStatus I2c_init(void);
extern ErrorStatus I2c_write(uint8_t devAddr, uint8_t *pBuff, uint8_t n);
extern ErrorStatus I2c_read(uint8_t devAddr, uint8_t *pBuff, uint8_t n);
extern ErrorStatus I2c_regRead(uint8_t devAddr, uint8_t regAddr, uint8_t *pBuff, uint8_t n);


#endif	/*_I2C_DRV_H_*/
