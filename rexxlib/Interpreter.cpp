/*
 * $Id: Interpreter.cpp,v 1.277 2013-08-03 07:34:23 pgr Exp $
 */

#if defined _WIN32
#include <io.h>

#elif defined MWERKS_NEEDS_ADDR_FIX // include
#include <unix_fcntl.h>

#else
#include <fcntl.h>
#include <unistd.h>
#endif

#if !defined MWERKS_NEEDS_ADDR_FIX  // include
#include <sys/stat.h>
#endif

#include <string.h>
#include <assert.h>

#include "../toolslib/SystemContext.h"
#include "../reslib/DayMonth.h"
#include "Clauses.h"
#include "Label.h"
#include "Exception.h"
#include "OnTimeCompiler.h"
#include "BatchCompiler.h"
#include "Interpreter.h"

#ifdef __MWERKS_NEEDS_MEMCMP_FIX__
#define memcmp memcmpFix4PalmOS30
#include "../../palmiolib/memcmp.h"
#endif

#ifdef YAXX_NAMESPACE
namespace YAXX_NAMESPACE {
#endif

#ifndef S_IREAD
#define S_IREAD 0
#define S_IWRITE 1
#endif

#define XSTR(s) STR(s)
#define STR(s) #s

/*---------------------------------------------------Interpreter::Interpreter-+
| Constructor                                                                 |
+----------------------------------------------------------------------------*/
Interpreter::Interpreter(
   RexxString & strArgs,
   Clauses & clauses,
   SymbolMap & mapSym,
   LabelMap & mapLabel,
   CodeBuffer & cb,
   #if !defined COM_JAXO_YAXX_DENY_XML
      CachedDtdList const & dtds,
   #endif
   UnicodeComposer & erh
) :
   m_erh(erh),
   m_clauses(clauses),
   m_mapSym(mapSym),
   m_mapLabel(mapLabel),
   m_cb(cb),
   m_tracer(m_loc, erh),
   m_varhdlr(m_tracer, mapSym),
   m_routines(strArgs, erh),
   #if !defined COM_JAXO_YAXX_DENY_XML
      m_builtIn(m_varhdlr, m_tracer, clauses, m_dqm, m_routines, dtds, erh),
   #else
      m_builtIn(m_varhdlr, m_tracer, clauses, m_dqm, m_routines, erh),
   #endif
   m_curClauseNo(0),
   m_prevClauseNo(0)
{
   erh.pushWatcher(new SyntaxWatcher(*this));
}

/*--------------------------------------------------Interpreter::~Interpreter-+
|                                                                             |
+----------------------------------------------------------------------------*/
Interpreter::~Interpreter() {
   m_erh.popWatcher();
}

#if !defined COM_JAXO_YAXX_DENY_XML
/*------------------------------------------Interpreter::setDefaultXmlStreams-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Interpreter::setDefaultXmlStreams(istream & xmlIn, ostream & xmlOut) {
   m_builtIn.setDefaultXmlStreams(xmlIn, xmlOut);
}
#endif

/*-----------------------------------------------------------Interpreter::run-+
|                                                                             |
+----------------------------------------------------------------------------*/
RexxString Interpreter::run()
{
   m_top = -1;
   m_cb.jumpTo(CodeBuffer::Start);

   for (;;) {
      try {
         try {
            return mainLoop();
         }catch (RexxString::Exception e) {
            m_erh << ECE__ERROR << makeMsgTemplateId(e.getReason()) << endm;
         }
      }catch (RecoverableException & e) {
         recoverFromException(e);
      }
   }
}

/*------------------------------------------Interpreter::recoverFromException-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Interpreter::recoverFromException(RecoverableException const & e)
{
   Signaled sig = e.getCondition();
   bool isSignalDisabled = m_routines.isSignalDisabled();

   /* first we need to terminate all the interpret strings */
   while (m_routines.m_ct == CT_INTERPRET) {
      endInterpret();
   }

   if (isSignalDisabled) {                        // SIG_SYNTAX only
      m_tracer.traceInteractiveError(e);
      if (m_top >= 0) {                           // not at the top level
         *m_stack[m_top] = "";                    // empty the result
      }
      /* try to pursue... */

   }else {
      RexxString & strCnd = m_routines.getSignal(sig);
      Label * label = m_mapLabel.get(strCnd);
      if (!label || label->isExternal()) {
         m_erh << ECE__ERROR << _REX__16_1 << strCnd << endm;
      }
      m_varhdlr.setRC(e.getRxMainCodeNo());
      m_varhdlr.setSIGL(m_clauses.getSourceLine(m_curClauseNo));
      if (e.isStmtSignal()) {
         m_top = m_routines.getStackTop();      // clear stack
         m_cb.jumpTo(label->getPos());
      }else {
         CodeBuffer::CallParams parms;
         parms.label = label;
         parms.presenceBits = 0;
         parms.sigl = m_clauses.getSourceLine(m_curClauseNo);
         parms.ct = CT_SUBROUTINE;
         // simulate PUSHTMP (adding room for the result string)
         ++m_top;
         m_stack[m_top] = m_tmpStack + m_top;
         startInternal(parms);
      }
   }
}

/*-----------------------------------------------Interpreter::goBackOneClause-+
|                                                                             |
+----------------------------------------------------------------------------*/
// inline bool Interpreter::goBackOneClause() {
//    if (m_curClauseNo == m_prevClauseNo) {
//       return false;
//    }else {
//       m_curClauseNo = m_prevClauseNo;
//       return true;
//    }
// }

/*------------------------------------------------------Interpreter::mainLoop-+
|                                                                             |
+----------------------------------------------------------------------------*/
RexxString Interpreter::mainLoop()
{
   RexxString * ppStrCreated[15];
   int iMaxCreated = 0;
   OpCode op;

   for (;;) {
      switch (op = m_cb.readOpCode()) {

      case I_CLAUSE:                // beginning of a clause
         m_builtIn.onNewClause();
         /* fall thru */
      case I_LABEL:
         processTraceClause(ppStrCreated, iMaxCreated);
         iMaxCreated = 0;
         continue;

      case I_PUSH:                  // push a literal
         if (++m_top==STACK_SIZE-1) m_erh << ECE__ERROR << _REX__5_0 << endm;
         m_cb.readString(m_stack + m_top);
         continue;

      case I_PUSHLIT:               // push a literal, and trace
         if (++m_top==STACK_SIZE-1) m_erh << ECE__ERROR << _REX__5_0 << endm;
         m_cb.readString(m_stack + m_top);
         if (m_tracer.isTracingIntermediates()) {
            m_tracer.traceLiteral(*m_stack[m_top]);
         }
         continue;

      case I_PUSHTMP:
         if (++m_top==STACK_SIZE-1) m_erh << ECE__ERROR << _REX__5_0 << endm;
         m_stack[m_top] = m_tmpStack + m_top;
         continue;

      case I_POP:                   // pop some stack items
         m_top -= m_cb.readByte();
         continue;

      case I_COPY:                  // copy top item to previous one
         *m_stack[m_top-1] = m_stack[m_top];
         m_top -= 2;
         continue;

      case I_COPY2TMP:
         m_tmpStack[m_top] = *m_stack[m_top];
         continue;

      case I_LOAD_VAR:              // load a variable
         if (++m_top == STACK_SIZE-1) m_erh << ECE__ERROR << _REX__5_0 << endm;
         {
            bool isAssigned;
            m_stack[m_top] = m_varhdlr.loadVar(
               (Symbol *)m_cb.readPointer(),
               m_tmpStack[m_top],
               isAssigned
            );
            if (!isAssigned) m_routines.raise(SIG_NOVALUE);
         }
         if (m_tracer.isTracingIntermediates()) {
            m_tracer.traceVariable(*m_stack[m_top]);
         }
         continue;

      case I_CREATE_VAR:            // create a variable
         if (++m_top == STACK_SIZE-1) m_erh << ECE__ERROR << _REX__5_0 << endm;
         m_stack[m_top] = m_varhdlr.createVar(
            (Symbol *)m_cb.readPointer()
         );
         if (iMaxCreated < sizeof ppStrCreated / sizeof ppStrCreated[0]) {
            ppStrCreated[iMaxCreated++] = m_stack[m_top];
         }
         continue;

      case I_DROP_VAR:              // drop a variable
         m_varhdlr.dropVar((Symbol *)m_cb.readPointer());
         continue;

      case I_DROP_REF:              // indirect drop, from stack
         /* assume that is UPPER case tmp */
         m_varhdlr.dropIndirect(*m_stack[m_top]);
         if (m_tracer.isTracingIntermediates()) {
            m_tracer.traceVariable(*m_stack[m_top]);
         }
         --m_top;
         continue;

      case I_SET_STEM:
         m_varhdlr.assignStem((Symbol *)m_cb.readPointer());
         continue;

      case I_LOAD_ARG:              // push an argument
         if (++m_top == STACK_SIZE-1) m_erh << ECE__ERROR << _REX__5_0 << endm;
         m_tmpStack[m_top] = m_routines.m_args.getArg(m_cb.readByte());
         m_stack[m_top] = &m_tmpStack[m_top];
         continue;

      case I_JMP_ALWAYS:   // jump unconditional
         m_cb.jumpTo(m_cb.readPos());
         continue;

      case I_JMP_FALSE:    // jump if false
         {
            CodePosition w = m_cb.readPos();
            if (!(bool)(*m_stack[m_top--])) {
               m_cb.jumpTo(w);
            }
         }
         continue;

      case I_JMP_TRUE:     // jump if true
         {
            CodePosition w = m_cb.readPos();
            if ((bool)(*m_stack[m_top--])) {
               m_cb.jumpTo(w);
            }
         }
         continue;

      case I_JMP_LBL:      // jmp to a label
         m_top = m_routines.getStackTop();   // clear stack
         {
            Label * label = (Label *)m_cb.readPointer();
            if (label->isExternal()) {
               m_erh << ECE__ERROR << _REX__16_1 << label->key() << endm;
            }
            m_varhdlr.setSIGL(m_clauses.getSourceLine(m_curClauseNo));
            m_cb.jumpTo(label->getPos());
         }
         continue;

      case I_JMP_VAL:      // jmp to a label resulting from an expr
         m_top = m_routines.getStackTop();   // clear stack
         {
            Label * label = m_mapLabel.get(*m_stack[m_top--]);
            if (!label || label->isExternal()) {
               m_erh << ECE__ERROR << _REX__16_1 << *m_stack[m_top+1] << endm;
            }
            m_varhdlr.setSIGL(m_clauses.getSourceLine(m_curClauseNo));
            m_cb.jumpTo(label->getPos());
         }
         continue;

      case I_NOP:          // NOP is no operation
         continue;

      case I_CALL_ANY:     // call internal or external
         processCall(false);
         continue;

      case I_CALL_EXT:     // As above, but bypass internals
         processCall(true);
         continue;

      case I_CALL_REF:     // call by reference
         {
            static RexxString hackedLabel("#DEFAULTPROC");
            Label * label = m_mapLabel.get(*m_stack[m_top--]);
            if (!label || label->isExternal()) {
               // AWFULL HACK....
               label = m_mapLabel.get(hackedLabel);
               if (!label || label->isExternal()) {
                  m_erh << ECE__ERROR << _REX__16_1 << *m_stack[m_top+1] << endm;
               }
            }
            processCall(false, label);
         }
         continue;

      case I_INTERPRET:             // interpret
         m_tmpStack[m_top] = *m_stack[m_top];
         startInterpret();
         continue;

      case I_SYSTEM:                // shell the system
         processCommand(*m_stack[m_top], *m_stack[m_top-1]);
         m_top -= 2;
         continue;

      case I_RETURN_EMPTY:          // clear stack and return
         while (m_routines.m_ct == CT_INTERPRET) {
            endInterpret();
         }
         if (m_routines.m_ct == CT_FUNCTION) {
            m_erh << ECE__ERROR << _REX__44_0 << endm;
         }
         if (m_routines.getDepth() == 0) {  // at root
            return RexxString::Nil;
         }
         endInternal();
         continue;

      case I_RETURN_RESULT:   // same as above, but a value is returned
         {
            RexxString result = *m_stack[m_top];        // get returned data
            while (m_routines.m_ct == CT_INTERPRET) {
               endInterpret();
            }
            if (m_routines.getDepth() == 0) {           // at root
               return result;
            }else if (m_routines.m_ct == CT_FUNCTION) {
               m_routines.m_args.getResult() = result;
               endInternal();
            }else {
               if (m_routines.m_isProcedure) {          // private scope
                  m_tmpStack[m_top] = result;
               }
               endInternal();
               m_varhdlr.setRESULT(result);
            }
            continue;
         }

      case I_EXIT_EMPTY:      // exit pgm with RC=0
         return RexxString::Nil;

      case I_EXIT_RESULT:     // exit pgm with RC=expr
         return *m_stack[m_top];

      case I_ERROR:
         m_erh << ECE__ERROR << getRxMainId(m_cb.readByte()) << endm;
         continue;

      case I_PROC:
         m_erh << ECE__ERROR << _REX__17_1 << endm;
         continue;

      case I_PLUS:
         --m_top;
         m_stack[m_top]->plus(
            *m_stack[m_top+1], m_routines.getDecRexxContext()
         );
         if (m_tracer.isTracingIntermediates()) {
            m_tracer.traceMonadicOp(*m_stack[m_top]);
         }
         continue;

      case I_MINUS:
         --m_top;
         m_stack[m_top]->minus(
            *m_stack[m_top+1], m_routines.getDecRexxContext()
         );
         if (m_tracer.isTracingIntermediates()) {
            m_tracer.traceMonadicOp(*m_stack[m_top]);
         }
         continue;

      case I_NOT:
         --m_top;
         m_stack[m_top]->assign(!(bool)(*m_stack[m_top+1]));
         if (m_tracer.isTracingIntermediates()) {
            m_tracer.traceMonadicOp(*m_stack[m_top]);
         }
         continue;

      case I_AND:
         m_top -= 2;
         m_stack[m_top]->assign((bool)
            (bool)*m_stack[m_top+1] && (bool)*m_stack[m_top+2]
         );
         if (m_tracer.isTracingIntermediates()) {
            m_tracer.traceDyadicOp(*m_stack[m_top]);
         }
         continue;

      case I_OR:
         m_top -= 2;
         m_stack[m_top]->assign((bool)
            (bool)*m_stack[m_top+1] || (bool)*m_stack[m_top+2]
         );
         if (m_tracer.isTracingIntermediates()) {
            m_tracer.traceDyadicOp(*m_stack[m_top]);
         }
         continue;

      case I_XOR:
         m_top -= 2;
         m_stack[m_top]->assign((bool)
            (bool)*m_stack[m_top+1] ^ (bool)*m_stack[m_top+2]
         );
         if (m_tracer.isTracingIntermediates()) {
            m_tracer.traceDyadicOp(*m_stack[m_top]);
         }
         continue;

      case I_CONCAT_A:   // abbutal
         m_top -= 2;
         m_stack[m_top]->concat(*m_stack[m_top+1], *m_stack[m_top+2]);
         if (m_tracer.isTracingIntermediates()) {
            m_tracer.traceDyadicOp(*m_stack[m_top]);
         }
         continue;

      case I_CONCAT_B:   // space
         m_top -= 2;
         m_stack[m_top]->bconcat(*m_stack[m_top+1], *m_stack[m_top+2]);
         if (m_tracer.isTracingIntermediates()) {
            m_tracer.traceDyadicOp(*m_stack[m_top]);
         }
         continue;

      case I_SUB:
         m_top -= 2;
         if (
            !m_stack[m_top]->subtract(
               *m_stack[m_top+1],
               *m_stack[m_top+2],
               m_routines.getDecRexxContext()
            )
         ) {
            m_routines.raise(SIG_LOSTDIGITS);
         }
         if (m_tracer.isTracingIntermediates()) {
            m_tracer.traceDyadicOp(*m_stack[m_top]);
         }
         continue;

      case I_ADD:
         m_top -= 2;
         if (
            !m_stack[m_top]->add(
               *m_stack[m_top+1],
               *m_stack[m_top+2],
               m_routines.getDecRexxContext()
            )
         ) {
            m_routines.raise(SIG_LOSTDIGITS);
         }
         if (m_tracer.isTracingIntermediates()) {
            m_tracer.traceDyadicOp(*m_stack[m_top]);
         }
         continue;

      case I_MUL:
         m_top -= 2;
         if (
            !m_stack[m_top]->multiply(
               *m_stack[m_top+1],
               *m_stack[m_top+2],
               m_routines.getDecRexxContext()
            )
         ) {
            m_routines.raise(SIG_LOSTDIGITS);
         }
         if (m_tracer.isTracingIntermediates()) {
            m_tracer.traceDyadicOp(*m_stack[m_top]);
         }
         continue;

      case I_DIV:
         m_top -= 2;
         if (
            !m_stack[m_top]->divide(
               *m_stack[m_top+1],
               *m_stack[m_top+2],
               m_routines.getDecRexxContext()
            )
         ) {
            m_routines.raise(SIG_LOSTDIGITS);
         }
         if (m_tracer.isTracingIntermediates()) {
            m_tracer.traceDyadicOp(*m_stack[m_top]);
         }
         continue;

      case I_IDIV:
         m_top -= 2;
         if (
            !m_stack[m_top]->divideInteger(
               *m_stack[m_top+1],
               *m_stack[m_top+2],
               m_routines.getDecRexxContext()
            )
         ) {
            m_routines.raise(SIG_LOSTDIGITS);
         }
         if (m_tracer.isTracingIntermediates()) {
            m_tracer.traceDyadicOp(*m_stack[m_top]);
         }
         continue;

      case I_MOD:
         m_top -= 2;
         if (
            !m_stack[m_top]->remainder(
               *m_stack[m_top+1],
               *m_stack[m_top+2],
               m_routines.getDecRexxContext()
            )
         ) {
            m_routines.raise(SIG_LOSTDIGITS);
         }
         if (m_tracer.isTracingIntermediates()) {
            m_tracer.traceDyadicOp(*m_stack[m_top]);
         }
         continue;

      case I_POWER:
         m_top -= 2;
         if (
            !m_stack[m_top]->power(
               *m_stack[m_top+1],
               *m_stack[m_top+2],
               m_routines.getDecRexxContext()
            )
         ) {
            m_routines.raise(SIG_LOSTDIGITS);
         }
         if (m_tracer.isTracingIntermediates()) {
            m_tracer.traceDyadicOp(*m_stack[m_top]);
         }
         continue;

      case I_NORMAL_EQ:
      case I_NORMAL_GE:
      case I_NORMAL_GT:
      case I_NORMAL_LE:
      case I_NORMAL_LT:
      case I_NORMAL_NE:
         m_top -= 2;
         m_stack[m_top]->assign(
            0 != (
               (op - I_NORMAL_GT + 1) &
               m_stack[m_top+1]->normalCompare(
                  *m_stack[m_top+2],
                  m_routines.getDecRexxContext()
               )
            )
         );
         if (m_tracer.isTracingIntermediates()) {
            m_tracer.traceDyadicOp(*m_stack[m_top]);
         }
         continue;

      case I_STRICT_EQ:
      case I_STRICT_GE:
      case I_STRICT_GT:
      case I_STRICT_LE:
      case I_STRICT_LT:
      case I_STRICT_NE:
         m_top -= 2;
         m_stack[m_top]->assign(
            0 != (
               (op - I_STRICT_GT + 1) &
               m_stack[m_top+1]->strictCompare(*m_stack[m_top+2])
            )
         );
         if (m_tracer.isTracingIntermediates()) {
            m_tracer.traceDyadicOp(*m_stack[m_top]);
         }
         continue;

      case I_SAY:                     // printf the string at the top
         SystemContext::cout().write(
            *m_stack[m_top],
            m_stack[m_top]->length()
         );
         SystemContext::cout() << std::endl;
         --m_top;
         continue;

      case I_OPTIONS:
         {
            RexxString & theOptions = *m_stack[m_top--];
         }
         continue;                  // NOT YET IMPLEMENTED

      case I_GET_PROP:              // get a property
         if (++m_top == STACK_SIZE-1) m_erh << ECE__ERROR << _REX__5_0 << endm;
         m_stack[m_top] = m_tmpStack + m_top;
         m_tmpStack[m_top] = getProperty((Property)m_cb.readByte());
         continue;

      case I_SET_PROP:              // set a property
         setProperty((Property)m_cb.readByte(), *m_stack[m_top]);
         --m_top;
         continue;

      case I_DO_CTL_INIT:           // check if the control var is numeric
         if (!m_stack[m_top+1]->isNumber()) {
            m_erh << ECE__ERROR << _REX__41_6 << *m_stack[m_top+1] << endm;
         }
         continue;

      case I_DO_TO_INIT:            // check if TO is numeric
         if (!m_stack[m_top]->isNumber()) {
            m_erh << ECE__ERROR << _REX__41_4 << *m_stack[m_top] << endm;
         }
//       m_tmpStack[m_top] = *m_stack[m_top];  // Useless
         continue;

      case I_DO_BY_INIT:            // check if BY is numeric
         if (!m_stack[m_top]->isNumber()) {
            m_erh << ECE__ERROR << _REX__41_5 << *m_stack[m_top] << endm;
         }
         continue;

      case I_DO_FOR_INIT:           // check if repeat is < 0
         if (m_stack[m_top]->isNegative()) {
            m_erh << ECE__ERROR << _REX__26_3 << *m_stack[m_top] << endm;
         }
         m_tmpStack[m_top] = *m_stack[m_top];
         continue;

      case I_DO_TO_BY_TEST:         // if BY is < 0 x<=TO, else x>=TO
         {
            RexxString temp;
            bool isAssigned;
            RexxString * by;
            RexxString * to = m_stack[m_top-m_cb.readByte()];  // TO
            RexxString * ctl = m_varhdlr.loadVar(
               (Symbol *)m_cb.readPointer(), temp, isAssigned
            );
            CodePosition w = m_cb.readPos();
            by = m_stack[m_top-m_cb.readByte()];
            if (!isAssigned) m_routines.raise(SIG_NOVALUE);
            RexxString::COMPARE_RESULT k = to->normalCompare(
               *ctl, m_routines.getDecRexxContext()
            );
            if (by->isNegative()) {
               if (k == RexxString::IS_GT) m_cb.jumpTo(w);
            }else {
               if (k == RexxString::IS_LT) m_cb.jumpTo(w);
            }
         }
         continue;

      case I_DO_TO_TEST:
         {
            RexxString temp;
            bool isAssigned;
            RexxString * to = m_stack[m_top-m_cb.readByte()];  // TO
            RexxString * ctl = m_varhdlr.loadVar(
               (Symbol *)m_cb.readPointer(), temp, isAssigned
            );
            CodePosition w = m_cb.readPos();
            if (!isAssigned) m_routines.raise(SIG_NOVALUE);
            if (RexxString::IS_LT == to->normalCompare(
                  *ctl, m_routines.getDecRexxContext()
               )
            ) {
               m_cb.jumpTo(w);
            }
         }
         continue;

      case I_DO_FOR_TEST:
         {
            RexxString & strWork = m_tmpStack[m_top-m_cb.readByte()];
            CodePosition w = m_cb.readPos();
            if (strWork.isZero()) {
               m_cb.jumpTo(w);
            }else {
               strWork.decrement(m_routines.getDecRexxContext());
            }
         }
         continue;

      case I_DO_BY_STEP:
         {
            RexxString temp;
            bool isAssigned;
            RexxString * by = m_stack[m_top-m_cb.readByte()];
            RexxString * ctl = m_varhdlr.loadVar(
               (Symbol *)m_cb.readPointer(), temp, isAssigned
            );
            if (!isAssigned) m_routines.raise(SIG_NOVALUE);
            ctl->add(*ctl, *by, m_routines.getDecRexxContext());
         }
         continue;

      case I_DO_STEP:
         {
            RexxString temp;
            bool isAssigned;
            RexxString * ctl = m_varhdlr.loadVar(
               (Symbol *)m_cb.readPointer(), temp, isAssigned
            );
            if (!isAssigned) m_routines.raise(SIG_NOVALUE);
            ctl->increment(m_routines.getDecRexxContext());
         }
         continue;

      case I_TEMPLATE_START:          // a template starts
         /* the RexxString must stay alive until the parse ends! */
         processTemplate(*m_stack[m_top]);
         --m_top;
         continue;

      case I_STACK_QUEUE:             // queue stack to Rexx queue
         m_dqm.enqueue(*m_stack[m_top--]);
         continue;

      case I_STACK_PUSH:              // push stack to Rexx queue
         m_dqm.push(*m_stack[m_top--]);
         continue;

      case I_STACK_PULL:              // pull stack to Rexx queue
         ++m_top;
         *(m_stack[m_top] = m_tmpStack + m_top) = m_dqm.pull();
         continue;

      case I_STACK_LINEIN:            // parse linenin
         ++m_top;
         *(m_stack[m_top] = m_tmpStack + m_top) = IOMgr::readLine();
         continue;

      case I_UPPER:
         m_stack[m_top]->convert(CharConverter::Upper);
         continue;

      case I_EOF:
         if (m_routines.m_ct == CT_INTERPRET) {
            endInterpret();
            continue;
         }else {
            return RexxString::Nil;
         }

      default:
         m_erh << ECE__FATAL << _REX__49_0 << endm;
      }
   }
}

/*-----------------------------------------------Interpreter::processTemplate-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Interpreter::processTemplate(RexxString const toParse)
{
   OpCode op;
   int begData = 0;
   int begBreak = 0;
   int endBreak = 0;
   int endSource = toParse.length();
   int const baseTop = m_top;
   int curTop;
   do {

       /*
       | Isolate the data to be processed on this iteration.
       |
       | begBreak and endBreak indicate the position in the source
       | string where there is a break that divides the source.
       | When the break is a pattern they are the start of the pattern
       | and the position just beyond it.
       |
       | this updates:
       | - begData which indicates the start of the isolated data
       | - begBreak and endBreak, ready for the next iteration.
       | - curTop by pushing all targets that need to be set
       |
       | The source data to be processed next will normally start
       | at the end of the break that ended the previous piece.
       | However, the relative positionals alter this.
       |
       | If a positional takes the break leftwards from the end of the
       | previous selection, the source selected is the rest of the string.
       */
       curTop = baseTop;
       begData = endBreak;
       for (;;) {
          unsigned int pos;
          RexxString * pTrigVal;
          RexxString strDefault;

          switch (op = m_cb.readOpCode()) {

          /*
          | When no more explicit breaks, break is at the end of the source.
          */
          case I_TEMPLATE_END:
             begBreak = endSource;
             endBreak = endSource;
             break;     // i.e.: break the loop

          /*
          | Targets (not triggers)
          */
          case I_TARGET_DOT:                  // dot target, not a trigger
             if (++curTop == STACK_SIZE-1) m_erh << ECE__ERROR << _REX__5_0 << endm;
             m_stack[curTop] = 0;             // just a place holder
             continue;

          case I_TARGET_VAR:                  // var target, not a trigger
             if (++curTop == STACK_SIZE-1) m_erh << ECE__ERROR << _REX__5_0 << endm;
             m_stack[curTop] = m_varhdlr.createVar(
                (Symbol *)m_cb.readPointer()
             );
             continue;

          /*
          | Triggers
          */
          default:    // I_TRIG_xxx
             switch (m_cb.readOpCode()) {
             case I_LOAD_VAR:
                // this is LOAD_VAR:  FixMe!!!!
                {
                   bool isAssigned;
                   pTrigVal = m_varhdlr.loadVar(
                      (Symbol *)m_cb.readPointer(),
                      strDefault,
                      isAssigned
                   );
                   if (!isAssigned) m_routines.raise(SIG_NOVALUE);
                }
                if (m_tracer.isTracingIntermediates()) {
                   m_tracer.traceVariable(*pTrigVal);
                }
                if (op != I_TRIG_LIT) pos = (int)(*pTrigVal);
                break;
             case I_IMM_STRING:
                m_cb.readString(&pTrigVal);
                break;
             case I_IMM_VAL:
                pos = m_cb.readWord();
                break;
             default:
                assert (false);
             }

             if (op == I_TRIG_LIT) {
                /*
                | Pattern Triggers
                */
                pos = pTrigVal->pos(toParse, begData+1);  // C index <-> Rexx
                if (pos) {
                   // Selected source runs up to the pattern.
                   begBreak = pos - 1;                    // C index <-> Rexx
                   endBreak = begBreak + pTrigVal->length();
                }else {
                   // the rest of the source is selected.
                   begBreak = endSource;
                   endBreak = endSource;
                }
             }else {

                /*
                | Positional Triggers
                | Specify where the relevant piece of the subject ends.
                | For a relative positional, the position is relative to the
                | start of the previous trigger, and the source segment starts
                | there.
                */
                switch (op) {
                case I_TRIG_PLUS_REL:
                   begData = begBreak;
                   begBreak += pos;
                   break;
                case I_TRIG_MINUS_REL:
                   begData = begBreak;
                   begBreak -= pos;
                   break;
                default:
                   assert (op == I_TRIG_ABS);
                   begBreak = pos-1;                // C index <-> Rexx
                   break;
                }

                // Adjustment should remain within the source
                if (begBreak < 0) {
                   begBreak = 0;
                }else if (begBreak > endSource) {
                   begBreak = endSource;
                }
                // No actual literal marks the boundary.
                endBreak = begBreak;
             }
          }
          break;
       }

       if (curTop > baseTop) {
          int pushed = curTop - baseTop;
          curTop = baseTop;
          /* Isolated data, to be assigned from */
          char const * pBegData = ((char const *)toParse) + begData;
          char const * pEndData;
          if (endBreak <= begData) {
             pEndData = ((char const *)toParse) + endSource;
          }else {
             pEndData = ((char const *)toParse) + begBreak;
          }

          do {              // Until no more targets for this data.
             char const * pBegWord;
             int lenWord;
             if (--pushed == 0) {
                /* Last target gets all the residue of the Data. */
                pBegWord = pBegData;
                lenWord = pEndData - pBegData;
             }else {
                /* Not last target; assign a word. */
                /* skip leading spaces */
                while ((pBegData < pEndData) && (*pBegData == ' ')) {
                   ++pBegData;
                }
                pBegWord = pBegData;

                /* the word terminator is not part of the residual data */
                lenWord = 0;
                while ((pBegData < pEndData) && (*pBegData++ != ' ')) {
                   ++lenWord;
                }
             }
             if (m_stack[++curTop] == 0) {  // it is a DOT variable (dummy)
                if (m_tracer.isTracingIntermediates()) {
                   m_tracer.tracePlaceholder(RexxString(pBegWord, lenWord));
                }
             }else {
                m_stack[curTop]->assign(pBegWord, lenWord);
                if (m_tracer.isTracingResults()) {
                   m_tracer.traceAssign(*m_stack[curTop]);
                }
             }
          }while (pushed);
       }
   }while (op != I_TEMPLATE_END);
}

/*---------------------------------------------------Interpreter::processCall-+
| IMPLEMENTATION:                                                             |
| 1) Commands and Built-in's differ from Internals.  If they fail, the        |
|    calling Internal is failing.  Also they don't use the interpreter stack. |
| 2) LOAD is processed by the Interpreter class: it modifies the clauses      |
|    and code buffer which are fields that we don't want to share.            |
+----------------------------------------------------------------------------*/
void Interpreter::processCall(bool bypassInternals, Label * label)
{
   CodeBuffer::CallParams parms;
   m_cb.readCallParams(parms);
   if (label) parms.label = label;  // call by reference
   if (parms.label->isBuiltIn(bypassInternals)) {
      Arguments args(m_stack, parms.presenceBits, m_top);
      if (parms.ct == CT_SUBROUTINE) {
         --m_top;
      }
      BuiltInFunction func = parms.label->getBuiltInFunction();
      if (BuiltIn::isLoadBuiltIn(func)) {
         loadFile(args);    // home made.  don't disturb builtin.
      }else {
         (m_builtIn.*func)(args);
      }
      if (parms.ct == CT_SUBROUTINE) {
         m_varhdlr.setRESULT(args.getResult());
         if (m_tracer.isTracingIntermediates()) {
            m_tracer.traceFunction(args.getResult());
         }
      }else {
         if (m_tracer.isTracingIntermediates()) {
            m_tracer.traceFunction(*m_stack[m_top]);
         }
      }

   }else if (parms.label->isExternal(bypassInternals)) {
      RexxString cmd = Arguments::catenateArgs(
         parms.label->key(),
         m_stack,
         parms.presenceBits,
         m_top
      );
      execCommand(
         cmd,
         DataQueueMgr::NONE,
         DataQueueMgr::FIFO,
         *m_stack[m_top]
      );
      if (m_tracer.isTracingIntermediates()) {
         m_tracer.traceFunction(*m_stack[m_top]);
      }

   }else {                    // Internal routine (function, or subroutine)
      startInternal(parms);
   }
}

/*-------------------------------------------------Interpreter::startInternal-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Interpreter::startInternal(CodeBuffer::CallParams & parms)
{
   m_varhdlr.setSIGL(parms.sigl);
   m_tracer.startRoutine();
   m_routines.startRoutine(
      parms.ct,
      m_cb,
      m_top,
      m_stack,
      parms.presenceBits
   );
   m_cb.jumpTo(parms.label->getPos());
   /*
   | Handle I_PROC here, so to make sure no instructions may
   | be executed before (error 17).  Because more than one label
   | is allowed, loop over I_CLAUSE.
   */
// @999
   while (
      (m_cb.peekOpCode() == I_CLAUSE) ||
      (m_cb.peekOpCode() == I_LABEL)
   ) {
      m_cb.readOpCode();
      processTraceClause(0, 0);
   }
   if (m_cb.peekOpCode() == I_PROC) {
      int exposed;
      m_cb.readOpCode();                 // skip I_PROC
      m_varhdlr.pushScope();
      m_routines.m_isProcedure = true;   // remember to pop it
      exposed = m_cb.readByte();         // exposed variables
      while (exposed-- > 0) {
         m_varhdlr.expose((Symbol *)m_cb.readPointer());
      }
   }
}

/*---------------------------------------------------Interpreter::endInternal-+
| Restore arguments after returning from a subroutine or function             |
+----------------------------------------------------------------------------*/
void Interpreter::endInternal()
{
   if (m_routines.m_isProcedure) m_varhdlr.popScope();
   m_routines.endRoutine(m_cb, m_top);
   m_tracer.endRoutine();
}

/*------------------------------------------------Interpreter::startInterpret-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Interpreter::startInterpret(bool isInteractiveTrace)
{
   CodePosition endPos = m_cb.getEndPos();        // SAVE/RESTORE
   m_tracer.startInterpret(isInteractiveTrace);
   m_routines.startInterpret(
      m_cb,
      m_top,
      m_top - 1,
      isInteractiveTrace
   );
   RexxString & source = *m_stack[m_top];   // GCC needs this.
   MemStream input(source, source.length(), MemStream::Constant);
   OnTimeCompiler(input, m_cb, m_mapSym, m_mapLabel, m_erh).run();
   m_cb.jumpTo(endPos);

}

/*--------------------------------------------------Interpreter::endInterpret-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Interpreter::endInterpret()
{
   m_routines.endInterpret(m_cb, m_top);
   m_tracer.endInterpret();
}

/*------------------------------------------------------Interpreter::loadFile-+
| The built-in LOAD is an exception, as it may modify the Clauses, the Code   |
| buffer, the file list.   It is then processed directly by the Interpreter,  |
| which is the only one entitled to modify these fields.                      |
|                                                                             |
| LOAD(filename)                                                              |
|       load a rexx file so it can be used as a library                       |
|       returns a return code from loadfile                                   |
|         "-1" when file is already loaded                                    |
|          "0" on success                                                     |
|          "1" on error opening the file                                      |
+----------------------------------------------------------------------------*/
void Interpreter::loadFile(Arguments & args)
{
   if (args.getCount() != 1) {
      m_erh << ECE__ERROR << _REX__40_0 << endm;
   }else {
      RexxString & argr = args.getResult();
      RexxString & path = args.getArg(0);
      istream * pInput = m_clauses.makeNewInput(path);
      if (!pInput) {               // already loaded!
         argr = RexxString(0);     // this is OK
      }else {
         CodePosition posCur = m_cb.getRunPos();
         argr = RexxString(1);     // assume an error occured
         BatchCompiler cpl(
            *pInput,
            path,
            m_clauses,
            m_cb,
            m_mapSym,
            m_mapLabel,
            m_erh
         );
         cpl.run();
         argr = RexxString(0);  // OK
         m_cb.jumpTo(posCur);

         // >>>JAL TODO stacked m_routines may have code buffer positions
         //          that need to be (if they can be?) updated
         //          when code is loaded with CALL Load().
         //          this may be the place to fix it, e.g., bump the
         //          code position in all the stacked routines
         //          so that endInterpret() doesn't truncate this
         //          loaded code!
      }
   }
}

/*---------------------------------------------------Interpreter::getProperty-+
|                                                                             |
+----------------------------------------------------------------------------*/
RexxString Interpreter::getProperty(Property prop) const
{
   switch (prop) {
   case PROP_ENVIRONMENT:
      return m_routines.getEnvironment();
   case PROP_DIGITS:
      return RexxString(m_routines.getDecRexxContext().getDigits());
   case PROP_FUZZ:
      return RexxString(m_routines.getDecRexxContext().getFuzz());
   case PROP_FORM:
      return m_routines.getForm();
   case PROP_VERSION:
      return getYaxxVersion();
   case PROP_SOURCE:
      {
         RexxString temp = STR_OPERATING_SYSTEM;
         temp.bconcat(STR_OPERATING_SYSTEM, m_routines.getCallType());
         temp.bconcat(temp, m_clauses.getMainFileName());
         temp.bconcat(temp, m_clauses.getProgramName());
         return temp;
      }
   default:
      assert (false);
      return RexxString::Nil;
   }
}

/*---------------------------------------------------Interpreter::setProperty-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Interpreter::setProperty(Property prop, RexxString & value)
{
   switch (prop) {
   case PROP_ENVIRONMENT:
      m_routines.setEnvironment(value);
      break;

   case PROP_TRACE:
      if (!m_tracer.setTraceSetting(value, false)) {
         m_erh << ECE__ERROR << _REX__24_1 << value << endm;
      }
      /*
      | Update / initialize the m_loc member field:
      | 1) Intermediates are traced in the main loop
      | -- because they don't occur at a clause boundary,
      | 2) As soon as a TRACE instruction occurs, we freeze,
      | not having even entered the clause that started it.
      | => it is very likely that processing the next p-code
      | will end in calling a traceOperation...
      | Note that a TRACE Built-in does not need this, because
      | a clause stop will intervene.
      */
      if (m_tracer.isTracingClausesOrLabels()) {
         m_clauses.locate(m_curClauseNo, m_routines.getDepth(), m_loc);
      }
      break;

   case PROP_DIGITS:
      if (value.length() == 0) {
         m_routines.setDefaultDigits();
      }else {
         m_routines.setDigits(value);
      }
      break;

   case PROP_FUZZ:
      if (value.length() == 0) {
         m_routines.setDefaultFuzz();
      }else {
         m_routines.setFuzz(value);
      }
      break;

   case PROP_FORM:
      m_routines.setForm(value);
      break;

   case PROP_SET_SIGNAL:
   case PROP_SET_CALLON:
      m_routines.setDefaultTrapper(value, prop == PROP_SET_SIGNAL);
      break;

   case PROP_SET_SIGNAL_NAME:
   case PROP_SET_CALLON_NAME:
      m_routines.setTrapper(
         value,
         *m_stack[m_top-1],
         prop == PROP_SET_SIGNAL_NAME
      );
      break;

   case PROP_UNSET_SIGNAL:
   case PROP_UNSET_CALLON:
      m_routines.unsetTrapper(value, prop == PROP_UNSET_SIGNAL);
      break;

   default:
      assert (false);
   }
}

/*------------------------------------------------Interpreter::processCommand-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Interpreter::processCommand(RexxString & cmd, RexxString & env)
{
   DataQueueMgr::QueueMode qmodeIn;
   DataQueueMgr::QueueMode qmodeOut;
   int rc;

   if (!env.length() || (env == STR_ANY_SHELL) || (env == STR_DEFAULT_SHELL)) {
      cmd = DataQueueMgr::stripQueueModes(cmd, qmodeIn, qmodeOut);
      rc = execCommand(cmd, qmodeIn, qmodeOut, RexxString::Nil);
   }else {
      rc = -3;
   }

   m_varhdlr.setRC(rc);
   if (rc) {
// @999  let's update m_loc first!  (but, be clever: I shouldn't do it ALWAYS)
      m_clauses.locate(m_curClauseNo, m_routines.getDepth(), m_loc);
      m_tracer.traceBadRc(rc);
      if (m_tracer.isMyselfInteractive()) {
         processInteractiveTrace();
      }
      m_routines.raise(SIG_ERROR);
   }
}

/*---------------------------------------Interpreter::processInteractiveTrace-+
|                                                                             |
+----------------------------------------------------------------------------*/
char Interpreter::processInteractiveTrace()
{
   /* Read the interactive string into a tmp var */
   ++m_top;
   *(m_stack[m_top] = m_tmpStack + m_top) = IOMgr::readLine();

   switch (m_stack[m_top]->length()) {
   case 0:   // null line
      --m_top;
      return 0;
   case 1:   // one character
      if ('=' == m_stack[m_top]->charAt(0)) {
         --m_top;
         return '=';
      }
      // fall thru
   default:
      startInterpret(true);    // make it quiet
      return '*';
   }
}

/*--------------------------------------------Interpreter::processTraceClause-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Interpreter::processTraceClause(RexxString ** ppStrCreated, int iMaxCreated)
{
   if (m_routines.hasPendingConds()) {
      m_cb.unreadOpCode();
      m_routines.raisePendingConds();
   }
   if (!m_tracer.isTracingClausesOrLabels()) {
      m_prevClauseNo = m_curClauseNo;
      m_curClauseNo = m_cb.readWord();
   }else {
      m_cb.unreadOpCode();
      if (m_tracer.isTracingResults()) {
         m_tracer.traceResults(ppStrCreated, iMaxCreated);
      }
      if (
         m_tracer.mustPause() &&
         ppStrCreated       // TEMPORARY: for mimic'ing precedent behaviour
// @999                       && fix_me_please
      ) {
         switch (processInteractiveTrace()) {
         case '*':         // User-enterered string and interpret started.
            return;        // stop back on this clause.
         case '=':
         default:
            break;
         }
      }
      /*
      | Either:
      | - there is no interactive tracing; or,
      | - the user entered a null line while in interactive tracing; or,
      | - current line didn't pause
      */
      bool isNotLabelClause = (m_cb.peekOpCode() == I_CLAUSE);
      do {
         m_cb.readOpCode();
         m_prevClauseNo = m_curClauseNo;
         m_curClauseNo = m_cb.readWord();
         m_clauses.locate(m_curClauseNo, m_routines.getDepth(), m_loc);
         if (isNotLabelClause) {
            m_tracer.traceNotLabelClause();
         }else {
            m_tracer.traceClause();
         }
      }while (
         (isNotLabelClause = (m_cb.peekOpCode() == I_CLAUSE)) ||
         (m_cb.peekOpCode() == I_LABEL)
      );
   }
}

/*---------------------------------------------------Interpreter::execCommand-+
|                                                                             |
+----------------------------------------------------------------------------*/
int Interpreter::execCommand(
   RexxString & cmd,
   DataQueueMgr::QueueMode qmodeIn,
   DataQueueMgr::QueueMode qmodeOut,
   RexxString & res
) {
   if (cmd.length() == 0) return 0;  // the empty string is a valid command

#if defined MWERKS_NEEDS_ADDR_FIX         // basic io, system
   m_erh << ECE__ERROR << _REX__43_1 << cmd << endm;
   return 0;

#else
   char pathIn[300], pathOut[300];
   int  rc;
   int  oldStdin, oldStdout;
   int  newStdin, newStdout;

   /* --- redirect input --- */
   if (qmodeIn != DataQueueMgr::NONE) {
      StdFileStreamBuf sbNewCin;
      if (sbNewCin.open(
            StdFileStreamBuf::makeTempPath(pathIn, "in"),
            StdFileStreamBuf::OPEN_WRITE_FLAGS
         )
      ) {
         m_dqm.readDataQueue(sbNewCin, qmodeIn);
         sbNewCin.close();
         oldStdin = dup(0);
         newStdin = open(pathIn, S_IREAD);
         dup2(newStdin, 0);
         close(newStdin);
      }else {
         qmodeIn = DataQueueMgr::NONE;
      }
   }

   /* --- redirect output --- */
   if (qmodeOut != DataQueueMgr::NONE) {
      oldStdout = dup(1);
      newStdout = creat(
         StdFileStreamBuf::makeTempPath(pathOut, "out"), S_IWRITE
      );
      dup2(newStdout, 1);
      close(newStdout);
   }

   /* --- Execute the command --- */
   rc = SystemContext::system(cmd);

   /* --- restore input --- */
   if (qmodeIn != DataQueueMgr::NONE) {
      close(0);
      dup2(oldStdin, 0);
      close(oldStdin);
      chmod(pathIn, 0666);
      remove(pathIn);
   }

   /* --- restore output --- */
   if (qmodeOut != DataQueueMgr::NONE) {
      StdFileStreamBuf sbNewCout;
      close(1);
      dup2(oldStdout, 1);
      close(oldStdout);
      chmod(pathOut, 0666);
      if (sbNewCout.open(pathOut, StdFileStreamBuf::OPEN_READ_FLAGS)) {
         if (res.length()) {
            StringBuffer buf;
            buf.append(sbNewCout);
            res = RexxString(buf);
         }else  {
            m_dqm.writeDataQueue(sbNewCout, qmodeOut);
         }
      }
      sbNewCout.close();
      remove(pathOut);
   }
   return rc;
#endif
}

/*------------------------------------------------Interpreter::getYaxxVersion-+
|                                                                             |
+----------------------------------------------------------------------------*/
RexxString Interpreter::getYaxxVersion()
{
   // see the Rexx programming language p.58
   return RexxString(XSTR(VERSION));
}

/*---------------------------------------------Interpreter::handleSyntaxError-+
| Syntax Error during interpretation.                                         |
+----------------------------------------------------------------------------*/
void Interpreter::handleSyntaxError(UnicodeComposer::Message & msg)
{
   if ((msg.inqSeverity() >= ECE__ERROR) && isRxMessage(msg.inqStringId())) {
// if (msg.inqSeverity() >= ECE__ERROR) {
      if (!m_routines.canRecoverFromSyntax()) {
         m_clauses.locate(
           m_curClauseNo, m_routines.getDepth(), m_loc
         );
         m_tracer.traceError(msg, m_loc);
         throw FatalException(getRxMainCodeNo(msg.inqStringId()));
      }else {
         throw RecoverableException(::getRxMainId(msg.inqStringId()));
      }
   }else {
      SystemContext::cerr() << msg.stringize() << std::endl;
   }
}

/*---------------------------------Interpreter::SyntaxWatcher::notifyNewEntry-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool Interpreter::SyntaxWatcher::notifyNewEntry(UnicodeComposer::Message & msg)
{
   m_interpreter.handleSyntaxError(msg);
   return true;
}

#ifdef YAXX_NAMESPACE
}
#endif
/*===========================================================================*/
