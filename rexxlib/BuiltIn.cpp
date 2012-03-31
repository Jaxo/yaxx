/* $Id: BuiltIn.cpp,v 1.217 2011-07-29 10:26:34 pgr Exp $ */

#include <string.h>
#if !defined COM_JAXO_YAXX_DENY_MATH
#include <math.h>
#endif

#include "Arguments.h"
#include "Tracer.h"
#include "Pool.h"
#include "Clauses.h"
#include "DataQueueMgr.h"
#include "Routines.h"
#include "VariableHandler.h"
#include "BuiltIn.h"

#ifdef YAXX_NAMESPACE
namespace YAXX_NAMESPACE {
#endif

RexxString BuiltIn::STR_CHAR("CHAR");
RexxString BuiltIn::STR_NUM("NUM");
RexxString BuiltIn::STR_VAR("VAR");
RexxString BuiltIn::STR_LIT("LIT");

/*------------------------------------BuiltIn::FunctionLookup::FunctionLookup-+
|                                                                             |
+----------------------------------------------------------------------------*/
BuiltIn::FunctionLookup::FunctionLookup(char const * pszFuncName) :
   BinarySearch((int)_BLTIN_NumberOf),
   m_pszFuncName(pszFuncName)
{
}

/*-------------------------------------------BuiltIn::FunctionLookup::compare-+
|                                                                             |
+----------------------------------------------------------------------------*/
int BuiltIn::FunctionLookup::compare(int ix) const {
   return strcmp(
      m_pszFuncName,
      getBuiltInName((BuiltInNameId)ix)
   );
}

/*STATIC-------------------------------------------------BuiltIn::getFunction-+
|                                                                             |
+----------------------------------------------------------------------------*/
BuiltInFunction BuiltIn::getFunction(RexxString const & strFunctionName)
{
   switch (FunctionLookup(strFunctionName).search()) {
   case _BLTIN__ABBREV:    return &BuiltIn::abbrev;
   case _BLTIN__ABS:       return &BuiltIn::abs;
   #if !defined COM_JAXO_YAXX_DENY_MATH
   case _BLTIN__ACOS:      return &BuiltIn::acos;
   #endif
   case _BLTIN__ADDRESS:   return &BuiltIn::address;
   case _BLTIN__ARG:       return &BuiltIn::arg;
   #if !defined COM_JAXO_YAXX_DENY_MATH
   case _BLTIN__ASIN:      return &BuiltIn::asin;
   case _BLTIN__ATAN:      return &BuiltIn::atan;
   case _BLTIN__ATAN2:     return &BuiltIn::atan2;
   #endif
   case _BLTIN__B2X:       return &BuiltIn::b2x;
   case _BLTIN__BITAND:    return &BuiltIn::bittand;
   case _BLTIN__BITOR:     return &BuiltIn::bittor;
   case _BLTIN__BITXOR:    return &BuiltIn::bitxor;
   case _BLTIN__C2D:       return &BuiltIn::c2d;
   case _BLTIN__C2X:       return &BuiltIn::c2x;
   case _BLTIN__CENTER:    return &BuiltIn::center;
   case _BLTIN__CENTRE:    return &BuiltIn::center;
   case _BLTIN__CHANGESTR: return &BuiltIn::changestr;
   case _BLTIN__CHARIN:    return &BuiltIn::charin;
   case _BLTIN__CHAROUT:   return &BuiltIn::charout;
   case _BLTIN__CHARS:     return &BuiltIn::chars;
   case _BLTIN__COMPARE:   return &BuiltIn::compare;
   case _BLTIN__COPIES:    return &BuiltIn::copies;
   #if !defined COM_JAXO_YAXX_DENY_MATH
   case _BLTIN__COS:       return &BuiltIn::cos;
   case _BLTIN__COSH:      return &BuiltIn::cosh;
   #endif
   case _BLTIN__COUNTSTR:  return &BuiltIn::countstr;
   case _BLTIN__D2C:       return &BuiltIn::d2c;
   case _BLTIN__D2X:       return &BuiltIn::d2x;
   case _BLTIN__DATATYPE:  return &BuiltIn::datatype;
   case _BLTIN__DATE:      return &BuiltIn::date;
   case _BLTIN__DELSTR:    return &BuiltIn::delstr;
   case _BLTIN__DELWORD:   return &BuiltIn::delword;
   case _BLTIN__DESBUF:    return &BuiltIn::desbuf;
   case _BLTIN__DIGITS:    return &BuiltIn::digits;
   case _BLTIN__DROPBUF:   return &BuiltIn::dropbuf;
   case _BLTIN__ERRORTEXT: return &BuiltIn::errortext;
   #if !defined COM_JAXO_YAXX_DENY_MATH
   case _BLTIN__EXP:       return &BuiltIn::exp;
   #endif
   case _BLTIN__FORM:      return &BuiltIn::form;
   case _BLTIN__FORMAT:    return &BuiltIn::format;
   case _BLTIN__FUZZ:      return &BuiltIn::fuzz;
   case _BLTIN__INSERT:    return &BuiltIn::insert;
   case _BLTIN__LASTPOS:   return &BuiltIn::lastpos;
   case _BLTIN__LEFT:      return &BuiltIn::left;
   case _BLTIN__LENGTH:    return &BuiltIn::length;
   case _BLTIN__LINEIN:    return &BuiltIn::linein;
   case _BLTIN__LINEOUT:   return &BuiltIn::lineout;
   case _BLTIN__LINES:     return &BuiltIn::lines;
   case _BLTIN__LOAD:      return &BuiltIn::load;
   #if !defined COM_JAXO_YAXX_DENY_MATH
   case _BLTIN__LOG:       return &BuiltIn::log;
   case _BLTIN__LOG10:     return &BuiltIn::log10;
   #endif
   case _BLTIN__MAKEBUF:   return &BuiltIn::makebuf;
   case _BLTIN__MAX:       return &BuiltIn::max;
   case _BLTIN__MIN:       return &BuiltIn::min;
   case _BLTIN__OVERLAY:   return &BuiltIn::overlay;
   case _BLTIN__POS:       return &BuiltIn::pos;
   #if !defined COM_JAXO_YAXX_DENY_MATH
   case _BLTIN__POW:       return &BuiltIn::pow;
   case _BLTIN__POW10:     return &BuiltIn::pow10;
   #endif
   case _BLTIN__QUEUED:    return &BuiltIn::queued;
   case _BLTIN__RANDOM:    return &BuiltIn::random;
   case _BLTIN__REVERSE:   return &BuiltIn::reverse;
   case _BLTIN__RIGHT:     return &BuiltIn::right;
   case _BLTIN__SIGN:      return &BuiltIn::sign;
   #if !defined COM_JAXO_YAXX_DENY_MATH
   case _BLTIN__SIN:       return &BuiltIn::sin;
   case _BLTIN__SINH:      return &BuiltIn::sinh;
   #endif
   case _BLTIN__SOURCELINE:return &BuiltIn::sourceline;
   case _BLTIN__SPACE:     return &BuiltIn::space;
   case _BLTIN__SQRT:      return &BuiltIn::sqrt;
   case _BLTIN__STREAM:    return &BuiltIn::stream;
   case _BLTIN__STRIP:     return &BuiltIn::strip;
   case _BLTIN__SUBSTR:    return &BuiltIn::substr;
   case _BLTIN__SUBWORD:   return &BuiltIn::subword;
   case _BLTIN__SYMBOL:    return &BuiltIn::symbol;
   #if !defined COM_JAXO_YAXX_DENY_MATH
   case _BLTIN__TAN:       return &BuiltIn::tan;
   case _BLTIN__TANH:      return &BuiltIn::tanh;
   #endif
   case _BLTIN__TIME:      return &BuiltIn::time;
   case _BLTIN__TRACE:     return &BuiltIn::trace;
   case _BLTIN__TRANSLATE: return &BuiltIn::translate;
   case _BLTIN__TRUNC:     return &BuiltIn::trunc;
   case _BLTIN__VALUE:     return &BuiltIn::value;
   case _BLTIN__VERIFY:    return &BuiltIn::verify;
   case _BLTIN__WORD:      return &BuiltIn::word;
   case _BLTIN__WORDINDEX: return &BuiltIn::wordindex;
   case _BLTIN__WORDLENGTH:return &BuiltIn::wordlength;
   case _BLTIN__WORDPOS:   return &BuiltIn::wordpos;
   case _BLTIN__WORDS:     return &BuiltIn::words;
   case _BLTIN__X2B:       return &BuiltIn::x2b;
   case _BLTIN__X2C:       return &BuiltIn::x2c;
   case _BLTIN__X2D:       return &BuiltIn::x2d;
   #if !defined COM_JAXO_YAXX_DENY_XML
   case _BLTIN__XMLATTVAL: return &BuiltIn::xmlattval;    /* YAXX extension */
   case _BLTIN__XMLIN:     return &BuiltIn::xmlin;        /* YAXX extension */
   case _BLTIN__XMLOUT:    return &BuiltIn::xmlout;       /* YAXX extension */
   case _BLTIN__XMLTAGNAME:return &BuiltIn::xmltagname;   /* YAXX extension */
   case _BLTIN__XMLTYPE:   return &BuiltIn::xmltype;      /* YAXX extension */
   #endif
   case _BLTIN__XRANGE:    return &BuiltIn::xrange;
   default:                return 0;
   }
}

/*-----------------------------------------------------------BuiltIn::BuiltIn-+
|                                                                             |
+----------------------------------------------------------------------------*/
BuiltIn::BuiltIn(
   VariableHandler & varhdlr,
   Tracer & trhdlr,
   Clauses const & clauses,
   DataQueueMgr & dqm,
   Routine & rtne,
   #if !defined COM_JAXO_YAXX_DENY_XML
      CachedDtdList const & dtds,
   #endif
   UnicodeComposer & erh
) :
   m_erh(erh),
   m_tracer(trhdlr),
   m_varhdlr(varhdlr),
   m_clauses(clauses),
   m_dqm(dqm),
   #if !defined COM_JAXO_YAXX_DENY_XML     // Yasp3
      m_xmlIo(dtds, erh),
   #endif
   m_rtne(rtne),
   m_isNowClockFrozen(false)
{
}

#if !defined COM_JAXO_YAXX_DENY_XML
/*----------------------------------------------BuiltIn::setDefaultXmlStreams-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::setDefaultXmlStreams(istream & xmlIn, ostream & xmlOut) {
   m_xmlIo.setDefaultXmlIn(xmlIn);
   m_xmlIo.setDefaultXmlOut(xmlOut);
}
#endif

/*--------------------------------------------------------------isLoadBuiltIn-+
| Recognize a call to LOAD.   These are handled separately.                   |
+----------------------------------------------------------------------------*/
bool BuiltIn::isLoadBuiltIn(BuiltInFunction func) {
   return (func == &BuiltIn::load);
}

/*------------------------------------------------------------------CheckArgs-+
| The purpose of the CheckArgs structure is to verify the validity of the     |
| several arguments passed to each built-in.                                  |
|                                                                             |
| The naming follows somehow the X3J18 terminology:                           |
| CheckArgs_xY_xY_.... where:                                                 |
| - x is one of: 'o' (for omittable) or 'r' (required)                        | |
| - Y is one of                                                               |
|     'A' (argument), 'Pad', 'O' (option), 'W' (whole),                       |
|     'Wp' (positive whole), 'Wpz' (positive or zero whole)                   |
|                                                                             |
|(a whole is a non-decimal number)                                            |
+----------------------------------------------------------------------------*/
struct CheckArgs_None {
   CheckArgs_None(Arguments & args, UnicodeComposer & erh) {
      if (args.getCount()) {
         erh << ECE__ERROR << _REX__40_0 << endm;
      }
   }
};

struct CheckArgs_OneOrMore {
   CheckArgs_OneOrMore(Arguments & args, UnicodeComposer & erh) {
      if (!args.getCount()) {
         erh << ECE__ERROR << _REX__40_0 << endm;
      }
   }
};

struct CheckArgs_oA_oA {
   CheckArgs_oA_oA(Arguments & args, UnicodeComposer & erh) {
      if (args.getCount() > 2) {
         erh << ECE__ERROR << _REX__40_0 << endm;
      }
   }
};

struct CheckArgs_oA_oA_oWp {
   CheckArgs_oA_oA_oWp(Arguments & args, UnicodeComposer & erh) {
      start = -1;
      if (
         (args.getCount() > 3) ||
         (args.isPresent(2) && (start = args.getInt(2), start <= 0))
      ) {
         erh << ECE__ERROR << _REX__40_0 << endm;
      }
   }
   int start;
};

struct CheckArgs_oA_oWp_oWpz {
   CheckArgs_oA_oWp_oWpz(Arguments & args, UnicodeComposer & erh) {
      start = -1;
      length = 1;
      if (
         (args.getCount() > 3) ||
         (args.isPresent(1) && (start = args.getInt(1), start <= 0)) ||
         (args.isPresent(2) && (length = args.getInt(2), length < 0))
      ) {
         erh << ECE__ERROR << _REX__40_0 << endm;
      }
   }
   int start;
   int length;
};

struct CheckArgs_oO {
   CheckArgs_oO(Arguments & args, char defaultOption, UnicodeComposer & erh) {
      option = defaultOption;
      if (args.getCount() > 1) {
         erh << ECE__ERROR << _REX__40_0 << endm;
      }
      if (args.isPresent(0)) {
         option = args.getOptionArg(0, erh);
      }
   }
   char option;
};

struct CheckArgs_oW {
   CheckArgs_oW(Arguments & args, UnicodeComposer & erh) {
      n = 1;
      if (args.getCount() > 1) {
         erh << ECE__ERROR << _REX__40_0 << endm;
      }
      if (args.isPresent(0)) n = args.getInt(0);
   }
   int n;
};

struct CheckArgs_oWp {
   CheckArgs_oWp(Arguments & args, UnicodeComposer & erh) {
      n = 1;
      if (
         (args.getCount() > 1) ||
         (args.isPresent(0) && (n = args.getInt(0), n <= 0))
      ) {
         erh << ECE__ERROR << _REX__40_0 << endm;
      }
   }
   int n;
};

struct CheckArgs_rA {
   CheckArgs_rA(Arguments & args, UnicodeComposer & erh) {
      if ((args.getCount() != 1) || !args.isPresent(0)) {
         erh << ECE__ERROR << _REX__40_0 << endm;
      }
   }
};

struct CheckArgs_rA_oA_oA {
   CheckArgs_rA_oA_oA(Arguments & args, UnicodeComposer & erh) {
      if (
         (args.getCount() < 1) || (args.getCount() > 3) ||
         !args.isPresent(0)
      ) {
         erh << ECE__ERROR << _REX__40_0 << endm;
      }
   }
};

struct CheckArgs_rA_oA_oA_oPad {
   CheckArgs_rA_oA_oA_oPad(Arguments & args, UnicodeComposer & erh) {
      pad = ' ';
      if ((args.getCount() < 1) || (args.getCount() > 4) || !args.isPresent(0)) {
         erh << ECE__ERROR << _REX__40_0 << endm;
      }
      if (args.isPresent(3)) pad = args.getPad(3, erh);
   }
   char pad;
};

struct CheckArgs_rA_oO {
   CheckArgs_rA_oO(Arguments & args, UnicodeComposer & erh) {
      if (
        (args.getCount() < 1) || (args.getCount() > 2) ||
        !args.isPresent(0)
      ) {
         erh << ECE__ERROR << _REX__40_0 << endm;
      }
      if (isOptionSpecified = args.isPresent(1), isOptionSpecified) {
         option = args.getOptionArg(1, erh);
      }
   }
   bool isOptionSpecified;
   char option;
};

struct CheckArgs_rA_oO_oA {
   CheckArgs_rA_oO_oA(Arguments & args, UnicodeComposer & erh) {
      if (
         (args.getCount() < 1) || (args.getCount() > 3) ||
         !args.isPresent(0)
      ) {
         erh << ECE__ERROR << _REX__40_0 << endm;
      }
      if (isOptionSpecified = args.isPresent(1), isOptionSpecified) {
         option = args.getOptionArg(1, erh);
      }
   }
   bool isOptionSpecified;
   char option;
};

struct CheckArgs_rA_oO_oPad {
   CheckArgs_rA_oO_oPad(Arguments & args, UnicodeComposer & erh) {
      pad = ' ';
      if (
         (args.getCount() < 1) || (args.getCount() > 3) ||
         !args.isPresent(0)
      ) {
         erh << ECE__ERROR << _REX__40_0 << endm;
      }
      if (isOptionSpecified = args.isPresent(1), isOptionSpecified) {
         option = args.getOptionArg(1, erh);
      }
      if (args.isPresent(2)) pad = args.getPad(2, erh);
   }
   bool isOptionSpecified;
   char option;
   char pad;
};

struct CheckArgs_rA_oWpz {
   CheckArgs_rA_oWpz(Arguments & args, UnicodeComposer & erh) {
      n = -1;
      if (
         (args.getCount() < 1) || (args.getCount() > 2) ||
         !args.isPresent(0) ||
         (args.isPresent(1) && (n = args.getInt(1), n < 0))
      ) {
         erh << ECE__ERROR << _REX__40_0 << endm;
      }
   }
   int n;
};

struct CheckArgs_rA_oWpz_oPad {
   CheckArgs_rA_oWpz_oPad(Arguments & args, UnicodeComposer & erh) {
      n = 1;
      pad = ' ';
      if (
         (args.getCount() < 1) || (args.getCount() > 3) ||
         !args.isPresent(0) ||
         (args.isPresent(1) && (n = args.getInt(1), n < 0))
      ) {
         erh << ECE__ERROR << _REX__40_0 << endm;
      }
      if (args.isPresent(2)) pad = args.getPad(2, erh);
   }
   int n;
   char pad;
};

struct CheckArgs_rA_rA {
   CheckArgs_rA_rA(Arguments & args, UnicodeComposer & erh) {
      if (
         (args.getCount() != 2) ||
         !args.isPresent(0) || !args.isPresent(1)
      ) {
         erh << ECE__ERROR << _REX__40_0 << endm;
      }
   }
};

struct CheckArgs_rA_rA_oO_oWp {
   CheckArgs_rA_rA_oO_oWp(Arguments & args, UnicodeComposer & erh) {
      start = 1;
      if (
         (args.getCount() < 2) || (args.getCount() > 4) ||
         !args.isPresent(0) || !args.isPresent(1) ||
         (args.isPresent(3) && (start = args.getInt(3), start <= 0))
      ) {
         erh << ECE__ERROR << _REX__40_0 << endm;
      }
      if (isOptionSpecified = args.isPresent(2), isOptionSpecified) {
         option = args.getOptionArg(2, erh);
      }
   }
   bool isOptionSpecified;
   char option;
   int start;
};

struct CheckArgs_rA_rA_oPad {
   CheckArgs_rA_rA_oPad(Arguments & args, UnicodeComposer & erh) {
      pad = ' ';
      if (
         (args.getCount() < 2) || (args.getCount() > 3) ||
         !args.isPresent(0) || !args.isPresent(1)
      ) {
         erh << ECE__ERROR << _REX__40_0 << endm;
      }
      if (args.isPresent(2)) pad = args.getPad(2, erh);
   }
   char pad;
};

struct CheckArgs_rA_rA_oWp {
   CheckArgs_rA_rA_oWp(Arguments & args, UnicodeComposer & erh) {
      start = -1;
      if (
         (args.getCount() < 2) || (args.getCount() > 3) ||
         !args.isPresent(0) || !args.isPresent(1) ||
         (args.isPresent(2) && (start = args.getInt(2), start <= 0))
      ) {
         erh << ECE__ERROR << _REX__40_0 << endm;
      }
   }
   int start;
};

struct CheckArgs_rA_rA_oWp_oWpz_oPad {
   CheckArgs_rA_rA_oWp_oWpz_oPad(Arguments & args, UnicodeComposer & erh) {
      n = 0;
      len = -1;
      pad = ' ';
      if (
         (args.getCount() < 2) || (args.getCount() > 5) ||
         !args.isPresent(0) || !args.isPresent(1) ||
         (args.isPresent(2) && (n = args.getInt(2), n < 0)) ||
         (args.isPresent(3) && (len = args.getInt(3), len < 0))
      ) {
         erh << ECE__ERROR << _REX__40_0 << endm;
      }
      if (args.isPresent(4)) pad = args.getPad(4, erh);
   }
   int n;
   int len;
   char pad;
};

struct CheckArgs_rA_rA_oWpz {
   CheckArgs_rA_rA_oWpz(Arguments & args, UnicodeComposer & erh) {
      len = -1;
      if (
         (args.getCount() < 2) || (args.getCount() > 3) ||
         !args.isPresent(0) || !args.isPresent(1) ||
         (args.isPresent(2) && (len = args.getInt(2), len < 0))
      ) {
         erh << ECE__ERROR << _REX__40_0 << endm;
      }
   }
   int len;
};

struct CheckArgs_rA_rA_rA {
   CheckArgs_rA_rA_rA(Arguments & args, UnicodeComposer & erh) {
      if (
         (args.getCount() != 3) ||
         !args.isPresent(0) || !args.isPresent(1) || !args.isPresent(2)
      ) {
         erh << ECE__ERROR << _REX__40_0 << endm;
      }
   }
};

struct CheckArgs_rA_rWp {
   CheckArgs_rA_rWp(Arguments & args, UnicodeComposer & erh) {
      if (
         (args.getCount() != 2) || !args.isPresent(0) || !args.isPresent(1) ||
         (n = args.getInt(1), n <= 0)
      ) {
         erh << ECE__ERROR << _REX__40_0 << endm;
      }
   }
   int n;
};

struct CheckArgs_rA_rWp_oWpz {
   CheckArgs_rA_rWp_oWpz(Arguments & args, UnicodeComposer & erh) {
      len = -1;
      if (
         (args.getCount() < 2) || (args.getCount() > 3) ||
         !args.isPresent(0) || !args.isPresent(1) ||
         (n = args.getInt(1), n <= 0) ||
         (args.isPresent(2) && (len = args.getInt(2), len < 0))
      ) {
         erh << ECE__ERROR << _REX__40_0 << endm;
      }
   }
   int n;
   int len;
};

struct CheckArgs_rA_rWp_oWpz_oPad {
   CheckArgs_rA_rWp_oWpz_oPad(Arguments & args, UnicodeComposer & erh) {
      len = -1;
      pad = ' ';
      if (
         (args.getCount() < 2) || (args.getCount() > 4) ||
         !args.isPresent(0) || !args.isPresent(1) ||
         (n = args.getInt(1), n <= 0) ||
         (args.isPresent(2) && (len = args.getInt(2), len < 0))
      ) {
         erh << ECE__ERROR << _REX__40_0 << endm;
      }
      if (args.isPresent(3)) pad = args.getPad(3, erh);
   }
   int n;
   int len;
   char pad;
};

struct CheckArgs_rA_rWpz {
   CheckArgs_rA_rWpz(Arguments & args, UnicodeComposer & erh) {
      if (
         (args.getCount() != 2) || !args.isPresent(0) || !args.isPresent(1) ||
         (n = args.getInt(1), n < 0)
      ) {
         erh << ECE__ERROR << _REX__40_0 << endm;
      }
   }
   int n;
};

struct CheckArgs_rA_rWpz_oPad {
   CheckArgs_rA_rWpz_oPad(Arguments & args, UnicodeComposer & erh) {
      pad = ' ';
      if (
         (args.getCount() < 2) || (args.getCount() > 3) ||
         !args.isPresent(0) || !args.isPresent(1) ||
         (n = args.getInt(1), n < 0)
      ) {
         erh << ECE__ERROR << _REX__40_0 << endm;
      }
      if (args.isPresent(2)) pad = args.getPad(2, erh);
   }
   int n;
   char pad;
};

struct CheckArgs_rWargno_oO {
   CheckArgs_rWargno_oO(Arguments & args, UnicodeComposer & erh) {
      if (
         !args.isPresent(0) ||
         (argNo = args.getInt(0), (argNo < 1) || (argNo > Arguments::MAXARGS))
      ) {
         erh << ECE__ERROR << _REX__40_0 << endm;
      }
      if (isOptionSpecified = args.isPresent(1), isOptionSpecified) {
         option = args.getOptionArg(1, erh);
      }
   }
   int argNo;
   char option;
   bool isOptionSpecified;
};

struct CheckArgs_rW_oWpz {    // rW is a true whole (not just an int)
   CheckArgs_rW_oWpz(Arguments & args, UnicodeComposer & erh) {
      n = -1;
      if (
         (args.getCount() < 1) || (args.getCount() > 2) ||
         !args.isPresent(0) ||
         (args.isPresent(1) && (n = args.getInt(1), n < 0)) ||
         !args.getArg(0).isNumber() ||
         !args.getArg(0).isWhole(val)
      ) {
         erh << ECE__ERROR << _REX__40_0 << endm;
      }
   }
   int n;
   DecRexx val;
};

struct CheckArgs_rWp {
   CheckArgs_rWp(Arguments & args, UnicodeComposer & erh) {
      if (
         (args.getCount() != 1) ||
         (!args.isPresent(0) || (n = args.getInt(0), n <= 0))
      ) {
         erh << ECE__ERROR << _REX__40_0 << endm;
      }
   }
   int n;
};

/*------------------------------------------------------------BuiltIn::abbrev-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::abbrev(Arguments & args) {
   CheckArgs_rA_rA_oWpz check(args, m_erh);
   args.getResult().assign(
      args.getArg(0).abbrev(args.getArg(1), check.len)
   );
}

/*---------------------------------------------------------------BuiltIn::abs-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::abs(Arguments & args) {
   CheckArgs_rA check(args, m_erh);
   args.getResult().abs(args.getArg(0), m_rtne.getDecRexxContext());
}

#if !defined COM_JAXO_YAXX_DENY_MATH
/*--------------------------------------------------------------BuiltIn::acos-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::acos(Arguments & args) {
   CheckArgs_rA check(args, m_erh);
   args.getResult().assign(::acos((double)args.getArg(0)));
}
#endif

/*-----------------------------------------------------------BuiltIn::address-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::address(Arguments & args) {
   CheckArgs_None check(args, m_erh);
   args.getResult() = m_rtne.getAddress();
}

#if !defined COM_JAXO_YAXX_DENY_MATH
/*--------------------------------------------------------------BuiltIn::asin-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::asin(Arguments & args) {
   CheckArgs_rA check(args, m_erh);
   args.getResult().assign(::asin((double)args.getArg(0)));
}
#endif

#if !defined COM_JAXO_YAXX_DENY_MATH
/*--------------------------------------------------------------BuiltIn::atan-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::atan(Arguments & args) {
   CheckArgs_rA check(args, m_erh);
   args.getResult() = RexxString(::atan((double)args.getArg(0)));
}
#endif

#if !defined COM_JAXO_YAXX_DENY_MATH
/*-------------------------------------------------------------BuiltIn::atan2-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::atan2(Arguments & args) {
   CheckArgs_rA_rA check(args, m_erh);
   args.getResult() = RexxString(
      ::atan2((double)args.getArg(0), (double)args.getArg(1))
   );
}
#endif

/*------------------------------------------------------------BuiltIn::bittand-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::bittand(Arguments & args) {
   CheckArgs_rA_oA_oA check(args, m_erh);
   args.getResult().bitIt(
      args.getArg(0),
      (args.isPresent(1))? args.getArg(1) : RexxString::Nil,
      (args.isPresent(2))? args.getPad(2, m_erh) & 0xFF : -1,
      '&'
   );
}

/*-------------------------------------------------------------BuiltIn::bittor-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::bittor(Arguments & args) {
   CheckArgs_rA_oA_oA check(args, m_erh);
   args.getResult().bitIt(
      args.getArg(0),
      (args.isPresent(1))? args.getArg(1) : RexxString::Nil,
      (args.isPresent(2))? args.getPad(2, m_erh) & 0xFF : -1,
      '|'
   );
}

/*------------------------------------------------------------BuiltIn::bitxor-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::bitxor(Arguments & args) {
   CheckArgs_rA_oA_oA check(args, m_erh);
   args.getResult().bitIt(
      args.getArg(0),
      (args.isPresent(1))? args.getArg(1) : RexxString::Nil,
      (args.isPresent(2))? args.getPad(2, m_erh) & 0xFF : -1,
      '^'
   );
}

/*---------------------------------------------------------------BuiltIn::c2d-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::c2d(Arguments & args)
{
   CheckArgs_rA_oWpz check(args, m_erh);
   bool isNegative;
   int len = args.getArg(0).length();
   if (check.n == 0) {
      isNegative = false;
   }else if ((check.n == -1) || (check.n > len)) {   // omitted or too big
      isNegative = false;
      check.n = len;
   }else {
      isNegative = ((args.getArg(0)[len-check.n] & 0x80) != 0);
   }
   args.getResult().c2d(
      ((char const *)args.getArg(0))+len-check.n,
      check.n,
      isNegative,
      m_rtne.getDecRexxContext()
   );
}

/*---------------------------------------------------------------BuiltIn::c2x-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::c2x(Arguments & args) {
   CheckArgs_rA check(args, m_erh);
   args.getResult().c2x(args.getArg(0));
}

/*------------------------------------------------------------BuiltIn::center-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::center(Arguments & args) {
   CheckArgs_rA_rWpz_oPad check(args, m_erh);
   args.getResult().center(args.getArg(0), check.n, check.pad);
}

/*---------------------------------------------------------BuiltIn::changestr-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::changestr(Arguments & args) {
   CheckArgs_rA_rA_rA check(args, m_erh);
   args.getResult().changestr(
      args.getArg(1), args.getArg(0), args.getArg(2)
   );
}

/*------------------------------------------------------------BuiltIn::charin-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::charin(Arguments & args)
{
   Signaled sig;
   CheckArgs_oA_oWp_oWpz check(args, m_erh);
   args.getResult() = m_io.charin(
      args.getArg(0), check.start, check.length, sig
   );
   m_rtne.raise(sig);
}

/*-----------------------------------------------------------BuiltIn::charout-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::charout(Arguments & args) {
   int res = 0;
   CheckArgs_oA_oA_oWp check(args, m_erh);
   Signaled sig;
   args.getResult() = RexxString(
      m_io.charout(args.getArg(0), args.getArg(1), check.start, sig)
   );
   m_rtne.raise(sig);
}

/*-------------------------------------------------------------BuiltIn::chars-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::chars(Arguments & args) {
   CheckArgs_oA_oA check(args, m_erh);
   // Option 1 is "C" or "N" -- not used here.  We will need it for HTTP.
   Signaled sig;
   args.getResult() = RexxString(m_io.chars(args.getArg(0), sig));
   m_rtne.raise(sig);
}

/*-----------------------------------------------------------BuiltIn::compare-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::compare(Arguments & args) {
   CheckArgs_rA_rA_oPad check(args, m_erh);
   args.getResult() = RexxString(
      args.getArg(0).compare(args.getArg(1), check.pad)
   );
}

/*------------------------------------------------------------BuiltIn::copies-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::copies(Arguments & args) {
   CheckArgs_rA_rWpz check(args, m_erh);
   args.getResult().copies(args.getArg(0), check.n);
}

#if !defined COM_JAXO_YAXX_DENY_MATH
/*---------------------------------------------------------------BuiltIn::cos-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::cos(Arguments & args) {
   CheckArgs_rA check(args, m_erh);
   args.getResult() = RexxString(::cos((double)args.getArg(0)));
}
#endif

#if !defined COM_JAXO_YAXX_DENY_MATH
/*--------------------------------------------------------------BuiltIn::cosh-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::cosh(Arguments & args) {
   CheckArgs_rA check(args, m_erh);
   args.getResult() = RexxString(::cosh((double)args.getArg(0)));
}
#endif

/*----------------------------------------------------------BuiltIn::countstr-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::countstr(Arguments & args) {
   CheckArgs_rA_rA check(args, m_erh);
   args.getResult() = RexxString(args.getArg(1).countstr(args.getArg(0)));
}

/*---------------------------------------------------------------BuiltIn::d2c-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::d2c(Arguments & args)
{
   CheckArgs_rW_oWpz check(args, m_erh);
   if ((check.n == -1) && args.getArg(0).isNegative()) {
      m_erh << ECE__ERROR << _REX__40_0 << endm;
   }
   args.getResult().d2c(check.val, check.n);
}

/*---------------------------------------------------------------BuiltIn::d2x-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::d2x(Arguments & args) {
   CheckArgs_rW_oWpz check(args, m_erh);
   if ((check.n == -1) && args.getArg(0).isNegative()) {
      m_erh << ECE__ERROR << _REX__40_0 << endm;
   }
   args.getResult().d2x(check.val, check.n);
}

/*--------------------------------------------------------------BuiltIn::date-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::date(Arguments & args)
{
   char fmtOut = args.getOptionOrDefault(0, 'N');
   char sepOut = args.getOptionOrDefault(3, fmtOut=='N'? ' ' : '/');
   bool badFmt = false;
   MemStream result;
   TimeClock * pUserClock = 0;
   TimeClock * pWhatClock;

   /* The date/time is 'frozen' throughout a clause. */
   if (!m_isNowClockFrozen) {
      m_isNowClockFrozen = true;
      m_nowClock.set();
   }

   if (args.getCount() > 5) {
      m_erh << ECE__ERROR << _REX__40_4 << "DATE" << '5' << endm;
   }
   if ((args.getArg(3).length() > 1) || isalnum(sepOut)) {
      m_erh << ECE__ERROR << _REX__40_45 << "" << '4' << args.getArg(3) << endm;
   }

   // What clock should we use? current? user defined?
   if (!args.isPresent(1)) {
      // If there is no second argument, the current date is returned.
      if (args.isPresent(2)) {
         // If the third argument is given then the second is mandatory.
         m_erh << ECE__ERROR << _REX__40_19 << "" << ""
             << args.getArg(2) << endm;
      }
      pWhatClock = &m_nowClock;
   }else {
      // If there is a second argument it provides the date to be converted.
      int days = -1;

      char fmtIn = args.getOptionOrDefault(2, 'N');
      char sepIn = args.getOptionOrDefault(4, fmtIn=='N'? ' ' : '/');
      if ((args.getArg(4).length() > 1) || isalnum(sepIn)) {
         m_erh << ECE__ERROR << _REX__40_45 << "" << '5' << args.getArg(4) << endm;
      }
      switch (fmtIn) {
      case 'B':    // nnn.. Basedays (since 1900)
         if (args.isPresent(4)) {
            badFmt = true;
         }else {
            days = args.getArg(1);
         }
         break;
      case 'D':    // nnn.. Days to year;  includes current day
         if (args.isPresent(4)) {
            badFmt = true;
         }else {
            days = m_nowClock.getBaseDays(((int)args.getArg(1))-1);
         }
         break;
      case 'E':    // dd/mm/yy
         days = m_nowClock.getBaseDays(args.getArg(1), 0, 1, 2, sepIn);
         break;
      case 'N':    // dd Mmm yyyy
         days = TimeClock::getBaseDays((char const *)args.getArg(1), sepIn);
         break;
      case 'O':    // yy/mm/dd
         days = m_nowClock.getBaseDays(args.getArg(1), 2, 1, 0, sepIn);
         break;
      case 'S':    // yyyymmdd
         if (!args.isPresent(4)) sepIn = '\0';
         days = TimeClock::getBaseDaysISO(args.getArg(1), sepIn);
         break;
      case 'U':    // mm/dd/yy
         days = m_nowClock.getBaseDays(args.getArg(1), 1, 0, 2, sepIn);
         break;
      default:
         m_erh << ECE__ERROR << _REX__40_28 << ""
             << '3' << "BDENOSU" << fmtIn << endm;
         break;
      }
      if (badFmt) {
         m_erh << ECE__ERROR << _REX__40_46 << "" << '3' << fmtIn << '5' << endm;
      }
      if (days < 0) {
         m_erh << ECE__ERROR << _REX__40_19 << ""
             << args.getArg(1) << fmtIn << endm;
      }
      pWhatClock = pUserClock = new TimeClockDays(days);
   }

   switch (fmtOut) {
   case 'B':       // nnn.. Basedays (since 1900)
      if (args.isPresent(3)) {
         badFmt = true;
      }else {
         pWhatClock->formatBaseDays(result);
      }
      break;
   case 'C':       // nnn.. Days in the century
      if (args.isPresent(3)) {
         badFmt = true;
      }else {
         pWhatClock->formatCentury(result);
      }
      break;
   case 'D':       // nnn.. Days to year;  includes current day
      if (args.isPresent(3)) {
         badFmt = true;
      }else {
         pWhatClock->formatDays(result);
      }
      break;
   case 'E':       // dd/mm/yy
      pWhatClock->formatEuropeanDate(result, sepOut);
      break;
   case 'M':       // Month
      if (args.isPresent(3)) {
         badFmt = true;
      }else {
         pWhatClock->formatMonthName(result);
      }
      break;
   case 'N':       // dd Mmm yyyy
      pWhatClock->formatNormalDate(result, sepOut);
      break;
   case 'O':       // yy/mm/dd
      pWhatClock->formatOrderedDate(result, sepOut);
      break;
   case 'S':       // yyyymmdd
      if (!args.isPresent(3)) sepOut = '\0';
      pWhatClock->formatStandardDate(result, sepOut);
      break;
   case 'U':       // mm/dd/yy
      pWhatClock->formatUsaDate(result, sepOut);
      break;
   case 'W':
      if (args.isPresent(3)) {
         badFmt = true;
      }else {
         pWhatClock->formatDayName(result);
      }
      break;
   default:
      if (pUserClock) delete pUserClock;
      m_erh << ECE__ERROR << _REX__40_28 << ""
            << '3' << "BCDEMNOSUW" << fmtOut << endm;
      break;
   }
   if (pUserClock) delete pUserClock;
   if (badFmt) {
      m_erh << ECE__ERROR << _REX__40_46 << "" << '1' << fmtOut << '4' << endm;
   }
   args.getResult() = RexxString(result.str(), result.rdbuf()->in_avail());
   result.rdbuf()->freeze(0);   // thaw
}

/*----------------------------------------------------------BuiltIn::datatype-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::datatype(Arguments & args)
{
   CheckArgs_rA_oO check(args, m_erh);
   if (check.isOptionSpecified) {
      DecRexx d;
      bool res;
      switch (check.option) {
      case 'A':  res = args.getArg(0).isAlphanum();     break;
      case 'B':  res = args.getArg(0).isBinary();       break;
      case 'L':  res = args.getArg(0).isLowerAlpha();   break;
      case 'M':  res = args.getArg(0).isAlpha();        break;
      case 'N':  res = args.getArg(0).isNumber();       break;
      case 'S':  res = args.getArg(0).isSymbol();       break;
      case 'U':  res = args.getArg(0).isUpperAlpha();   break;
      case 'W':  res = args.getArg(0).isWhole(d);       break;
      case 'X':  res = args.getArg(0).isHexa();         break;
      default:
         m_erh << ECE__ERROR << _REX__40_0 << endm;
         break;
      }
      args.getResult().assign(res);
   }else {
      if (args.getArg(0).isNumber()) {
         args.getResult() = STR_NUM;
      }else {
         args.getResult() = STR_CHAR;
      }
   }
}

/*------------------------------------------------------------BuiltIn::delstr-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::delstr(Arguments & args) {
   CheckArgs_rA_rWp_oWpz check(args, m_erh);
   args.getResult().delstr(args.getArg(0), --check.n, check.len); // Rexx->C
}

/*-----------------------------------------------------------BuiltIn::delword-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::delword(Arguments & args) {
   CheckArgs_rA_rWp_oWpz check(args, m_erh);
   args.getResult().delword(args.getArg(0), check.n, check.len);
}

/*------------------------------------------------------------BuiltIn::desbuf-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::desbuf(Arguments & args) {
   CheckArgs_None check(args, m_erh);
   args.getResult() = RexxString(m_dqm.removeDataQueues());
}

/*------------------------------------------------------------BuiltIn::digits-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::digits(Arguments & args) {
   CheckArgs_None check(args, m_erh);
   args.getResult() = RexxString(m_rtne.getDecRexxContext().getDigits());
}

/*-----------------------------------------------------------BuiltIn::dropbuf-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::dropbuf(Arguments & args) {
   CheckArgs_oW check(args, m_erh);
   args.getResult() = RexxString(m_dqm.removeDataQueues(check.n));
}

/*---------------------------------------------------------BuiltIn::errortext-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::errortext(Arguments & args) {
   CheckArgs_rWp check(args, m_erh);
   args.getResult() = RexxString(
      getMsgTemplate((MsgTemplateId)(check.n + _REX__0_0))
   );
}

#if !defined COM_JAXO_YAXX_DENY_MATH
/*---------------------------------------------------------------BuiltIn::exp-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::exp(Arguments & args) {
   CheckArgs_rA check(args, m_erh);
   args.getResult() = RexxString(::exp((double)args.getArg(0)));
}
#endif

/*--------------------------------------------------------------BuiltIn::form-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::form(Arguments & args) {
   CheckArgs_None check(args, m_erh);
   args.getResult() = m_rtne.getForm();
}

/*------------------------------------------------------------BuiltIn::format-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::format(Arguments & args)
{
   int before = -1;
   int after = -1;
   int expp = -1;
   int expt = -1;

   if (
      (args.getCount() < 1) || (args.getCount() > 5) || !args.isPresent(0) ||
      (args.isPresent(1) && (before = args.getInt(1), before < 0)) ||
      (args.isPresent(2) && (after = args.getInt(2), after < 0)) ||
      (args.isPresent(3) && (expp = args.getInt(3), expp < 0)) ||
      (args.isPresent(4) && (expt = args.getInt(4), expt < 0)) ||
      !args.getResult().format(
         args.getArg(0),
         before, after, expp, expt, m_rtne.getDecRexxContext()
      )
   ) {
      m_erh << ECE__ERROR << _REX__40_0 << endm;
   }
}

/*--------------------------------------------------------------BuiltIn::fuzz-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::fuzz(Arguments & args) {
   CheckArgs_None check(args, m_erh);
   args.getResult() = RexxString(m_rtne.getDecRexxContext().getFuzz());
}

/*------------------------------------------------------------BuiltIn::insert-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::insert(Arguments & args) {
   CheckArgs_rA_rA_oWp_oWpz_oPad check(args, m_erh);
   args.getResult().insert(
      args.getArg(1), args.getArg(0), check.n, check.len, check.pad
   );
}

/*-----------------------------------------------------------BuiltIn::lastpos-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::lastpos(Arguments & args) {
   CheckArgs_rA_rA_oWp check(args, m_erh);
   args.getResult() = RexxString(
      args.getArg(0).lastpos(args.getArg(1), check.start)
   );
}

/*--------------------------------------------------------------BuiltIn::left-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::left(Arguments & args) {
   CheckArgs_rA_rWpz_oPad check(args, m_erh);
   args.getResult().left(args.getArg(0), check.n, check.pad);
}

/*------------------------------------------------------------BuiltIn::length-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::length(Arguments & args) {
   if (args.getCount() != 1) {
      m_erh << ECE__ERROR << _REX__40_0 << endm;
   }
   args.getResult().assign(args.getArg(0).length());
}

/*------------------------------------------------------------BuiltIn::linein-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::linein(Arguments & args)
{
   Signaled sig;
   CheckArgs_oA_oWp_oWpz check(args, m_erh);
   if (check.length > 1) {
      m_erh << ECE__ERROR << _REX__40_0 << endm;
   }
   args.getResult() = m_io.linein(
      args.getArg(0), check.start, check.length, sig
   );
   m_rtne.raise(sig);
}

/*-----------------------------------------------------------BuiltIn::lineout-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::lineout(Arguments & args) {
   CheckArgs_oA_oA_oWp check(args, m_erh);
   Signaled sig;
   args.getResult() = RexxString(
      m_io.lineout(args.getArg(0), args.getArg(1), check.start, sig)
   );
   m_rtne.raise(sig);
}

/*-------------------------------------------------------------BuiltIn::lines-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::lines(Arguments & args) {
   CheckArgs_oA_oA check(args, m_erh);
   // Option 1 is "C" or "N" -- not used here.  We will need it for HTTP.
   Signaled sig;
   args.getResult() = RexxString(m_io.lines(args.getArg(0), sig));
   m_rtne.raise(sig);
}

/*--------------------------------------------------------------BuiltIn::load-+
| The built-in LOAD is an exception, as it may modify the Clauses, the Code   |
| buffer, the file list.   It is then processed directly by the Interpreter,  |
| which is the only one entitled to modify these fields.                      |
+----------------------------------------------------------------------------*/
void BuiltIn::load(Arguments & args) {
   assert (false);
}

#if !defined COM_JAXO_YAXX_DENY_MATH
/*---------------------------------------------------------------BuiltIn::log-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::log(Arguments & args) {
   CheckArgs_rA check(args, m_erh);
   args.getResult() = RexxString(::log((double)args.getArg(0)));
}
#endif

#if !defined COM_JAXO_YAXX_DENY_MATH
/*-------------------------------------------------------------BuiltIn::log10-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::log10(Arguments & args) {
   CheckArgs_rA check(args, m_erh);
   args.getResult() = RexxString(::log10((double)args.getArg(0)));
}
#endif

/*-----------------------------------------------------------BuiltIn::makebuf-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::makebuf(Arguments & args) {
   CheckArgs_None check(args, m_erh);
   m_dqm.addDataQueue();
   args.getResult() = RexxString(m_dqm.countDataQueues()-1);
}

/*---------------------------------------------------------------BuiltIn::max-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::max(Arguments & args) {
   CheckArgs_OneOrMore check(args, m_erh);
   args.getResult() = args.getArg(
      RexxString::max(
         args.getCount(),
         args.getArgs(),
         m_rtne.getDecRexxContext()
      )
   );
}

/*---------------------------------------------------------------BuiltIn::min-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::min(Arguments & args) {
   CheckArgs_OneOrMore check(args, m_erh);
   args.getResult() = args.getArg(
      RexxString::min(
         args.getCount(),
         args.getArgs(),
         m_rtne.getDecRexxContext()
      )
   );
}

/*-----------------------------------------------------------BuiltIn::overlay-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::overlay(Arguments & args) {
   CheckArgs_rA_rA_oWp_oWpz_oPad check(args, m_erh);
   args.getResult().overlay(
      args.getArg(1), args.getArg(0), check.n-1, check.len, check.pad
   );
}

/*---------------------------------------------------------------BuiltIn::pos-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::pos(Arguments & args) {
   CheckArgs_rA_rA_oWp check(args, m_erh);
   args.getResult() = RexxString(
      args.getArg(0).pos(args.getArg(1), check.start)
   );
}

#if !defined COM_JAXO_YAXX_DENY_MATH
/*---------------------------------------------------------------BuiltIn::pow-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::pow(Arguments & args) {
   CheckArgs_rA_rA check(args, m_erh);
   args.getResult() = RexxString(
      ::pow((double)args.getArg(0), (double)args.getArg(1))
   );
}
#endif

#if !defined COM_JAXO_YAXX_DENY_MATH
/*-------------------------------------------------------------BuiltIn::pow10-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::pow10(Arguments & args) {
   CheckArgs_rA check(args, m_erh);
   args.getResult() = RexxString(::pow((double)10.0, (double)args.getArg(0)));
}
#endif

/*------------------------------------------------------------BuiltIn::queued-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::queued(Arguments & args)
{
   CheckArgs_oO check(args, 'N', m_erh);
   switch (check.option) {
   case 'T':
      args.getResult() = RexxString(m_dqm.currentQueueSize());
      break;
   case 'B':
      args.getResult() = RexxString(m_dqm.countDataQueues());
      break;
   case 'N':       /* the default option, set above */
   case 'A':
      args.getResult() = RexxString(m_dqm.totalSize());
      break;
   default:
      m_erh << ECE__ERROR << _REX__40_0 << endm;
      break;
   }
}

/*------------------------------------------------------------BuiltIn::random-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::random(Arguments & args)
{
   int max = 999;
   int min = 0;
   int seed = -1;

   if (
      (args.getCount() < 0) || (args.getCount() > 3) ||
      (args.isPresent(0) && (min = args.getArg(0), min < 0)) ||
      (args.isPresent(1) && (max = args.getArg(1), max < 0)) ||
      (args.isPresent(2) && (seed = args.getArg(2), seed < 0)) ||
      (min > max)
   ) {
      m_erh << ECE__ERROR << _REX__40_0 << endm;
   }

   if (args.getCount() == 1) { max = min; min = 0; } // RANDOM(max)

   if (seed > 0) {
      m_rtne.setSeed((unsigned int)seed);
   }else if (seed = m_rtne.getSeed(), seed == -1) {
      m_rtne.setSeed((unsigned int)(::time((time_t *)0)%(3600*24)));
   }
   int value = rand();
   value = min + (int) ((1.0+max-min) * value / (RAND_MAX + 1.0));
   args.getResult() = RexxString(value);
}

/*-----------------------------------------------------------BuiltIn::reverse-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::reverse(Arguments & args) {
   CheckArgs_rA check(args, m_erh);
   args.getResult().reverse(args.getArg(0));
}

/*-------------------------------------------------------------BuiltIn::right-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::right(Arguments & args) {
   CheckArgs_rA_rWpz_oPad check(args, m_erh);
   args.getResult().right(args.getArg(0), check.n, check.pad);
}

/*--------------------------------------------------------------BuiltIn::sign-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::sign(Arguments & args) {
   CheckArgs_rA check(args, m_erh);
   RexxString fooBar(args.getArg(0).sign(m_rtne.getDecRexxContext()));
   args.getResult() = fooBar;
//   args.getResult() = RexxString(
//      args.getArg(0).sign(m_rtne.getDecRexxContext())
//   );
}

#if !defined COM_JAXO_YAXX_DENY_MATH
/*---------------------------------------------------------------BuiltIn::sin-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::sin(Arguments & args) {
   CheckArgs_rA check(args, m_erh);
   args.getResult() = RexxString(::sin((double)args.getArg(0)));
}
#endif

#if !defined COM_JAXO_YAXX_DENY_MATH
/*--------------------------------------------------------------BuiltIn::sinh-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::sinh(Arguments & args) {
   CheckArgs_rA check(args, m_erh);
   args.getResult() = RexxString(::sinh((double)args.getArg(0)));
}
#endif

/*--------------------------------------------------------BuiltIn::sourceline-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::sourceline(Arguments & args) {
   CheckArgs_oWp check(args, m_erh);
   if (args.isPresent(0)) {
      args.getResult() = m_clauses.getLineAt(check.n);
   }else {
      args.getResult() = RexxString(m_clauses.getTotalLines());
   }
}

/*-------------------------------------------------------------BuiltIn::space-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::space(Arguments & args) {
   CheckArgs_rA_oWpz_oPad check(args, m_erh);
   args.getResult().wordsSpace(args.getArg(0), check.n, check.pad);
}

/*--------------------------------------------------------------BuiltIn::sqrt-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::sqrt(Arguments & args) {
   CheckArgs_rA_oWpz check(args, m_erh);
   args.getResult().sqrt(args.getArg(0), m_rtne.getDecRexxContext());
}

/*------------------------------------------------------------BuiltIn::stream-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::stream(Arguments & args)
{
   CheckArgs_rA_oO_oA check(args, m_erh);

   if (!check.isOptionSpecified) {
      check.option = 'S';            // State
   }
   switch (check.option) {
   case 'C':          // Command   STREAM('blah.txt', 'C', 'CLOSE')
      if (!args.isPresent(2)) m_erh << ECE__ERROR << _REX__40_0 << endm;
      Signaled sig;
      args.getResult() = RexxString(
         m_io.streamCmnd(args.getArg(0), args.getArg(2), sig)
      );
      if (sig == SIG_ERROR) m_erh << ECE__ERROR << _REX__40_0 << endm;
      m_rtne.raise(sig);
      break;
   case 'D':          // get a description of the state
      if (args.isPresent(2)) m_erh << ECE__ERROR << _REX__40_0 << endm;
      args.getResult() = m_io.describe(args.getArg(0));
      break;
   case 'S':          // result of the last operation
      if (args.isPresent(2)) m_erh << ECE__ERROR << _REX__40_0 << endm;
      args.getResult() = m_io.state(args.getArg(0));
      break;
   default:
      m_erh << ECE__ERROR << _REX__40_0 << endm;
   }
}

/*-------------------------------------------------------------BuiltIn::strip-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::strip(Arguments & args)
{
   CheckArgs_rA_oO_oPad check(args, m_erh);
   RexxString::Kill options = RexxString::KillBOTH;

   if (check.isOptionSpecified) {
      switch (check.option) {
      case 'B':
         options = RexxString::KillBOTH;
         break;
      case 'L':
         options = RexxString::KillLEAD;
         break;
      case 'T':
         options = RexxString::KillTRAIL;
         break;
      default:
         m_erh << ECE__ERROR << _REX__40_0 << endm;
         break;
      }
   }
   args.getResult().strip(args.getArg(0), options, check.pad);
}

/*------------------------------------------------------------BuiltIn::substr-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::substr(Arguments & args) {
   CheckArgs_rA_rWp_oWpz_oPad check(args, m_erh);
   args.getResult().substr(
      args.getArg(0), check.n-1, check.len, check.pad   // Rexx-> C
   );
}

/*-----------------------------------------------------------BuiltIn::subword-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::subword(Arguments & args) {
   CheckArgs_rA_rWp_oWpz check(args, m_erh);
   args.getResult().subword(args.getArg(0), check.n, check.len);
}

/*------------------------------------------------------------BuiltIn::symbol-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::symbol(Arguments & args)
{
   CheckArgs_rA check(args, m_erh);
   if (!args.getArg(0).isSymbol()) {
      args.getResult() = RexxString("BAD");
   }else {
      bool found;
      RexxString needle(args.getArg(0), CharConverter::Upper);
      m_varhdlr.find(needle, found);
      if (found) {
         args.getResult() = RexxString("VAR");
      }else {
         args.getResult() = RexxString("LIT");
      }
   }
}

#if !defined COM_JAXO_YAXX_DENY_MATH
/*---------------------------------------------------------------BuiltIn::tan-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::tan(Arguments & args) {
   CheckArgs_rA check(args, m_erh);
   args.getResult() = RexxString(::tan((double)args.getArg(0)));
}
#endif

#if !defined COM_JAXO_YAXX_DENY_MATH
/*--------------------------------------------------------------BuiltIn::tanh-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::tanh(Arguments & args) {
   CheckArgs_rA check(args, m_erh);
   args.getResult() = RexxString(::tanh((double)args.getArg(0)));
}
#endif

/*--------------------------------------------------------------BuiltIn::time-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::time(Arguments & args)
{
   char fmtOut = args.getOptionOrDefault(0, 'N');
   MemStream result;
   TimeClock * pUserClock = 0;
   TimeClock * pWhatClock;

   /* The date/time is 'frozen' throughout a clause. */
   if (!m_isNowClockFrozen) {
      m_isNowClockFrozen = true;
      m_nowClock.set();
   }

   if (args.getCount() > 3) {
      m_erh << ECE__ERROR << _REX__40_4 << "TIME" << '3' << endm;
   }

   // What clock should we use? current? user defined?
   if (!args.isPresent(1)) {
      // If there is no second argument, the current time is returned.
      if (args.isPresent(2)) {
         // If the third argument is given then the second is mandatory.
         m_erh << ECE__ERROR << _REX__40_19 << "" << ""
             << args.getArg(2) << endm;
      }
      pWhatClock = &m_nowClock;
   }else {
      // If there is a second argument it provides the time to be converted.
      int secs = -1;
      char fmtIn = args.getOptionOrDefault(2, 'N');
      if ((fmtIn == 'E') || (fmtIn == 'R') || (fmtIn == 'O')) {
         m_erh << ECE__ERROR << _REX__40_29 << "" << fmtOut << endm;
      }
      switch (fmtIn) {
      case 'C':
         secs = TimeClock::getDaySecsCivil(args.getArg(1));
         break;
      case 'H':
         secs = TimeClock::getDaySecsHours(args.getArg(1));
         break;
      case 'L':
         secs = TimeClock::getDaySecsNormal(args.getArg(1));
         break;
      case 'M':
         secs = TimeClock::getDaySecsMins(args.getArg(1));
         break;
      case 'N':
         secs = TimeClock::getDaySecsNormal(args.getArg(1));
      case 'S':
         secs = TimeClock::getDaySecsSecs(args.getArg(1));
         break;
      default:
         m_erh << ECE__ERROR << _REX__40_28 << ""
               << '3' << "CHLMNS" << fmtIn << endm;
         break;
      }
      if (secs < 0) {
         m_erh << ECE__ERROR << _REX__40_19 << ""
             << args.getArg(1) << fmtIn << endm;
      }
      pWhatClock = pUserClock = new TimeClockSecs(secs);
   }

   switch (fmtOut) {
   case 'C': pWhatClock->formatCivilTime(result);                      break;
   case 'H': pWhatClock->formatHours(result);                          break;
   case 'L': pWhatClock->formatLongTime(result);                       break;
   case 'M': pWhatClock->formatMinutes(result);                        break;
   case 'N': pWhatClock->formatNormalTime(result);                     break;
   case 'S': pWhatClock->formatSeconds(result);                        break;
   case 'E': m_rtne.getElapsedTime(m_nowClock, result);                break;
   case 'R': m_rtne.setElapsedTime(m_nowClock, result);                break;
   case 'O': result << (m_nowClock.getGmtAdjust() * 3600) << "000000"; break;
   default:
      if (pUserClock) delete pUserClock;
      m_erh << ECE__ERROR << _REX__40_28 << ""
            << '3' << "CEHLMNORS" << fmtOut << endm;
      break;
   }
   args.getResult() = RexxString(result.str(), result.rdbuf()->in_avail());
   result.rdbuf()->freeze(0);   // thaw
   if (pUserClock) delete pUserClock;
}

/*-------------------------------------------------------------BuiltIn::trace-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::trace(Arguments & args) {
   CheckArgs_oO check(args, 'N', m_erh);
   args.getResult() = m_tracer.getTraceSetting();
   if (args.isPresent(0) && !m_tracer.setTraceSetting(args.getArg(0), true)) {
      m_erh << ECE__ERROR << _REX__40_0 << endm;
   }
}

/*---------------------------------------------------------BuiltIn::translate-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::translate(Arguments & args)
{
   CheckArgs_rA_oA_oA_oPad check(args, m_erh);
   if (args.getCount() == 1) {
      args.getResult() = RexxString(args.getArg(0), CharConverter::Upper);
   }else {
      if (!args.isPresent(1) && !args.isPresent(2)) {
         args.getResult() = RexxString(check.pad, args.getArg(0).length());
      }else {
         if (!args.isPresent(1)) {  // only tablei
            args.getResult() = RexxString(
               args.getArg(0),
               DynamicCharConverter(args.getArg(2), check.pad)
            );
         }else if (!args.isPresent(2)) {  // only tableo
            args.getResult() = RexxString(
               args.getArg(0),
               DynamicCharConverter(check.pad, args.getArg(1))
            );
         }else {
            args.getResult() = RexxString(
               args.getArg(0),
               DynamicCharConverter(args.getArg(2), check.pad, args.getArg(1))
            );
         }
      }
   }
}

/*-------------------------------------------------------------BuiltIn::trunc-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::trunc(Arguments & args) {
   CheckArgs_rA_oWpz check(args, m_erh);
   if (check.n == -1) check.n = 0;
   args.getResult().truncate(
      args.getArg(0), check.n, m_rtne.getDecRexxContext()
   );
}

/*-------------------------------------------------------------BuiltIn::value-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::value(Arguments & args)
{
   CheckArgs_rA_oA_oA check(args, m_erh);

   if (args.isPresent(2)) {     // An external pool?
      Pool * pool = m_varhdlr.getPool(args.getArg(2));
      if (!pool) {
         m_erh << ECE__ERROR << _REX__40_37 << args.getArg(2) << endm;
      }
      args.getResult() = pool->get(args.getArg(0));
      if (args.isPresent(1) && !pool->set(args.getArg(0), args.getArg(1))) {
         m_erh << ECE__ERROR << _REX__40_36 << args.getArg(0) << endm;
      }
   }else if (args.isPresent(1)) {
      args.getResult() = m_varhdlr.setValue(args.getArg(0), args.getArg(1));
   }else {
      args.getResult() = m_varhdlr.getValue(args.getArg(0));
   }
}

/*-----------------------------------------------------------BuiltIn::vardump-+
|                                                                             |
+----------------------------------------------------------------------------*/
/*
| void BuiltIn::vardump(Arguments & args) {
| {
|    bool isHex = false;;
|
|    if (args.getCount() > 2) {
|       m_erh << ECE__ERROR << _REX__40_0 << endm;
|    }
|    if (args.getCount() == 2) {
|       switch (args.getArg(1).charAt(0)) {
|       case 'H': case 'X': case 'h': case 'x':
|          isHex = true;
|          break;
|       }
|    }
|    if (!args.isPresent(0)) {
|       m_varhdlr.dumpVarmap(args.getResult(), isHex);
|    }else if (args.getArg(0).datatype('S')) {
|       bool found;
|       RexxString name(args.getArg(0));
|       name.upper();
|       Variable * var = m_varhdlr.find2(name, found);
|       if (found) {
|          var->dump2(args.getResult(), isHex);
|       }
|    }
| }
*/

/*------------------------------------------------------------BuiltIn::verify-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::verify(Arguments & args)
{
   bool match = false;
   CheckArgs_rA_rA_oO_oWp check(args, m_erh);
   if (check.isOptionSpecified) {
      switch (check.option) {
      case 'M':          // Match
         match = true;
         break;
      case 'N':          // Nomatch
         match = false;
         break;
      default:
         m_erh << ECE__ERROR << _REX__40_0 << endm;
         break;
      }
   }
   args.getResult() = RexxString(
      1 + args.getArg(0).verify(args.getArg(1), match, check.start-1)
   );
}

/*--------------------------------------------------------------BuiltIn::word-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::word(Arguments & args) {
   CheckArgs_rA_rWp check(args, m_erh);
   args.getResult().wordAt(args.getArg(0), check.n);
}

/*---------------------------------------------------------BuiltIn::wordindex-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::wordindex(Arguments & args) {
   CheckArgs_rA_rWp check(args, m_erh);               // index C->Rexx
   args.getResult() = RexxString(1+args.getArg(0).wordIndex(check.n));
}

/*--------------------------------------------------------BuiltIn::wordlength-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::wordlength(Arguments & args) {
   CheckArgs_rA_rWp check(args, m_erh);
   args.getResult() = RexxString(args.getArg(0).wordLength(check.n));
}

/*-----------------------------------------------------------BuiltIn::wordpos-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::wordpos(Arguments & args) {
   CheckArgs_rA_rA_oWp check(args, m_erh);
   args.getResult() = RexxString(
      args.getArg(0).wordpos(args.getArg(1), check.start)
   );
}

/*-------------------------------------------------------------BuiltIn::words-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::words(Arguments & args) {
   CheckArgs_rA check(args, m_erh);
   args.getResult() = RexxString(args.getArg(0).wordsCount());
}

/*---------------------------------------------------------------BuiltIn::x2b-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::x2b(Arguments & args)
{
   CheckArgs_rA check(args, m_erh);
   int srcLen = args.getArg(0).length();
   int resLen = 1 + (srcLen * 4);
   char * bufOut = new char[resLen];
   char * d_p = RexxString::hexToBin(args.getArg(0), srcLen, bufOut, &resLen);
   if (!d_p) {
      m_erh << ECE__ERROR << _REX__40_0 << endm;   // hex string is invalid
   }
   args.getResult() = RexxString(d_p, resLen);
   delete [] bufOut;
}


/*---------------------------------------------------------------BuiltIn::x2c-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::x2c(Arguments & args) {
   CheckArgs_rA check(args, m_erh);
   int srcLen = args.getArg(0).length();
   int resLen = 1 + (srcLen/2);
   char * bufOut = new char[resLen];
   char *d_p = RexxString::hexToChar(args.getArg(0), srcLen, bufOut, &resLen);
   if (!d_p) {
      m_erh << ECE__ERROR << _REX__40_0 << endm;   // hex string is invalid
   }
   args.getResult() = RexxString(d_p, resLen);
   delete [] bufOut;
}

/*---------------------------------------------------------------BuiltIn::x2d-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::x2d(Arguments & args)
{
   CheckArgs_rA_oWpz check(args, m_erh);
   if (check.n == 0) {
      args.getResult() = RexxString("0", 1);
   }else {
      int srcLen = args.getArg(0).length();
      bool isNegative;
      int n;
      int resLen;
      if ((check.n == -1) || (check.n > srcLen)) {
         n = srcLen;
         isNegative = false;
      }else {
         n = check.n;
         isNegative = (args.getArg(0)[srcLen-n] >='8'); // [a-f] is > 8
      }
      resLen = 1 + (n/2);
      char * bufOut = new char[resLen];
      char * c_p = RexxString::hexToChar(
         ((char const *)args.getArg(0))+srcLen-n, n, bufOut, &resLen
      );
      if (!c_p) {
         m_erh << ECE__ERROR << _REX__40_0 << endm;   // hex string is invalid
      }
      if (isNegative && (n & 1)) *c_p |= 0xF0;
      args.getResult().c2d(
         c_p,
         resLen,
         isNegative,
         m_rtne.getDecRexxContext()
      );
      delete [] bufOut;
   }
}

#if !defined COM_JAXO_YAXX_DENY_XML
/*---------------------------------------------------------BuiltIn::xmlattval-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::xmlattval(Arguments & args)
{
   CheckArgs_rA_rA check(args, m_erh);
   args.getResult().attval(args.getArg(0), args.getArg(1));
}
#endif

#if !defined COM_JAXO_YAXX_DENY_XML
/*-------------------------------------------------------------BuiltIn::xmlin-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::xmlin(Arguments & args) {
   if (args.getCount() > 1) {
      m_erh << ECE__ERROR << _REX__40_0 << endm;
   }
   Signaled sig;
   args.getResult() = m_xmlIo.xmlin(args.getArg(0), sig);
   m_rtne.raise(sig);
}
#endif

#if !defined COM_JAXO_YAXX_DENY_XML
/*------------------------------------------------------------BuiltIn::xmlout-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::xmlout(Arguments & args) {
   CheckArgs_oA_oA check(args, m_erh);
   Signaled sig;
   args.getResult() = RexxString(
      m_xmlIo.xmlout(args.getArg(0), args.getArg(1), sig)
   );
   m_rtne.raise(sig);
}
#endif

#if !defined COM_JAXO_YAXX_DENY_XML
/*--------------------------------------------------------BuiltIn::xmltagname-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::xmltagname(Arguments & args) {
   CheckArgs_rA check(args, m_erh);
   args.getResult().tagname(args.getArg(0));
}
#endif

#if !defined COM_JAXO_YAXX_DENY_XML
/*-----------------------------------------------------------BuiltIn::xmltype-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::xmltype(Arguments & args) {
   CheckArgs_rA_oO check(args, m_erh);
   if (check.isOptionSpecified) {
      bool result;
      switch (check.option) {
      case 'S':  result = args.getArg(0).isEvRegStartTag();   break;
      case 'I':  result = args.getArg(0).isEvInfStartTag();   break;
      case 'V':  result = args.getArg(0).isEvRegEmptyTag();   break;
      case 'J':  result = args.getArg(0).isEvInfEmptyTag();   break;
      case 'E':  result = args.getArg(0).isEvRegEndTag();     break;
      case 'K':  result = args.getArg(0).isEvInfEndTag();     break;
      case 'T':  result = args.getArg(0).isEvText();          break;
      case 'D':  result = args.getArg(0).isEvData();          break;
      case 'P':  result = args.getArg(0).isEvPi();            break;
      case 'C':  result = args.getArg(0).isEvComment();       break;
      case '{':  result = args.getArg(0).isEvStartParse();    break;
      case '}':  result = args.getArg(0).isEvEndParse();      break;
      case '?':  result = args.getArg(0).isEvBad();           break;
      default:
         result = false;
         m_erh << ECE__ERROR << _REX__40_0 << endm;
         break;
      }
      args.getResult().assign(result);
   }else {
      args.getResult().assign(args.getArg(0).xmltype());
   }
}
#endif

/*------------------------------------------------------------BuiltIn::xrange-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::xrange(Arguments & args) {
   CheckArgs_oA_oA check(args, m_erh);
   args.getResult().xrange(
      (unsigned char)(args.isPresent(0)? args.getPad(0, m_erh) : 0),
      (unsigned char)(args.isPresent(1)? args.getPad(1, m_erh) : 0xFF)
   );
}

/*---------------------------------------------------------------BuiltIn::b2x-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::b2x(Arguments & args)
{
   CheckArgs_rA check(args, m_erh);
   bool isOdd;
   int srcLen = args.getArg(0).length();
   int resLen = 1 + (srcLen/8);
   char * bufOut = new char[resLen];
   char *d_p = RexxString::binToChar(    // fill bufOut's last bytes
      args.getArg(0), srcLen, bufOut, &resLen, &isOdd
   );
   if (!d_p) {
      m_erh << ECE__ERROR << _REX__40_0 << endm;  // bin is invalid
   }
   RexxString res(d_p, resLen);
   delete [] bufOut;
   resLen = 2 * srcLen;
   if (isOdd) --resLen;
   args.getResult().c2x(res, resLen);
}

/*---------------------------------------------------------------BuiltIn::arg-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BuiltIn::arg(Arguments & args)
{
   if (args.getCount() == 0) {
      args.getResult().assign(m_rtne.m_args.getCount());
   }else {
      CheckArgs_rWargno_oO check(args, m_erh);
      if (!check.isOptionSpecified) {
         args.getResult() = m_rtne.m_args.getArg(check.argNo-1);
      }else {
         switch (check.option) {
         case 'E':
            args.getResult() = RexxString(
               m_rtne.m_args.isPresent(check.argNo-1)? 1 : 0
            );
            break;
         case 'O':
            args.getResult() = RexxString(
               m_rtne.m_args.isPresent(check.argNo-1)? 0 : 1
            );
            break;
         default:
            m_erh << ECE__ERROR << _REX__40_0 << endm;
            break;
         }
      }
   }
}

#ifdef YAXX_NAMESPACE
}
#endif
/*===========================================================================*/
