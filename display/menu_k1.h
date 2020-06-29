/*
 * menu_k1.h
 *
 *  Created on: 2020?��6??2��?
 *      Author: Administrator
 */

#ifndef ZKS_DISPLAY_MENU_K1_H_
#define ZKS_DISPLAY_MENU_K1_H_
#ifdef BOARD_S6_6
typedef struct _MenuMode {
    uint8_t count;                  //�̡�?��2??����?��y
    const uint8_t *string;          //2?�̣�������a
    void (*func)();                 //?����?o����y
} MenuMode_t;

typedef struct _MenuModeObject {
   uint8_t index;
   MenuMode_t item;
   uint32_t numEnter;
   uint8_t keyDoing;
   uint8_t selectIndex;
   uint32_t devicesId;
   uint32_t groudId;
   uint8_t  timerefesh;
} MenuModeObject;

typedef struct _MenuAlarmObject {
   uint32_t devicesId;
   uint8_t  alarmType;
} MenuAlarmObject;

typedef enum {
    ALARM_TYPE_NULL=0,
    ALARM_TYPE_LOW_POWER=0x01,
    ALARM_TYPE_UNWEAR=0x02,
    ALARM_TYPE_MAX
} ALARM_TYPE;

extern void menuc_main(KEY_CODE_E keyCode);
void Menu_low_power_display(uint32_t devicesId);
void Menu_not_wearing_well_display(uint32_t devicesId);
void Menu_term_is_destroyed(uint32_t devicesId);
void power_on_init_key_code(void);
extern void menuc_alarm_main(KEY_CODE_E keyCode);
#endif
#endif /* ZKS_DISPLAY_MENU_K1_H_ */
