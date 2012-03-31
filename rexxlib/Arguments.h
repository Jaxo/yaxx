/* $Id: Arguments.h,v 1.38 2002-07-28 16:28:33 pgr Exp $ */

#ifndef COM_JAXO_YAXX_ARGUMENTS_H_INCLUDED
#define COM_JAXO_YAXX_ARGUMENTS_H_INCLUDED

/*---------+
| Includes |
+---------*/
#ifdef __MWERKS_NEEDS_CTYPE_FIX__
#include "../../palmiolib/ctype_fix.h"
#else
#include <ctype.h>
#endif

#include "../toolslib/uccompos.h"
#include "RexxString.h"

#ifdef YAXX_NAMESPACE
namespace YAXX_NAMESPACE {
#endif

/*---------------------------------------------------------- class Arguments -+
|                                                                             |
+----------------------------------------------------------------------------*/
class Arguments {
public:
   typedef unsigned short PresenceBits;
   enum { MAXARGS = (8 * sizeof (PresenceBits)) };
   Arguments(RexxString & strArgs);
   Arguments(
      RexxString ** stack,
      PresenceBits presenceBits,
      int & base
   );
   static RexxString catenateArgs(
      RexxString const & cmd,
      RexxString ** stack,
      PresenceBits presenceBits,
      int & base
   );
   int getCount() const;
   RexxString const * const * getArgs() const;
   RexxString & getResult();
   RexxString & getArg(int i);
   int getInt(int i) const;
   char getPad(int i, UnicodeComposer & erh) const;
   char getOptionArg(int i, UnicodeComposer & erh) const;
   char getOptionOrDefault(int i, char deflt) const;
   bool isPresent(int i) const;


private:
   int m_n;                    // number of args
   RexxString * m_a[MAXARGS];  // argument pointers (constructed by m_r)
   RexxString & m_r;           // returned

   RexxString & init(RexxString **, PresenceBits, int &);
   Arguments & operator=(Arguments const & source);  // undefined
};

/* -- INLINES -- */
inline int Arguments::getCount() const {
   return m_n;
}
inline RexxString const * const * Arguments::getArgs() const {
   return m_a;
}
inline RexxString & Arguments::getResult() {
   return m_r;
}
inline RexxString & Arguments::getArg(int i) {
   return m_a[i]? *m_a[i] : RexxString::Nil;
}
inline char Arguments::getPad(int i, UnicodeComposer & erh) const {
   if (m_a[i]->length() != 1) {
      erh << ECE__ERROR << _REX__40_0 << endm;
   }
   return m_a[i]->charAt(0);
}
inline int Arguments::getInt(int i) const {
   return *m_a[i];
}
inline char Arguments::getOptionArg(int i, UnicodeComposer & erh) const {
   if (m_a[i]->length() < 1) {
      erh << ECE__ERROR << _REX__40_0 << endm;
   }
   char c = m_a[i]->charAt(0);
   return islower(c)? c += 'A' - 'a' : c;
}
inline char Arguments::getOptionOrDefault(int i, char deflt) const {
   if (m_a[i] == 0) {
      return deflt;
   }else if (m_a[i]->length()) {
      char c = m_a[i]->charAt(0);
      return islower(c)? c += 'A' - 'a' : c;
   }else {
      return '\0';   // empty string
   }
}
inline bool Arguments::isPresent(int i) const {
   return m_a[i] != 0;
}

#ifdef YAXX_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
