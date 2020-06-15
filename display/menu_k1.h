/*
 * menu_k1.h
 *
 *  Created on: 2020?¨º6??2¨¨?
 *      Author: Administrator
 */

#ifndef ZKS_DISPLAY_MENU_K1_H_
#define ZKS_DISPLAY_MENU_K1_H_
#ifdef BOARD_S6_6


typedef struct _MenuMode {
    uint8_t count;                  //¦Ì¡À?¡ã2??¨²¦Ì?¨ºy
    const uint8_t *string;          //2?¦Ì£¤¡À¨º¨¬a
    void (*func)();                 //?¨²¦Ì?o¡¥¨ºy
} MenuMode_t;

typedef struct _MenuModeObject {
   uint8_t index;
   MenuMode_t item;
   uint16_t numEnter;
   uint8_t keyDoing;
   uint8_t selectIndex;
   uint16_t devicesId;
   uint16_t groudId;
} MenuModeObject;
extern void menuc_main(KEY_CODE_E keyCode);
void Menu_low_power_display(uint32_t devicesId);
void Menu_not_wearing_well_display(uint32_t devicesId);
#endif
#endif /* ZKS_DISPLAY_MENU_K1_H_ */
