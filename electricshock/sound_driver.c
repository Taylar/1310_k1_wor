#include "../general.h"


#define SOUND_DATA_PIN                                IOID_25     //澹伴煶鏁版嵁绾�

#define SOUND_BUSY_PIN                             	  IOID_24      //澹伴煶鐘舵�佺嚎

#define SOUND_STATE_BUSY							1
#define SOUND_STATE_IDLE							0



static PIN_State   soundDriverState;
static PIN_Handle  soundDriverHandle;

const PIN_Config soundPinTable[] = {
    SOUND_DATA_PIN | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,       /* 浣庢。鐢靛帇妗ｄ綅         */
    SOUND_BUSY_PIN | PIN_INPUT_EN | PIN_PULLDOWN,
    PIN_TERMINATE
};

uint8_t soundEventType;

void SoundEventSet(uint8_t event)
{
	soundEventType = event;
	Sys_event_post(SYS_EVT_SOUND_PLAY);
}

void SoundDriverInit(void)
{
    soundDriverHandle = PIN_open(&soundDriverState, soundPinTable);
}



void SoundDriverSet(uint8_t soundType)
{
	uint8_t i;
	uint8_t delayTime = 0;
	while(delayTime++<20){
		if(PIN_getInputValue(SOUND_BUSY_PIN) == SOUND_STATE_IDLE)
			break;
	}
	if(delayTime >= 20)
		return;

	IntMasterDisable();
	PIN_setOutputValue(soundDriverHandle, SOUND_DATA_PIN, 1);
	delay_ms(8);
	PIN_setOutputValue(soundDriverHandle, SOUND_DATA_PIN, 0);
	delay_ms(1);

	for(i = 0; i < 8; i++){
		if(soundType & (0x80>>i)){
			PIN_setOutputValue(soundDriverHandle, SOUND_DATA_PIN, 1);
			delay_us(1500);
			PIN_setOutputValue(soundDriverHandle, SOUND_DATA_PIN, 0);
			delay_us(500);
		}else{
			PIN_setOutputValue(soundDriverHandle, SOUND_DATA_PIN, 1);
			delay_us(500);
			PIN_setOutputValue(soundDriverHandle, SOUND_DATA_PIN, 0);
			delay_us(1500);
		}
	}

	IntMasterEnable();	
}
