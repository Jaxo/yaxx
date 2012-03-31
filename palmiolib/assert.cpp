/*
* $Id: assert.cpp,v 1.9 2002-08-06 10:14:04 pgr Exp $
*/

/*---------+
| Includes |
+---------*/
#include <stdlib.h>
#include "assert.h"
#include "MemStream.h"

/*-----------------------------------------------------__msl_assertion_failed-+
|                                                                             |
+----------------------------------------------------------------------------*/
void __msl_assertion_failed(char const * condition, char const * filename, char const * funcname, int lineno);
void __msl_assertion_failed(
   char const * condition,
   char const * filename,
   char const * funcname,
   int lineno
) {
   MemStreamBuf buf;
   ostream(&buf) << "Assertion (" << condition
      << ") failed in \"" << filename
      << "\", function \"" << funcname << "\", line " << lineno
      << endl << '\0';
   FrmCustomAlert(1000, buf.str(), 0, 0);
   buf.freeze(0);
   abort();
}


/*------------------------------------------------------------------showError-+
|                                                                             |
+----------------------------------------------------------------------------*/
void showError(int err, char const * filename, int lineno)
{
   UInt32 romVersion;
   MemStreamBuf buf;
   ostream os(&buf);
   FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);

   os << "In \"" << filename << "\", line " << lineno << ":\n";
   if (romVersion >= 0x03203000) {
      os << '\0';
      ErrAlertCustom(err, 0, buf.str(), 0);
   }else {
      os << "System error: " << hex << err << endl << '\0';
      FrmCustomAlert(1000, buf.str(), 0, 0);
   }
   buf.freeze(0);
}

/*===========================================================================*/
