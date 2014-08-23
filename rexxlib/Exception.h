/* $Id: Exception.h,v 1.17 2002-03-19 12:20:42 pgr Exp $ */
/*
| DAG:
| RecoverableException --- BareSyntaxException -+- TokenizerBareSyntaxException
|                                          |
|                                          +- SyntaxException
*/
#ifndef COM_JAXO_YAXX_EXCEPTION_H_INCLUDED
#define COM_JAXO_YAXX_EXCEPTION_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include "RexxString.h"
#include "../toolslib/uccompos.h"

#ifdef YAXX_NAMESPACE
namespace YAXX_NAMESPACE {
#endif

class SyntaxException;

/* signal on condition */
enum Signaled {
   SIG_NONE       = 0,
   SIG_ERROR      = 1 << 0,
   SIG_FAILURE    = 1 << 1,
   SIG_HALT       = 1 << 2,
   SIG_NOTREADY   = 1 << 3,
   SIG_NOVALUE    = 1 << 4,
   SIG_SYNTAX     = 1 << 5,
   SIG_LOSTDIGITS = 1 << 6,
   SIG_DISABLED   = 1 << 7
};

/*----------------------------------------------- class RecoverableException -+
|                                                                             |
+----------------------------------------------------------------------------*/
class RecoverableException {
public:
   RecoverableException(Signaled sig, bool isStmtSignal);  // all, but SYNTAX
   RecoverableException(MsgTemplateId rxMainMsgId);        // SYNTAX only!
   RecoverableException();
   Signaled getCondition() const;
   bool isStmtSignal() const;
   int getRxMainCodeNo() const;
   MsgTemplateId getRxMainMsgId() const;
private:
   MsgTemplateId m_rxMainMsgId;
   bool m_isStmtSignal;
   Signaled m_sig;
};

/* -- INLINES -- */
inline Signaled RecoverableException::getCondition() const {
   return m_sig;
}
inline bool RecoverableException::isStmtSignal() const {
   return m_isStmtSignal;
}

/*----------------------------------------------------- class FatalException -+
|                                                                             |
+----------------------------------------------------------------------------*/
class FatalException {
public:
   FatalException(int codeNo) : m_codeNo(codeNo) {}

   FatalException(int codeNo, MemStream & msg) :
   m_codeNo(codeNo), m_msg(msg.str(), msg.pcount()) {
      msg.rdbuf()->freeze(0); // thaw (after str)
   }

   // All exception types should be copyable
   FatalException(FatalException const & source) :            // yes!
   m_codeNo(source.m_codeNo), m_msg(source.m_msg, source.m_msg.length()) {}

   FatalException & operator=(FatalException const & source); // no

   int const m_codeNo;
   StringBuffer m_msg;
};

extern MsgTemplateId makeMsgTemplateId(RexxString::Exception::Reason r);

#ifdef YAXX_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
