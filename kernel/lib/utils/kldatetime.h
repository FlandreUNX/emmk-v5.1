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

#ifndef _KLDATETIME_H_
#define _KLDATETIME_H_

#include <stdint.h>
#include <time.h>

/**
 * @addtogroup Define
 * @note none
 */
 
/*@{*/

#define SECS_PER_MIN        ((time_t) (60UL))
#define SECS_PER_HOUR       (60 * 60)
#define SECS_PER_DAY        (SECS_PER_HOUR * 24)
#define DAYS_PER_WEEK       ((time_t) (7UL))
#define SECS_PER_WEEK       ((time_t) (SECS_PER_DAY * DAYS_PER_WEEK))
#define SECS_PER_YEAR       ((time_t) (SECS_PER_DAY * 365UL)) // TODO: ought to handle leap years
#define SECS_YR_2000        ((time_t) (946684800UL)) // the time at the start of y2k
 
/* Useful Macros for getting elapsed time */
#define NUMBER_OF_SECONDS(_time_) ((_time_) % SECS_PER_MIN)

#define NUMBER_OF_MINUTES(_time_) (((_time_) / SECS_PER_MIN) % SECS_PER_MIN) 
#define HUMBER_OF_HOURS(_time_) (((_time_) % SECS_PER_DAY) / SECS_PER_HOUR)
#define DAY_OF_WEEK_SUN0(_time_) ((((_time_) / SECS_PER_DAY + 4)  % DAYS_PER_WEEK)) // Sunday = 0
#define ELAPSED_DAYS(_time_) ((_time_) / SECS_PER_DAY)  // this is number of days since Jan 1 1970
#define ELAPSED_SECS_TO_DAY(_time_) ((_time_) % SECS_PER_DAY)   // the number of seconds since last midnight 
// The following macros are used in calculating alarms and assume the clock is set to a date later than Jan 1 1971
// Always set the correct time before setting alarms
#define PREVIOUS_MIDNIGHT(_time_) (((_time_) / SECS_PER_DAY) * SECS_PER_DAY)  // time at the start of the given day
#define NEXT_MIDNIGHT(_time_) (PREVIOUS_MIDNIGHT(_time_)  + SECS_PER_DAY)   // time at the end of the given day 
#define ELAPSED_SECS_THIS_WEEK(_time_) (ELAPSED_SECS_TO_DAY(_time_) +  ((DAY_OF_WEEK_SUN0(_time_)) * SECS_PER_DAY))   // note that week starts on day 1
#define PREVIOUS_SUNDAY(_time_) ((_time_) - ELAPSED_SECS_THIS_WEEK(_time_))      // time at the start of the week for the given time
#define NEXT_SUNDAY(_time_) (PREVIOUS_SUNDAY(_time_)+SECS_PER_WEEK)          // time at the end of the week for the given time

/*@}*/

/**
 * @addtogroup Typedef
 * @note none
 */
 
/*@{*/

typedef struct {
    uint8_t sec;  // 0-59
    uint8_t min;  // 0-59
    uint8_t hour; // 0-23
    uint8_t wday; // 1-7, monday=1, shunday = 7
    uint8_t day;  // 1-31
    uint8_t mon;  // 1-12
    uint16_t year; // offset = 0, FORMAT=YYYY
} klDateTime_SampleTm_t;

/*@}*/

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

extern uint32_t klDateTime_getWeekday(uint32_t y, uint32_t m, uint32_t d);

#define LEAP_YEAR(Y) \
    (((1970 + (Y)) > 0) && !((1970 + (Y)) % 4) && ( ((1970 + (Y)) % 100) || !((1970 + (Y)) % 400)))
extern const uint8_t monthDays[];

extern void klDateTime_bktime(uint32_t ts, klDateTime_SampleTm_t *tm);

extern uint32_t klDateTime_mktime(klDateTime_SampleTm_t *tm);

extern uint8_t klDateTime_24hTo12h(uint8_t h24, uint8_t m24, uint8_t *h12, uint8_t *m12);
extern void klDateTime_12hTo24h(uint8_t am0pm1, uint8_t h12, uint8_t m12, uint8_t *h24, uint8_t *m24);

/*@}*/

#endif
