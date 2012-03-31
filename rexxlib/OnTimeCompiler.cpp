/* $Id: OnTimeCompiler.cpp,v 1.5 2002-05-21 18:01:33 pgr Exp $ */

#include "OnTimeCompiler.h"

#ifdef YAXX_NAMESPACE
namespace YAXX_NAMESPACE {
#endif

/*---------------------------------------------OnTimeCompiler::OnTimeCompiler-+
| Constructor for interpreting a string (on the fly)                          |
+----------------------------------------------------------------------------*/
OnTimeCompiler::OnTimeCompiler(
   istream & input,
   CodeBuffer & cb,
   SymbolMap & mapSym,
   LabelMap & mapLabel,
   UnicodeComposer & erh
) :
   Compiler(input, cb, mapSym, mapLabel, erh)
{
}

/*------------------------------------------------OnTimeCompiler::onNewClause-+
| MN_NEWCLAUSE is just for tracing.                                           |
| Interpreted string have no real 'line numbers' and are not traced.          |
+----------------------------------------------------------------------------*/
void OnTimeCompiler::onNewClause() {
}

/*-------------------------------------------------OnTimeCompiler::onNewLabel-+
| MN_LABELCLAUSE is just for tracing labels or clauses.                       |
| Interpreted string can not have labels.                                     |
+----------------------------------------------------------------------------*/
bool OnTimeCompiler::onNewLabel() {
   return false;
}

/*------------------------------------------------OnTimeCompiler::onEndOfFile-+
|                                                                             |
+----------------------------------------------------------------------------*/
void OnTimeCompiler::onEndOfFile(streampos endPos, int lineno) {
}


#ifdef YAXX_NAMESPACE
}
#endif
/*===========================================================================*/
