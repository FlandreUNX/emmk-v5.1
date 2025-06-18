/*
 * Copyright (C) 2020 Flandreunx@outlook.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdint.h>
#include <time.h>

#include "./kldatetime.h"

/**
 * @addtogroup Private func
 * @note none
 */
 
/*@{*/

/*@}*/

/**
 * @addtogroup Public functions
 * @note none
 */
 
/*@{*/

//// Calculate weekday (from here: http://stackoverflow.com/a/21235587)
//      As reported also by Wikipedia, in 1990 Michael Keith 
//      and Tom Craver published an expression to minimise the number of keystrokes needed 
//      to enter a self-contained function for converting a Gregorian date into a numerical day of the week.
//      
//      The expression does preserve neither y nor d, 
//      and returns a zero-based index representing the day, starting with Sunday, i.e. 
//      if the day is Monday the expression returns 1.
//      
//      A code example which uses the expression follows:
// int d    = 15   ; //Day     1-31
// int m    = 5    ; //Month   1-12`
// int y    = 2013 ; //Year    2013` 
// int weekday  = klDateTime_getWeekday(y, m, d);
// @ https://stackoverflow.com/questions/28520421/c-comma-in-ternary-statement/28520469#28520469
inline uint32_t klDateTime_getWeekday(uint32_t y, uint32_t m, uint32_t d) {
    return (d += m < 3 ? y-- : y - 2, 23 * m / 9 + d + 4 + y / 4 - y / 100 + y / 400) % 7;// + 1;
}


const uint8_t monthDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

inline void klDateTime_bktime(uint32_t ts, klDateTime_SampleTm_t *tm) {
    uint8_t year;
    uint8_t month, monthLength;
    uint32_t time;
    unsigned long days;
    
    time = (uint32_t) ts;
    tm->sec = time % 60;
    time /= 60; // now it is minutes
    tm->min = time % 60;
    time /= 60; // now it is hours
    tm->hour = time % 24;
    time /= 24; // now it is days
    tm->wday = ((time + 4) % 7); // sunday = 0
    if (tm->wday == 0) {
        tm->wday = 7; // adjust sunday = 7
    }
    
    
    year = 0;  
    days = 0;
    while((unsigned)(days += (LEAP_YEAR(year) ? 366 : 365)) <= time) {
        year++;
    }
    tm->year = year + 1970; // year is offset from 1970 
    
    days -= LEAP_YEAR(year) ? 366 : 365;
    time  -= days; // now it is days in this year, starting at 0
    
    days = 0;
    month = 0;
    monthLength = 0;
    for (month = 0; month < 12; month++) {
        if (month == 1) { // february
            if (LEAP_YEAR(year)) {
                monthLength = 29;
            } else {
                monthLength = 28;
            }
        } else {
            monthLength = monthDays[month];
        }

        if (time >= monthLength) {
            time -= monthLength;
        } else {
            break;
        }
    }
    tm->mon = month + 1;  // jan is month 1  
    tm->day = time + 1;     // day of month
}


/* Converts Gregorian date to seconds since 1970-01-01 00:00:00.
 * Assumes input in normal date format, i.e. 1980-12-31 23:59:59
 * => year=1980, mon=12, day=31, hour=23, min=59, sec=59.
 *
 * [For the Julian calendar (which was used in Russia before 1917,
 * Britain & colonies before 1752, anywhere else before 1582,
 * and is still in use by some communities) leave out the
 * -year/100+year/400 terms, and add 10.]
 *
 * This algorithm was first published by Gauss (I think).
 *
 * WARNING: this function will overflow on 2106-02-07 06:28:16 on
 * machines where long is 32-bit! (However, as time_t is signed, we
 * will already get problems at other places on 2038-01-19 03:14:08)
 */
//// As 
//// unsigned long result1=mktime(2015,12,6,13,20,10);
//// 
inline uint32_t klDateTime_mktime(klDateTime_SampleTm_t *tm) {
     unsigned int mon = tm->mon, year = tm->year;

    if (0 >= (int) (mon -= 2)) {
        mon += 12;  /* Puts Feb last since it has leap day */
        year -= 1;
    }

    return (((
             (unsigned long) (year / 4 - tm->year / 100 + tm->year / 400 + 367 * mon / 12 + tm->day) +
             (year * 365) - 719499
          ) * 24 + tm->hour
       ) * 60 + tm->min
    ) * 60 + tm->sec;
}


// Return 0, AM
// Return 1, PM
inline uint8_t klDateTime_24hTo12h(uint8_t h24, uint8_t m24, uint8_t *h12, uint8_t *m12) {
    if (h24 == 0 
            && m24 <= 59) {
        *h12 = 0;
        *m12 = m24;
        return 0;
    } else if (h24 >= 1 && h24 <= 11
            && m24 <= 59) {
        *h12 = h24;
        *m12 = m24;
        return 0;
    } else if (h24 >= 13 && h24 <= 23
            && m24 <= 59) {
        *h12 = h24 - 12;
        *m12 = m24;
        return 1;
    }
        
    return 0;
}


inline void klDateTime_12hTo24h(uint8_t am0pm1, uint8_t h12, uint8_t m12, uint8_t *h24, uint8_t *m24) {
    if (am0pm1 == 0
            && h12 == 12 
            && m12 <= 59) {
        *h24 = 0;
        *m24 = m12;
    } else if (h12 >= 1 && h12 <= 12
            && m12 <= 59) {
        *h24 = h12;
        *m24 = m12;
    } else if (am0pm1 == 1 
            && h12 >= 1 && h12 <= 11
            && m12 <= 59) {
        *h24 = h12 + 12;
        *m24 = m12;
    }
}

/*@}*/
