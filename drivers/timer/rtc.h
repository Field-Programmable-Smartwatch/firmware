#ifndef RTC_H
#define RTC_H

#include <stdint.h>
#include <time.h>

void rtc_set_date_and_time(datetime_t *datetime);
void rtc_get_date_and_time(datetime_t *datetime);
void rtc_init();

#endif
