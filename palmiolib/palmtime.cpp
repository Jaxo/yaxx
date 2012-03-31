/*
* $Id: palmtime.cpp,v 1.5 2002-07-27 16:43:46 pgr Exp $
*/

/*---------+
| Includes |
+---------*/
#include "palmtime.h"

clock_t clock(void) SYS_TRAP(sysTrapTimGetTicks);
static UInt32 PrvSecondsOffset = 0;
static struct tm * palmTimeToAnsi(UInt32 palmTime, struct tm *ansi_time);

/*-----------------------------------------------------------------------time-+
|                                                                             |
+----------------------------------------------------------------------------*/
time_t time(time_t * t)
{
   if (!t) {
      return (time_t)TimGetSeconds() - PALM_TIME_POSIX_EPOCH;
   }
   *t = (time_t)TimGetSeconds() - PALM_TIME_POSIX_EPOCH;
   return *t;
}

/*-------------------------------------------------------------------difftime-+
|                                                                             |
+----------------------------------------------------------------------------*/
time_t difftime(time_t t1, time_t t0) {
   return t1 - t0;
}

/*-------------------------------------------------------------------gmtime_r-+
|                                                                             |
+----------------------------------------------------------------------------*/
struct tm * gmtime_r(const time_t *tim, struct tm *ansi_time)
{
   UInt32 romVersion;

   FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);
   if (romVersion >= sysMakeROMVersion(4, 0, 0, sysROMStageRelease, 0)) {
      return palmTimeToAnsi(
         TimTimeZoneToUTC(
            PALM_TIME_POSIX_EPOCH + *tim,
            PrefGetPreference(prefTimeZone),
            PrefGetPreference(prefDaylightSavingAdjustment)
         ),
         ansi_time
      );
   }else {  // sigh.  no tz, no daylight saving.  assume he is british.
      return palmTimeToAnsi(PALM_TIME_POSIX_EPOCH + *tim, ansi_time);
   }
}

/*----------------------------------------------------------------localtime_r-+
|                                                                             |
+----------------------------------------------------------------------------*/
struct tm * localtime_r(const time_t *tim, struct tm *ansi_time)
{
   return palmTimeToAnsi(PALM_TIME_POSIX_EPOCH + *tim, ansi_time);
}

/*-------------------------------------------------------------palmTimeToAnsi-+
|                                                                             |
+----------------------------------------------------------------------------*/
struct tm * palmTimeToAnsi(UInt32 palmTime, struct tm *ansi_time)
{
   DateTimeType palmos_time;
   TimSecondsToDateTime(palmTime, &palmos_time);

   int currentMonth = palmos_time.month;
   int yday =  palmos_time.day - 1;
   for (int month=1; month < currentMonth; ++month) {
      yday += DaysInMonth(month, palmos_time.year);
   }
   ansi_time->tm_sec = palmos_time.second;
   ansi_time->tm_min  = palmos_time.minute;
   ansi_time->tm_hour = palmos_time.hour;
   ansi_time->tm_mday = palmos_time.day;
   ansi_time->tm_mon  = palmos_time.month - 1;
   ansi_time->tm_year = palmos_time.year - 1900;
   ansi_time->tm_wday = palmos_time.weekDay;
   ansi_time->tm_yday = yday;
   ansi_time->tm_isdst = 0;
   return ansi_time;
}

/*---------------------------------------------------------------------mktime-+
|                                                                             |
+----------------------------------------------------------------------------*/
time_t mktime(const struct tm *timPtr)
{
   DateTimeType palmos_time;

   palmos_time.second = timPtr->tm_sec;
   palmos_time.minute = timPtr->tm_min;
   palmos_time.hour = timPtr->tm_hour;
   palmos_time.day = timPtr->tm_mday;
   palmos_time.month = timPtr->tm_mon + 1;
   palmos_time.year = timPtr->tm_year + 1900;
   palmos_time.weekDay = timPtr->tm_wday;
   return TimDateTimeToSeconds( &palmos_time ) - PALM_TIME_POSIX_EPOCH;
}


/*---------------------------------------------------------------gettimeofday-+
|                                                                             |
+----------------------------------------------------------------------------*/
Int16 gettimeofday(struct timeval* tP, struct timezone* tzP)
{
   UInt32 ticks, secondsOn, subTicksOn, estSeconds;
   UInt32 actSeconds;
   Int32 dSecs;

   if (tzP) {             // If tzP is non-nil, get the time zone info
      tzP->tz_minuteswest = PrefGetPreference(prefMinutesWestOfGMT);
      tzP->tz_dsttime = DST_NONE;
   }
   if (!tP) return 0;     // If no tP, do nothing

   /*
   | Get the current value from our real-time clock and our current
   | tick count.
   */
   actSeconds = TimGetSeconds();
   ticks = TimGetTicks();

   /*
   | We use ticks to estimate the time since we desire microseconds
   | granularity, not 1 second granularity like our real-time clock
   | provides.
   */
   secondsOn = ticks / sysTicksPerSecond;
   subTicksOn = ticks % sysTicksPerSecond;

   /*
   | If we have a calculated offset (this is not the first invocation),
   | add that to our current seconds calculation to get the seconds
   | since 1970.
   */
   estSeconds = secondsOn + PrvSecondsOffset;

   /*
   | Now, make sure we're in the ballpark by comparing our tick time
   | with our real-time clock time.
   */
   dSecs = actSeconds - estSeconds;
   if (dSecs < 0) dSecs = -dSecs;
   if (dSecs > 10) {       // If way off, re-calculate PrvSecondsOffset;
      PrvSecondsOffset = actSeconds - secondsOn;
      estSeconds = actSeconds;
      subTicksOn = 0;
   }

   /*
   | Use subTicksOn to estimate microseconds and return the time
   | since 1970, not 1904 like our native code does.
   */
   tP->tv_sec = estSeconds - offset1970;
   tP->tv_usec = (subTicksOn * 1000000L) / sysTicksPerSecond;

   return 0;
}

/*---------------------------------------------------------------settimeofday-+
|                                                                             |
+----------------------------------------------------------------------------*/
Int16 settimeofday(struct timeval* tP, struct timezone* tzP)
{
   if (!tP) return 0;           // If no tP, do nothing
   if (tzP) {                   // if timezone info, set that
      PrefSetPreference(prefMinutesWestOfGMT, tzP->tz_minuteswest);
   }
   TimSetSeconds(tP->tv_sec + offset1970);   // Set the time
   return 0;
}

/*===========================================================================*/
