/* $Id: testTime.cpp,v 1.1 2002/09/15 07:46:30 pgr Exp $ */

#include <string.h>
#include "miscutil.h"
#include "palmtime.h"
#include "testsRsc.h"

/*-------------------------------------------------------------------testTime-+
|                                                                             |
+----------------------------------------------------------------------------*/
void testTime();
void testTime()
{
   char msg[100];
   char *cp;
   time_t t1 = time(0);

   FrmCustomAlert(
      MessageBoxInfoAlert, "About to start \"Time\" test.", "", ""
   );

   strcpy(msg, "The time is ");
   cp = msg + strlen(msg);
   strcpy(cp + ltols(t1, cp),  " secs since 1970.");
   FrmCustomAlert(MessageBoxInfoAlert, msg, "", "");

   strcpy(msg, "The time is now ");
   cp = msg + strlen(msg);
   strcpy(
      cp + ltols(time(0)-t1, cp),
      " secs since the last time we checked the time."
   );
   FrmCustomAlert(MessageBoxInfoAlert, msg, "", "");
   return;
}

/*===========================================================================*/
