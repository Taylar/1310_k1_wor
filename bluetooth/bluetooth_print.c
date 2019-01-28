//***********************************************************************************
// Copyright 2017, Zksiot Development Ltd.
// Created by Linjie, 2017.09.13
// MCU:	MSP430F5529
// OS: TI-RTOS
// Project:
// File name: bluetooth_print.c
// Description: bluetooth print process routine.
//***********************************************************************************
#include "../general.h"

#ifdef SUPPORT_WATCHDOG
extern Watchdog_Handle watchdogHandle;
#endif

#ifdef SUPPORT_BLUETOOTH_PRINT

#define BT_POWER_PIN            IOID_23

#define BT_CONNECT_PIN          IOID_1


static Semaphore_Struct btpAckSemStruct;
static Semaphore_Handle btpAckSemHandle;

static uint8_t bBtpAckState;
static uint8_t bBtpCmdTpye;
static uint8_t bBtpDeviceName[DEVICE_NAME_SIZE];
static uint8_t bBtpDeviceMac[12];
#ifdef  SUPPORT_DEVICED_STATE_UPLOAD
static uint32_t Btp_print_num;
#endif
#ifdef SUPPORT_WATCHDOG
extern Watchdog_Handle watchdogHandle;
#endif

uint8_t PrintProStep = 0;
FlashPrintRecordAddr_t recordAddr_blue_print;
uint8_t sensorNum,fgPrint;

static const PIN_Config blueControlPinConfig[] = {
    BT_CONNECT_PIN  | PIN_INPUT_EN | PIN_PULLDOWN,
    BT_POWER_PIN    | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_NOPULL,
    PIN_TERMINATE
};

PIN_State   blueControlPinState;
PIN_Handle  blueControlPinHandle;

//***********************************************************************************
//
// Bluetooth print send AT command.
//
//***********************************************************************************
void Btp_send_data(uint8_t *pData, uint16_t length)
{
    UInt key;

    bBtpAckState = BTP_ACK_WAIT;

    /* Disable preemption. */
    key = Hwi_disable();
    g_rUart1RxData.length = 0;
    Hwi_restore(key);

    Uart_send_burst_data(UART_0, pData, length);
}

//***********************************************************************************
//
// Bluetooth print send AT command.
//
//***********************************************************************************
void Btp_send_cmd(uint8_t *string)
{
    UInt key;

    bBtpAckState = BTP_ACK_WAIT;

    /* Disable preemption. */
    key = Hwi_disable();
    g_rUart1RxData.length = 0;
    Hwi_restore(key);

    Uart_send_string(UART_0, string);
}

//***********************************************************************************
//
// Bluetooth print wait cmd ack.
//
//***********************************************************************************
void Btp_wait_ack(uint32_t timeout)
{
    Semaphore_reset(btpAckSemHandle, 0);
    Semaphore_pend(btpAckSemHandle, timeout * CLOCK_UNIT_MS);
}

//***********************************************************************************
//
// Bluetooth print command respond parse.
//
//***********************************************************************************
void Btp_respond_parse(uint8_t *pBuf)
{
    char *ptr;
    uint16_t i = 0;

    switch (bBtpCmdTpye) {
        case BTP_CMD_COMMON:
            ptr = strstr((char *)pBuf, "OK");
            if (ptr != NULL) {
                bBtpAckState = BTP_ACK_OK;
                Semaphore_post(btpAckSemHandle);
                break;
            }
            ptr = strstr((char *)pBuf, "ERROR");
            if (ptr != NULL) {
                bBtpAckState = BTP_ACK_ERROR;
                Semaphore_post(btpAckSemHandle);
            }
            break;

        case BTP_CMD_GET_DEVICE_NAME:
            ptr = strstr((char *)pBuf, "OK");
            if (ptr != NULL) {
                bBtpAckState = BTP_ACK_OK;
                ptr = strstr((char *)pBuf, "=");
                if (ptr != NULL) {
                    ptr++;
                    while (*ptr != '\r' && i < DEVICE_NAME_SIZE) {
                        bBtpDeviceName[i] = *ptr++;
                        i++;
                    }
                    if (i < DEVICE_NAME_SIZE - 1)
                        bBtpDeviceName[i] = '\0';
                    else
                        bBtpDeviceName[DEVICE_NAME_SIZE - 1] = '\0';
                }
                Semaphore_post(btpAckSemHandle);
                break;
            }
            break;
    }
}

//***********************************************************************************
//
// Bluetooth print hwi isr callback function.
//
//***********************************************************************************
void Btp_hwiIntCallback(void)
{
    if (g_rUart1RxData.length <= 2)
        return;

    if (g_rUart1RxData.buff[g_rUart1RxData.length - 2] == '\r'
            && g_rUart1RxData.buff[g_rUart1RxData.length - 1] == '\n') {

        g_rUart1RxData.buff[g_rUart1RxData.length] = '\0';
        Btp_respond_parse(g_rUart1RxData.buff);
    }
}

//***********************************************************************************
//
// Bluetooth print init.
//
//***********************************************************************************
void Btp_init(void)
{
    bBtpAckState = BTP_ACK_WAIT;
    bBtpCmdTpye = BTP_CMD_COMMON;
    memset((char *)bBtpDeviceName, 0, sizeof(bBtpDeviceName));

    blueControlPinHandle = PIN_open(&blueControlPinState, blueControlPinConfig);

    /* Construct Semaphore object to be use as a resource lock, inital count 1 */
	Semaphore_Params semParams;
	Semaphore_Params_init(&semParams);
	semParams.mode = Semaphore_Mode_BINARY;
	Semaphore_construct(&btpAckSemStruct, 1, &semParams);
	btpAckSemHandle = Semaphore_handle(&btpAckSemStruct);
}

//***********************************************************************************
//
// Bluetooth print init.
//
//***********************************************************************************
void Btp_poweron(void)
{
    PIN_setOutputValue(blueControlPinHandle, BT_POWER_PIN, 1);
    Task_sleep(1000 * CLOCK_UNIT_MS);

#ifdef  SUPPORT_NETWORK
    Nwk_poweroff();
    while(Nwk_is_Active())
    {
        Nwk_poweroff();
        WdtClear();
        Task_sleep(100 * CLOCK_UNIT_MS);
    }
#endif

     //Init UART.
    UartHwInit(UART_0, 9600, Btp_hwiIntCallback, UART_BLUE);
}

//***********************************************************************************
//
// Bluetooth print init.
//
//***********************************************************************************
void Btp_poweroff(void)
{
    UartClose(UART_0);
    UartPortDisable(UART_BLUE);

    PIN_setOutputValue(blueControlPinHandle, BT_POWER_PIN, 0);
}

//***********************************************************************************
//
// Bluetooth print get device name.
//
//***********************************************************************************
ErrorStatus Btp_get_device_name(uint8_t *pNameBuf)
{
    uint8_t i;
    ErrorStatus ret = ES_SUCCESS;

    for (i = 0; i < 10; i++) {
        bBtpCmdTpye = BTP_CMD_GET_DEVICE_NAME;
        Btp_send_cmd(BTPCMD_GET_DEVICE_NAME);
        Btp_wait_ack(500);
        if (bBtpAckState == BTP_ACK_OK)
            break;
    }
    if (i >= 10) {
        memset((char *)bBtpDeviceName, 0, DEVICE_NAME_SIZE);
        ret = ES_ERROR;
    }

    i = 0;
    while (1) {
        pNameBuf[i] = bBtpDeviceName[i];
        if (bBtpDeviceName[i] == '\0' || i >= DEVICE_NAME_SIZE - 1)
            break;
        i++;
    }

    return ret;
}

//***********************************************************************************
//
// Bluetooth print set device name.
//
//***********************************************************************************
ErrorStatus Btp_set_device_name(uint8_t *pNameBuf)
{
    uint8_t i;
    uint8_t buff[48], index, length;

    strcpy((char *)buff, BTPCMD_SET_DEVICE_NAME);
    index = sizeof(BTPCMD_SET_DEVICE_NAME) - 1;
    length = sprintf((char *)(buff + index), "%s\r\n", pNameBuf);
    index += length;
    buff[index] = '\0';

    for (i = 0; i < 10; i++) {
        bBtpCmdTpye = BTP_CMD_COMMON;
        Btp_send_cmd(buff);
        Btp_wait_ack(500);
        if (bBtpAckState == BTP_ACK_OK)
            break;
    }
    if (i >= 10) {
        return ES_ERROR;
    }

    return ES_SUCCESS;
}

//***********************************************************************************
//
// Bluetooth print set device name.
//
//***********************************************************************************
ErrorStatus Btp_scan_device_name(void)
{
    uint8_t i, index, length;
    uint8_t buff[64];

    ErrorStatus ret = ES_SUCCESS;


    // test if connect
    if (PIN_getInputValue(BT_CONNECT_PIN))
    {
        return ES_SUCCESS;
    }

    // change the mode, stop connect

    for (i = 0; i < 10; i++) {
        bBtpCmdTpye = BTP_CMD_COMMON;
        Btp_send_cmd(BTPCMD_CHANGE_MODE);
        Btp_wait_ack(500);
        if (bBtpAckState == BTP_ACK_OK)
            break;
    }
    // Btp_poweroff();
    // Task_sleep(1*CLOCK_UNIT_S);
    // Btp_poweron();   


    // test if connect
    if (PIN_getInputValue(BT_CONNECT_PIN))
    {
        return ES_SUCCESS;
    }

#ifdef SUPPORT_WATCHDOG
    if(watchdogHandle!=NULL){
    	Watchdog_clear(watchdogHandle);
    }
#endif

    Btp_get_device_name(buff);

    // test if connect
    if (PIN_getInputValue(BT_CONNECT_PIN))
    {
        return ES_SUCCESS;
    }

    if(i >= 10)
    {
        return ES_ERROR; 
    }
    Task_sleep(500*CLOCK_UNIT_MS);

    // scan the ble deceive
    for (i = 0; i < 4; i++) {
#ifdef SUPPORT_WATCHDOG
	if(watchdogHandle!=NULL){
    	Watchdog_clear(watchdogHandle);
    }
#endif
        bBtpCmdTpye = BTP_CMD_SCAN_DEVICE_NAME;
        if (PIN_getInputValue(BT_CONNECT_PIN))
        {
            return ES_SUCCESS;
        }
        Btp_send_cmd(BTPCMD_SCAN_DEVICE_NAME);
        Btp_wait_ack(5000);
        if (bBtpAckState == BTP_ACK_OK)
            break;

        if (PIN_getInputValue(BT_CONNECT_PIN))
        {
            return ES_SUCCESS;
        }

        // STOP scan the ble deceive
        bBtpCmdTpye = BTP_CMD_STOP_SCAN;
        Btp_send_cmd(BTPCMD_STOP_SCAN);
        Btp_wait_ack(500);
    }

    

#ifdef SUPPORT_WATCHDOG
    if(watchdogHandle!=NULL){
    Watchdog_clear(watchdogHandle);
    }
#endif

    if(i >= 4)
    {
        return ES_ERROR; 
    }
    Task_sleep(500*CLOCK_UNIT_MS);

    bBtpCmdTpye = BTP_CMD_STOP_SCAN;
    Btp_send_cmd(BTPCMD_STOP_SCAN);
    Btp_wait_ack(500);

#ifdef SUPPORT_WATCHDOG
    if(watchdogHandle!=NULL){
    Watchdog_clear(watchdogHandle);
    }
#endif

    Task_sleep(1*CLOCK_UNIT_S);


    // start connect the deceive by bleMAC
    strcpy((char *)buff, BTPCMD_SPP_CONNECT);
    index = sizeof(BTPCMD_SPP_CONNECT) - 1;
    memcpy((char *)(buff + index), bBtpDeviceMac, 12);
    index += 12;
    length = sprintf((char *)(buff + index), "\r\n");
    index += length;
    buff[index] = '\0';

    bBtpCmdTpye = BTP_CMD_COMMON;
    Btp_send_cmd(buff);
    for (i = 0; i < 10; i++) {
        Btp_wait_ack(500);
        if (bBtpAckState == BTP_ACK_OK)
            break;
    }
    if (i >= 10) {
        return ES_ERROR;
    }

    return ret;
}


//***********************************************************************************
//
// Bluetooth print module is connect.
//
//***********************************************************************************
uint8_t Btp_is_connect(void)
{
    uint8_t timeout = 40;   // 40s

    while (timeout--) {        
        if (PIN_getInputValue(BT_CONNECT_PIN)) {
            //Bluetooth connect success
            return 1;
        }
#ifdef SUPPORT_WATCHDOG
        if(watchdogHandle!=NULL){
        Watchdog_clear(watchdogHandle);
        }
#endif
        Task_sleep(1 * CLOCK_UNIT_S);
    }

    return 0;
}

#if defined(S_A)
#define BT_PRINT_START_END_TIME
#define BT_PRINT_AVG_TEMP
#define CALC_AVG_TEMP_USE_PRINT_DATA
#define SUPPORT_PRINT_STRATEGY
#endif


#ifdef SUPPORT_PRINT_STRATEGY
uint8_t chn,printCnt,printTime,alarmCurr,alarmPre = 3 ;
#endif

#ifdef BT_PRINT_AVG_TEMP
int32_t HighTemp, LowTemp, AvgTemp,CountTemp;
#endif



#ifdef SUPPORT_PRINT_STRATEGY
static uint8_t Judge_whether_over_temp(uint8_t chn,int32_t temp)
{
   uint8_t ret = 0;
   temp = (int32_t)round( temp / 10.0);

#if 0
     switch(g_rSysConfigInfo.sensorModule[chn]){

       case SEN_TYPE_SHT2X:
       case SEN_TYPE_NTC:
           if(( (g_rSysConfigInfo.alarmTemp[chn].high != ALARM_TEMP_HIGH )&& (temp >= (int32_t)round( g_rSysConfigInfo.alarmTemp[chn].high / 10.0)))
              ||((g_rSysConfigInfo.alarmTemp[chn].low != ALARM_TEMP_LOW )&& (temp  <= (int32_t)round(g_rSysConfigInfo.alarmTemp[chn].low / 10.0 ))))
             {
               ret = 1;
             }
           break;

       case SEN_TYPE_DEEPTEMP:
           if(( (g_rSysConfigInfo.alarmTemp[chn].high != DEEP_TEMP_H )&& (temp >= (int32_t)round( g_rSysConfigInfo.alarmTemp[chn].high / 10.0)))
              ||((g_rSysConfigInfo.alarmTemp[chn].low != DEEP_TEMP_L )&& (temp  <= (int32_t)round(g_rSysConfigInfo.alarmTemp[chn].low / 10.0 ))))
             {
               ret = 1;
             }
           break;

     }
#else
     //code is not enough  space,so use this Code segment
     if( ((g_rSysConfigInfo.alarmTemp[chn].high != ALARM_TEMP_HIGH) &&  (temp >= (int32_t)round( g_rSysConfigInfo.alarmTemp[chn].high / 10.0)))
       ||((g_rSysConfigInfo.alarmTemp[chn].low != ALARM_TEMP_LOW)&& (temp  <= (int32_t)round(g_rSysConfigInfo.alarmTemp[chn].low / 10.0 ))) ){

             ret = 1;
     }
#endif

   return ret;
}
#endif
//***********************************************************************************
//
// Bluetooth print module start print flash data.
//
//***********************************************************************************


#ifdef BT_PRINT_AVG_TEMP
static uint8_t TempToPrintBuff(int32_t value,uint8_t *buff,uint8_t sensor_type,char * str)
{
      uint8_t len = 0;

#if 0
      switch(sensor_type){

       case SEN_TYPE_SHT2X:
       case SEN_TYPE_NTC:
            if((value == ALARM_TEMP_HIGH)||(value == ALARM_TEMP_LOW)){
              len = sprintf((char*)buff, "%s ~ \n", str) ;
              return len;
            }
            break;
       case SEN_TYPE_DEEPTEMP:
           if((value == DEEP_TEMP_H)||(value == DEEP_TEMP_L)){
             len = sprintf((char*)buff, "%s ~ \n", str) ;
             return len;
            }
            break;
      }
#else
      //code is not enough space,so use this Code segment
      if( (value == ALARM_TEMP_HIGH) || (value == ALARM_TEMP_LOW)){

          len = sprintf((char*)buff, "%s ~ \n", str) ;
          return len;
      }
#endif
      value = (int32_t)round( value / 10.0);
       if(value < 0){

           len = sprintf((char*)buff, "%s -%2d.%d℃\n", str,(uint16_t)(-value/10), (uint16_t)(-value %10));
       }
       else{
           len = sprintf((char*)buff, "%s %2d.%d℃\n",str,(uint16_t)(value/10), (uint16_t)(value %10));

       }

    return len;
}
#endif


//===============================================================


#ifndef S_G
//========================================================
void Btp_print_record(void)
{
#if defined(FLASH_EXTERNAL) && defined(SUPPORT_SENSOR)
    uint8_t buff[FLASH_SENSOR_DATA_SIZE], length, i, index;

    Calendar calendar;
    int16_t temperature;
    uint16_t humidty;
    int32_t deepTemp;
#ifdef BT_PRINT_AVG_TEMP
    int32_t PrintTemp;
#endif
#ifdef CALC_AVG_TEMP_USE_PRINT_DATA
	bool TempisValid = 0;
#endif


    if(0==PrintProStep){

#ifdef SUPPORT_PRINT_STRATEGY
     alarmPre = 3;
    if(g_rSysConfigInfo.collectPeriod == 60)
       printTime = FIVE_MINUTE_PRINT_PERIOD;
    else
       printTime =  PRINT_PERIOD_IS_NULL;
#endif

#ifdef  SUPPORT_DEVICED_STATE_UPLOAD
    Btp_print_num = 0;    
    Flash_store_devices_state(TYPE_BT_PRINT_START);
#endif

    Btp_send_cmd("\e!\x30温湿度监控记录\r\n");

    sensorNum = 0;
	PrintSensorType = SEN_TYPE_NONE;
    for (i = 0; i < MODULE_SENSOR_MAX; i++) {
        if (g_rSysConfigInfo.sensorModule[i] != SEN_TYPE_NONE) {
            sensorNum++;
        }

        if(g_rSysConfigInfo.sensorModule[i]  == SEN_TYPE_SHT2X){
	            PrintSensorType = SEN_TYPE_SHT2X;

            #ifdef SUPPORT_PRINT_STRATEGY
            chn = i;
            #endif
        }

        if (g_rSysConfigInfo.sensorModule[i] == SEN_TYPE_NTC) {
	            PrintSensorType = SEN_TYPE_NTC;

            #ifdef SUPPORT_PRINT_STRATEGY
            chn = i;
            #endif
        }

        if (g_rSysConfigInfo.sensorModule[i] == SEN_TYPE_DEEPTEMP) {
	            PrintSensorType = SEN_TYPE_DEEPTEMP;
            #ifdef SUPPORT_PRINT_STRATEGY
            chn = i;
            #endif
        }
    }
    if (sensorNum == 0) {
        Btp_send_cmd("\e!\x30传感器配置错误\r\n\n\n");
		
#ifdef  SUPPORT_DEVICED_STATE_UPLOAD        
		Flash_store_devices_state(TYPE_BT_PRINT_END);
#endif
        return;
    }

    recordAddr_blue_print = Flash_get_record_addr();

    if(Flash_get_record_items()>0){
        recordAddr_blue_print.end = (recordAddr_blue_print.start + Flash_get_record_items()*FLASH_SENSOR_DATA_SIZE) % (FLASH_SENSOR_DATA_SIZE * FLASH_SENSOR_DATA_NUMBER);
    }
    else{
        recordAddr_blue_print.end = recordAddr_blue_print.start;
    }

    if (recordAddr_blue_print.start == recordAddr_blue_print.end) {
        Btp_send_cmd("\e!\x30无打印数据\r\n\n\n");
#ifdef  SUPPORT_DEVICED_STATE_UPLOAD        
        Flash_store_devices_state(TYPE_BT_PRINT_END);
#endif
        return;
    }

    //set print font.
    buff[0] = 0x1b;
    buff[1] = 0x21;
    buff[2] = 0x00;
    Btp_send_data(buff, 3);

    Btp_send_cmd("设备: S6\n");

    length = sprintf((char *)buff, "设备ID: %02x%02x%02x%02x\n",g_rSysConfigInfo.DeviceId[0], g_rSysConfigInfo.DeviceId[1],
                                                            g_rSysConfigInfo.DeviceId[2], g_rSysConfigInfo.DeviceId[3]);

    Btp_send_data(buff, length);
    
    calendar = Rtc_get_calendar();
    length = sprintf((char *)buff, "打印时间: %4d-%02d-%02d %02d:%02d:%02d\n",
                        calendar.Year, calendar.Month, calendar.DayOfMonth,
                        calendar.Hours, calendar.Minutes, calendar.Seconds);
    Btp_send_data(buff, length);
	
#ifdef BT_PRINT_START_END_TIME

	Flash_get_record(recordAddr_blue_print.start, buff, FLASH_SENSOR_DATA_SIZE);
	index = 8;
	calendar.Year = buff[index++] + CALENDAR_BASE_YEAR;
	calendar.Month = buff[index++];
	calendar.DayOfMonth = buff[index++];
	calendar.Hours = buff[index++];
	calendar.Minutes = buff[index++];
	calendar.Seconds = buff[index++];
    length = sprintf((char *)buff, "起始时间: %4x-%02x-%02x %02x:%02x:%02x\n",
                        calendar.Year, calendar.Month, calendar.DayOfMonth,
                        calendar.Hours, calendar.Minutes, calendar.Seconds);
    Btp_send_data(buff, length);

	Flash_get_record(((FLASH_SENSOR_DATA_SIZE * FLASH_SENSOR_DATA_NUMBER) + recordAddr_blue_print.end - FLASH_SENSOR_DATA_SIZE) % (FLASH_SENSOR_DATA_SIZE * FLASH_SENSOR_DATA_NUMBER),
		buff, FLASH_SENSOR_DATA_SIZE);	
	index = 8;
	calendar.Year = buff[index++] + CALENDAR_BASE_YEAR;
	calendar.Month = buff[index++];
	calendar.DayOfMonth = buff[index++];
	calendar.Hours = buff[index++];
	calendar.Minutes = buff[index++];
	calendar.Seconds = buff[index++];
	length = sprintf((char *)buff, "结束时间: %4x-%02x-%02x %02x:%02x:%02x\r\n",
						calendar.Year, calendar.Month, calendar.DayOfMonth,
						calendar.Hours, calendar.Minutes, calendar.Seconds);
	Btp_send_data(buff, length);

	HighTemp = ALARM_TEMP_LOW;
	LowTemp  = ALARM_TEMP_HIGH;	
	AvgTemp  = 0;
	CountTemp = 0;
#endif

	PrintProStep = 1;
}

  //  while (1)
	if(1 ==PrintProStep) {

#ifdef SUPPORT_PRINT_STRATEGY
START_RUN_LAB:
#endif
        #ifdef SUPPORT_WATCHDOG
        if(watchdogHandle!=NULL){
        Watchdog_clear(watchdogHandle);
        }
        #endif
        Task_sleep(1 * CLOCK_UNIT_MS);
        Flash_get_record(recordAddr_blue_print.start, buff, FLASH_SENSOR_DATA_SIZE);
        index = 8;
        calendar.Year = buff[index++] + CALENDAR_BASE_YEAR;
        calendar.Month = buff[index++];
        calendar.DayOfMonth = buff[index++];
        calendar.Hours = buff[index++];
        calendar.Minutes = buff[index++];
        calendar.Seconds = buff[index++];

        index += 2;
        for (i = 0; i < sensorNum; i++) {
            if (SEN_TYPE_SHT2X == PrintSensorType && buff[index + 1] == SEN_TYPE_SHT2X) {
                temperature = (buff[index + 2] << 8) | buff[index + 3];
                humidty = (buff[index + 4] << 8) | buff[index + 5];
                break;
            } else if (SEN_TYPE_NTC == PrintSensorType && buff[index + 1] == SEN_TYPE_NTC) {
                temperature = (buff[index + 2] << 8) | buff[index + 3];
                break;
            } else if (SEN_TYPE_DEEPTEMP == PrintSensorType && buff[index + 1] == SEN_TYPE_DEEPTEMP) {
                deepTemp = (int32_t)(((uint32_t)buff[index + 2] << 24) | ((uint32_t)buff[index + 3] << 16) | ((uint32_t)buff[index + 4] << 8));
                deepTemp  = (int32_t)(deepTemp >> 12);
                break;
            } else {
                switch(buff[index + 1]) {
                    case SEN_TYPE_SHT2X :
                        index += 6;
                        break;
                    case SEN_TYPE_NTC :
                        index += 4;
                        break;
                    case SEN_TYPE_OPT3001 :
                        index += 6;
                        break;
                    case SEN_TYPE_DEEPTEMP :
                        index += 5;
                        break;
                    /*case SEN_TYPE_HCHO :
                        index += 4;
                        break;
                    case SEN_TYPE_PM25 :
                        index += 4;
                        break;
                    case SEN_TYPE_CO2 :
                        index += 4;
                        break;*/
                    case SEN_TYPE_GSENSOR :
                        index += 4;
                        break;
                    default :
                        break;
                }
            }
        }

        length = sprintf((char *)buff, "%4x-%02x-%02x %02x:%02x ",
                            calendar.Year, calendar.Month, calendar.DayOfMonth,
                            calendar.Hours, calendar.Minutes);

		#ifdef CALC_AVG_TEMP_USE_PRINT_DATA
		TempisValid = 0;
		#endif
		
        if (SEN_TYPE_NTC == PrintSensorType) {
            if (temperature == TEMPERATURE_OVERLOAD) {
                length += sprintf((char *)buff + length, "  -- ℃\n");
            } else {

				#ifdef BT_PRINT_AVG_TEMP            

#ifdef CALC_AVG_TEMP_USE_PRINT_DATA
				TempisValid = 1;
#else
                CountTemp++;AvgTemp += temperature;
#endif
				PrintTemp = temperature;
				#endif

                #ifdef  SUPPORT_PRINT_STRATEGY
                alarmCurr = Judge_whether_over_temp(chn,temperature);
                #endif
                temperature = (int16_t)round((float)temperature / 10.0);
                if (temperature < 0) {
                    temperature = -temperature;
                    buff[length++] = '-';
                } else {
                    buff[length++] = ' ';
                }
                length += sprintf((char *)buff + length, "%2d.%d℃\n", (temperature / 10) % 100, temperature % 10);
            }
        } 
		else if (SEN_TYPE_SHT2X == PrintSensorType){
            humidty /= 100;
            if (temperature == TEMPERATURE_OVERLOAD || humidty > 99) {
                length += sprintf((char *)buff + length, "  -- ℃  --%%\n");
            } else {
            	
				#ifdef BT_PRINT_AVG_TEMP
				
#ifdef CALC_AVG_TEMP_USE_PRINT_DATA
				TempisValid = 1;
#else
                CountTemp++;AvgTemp += temperature;
#endif
				PrintTemp = temperature;
				#endif

                #ifdef  SUPPORT_PRINT_STRATEGY
                alarmCurr = Judge_whether_over_temp(chn,temperature);
                #endif
                temperature = (int16_t)round((float)temperature / 10.0);
                if (temperature < 0) {
                    temperature = -temperature;
                    buff[length++] = '-';
                } else {
                    buff[length++] = ' ';
                }
                length += sprintf((char *)buff + length, "%2d.%d℃ %3d%%\n", (temperature / 10) % 100, temperature % 10, humidty);
            }
        } 
		else if (SEN_TYPE_DEEPTEMP == PrintSensorType) {
            if (deepTemp == (DEEP_TEMP_OVERLOAD >> 4)) {
                length += sprintf((char *)buff + length, "  -- ℃\n");
            } else {
			
				#ifdef BT_PRINT_AVG_TEMP
				
#ifdef CALC_AVG_TEMP_USE_PRINT_DATA
				TempisValid = 1;
#else
                CountTemp++;AvgTemp += deepTemp;
#endif

				PrintTemp = deepTemp;
				#endif
				
                #ifdef  SUPPORT_PRINT_STRATEGY
                alarmCurr = Judge_whether_over_temp(chn,deepTemp);
                #endif
                temperature = (int16_t)round((float)deepTemp / 10.0);
                if (temperature < 0) {
                    temperature = -temperature;
                    buff[length++] = '-';
                } else {
                    buff[length++] = ' ';
                }
                length += sprintf((char *)buff + length, "%0d.%d℃\n", (temperature / 10) % 1000, temperature % 10);
            }
        }

#ifdef  SUPPORT_DEVICED_STATE_UPLOAD
        Btp_print_num++;
#endif

#ifdef SUPPORT_PRINT_STRATEGY
       if( printTime == FIVE_MINUTE_PRINT_PERIOD){

		   if(alarmCurr != alarmPre ){//state change, must print
				printCnt = 0;
		   }
		   else{
		   	   if(alarmCurr == 1){//temp over, must print
				   printCnt = 0;
			   }
			   else{
           printCnt ++;
				   if(printCnt % 5 == 0) {//count reach to 5, print
						printCnt = 0;
				   }
				   else{
                  recordAddr_blue_print.start = (recordAddr_blue_print.start + FLASH_SENSOR_DATA_SIZE) % (FLASH_SENSOR_DATA_SIZE * FLASH_SENSOR_DATA_NUMBER);

						 //it is the last data ,must print
						 if (recordAddr_blue_print.start == recordAddr_blue_print.end){
							 PrintProStep = 0;
						 }
						 else{						 
                      goto START_RUN_LAB;
                  }
			   	   }
			   }
           }

           alarmPre = alarmCurr;

       }
#endif

#ifdef BT_PRINT_AVG_TEMP
        if(PrintTemp > HighTemp)HighTemp = PrintTemp;
        if(PrintTemp < LowTemp)LowTemp = PrintTemp;

#ifdef CALC_AVG_TEMP_USE_PRINT_DATA
		if(TempisValid){
        AvgTemp += PrintTemp;
        CountTemp++;
		}
#endif		

#endif


        Btp_send_data(buff, length);
		

        recordAddr_blue_print.start = (recordAddr_blue_print.start + FLASH_SENSOR_DATA_SIZE) % (FLASH_SENSOR_DATA_SIZE * FLASH_SENSOR_DATA_NUMBER);

        if (recordAddr_blue_print.start == recordAddr_blue_print.end)
            PrintProStep = 0;
           // break;
    }

if( 1 ==PrintProStep ){
    Sys_event_post(SYS_EVT_PRINT_CONTINU);//
}
else{
#ifdef BT_PRINT_AVG_TEMP

    length = TempToPrintBuff(g_rSysConfigInfo.alarmTemp[chn].high ,buff,g_rSysConfigInfo.sensorModule[chn],"\n高温报警阀值:");
    Btp_send_data(buff, length);

    length = TempToPrintBuff(g_rSysConfigInfo.alarmTemp[chn].low ,buff,g_rSysConfigInfo.sensorModule[chn],"低温报警阀值:");
    Btp_send_data(buff, length);

    length = TempToPrintBuff(HighTemp,buff,g_rSysConfigInfo.sensorModule[chn],"最高温度:");
	Btp_send_data(buff, length);

	length = TempToPrintBuff(LowTemp,buff,g_rSysConfigInfo.sensorModule[chn],"最低温度:");
	Btp_send_data(buff, length);

	AvgTemp = AvgTemp/CountTemp;
	length = TempToPrintBuff(AvgTemp,buff,g_rSysConfigInfo.sensorModule[chn],"平均温度:");
	Btp_send_data(buff, length);
#endif
	Btp_send_cmd("\r\n接收单位:\r\n");
    Btp_send_cmd("确认签字:\r\n\n\n\n");
    
#ifdef  SUPPORT_DEVICED_STATE_UPLOAD
    Flash_store_devices_state(TYPE_BT_PRINT_END);
#endif
    Task_sleep(3 * CLOCK_UNIT_S);
    Btp_poweroff();
    Menu_exit();
    Sys_lcd_start_timing();
}
#else
    Btp_send_cmd("\e!\x30无存储功能\r\n\n\n");
    Task_sleep(3 * CLOCK_UNIT_S);
    Btp_poweroff();
    Menu_exit();
    Sys_lcd_start_timing();
#endif
}
#endif // #define S_G

#ifdef  SUPPORT_DEVICED_STATE_UPLOAD
uint32_t Btp_GetPrintNum()
{
    return Btp_print_num;
}
#endif

// print list
#include "print_list.h"

#ifdef SUPPORT_BLE_PRINT_LIST_MODE

// ret : 1 a>b  0 a==b    -1 a<b
int Record_time_compare(const void* rt_a, const void* rt_b, const int size)
{
    uint8_t *p1 = (uint8_t*)rt_a;
    uint8_t *p2 = (uint8_t*)rt_b;
    int i;
    for(i=0; i < size; i++){
        if(*(p1+i) == *(p2+i))
            continue;
        if(*(p1+i) > *(p2+i))
               return 1;

        if(*(p1+i) < *(p2+i))
               return -1;
    }
    return 0;
}
int Record_time_compare_YMD_HMS(Record_time_t* rt_a, Record_time_t* rt_b)
{
    return Record_time_compare(rt_a, rt_b, sizeof(Record_time_t));
}
int Record_time_compare_YMD_HM(Record_time_t* rt_a, Record_time_t* rt_b)
{
    return Record_time_compare(rt_a, rt_b, sizeof(Record_time_t)-1);
}
int Record_time_compare_YMD(Record_time_t* rt_a, Record_time_t* rt_b)
{
    return Record_time_compare(rt_a, rt_b, 3);
}

int Btp_printList_print_HEAD(void)
{
#if defined(FLASH_EXTERNAL) // && defined(SUPPORT_SENSOR)
    uint8_t buff[FLASH_SENSOR_DATA_SIZE], length;
    FlashPrintRecordAddr_t recordAddr;
    Calendar calendar;

    Btp_send_cmd("\e!\x30温湿度监控记录\r\n");

    if(1 == Record_get_status()){// 还在记录状态  读取当前记录地址作为end点
        recordAddr = Flash_get_current_record_addr();//Flash_get_record_addr();
    }else{
        recordAddr = Flash_get_record_addr();
    }
    if (recordAddr.start == recordAddr.end) {
        Btp_send_cmd("\e!\x30无打印数据\r\n\n\n");
        return -1;
    }

    //set print font.
    buff[0] = 0x1b;
    buff[1] = 0x21;
    buff[2] = 0x00;
    Btp_send_data(buff, 3);

    Btp_send_cmd("设备: S6\n");

    length = sprintf((char *)buff, "网关设备ID: %02x%02x%02x%02x\n",g_rSysConfigInfo.DeviceId[0], g_rSysConfigInfo.DeviceId[1],
                                                            g_rSysConfigInfo.DeviceId[2], g_rSysConfigInfo.DeviceId[3]);
    Btp_send_data(buff, length);

    calendar = Rtc_get_calendar();
    length = sprintf((char *)buff, "打印时间: %4d-%02d-%02d %02d:%02d:%02d\n",
                        calendar.Year, calendar.Month, calendar.DayOfMonth,
                        calendar.Hours, calendar.Minutes, calendar.Seconds);
    Btp_send_data(buff, length);
#endif
    return 0; // OK
}

void Btp_printList_print_END(void)
{
    Btp_send_cmd("\r\n接收单位:\r\n");
    Btp_send_cmd("确认签字:\r\n\n\n\n");
}

int8_t  Btp_printList_print_YMD_DID_head(uint8_t* print_buff, const void*Record_Data, SENSOR_TYPE *type_list, const uint32_t *DID, const uint8_t List_Num)
{
    //uint8_t buff[FLASH_SENSOR_DATA_SIZE];
    uint8_t *buff = print_buff;
    uint8_t length;
    Record_Header_t *Record_Head;
    Record_time_t *Record_time;
    int i;
    if((NULL ==Record_Data) || (NULL == DID))
        return -1;

    Record_Head = (Record_Header_t *)Record_Data;
    Record_time = &(Record_Head->Record_time);

    //length = sprintf((char *)buff, "20%02x-%02x-%02x \n",
    length = sprintf((char *)buff, "20%02x-%02x-%02x\n",
                     Record_time->YMD.Year,Record_time->YMD.Month,Record_time->YMD.Date);
    Btp_send_data((uint8_t*)buff, length);

    Btp_send_cmd("LIST   ");
    for(i= 0; i < List_Num; i++)
    {
        /*
        length = sprintf((char *)buff, "%02x%02x%02x%02x ", HIBYTE(HIWORD(DID[i])),
                LOBYTE(HIWORD(DID[i])), HIBYTE(LOWORD(DID[i])),LOBYTE(LOWORD(DID[i])));
                */

        length = sprintf((char *)buff, "DID%d   ", i);
        Btp_send_data((uint8_t*)buff, length);
    }
    Btp_send_cmd("\n");

    if(type_list == NULL)
        return -1;

    Btp_send_cmd("       ");
    for(i= 0; i < List_Num; i++)
    {

        switch(type_list[i]){
        case SEN_TYPE_SHT2X :
            length = sprintf((char *)buff,"%s", "  ℃/%");
            break;
        case SEN_TYPE_NTC :
            length = sprintf((char *)buff,"%s", "  ℃");
            break;
        case SEN_TYPE_OPT3001 : // ?
            length = sprintf((char *)buff,"%s", "  ℃");
            break;
        case SEN_TYPE_DEEPTEMP :    // MAX31855 ??

                break;
        case SEN_TYPE_HCHO :

            break;
        case SEN_TYPE_PM25 :

            break;
        case SEN_TYPE_CO2 :

            break;
        case SEN_TYPE_GSENSOR :

            break;
        default :
            length = sprintf((char *)buff,"%s", " ----");
            break;
        }

        Btp_send_data((uint8_t*)buff, length);
    }
    Btp_send_cmd("\n");

    return 0; // OK
}

int8_t Btp_printList_print_DID_type_list_head(uint8_t* print_buff, SENSOR_TYPE *type_list, const uint32_t *DID, const uint8_t List_Num, int pos)
{
    uint8_t *buff = print_buff + pos;
    uint8_t length;
    //Record_Header_t *Record_Head;
    //Record_time_t *Record_time;
    int i;
    if((NULL ==type_list) || (NULL == DID))
        return -1;

    Btp_send_cmd("LIST   ");
    for(i= 0; i < List_Num; i++)
    {
        /*
        length = sprintf((char *)buff, "%02x%02x%02x%02x ", HIBYTE(HIWORD(DID[i])),
                LOBYTE(HIWORD(DID[i])), HIBYTE(LOWORD(DID[i])),LOBYTE(LOWORD(DID[i])));
                */

        length = sprintf((char *)buff, "DID%d   ", i);
        Btp_send_data((uint8_t*)buff, length);
    }
    Btp_send_cmd("\n");

    if(type_list == NULL)
        return -1;

    Btp_send_cmd("       ");
    for(i= 0; i < List_Num; i++)
    {

        switch(type_list[i]){
        case SEN_TYPE_SHT2X :
            length = sprintf((char *)buff,"%s", "  ℃/%");
            break;
        case SEN_TYPE_NTC :
            length = sprintf((char *)buff,"%s", "  ℃");
            break;
        case SEN_TYPE_OPT3001 : // ?
            length = sprintf((char *)buff,"%s", "  ℃");
            break;
        case SEN_TYPE_DEEPTEMP :    // MAX31855 ??

                break;
        case SEN_TYPE_HCHO :

            break;
        case SEN_TYPE_PM25 :

            break;
        case SEN_TYPE_CO2 :

            break;
        case SEN_TYPE_GSENSOR :

            break;
        default :
            length = sprintf((char *)buff,"%s", " ----");
            break;
        }
        Btp_send_data((uint8_t*)buff, length);
    }

    Btp_send_cmd("\n");
    return 0; // OK
}
int Btp_printList_print_YMD_pos(uint8_t* print_buff, Record_time_t *Record_time, int pos)
{
    uint8_t *buff = print_buff + pos;
    int length;
    if((NULL ==Record_time) || (NULL == print_buff))
        return 0;

    length = sprintf((char *)buff, "20%02x-%02x-%02x ",
                     Record_time->YMD.Year,Record_time->YMD.Month,Record_time->YMD.Date);
    return length; // OK
}

int Btp_printList_print_sign(uint8_t* print_buff, uint8_t* str, int pos)
{
    uint8_t *buff = print_buff + pos;
    int length;
    if((NULL ==str) || (NULL == print_buff))
        return 0;

    length = sprintf((char *)buff, "%s",str);
    return length; // OK
}

/* 找到第一个符合条件的温度类型  */
SENSOR_TYPE Record_data_Get_first_temp_type( const void * Record_data)
{
    Record_Header_t *Record_Head;
    Record_SData_t *Record_SData;
    uint8_t length;
    uint8_t *pdata;

    if( (NULL == Record_data))
        return SEN_TYPE_NONE;

    Record_Head = (Record_Header_t *)Record_data;
    length = Record_Head->Length;

    if(length ==0 || length == 0xff || length > FLASH_SENSOR_DATA_SIZE-1)
        return SEN_TYPE_NONE;

    // first
    pdata = Record_Head->DataType;
    Record_SData = (Record_SData_t*)pdata;
    while( (pdata - ((uint8_t*) Record_data)) < length){

        switch(Record_SData->sensor_type){
            case SEN_TYPE_SHT2X :
                    return SEN_TYPE_SHT2X;
                //pdata += 6;
                //break;
            case SEN_TYPE_NTC :
                    return SEN_TYPE_NTC;
                //pdata += 4;
                //break;
            case SEN_TYPE_OPT3001 : // ?
                pdata += 6;
                break;
            case SEN_TYPE_DEEPTEMP :    // MAX31855 ??
                    return SEN_TYPE_DEEPTEMP;
                //pdata += 5;
                //break;
            case SEN_TYPE_HCHO :
                pdata += 4;
                break;
            case SEN_TYPE_PM25 :
                pdata += 4;
                break;
            case SEN_TYPE_CO2 :
                pdata += 4;
                break;
            case SEN_TYPE_GSENSOR :
                pdata += 4;
                break;
            default :
                return SEN_TYPE_NONE;
                //break;
        }
        Record_SData = (Record_SData_t*)pdata;
    }
    return SEN_TYPE_NONE;
}

SENSOR_TYPE Record_data_Get_type_value(SENSOR_TYPE type, Record_SData_t* Record_value, const void * Record_data)
{
    Record_Header_t *Record_Head;
    //Record_time_t Record_time;
    Record_SData_t *Record_SData;
    uint8_t length;
    uint8_t *pdata;

    if((NULL == Record_value) || (NULL == Record_data))
        return SEN_TYPE_NONE;

    Record_Head = (Record_Header_t *)Record_data;
    //Record_time = Record_Head->Record_time;
    length = Record_Head->Length;

    if(length ==0 || length == 0xff) // || length != FLASH_SENSOR_DATA_SIZE-1)
        return SEN_TYPE_NONE;

    // first
    pdata = Record_Head->DataType;
    Record_SData = (Record_SData_t*)pdata;
    while( (pdata - ((uint8_t*) Record_data)) < length){

        switch(Record_SData->sensor_type){
            case SEN_TYPE_SHT2X :
                if(Record_SData->sensor_type == type){
                    *Record_value = *Record_SData;
                    return SEN_TYPE_SHT2X;
                }
                pdata += 6;
                break;
            case SEN_TYPE_NTC :
                if(Record_SData->sensor_type == type)
                {
                    *Record_value = *Record_SData;
                    return SEN_TYPE_NTC;
                }
                pdata += 4;
                break;
            case SEN_TYPE_OPT3001 : // ?
                pdata += 6;
                break;
            case SEN_TYPE_DEEPTEMP :    // MAX31855 ??
                if(Record_SData->sensor_type == type)
                {
                    *Record_value = *Record_SData;
                    return SEN_TYPE_DEEPTEMP;
                }
                pdata += 5;
                break;
            case SEN_TYPE_HCHO :
                pdata += 4;
                break;
            case SEN_TYPE_PM25 :
                pdata += 4;
                break;
            case SEN_TYPE_CO2 :
                pdata += 4;
                break;
            case SEN_TYPE_GSENSOR :
                pdata += 4;
                break;
            default :
                return SEN_TYPE_NONE;
                //break;
        }

        Record_SData = (Record_SData_t*)pdata;
    }
    return SEN_TYPE_NONE;
}

int8_t Btp_printList_print_HMS(uint8_t* print_buff, const uint8_t *Record_buff)
{
    uint8_t *buff = print_buff;
    uint8_t length = 0;
    Record_Header_t *Record_Head;
    Record_time_t *Record_time;

    if(NULL == Record_buff)
        return -1;

    Record_Head = (Record_Header_t *)Record_buff;
    Record_time = &(Record_Head->Record_time);

    //length = sprintf((char *)buff, "%02x:%02x:%02x",Record_time->HMS.Hour,Record_time->HMS.Minute,Record_time->HMS.Second);
    length = sprintf((char *)buff, "%02x:%02x ", Record_time->HMS.Hour,Record_time->HMS.Minute); // no second

    Btp_send_data(buff, length);

    return 0;
}

uint8_t BCD2DEC(uint8_t BCD);
uint8_t DEC2BCD(uint8_t DEC);

uint8_t BCD2DEC(uint8_t BCD)
{
    uint8_t temp;
    temp = (((BCD >> 4)&0xFF)*10) + (BCD&0xf);
    return temp;
}
uint8_t DEC2BCD(uint8_t DEC)
{
    uint8_t temp;
    temp = ((DEC/10)<<4) | (DEC%10);
    return temp;
}

// ret 0: update HMS OK
// ret 1: update YMD
int8_t Record_time_HMS_add_one_min( Record_time_t *Record_time)
{
//    if(NULL == Record_time)
//        return -1;

    uint8_t Minutes, Hours;
    uint8_t DayOfMonth , Month;

    uint16_t Year;

    uint8_t ret =0;

    Minutes = BCD2DEC(Record_time->HMS.Minute);
    Hours = BCD2DEC(Record_time->HMS.Hour);

    DayOfMonth = BCD2DEC(Record_time->YMD.Date);
    Month = BCD2DEC(Record_time->YMD.Month);
    Year = BCD2DEC(Record_time->YMD.Year) + 2000; //  + CALENDAR_BASE_YEAR

    // 时间以及闰年月处理
    Minutes += 1;
    if(Minutes >= 60){
        Hours +=  Minutes / 60;
        Minutes %= 60;
        if(Hours >= 24){

            ret = 1;
            Hours %= 24;
            DayOfMonth++;
            if(DayOfMonth > MonthMaxDay(Year, Month)){
                DayOfMonth = 1;
                Month++;
                if(Month >12){
                    Month = 1;
                    Year++;
                }
            }
        }
    }

    Record_time->HMS.Minute = DEC2BCD(Minutes);
    Record_time->HMS.Hour = DEC2BCD(Hours);

    Record_time->YMD.Date = DEC2BCD(DayOfMonth);
    Record_time->YMD.Month = DEC2BCD(Month);
    Record_time->YMD.Year = DEC2BCD(Year - 2000); // - CALENDAR_BASE_YEAR

    return ret;
}

// ret -1: error
// ret 0: do nothing
// ret 1: update
int8_t Record_time_Update( Record_time_t *Record_time, Record_time_t *Record_time_CMP)
{
    if(NULL == Record_time || NULL == Record_time_CMP)
        return -1;

    if(Record_time_compare_YMD(Record_time, Record_time_CMP) < 0){
        *Record_time = *Record_time_CMP;
        Record_time->HMS.Minute=0;
        Record_time->HMS.Minute = 0;
        Record_time->HMS.Hour = 0;
        return 1;
    }

    return 0;
}

#if 1
char Btp_printList_print_HMS_pos(uint8_t* print_buff_line, Record_time_t *Record_time, int pos)
{
    uint8_t *buff = print_buff_line + pos;
    uint8_t length = 0;

    if(NULL == print_buff_line || NULL == Record_time || pos < 0)
        return 0;

    length = sprintf((char *)buff, "%02x:%02x ", Record_time->HMS.Hour,Record_time->HMS.Minute); // no second

    return length;
}
#else

int Btp_printList_print_HMS_pos(uint8_t* print_buff_line, const uint8_t *Record_buff, int line_pos)
{
    uint8_t length = 0;
    Record_Header_t *Record_Head;
    Record_time_t *Record_time;

    uint8_t *buff = print_buff_line + line_pos;

    if(NULL == Record_buff || NULL == print_buff_line)
        return 0;

    Record_Head = (Record_Header_t *)Record_buff;
    Record_time = &(Record_Head->Record_time);

    length = sprintf((char *)buff, "%02x:%02x", Record_time->HMS.Hour,Record_time->HMS.Minute); // no second

    return length;
}
#endif

int Btp_printList_print_DATA_pos(uint8_t* print_buff_line, SENSOR_TYPE type,  const uint8_t *Record_buff, int line_pos)
{

    uint8_t length = 0;
    SENSOR_TYPE type_ret;
    Record_SData_t Record_value;

    int16_t temperature;
    uint16_t humidty;
    int32_t deepTemp;

    uint8_t *buff = print_buff_line + line_pos;

    if(NULL == Record_buff || NULL == print_buff_line)
        return 0;

    if(type == SEN_TYPE_NONE)
        return 0;

    //type = SEN_TYPE_SHT2X;
    type_ret = Record_data_Get_type_value( type, &Record_value, Record_buff);
    if(type_ret == type){
        length = 0;
        temperature = Record_value.value[0]<<8 |Record_value.value[1]; //  (buff[index + 2] << 8) | buff[index + 3];
        humidty = Record_value.value[2]<<8 |Record_value.value[3]; //(buff[index + 4] << 8) | buff[index + 5];

        humidty /= 100;
        if (temperature == TEMPERATURE_OVERLOAD || humidty > 99) {
            length = sprintf((char *)buff, "--℃ --%% ");
        } else {
            temperature = (int16_t)round((float)temperature / 10.0);
            if (temperature < 0) {
                temperature = -temperature;
                buff[length++] = '-';
            }
            //else {
            //    buff[length++] = ' ';
            //}

            length += sprintf((char *)buff+length, "%2d.%d℃/%2d%% ", (temperature / 10) % 100, temperature % 10, humidty);
        }
        return length;
    }

    //type = SEN_TYPE_NTC;
    type_ret = Record_data_Get_type_value( type, & Record_value, Record_buff);
    if(type_ret == type){
        length = 0;

        temperature = Record_value.value[0]<<8 |Record_value.value[1];

        if (temperature == TEMPERATURE_OVERLOAD) {
            length = sprintf((char *)buff, "--℃ ");
        } else {
            temperature = (int16_t)round((float)temperature / 10.0);

            if (temperature < 0) {
                temperature = -temperature;
                buff[length++] = '-';
            } else {
                buff[length++] = ' ';
            }

            length += sprintf((char *)buff+length, "%2d.%d℃ ", (temperature / 10) % 100, temperature % 10);
        }

        return length;
    }

    //type = SEN_TYPE_DEEPTEMP;
    type_ret = Record_data_Get_type_value( type, & Record_value, Record_buff);
    if(type_ret == type){
        length = 0;
        deepTemp = (int32_t)(((uint32_t)Record_value.value[0] << 24) | ((uint32_t)Record_value.value[1] << 16) | ((uint32_t)Record_value.value[2] << 8));
        deepTemp  = (int32_t)(deepTemp >> 12);

        if (deepTemp == (DEEP_TEMP_OVERLOAD >> 4)) {
            length = sprintf((char *)buff, "--℃ ");
        } else {
            temperature = (int16_t)round((float)deepTemp / 10.0);
            if (temperature < 0) {
                temperature = -temperature;
                buff[length++] = '-';
            } else {
                buff[length++] = ' ';
            }
            length += sprintf((char *)buff + length, "%0d.%d℃ ", (temperature / 10) % 1000, temperature % 10);
        }
        return length;
    }
    return 0;
}

int Record_get_Data_with_DID_Date(uint32_t *addr_start, const uint32_t addr_end, char addrnext_update, uint8_t *pData_buff, const uint16_t len_buff, const uint32_t DID, Record_time_t *Record_Date)
{
    Record_Header_t *Record_Head;
    Record_time_t *Record_time;
    Record_time_t  compareTime;
    Calendar calendar; 
    uint32_t addr_temp = *addr_start;
    uint32_t Did_temp;

    if (*addr_start == addr_end)
            return -1;

    while (1) {

        Flash_get_record(addr_temp, pData_buff, FLASH_SENSOR_DATA_SIZE);

        Record_Head = (Record_Header_t *)pData_buff;
        if(Record_Head->Length == 0 || Record_Head->Length ==0xff || Record_Head->Length > FLASH_SENSOR_DATA_SIZE-1)
            goto NEXT_RECORD;

        Did_temp = (uint32_t)Record_Head->DeviceID[0] << 24 | (uint32_t)Record_Head->DeviceID[1] << 16 | (uint32_t)Record_Head->DeviceID[2] << 8 | (uint32_t)Record_Head->DeviceID[3];
        if(Did_temp == DID)
        {
            Record_time = &Record_Head->Record_time;
            if(Record_time_compare_YMD_HMS(Record_Date, Record_time) == 0){
                break;
            }
            if(Record_time_compare_YMD_HMS(Record_Date, Record_time) < 0){
                calendar = Rtc_get_calendar();
                compareTime.YMD.Year   = TransHexToBcd((uint8_t)(calendar.Year-2000));
                compareTime.YMD.Month  = TransHexToBcd((uint8_t)(calendar.Month));
                compareTime.YMD.Date   = TransHexToBcd((uint8_t)(calendar.DayOfMonth));
                compareTime.HMS.Hour   = TransHexToBcd((uint8_t)(calendar.Hours));
                compareTime.HMS.Minute = TransHexToBcd((uint8_t)(calendar.Minutes));
                compareTime.HMS.Second = TransHexToBcd((uint8_t)(calendar.Seconds));
                if(Record_time_compare_YMD_HMS(Record_time, &compareTime) >= 0)
                {
                    if(addrnext_update){
                        // addr_start move to next data after current position data
                        *addr_start = (addr_temp + FLASH_SENSOR_DATA_SIZE) % (FLASH_SENSOR_DATA_SIZE * FLASH_SENSOR_DATA_NUMBER);//addr_temp;
                    }else
                    {
                        *addr_start = addr_temp; // addr_start move to current position with data
                    }
                }
                return 1; //记录时间已径大于了查找时间，返回 1
            }
        }

        NEXT_RECORD:
        addr_temp = (addr_temp + FLASH_SENSOR_DATA_SIZE) % (FLASH_SENSOR_DATA_SIZE * FLASH_SENSOR_DATA_NUMBER);
        if (addr_temp == addr_end){
            *addr_start = addr_temp;
            return -1;
        }
    }
    if(addrnext_update){
        // addr_start move to next data after current position data
        *addr_start = (addr_temp + FLASH_SENSOR_DATA_SIZE) % (FLASH_SENSOR_DATA_SIZE * FLASH_SENSOR_DATA_NUMBER);//addr_temp;
    }else
    {
        *addr_start = addr_temp; // addr_start move to current position with data
    }
    return 0;
}

int Record_get_Data_with_DID(uint32_t *addr_start, const uint32_t addr_end, char addrnext_update, uint8_t *pData_buff, const uint16_t len_buff, uint32_t DID)
{
    Record_Header_t *Record_Head;
    uint32_t Did_temp = 0;
    uint32_t addr_temp = *addr_start;

    if (*addr_start == addr_end)
            return -1;

    while (1){

        Flash_get_record(addr_temp, pData_buff, FLASH_SENSOR_DATA_SIZE);

        Record_Head = (Record_Header_t *)pData_buff;

        Did_temp = (uint32_t)Record_Head->DeviceID[0] << 24 | (uint32_t)Record_Head->DeviceID[1] << 16 | (uint32_t)Record_Head->DeviceID[2] << 8 | (uint32_t)Record_Head->DeviceID[3];
        if(Did_temp == DID)
            break;

        addr_temp = (addr_temp + FLASH_SENSOR_DATA_SIZE) % (FLASH_SENSOR_DATA_SIZE * FLASH_SENSOR_DATA_NUMBER);
        if (addr_temp == addr_end){ // not find,
            *addr_start = addr_temp;
            return -1;
        }
    }

    if(addrnext_update){
        // addr_start move to next data after current position data
        *addr_start = (addr_temp + FLASH_SENSOR_DATA_SIZE) % (FLASH_SENSOR_DATA_SIZE * FLASH_SENSOR_DATA_NUMBER);//addr_temp;
    }else
    {
        *addr_start = addr_temp; // addr_start move to current position with data
    }
    return 0;
}


Record_time_t current_print_time;

void Record_current_print_time(void)
{
    Calendar calendar;
    calendar = Rtc_get_calendar();

    current_print_time.YMD.Year = calendar.Year;
    current_print_time.YMD.Month = calendar.Month;
    current_print_time.YMD.Date = calendar.DayOfMonth;

    current_print_time.HMS.Hour = calendar.Hours;
    current_print_time.HMS.Minute = calendar.Minutes;
    current_print_time.HMS.Second = calendar.Seconds;
}

int flag_btp_print_enable = 0;
extern void USBCDCD_raiseSimulConnect(void);

void Btp_printList_flag_set(void)
{
    Record_current_print_time();

    flag_btp_print_enable = 1;

    if( 1 ==PrintProStep ){
        Sys_event_post(SYS_EVT_PRINT_CONTINU);//
    }
    //USBCDCD_raiseSimulConnect();
}

void Btp_printList_flag_reset(void)
{
    flag_btp_print_enable = 0;

    Task_sleep(3 * CLOCK_UNIT_S);
    Btp_poweroff();
    Nwk_poweron();
    Sys_event_post(SYSTEMAPP_EVT_DISP);
}

/*
 * Layout
 * |XXXXXXXXXX-XXXXXXXXX-XXXXXXXXXX| // 32 CHAR
 * |19:20 |XXXC |XXXXXXC/XXX%| XXXX|
 * 0      6     11           24    32
 *
 *
 *
 * */
#define MAX_PRINT_LINE_CHAR_LEN 32
void Btp_printList_PrintLayout(const uint32_t * DID, const uint8_t DID_num_2)
{
#if defined(FLASH_EXTERNAL) // && defined(SUPPORT_SENSOR)
    int ret = 0;
    int i;
    uint8_t buff[FLASH_SENSOR_DATA_SIZE];
    int length;
    static int pos[3] = {0};

    uint8_t DID_print_num = DID_num_2;
    static uint8_t DID_num;

    DID_num = DID_num_2;

    uint8_t print_buff[35];

    static uint32_t pPrintRecordAddr[LSIT_PRINT_SELECT_MAX] = {0};
    static SENSOR_TYPE type_list[LSIT_PRINT_SELECT_MAX] = {SEN_TYPE_NONE};
    static Record_time_t maxRecord_time = {0};
    static FlashPrintRecordAddr_t recordAddr;

    Record_Header_t *Record_Head;
    Record_time_t Record_time = {0};

    uint8_t find_sum = 0;
    int br_flag = 0;

if(0 == PrintProStep){

    memset(&maxRecord_time, 0, sizeof(Record_time_t));
#ifdef  SUPPORT_DEVICED_STATE_UPLOAD
	Btp_print_num = 0;	  
	Flash_store_devices_state(TYPE_BT_PRINT_START);
#endif

    ret = Btp_printList_print_HEAD();
    if(0 != ret)
        goto END_RET;

    Btp_send_cmd("\n设备列表：\n");
    for(i=0; i < DID_num; i++){
        length = sprintf((char *)buff, "DID%d=%02x%02x%02x%02x\n",i, HIBYTE_ZKS(HIWORD_ZKS(DID[i])),
                LOBYTE_ZKS(HIWORD_ZKS(DID[i])), HIBYTE_ZKS(LOWORD_ZKS(DID[i])),LOBYTE_ZKS(LOWORD_ZKS(DID[i])));
        Btp_send_data((uint8_t*)buff, length);
    }

    //recordAddr = Flash_get_current_record_addr(); //Flash_get_record_addr();
    if(1 == Record_get_status()){// 还在记录状态  读取当前记录地址作为end点
        recordAddr = Flash_get_current_record_addr();//Flash_get_record_addr();
    }else{
        recordAddr = Flash_get_record_addr();
    }
    if (recordAddr.start == recordAddr.end) {
        ret = -1;
        goto END_RET;
    }

    // 指针赋值
    for(i=0; i < DID_num; i++){
        pPrintRecordAddr[i] = recordAddr.start;
    }

    // get first
    for(i=0; i < DID_num; i++){

        ret = Record_get_Data_with_DID(&pPrintRecordAddr[i], recordAddr.end, 0, buff, FLASH_SENSOR_DATA_SIZE, DID[i]);
        if(0 == ret){
            Record_Head = (Record_Header_t *)buff;
            Record_time = Record_Head->Record_time;

            //tempRecord_time[i] = Record_time;
            if(Record_time_compare_YMD_HMS(&maxRecord_time, &Record_time) < 0){
                maxRecord_time = Record_time;
            }
            type_list[i] =  Record_data_Get_first_temp_type( buff);

            int length;
            length = sprintf((char *)buff, "%02x%02x%02x%02x find.\n", HIBYTE_ZKS(HIWORD_ZKS(DID[i])),
                             LOBYTE_ZKS(HIWORD_ZKS(DID[i])), HIBYTE_ZKS(LOWORD_ZKS(DID[i])),LOBYTE_ZKS(LOWORD_ZKS(DID[i])));
            Btp_send_data((uint8_t*)buff, length);
            find_sum++;
        }else{
            int length;
            length = sprintf((char *)buff, "%02x%02x%02x%02x No find.\n", HIBYTE_ZKS(HIWORD_ZKS(DID[i])),
                             LOBYTE_ZKS(HIWORD_ZKS(DID[i])), HIBYTE_ZKS(LOWORD_ZKS(DID[i])),LOBYTE_ZKS(LOWORD_ZKS(DID[i])));

            Btp_send_data((uint8_t*)buff, length);
            type_list[i] = SEN_TYPE_NONE;
        }
    }
    if(find_sum == 0){
        ret = -1;
        goto END_RET;
    }
    ret = 0;
    for(i=0; i < DID_num; i++){
        pPrintRecordAddr[i] = recordAddr.start;
    }

    if(type_list[0] == SEN_TYPE_NONE || type_list[0] == SEN_TYPE_NTC || type_list[0] == SEN_TYPE_OPT3001 ){
        pos[0] = 7;

        pos[1] = 13;
        if(type_list[1] == SEN_TYPE_SHT2X)
            pos[2] = 25;
        else
            pos[2] = 20;
    }

    if(type_list[0] == SEN_TYPE_SHT2X){
        pos[0] = 7;

        pos[1] = 19;
        if(type_list[1] == SEN_TYPE_SHT2X){
            pos[2] = 31;
            DID_print_num = 2; // 双温湿度 只显示前两个选择
        }
        else
            pos[2] = 25;
    }
    // 打印列头
    Btp_printList_print_DID_type_list_head(print_buff, type_list, DID, DID_num,0);
    // 打印 年-月-日
    length = Btp_printList_print_YMD_pos( print_buff, &maxRecord_time,  0);
    Btp_send_data((uint8_t*)print_buff, length);
    Btp_send_cmd("\n");

    PrintProStep = 1;
    Btp_printList_flag_set();
    return;
}

    if(1 == PrintProStep)
    //while (1)
    {
        if(PIN_getInputValue(BT_CONNECT_PIN) == 0)
            goto BREAK_OUT;
        //recordAddr = Flash_get_current_record_addr();//Flash_get_record_addr(); //更新 recordAddr.end 时间
        if(1 == Record_get_status()){
            recordAddr = Flash_get_current_record_addr();//Flash_get_record_addr();
        }else{
            recordAddr = Flash_get_record_addr();
        }
        // 0X20 (space)
        memset(print_buff, 0x20 , sizeof(print_buff));
        length = Btp_printList_print_HMS_pos( print_buff, &maxRecord_time, 0);

        for(i=0; i < DID_print_num /*DID_num*/; i++) {

#define USE_DATE_MODE 1
#if !USE_DATE_MODE
            ret = Record_get_Data_with_DID(&pPrintRecordAddr[i], recordAddr.end, 1, buff, FLASH_SENSOR_DATA_SIZE, DID[i]);
#else
            ret = Record_get_Data_with_DID_Date(&pPrintRecordAddr[i], recordAddr.end, 1, buff, FLASH_SENSOR_DATA_SIZE, DID[i], &maxRecord_time);
#endif
            // 读出的数据
            Record_Head = (Record_Header_t *)buff;

            if(0 == ret){
                //Btp_printList_print_DATA(print_buff,buff);
                length += Btp_printList_print_DATA_pos( print_buff, type_list[i],  buff, pos[i]);
            }else{
                length += Btp_printList_print_sign(print_buff, "--", pos[i]);

            }
       }

        Btp_send_data((uint8_t*)print_buff, MAX_PRINT_LINE_CHAR_LEN );
        Btp_send_cmd("\n");

        // update HMS
        // 没有更新日期，就只更新时分
        // update YMD // 时间基准内 选中的 都没有找到最小日期数据，按选中的个数中的最小时间为基准开始查找
        if(1 == Record_time_HMS_add_one_min( &maxRecord_time)){ // RET = 1: YMD update also
            length = Btp_printList_print_YMD_pos( print_buff, &maxRecord_time,  0);
            Btp_send_data((uint8_t*)print_buff, length);
            Btp_send_cmd("\n");
        }

#ifdef  SUPPORT_DEVICED_STATE_UPLOAD
		Btp_print_num++;	  
#endif

        // judgement if all find to end position
        for(i =0 ; i < DID_num; i++){
            if (pPrintRecordAddr[i] >= recordAddr.end)
                br_flag++;
        }

        #ifdef SUPPORT_WATCHDOG
        if (watchdogHandle != NULL)
            Watchdog_clear(watchdogHandle);
        #endif

        if(br_flag == DID_num)
            //break;
            goto BREAK_OUT;
#if 1   // 网管时间未同步 如果还以当前时间为基准 结束打印则不准确
        if(1 == Record_get_status()){ // 还在记录状态 采用时间基准
            if(Record_time_compare_YMD_HMS(&maxRecord_time, &current_print_time) > 0){ // 如果要读取的时间 大于 打印开始的时间 则退出打印
                //break;
                goto BREAK_OUT;
            }
        }
#endif
        if( 1 ==PrintProStep ){
            Sys_event_post(SYS_EVT_PRINT_CONTINU);//
            return;
        }
        //continue;
        BREAK_OUT:
        {
          PrintProStep = 0;
        }

    }
    ret = 0;

END_RET:
    if(0 == ret){
        Btp_printList_print_END();
    }else{
        Btp_send_cmd("\n=====END====\n");
    }

#ifdef  SUPPORT_DEVICED_STATE_UPLOAD
		Flash_store_devices_state(TYPE_BT_PRINT_END);
#endif
		// 蓝牙打印断电
		Btp_printList_flag_reset();
#endif
}

uint32_t PrintListDID[LSIT_PRINT_SELECT_MAX];
uint8_t DID_Num = 0;
void Btp_print_record(void)
{
    Btp_printList_PrintLayout((const uint32_t *)&PrintListDID, DID_Num);
}

int Btp_printList_get_flag_status(void)
{
    return flag_btp_print_enable;
}

/* Print List
 * */
void Btp_printList_record_bind_node(void)
{
#if defined(FLASH_EXTERNAL)  //&& defined(SUPPORT_SENSOR)
    //uint32_t PrintListDID[LSIT_PRINT_SELECT_MAX];
    //uint8_t DID_Num = 0;
    uint8_t ch_num = '0';

    PrintList_getSelectedDID_NUM(PrintListDID,  &DID_Num);

    switch(DID_Num){
    case 1:
    case 2:
    case 3:
        Btp_send_cmd("Select ");
        ch_num = '0' + DID_Num;
        Btp_send_data((uint8_t*)&ch_num, 1 );
        Btp_send_cmd(" Group\n");
        break;
    case 0:
        Btp_send_data((uint8_t*)&ch_num, 1 );
        Btp_send_cmd(" Group\n");
    default:
        Btp_send_cmd("Support format\n1.Max support 3 Group T\n2. 2 group T/H\n");
        break;
    }
    if(DID_Num > 0)
        Btp_printList_PrintLayout((const uint32_t *)&PrintListDID, DID_Num);

#else
    Btp_send_cmd("\e!\x30无存储功能\r\n\n\n");
#endif

   // Btp_printList_flag_reset();
}
#endif // SUPPORT_BLE_PRINT_LIST_MODE

#endif  /* SUPPORT_BLUETOOTH_PRINT */
