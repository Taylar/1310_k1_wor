/************************************************************************
/ Copyright 2015, ShenZhen Shanling Development Ltd.
/ Created by Linjie, 2015.06.02
/ MCU:	PIC18F66J11
/ Project: ShanLing PCS2.2
/ File name: 	soft_I2c_drv.c
/ Description:	soft_I2C driver.
************************************************************************/
#include "../general.h"

#ifdef SOFT_I2C_BUS

#define I2c_delay()		__delay_cycles(10)


const PIN_Config soft_i2c_sda_input_config[] = {
    SOFT_I2C_SDA_PIN   | PIN_INPUT_EN | PIN_PULLUP,    /*          */
    PIN_TERMINATE
};

const PIN_Config soft_i2c_sda_output_config[] = {
    SOFT_I2C_SDA_PIN   | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH  | PIN_PUSHPULL | PIN_DRVSTR_MAX,    /*          */
    PIN_TERMINATE
};

const PIN_Config soft_i2c_scl_input_config[] = {
    SOFT_I2C_SCL_PIN   | PIN_INPUT_EN | PIN_PULLUP,      /*          */
    PIN_TERMINATE
};

const PIN_Config soft_i2c_scl_output_config[] = {
    SOFT_I2C_SCL_PIN   | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH  | PIN_PUSHPULL | PIN_DRVSTR_MAX,      /*          */
    PIN_TERMINATE
};


static PIN_State   softI2cSclPinState, softI2cSdaPinState;
static PIN_Handle  softI2cSclPinHandle = NULL, softI2cSdaPinHandle = NULL;




/*==================================================================
Function:  Send I2C start bit to slave device.

Parameter: void.

Return:    void.
==================================================================*/
void I2C_start(void)
{
	I2C_BUS_OUTPUT;
	I2c_delay();
	I2C_SDA_SET;
	I2C_SCL_SET;
	I2c_delay();
	I2C_SDA_CLR;
	I2c_delay();
	I2C_SCL_CLR;
}

/*==================================================================
Function:  Send I2C 8 bit data to slave device.

Parameter: Value: the data which want to write to slave device.

Return:    void.
==================================================================*/
void I2C_send_byte(uint8_t Value)
{
	uint8_t i;

// Send most significant first
	for(i = 0; i < 8; i++) {
		if (Value & 0x80) {
			I2C_SDA_SET;
		} else {
			I2C_SDA_CLR;
		}

		I2c_delay();
		I2C_SCL_SET;
		I2c_delay();
		Value <<= 1;
		I2C_SCL_CLR;
	}
}

#ifdef I2C_ARRAY_OPERATION
/*==================================================================
Function:  Send I2C acknowledgement bit to slave device.

Parameter: void.

Return:    void.
==================================================================*/
void I2C_ack_send(void)
{
	I2C_SDA_CLR;
	I2c_delay();
	I2C_SCL_SET;
	I2c_delay();
	I2C_SCL_CLR;
	I2c_delay();
	I2C_SDA_SET;
	I2c_delay();
}
#endif

/*==================================================================
Function:  Send I2C unacknowledgement bit to slave device.

Parameter: void.

Return:    void.
==================================================================*/
void I2C_noack_send(void)
{
	I2C_SDA_SET;
	I2c_delay();
	I2C_SCL_SET;
	I2c_delay();
	I2C_SCL_CLR;
	I2c_delay();
}

/*==================================================================
Function:  Receive I2C acknowledgement bit from slave device.

Parameter: void.

Return:		0:	receive acknowledgement failure.
			1:	receive acknowledgement success.
==================================================================*/
Bool I2C_ack_receive(void)
{
	uint8_t WaitCounter = 0;

	I2C_SDA_SET;
	I2C_SDA_INPUT;
	I2c_delay();
	I2C_SCL_SET;
	//I2c_delay();

	while (I2C_SDA) {
		WaitCounter++;
		if (WaitCounter == 255) {
			I2C_SCL_CLR;
			I2c_delay();
			I2C_BUS_OUTPUT;
			return FALSE;
		}
	}

	I2C_SCL_CLR;
	I2C_BUS_OUTPUT;
	I2c_delay();

	return TRUE;
}

/*==================================================================
Function:  Receive I2C 8 bit data from slave device.

Parameter: void.

Return:    Value:	the data which receive from slave device.
==================================================================*/
uint8_t I2C_receive_byte(void)
{
	uint8_t i;
	uint8_t Value = 0;

	I2C_SDA_INPUT;
	I2c_delay();

// Receive most significant first
	for(i = 0; i < 8; i++) {
		Value <<= 1;

		I2C_SCL_SET;
		I2c_delay();
		if (I2C_SDA)
			Value |= 0x01;

		I2C_SCL_CLR;
		I2c_delay();
	}

	I2C_BUS_OUTPUT;
	return Value;
}

/*==================================================================
Function:  Send I2C stop bit to slave device.

Parameter: void.

Return:    void.
==================================================================*/
void I2C_stop(void)
{
	I2C_SDA_CLR;
	I2c_delay();
	I2C_SCL_SET;
	I2c_delay();
	I2C_SDA_SET;
	I2c_delay();
}

/*==================================================================
Function:  I2C send command.

Parameter:	DeviceID:	device's address.
			Addr:		corresponding device register's address.
			Value:		the data which want to write in register.

Return:    ack:	0:	send failure.
				1:	send success.
==================================================================*/
void I2C_bytewrite(uint8_t DeviceID, int16_t Addr, uint8_t Value)
{
// We can write five times, if write failure.
// If ack equal 1, write success, otherwise, write failure.
	uint8_t ack, retrycnt;

	for(retrycnt = 0; retrycnt < I2C_RW_TRY; retrycnt++) {
		ack = TRUE;
		I2C_start();
		
		I2C_send_byte(DeviceID);
		ack &= I2C_ack_receive();
		
		if (Addr >= 0) {
			I2C_send_byte(Addr);
			ack &= I2C_ack_receive();
		}
		
		I2C_send_byte(Value);
		ack &= I2C_ack_receive();
		
		I2C_stop();
		
		if(ack)
			return;
	}
}

/*==================================================================
Function:  I2C receive command.

Parameter:	DeviceID:	device's address.
			Addr:		corresponding device register's address.
Return:    	0:		send failure.
			value:	send success.
==================================================================*/
uint8_t I2C_byteread(uint8_t DeviceID, int16_t Addr)
{
// We can read five times, if read failure.
// If ack equal 1, read success, otherwise, read failure.
	uint8_t ack, retrycnt, value;

	for(retrycnt = 0; retrycnt < I2C_RW_TRY; retrycnt++) {
		ack = TRUE;
// If Addr & 0x80 == 0x80, the sub address is not need.
		if (Addr >= 0) {
			I2C_start();
			I2C_send_byte(DeviceID);
			ack &= I2C_ack_receive();
			I2C_send_byte(Addr);
			ack &= I2C_ack_receive();
		}

		I2C_start();
		I2C_send_byte(DeviceID | 0x01);
		ack &= I2C_ack_receive();
		value = I2C_receive_byte();
		I2C_noack_send();
		I2C_stop();

		if(ack)
			return value;
	}
	return 0;
}

#ifdef I2C_ARRAY_OPERATION
void I2C_arraywrite(uint8_t DeviceID, int16_t Addr, uint8_t *array, uint8_t n)
{
// We can write five times, if write failure.
// If ack equal 1, write success, otherwise, write failure.

	uint8_t i,ack, retrycnt;

	for(retrycnt = 0; retrycnt < I2C_RW_TRY; retrycnt++) {

	    ack = TRUE;
		I2C_start();
		
		I2C_send_byte(DeviceID);
		ack &= I2C_ack_receive();
		if (Addr >= 0) {
			I2C_send_byte(Addr);
			ack &= I2C_ack_receive();
		}
		for (i = 0; i < n; i++) {
			I2C_send_byte(array[i]);
			ack &= I2C_ack_receive();
		}
		I2C_stop();
		
		if(ack)
			return;
	}
}

void I2C_arrayread(uint8_t DeviceID, int16_t Addr, uint8_t *array, uint8_t n)
{
// We can read five times, if read failure.
// If ack equal 1, read success, otherwise, read failure.

	uint8_t i, ack, retrycnt;
	for(retrycnt = 0; retrycnt < I2C_RW_TRY; retrycnt++) {
	    ack = TRUE;
		if (Addr >= 0) {
			I2C_start();
			I2C_send_byte(DeviceID);
			ack &= I2C_ack_receive();
			I2C_send_byte(Addr);
			ack &= I2C_ack_receive();
		}
		I2C_start();
		I2C_send_byte(DeviceID | 0x01);
		ack &= I2C_ack_receive();
		for (i = 0; i < n-1; i++) {
			array[i] = I2C_receive_byte();
			I2C_ack_send();
		}

		array[n-1] = I2C_receive_byte();
		I2C_noack_send();
		I2C_stop();

		if(ack)
			return ;
	}
}
#endif

//***********************************************************************************
//
// I2C master init.
//
//***********************************************************************************
ErrorStatus I2c_init(void)
{
	I2C_BUS_OUTPUT;
	I2C_SCL_OUTPUT;

    return ES_SUCCESS;
}

//***********************************************************************************
//
// I2C master write n bytes.
//
//***********************************************************************************
ErrorStatus I2c_write(uint8_t devAddr, uint8_t *pBuff, uint8_t n)
{

	I2C_arraywrite(devAddr<<1, pBuff[0], pBuff+1, n-1);

    return ES_SUCCESS;
}


//***********************************************************************************
//
// I2C master read n bytes.
//
//***********************************************************************************
ErrorStatus I2c_read(uint8_t devAddr, uint8_t *pBuff, uint8_t n)
{

    I2C_arrayread(devAddr<<1, -1, pBuff, n);
    return ES_SUCCESS;
}


//***********************************************************************************
//
// I2C master read n byte from a register.
//
//***********************************************************************************
ErrorStatus I2c_regRead(uint8_t devAddr, uint8_t regAddr, uint8_t *pBuff, uint8_t n)
{
	I2C_arrayread(devAddr<<1, regAddr, pBuff, n);
    return ES_SUCCESS;
}

#endif // SOFT_I2C_BUS
