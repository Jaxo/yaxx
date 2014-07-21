/* $Id: TimeClock.cpp,v 1.16 2011-07-29 10:26:35 pgr Exp $ */

#include <string.h>

#ifdef __MWERKS__
#include "iostream.h"
#else
#include <iostream>
#include <iomanip>
#endif

#include "../reslib/DayMonth.h"
#include "TimeClock.h"
#include "../toolslib/miscutil.h"

#ifdef YAXX_NAMESPACE
namespace YAXX_NAMESPACE {
#endif

unsigned short const TimeClock::maxMonthDays[] = {
   31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};
unsigned short const TimeClock::sumMonthDays[] = {
   0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334
};

/*-----------------------------------------------StopWatch::formatElapsedTime-+
|                                                                             |
+----------------------------------------------------------------------------*/
void StopWatch::formatElapsedTime(
   StopWatch const & base,
   std::ostream & result
) const {
   long secs = base.m_secs - m_secs;
   long usecs = base.m_usecs - m_usecs;
   if (usecs < 0) {
      usecs += 1000000;
      --secs;
   }
   if (!usecs && !secs) {
      result << '0';
   }else {
      result
         << secs
         << '.'
         << std::setfill('0')
         << std::setw(6) << usecs
         << std::setfill(' ');
   }
}

/*STATIC-----------------------------------------------TimeClock::getBaseDays-+
| Format: 12 Aug 1997                                                         |
+----------------------------------------------------------------------------*/
int TimeClock::getBaseDays(char const * source, char sep)
{
   long d;
   int m;
   long y;
   char const * p = source;
   if (::stol(&source, &d) && ((sep == '\0') || (*source++ == sep))) {
      for (m=0; m < 12; ++m) {
         char const * month = ::getMonthName(m);
         if (
            (month[0] == source[0]) &&
            (month[1] == source[1]) &&
            (month[2] == source[2])
         ) {
            source += 3;
            if (sep) while ((*source) && (*source++ != sep)) {}
            if ((*source) && ::stol(&source, &y) && (*source == '\0')) {
               return getBaseDays(d, m, y);
            }
            break;
         }
      }
   }
   return -1;
}

/*STATIC--------------------------------------------TimeClock::getBaseDaysISO-+
| Format: 19701231                                                            |
+----------------------------------------------------------------------------*/
int TimeClock::getBaseDaysISO(char const * source, char sep)
{
   long d;
   long m;
   long y;
   char buf[9];
   char const * p;
   if (sep == '\0') {
      if (strlen(source) == 8) {
         strcpy(buf, source);
         p = source+6;
         if (
            (p=buf+6, ::stol(&p, &d) && (*p == '\0')) &&
            (buf[6]='\0', p=buf+4, ::stol(&p, &m) && (*p == '\0')) &&
            (buf[4]='\0', p=buf, ::stol(&p, &y) && (*p == '\0'))
         ) {
            return getBaseDays(d, m-1, y);
         }
      }
   }else {
      if (
         (strlen(source) <= 10) &&
         ::stol(&source, &y) && (*source++ == sep) &&
         ::stol(&source, &m) && (*source++ == sep) &&
         ::stol(&source, &d) && (*source == '\0')
      ) {
         return getBaseDays(d, m-1, y);
      }
   }
   return -1;
}

/*-----------------------------------------------------TimeClock::getBaseDays-+
| Format: xx/xx/xx  i0: what xx is the d, i1: the m, i2: the y                |
+----------------------------------------------------------------------------*/
int TimeClock::getBaseDays(
   char const * source, int i0, int i1, int i2, char sep
) {
   long dmy[3];
   char buf[9];
   if (sep == '\0') {
      if (strlen(source) != 6) return -1;
      buf[0] = source[0];
      buf[1] = source[1];
      buf[2] = '\0';
      buf[3] = source[2];
      buf[4] = source[3];
      buf[5] = '\0';
      buf[6] = source[4];
      buf[7] = source[5];
      buf[8] = '\0';
      source = buf;
   }
   if (
      (strlen(source) <= 8) &&
      ::stol(&source, dmy+i0) && (*source++ == sep) &&
      ::stol(&source, dmy+i1) && (*source++ == sep) &&
      ::stol(&source, dmy+i2) && (*source == '\0')
   ) {
      int const y = m_tm.tm_year + 1900 - 50;
      int yy = dmy[2];
      while (yy < y) yy += 100;
      return getBaseDays(dmy[0], dmy[1]-1, yy);
   }
   return -1;
}

/*-----------------------------------------------------TimeClock::getBaseDays-+
| dty is the nbr of days since the beginning of the year                      |
+----------------------------------------------------------------------------*/
int TimeClock::getBaseDays(int dty) const
{
   int y = m_tm.tm_year + 1900 - 1;
   return dty + (y*365) + (y/4) - (y/100) + (y/400);
}


/*STATIC-----------------------------------------------TimeClock::getBaseDays-+
|                                                                             |
+----------------------------------------------------------------------------*/
int TimeClock::getBaseDays(int d, int m, int y)
{
   if (
      (d <= 0) || (m < 0) || (y <= 0) ||
      (m >= 12) || (d > maxMonthDays[m]) || (y > 9999)
   ) {
      return -1;
   }else {
      /* Convert Month & Day to Days of year. */
      int dty = sumMonthDays[m] + d-1;
      if ((m > 1) && ((y%4) == 0) && (((y%100) != 0) || ((y%400) == 0))) {
         ++dty;
      }
      /* Convert to BaseDays */
      y -= 1;
      return dty + (y*365) + (y/4) - (y/100) + (y/400);
   }
}

/*STATIC-------------------------------------------TimeClock::getDaySecsCivil-+
| Format: 9:54am                                                              |
+----------------------------------------------------------------------------*/
int TimeClock::getDaySecsCivil(char const * source)
{
   long h;
   long m;
   if (
      (strlen(source) <= 7) &&
      ::stol(&source, &h) && (*source++ == ':') &&
      ::stol(&source, &m) &&
      ((*source == 'a') || (*source == 'p')) &&
      (*(source+1) == 'm') && (*(source+2) == '\0')
   ) {
      if (*source == 'p') h += 12;
      return getDaySecs(h, m, 0);
   }
   return -1;
}


/*STATIC------------------------------------------TimeClock::getDaySecsNormal-+
| Format: 09:54:35                                                            |
+----------------------------------------------------------------------------*/
int TimeClock::getDaySecsNormal(char const * source)
{

   long h;
   long m;
   long s;
   if (
      (strlen(source) == 8) &&
      ::stol(&source, &h) && (*source++ == ':') &&
      ::stol(&source, &m) && (*source++ == ':') &&
      ::stol(&source, &s) && (*source++ == ':') && (*source == '\0')
   ) {
      return getDaySecs(h, m, s);
   }
   return -1;
}

/*STATIC-------------------------------------------TimeClock::getDaySecsHours-+
| Format: 9  [0 - 23]                                                         |
+----------------------------------------------------------------------------*/
int TimeClock::getDaySecsHours(int value) {
   return getDaySecs(value, 0, 0);
}

/*STATIC--------------------------------------------TimeClock::getDaySecsMins-+
| Format: 594 (9*60 + 54)                                                     |
+----------------------------------------------------------------------------*/
int TimeClock::getDaySecsMins(int value) {
   return getDaySecs(value/60, value%60, 0);
}

/*STATIC--------------------------------------------TimeClock::getDaySecsSecs-+
| Format: 35675 ((9*60 + 54)*60 + 35)                                         |
+----------------------------------------------------------------------------*/
int TimeClock::getDaySecsSecs(int value) {
   int r = value % 3600;
   return getDaySecs(value/3600, r/60, r%60);
}

/*STATIC------------------------------------------------TimeClock::getDaySecs-+
|                                                                             |
+----------------------------------------------------------------------------*/
int TimeClock::getDaySecs(int h, int m, int s)
{
   if ((h < 0) || (m < 0) || (s < 0) || (h > 23) || (m > 59) || (s >59)) {
      return -1;
   }else {
      return (((h*60) + m) * 60) + s;
   }
}

/*STATIC----------------------------------------------TimeClock::getGmtAdjust-+
| Returns the number of hours between local and GMT                           |
+----------------------------------------------------------------------------*/
int TimeClock::getGmtAdjust()
{
   int adj;
   struct tm tmGmt;

   #if defined __MWERKS__
      int secs = ::time(0);
      ::gmtime_r((time_t *)&secs, &tmGmt);
   #elif defined _WIN32
      struct _timeb tb;
      _ftime(&tb);
      // hope it is reentrant!
      tmGmt = *(::gmtime((time_t *)&tb.time));
   #else
      struct timeval tv;
      struct timezone tz;
      gettimeofday(&tv, &tz);
      ::gmtime_r((time_t *)&tv.tv_sec, &tmGmt);
   #endif
   if (m_tm.tm_yday == tmGmt.tm_yday) {
      adj = 0;
   }else if (m_tm.tm_year == tmGmt.tm_year) {
      adj = (m_tm.tm_yday > tmGmt.tm_yday)? 24 : -24;
   }else {
      adj = (m_tm.tm_year > tmGmt.tm_year)? 24 : -24;
   }
   return m_tm.tm_hour - tmGmt.tm_hour + adj;
}

/*-------------------------------------------------TimeClock::formatCivilTime-+
|                                                                             |
+----------------------------------------------------------------------------*/
void TimeClock::formatCivilTime(std::ostream & result) const
{
   int hour = m_tm.tm_hour;
   char const * ampm = (hour > 11) ? "pm" : "am" ;
   hour = hour % 12;
   if (hour == 0) hour = 12;
   result
      << std::setfill('0')
      << hour << ':'
      << std::setw(2) << m_tm.tm_min
      << ampm
      << std::setfill(' ');
}

/*-----------------------------------------------------TimeClock::formatHours-+
|                                                                             |
+----------------------------------------------------------------------------*/
void TimeClock::formatHours(std::ostream & result) const {
   result << m_tm.tm_hour;
}

/*---------------------------------------------------TimeClock::formatMinutes-+
|                                                                             |
+----------------------------------------------------------------------------*/
void TimeClock::formatMinutes(std::ostream & result) const {
   result << ((m_tm.tm_hour * 60) + m_tm.tm_min);
}

/*---------------------------------------------------TimeClock::formatSeconds-+
|                                                                             |
+----------------------------------------------------------------------------*/
void TimeClock::formatSeconds(std::ostream & result) const
{
   result << ((((m_tm.tm_hour * 60) + m_tm.tm_min) * 60) + m_tm.tm_sec);
}

/*--------------------------------------------------TimeClock::formatLongTime-+
|                                                                             |
+----------------------------------------------------------------------------*/
void TimeClock::formatLongTime(std::ostream & result) const
{
   result
      << std::setfill('0')
      << std::setw(2) << m_tm.tm_hour << ':'
      << std::setw(2) << m_tm.tm_min << ':'
      << std::setw(2) << m_tm.tm_sec << '.'
      << std::setw(6) << m_usecs
      << std::setfill(' ');
}

/*------------------------------------------------TimeClock::formatNormalTime-+
|                                                                             |
+----------------------------------------------------------------------------*/
void TimeClock::formatNormalTime(std::ostream & result) const
{
   result
      << std::setfill('0')
      << std::setw(2) << m_tm.tm_hour << ':'
      << std::setw(2) << m_tm.tm_min << ':'
      << std::setw(2) << m_tm.tm_sec
      << std::setfill(' ');
}

/*--------------------------------------------------TimeClock::formatBaseDays-+
|                                                                             |
+----------------------------------------------------------------------------*/
void TimeClock::formatBaseDays(std::ostream & result) const
{
   int i = m_tm.tm_year + 1900;
   i = ((i-1)*365) + ((i-1)/4) - ((i-1)/100) + ((i-1)/400);
   result << (m_tm.tm_yday+i);
}

/*---------------------------------------------------TimeClock::formatCentury-+
|                                                                             |
+----------------------------------------------------------------------------*/
void TimeClock::formatCentury(std::ostream & result) const
{
   int i = m_tm.tm_year % 100;
   if (i > 0) {
      i = (i * 365) + ((i-1)/4);
      if (0 == (19 + (m_tm.tm_year / 100)) % 4) ++i;
   }
   result << (m_tm.tm_yday+i+1);
}

/*------------------------------------------------------TimeClock::formatDays-+
|                                                                             |
+----------------------------------------------------------------------------*/
void TimeClock::formatDays(std::ostream & result) const {
   result << m_tm.tm_yday+1;
}

/*----------------------------------------------TimeClock::formatEuropeanDate-+
|                                                                             |
+----------------------------------------------------------------------------*/
void TimeClock::formatEuropeanDate(std::ostream & result, char s) const
{
   char sep[2] = {'x','\0'};
   //char * sep = "x";  // On the Palm, we're using compiler flags that
                       // make doing this a no-no, causing it to core dump
                      // with "illegal access" on the next line.
   sep[0] = s;
   result
      << std::setfill('0')
      << std::setw(2) << m_tm.tm_mday << sep
      << std::setw(2) << (m_tm.tm_mon+1) << sep
      << std::setw(2) << ((m_tm.tm_year+1900)%100)
      << std::setfill(' ');
}

/*-------------------------------------------------TimeClock::formatMonthName-+
|                                                                             |
+----------------------------------------------------------------------------*/
void TimeClock::formatMonthName(std::ostream & result) const {
   result << ::getMonthName(m_tm.tm_mon);
}

/*------------------------------------------------TimeClock::formatNormalDate-+
|                                                                             |
+----------------------------------------------------------------------------*/
void TimeClock::formatNormalDate(std::ostream & result, char s) const {
   char sep[2] = {'x','\0'};
   sep[0] = s;
   result << std::setfill('0') << m_tm.tm_mday << sep;
   result.write(::getMonthName(m_tm.tm_mon), 3);
   result << sep << (m_tm.tm_year+1900) << std::setfill(' ');
}

/*-----------------------------------------------TimeClock::formatOrderedDate-+
|                                                                             |
+----------------------------------------------------------------------------*/
void TimeClock::formatOrderedDate(std::ostream & result, char s) const
{
   char sep[2] = {'x','\0'};
   sep[0] = s;
   result
      << std::setfill('0')
      << std::setw(2) << ((m_tm.tm_year+1900)%100) << sep
      << std::setw(2) << (m_tm.tm_mon+1) << sep
      << std::setw(2) << m_tm.tm_mday
      << std::setfill(' ');
}

/*----------------------------------------------TimeClock::formatStandardDate-+
|                                                                             |
+----------------------------------------------------------------------------*/
void TimeClock::formatStandardDate(std::ostream & result, char s) const
{
   char sep[2] = {'x','\0'};
   sep[0] = s;
   result
      << std::setfill('0')
      << std::setw(4) << (m_tm.tm_year+1900) << sep
      << std::setw(2) << (m_tm.tm_mon+1) << sep
      << std::setw(2) << m_tm.tm_mday
      << std::setfill(' ');
}

/*---------------------------------------------------TimeClock::formatUsaDate-+
|                                                                             |
+----------------------------------------------------------------------------*/
void TimeClock::formatUsaDate(std::ostream & result, char s) const
{
   char sep[2] = {'x','\0'};
   sep[0] = s;
   result
      << std::setfill('0')
      << std::setw(2) << (m_tm.tm_mon+1) << sep
      << std::setw(2) << m_tm.tm_mday << sep
      << std::setw(2) << ((m_tm.tm_year+1900)%100)
      << std::setfill(' ');
}

/*---------------------------------------------------TimeClock::formatDayName-+
|                                                                             |
+----------------------------------------------------------------------------*/
void TimeClock::formatDayName(std::ostream & result) const {
   result << ::getDayName(m_tm.tm_wday);
}

/*-----------------------------------------------TimeClockDays::TimeClockDays-+
|                                                                             |
+----------------------------------------------------------------------------*/
TimeClockDays::TimeClockDays(int baseDays)
{
   memset(&m_tm, 0, sizeof m_tm);

   /* At this point, the days are the days since the 0001 base date. */
   m_tm.tm_wday = (baseDays+1) % 7;
   ++baseDays;

   /*
   | Compute either the fitting year, or some year not too far earlier.
   | Compute the number of days left on the first of January of this year.
   */
   int y = baseDays / 366;
   baseDays = baseDays - ((y*365) + (y/4) - (y/100) + (y/400));
   ++y;

   /*
   | Now if the number of days left is larger than the number of days
   | in the year we computed, increment the year, and decrement the
   | number of days accordingly.
   */
   for (;;) {
      int n = (((y%4) == 0) && (((y%100) != 0) || ((y%400) == 0)))? 366 : 365;
      if (baseDays <= n) break;
      baseDays -= n;
      ++y;
   }

   /* At this point, the days left pertain to this year. */
   m_tm.tm_year = y - 1900;
   m_tm.tm_yday = baseDays - 1;

   /*
   | Now step through the months, increment the number of the month,
   | and decrement the number of days accordingly (taking into
   | consideration that in a leap year February has 29 days), until
   | further reducing the number of days and incrementing the month
   | would lead to a negative number of days
   */

   for (int m=0; m < 12; ++m) {
      int n = maxMonthDays[m];
      if ((m==1) && (((y%4) != 0) || (((y%100) == 0) && ((y%400) != 0)))) {
         --n;
      }
      if (baseDays <= n) {
         m_tm.tm_mon = m;
         m_tm.tm_mday = baseDays;
         break;
      }
      baseDays -= n;
   }
}

/*-----------------------------------------------TimeClockSecs::TimeClockSecs-+
|                                                                             |
+----------------------------------------------------------------------------*/
TimeClockSecs::TimeClockSecs(int secs)
{
   int r = secs % 3600;
   memset(&m_tm, 0, sizeof m_tm);

   /* secs are the seconds since the day raised in Greenwich. */
   m_tm.tm_hour = secs / 3600;
   m_tm.tm_min = r / 60;
   m_tm.tm_sec = r % 60;
}

/*-------------------------------------------------------------StopWatch::set-+
|                                                                             |
+----------------------------------------------------------------------------*/
void StopWatch::set()
{
   #if defined __MWERKS__
      m_secs = ::time(0);
      ::localtime_r((time_t *)&m_secs, &m_tm);
   #elif defined _WIN32
      struct _timeb tb;
      _ftime(&tb);
      m_secs = tb.time;
      m_usecs = tb.millitm * 1000;
      // hope it is reentrant!
      m_tm = *(::localtime((time_t *)&m_secs));
   #else
      struct timeval tv;
      struct timezone tz;
      gettimeofday(&tv, &tz);
      m_secs = tv.tv_sec;
      m_usecs = tv.tv_usec;
      ::localtime_r((time_t *)&m_secs, &m_tm);
   #endif
}

#ifdef YAXX_NAMESPACE
}
#endif
/*===========================================================================*/
