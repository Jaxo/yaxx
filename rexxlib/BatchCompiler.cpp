/* $Id: BatchCompiler.cpp,v 1.13 2002-06-07 09:31:07 pgr Exp $ */

#include "BatchCompiler.h"
#include "Clauses.h"
#include "Exception.h"
#include "Tracer.h"
#include "IOMgr.h"

#ifdef YAXX_NAMESPACE
namespace YAXX_NAMESPACE {
#endif

/*-----------------------------------------------BatchCompiler::BatchCompiler-+
| To compile a stream in "batch" (vs. dynamic interpretation)                 |
|                                                                             |
| Implementation: notice that the constructor does not really throw an        |
| exception.  clauses.read may throw an exception, but this exception         |
| is thrown *before* the constructor is called.                               |
+----------------------------------------------------------------------------*/
BatchCompiler::BatchCompiler(
   istream & input,
   RexxString const & strPath,
   Clauses & clauses,
   CodeBuffer & cb,
   SymbolMap & mapSym,
   LabelMap & mapLabel,
   UnicodeComposer & erh
) :
   Compiler(input, cb, mapSym, mapLabel, erh),
   m_clauses(clauses)
{
   m_erh.pushWatcher(new SyntaxWatcher(*this));
   m_clauses.start(input, strPath, m_erh);
}

/*----------------------------------------------BatchCompiler::~BatchCompiler-+
|                                                                             |
+----------------------------------------------------------------------------*/
BatchCompiler::~BatchCompiler() {
   m_erh.popWatcher();
}

/*-------------------------------------------------BatchCompiler::onNewClause-+
| MN_NEWCLAUSE is just for tracing clauses.                                   |
+----------------------------------------------------------------------------*/
void BatchCompiler::onNewClause() {
   addClause(m_clauses);
}

/*--------------------------------------------------BatchCompiler::onNewLabel-+
| MN_LABELCLAUSE is just for tracing labels or clauses.                       |
+----------------------------------------------------------------------------*/
bool BatchCompiler::onNewLabel() {
   addLabel(m_clauses);
   return true;
}

/*-------------------------------------------------BatchCompiler::onEndOfFile-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BatchCompiler::onEndOfFile(streampos endPos, int lineno) {
   m_clauses.end(endPos, lineno);
}

/*-------------------------------------------BatchCompiler::handleSyntaxError-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BatchCompiler::handleSyntaxError(UnicodeComposer::Message & msg)
{
   Location loc;
   if (msg.inqSeverity() >= ECE__ERROR) {
      m_clauses.locate(getLastPos(), loc);
      Tracer::traceError(msg, loc);
      throw FatalException(getRxMainCodeNo(msg.inqStringId()));
   }else {
      Tracer::traceWarning(msg);
   }
}

/*-------------------------------BatchCompiler::SyntaxWatcher::notifyNewEntry-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool BatchCompiler::SyntaxWatcher::notifyNewEntry(
   UnicodeComposer::Message & msg
) {
   m_compiler.handleSyntaxError(msg);
   return true;
}

#ifdef YAXX_NAMESPACE
}
#endif
/*===========================================================================*/
