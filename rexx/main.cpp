/* $Id: main.cpp,v 1.3 2002-01-26 03:52:21 jlatone Exp $ */

#include <string.h>
#include "../toolslib/StringBuffer.h"
#include "../rexxlib/Rexx.h"

/*-----------------------------------------------------------------------main-+
|                                                                             |
+----------------------------------------------------------------------------*/
int main(int argc, char ** argv)
{
   StringBuffer strArgs;
   int rc;

   if ((argc < 2) || (argv[1][0] == '?')) {
      puts("\nsyntax: rexx_test (filename | '-') [args]");
      return 0;
   }

   if (argc > 2) {
      int i = 2;
      for (;;) {
         strArgs.append(argv[i], strlen(argv[i]));
         if (++i >= argc) break;
         strArgs.append(' ');
      }
   }
   if ((argv[1][0] == '-') && (argv[1][1] == 0)) {
      rc = YAXX_NAMESPACE::run(argv[0], 0, strArgs);
   }else {
      rc = YAXX_NAMESPACE::run(argv[0], argv[1], strArgs);
   }
   return rc;
}

/*===========================================================================*/
