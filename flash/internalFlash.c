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



