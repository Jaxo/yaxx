/* $Id: Compiler.cpp,v 1.206 2002-10-24 14:33:15 pgr Exp $ */

#include "Arguments.h"
#include "Symbol.h"
#include "Clauses.h"
#include "Label.h"
#include "Tracer.h"
#include "Compiler.h"

#ifdef YAXX_NAMESPACE
namespace YAXX_NAMESPACE {
#endif

#if defined _WIN32
#pragma warning(disable:4355)
#endif

/*---------------------------------------------------------Compiler::Compiler-+
| Regular Constructor                                                         |
+----------------------------------------------------------------------------*/
Compiler::Compiler(
   istream & input,
   CodeBuffer & cb,
   SymbolMap & mapSym,
   LabelMap & mapLabel,
   UnicodeComposer & erh
) :
   Tokenizer(*input.rdbuf(), erh),
   BinarySearch((int)_KWD_NumberOf),
   m_cb(cb),
   m_mapSym(mapSym),
   m_mapLabel(mapLabel)
{
}

/*--------------------------------------------------------------Compiler::run-+
| Main loop                                                                   |
+----------------------------------------------------------------------------*/
void Compiler::run()
{
   m_nesting = 0;
   try {
      nextToken();
      do nextClause(); while (token != TK_EOF);
      m_cb.addOp(I_EOF);
      onEndOfFile(getLastPos(), getLineNo());
   }catch (RexxString::Exception e) {
      m_erh << ECE__ERROR << makeMsgTemplateId(e.getReason()) << endm;
   }
}

/*--------------------------------------------------------Compiler::~Compiler-+
|                                                                             |
+----------------------------------------------------------------------------*/
Compiler::~Compiler() {
}

/*-------------------------------------------------------Compiler::nextClause-+
| (blue book, p 26)                                                           |
| Clauses may be subdivided into five groups: Nulls, Labels, Assignments,     |
| Instruction, and Commands                                                   |
+----------------------------------------------------------------------------*/
void Compiler::nextClause()
{
   ++m_nesting;
loop:
   switch (token) {
   case TK_SEMICOLON:       // NULL clause
      nextToken();
      goto loop;            // a Null clause is completely ignored by REXX

   case TK_EOF:
      break;

   case TK_LABEL:           // Label clause: process here
      if (m_nesting != 1) m_erh << ECE__ERROR << _REX__47_0 << endm;
      m_mapLabel.access(getLastTerm(), Label::LABEL, m_cb.getEndPos());
      if (!onNewLabel()) {
         m_erh << ECE__ERROR << _REX__47_1 << getLastTerm() << endm;
      }
      if ((nextToken() == TK_SIMPLE_SYMBOL) && (termEquals(_KWD__PROCEDURE))) {
         instrProcedure();
         nextToken();
      }
      break;

   case TK_END:             // End is special: cannot be a command!
      m_erh << ECE__ERROR << _REX__10_1 << endm;
      break;

   default:                 // Assignments, Instructions, or Commands
      onNewClause();
      if (tryAssign(RSVD_NONE)) break;
      if (token == TK_SIMPLE_SYMBOL) {
         switch (search()) {
         case _KWD__ADDRESS:     instrAddress();        break;
         case _KWD__ARG:         instrArg();            break;
         case _KWD__CALL:        instrCall();           break;
         case _KWD__DO:          instrDo();             break;
         case _KWD__DROP:        instrDrop();           break;
         case _KWD__ELSE:        instrElseBad();        break;
         case _KWD__END:         instrEnd();            break;
         case _KWD__EXIT:        instrExit();           break;
         case _KWD__IF:          instrIf();             break;
         case _KWD__INTERPRET:   instrInterpret();      break;
         case _KWD__ITERATE:     instrIterate();        break;
         case _KWD__LEAVE:       instrLeave();          break;
         case _KWD__NOP:         instrNop();            break;
         case _KWD__NUMERIC:     instrNumeric();        break;
         case _KWD__OPTIONS:     instrOptions();        break;
         case _KWD__OTHERWISE:   instrOtherwiseBad();   break;
         case _KWD__PARSE:       instrParse();          break;
         case _KWD__PROCEDURE:   instrProcedureBad();   break;
         case _KWD__PULL:        instrPull();           break;
         case _KWD__PUSH:        instrPush();           break;
         case _KWD__QUEUE:       instrQueue();          break;
         case _KWD__RETURN:      instrReturn();         break;
         case _KWD__SAY:         instrSay();            break;
         case _KWD__SELECT:      instrSelect();         break;
         case _KWD__SIGNAL:      instrSignal();         break;
         case _KWD__THEN:        instrThenBad();        break;
         case _KWD__TRACE:       instrTrace();          break;
         case _KWD__WHEN:        instrWhenBad();        break;
         default:                processHostCmd();      break;
         }
      }else {
         processHostCmd();
         break;
      }
   }
   --m_nesting;
}

/*----------------------------------------------------------Compiler::compare-+
| Overload of the BinarySearch base class                                     |
+----------------------------------------------------------------------------*/
int Compiler::compare(int ix) const {
   return termCompares((KeywordId)ix);
}

/*--------------------------------------------------------Compiler::addClause-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Compiler::addClause(Clauses & clauses) {
   m_cb.addOpWord(
      I_CLAUSE,
      clauses.addClause(getLineNo(), m_nesting, getLastPos())
   );
}

/*---------------------------------------------------------Compiler::addLabel-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Compiler::addLabel(Clauses & clauses) {
   m_cb.addOpWord(
      I_LABEL,
      clauses.addClause(getLineNo(), m_nesting, getLastPos())
   );
}

/*---------------------------------------------------Compiler::stringizeToken-+
| Make a human readable value of the current token.                           |
+----------------------------------------------------------------------------*/
RexxString const Compiler::stringizeToken() const
{
   if (token == TK_EOF) {
      return "[EOF]";
   }else if (token & TK_OPERATOR) {
      return stringizeOp(token);
   }else {
      return getLastTerm();
   }
}

/*-----------------------------------------------------Compiler::instrAddress-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Compiler::instrAddress()
{
   switch (nextToken()) {
   case TK_SEMICOLON:
      m_cb.addOpString(I_PUSH, STR_EMPTY);
      m_cb.addOpByte(I_SET_PROP, PROP_ENVIRONMENT);
      return;
   case TK_LEFTPAR:
      parseExpression();
      m_cb.addOpByte(I_SET_PROP, PROP_ENVIRONMENT);
      return;
   case TK_SIMPLE_SYMBOL:
      if (termEquals(_KWD__VALUE)) {
         nextToken();
         parseExpression();
         m_cb.addOpByte(I_SET_PROP, PROP_ENVIRONMENT);
         return;
      }
      break;
   default:
      if (0 == (token & TK_SYMBOL)) {
         m_erh << ECE__ERROR << _REX__35_0 << endm;
      }
      break;
   }
   m_cb.addOpString(I_PUSHLIT, m_mapSym.put(getLastTerm(), token)->key());
   if (nextToken() != TK_SEMICOLON) {
      parseExpression();
      m_cb.addOp(I_SYSTEM);
   }else {
      m_cb.addOpByte(I_SET_PROP, PROP_ENVIRONMENT);
   }
}

/*---------------------------------------------------------Compiler::instrArg-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Compiler::instrArg()
{
   unsigned char ai = 0;
   do {
      m_cb.addOpByte(I_LOAD_ARG, ai);
      m_cb.addOp(I_COPY2TMP);
      m_cb.addOp(I_UPPER);
      processTemplate();
      ++ai;
   }while (token == TK_COMMA);
}

/*--------------------------------------------------------Compiler::instrCall-+
| call   := 'CALL' (                                                          |
|               callon_spec                                                   |
|             | (taken_constant | '(' value ')' | Msg19.2) [expression_list]  |
|           )                                                                 |
|     (value) is a YAXX extension.                                            |
+----------------------------------------------------------------------------*/
void Compiler::instrCall()
{
   if (((nextToken() & TK_SYMBOL) == 0) && (token != TK_LEFTPAR)) {
      m_erh << ECE__ERROR << _REX__19_2 << stringizeToken() << endm;
   }
   bool isOn;
   if ((isOn = (termEquals(_KWD__ON))) || (termEquals(_KWD__OFF))) {
      handleCondition(false, isOn);
   }else {
      // add room to the stack for the result string
      m_cb.addOp(I_PUSHTMP);
      parseRoutine(CT_SUBROUTINE, RSVD_NONE);
   }
}

/*------------------------------------------------------------handleCondition-+
| callon_spec := 'ON' (callable_condition | Msg25.1)                          |
|                  ['NAME' (taken_constant | Msg19.3)]                        |
|               | 'OFF' (callable_condition | Msg25.2)                        |
| signal_spec := 'ON' (condition | Msg25.3)                                   |
|                  ['NAME' (taken_constant | Msg19.3)]                        |
|               | 'OFF' (condition | Msg25.4)                                 |
|                                                                             |
| condition := callable_condition | 'NOVALUE' | 'SYNTAX' | 'LOSTDIGITS'       |
| callable_condition:= 'ERROR' | 'FAILURE' | 'HALT' | 'NOTREADY'              |
+----------------------------------------------------------------------------*/
void Compiler::handleCondition(bool isSignalStmt, bool isOn)
{
   unsigned char cnd;
   RexxString * pStr = 0;
   if (nextToken() == TK_SIMPLE_SYMBOL) {
      if (termEquals(_KWD__ERROR)) {
         pStr = &STR_ERROR;
      }else if (termEquals(_KWD__FAILURE)) {
         pStr = &STR_FAILURE;
      }else if (termEquals(_KWD__HALT)) {
         pStr = &STR_HALT;
      }else if (termEquals(_KWD__NOTREADY)) {
         pStr = &STR_NOTREADY;
      }else if (isSignalStmt) {
         if (termEquals(_KWD__NOVALUE)) {
            pStr = &STR_NOVALUE;
         }else if (termEquals(_KWD__SYNTAX)) {
            pStr = &STR_SYNTAX;
         }else if (termEquals(_KWD__LOSTDIGITS)) {
            pStr = &STR_LOSTDIGITS;
         }
      }
   }
   if (pStr) {
      m_mapLabel.access(*pStr, Label::LABEL, CodeBuffer::Unknown);
      nextToken();
      if (isSignalStmt) {
         cnd = isOn? PROP_SET_SIGNAL : PROP_UNSET_SIGNAL;
      }else {
         cnd = isOn? PROP_SET_CALLON : PROP_UNSET_CALLON;
      }
   }else {
      MsgTemplateId stringId;
      if (!isSignalStmt) {
         stringId = isOn? _REX__25_1 : _REX__25_2;
      }else {
         stringId = isOn? _REX__25_3 : _REX__25_4;
      }
      m_erh << ECE__ERROR << stringId << stringizeToken() << endm;
   }
   // NAME argument?
   if ((token == TK_SIMPLE_SYMBOL) && (termEquals(_KWD__NAME))) {
      if (0 == (nextToken() & TK_VARIABLE_SYMBOL)) {
         m_erh << ECE__ERROR << _REX__19_3 << stringizeToken() << endm;
      }
      if (isOn) {
         m_cb.addOpString(I_PUSH, m_mapSym.put(getLastTerm(), token)->key());
         if (isSignalStmt) {
            cnd = PROP_SET_SIGNAL_NAME;
         }else {
            cnd = PROP_SET_CALLON_NAME;
         }
      }
      nextToken();
   }
   m_cb.addOpString(I_PUSH, *pStr);
   m_cb.addOpByte(I_SET_PROP, cnd);

   if (cnd == PROP_SET_SIGNAL_NAME) {
      m_cb.addOpByte(I_POP, 1);
   }
}


/*-----------------------------------------------Compiler::LoopCtrl::LoopCtrl-+
|                                                                             |
+----------------------------------------------------------------------------*/
Compiler::LoopCtrl::LoopCtrl(
   CodePosition iterate,
   CodePosition leave,
   int iVarCount,
   RexxString & strControl
) :
   m_strControl(strControl)
{
   m_iterate = iterate;
   m_leave = leave;
   m_iVarCount = iVarCount;
}

/*----------------------------------------------------------Compiler::instrDo-+
| do               := do_specification (ncl | Msg21.1 | Msg27.1)              |
|                  [instruction_list] do_ending                               |
| do_ending        := 'END' [VAR_SYMBOL] ncl                                  |
|                  | EOS Msg14.1 | Msg35.1                                    |
| do_specification := do_simple | do_repetitive                               |
|   do_simple      := 'DO'                                                    |
|   do_repetitive  := 'DO' dorep | 'DO' docond                                |
|                  | 'DO' dorep docond                                        |
|                  | 'DO' 'FOREVER' [docond | Msg25.16]                       |
|   docond         := 'WHILE' whileexpr | 'UNTIL' untilexpr                   |
|     untilexpr    := expression                                              |
|     whileexpr    := expression                                              |
|   dorep          := assignment [docount] | repexpr                          |
|     repexpr      := expression                                              |
|     docount      := dot [dobf] | dob [dotf] | dof [dotb]                    |
|       dobf       := dob [dof] | dof [dob]                                   |
|       dotf       := dot [dof] | dof [dot]                                   |
|       dotb       := dot [dob] | dob [dot]                                   |
|       dot        := 'TO' toexpr                                             |
|         toexpr   := expression                                              |
|       dob        := 'BY' byexpr                                             |
|         byexpr   := expression                                              |
|       dof        := 'FOR' forexpr                                           |
|         forexpr  := expression                                              |
+----------------------------------------------------------------------------*/
void Compiler::instrDo()
{
   LoopCtrl * lc;
   RexxString strControl;
   Symbol * symControl;
   CodePosition posBody;
   CodePosition posIterate;
   CodePosition posLeave;
   CodePosition posUntil = CodeBuffer::Unknown;
   CodePosition posEndUntil;
   unsigned char idx=0;
   unsigned char idxTO=0;
   unsigned char idxBY=0;
   unsigned char idxFOR=0;
   bool invalidSyntax = false;

   nextToken(RSVD_WITHIN_DO);
   if (token == TK_SEMICOLON) {            // Simple DO; ... END */
      do {
         nextClause();
         if (token == TK_EOF) m_erh << ECE__ERROR << _REX__14_1 << endm;
      }while (token != TK_END);
      nextToken();                         // Skip "END"
      return;
   }

   // Repetitor
   symControl = tryAssign(RSVD_WITHIN_DO);
   if (symControl) {                       // name=expri
      strControl = symControl->key();
      m_cb.addOp(I_DO_CTL_INIT);
      do {
         switch (token) {
         case TK_TO:
            if (idxTO) {
               invalidSyntax = true;
            }else {
               idxTO = ++idx;
               nextToken(RSVD_WITHIN_DO);  // Skip TO
               parseExpression(RSVD_WITHIN_DO);
               m_cb.addOp(I_DO_TO_INIT);
            }
            continue;
         case TK_BY:
            if (idxBY) {
               invalidSyntax = true;
            }else {
               idxBY = ++idx;
               nextToken(RSVD_WITHIN_DO);  // Skip BY
               parseExpression(RSVD_WITHIN_DO);
               m_cb.addOp(I_DO_BY_INIT);
            }
            continue;
         case TK_FOR:
            if (idxFOR) {
               invalidSyntax = true;
            }else {
               idxFOR = ++idx;
               nextToken(RSVD_WITHIN_DO);  // Skip FOR
               parseExpression(RSVD_WITHIN_DO);
               m_cb.addOp(I_DO_FOR_INIT);
            }
            continue;
         default:
            break;                         // Done!
         }
         break;
      }while (!invalidSyntax);


   }else if (termEquals(_KWD__FOREVER)) {
      if (token != TK_SIMPLE_SYMBOL) {
         invalidSyntax = true;
      }else {
         nextToken(RSVD_WITHIN_DO);        // Skip FOREVER
      }

   }else if ((token != TK_WHILE) && (token != TK_UNTIL)) { // exprr
      idxFOR = ++idx;
      parseExpression(RSVD_WITHIN_DO);
      m_cb.addOp(I_DO_FOR_INIT);
   }

   if (invalidSyntax) {
      m_erh << ECE__ERROR << _REX__27_1 << stringizeToken() << endm;
   }

   //          JMP body
   // leave:   JMP ???
   // iterate: JMP ???   ; only if DO_ASSIGN
   // body:    ...
   posBody = m_cb.addOpNoPos(I_JMP_ALWAYS);      // use body as a temp...
   posLeave = m_cb.addOpNoPos(I_JMP_ALWAYS);
   if (symControl) {
      posIterate = m_cb.addOpNoPos(I_JMP_ALWAYS);
   }
   m_cb.resolvePos(posBody);
   posBody = m_cb.getEndPos();             // the real body

   /* Create the main loop control */
   if (symControl) {
      lc = new LoopCtrl(posIterate, posLeave, idx, strControl);
   }else {
      lc = new LoopCtrl(posBody, posLeave, idx, strControl);
   }
   m_stkLoopCtrl.addLast(lc);

   if (token == TK_WHILE) {
      nextToken(RSVD_WITHIN_DO);           // Skip WHILE
      parseExpression(RSVD_WITHIN_DO);
      m_cb.addOpPos(I_JMP_FALSE, posLeave);

   }else if (token == TK_UNTIL) {
      nextToken(RSVD_WITHIN_DO);           // Skip UNTIL
      CodePosition posTemp = m_cb.addOpNoPos(I_JMP_ALWAYS);
      posUntil = m_cb.getEndPos();

      // fix the iterate address to check the UNTIL expr after the iteration
      lc->m_iterate = posUntil;
      parseExpression(RSVD_WITHIN_DO);
      m_cb.addOpPos(I_JMP_TRUE, posLeave);
      posEndUntil = m_cb.addOpNoPos(I_JMP_ALWAYS);
      m_cb.resolvePos(posTemp);
   }

   if (token != TK_SEMICOLON) {
      m_erh << ECE__ERROR << _REX__21_1 << stringizeToken() << endm;
   }
   nextToken();              // Skip the required semicolon

   // tests for TO..(BY)? and FOR
   if (idxTO) {
      if (idxBY) {      // if BY is < 0 x<=TO, else x>=TO
         m_cb.addOpByteSymbolPosByte(
            I_DO_TO_BY_TEST,
            (unsigned char)(idx-idxTO),
            symControl,
            posLeave,
            idx-idxBY
         );
      }else {
         m_cb.addOpByteSymbolPos(
            I_DO_TO_TEST,
            (unsigned char)(idx-idxTO),
            symControl,
            posLeave
         );
      }
   }

   if (idxFOR) {                 // variable
      m_cb.addOpBytePos(I_DO_FOR_TEST, (unsigned char)(idx-idxFOR), posLeave);
   }

   /* process the body of the main loop */
   do {
      nextClause();
      if (token == TK_EOF) m_erh << ECE__ERROR << _REX__14_1 << endm;
   }while (token != TK_END);
   nextToken();              // Skip END

   // if END is followed by the name of the control variable
   if (symControl) {
      if (token & TK_VARIABLE_SYMBOL) {
          if (getLastTerm() == strControl) {
             nextToken();
          }else {                // see p44.
             m_erh << ECE__ERROR << _REX__10_2 << getLastTerm() << endm;
          }
      }else if ((token != TK_SEMICOLON) && (token != TK_EOF)) {
         m_erh << ECE__ERROR << _REX__20_1 << stringizeToken() << endm;
      }

   }else if (token & TK_VARIABLE_SYMBOL) {
      m_erh << ECE__ERROR << _REX__10_3 << getLastTerm() << endm;
   }

   /* if UNTIL in DO */
   if (posUntil != CodeBuffer::Unknown) {
      m_cb.addOpPos(I_JMP_ALWAYS, posUntil);
      m_cb.resolvePos(posEndUntil);
   }

   // step the control variable
   if (symControl) {
      m_cb.resolvePos(posIterate);
      if (idxBY) {
         m_cb.addOpByteSymbol(
            I_DO_BY_STEP, (unsigned char)(idx-idxBY), symControl
         );
      }else {
         m_cb.addOpSymbol(I_DO_STEP, symControl);    // default: BY is 1
      }
   }

   /* end of loop, add a jump to the beginning */
   m_cb.addOpPos(I_JMP_ALWAYS, posBody);
   m_cb.resolvePos(posLeave);
   m_stkLoopCtrl.removeLast();

   if (idx) {           /* pop the BY, TO, FOR values from the stack */
      m_cb.addOpByte(I_POP, idx);
   }
}

/*--------------------------------------------------------Compiler::instrDrop-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Compiler::instrDrop()
{
   for (;;) {
      if (nextToken() & TK_VARIABLE_SYMBOL) {
         m_cb.addOpSymbol(
            I_DROP_VAR,
            m_mapSym.put(getLastTerm(), token)
         );

      }else if (token == TK_LEFTPAR) {
         if (0 == (nextToken() & TK_VARIABLE_SYMBOL)) {
            m_erh << ECE__ERROR << _REX__20_1 << stringizeToken() << endm;
         }
         m_cb.addOpSymbol(
            I_LOAD_VAR,
            m_mapSym.put(getLastTerm(), token)
         );
         m_cb.addOp(I_COPY2TMP);
         m_cb.addOp(I_UPPER);
         if (nextToken() != TK_RIGHTPAR) {
            m_erh << ECE__ERROR << _REX__36_0 << endm;
         }
         m_cb.addOp(I_DROP_REF);

      }else {
         break;
      }
   }
}

/*-----------------------------------------------------Compiler::instrElseBad-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Compiler::instrElseBad() {
   m_erh << ECE__ERROR << _REX__8_2 << endm;
}

/*---------------------------------------------------------Compiler::instrEnd-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Compiler::instrEnd() {
   token = TK_END;
}

/*--------------------------------------------------------Compiler::instrExit-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Compiler::instrExit()
{
   if ((nextToken() == TK_SEMICOLON) || (token == TK_EOF)) {
      m_cb.addOp(I_EXIT_EMPTY);
   }else {
      parseExpression();
      m_cb.addOp(I_EXIT_RESULT);
   }
}

/*----------------------------------------------------------Compiler::instrIf-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Compiler::instrIf()
{
   CodePosition posNext;

   nextToken(RSVD_WITHIN_IF);
   parseExpression(RSVD_WITHIN_IF);
   while (token == TK_SEMICOLON) nextToken(RSVD_WITHIN_IF);
   posNext = m_cb.addOpNoPos(I_JMP_FALSE);
   onNewClause();
   if (token != TK_THEN) {
      m_erh << ECE__ERROR << _REX__18_1 << stringizeToken() << endm;
   }
   nextToken();               // Skip THEN
   if (token == TK_EOF) {
      m_erh << ECE__ERROR << _REX__14_3 << endm;
   }
   nextClause();

   while (token==TK_SEMICOLON) nextToken();
   if ((token == TK_SIMPLE_SYMBOL) && (termEquals(_KWD__ELSE))) {
      CodePosition posEnd = m_cb.addOpNoPos(I_JMP_ALWAYS);
      m_cb.resolvePos(posNext);
      onNewClause();
      nextToken();            // Skip ELSE
      if (token == TK_EOF) {
         m_erh << ECE__ERROR << _REX__14_4 << endm;
      }
      nextClause();
      posNext = posEnd;
   }
   m_cb.resolvePos(posNext);
}

/*---------------------------------------------------Compiler::instrInterpret-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Compiler::instrInterpret()
{
   nextToken();
   parseExpression();
   m_cb.addOp(I_INTERPRET);
}

/*-----------------------------------------------------Compiler::instrOptions-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Compiler::instrOptions()
{
   nextToken();
   parseExpression();
   m_cb.addOp(I_OPTIONS);
}

/*------------------------------------------------Compiler::instrOtherwiseBad-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Compiler::instrOtherwiseBad() {
   m_erh << ECE__ERROR << _REX__9_0 << endm;
}

/*------------------------------------------------Compiler::instrProcedureBad-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Compiler::instrProcedureBad() {
   m_erh << ECE__ERROR << _REX__17_0 << endm;
}

/*---------------------------------------------------Compiler::instrProcedure-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Compiler::instrProcedure()
{
   unsigned char exposed = 0;
   CodePosition pos;

   pos = m_cb.addOpNoByte(I_PROC);
   if ((nextToken() == TK_SIMPLE_SYMBOL) && (termEquals(_KWD__EXPOSE))) {
      while (nextToken() & TK_VARIABLE_SYMBOL) {
         m_cb.addPointer(m_mapSym.put(getLastTerm(), token));
         ++exposed;
      }
      m_cb.resolveByte(pos, exposed);
   }
   if ((token != TK_SEMICOLON) && (token != TK_EOF)) {
      m_erh << ECE__ERROR << _REX__21_1 << stringizeToken() << endm;
   }
}

/*-----------------------------------------------------Compiler::instrIterate-+
| Iterate the innermost loop, or the named controlled one.                    |
+----------------------------------------------------------------------------*/
void Compiler::instrIterate()
{
   if (!m_stkLoopCtrl.size()) {
      m_erh << ECE__ERROR << _REX__28_2 << endm;
   }else {
      LoopCtrl * lc = queryLoopCtrl();
      if (!lc) {
         m_erh << ECE__ERROR << _REX__28_4 << stringizeToken() << endm;
      }else {
         m_cb.addOpPos(I_JMP_ALWAYS, lc->m_iterate);
      }
   }
}

/*-------------------------------------------------------Compiler::instrLeave-+
| Leave the innermost loop, or the named controlled one.                      |
+----------------------------------------------------------------------------*/
void Compiler::instrLeave()
{
   if (!m_stkLoopCtrl.size()) {
      m_erh << ECE__ERROR << _REX__28_1 << endm;
   }else {
      LoopCtrl * lc = queryLoopCtrl();
      if (!lc) {
         m_erh << ECE__ERROR << _REX__28_3 << stringizeToken() << endm;
      }else {
         m_cb.addOpPos(I_JMP_ALWAYS, lc->m_leave);
      }
   }
}

/*----------------------------------------------------Compiler::queryLoopCtrl-+
|                                                                             |
+----------------------------------------------------------------------------*/
Compiler::LoopCtrl * Compiler::queryLoopCtrl()
{
   if (nextToken() & TK_VARIABLE_SYMBOL) {        // e.g.: leave k
      LoopCtrl *lc;
      RexxString strVarName = getLastTerm();
      unsigned char iVarCount = 0;
      LinkedList::Iterator it(m_stkLoopCtrl);
      for (;;) {
         lc = (LoopCtrl *)it.previous();
         if (!lc) return 0;
         if (lc->m_strControl.length()) {
            if (strVarName == lc->m_strControl) {  // see p52.
               break;
            }
            iVarCount += lc->m_iVarCount;
         }
      }
      nextToken();
      if (iVarCount) {
         m_cb.addOpByte(I_POP, iVarCount);
      }
      return lc;
   }else if (token == TK_SEMICOLON) {
      return (LoopCtrl *)m_stkLoopCtrl.getLast();
   }else {
      m_erh << ECE__ERROR << _REX__20_2 << stringizeToken() << endm;
      return 0;
   }
}

/*---------------------------------------------------------Compiler::instrNop-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Compiler::instrNop()
{
   nextToken();
   m_cb.addOp(I_NOP);
}

/*-----------------------------------------------------Compiler::instrNumeric-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Compiler::instrNumeric()
{
   nextToken();
   if (termEquals(_KWD__DIGITS)) {
       nextToken();
       parseExpression();
       m_cb.addOpByte(I_SET_PROP, PROP_DIGITS);

   }else if (termEquals(_KWD__FORM)) {
      RexxString *pStrKey = 0;
      if (nextToken() == TK_SEMICOLON) {
         pStrKey = &STR_SCIENTIFIC;
      }else if (token == TK_SIMPLE_SYMBOL) {
         if (termEquals(_KWD__SCIENTIFIC)) {
            nextToken();
            pStrKey = &STR_SCIENTIFIC;
         }else if (termEquals(_KWD__ENGINEERING)) {
            nextToken();
            pStrKey = &STR_ENGINEERING;
         }else if (termEquals(_KWD__VALUE)) {
            nextToken();
         }
      }
      if (!pStrKey) {
         parseExpression();
         m_cb.addOpByte(I_SET_PROP, PROP_FORM);
      }else {
         m_cb.addOpString(I_PUSH, *pStrKey);
         m_cb.addOpByte(I_SET_PROP, PROP_FORM);
      }

   }else if (termEquals(_KWD__FUZZ)) {
      nextToken();
      parseExpression();
      m_cb.addOpByte(I_SET_PROP, PROP_FUZZ);

   }else {
      m_erh << ECE__ERROR << _REX__25_15 << stringizeToken() << endm;
   }
}

/*-------------------------------------------------------Compiler::instrParse-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Compiler::instrParse()
{
   bool isToUpper = false;
   bool isOK = true;

   if (nextToken() != TK_SIMPLE_SYMBOL) {
      isOK = false;
   }else if (termEquals(_KWD__UPPER)) {
      isToUpper = true;
      if (nextToken() != TK_SIMPLE_SYMBOL) isOK = false;
   }

   if (isOK) {
      if (termEquals(_KWD__ARG)) {
         unsigned char ai = 0;
         do {
            m_cb.addOpByte(I_LOAD_ARG, ai);
            if (isToUpper) {
               m_cb.addOp(I_COPY2TMP);
               m_cb.addOp(I_UPPER);
            }
            processTemplate();
            ++ai;
         }while (token==TK_COMMA);
         return;
      }

      if (termEquals(_KWD__LINEIN)) {
         m_cb.addOp(I_STACK_LINEIN);

      }else if (termEquals(_KWD__PULL)) {
         m_cb.addOp(I_STACK_PULL);

      }else if (termEquals(_KWD__VAR)) {
         if (0 == (nextToken() & TK_VARIABLE_SYMBOL)) {
            m_erh << ECE__ERROR << _REX__38_0 << endm;
         }
         m_cb.addOpSymbol(
            I_LOAD_VAR,
            m_mapSym.put(getLastTerm(), token)
         );

      }else if (termEquals(_KWD__SOURCE)) {
         m_cb.addOpByte(I_GET_PROP, PROP_SOURCE);

      }else if (termEquals(_KWD__VALUE)) {
         nextToken(RSVD_WITHIN_PARSE);
         parseExpression(RSVD_WITHIN_PARSE);
         if (token != TK_WITH) {
            m_erh << ECE__ERROR << _REX__38_3 << stringizeToken() << endm;
         }

      }else if (termEquals(_KWD__VERSION)) {
         m_cb.addOpByte(I_GET_PROP, PROP_VERSION);

      }else {
         isOK = false;
      }

      if (isOK) {
         if (isToUpper) {
            m_cb.addOp(I_COPY2TMP);
            m_cb.addOp(I_UPPER);
         }
         processTemplate();
         return;
      }
   }
   MsgTemplateId stringId;
   if (isToUpper) {
      stringId = _REX__25_13;
   }else {
      stringId = _REX__25_12;
   }
   m_erh << ECE__ERROR << stringId << stringizeToken() << endm;
}

/*--------------------------------------------------------Compiler::instrPull-+
| Aka PARSE UPPER PULL                                                        |
+----------------------------------------------------------------------------*/
void Compiler::instrPull()
{
   m_cb.addOp(I_STACK_PULL);
   m_cb.addOp(I_UPPER);
   processTemplate();
}

/*--------------------------------------------------------Compiler::instrPush-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Compiler::instrPush()
{
   if (nextToken() == TK_SEMICOLON) {
      m_cb.addOpString(I_PUSH, STR_EMPTY);
   }else {
      parseExpression();
   }
   m_cb.addOp(I_STACK_PUSH);
}

/*-------------------------------------------------------Compiler::instrQueue-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Compiler::instrQueue()
{
   if (nextToken() == TK_SEMICOLON) {
      m_cb.addOpString(I_PUSH, STR_EMPTY);
   }else {
      parseExpression();
   }
   m_cb.addOp(I_STACK_QUEUE);
}

/*------------------------------------------------------Compiler::instrReturn-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Compiler::instrReturn()
{
   if ((nextToken() == TK_SEMICOLON || token == TK_EOF)) {
      m_cb.addOp(I_RETURN_EMPTY);
   }else {
      parseExpression();
      m_cb.addOp(I_RETURN_RESULT);
   }
}

/*---------------------------------------------------------Compiler::instrSay-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Compiler::instrSay()
{
   nextToken();
   parseExpression();
   m_cb.addOp(I_SAY);
}

/*------------------------------------------------------Compiler::instrSelect-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Compiler::instrSelect()
{
   CodePosition posNext;
   CodePosition posEnd;
   bool isWhenFound = false;
   bool isOwiseFound;
   int selLineno = getLineNo();

   if (nextToken() != TK_SEMICOLON) {
      m_erh << ECE__ERROR << _REX__21_1 << stringizeToken() << endm;
   }
   nextToken();

   posNext = m_cb.addOpNoPos(I_JMP_ALWAYS);  // jump to next when
   posEnd = m_cb.addOpNoPos(I_JMP_ALWAYS);   // jump if not

   ++m_nesting;
   for (;;) {
      while (token==TK_SEMICOLON) nextToken();
      if (
         (token != TK_SIMPLE_SYMBOL) || (
            (!isWhenFound  || (
                  isWhenFound = false,
                  (isOwiseFound=termEquals(_KWD__OTHERWISE), !isOwiseFound) &&
                  !termEquals(_KWD__END)
               )
            ) &&
            (isWhenFound=termEquals(_KWD__WHEN), !isWhenFound)
         )
      ) {
         m_erh
            << ECE__ERROR
            << (isWhenFound? _REX__7_2:_REX__7_1)
            << selLineno
            << stringizeToken()
            << endm;
      }
      if (!isWhenFound) break;
      m_cb.resolvePos(posNext);
      onNewClause();

      nextToken(RSVD_WITHIN_IF);
      parseExpression(RSVD_WITHIN_IF);
      while (token == TK_SEMICOLON) nextToken(RSVD_WITHIN_IF);
      posNext = m_cb.addOpNoPos(I_JMP_FALSE);
      onNewClause();
      if (token != TK_THEN) {
         m_erh << ECE__ERROR << _REX__18_1 << stringizeToken() << endm;
      }
      nextToken();              // Skip THEN
      nextClause();
      m_cb.addOpPos(I_JMP_ALWAYS, posEnd);
   }
   m_cb.resolvePos(posNext);
   onNewClause();
   if (!isOwiseFound) {
      m_cb.addOpByte(I_ERROR, 0x7);   //  _REX__7_0  - should be 7.3
   }else {                     // descend the clause list
      nextToken();             // Skip OTHERWISE
      do {
        nextClause();
        if (token == TK_EOF) m_erh << ECE__ERROR << _REX__14_2 << endm;
      }while (token != TK_END);
   }
   nextToken();
   if (token & TK_VARIABLE_SYMBOL) {
      m_erh
         << ECE__ERROR
         << _REX__10_4
         << selLineno
         << stringizeToken()
         << endm;
   }
   m_cb.resolvePos(posEnd);
   --m_nesting;
}

/*------------------------------------------------------Compiler::instrSignal-+
| signal := 'SIGNAL' (signal_spec | valueexp | taken_constant | Msg19.4)      |
+----------------------------------------------------------------------------*/
void Compiler::instrSignal()
{
   if (nextToken() & TK_VARIABLE_SYMBOL) {
      bool isOn;
      if ((isOn = (termEquals(_KWD__ON))) || (termEquals(_KWD__OFF))) {
         handleCondition(true, isOn);
      }else if (termEquals(_KWD__VALUE)) {
         nextToken();
         parseExpression();
         m_cb.addOp(I_JMP_VAL);
      }else {
         m_cb.addOpPtr(
            I_JMP_LBL,
            m_mapLabel.access(
               getLastTerm(),
               Label::LABEL,
               CodeBuffer::Unknown
            )
         );
         nextToken();
      }
   }else {
      m_erh << ECE__ERROR << _REX__19_4 << stringizeToken() << endm;
   }
}

/*-----------------------------------------------------Compiler::instrThenBad-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Compiler::instrThenBad() {
   m_erh << ECE__ERROR << _REX__8_1 << endm;
}

/*-------------------------------------------------------Compiler::instrTrace-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Compiler::instrTrace()
{
   if (nextToken() == TK_SEMICOLON) {
      m_cb.addOpString(I_PUSH, STR_NORMAL);
   }else if (token == TK_SIMPLE_SYMBOL) {
      if (termEquals(_KWD__VALUE)) {
         nextToken();
         parseExpression();
         m_cb.addOp(I_COPY2TMP);
      }else {
         m_cb.addOpString(I_PUSH, m_mapSym.put(getLastTerm(), token)->key());
         nextToken();
      }
   }else {
      parseExpression();
      m_cb.addOp(I_COPY2TMP);
   }
   m_cb.addOpByte(I_SET_PROP, PROP_TRACE);
}

/*-----------------------------------------------------Compiler::instrWhenBad-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Compiler::instrWhenBad() {
   m_erh << ECE__ERROR << _REX__9_0 << endm;
}

/*---------------------------------------------------Compiler::processHostCmd-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Compiler::processHostCmd()
{
   m_cb.addOpByte(I_GET_PROP, PROP_ENVIRONMENT);
   if (!parseExpression()) {
      // >>>PGR: FIXME (temp work around)
      /*
      | What happens is that the expression parser does not call nextToken for
      | a DOT_TK, or a COMMA_TK, then, we are stuck in a big infinite loop.
      | May be this can happen with other stuff?  not sure.
      | We have to check that the tokenization progressed!
      */
      m_erh << ECE__ERROR << _REX__35_0 << endm;
   }
   m_cb.addOp(I_SYSTEM);
}

/*--------------------------------------------------------Compiler::tryAssign-+
| Only VARIABLE_SYMBOL can be assigned, however for DO x=expr, the            |
| symbol x can be a reserved keyword.                                         |
+----------------------------------------------------------------------------*/
Symbol * Compiler::tryAssign(ReservedKeywords rsvd)
{
   if (!isAssigned()) {          // look-ahead
      return 0;
   }else if (0 == (token & (TK_VARIABLE_SYMBOL | TK_KEYWORD))) {
      MsgTemplateId id;
      switch (token) {
      case TK_NUMBER_CONSTANT_SYMBOL:
         id = _REX__31_1;
         break;
      case TK_OTHER_CONSTANT_SYMBOL:
         id = (stringizeToken().charAt(0) == '.')? _REX__31_3 : _REX__31_2;
         break;
      default:
         return 0;
      }
      m_erh << ECE__ERROR  << id << stringizeToken() << endm;
      return 0;
   }else {
      Symbol * var = m_mapSym.put(getLastTerm(), token);
      bool isStem = (token == TK_STEM_SYMBOL);

      nextToken(rsvd);
      assert (token == TK_NORMAL_EQ);

      m_cb.addOpPtr(I_CREATE_VAR, var);
      if (token != TK_NORMAL_EQ) {
         m_erh << ECE__ERROR << _REX__35_1 << stringizeToken() << endm;
      }
      nextToken(rsvd);
      parseAssignedExpression(rsvd);
      if (isStem) {
         m_cb.addOpPtr(I_SET_STEM, var);
      }
      return var;
   }
}

/*------------------------------------------------Compiler::handleVariableRef-+
| Variable reference - introduced by '(value)' in patterns or CALL (YAXX ext) |
+----------------------------------------------------------------------------*/
void Compiler::handleVariableRef()
{
   if (0 == (nextToken() & TK_VARIABLE_SYMBOL)) {
      m_erh << ECE__ERROR << _REX__19_7 << stringizeToken() << endm;
   }
   m_cb.addOpSymbol(
      I_LOAD_VAR,
      m_mapSym.put(getLastTerm(), token)
   );
   if (nextToken() != TK_RIGHTPAR) {
      m_erh << ECE__ERROR << _REX__46_1 << stringizeToken() << endm;
   }
}

/*---------------------------------------------------Compiler::getLastIntTerm-+
|                                                                             |
+----------------------------------------------------------------------------*/
int Compiler::getLastIntTerm() const {
   int whatInt;
   if (!getLastTerm().isInt(whatInt)) {
      m_erh << _REX__26_4 << getLastTerm() << endm;
   }
   return whatInt;
}
/*---------------------------------------------------------Compiler::position-+
| position := NUMBER | vrefp
+----------------------------------------------------------------------------*/
void Compiler::position()
{
   switch (nextToken()) {
   case TK_LEFTPAR:
      handleVariableRef();
      break;
   case TK_NUMBER_CONSTANT_SYMBOL:
      m_cb.addOpWord(I_IMM_VAL, getLastIntTerm());
      break;
   default:
      m_erh << ECE__ERROR << _REX__38_2 << stringizeToken() << endm;
      break;
   }
}

/*--------------------------------------------------Compiler::processTemplate-+
| [6.3.2.86]                                                                  |
|                                                                             |
| template_list := template | [template] ',' [template_list]                  |
| template   := (trigger | target | Msg38.1)+                                 |
| target     := VAR_SYMBOL | '.'                                              |
| trigger    := pattern | positional                                          |
| pattern    := STRING | vrefp                                                |
| vrefp      := '(' (VAR_SYMBOL | Msg19.7) (')' | Msg46.1)                    |
| positional := absolute_pos | relative_pos                                   |
| absolute_pos := NUMBER | '=' position                                       |
| position   := NUMBER | vrefp  | Msg38.2                                     |
| relative_pos := ('+' | '-') position                                        |
+----------------------------------------------------------------------------*/
void Compiler::processTemplate()
{
   m_cb.addOp(I_TEMPLATE_START);
   for (;;) {
      switch (nextToken()) {

      /*
      | End Of Template
      */
      case TK_SEMICOLON:
      case TK_COMMA:
      case TK_EOF:
         m_cb.addOp(I_TEMPLATE_END);
         return;

      /*
      | Target (not a trigger)
      */
      case TK_SIMPLE_SYMBOL:
      case TK_COMPOUND_SYMBOL:
      case TK_STEM_SYMBOL:
         m_cb.addOpPtr(I_TARGET_VAR, m_mapSym.put(getLastTerm(), token));
         continue;

      case TK_DOT:
         m_cb.addOp(I_TARGET_DOT);
         continue;

      /*
      | Positionals (Trigger)
      | A positional specifies where the relevant piece of the subject ends.
      | The adjustment can be given as a number or a variable in parentheses.
      */
      case TK_MINUS:
         m_cb.addOp(I_TRIG_MINUS_REL);  // I_IMM_VAL or I_LOAD_VAR follows
         position();
         break;

      case TK_PLUS:
         m_cb.addOp(I_TRIG_PLUS_REL);   // I_IMM_VAL or I_LOAD_VAR follows
         position();
         break;

      case TK_NORMAL_EQ:
         m_cb.addOp(I_TRIG_ABS);        // I_IMM_VAL or I_LOAD_VAR follows
         position();
         break;

      case TK_NUMBER_CONSTANT_SYMBOL:
         m_cb.addOp(I_TRIG_ABS);        // followed by I_IMM_VAL
         m_cb.addOpWord(I_IMM_VAL, getLastIntTerm());
         break;

      /*
      | Pattern (Trigger)
      */
      case TK_STRING_CONSTANT_SYMBOL:
         m_cb.addOp(I_TRIG_LIT);        // I_IMM_STRING or I_LOAD_VAR
         m_cb.addOpString(
            I_IMM_STRING,
            m_mapSym.put(getLastTerm(), token)->key()
         );
         break;

      case TK_LEFTPAR:
         m_cb.addOp(I_TRIG_LIT);        // followed by I_LOAD_VAR
         handleVariableRef();
         break;

//    case TK_OTHER_CONSTANT_SYMBOL:
      default:
         m_erh << ECE__ERROR << _REX__38_1 << stringizeToken() << endm;
         break;
      }
   }
}

/*-----------------------------------------------------Compiler::parseRoutine-+
| Parse a function or a subroutine call.                                      |
|                                                                             |
| Whem entering, token is one of TK_FUNCTION, TK_SYMBOL, TK_LEFTPAR           |
+----------------------------------------------------------------------------*/
void Compiler::parseRoutine(CallType ct, ReservedKeywords rsvd)
{
   CodeBuffer::CallParams parms;
   OpCode op;
   Token tkEnd;

   parms.sigl = getLineNo();                  // signaled line
   parms.ct = ct;
   if (ct == CT_FUNCTION) {
      tkEnd = TK_RIGHTPAR;
      op = (token == TK_FUNCTION_LITERAL)? I_CALL_EXT : I_CALL_ANY;
   }else {
      tkEnd = TK_SEMICOLON;
      op = (token == TK_STRING_CONSTANT_SYMBOL)? I_CALL_EXT : I_CALL_ANY;
   }
   if (token == TK_LEFTPAR) {
      handleVariableRef();
      op = I_CALL_REF;
      parms.label = (Label *)0xEFBEADDE;
   }else {
      parms.label = m_mapLabel.access(
         getLastTerm(),
         Label::FUNCTION,
         CodeBuffer::Unknown
      );
   }
   Arguments::PresenceBits mask = 1 << ((8 * sizeof mask)-1);
   parms.presenceBits = 0;                     // bit map of existing args
   while (nextToken(rsvd) != tkEnd) {
      if (token != TK_COMMA) {
         parseExpression(rsvd);
         parms.presenceBits |= mask;
         if (token != TK_COMMA) break;
      }
      if ((mask >>= 1) == 0) {
         m_erh << ECE__ERROR << _REX__40_0 << endm;
      }
   }
   m_cb.addOpCallParams(op, parms);
}

/*------------------------------------------Compiler::parseAssignedExpression-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Compiler::parseAssignedExpression(ReservedKeywords rsvd)
{
   ExpressionContext expCtxt(m_cb.getEndPos(), true, rsvd);

   exprPrecedence0(expCtxt);
   if (m_cb.getEndPos() == expCtxt.m_start) {
      // Nothing was processed: push an empty string
      m_cb.addOpString(I_PUSH, STR_EMPTY);
   }
   if (expCtxt.m_isJustCopy) {
      m_cb.addOp(I_COPY);
   }else {
      m_cb.addOpByte(I_POP, 1);
   }
}

/*--------------------------------------------------Compiler::parseExpression-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool Compiler::parseExpression(ReservedKeywords rsvd)
{
   ExpressionContext expCtxt(m_cb.getEndPos(), false, rsvd);

   exprPrecedence0(expCtxt);
   if (m_cb.getEndPos() == expCtxt.m_start) {
      // Nothing was processed: push an empty string
      m_cb.addOpString(I_PUSH, STR_EMPTY);
      return false;
   }else {
      return true;
   }
}

/*----------------------------------------------------Compiler::insertPushTmp-+
|                                                                             |
+----------------------------------------------------------------------------*/
inline void Compiler::insertPushTmp(
   ExpressionContext & expCtxt,
   CodePosition pos
) {
   if (expCtxt.m_isJustCopy && (pos == expCtxt.m_start)) {
      expCtxt.m_isJustCopy = false;
   }else {
      m_cb.insertByte(pos, I_PUSHTMP);
   }
}

/*--------------------------------------------------Compiler::exprPrecedence0-+
| Or, exclusive Or (Lowest precedence)                                        |
+----------------------------------------------------------------------------*/
void Compiler::exprPrecedence0(ExpressionContext & expCtxt)
{
   CodePosition pos = m_cb.getEndPos();
   exprPrecedence1(expCtxt);

   while ((token == TK_OR) || (token == TK_XOR)) {  /* Logical OR; XOR */
      if (m_cb.getEndPos() == pos) m_erh << ECE__ERROR << _REX__35_0 << endm;
      OpCode op = (OpCode)(token & TK_I_MASK);
      nextToken((ReservedKeywords)expCtxt.m_rsvd);
      exprPrecedence1(expCtxt);
      insertPushTmp(expCtxt, pos);
      m_cb.addOp(op);
   }
}

/*--------------------------------------------------Compiler::exprPrecedence1-+
| And                                                                         |
+----------------------------------------------------------------------------*/
void Compiler::exprPrecedence1(ExpressionContext & expCtxt)
{
   CodePosition pos = m_cb.getEndPos();
   exprPrecedence2(expCtxt);

   while (token == TK_AND) {      /* Logical AND  */
      if (m_cb.getEndPos() == pos) m_erh << ECE__ERROR << _REX__35_0 << endm;
      nextToken((ReservedKeywords)expCtxt.m_rsvd);
      exprPrecedence2(expCtxt);
      insertPushTmp(expCtxt, pos);
      m_cb.addOp(I_AND);
   }
}

/*--------------------------------------------------Compiler::exprPrecedence2-+
| Comparison Operators                                                        |
+----------------------------------------------------------------------------*/
void Compiler::exprPrecedence2(ExpressionContext & expCtxt)
{
   CodePosition pos = m_cb.getEndPos();
   exprPrecedence3(expCtxt);

   if (token & TK_OPGROUP_2) {
      if (m_cb.getEndPos() == pos) m_erh << ECE__ERROR << _REX__35_0 << endm;
      OpCode op = (OpCode)(token & TK_I_MASK);
      nextToken((ReservedKeywords)expCtxt.m_rsvd);
      exprPrecedence3(expCtxt);
      insertPushTmp(expCtxt, pos);
      m_cb.addOp(op);
   }
}

/*--------------------------------------------------Compiler::exprPrecedence3-+
| Concatenation                                                               |
+----------------------------------------------------------------------------*/
void Compiler::exprPrecedence3(ExpressionContext & expCtxt)
{
   CodePosition pos = m_cb.getEndPos();
   exprPrecedence4(expCtxt);

   while (
      (token & TK_SYMBOL_OR_FUNCTION) ||
      (token == TK_CONCAT) || (token == TK_LEFTPAR) || (token == TK_NOT)
   ) {
      if (m_cb.getEndPos() == pos) m_erh << ECE__ERROR << _REX__35_0 << endm;
      OpCode op;
      if (token == TK_CONCAT) {
         op = I_CONCAT_A;
         nextToken((ReservedKeywords)expCtxt.m_rsvd);
      }else {
         op = isAbbutal()? I_CONCAT_A : I_CONCAT_B;
      }
      exprPrecedence4(expCtxt);
      insertPushTmp(expCtxt, pos);
      m_cb.addOp(op);
   }
}

/*--------------------------------------------------Compiler::exprPrecedence4-+
| Addition and Substraction                                                   |
+----------------------------------------------------------------------------*/
void Compiler::exprPrecedence4(ExpressionContext & expCtxt)
{
   CodePosition pos = m_cb.getEndPos();
   exprPrecedence5(expCtxt);

   while ((token == TK_PLUS) || (token == TK_MINUS)) {
      if (m_cb.getEndPos() == pos) m_erh << ECE__ERROR << _REX__35_0 << endm;
      OpCode op = (OpCode)(token & TK_I_MASK);
      nextToken((ReservedKeywords)expCtxt.m_rsvd);
      exprPrecedence5(expCtxt);
      insertPushTmp(expCtxt, pos);
      m_cb.addOp(op);
   }
}

/*--------------------------------------------------Compiler::exprPrecedence5-+
| Multiplication and Division                                                 |
+----------------------------------------------------------------------------*/
void Compiler::exprPrecedence5(ExpressionContext & expCtxt)
{
   CodePosition pos = m_cb.getEndPos();
   exprPrecedence6(expCtxt);

   while (token & TK_OPGROUP_5) {
      if (m_cb.getEndPos() == pos) m_erh << ECE__ERROR << _REX__35_0 << endm;
      OpCode op = (OpCode)(token & TK_I_MASK);
      nextToken((ReservedKeywords)expCtxt.m_rsvd);
      exprPrecedence6(expCtxt);
      insertPushTmp(expCtxt, pos);
      m_cb.addOp(op);
   }
}

/*--------------------------------------------------Compiler::exprPrecedence6-+
| Power operators                                                             |
+----------------------------------------------------------------------------*/
void Compiler::exprPrecedence6(ExpressionContext & expCtxt)
{
   CodePosition pos = m_cb.getEndPos();
   exprPrecedence7(expCtxt);

   while (token == TK_POWER) {
      if (m_cb.getEndPos() == pos) m_erh << ECE__ERROR << _REX__35_0 << endm;
      nextToken((ReservedKeywords)expCtxt.m_rsvd);
      exprPrecedence7(expCtxt);
      insertPushTmp(expCtxt, pos);
      m_cb.addOp(I_POWER);
   }
}

/*--------------------------------------------------Compiler::exprPrecedence7-+
| Prefix operators                                                            |
+----------------------------------------------------------------------------*/
void Compiler::exprPrecedence7(ExpressionContext & expCtxt)
{
   CodePosition pos = m_cb.getEndPos();
   OpCode op = I_NOP;

   switch (token) {
   case TK_PLUS:
      op = I_PLUS;
      nextToken((ReservedKeywords)expCtxt.m_rsvd);
      break;
   case TK_MINUS:
      op = I_MINUS;
      nextToken((ReservedKeywords)expCtxt.m_rsvd);
      break;
   case TK_NOT:
      op = I_NOT;
      nextToken((ReservedKeywords)expCtxt.m_rsvd);
      break;
   }
   if (token & TK_VARIABLE_SYMBOL) {
      m_cb.addOpSymbol(
         I_LOAD_VAR,
         m_mapSym.put(getLastTerm(), token)
      );
      nextToken((ReservedKeywords)expCtxt.m_rsvd);

   }else if (token & TK_CONSTANT_SYMBOL) {
      m_cb.addOpString(I_PUSHLIT, m_mapSym.put(getLastTerm(), token)->key());
      nextToken((ReservedKeywords)expCtxt.m_rsvd);

   }else if (token & TK_FUNCTION) {
      /* add room to the stack for the result string */
      if (expCtxt.m_isJustCopy && (m_cb.getEndPos() == expCtxt.m_start)) {
         expCtxt.m_isJustCopy = false;
      }else {
         m_cb.addOp(I_PUSHTMP);
      }
      parseRoutine(CT_FUNCTION, (ReservedKeywords)expCtxt.m_rsvd);
      if (token != TK_RIGHTPAR) {
         m_erh << ECE__ERROR << _REX__36_0 << endm;
      }
      nextToken((ReservedKeywords)expCtxt.m_rsvd);

   }else if (token == TK_LEFTPAR) {
      nextToken((ReservedKeywords)expCtxt.m_rsvd);
      exprPrecedence0(expCtxt);
      if (token != TK_RIGHTPAR) {
         m_erh << ECE__ERROR << _REX__36_0 << endm;
      }
      nextToken((ReservedKeywords)expCtxt.m_rsvd);
   }

   if (op != I_NOP) {
      if (m_cb.getEndPos()==pos) m_erh << ECE__ERROR << _REX__35_0 << endm;
      insertPushTmp(expCtxt, pos);
      m_cb.addOp(op);
   }
}

#ifdef YAXX_NAMESPACE
}
#endif
/*===========================================================================*/
