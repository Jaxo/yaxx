/* $Id: BatchCompiler.h,v 1.6 2002-05-21 18:01:33 pgr Exp $ */

#ifndef COM_JAXO_YAXX_BATCHCOMPILER_H_INCLUDED
#define COM_JAXO_YAXX_BATCHCOMPILER_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include "Compiler.h"

#ifdef YAXX_NAMESPACE
namespace YAXX_NAMESPACE {
#endif

/*------------------------------------------------------ class BatchCompiler -+
| To compile a string in "batch" (vs. dynamic interpretation)                  |
+----------------------------------------------------------------------------*/
class BatchCompiler : public Compiler {
public:
   BatchCompiler(
      istream & input,
      RexxString const & strPath,
      Clauses & clauses,
      CodeBuffer & cb,
      SymbolMap & mapSym,
      LabelMap & mapLabel,
      UnicodeComposer & erh
   );
   ~BatchCompiler();

   void handleSyntaxError(UnicodeComposer::Message & msg);

private:
   class SyntaxWatcher : public UnicodeComposer::Watcher {
   public:
      SyntaxWatcher(BatchCompiler & cplr) : m_compiler(cplr) {}
   private:
      BatchCompiler & m_compiler;

      bool notifyNewEntry(UnicodeComposer::Message & msg);
   };
   Clauses & m_clauses;

   void onNewClause();
   bool onNewLabel();
   void onEndOfFile(streampos endPos, int lineno);
};

#ifdef YAXX_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
