#ifndef  ELECTRIC_SHOCK
#define  ELECTRIC_SHOCK


typedef enum {
    ELE_SHOCK_HIGH,
    ELE_SHOCK_MID,
    ELE_SHOCK_LOW,
    ELE_SHOCK_POWER_ENABLE,
    ELE_MOTO_ENABLE,
    ELE_PREVENT_INSERT_ENABLE,
    ELE_PREVENT_INSERT2_ENABLE,
    ELE_SHOCK_PIN_MAX
} ELECTRIC_CTR_ID;


#define 		ELECTRIC_LOW_LEVEL		0
#define 		ELECTRIC_MID_LEVEL		1
#define 		ELECTRIC_HIGH_LEVEL		2


#define         ELECTRIC_SHOCK_TIME     10

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


extern void EletricShockPulseEnable(void);
extern void EletricShockPulseDisable(void);
extern void EletricShockPulseInit(void);
extern void EletricPulseSetTime_S(uint16_t keepTime_S);
extern void ElecPreventInsertInit(void);
extern void ElecPreventInsertMeasure(void);
extern uint8_t ElecPreventInsertState(void);


extern void eleShock_ctrl(uint8_t ledId, uint8_t state, uint32_t period, uint8_t times);
extern void eleShock_ctrl2(uint8_t ledId, uint8_t state, uint32_t period, uint32_t period2, uint8_t times);
extern void eleShock_set(uint8_t ledId, uint8_t status);
extern void eleShock_toggle(uint8_t ledId);
extern void ElectricShockInit(void);
extern uint8_t DestroyPinRead(void);

extern void ElectricShockPowerEnable(void);

extern void ElectricShockPowerDisable(void);

void ElectricShockLevelSet(uint8_t level);

#endif //ELECTRIC_SHOCK

