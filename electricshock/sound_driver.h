#ifndef SOUND_DRIVER_H
#define SOUND_DRIVER_H

#define  	SOUND_TYPE_LOW_BAT				0X00
#define  	SOUND_TYPE_INSERT				0X01
#define  	SOUND_TYPE_CONTROL_ENABLE		0X02
#define  	SOUND_TYPE_DESTROYED			0X03
#define  	SOUND_TYPE_SINGLE_TEST			0X04
#define  	SOUND_TYPE_SET_GROUND_SUSCESS	0X05
#define  	SOUND_TYPE_SET_POWER_SUSCESS	0X06
#define  	SOUND_TYPE_DI					0X07
#define  	SOUND_TYPE_DI_DI				0X08
#define  	SOUND_TYPE_DI_DI_DI				0X09


#define  	SOUND_TYPE_VOLUME_MAX			0XFF

void SoundDriverInit(void);
void SoundEventSet(uint8_t event);
void SoundDriverSet(uint8_t soundType);
extern uint8_t soundEventType;


#endif //_SOUND_DRIVER_H
