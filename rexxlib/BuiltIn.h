/* $Id: BuiltIn.h,v 1.52 2002-11-12 16:41:26 pgr Exp $ */

#ifndef COM_JAXO_YAXX_BUILTIN_H_INCLUDED
#define COM_JAXO_YAXX_BUILTIN_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include "Constants.h"
#include "IOMgr.h"
#include "TimeClock.h"
#include "../toolslib/BinarySearch.h"
#include "../reslib/BuiltInName.h"
#if !defined COM_JAXO_YAXX_DENY_XML
#include "XmlIOMgr.h"
#endif

class CachedDtdList;

#ifdef TOOLS_NAMESPACE
namespace TOOL_NAMESPACE {
#endif
class UnicodeComposer;
#ifdef TOOLS_NAMESPACE
}
#endif

#ifdef YAXX_NAMESPACE
namespace YAXX_NAMESPACE {
#endif

class VariableHandler;
class DataQueueMgr;
class Tracer;
class Routine;
class Clauses;
class Arguments;
class BuiltIn;
typedef void (BuiltIn::* BuiltInFunction)(Arguments &);

/*------------------------------------------------------------ class BuiltIn -+
|                                                                             |
+----------------------------------------------------------------------------*/
class BuiltIn : private Constants {
public:
   BuiltIn(
      VariableHandler & varhdlr,
      Tracer & trhdlr,
      Clauses const &,
      DataQueueMgr & dqm,
      Routine & rtne,
      #if !defined COM_JAXO_YAXX_DENY_XML
         CachedDtdList const & dtds,
      #endif
      UnicodeComposer & erh
   );
   void setDefaultXmlStreams(istream & xmlIn, ostream & xmlOut);
   void onNewClause();
   static BuiltInFunction getFunction(RexxString const &);
   static bool isLoadBuiltIn(BuiltInFunction func);

private:
   class FunctionLookup : public BinarySearch {
   public:
      FunctionLookup(char const * pszFuncName);
      int compare(int ix) const;
      char const * m_pszFuncName;
   };
   friend class FunctionLookup;

   UnicodeComposer & m_erh;
   Tracer & m_tracer;
   VariableHandler & m_varhdlr;
   Clauses const & m_clauses;
   DataQueueMgr & m_dqm;
   Routine & m_rtne;
   IOMgr m_io;
   StopWatch m_nowClock;       // constant within a clause
   bool m_isNowClockFrozen;    // clock has been set for this clause
   #if !defined COM_JAXO_YAXX_DENY_XML
      XmlIOMgr m_xmlIo;
   #endif

   static RexxString STR_CHAR;
   static RexxString STR_NUM;
   static RexxString STR_VAR;
   static RexxString STR_LIT;

   void abbrev(Arguments & args);
   void abs(Arguments & args);
   #if !defined COM_JAXO_YAXX_DENY_MATH
      void acos(Arguments & args);
   #endif
   void address(Arguments & args);
   void arg(Arguments & args);
   #if !defined COM_JAXO_YAXX_DENY_MATH
      void asin(Arguments & args);
      void atan(Arguments & args);
      void atan2(Arguments & args);
   #endif
   void b2x(Arguments & args);
   void bittand(Arguments & args);
   void bittor(Arguments & args);
   void bitxor(Arguments & args);
   void c2d(Arguments & args);
   void c2x(Arguments & args);
   void center(Arguments & args);
   void changestr(Arguments & args);
   void charin(Arguments & args);
   void charout(Arguments & args);
   void chars(Arguments & args);
   void compare(Arguments & args);
   void copies(Arguments & args);
   #if !defined COM_JAXO_YAXX_DENY_MATH
      void cos(Arguments & args);
      void cosh(Arguments & args);
   #endif
   void countstr(Arguments & args);
   void d2c(Arguments & args);
   void d2x(Arguments & args);
   void datatype(Arguments & args);
   void date(Arguments & args);
   void delstr(Arguments & args);
   void delword(Arguments & args);
   void desbuf(Arguments & args);
   void digits(Arguments & args);
   void dropbuf(Arguments & args);
   void errortext(Arguments & args);
   #if !defined COM_JAXO_YAXX_DENY_MATH
      void exp(Arguments & args);
   #endif
   void form(Arguments & args);
   void format(Arguments & args);
   void fuzz(Arguments & args);
   void insert(Arguments & args);
   void lastpos(Arguments & args);
   void left(Arguments & args);
   void length(Arguments & args);
   void linein(Arguments & args);
   void lineout(Arguments & args);
   void lines(Arguments & args);
   void load(Arguments & args);
   #if !defined COM_JAXO_YAXX_DENY_MATH
      void log(Arguments & args);
      void log10(Arguments & args);
   #endif
   void makebuf(Arguments & args);
   void max(Arguments & args);
   void min(Arguments & args);
   void overlay(Arguments & args);
   void pos(Arguments & args);
   #if !defined COM_JAXO_YAXX_DENY_MATH
      void pow(Arguments & args);
      void pow10(Arguments & args);
   #endif
   void queued(Arguments & args);
   void random(Arguments & args);
   void reverse(Arguments & args);
   void right(Arguments & args);
   void sign(Arguments & args);
   #if !defined COM_JAXO_YAXX_DENY_MATH
      void sin(Arguments & args);
      void sinh(Arguments & args);
   #endif
   void sourceline(Arguments & args);
   void space(Arguments & args);
   void sqrt(Arguments & args);
   void stream(Arguments & args);
   void strip(Arguments & args);
   void substr(Arguments & args);
   void subword(Arguments & args);
   void symbol(Arguments & args);
   #if !defined COM_JAXO_YAXX_DENY_MATH
      void tan(Arguments & args);
      void tanh(Arguments & args);
   #endif
   void time(Arguments & args);
   void trace(Arguments & args);
   void translate(Arguments & args);
   void trunc(Arguments & args);
   void value(Arguments & args);
   void verify(Arguments & args);
   void word(Arguments & args);
   void wordindex(Arguments & args);
   void wordlength(Arguments & args);
   void wordpos(Arguments & args);
   void words(Arguments & args);
   void x2b(Arguments & args);
   void x2c(Arguments & args);
   void x2d(Arguments & args);
   #if !defined COM_JAXO_YAXX_DENY_XML
      void xmlattval(Arguments & args);
      void xmlin(Arguments & args);
      void xmlout(Arguments & args);
      void xmltagname(Arguments & args);
      void xmltype(Arguments & args);
   #endif
   void xrange(Arguments & args);
};

/* -- INLINES -- */
inline void BuiltIn::onNewClause() {
   m_isNowClockFrozen = false;
}


#ifdef YAXX_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
