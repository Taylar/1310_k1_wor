#include "../general.h"


// **************************************************************************
// variable
// rtc timer
static Clock_Struct rtcSecondsClock;     /* not static so you can see in ROV */

static Clock_Handle rtcSecondsClockHandle;


// semaphore 
static Semaphore_Struct rtcSemStruct;
static Semaphore_Handle rtcSemHandle;


// rtc
static Calendar rtc;

static void (*RtcSecIsbCB)(void);

// *******************************************************************************
// *Funtion name: MonthMaxDay
// *Description :
// *
// *Input: the month and year
// *Output:the last day of the month
// /******************************************************************************
uint8_t MonthMaxDay(uint16_t year_1, uint8_t month_1)
{
    uint8_t maxday;

    // leap year judgement
    if((year_1%4) == 0)
    {
        if((year_1 % 100) != 0)
            maxday = 29;
        else
        {
            if((year_1%400) == 0)
                maxday  = 29;
            else
                maxday  = 28;
        }
    }
    else
    {
        maxday = 28;
    }
    
    switch(month_1)
    {
        case 1:
            return(31);
        case 2:
            return(maxday);
        case 3:
            return(31);
        case 4:
            return (30);
        case 5:
            return(31);
        case 6:
            return(30);
        case 7:
            return(31);
        case 8:
            return (31);
        case 9:
            return(30);
        case 10:
            return(31);
        case 11:
            return(30);
        case 12:
            return (31);
        default:
            break;
    }
        return 0xff;
}


void RtcSecondsIsrCb(UArg arg0)
{
    // Semaphore_pend(rtcSemHandle, BIOS_WAIT_FOREVER);

    rtc.sec++;
    if(rtc.sec >= 60){
        rtc.sec = 0;
        rtc.min++;
        if(rtc.min >= 60){
            rtc.min = 0;
            rtc.hour++;
            if(rtc.hour >= 24){
                rtc.hour = 0;
                rtc.day++;
                rtc.week++;
                if(rtc.week > 6)
                    rtc.week = 0;

                if(rtc.day > MonthMaxDay(rtc.year, rtc.month)){
                    rtc.day = 1;
                    rtc.month++;
                    if(rtc.month >12){
                        rtc.month = 1;
                        rtc.year ++;
                    }
                }
            }
        }
    }
    // Semaphore_post(rtcSemHandle);

    RtcSecIsbCB();
}




//***********************************************************************************
//
// RTC init Calendar Mode.
//
//***********************************************************************************
void RtcInit(void (*Cb)(void))
{

    /* Create clock object which is used for fast report timeout */
    Clock_Params clkParams;
    clkParams.period = CLOCK_UNIT_S;
    clkParams.startFlag = FALSE;
    Clock_construct(&rtcSecondsClock, RtcSecondsIsrCb, 1, &clkParams);
    rtcSecondsClockHandle = Clock_handle(&rtcSecondsClock);
    

    rtc.year  = 2017;
    rtc.month = 12;
    rtc.day   = 30;
    rtc.week  = 6;
    rtc.hour  = 0;
    rtc.min   = 0;
    rtc.sec   = 0;


    RtcSecIsbCB = Cb;
}

//***********************************************************************************
//
// RTC start.
//
//***********************************************************************************
void RtcStart(void)
{
    if(Clock_isActive(rtcSecondsClockHandle) == false)
        Clock_start(rtcSecondsClockHandle);
}

//***********************************************************************************
//
// RTC stop.
//
//***********************************************************************************
void RtcStop(void)
{
    if(Clock_isActive(rtcSecondsClockHandle))
        Clock_stop(rtcSecondsClockHandle);
}




//***********************************************************************************
//
// RTC set Calendar time.
//
//***********************************************************************************
void Rtc_set_calendar(Calendar *currentTime)
{
    UInt key;

    key = Hwi_disable();
    memcpy(&rtc, currentTime, sizeof(Calendar));
    Hwi_restore(key);
}

//***********************************************************************************
//
// RTC get Calendar time.
//
//***********************************************************************************
Calendar Rtc_get_calendar(void)
{
    return rtc;
}
