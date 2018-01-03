#ifndef			__RTC_DRV_H__
#define			__RTC_DRV_H__


#define CALENDAR_BASE_YEAR              2000


typedef struct Calendar
{
    //! Seconds of minute between 0-59
    uint8_t sec;
    //! Minutes of hour between 0-59
    uint8_t min;
    //! Hour of day between 0-23
    uint8_t hour;
    //! Day of week between 0-6
    uint8_t week;
    //! Day of month between 1-31
    uint8_t day;
    //! Month between 0-11
    uint8_t month;
    //! Year between 0-4095
    uint16_t year;
} Calendar;


void RtcInit(void (*Cb)(void));

void RtcStart(void);

void RtcStop(void);

void Rtc_set_calendar(Calendar *currentTime);

Calendar Rtc_get_calendar(void);

#endif    // __RTC_DRV_H__


