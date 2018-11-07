/*
* @Author: zxt
* @Date:   2018-05-09 14:02:37
* @Last Modified by:   zxt
* @Last Modified time: 2018-08-28 19:16:31
*/
#include "../general.h"

#ifdef BOARD_B2S
#define BUZZER_PIN                       IOID_19
#else
#define BUZZER_PIN                       IOID_15
#endif //BOARD_B2S

PWMTimerCC26XX_Object pwmtimerCC26xxObjects[CC1310_LAUNCHXL_PWMCOUNT];

const PWMTimerCC26XX_HwAttrs pwmtimerCC26xxHWAttrs[CC1310_LAUNCHXL_PWMCOUNT] = {
    { .pwmPin = BUZZER_PIN, .gpTimerUnit = CC1310_LAUNCHXL_GPTIMER0A },
    // { .pwmPin = CC1310_LAUNCHXL_PWMPIN1, .gpTimerUnit = CC1310_LAUNCHXL_GPTIMER0B },
    // { .pwmPin = CC1310_LAUNCHXL_PWMPIN2, .gpTimerUnit = CC1310_LAUNCHXL_GPTIMER1A },
    // { .pwmPin = CC1310_LAUNCHXL_PWMPIN3, .gpTimerUnit = CC1310_LAUNCHXL_GPTIMER1B },
    // { .pwmPin = CC1310_LAUNCHXL_PWMPIN4, .gpTimerUnit = CC1310_LAUNCHXL_GPTIMER2A },
    // { .pwmPin = CC1310_LAUNCHXL_PWMPIN5, .gpTimerUnit = CC1310_LAUNCHXL_GPTIMER2B },
    // { .pwmPin = CC1310_LAUNCHXL_PWMPIN6, .gpTimerUnit = CC1310_LAUNCHXL_GPTIMER3A },
    // { .pwmPin = CC1310_LAUNCHXL_PWMPIN7, .gpTimerUnit = CC1310_LAUNCHXL_GPTIMER3B },
};

const PWM_Config PWM_config[CC1310_LAUNCHXL_PWMCOUNT] = {
    { &PWMTimerCC26XX_fxnTable, &pwmtimerCC26xxObjects[CC1310_LAUNCHXL_PWM0], &pwmtimerCC26xxHWAttrs[CC1310_LAUNCHXL_PWM0] },
    // { &PWMTimerCC26XX_fxnTable, &pwmtimerCC26xxObjects[CC1310_LAUNCHXL_PWM1], &pwmtimerCC26xxHWAttrs[CC1310_LAUNCHXL_PWM1] },
    // { &PWMTimerCC26XX_fxnTable, &pwmtimerCC26xxObjects[CC1310_LAUNCHXL_PWM2], &pwmtimerCC26xxHWAttrs[CC1310_LAUNCHXL_PWM2] },
    // { &PWMTimerCC26XX_fxnTable, &pwmtimerCC26xxObjects[CC1310_LAUNCHXL_PWM3], &pwmtimerCC26xxHWAttrs[CC1310_LAUNCHXL_PWM3] },
    // { &PWMTimerCC26XX_fxnTable, &pwmtimerCC26xxObjects[CC1310_LAUNCHXL_PWM4], &pwmtimerCC26xxHWAttrs[CC1310_LAUNCHXL_PWM4] },
    // { &PWMTimerCC26XX_fxnTable, &pwmtimerCC26xxObjects[CC1310_LAUNCHXL_PWM5], &pwmtimerCC26xxHWAttrs[CC1310_LAUNCHXL_PWM5] },
    // { &PWMTimerCC26XX_fxnTable, &pwmtimerCC26xxObjects[CC1310_LAUNCHXL_PWM6], &pwmtimerCC26xxHWAttrs[CC1310_LAUNCHXL_PWM6] },
    // { &PWMTimerCC26XX_fxnTable, &pwmtimerCC26xxObjects[CC1310_LAUNCHXL_PWM7], &pwmtimerCC26xxHWAttrs[CC1310_LAUNCHXL_PWM7] },
};

const uint_least8_t PWM_count = CC1310_LAUNCHXL_PWMCOUNT;


//***********************************************************************************
// brief:Init the pwm function, this function should be call before other pwm api
// 
// parameter: 
//***********************************************************************************
void PwmDriverInit(void)
{
    PWM_init();
}
