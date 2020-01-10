#ifndef  ELECTRIC_SHOCK
#define  ELECTRIC_SHOCK


typedef enum {
    ELE_SHOCK_HIGH,
    ELE_SHOCK_MID,
    ELE_SHOCK_LOW,
    ELE_SHOCK_ENABLE,
    ELE_SHOCK_PIN_MAX
} LED_ID;

// variable typedef
typedef struct 
{
	uint8_t enable;
	uint8_t state;
	uint8_t times;
	uint8_t periodT1Set;
	uint8_t periodT2Set;
	uint8_t periodT1;
	uint8_t periodT2;
} singleport_drive_t;


extern void eleShock_ctrl(uint8_t ledId, uint8_t state, uint32_t period, uint8_t times);
extern void eleShock_ctrl2(uint8_t ledId, uint8_t state, uint32_t period, uint32_t period2, uint8_t times);
extern void eleShock_set(uint8_t ledId, uint8_t status);
extern void eleShock_toggle(uint8_t ledId);
extern void ElectricShockInit(void);




#endif //ELECTRIC_SHOCK
