/* $Id: testMemStream.cpp,v 1.1 2002-05-25 10:51:06 pgr Exp $ */

// simple memstream test

#include "../toolslib/MemStream.h"
#include <iomanip.h>

/*--------------------------------------------------------------testMemStream-+
|                                                                             |
+----------------------------------------------------------------------------*/
int testMemStream()
{
   int rc = 987654321;
   int const maxOutputSize = 7;
   cout << "--------------- testMemStream" << endl;
   MemStream rcAsString;
   rcAsString << "rc=" << rc;
   if (rcAsString.rdbuf()->in_avail() > maxOutputSize) {
      rcAsString.seekp(maxOutputSize);
      rcAsString << "...";
   }
   rcAsString << endl;
   cout << rcAsString.rdbuf();
   return 0;
}

/*===========================================================================*/

