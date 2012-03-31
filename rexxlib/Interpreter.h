/* $Id: Interpreter.h,v 1.90 2011-07-29 10:26:35 pgr Exp $ */

#ifndef COM_JAXO_YAXX_INTERPRETER_H_INCLUDED
#define COM_JAXO_YAXX_INTERPRETER_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include "Tracer.h"
#include "DataQueueMgr.h"
#include "VariableHandler.h"
#include "BuiltIn.h"
#include "Routines.h"
#include "CodeBuffer.h"

// class istream;
// class ostream;
class CachedDtdList;
class UnicodeComposer;

#ifdef YAXX_NAMESPACE
namespace YAXX_NAMESPACE {
#endif

class Clauses;
class SymbolMap;
class LabelMap;
class RecoverableException;
class Label;

/*-------------------------------------------------------- class Interpreter -+
|                                                                             |
+----------------------------------------------------------------------------*/
class Interpreter : private Constants
{
public:
   Interpreter(
      RexxString & strArgs,
      Clauses & clauses,
      SymbolMap & mapSym,
      LabelMap & mapLabel,
      CodeBuffer & codeBuffer,
      #if !defined COM_JAXO_YAXX_DENY_XML
         CachedDtdList const & dtds,
      #endif
      UnicodeComposer & erh
   );
   ~Interpreter();
   #if !defined COM_JAXO_YAXX_DENY_XML
   void setDefaultXmlStreams(istream & xmlIn, ostream & xmlOut);
   #endif
   RexxString run();

   void handleSyntaxError(UnicodeComposer::Message & msg);
   void handleFileCompilerError(
      UnicodeComposer::Message & msg,
      Location const & loc
   );

   static RexxString getYaxxVersion();  //<<<JAL FIXME Remove when we do buildNo object

private:
   enum { STACK_SIZE = 1000 };

   class SyntaxWatcher : public UnicodeComposer::Watcher {
   public:
      SyntaxWatcher(Interpreter & itpt) : m_interpreter(itpt) {}
   private:
      Interpreter & m_interpreter;

      bool notifyNewEntry(UnicodeComposer::Message & msg);
   };

   UnicodeComposer & m_erh;
   Location m_loc;
   Clauses & m_clauses;
   SymbolMap & m_mapSym;
   LabelMap & m_mapLabel;
   CodeBuffer & m_cb;
   Tracer m_tracer;
   DataQueueMgr m_dqm;
   VariableHandler m_varhdlr;          // variable handler
   BuiltIn m_builtIn;                  // builtin function handler

   RexxString * m_stack[STACK_SIZE];   // Array of String pointers
   RexxString m_tmpStack[STACK_SIZE];  // Stack of work Strings
   int m_top;                          // top item of stack

   Routines m_routines;

   int m_curClauseNo;
   int m_prevClauseNo;

   // bool goBackOneClause();

   void recoverFromException(RecoverableException const & e);
   RexxString mainLoop();
   void processCall(bool bypassInternals, Label * label=0);
   void startInternal(CodeBuffer::CallParams & parms);
   void endInternal();
   void startInterpret(bool isInteractiveTrace = false);
   void endInterpret();
   RexxString getProperty(Property prop) const;
   void setProperty(Property prop, RexxString & value);
   void loadFile(Arguments & args);
   void processTemplate(RexxString source);
   void processCommand(RexxString & cmd, RexxString & env);
   char processInteractiveTrace();
   void processTraceClause(RexxString ** ppStrCreated, int iMax);
   void expandRoutineArray();
   int execCommand(
      RexxString & cmd,
      DataQueueMgr::QueueMode qmodeIn,
      DataQueueMgr::QueueMode qmodeOut,
      RexxString & res
   );
};


#ifdef YAXX_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
