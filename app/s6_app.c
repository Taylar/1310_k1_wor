/*
* @Author: zxt
* @Date:   2018-03-09 11:15:03
* @Last Modified by:   zxt
* @Last Modified time: 2018-03-09 17:13:01
*/
#include "../general.h"





/***** Defines *****/
#define     SCREEN_SLEEP_TIME           15

/***** Type declarations *****/


/***** Variable declarations *****/
uint8_t  screenSleepMonitorCnt;



/***** Prototypes *****/



/***** Function definitions *****/


//***********************************************************************************
// brief:   
// 
// parameter: 
//***********************************************************************************
void S6HwInit(void)
{
	KeyInit();
    KeyRegister(SystemKeyEventPostIsr, KEY_0_SHORT_PRESS);

    KeyRegister(SystemLongKeyEventPostIsr, KEY_0_LONG_PRESS);

	AdcDriverInit();
    Disp_init();


    Spi_init();

    Flash_init();


#ifdef BOARD_S2_2
    DeepTemp_FxnTable.initFxn(MAX31855_SPI_CH0);
#endif

#ifdef BOARD_S6_6
    KeyRegister(SystemKey1EventPostIsr, KEY_1_SHORT_PRESS);

    KeyRegister(SystemLongKey1EventPostIsr, KEY_1_LONG_PRESS);
    
    screenSleepMonitorCnt = 0;
    NTC_FxnTable.initFxn(NTC_CH0);

    UsbIntInit(SystemUsbIntEventPostIsr);
#endif

    Battery_init();
    Battery_voltage_measure();

    Disp_poweron();
    Disp_proc();
}



//***********************************************************************************
// brief:the S6 Concenter short key application
// 
// parameter: 
//***********************************************************************************
void S6ShortKeyApp(void)
{
    screenSleepMonitorCnt = 0;
    switch(deviceMode)
    {
        case DEVICES_ON_MODE:
        Disp_info_switch();
        Disp_proc();
        // Led_ctrl(LED_B, 1, 500 * CLOCK_UNIT_MS, 1);
        break;

        case DEVICES_OFF_MODE:
        // Disp_info_switch();
        // Disp_proc();
        Led_ctrl(LED_R, 1, 500 * CLOCK_UNIT_MS, 1);
        break;


        case DEVICES_MENU_MODE:
        Menu_action_proc(MENU_AC_DOWN);
        Disp_proc();
        break;

        case DEVICES_SLEEP_MODE:
        Disp_poweron();
        Disp_proc();
        deviceMode = DEVICES_ON_MODE;
        break;
    }
}

//***********************************************************************************
// brief:the Concenter long key application
// 
// parameter: 
//***********************************************************************************
void S6ConcenterLongKeyApp(void)
{
    screenSleepMonitorCnt = 0;
    switch(deviceMode)
    {
        case DEVICES_ON_MODE:
        Disp_poweroff();
        ConcenterSleep();
        Led_ctrl(LED_R, 1, 250 * CLOCK_UNIT_MS, 6);
        break;

        case DEVICES_OFF_MODE:
        Led_ctrl(LED_B, 1, 250 * CLOCK_UNIT_MS, 6);
        ConcenterWakeup();
        Disp_poweron();
        Disp_proc();
        break;

        case DEVICES_SLEEP_MODE:
        Disp_poweron();
        Disp_proc();
        deviceMode = DEVICES_ON_MODE;
        break;
    }
}

//***********************************************************************************
// brief:the S6 Concenter short key application
// 
// parameter: 
//***********************************************************************************
void S6ShortKey1App(void)
{
    screenSleepMonitorCnt = 0;
    switch(deviceMode)
    {
        case DEVICES_ON_MODE:
        Disp_info_close();
        Disp_proc();
        break;

        case DEVICES_OFF_MODE:
        break;

        case DEVICES_MENU_MODE:
        Menu_action_proc(MENU_AC_ENTER);
        if(DEVICES_ON_MODE == deviceMode)
        {
            Disp_proc();
        }
        break;

        case DEVICES_SLEEP_MODE:
        Disp_poweron();
        Disp_proc();
        deviceMode = DEVICES_ON_MODE;
        break;

    }
}



//***********************************************************************************
// brief:the Concenter long key application
// 
// parameter: 
//***********************************************************************************
void S6LongKey1App(void)
{
    screenSleepMonitorCnt = 0;
    switch(deviceMode)
    {
        case DEVICES_ON_MODE:
        deviceMode = DEVICES_MENU_MODE;
        PoweroffMenu_init();
        Disp_proc();
        break;

        case DEVICES_OFF_MODE:
        Led_ctrl(LED_B, 1, 250 * CLOCK_UNIT_MS, 6);
        ConcenterWakeup();
        Disp_poweron();
        Disp_info_close();
        Disp_proc();
        break;

        case DEVICES_MENU_MODE:

        break;

        case DEVICES_SLEEP_MODE:
        Disp_poweron();
        Disp_proc();
        deviceMode = DEVICES_ON_MODE;
        break;
    }
}


//***********************************************************************************
// brief:the screen monitor
// 
// parameter: 
//***********************************************************************************
void S6AppRtcProcess(void)
{
    if(Disp_powerState())
    {
        screenSleepMonitorCnt ++;
        if(screenSleepMonitorCnt >= SCREEN_SLEEP_TIME)
        {
            Disp_poweroff();
            deviceMode = DEVICES_SLEEP_MODE;
        }
    }
}



#ifdef SUPPORT_NETGATE_DISP_NODE

sensordata_mem pMemSensor[MEMSENSOR_NUM];//  
uint8_t MemSensorIndex = 0;

//***********************************************************************************
//
// unpackage  sensor data and save  sensor mac , index, type and value to mem
//
//***********************************************************************************
void sensor_unpackage_to_memory(uint8_t *pData, uint16_t length)
{    
    uint8_t i;
    uint16_t Index;    
    sensordata_mem cursensor;
    
    Index = 2;//DeviceId  start

    HIBYTE(HIWORD(cursensor.DeviceId)) = pData[Index++];
    LOBYTE(HIWORD(cursensor.DeviceId)) = pData[Index++];
    HIBYTE(LOWORD(cursensor.DeviceId)) = pData[Index++];
    LOBYTE(LOWORD(cursensor.DeviceId)) = pData[Index++];  
    
    Index = 16;//sensor  start

    while(Index < length)
    {
        cursensor.index = pData[Index++];
        cursensor.type  = pData[Index++];
        
        switch(cursensor.type)
        {
            case PARATYPE_TEMP_HUMI_SHT20:
            HIBYTE(cursensor.temp) = pData[Index++];
            LOBYTE(cursensor.temp) = pData[Index++];
            HIBYTE(cursensor.humi) = pData[Index++];
            LOBYTE(cursensor.humi) = pData[Index++];
            break;

            case PARATYPE_NTC:
            HIBYTE(cursensor.temp) = pData[Index++];
            LOBYTE(cursensor.temp) = pData[Index++];
            break;

            case PARATYPE_ILLUMINATION:
            break;

            case PARATYPE_TEMP_MAX31855:
            HIBYTE(HIWORD(cursensor.tempdeep)) = pData[Index++];
            LOBYTE(HIWORD(cursensor.tempdeep)) = pData[Index++];
            HIBYTE(LOWORD(cursensor.tempdeep)) = pData[Index++];
            cursensor.tempdeep >>= 8;
            break;

        }

        //find in mem 
        for (i = 0; i < MEMSENSOR_NUM; ++i) {
            if (((pMemSensor) + i)->DeviceId == cursensor.DeviceId &&
               ((pMemSensor) + i)->index == cursensor.index &&
               ((pMemSensor) + i)->type == cursensor.type )
                break;

        }
        
        if (i < MEMSENSOR_NUM) {//update
             memcpy((pMemSensor) + i, &cursensor, sizeof(sensordata_mem));
        }
        else {
            //new sensor id
            memcpy((pMemSensor) + MemSensorIndex, &cursensor, sizeof(sensordata_mem));

            MemSensorIndex = (MemSensorIndex + 1) % MEMSENSOR_NUM;
        }
        
        
    }

}

bool get_next_sensor_memory(sensordata_mem *pSensor)
{    
    static uint8_t dispSensorIndex = 0;

restart:
    if (((sensordata_mem*)(pMemSensor) + dispSensorIndex)->DeviceId != 0x00000000 ){//valid data  
        memcpy(pSensor, (sensordata_mem*)(pMemSensor) + dispSensorIndex, sizeof(sensordata_mem));
        dispSensorIndex = (dispSensorIndex + 1) % MEMSENSOR_NUM;
        return true;
    }
    else {
        if(dispSensorIndex == 0)    
            return false;
        else {
            dispSensorIndex = 0;
            goto restart;
        }
    }
}

#endif

