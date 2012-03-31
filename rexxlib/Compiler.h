/* $Id: Compiler.h,v 1.98 2002-09-21 08:15:38 pgr Exp $ */

#ifndef COM_JAXO_YAXX_COMPILER_H_INCLUDED
#define COM_JAXO_YAXX_COMPILER_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include "Tokenizer.h"
#include "CodeBuffer.h"
#include "../toolslib/LinkedList.h"
#include "../toolslib/BinarySearch.h"
#include "../reslib/Keyword.h"
class UnicodeComposer;

#ifdef YAXX_NAMESPACE
namespace YAXX_NAMESPACE {
#endif

class RexxString;
class SymbolMap;
class LabelMap;
class Clauses;

/*----------------------------------------------------------- class Compiler -+
|                                                                             |
+----------------------------------------------------------------------------*/
class Compiler : public Constants, private Tokenizer, private BinarySearch
{
public:
   void run();

protected:
   Compiler(
      istream & input,
      CodeBuffer & cb,
      SymbolMap & mapSym,
      LabelMap & mapLabel,
      UnicodeComposer & erh
   );
   virtual ~Compiler();

   virtual void onNewClause() = 0;
   virtual bool onNewLabel() = 0;
   virtual void onEndOfFile(streampos endPos, int lineno) = 0;

   void addLabel(Clauses & clauses);
   void addClause(Clauses & clauses);

   Tokenizer::m_erh;
   Tokenizer::getLastPos;           // access-adjuster

private:
   class LoopCtrl : public LinkedList::Item {
   public:
      LoopCtrl(
         CodePosition iterate,
         CodePosition leave,
         int iVarCount,
         RexxString & strControl
      );
      CodePosition m_iterate;
      CodePosition m_leave;
      int m_iVarCount;
      RexxString & m_strControl;   // see p44 and p52.
   };

   struct RexxInstructions {
      enum KeywordId kwdId;
      void (Compiler::*func)();
   };

   class ExpressionContext {
   public:
      ExpressionContext(
        CodePosition start,
        bool isAssignment,
        int /*Tokenizer::ReservedKeywords*/ rsvd    // POOR MSVC!
      ) :
        m_start(start),
        m_isJustCopy(isAssignment),
        m_rsvd(rsvd)
      {}

      int m_rsvd;
      CodePosition const m_start;
      bool m_isJustCopy;
   };

   LinkedList m_stkLoopCtrl;   // Stack for LEAVE & ITERATE jmp points
   SymbolMap & m_mapSym;       // Current symbol map
   LabelMap & m_mapLabel;      // Current label map
   CodeBuffer & m_cb;          // Code buffer
   int  m_nesting;
   Token token;

   Token nextToken(ReservedKeywords rsvd = RSVD_NONE);
   void nextClause();
   int compare(int ix) const;  // to find statement proc (BinarySearch)

   void instrAddress();
   void instrArg();
   void instrCall();
   void instrDo();
   void instrDrop();
   void instrElseBad();
   void instrEnd();
   void instrExit();
   void instrIf();
   void instrInterpret();
   void instrIterate();
   void instrLeave();
   void instrNop();
   void instrNumeric();
   void instrOptions();
   void instrOtherwiseBad();
   void instrParse();
   void instrProcedure();
   void instrProcedureBad();
   void instrPull();
   void instrPush();
   void instrQueue();
   void instrReturn();
   void instrSay();
   void instrSelect();
   void instrSignal();
   void instrThenBad();
   void instrTrace();
   void instrWhenBad();

   int getLastIntTerm() const;
   LoopCtrl * queryLoopCtrl();
   Symbol * tryAssign(ReservedKeywords rsvd);
   void processHostCmd();
   void handleVariableRef();
   void position();
   void processTemplate();
   RexxString const stringizeToken() const;

   void handleCondition(bool isSignalStmt, bool isOn);

   void parseRoutine(CallType ct, ReservedKeywords rsvd);
   bool parseExpression(ReservedKeywords rsvd = RSVD_NONE);
   void parseAssignedExpression(ReservedKeywords rsvd = RSVD_NONE);
   void insertPushTmp(ExpressionContext & expCtxt, CodePosition pos);

   void exprPrecedence0(ExpressionContext & expCtxt);
   void exprPrecedence1(ExpressionContext & expCtxt);
   void exprPrecedence2(ExpressionContext & expCtxt);
   void exprPrecedence3(ExpressionContext & expCtxt);
   void exprPrecedence4(ExpressionContext & expCtxt);
   void exprPrecedence5(ExpressionContext & expCtxt);
   void exprPrecedence6(ExpressionContext & expCtxt);
   void exprPrecedence7(ExpressionContext & expCtxt);
   void exprPrecedence8(ExpressionContext & expCtxt);
};


inline Token Compiler::nextToken(ReservedKeywords rsvd) {
   token = next(rsvd);
   return token;
}

#ifdef YAXX_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
