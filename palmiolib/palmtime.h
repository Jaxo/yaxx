/*
* $Id: palmtime.h,v 1.4 2002-07-27 16:43:46 pgr Exp $
*/
#ifndef COM_JAXO_PALMIO_PALMTIME_H_INCLUDED
#define COM_JAXO_PALMIO_PALMTIME_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include <sys_time.h>

#define PALM_TIME_POSIX_EPOCH 2082844800UL
#define offset1970         PALM_TIME_POSIX_EPOCH

typedef unsigned long time_t;
typedef unsigned long clock_t;

struct  tm {
  int tm_sec;   /* seconds after the minute -- [0,61] */
  int tm_min;   /* minutes after the hour   -- [0,59] */
  int tm_hour;  /* hours after midnight     -- [0,23] */
  int tm_mday;  /* day of the month         -- [1,31] */
  int tm_mon;   /* months since January     -- [0,11] */

  int tm_year;  /* years since 1900                   */
  int tm_wday;  /* days since Sunday        -- [0,6]  */
  int tm_yday;  /* days since January 1     -- [0,365]*/
  int tm_isdst; /* Daylight Savings Time flag */
};

extern time_t time(time_t* pt);
extern time_t difftime(time_t t1, time_t t0);
extern struct tm *gmtime_r(const time_t *tim,struct tm *ansi_time);
extern struct tm *localtime_r(const time_t *tim,struct tm *ansi_time);
extern time_t mktime(const struct tm *tmPtr);

extern Int16 gettimeofday(struct timeval* tP, struct timezone* tzP);
extern Int16 settimeofday(struct timeval* tP, struct timezone* tzP);

#endif
/*===========================================================================*/
