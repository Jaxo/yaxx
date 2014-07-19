#include <string.h>
#include <fstream>
#include "../toolslib/SystemContext.h"
#include "../toolslib/StdFileStream.h"
#include "../rexxlib/Rexx.h"

#ifndef YAXX_NAMESPACE
#define YAXX_NAMESPACE
#endif

#ifndef TOOLS_NAMESPACE
#define TOOLS_NAMESPACE
#endif

#ifndef COM_JAXO_YAXX_DENY_XML
#error Bare REXX must deny XML extension
#endif

/*----------------------------------------------------------------------usage-+
|                                                                             |
+----------------------------------------------------------------------------*/
static void usage() {
   cout << "Usage: \trexx rexxFile\n" << endl;
}

/*-----------------------------------------------------------------------main-+
|                                                                             |
+----------------------------------------------------------------------------*/
int main(int argc, char ** argv)
{
   YAXX_NAMESPACE::Rexx rexx;

   // -- need a SystemContext, red priority!
   char * fileBaseUri = TOOLS_NAMESPACE::StdFileStreamBuf::makeBaseUri();
   TOOLS_NAMESPACE::SystemContext context(
      fileBaseUri,
      TOOLS_NAMESPACE::StdFileSchemeHandler()
   );
   free(fileBaseUri);

   if (argc != 2) {
      usage();
      return -1;
   }else {
      YAXX_NAMESPACE::Rexx::Script script(argv[1]); // create the script
      return rexx.interpret(script);
   }
}

/*===========================================================================*/
