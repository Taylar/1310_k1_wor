#include "../general.h"



void WriteOn(void)//打开写保护
{

	I2C_bytewrite_2(0x64, 0x10, 0x80);

	I2C_bytewrite_2(0x64, 0x0f, 0x84);
 
}

void WriteOff(void)//关闭写保护
{
	I2C_bytewrite_2(0x64, 0x10, 0x00);

	I2C_bytewrite_2(0x64, 0x0f, 0x00);
}

Calendar read_time_from_sd30xx(void)//从sd30xx读时间
{
	uint8_t n = 0, s_data[7];
	Calendar calendarRead;

	I2C_start_2();
	I2C_send_byte_2(0x65);
	I2C_ack_receive_2();
	for (n = 0; n < 7; n++)
	{
		s_data[n] = I2C_receive_byte_2();
		if (n == 6)
		{
			I2C_noack_send_2();
	    }
	    I2C_ack_send_2();
  	}
  	I2C_stop_2();
	calendarRead.Seconds    = TransBcdToHex(s_data[0]);
	calendarRead.Minutes    = TransBcdToHex(s_data[1]);
	calendarRead.Hours      = TransBcdToHex(s_data[2] & 0x7f);
	calendarRead.DayOfWeek  = TransBcdToHex(s_data[3]);
	calendarRead.DayOfMonth = TransBcdToHex(s_data[4]);
	calendarRead.Month      = TransBcdToHex(s_data[5]);
	calendarRead.Year       = TransBcdToHex(s_data[6])+2000;

	return calendarRead;
}



void write_time_to_sd30xx(Calendar calendarWrite)//写时间到sd30xx
{
	uint8_t n = 0, s_data[7];

	s_data[0] = TransHexToBcd((uint8_t)(calendarWrite.Seconds));
	s_data[1] = TransHexToBcd((uint8_t)(calendarWrite.Minutes));
	s_data[2] = TransHexToBcd((uint8_t)(calendarWrite.Hours)) | 0x80;
	s_data[3] = TransHexToBcd((uint8_t)(calendarWrite.DayOfWeek));
	s_data[4] = TransHexToBcd((uint8_t)(calendarWrite.DayOfMonth));
	s_data[5] = TransHexToBcd((uint8_t)(calendarWrite.Month));
	s_data[6] = TransHexToBcd((uint8_t)(calendarWrite.Year%100));


	WriteOn();
	  
	I2C_start_2();
	I2C_send_byte_2(0x64);
	I2C_ack_receive_2();
	I2C_send_byte_2(0x00);
	I2C_ack_receive_2();
	for (n = 0; n < 7; n++)
	{
	    I2C_send_byte_2(s_data[n]);
		I2C_ack_receive_2();
	}
	I2C_stop_2();
	
	WriteOff();
}
