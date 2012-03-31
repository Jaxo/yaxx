/* $Id: DayMonth.h,v 1.1 2002-05-02 07:55:20 pgr Exp $ */

#ifndef DAYMONTH_HEADER
#define DAYMONTH_HEADER

/*---------+
| Includes |
+---------*/
#include "resgendef.h"

RESOURCES_API char const * getDayName(int day);      // 0 is Sunday!
RESOURCES_API char const * getMonthName(int month);  // 0 is January

#endif /* DAYMONTH_HEADER ===================================================*/
