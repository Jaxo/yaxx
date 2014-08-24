#include <string.h>
#include <fstream>
#include "../toolslib/StdFileStream.h"
#include "../toolslib/StringBuffer.h"
#include "../toolslib/SystemContext.h"
#include "../rexxlib/Exception.h"
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

/*-----------------------------------------------------------------------main-+
|                                                                             |
+----------------------------------------------------------------------------*/
int main(int argc, char ** argv)
{

   if ((argc < 2) || (argv[1][0] == '?')) {
      puts("\nsyntax: rexx rexxfilepath [args]");
      return 0;
   }else {
      YAXX_NAMESPACE::Rexx rexx;
      // -- need a SystemContext (singleton), red priority!
      char * fileBaseUri = TOOLS_NAMESPACE::StdFileStreamBuf::makeBaseUri();
      TOOLS_NAMESPACE::SystemContext context(
         fileBaseUri,
         TOOLS_NAMESPACE::StdFileSchemeHandler()
      );
      free(fileBaseUri);

      try {
         // create the script
         YAXX_NAMESPACE::Rexx::Script script(argv[1]);

         // reassemble arguments for REXX if any
         if (argc > 2) {
            StringBuffer arg;
            int i = 2;
            for (;;) {
               arg.append(argv[i], strlen(argv[i]));
               if (++i >= argc) break;
               arg.append(' ');
            }
            // run the argued script
            return rexx.interpret(script, arg);
         }else {
            // run the bare script
            return rexx.interpret(script);
         }
      }catch (FatalException & e) {
         // the exception was already printed
         return e.m_codeNo;
      }catch (...) {
         // puts("Unhandled exception\n"); ???
         return 1000;
      }
   }
}

/*===========================================================================*/
