#include "../general.h"

/***** Defines *****/




/***** Type declarations *****/


/***** Variable declarations *****/






/***** Prototypes *****/




/***** Function definitions *****/

/*********************************************************************
 * @fn      DisableCache
 *
 * @brief   Resumes system after a write to flash, if necessary.
 *
 * @param   None.
 *
 * @return  VIMS_MODE_ENABLED if cache was in use before this operation,
 *          VIMS_MODE_DISABLED otherwise.
 */
static uint8_t DisableCache(void)
{
  uint8_t state = VIMSModeGet(VIMS_BASE);

  // Check VIMS state
  if (state != VIMS_MODE_DISABLED)
  {
    // Invalidate cache
    VIMSModeSet(VIMS_BASE, VIMS_MODE_DISABLED);

    // Wait for disabling to be complete
    while (VIMSModeGet(VIMS_BASE) != VIMS_MODE_DISABLED);
  }

  return state;
}


/*********************************************************************
 * @fn      EnableCache
 *
 * @brief   Prepares system for a write to flash, if necessary.
 *
 * @param   None.
 *
 * @return  None.
 */
static void EnableCache(uint8_t state)
{
  if (state != VIMS_MODE_DISABLED)
  {
    // Enable the Cache.
    VIMSModeSet(VIMS_BASE, VIMS_MODE_ENABLED);
  }
}


/*********************************************************************
 * @fn      WriteInternalFlash
 *
 * @brief   Write data to flash.
 *
 * @param   addr   - addr into flash page to begin writing
 * @param   pBuf   - pointer to buffer of data to write
 * @param   len    - length of data to write in bytes
 *
 * @return  None.
 */
void WriteInternalFlash(uint8_t *pBuf, uint32_t addr, uint16_t len)
{
  uint8_t cacheState;

  cacheState = DisableCache();

  FlashProgram(pBuf, addr, len);

  EnableCache(cacheState);
}

/*********************************************************************
 * @fn      EraseInternalFlash
 *
 * @brief   Erase selected flash page(4k).
 *
 * @param   addr - the page to erase.
 *
 * @return  None.
 */
void EraseInternalFlash(uint32_t addr)
{
  uint8_t cacheState;

  cacheState = DisableCache();

  FlashSectorErase(addr);

  EnableCache(cacheState);
}




const uint8_t configFlag[12] = CONFIG_VALID_FLAG;
//***********************************************************************************
// brief: store the config from the ext flash   
// 
// note: every sector is 4K
// parameter: 
//***********************************************************************************
void Flash_store_config(void)
{


    EraseInternalFlash(CONFIG_FLAG_INTERNAL_ADDR);
    
    WriteInternalFlash((uint8_t *)configFlag, CONFIG_FLAG_INTERNAL_ADDR, 12);
    WriteInternalFlash((uint8_t*)(&g_rSysConfigInfo), CONFIG_DATA_INTERNAL_ADDR,  sizeof(ConfigInfo_t));
    
}

//***********************************************************************************
// brief: load the config from the ext flash   
// 
// parameter: 
//***********************************************************************************
bool Flash_load_config(void)
{
    uint8_t i, buf[16];

    memcpy(buf, (uint8_t *)CONFIG_FLAG_INTERNAL_ADDR, 12);
    for(i = 0; i < 12; i++)
    {
        if(configFlag[i] != buf[i])
        {
            return false;
        }
    }

    memcpy((uint8_t*)&g_rSysConfigInfo, (uint8_t *)CONFIG_DATA_INTERNAL_ADDR, sizeof(ConfigInfo_t));
    // if(g_rSysConfigInfo.swVersion != FW_VERSION)
    // {
    //     return false;
    // }

    return true;
}


//***********************************************************************************
// brief: reset the config data  
// 
// parameter: 
//***********************************************************************************
void Sys_config_reset(void)
{
    uint8_t i;

    g_rSysConfigInfo.size = sizeof(ConfigInfo_t);
    g_rSysConfigInfo.swVersion = FW_VERSION;
    g_rSysConfigInfo.DeviceId[0] = (uint8_t)((DECEIVE_ID_DEFAULT>>24)&0xff);
    g_rSysConfigInfo.DeviceId[1] = (uint8_t)((DECEIVE_ID_DEFAULT>>16)&0xff);
    g_rSysConfigInfo.DeviceId[2] = (uint8_t)((DECEIVE_ID_DEFAULT>>8)&0xff);
    g_rSysConfigInfo.DeviceId[3] = (uint8_t)((DECEIVE_ID_DEFAULT)&0xff);;

    g_rSysConfigInfo.customId[0] = (uint8_t)(CUSTOM_ID_DEFAULT >> 8);
    g_rSysConfigInfo.customId[1] = (uint8_t)(CUSTOM_ID_DEFAULT);

    g_rSysConfigInfo.status = 0;

    for (i = 0; i < MODULE_SENSOR_MAX; i++) {
        g_rSysConfigInfo.sensorModule[i]     = SEN_TYPE_NONE;
        g_rSysConfigInfo.alarmTemp[i].high   = ALARM_TEMP_HIGH;
        g_rSysConfigInfo.alarmTemp[i].low    = ALARM_TEMP_LOW;
        g_rSysConfigInfo.WarningTemp[i].high = ALARM_TEMP_HIGH;
        g_rSysConfigInfo.WarningTemp[i].low  = ALARM_TEMP_LOW;
    }

#ifdef      BOARD_S2_2

    g_rSysConfigInfo.module          = MODULE_NWK | MODULE_RADIO;
    g_rSysConfigInfo.serverIpAddr[0] = 114;
    g_rSysConfigInfo.serverIpAddr[1] = 215;
    g_rSysConfigInfo.serverIpAddr[2] = 122;
    g_rSysConfigInfo.serverIpAddr[3] = 32;
    g_rSysConfigInfo.serverIpPort    = 12200;

    g_rSysConfigInfo.batLowVol       = BAT_VOLTAGE_LOW;
    g_rSysConfigInfo.apnuserpwd[0]   = 0;
    g_rSysConfigInfo.hbPeriod        = UPLOAD_PERIOD_DEFAULT;     // unit is sec
    g_rSysConfigInfo.rfStatus       |= STATUS_1310_MASTER;
#endif

#ifdef      BOARD_S6_6

    g_rSysConfigInfo.module          = MODULE_NWK | MODULE_RADIO | MODULE_LCD;
    g_rSysConfigInfo.serverIpAddr[0] = 114;
    g_rSysConfigInfo.serverIpAddr[1] = 215;
    g_rSysConfigInfo.serverIpAddr[2] = 122;
    g_rSysConfigInfo.serverIpAddr[3] = 32;
    g_rSysConfigInfo.serverIpPort    = 12200;

    g_rSysConfigInfo.batLowVol       = BAT_VOLTAGE_LOW;
    g_rSysConfigInfo.apnuserpwd[0]   = 0;
    g_rSysConfigInfo.hbPeriod        = UPLOAD_PERIOD_DEFAULT;     // unit is sec
    g_rSysConfigInfo.rfStatus       |= STATUS_1310_MASTER;
    // g_rSysConfigInfo.sensorModule[0] = SEN_TYPE_SHT2X;
    // g_rSysConfigInfo.sensorModule[1] = SEN_TYPE_OPT3001;
#endif


#ifdef      BOARD_S1_2

    g_rSysConfigInfo.module          = MODULE_RADIO;
    g_rSysConfigInfo.batLowVol       = BAT_VOLTAGE_LOW;
    g_rSysConfigInfo.apnuserpwd[0]   = 0;
    g_rSysConfigInfo.rfStatus        = 0;
    g_rSysConfigInfo.sensorModule[0] = SEN_TYPE_SHT2X;
#endif


    g_rSysConfigInfo.collectPeriod   = UPLOAD_PERIOD_DEFAULT;   //unit is sec
    
    g_rSysConfigInfo.uploadPeriod    = UPLOAD_PERIOD_DEFAULT; // unit is sec
    
    g_rSysConfigInfo.ntpPeriod       = NTC_DEFAULT;    // 
    
    g_rSysConfigInfo.gnssPeriod      = 10;  // 10sec
    
    
}

