/* $Id: DayMonth.cpp,v 1.1 2002-05-02 07:55:20 pgr Exp $ */

#include "DayMonth.h"

#if defined __MWERKS__   // MWERKS has its own day and month names resources
RESOURCES_API char const * getDayNameRsc(int day);
RESOURCES_API char const * getMonthNameRsc(int month);

#else
static char const * dayTable[] = {
   "Sunday",
   "Monday",
   "Tuesday",
   "Wednesday",
   "Thursday",
   "Friday",
   "Saturday"
};
static char const * monthTable[] = {
   "January",
   "February",
   "March",
   "April",
   "May",
   "June",
   "July",
   "August",
   "September",
   "October",
   "November",
   "December"
};
#endif

/*-----------------------------------------------------------------getDayName-+
|                                                                             |
+----------------------------------------------------------------------------*/
char const * getDayName(int day) {
#if defined __MWERKS__
   return getDayNameRsc(day);
#else
   return dayTable[day];
#endif
}

/*---------------------------------------------------------------getMonthName-+
|                                                                             |
+----------------------------------------------------------------------------*/
char const * getMonthName(int month) {
#if defined __MWERKS__
   return getMonthNameRsc(month);
#else
   return monthTable[month];
#endif
}

/*===========================================================================*/

