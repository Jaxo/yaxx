/* $Id: Rexx.h,v 1.103 2011-07-29 10:26:35 pgr Exp $ */

#ifndef COM_JAXO_YAXX_REXX_H_INCLUDED
#define COM_JAXO_YAXX_REXX_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include "rexxgendef.h"

#if !defined COM_JAXO_YAXX_DENY_XML
#include "../yasp3lib/parser/CachedDtd.h"
#endif

#include "Clauses.h"
#include "Label.h"
#include "Interpreter.h"

// class streambuf;

#ifdef TOOLS_NAMESPACE
namespace TOOL_NAMESPACE {
#endif
class FileStreamBuf;
#ifdef TOOLS_NAMESPACE
}
#endif

#ifdef YAXX_NAMESPACE
namespace YAXX_NAMESPACE {
#endif

/*--------------------------------------------------------------- class Rexx -+
| Typical calls:                                                              |
|                                                                             |
|  void main()                                                                |
|  {                                                                          |
|     Rexx rexx;                                                              |
|     Rexx::Script script("animal.r");                                        |
|     RexxString result;                                                      |
|                                                                             |
|     // to interpret the script:                                             |
|     Rexx.interpret(script, "", result);                                     |
|                                                                             |
|     // to transform an XML document xmlIn to xmlOut:                        |
|     Rexx.interpret(script, "arg", result, xmlIn, xmlOut);                   |
|  }                                                                          |
|                                                                             |
|  A stream can also be passed as the first argument to                       |
|  the Rexx::Script constructor.   If possible, the second argument           |
|  should be an identifier for clearly indicating the origin in the           |
|  error messages (otherwise; it will be "[UNKNOWN]")                         |
|                                                                             |
|  Ex:                                                                        |
|  void main()                                                                |
|  {                                                                          |
|     Rexx rexx;                                                              |
|     MemStream myRexx("Say 'Hello World!'");                                 |
|     Rexx::Script script(myRexx, "Inline Hello World");                      |
|     RexxString result;                                                      |
|     Rexx.interpret(script, "", result);                                     |
|  }                                                                          |
|                                                                             |
|  *** WARNING ***                                                            |
|  The passed stream must stay alive for the lifetime of the returned script! |
|                                                                             |
+----------------------------------------------------------------------------*/
class REXX_API Rexx {
public:
   class REXX_API Script {
      friend class Rexx;
   public:
      Script(
         istream & input,            //>>>PGR: warning, it is transient!
         RexxString const & strPath, // for info only
         streambuf * pSb = 0         // if caller has a preferred codebuffer streambuf
      );
      Script(
         RexxString const & strPath, // a qualified path to the file system
         streambuf * pSb = 0         // if caller has a preferred codebuffer streambuf
      );

   private:
      Clauses m_clauses;
      SymbolMap m_mapSym;
      LabelMap m_mapLabel;
      CodeBuffer m_codeBuf;
      istream & m_input;
      RexxString m_strPath;
      bool m_isCompiled;

      int interpret(
         char const * arg,
         RexxString & result,
         #if !defined COM_JAXO_YAXX_DENY_XML
            istream & xmlIn,
            ostream & xmlOut,
            CachedDtdList const & dtds,
         #endif
         UnicodeComposer & erh
      );
   };

private:
   static RexxString resultDeflt;
   static NullStream nilDevice;

public:
   /*---------------+
   | Public Methods |
   +---------------*/
   Rexx();

   int interpret(                         // return 0 or Rexx error no
      Script & script,                    // Script to interpret
      char const * arg = "",              // Rexx ARG (asciiz string)
      RexxString & result = resultDeflt,  // Rexx RESULT
      istream & xmlIn = nilDevice,        // HTML input stream
      ostream & xmlOut = nilDevice        // Transformed Output stream
   );

private:
   UnicodeComposer m_erh;
   #if !defined COM_JAXO_YAXX_DENY_XML
   CachedDtdList m_dtds;
   #endif
};

#ifdef YAXX_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
