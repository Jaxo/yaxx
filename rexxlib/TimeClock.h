/* $Id: TimeClock.h,v 1.9 2011-07-29 10:26:35 pgr Exp $ */

#ifndef COM_JAXO_YAXX_TIMECLOCK_H_INCLUDED
#define COM_JAXO_YAXX_TIMECLOCK_H_INCLUDED

/*---------+
| Includes |
+---------*/
#if defined __MWERKS__
#include "palmtime.h"
#elif defined _WIN32
#include <time.h>
#include <sys/types.h>
#include <sys/timeb.h>
#else
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <iostream>
#endif

// class ostream;

#ifdef YAXX_NAMESPACE
namespace YAXX_NAMESPACE {
#endif

typedef struct tm StandardTimeStruct;

/*---------------------------------------------------------- class TimeClock -+
|                                                                             |
+----------------------------------------------------------------------------*/
class TimeClock {
public:
   static int getBaseDays(char const *, char);         // 12 Aug 1987
   static int getBaseDaysISO(char const *, char);      // 19870812
   int getBaseDays(char const *, int, int, int, char); // xx/xx/xx
   int getBaseDays(int dty) const;                     // dty = [0 .. 365]

   static int getDaySecsCivil(char const * source);    // 9:54am
   static int getDaySecsNormal(char const * source);   // 09:54:35
   static int getDaySecsHours(int value);              // 9
   static int getDaySecsMins(int value);               // 594 (9*60 + 54)
   static int getDaySecsSecs(int value);      // 35675 ((9*60 + 54)*60 + 35)
   static int getDaySecs(int h, int m, int s);

   int getGmtAdjust();                        // local = gmt + getGmtAdjust()

   void formatCivilTime(std::ostream & result) const;
   void formatHours(std::ostream & result) const;
   void formatMinutes(std::ostream & result) const;
   void formatSeconds(std::ostream & result) const;
   void formatLongTime(std::ostream & result) const;
   void formatNormalTime(std::ostream & result) const;
   void formatBaseDays(std::ostream & result) const;
   void formatCentury(std::ostream & result) const;
   void formatDays(std::ostream & result) const;
   void formatEuropeanDate(std::ostream & result, char sep) const;
   void formatMonthName(std::ostream & result) const;
   void formatNormalDate(std::ostream & result, char sep) const;
   void formatOrderedDate(std::ostream & result, char sep) const;
   void formatStandardDate(std::ostream & result, char sep) const;
   void formatUsaDate(std::ostream & result, char sep) const;
   void formatDayName(std::ostream & result) const;

protected:
   TimeClock();
   long m_usecs; // microseconds (u stands for the greek letter 'mu')
   struct tm m_tm;
   static unsigned short const maxMonthDays[];

private:
   static int getBaseDays(int d, int m, int y);
   static unsigned short const sumMonthDays[];
};

/*-- INLINES --*/
inline TimeClock::TimeClock() {
   m_usecs = 0;
}

/*------------------------------------------------------ class TimeClockDays -+
|                                                                             |
+----------------------------------------------------------------------------*/
class TimeClockDays : public TimeClock {
public:
   TimeClockDays(int baseDays);     // days since 1 Jan of Year 1
};

/*------------------------------------------------------ class TimeClockSecs -+
|                                                                             |
+----------------------------------------------------------------------------*/
class TimeClockSecs : public TimeClock {
public:
   TimeClockSecs(int secs);         // seconds the beginning of the day
};

/*------------------------------------------------------------------StopWatch-+
|                                                                             |
+----------------------------------------------------------------------------*/
class StopWatch : public TimeClock
{
public:
   StopWatch();
   void set();
   bool isSet() const;

   void formatElapsedTime(StopWatch const & base, std::ostream & result) const;
private:
   long m_secs;  // seconds elapsed since 00:00:00 on January 1, 1970
};

/*-- INLINES --*/
inline StopWatch::StopWatch() {
   m_secs = 0;
}

inline bool StopWatch::isSet() const {
   return (m_secs != 0);
}

#ifdef YAXX_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
