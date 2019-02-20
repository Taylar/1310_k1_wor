#ifndef			__RTC_DRV_H__
#define			__RTC_DRV_H__


#define CALENDAR_BASE_YEAR              2000


typedef struct Calendar
{
    //! Seconds of minute between 0-59
    uint8_t Seconds;
    //! Minutes of hour between 0-59
    uint8_t Minutes;
    //! Hour of day between 0-23
    uint8_t Hours;
    //! Day of week between 0-6
    uint8_t DayOfWeek;
    //! Day of month between 1-31
    uint8_t DayOfMonth;
    //! Month between 0-11
    uint8_t Month;
    //! Year between 0-4095
    uint16_t Year;
} Calendar;



void RtcInit(void (*Cb)(void));

void RtcStart(void);

void RtcStop(void);

void Rtc_set_calendar(Calendar *currentTime);

Calendar Rtc_get_calendar(void);

uint8_t RtcGetSec(void);

uint8_t MonthMaxDay(uint16_t year_1, uint8_t month_1);

void RtcAdjust(Calendar *calendar);
#endif    // __RTC_DRV_H__


