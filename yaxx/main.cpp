/* $Id: main.cpp,v 1.24 2002-09-11 09:12:58 pgr Exp $ */

/* Ex:
-r D:\u\newdev\yaxx\tests\data\XMLin10points.html D:\u\newdev\yaxx\yaxx\identity.r
*/


#include <string.h>
#include <fstream>
#include "../toolslib/SystemContext.h"
#include "../toolslib/MemStream.h"
#include "../toolslib/StringBuffer.h"
#include "../toolslib/StdFileStream.h"
#include "../toolslib/ConsoleSchemeHandler.h"
#include "../rexxlib/Rexx.h"

#ifndef YAXX_NAMESPACE
#define YAXX_NAMESPACE
#endif

#ifndef TOOLS_NAMESPACE
#define TOOLS_NAMESPACE
#endif

/*----------------------------------------------------------------------usage-+
|                                                                             |
+----------------------------------------------------------------------------*/
static void usage() {
   cout <<
     "For native Rexx interpreter:\n"
     "\tyaxx (rexxFile | '-') [args]\n"
     "or, for RXF mode:\n"
     "\tyaxx -r xmlInputFile rexxTransformFile [args]\n"
     << endl;
}

/*--------------------------------------------------------reassembleArguments-+
| Reassemble arguments form the command line as one string.                   |
+----------------------------------------------------------------------------*/
void reassembleArguments(
   char ** argv,
   int iStart,
   int iEnd,
   StringBuffer & arg
) {
   if (iStart < iEnd) {
      for (;;) {
         arg.append(argv[iStart], strlen(argv[iStart]));
         if (++iStart >= iEnd) break;
         arg.append(' ');
      }
   }
   arg.append('\0');  // aka the infamous sentinel.
}

/*-----------------------------------------------------------------------main-+
|                                                                             |
+----------------------------------------------------------------------------*/
int main(int argc, char ** argv)
{

// MemStream err;        // so cerr won't be mixed with cout
   YAXX_NAMESPACE::Rexx rexx;
   char const * xmlFilePath;
   char const * rexxFilePath = 0;
   StringBuffer arg;
   int rc;

// #ifdef ISO_14882
//    cerr.rdbuf(err.rdbuf());
// #else
//    cerr = err;
// #endif

   // -- need a SystemContext, red priority!
   char * fileBaseUri = TOOLS_NAMESPACE::StdFileStreamBuf::makeBaseUri();
   TOOLS_NAMESPACE::SystemContext context(
      fileBaseUri, TOOLS_NAMESPACE::StdFileSchemeHandler()
   );
   free(fileBaseUri);


   // --- Process the command line arguments
   if ((argc < 2) || (argv[1][0] == '?')) {
      usage();
      return -1;
   }
   if (argv[1][0] == '-') {
      switch (argv[1][1]) {
      case '\0':
         xmlFilePath = 0;
         rexxFilePath = 0;
         reassembleArguments(argv, 2, argc, arg);
         break;
      case 'r': case 'R':
         if (argc < 4) {
            usage();
            return -1;
         }
         xmlFilePath = argv[2];
         rexxFilePath = argv[3];
         reassembleArguments(argv, 4, argc, arg);
         break;
      default:
         usage();
         return -1;
      }
   }else {
      xmlFilePath = 0;
      rexxFilePath = argv[1];
      reassembleArguments(argv, 2, argc, arg);
   }

   // --- Create the Rexx Script
   YAXX_NAMESPACE::Rexx::Script script(rexxFilePath);

   if (!xmlFilePath) {
      /*
      | Example of a simple Rexx interpret
      */
      rc = rexx.interpret(script, arg);
   }else {
      /*
      | Example of an IRexx transform
      */
      istream * pInput = TOOLS_NAMESPACE::SystemContext::makeStream(
         xmlFilePath,
         ios::in
      );

      if (!pInput->good()) {
         cout << "XML file \"" << xmlFilePath << " not found." << endl;
         usage();
         rc = -1;
      }else {
         YAXX_NAMESPACE::RexxString result;
         rc = rexx.interpret(script, "", result, *pInput, cout);
         cout.flush();

//       err.seekg(0);
//       cout << endl << "---- ERRORS ----" << endl << err.rdbuf();
//       cout.flush();
      }
      delete pInput;
   }
   return rc;
}

/*===========================================================================*/
