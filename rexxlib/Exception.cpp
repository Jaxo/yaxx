/* $Id: Exception.cpp,v 1.11 2002-07-09 08:54:59 pgr Exp $ */

#include <assert.h>
#include "Exception.h"

#ifdef YAXX_NAMESPACE
namespace YAXX_NAMESPACE {
#endif

/*---------------------------------RecoverableException::RecoverableException-+
| Any, but SIG_SYNTAX                                                         |
+----------------------------------------------------------------------------*/
RecoverableException::RecoverableException(Signaled sig, bool isStmtSignal)
{
   assert (m_sig != SIG_SYNTAX);
   m_sig = sig;
   m_isStmtSignal = isStmtSignal;
   m_rxMainMsgId = _REX__4_0;
}

/*---------------------------------RecoverableException::RecoverableException-+
| SIG_SYNTAX only                                                             |
+----------------------------------------------------------------------------*/
RecoverableException::RecoverableException(MsgTemplateId rxMainMsgId) {
   m_sig = SIG_SYNTAX;
   m_isStmtSignal = true;
   m_rxMainMsgId = rxMainMsgId;
}

/*--------------------------------------RecoverableException::getRxMainCodeNo-+
|                                                                             |
+----------------------------------------------------------------------------*/
int RecoverableException::getRxMainCodeNo() const {
   return ::getRxMainCodeNo(m_rxMainMsgId);
}

/*---------------------------------------RecoverableException::getRxMainMsgId-+
|                                                                             |
+----------------------------------------------------------------------------*/
MsgTemplateId RecoverableException::getRxMainMsgId() const {
   return m_rxMainMsgId;
}

/*----------------------------------------------------------makeMsgTemplateId-+
|                                                                             |
+----------------------------------------------------------------------------*/
MsgTemplateId makeMsgTemplateId(RexxString::Exception::Reason r)
{
   switch (r) {
   case RexxString::Exception::NON_INTEGER_REASON:
      return _REX__26_0;
   case RexxString::Exception::NON_BOOLEAN_REASON:
      return _REX__34_0;
   case RexxString::Exception::DIVIDE_BY_ZERO_REASON:
      return _REX__42_3;
   case RexxString::Exception::OVERFLOW_REASON:
   case RexxString::Exception::UNDERFLOW_REASON:
      return _REX__42_0;

// case RexxString::Exception::NON_NUMERIC:
// case RexxString::Exception::BAD_ARITH;
   default:
      return _REX__41_0;
   }
}

#ifdef YAXX_NAMESPACE
}
#endif
/*===========================================================================*/
