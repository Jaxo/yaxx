/* $Id: Rexx.cpp,v 1.137 2002-11-13 21:06:25 jlatone Exp $ */

#include "Rexx.h"
#include "BatchCompiler.h"

#ifdef YAXX_NAMESPACE
namespace YAXX_NAMESPACE {
#endif

NullStream Rexx::nilDevice;
RexxString Rexx::resultDeflt("");  // unchanged

/*-------------------------------------------------------Rexx::Script::Script-+
| Constructor for an owned stream                                                                      |
+----------------------------------------------------------------------------*/
Rexx::Script::Script(
   istream & input,               // stream to Rexx Script
   RexxString const & strPath,    // file identifier
   streambuf * pSb                // possibly a streambuf for codebuffer
) :
   m_clauses("IREXX"),
   m_input(input),
   m_strPath(strPath),
   m_isCompiled(false),
   m_codeBuf(pSb)
{
   if (!strPath.exists()) m_strPath = Constants::STR_UNKNOWN_PATH;
}

/*-------------------------------------------------------Rexx::Script::Script-+
| Constructor for a file-system/path based stream                             |
+----------------------------------------------------------------------------*/
Rexx::Script::Script(
   RexxString const & strPath,    // file path to Rexx Script
   streambuf * pSb                // possibly a streambuf for codebuffer
) :
   m_clauses("IREXX"),
   m_input(*m_clauses.makeNewInput(strPath)),
   m_strPath(strPath),
   m_isCompiled(false),
   m_codeBuf(pSb)
{
}

/*PRIVATE---------------------------------------------Rexx::Script::interpret-+
|                                                                             |
+----------------------------------------------------------------------------*/
int Rexx::Script::interpret(
   char const * arg,              // In: Rexx ARG (asciiz string, NO NULL!)
   RexxString & result,           // Result
   #if !defined COM_JAXO_YAXX_DENY_XML
      istream & xmlIn,            // HTML input stream
      ostream & xmlOut,           // Transformed output stream
      CachedDtdList const & dtds, // list of known DTD's (parsed, and cached)
   #endif
   UnicodeComposer & erh          // Error Handler
) {
   int rc = -1;
   RexxString strArgs(arg, strlen(arg));
   BatchCompiler * pCompiler = 0;
   Interpreter * pInterpreter = 0;
   try {
      if (!m_isCompiled) {
         pCompiler = new BatchCompiler(
            m_input,
            m_strPath,
            m_clauses,
            m_codeBuf,
            m_mapSym,
            m_mapLabel,
            erh
         );
         pCompiler->run();
         delete pCompiler;
         pCompiler = 0;
         m_isCompiled = true;
      }
      pInterpreter = new Interpreter(
         strArgs,
         m_clauses,
         m_mapSym,
         m_mapLabel,
         m_codeBuf,
         #if !defined COM_JAXO_YAXX_DENY_XML
            dtds,
         #endif
         erh
      );
      #if !defined COM_JAXO_YAXX_DENY_XML
         pInterpreter->setDefaultXmlStreams(xmlIn, xmlOut);
      #endif
      result = pInterpreter->run();
      delete pInterpreter;
      return 0;
   }catch (FatalException & e) {
      delete pCompiler;
      delete pInterpreter;
      return e.m_codeNo;
   }catch (...) {
      delete pCompiler;
      delete pInterpreter;
      throw;
   }
   return -1;
}

/*-----------------------------------------------------------------Rexx::Rexx-+
|                                                                             |
+----------------------------------------------------------------------------*/
Rexx::Rexx()
{
   #if !defined COM_JAXO_YAXX_DENY_XML
   #if !defined MWERKS_NEEDS_DTD_FIX
      CachedDtdListFactory factory(m_erh, m_dtds);
      // YASP3 gets this far.  Probably related to CODEBASE_SIZE problem.
      factory.addDtd(
         "HTML",
         new MemStream(getHtmlDtd(), getHtmlDtdLength(), MemStream::Constant)
      );
   #endif
   #endif
}

/*------------------------------------------------------------Rexx::interpret-+
|                                                                             |
+----------------------------------------------------------------------------*/
int Rexx::interpret(
   Script & script,
   char const * arg,             // In: Rexx ARG (asciiz string)
   RexxString & result,          // Out: Result
   istream & xmlIn,              // HTML input stream
   ostream & xmlOut              // Output stream
) {
   RexxString temp;
   return script.interpret(
      arg,
      (&result == &resultDeflt)? temp : result,
      #if !defined COM_JAXO_YAXX_DENY_XML
         xmlIn,
         xmlOut,
         m_dtds,
      #endif
      m_erh
   );
}

#ifdef YAXX_NAMESPACE
}
#endif
/*===========================================================================*/
