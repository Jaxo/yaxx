/*
* $Id: ucformat.h,v 1.13 2011-07-29 10:26:38 pgr Exp $
*
* Group in specific classes formatting of Unicode (string / streams)
*/

#ifndef COM_JAXO_TOOLS_UCFORMAT_H_INCLUDED
#define COM_JAXO_TOOLS_UCFORMAT_H_INCLUDED

/*---------+
| Includes |
+---------*/
#if defined __MWERKS__
#include "ios.h"
#include "iostream.h"  //JAL Have to use "" instead of <> for MWERKS
#else
#include <iostream>
#endif

#include "toolsgendef.h"

#include "migstream.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

typedef unsigned short UCS_2;

/*-----------------------------------------------------------------TpManipInt-+
|                                                                             |
+----------------------------------------------------------------------------*/
class TpManipInt {
public:
   class Set {
   public:
      virtual void setWidth(int) = 0;
   };
   TpManipInt(void (Set::*f)(int), int v) : fct(f), val(v) { }
   void (Set::*fct)(int);
   int val;
};

inline TpManipInt setW(int w) {
   return TpManipInt(&TpManipInt::Set::setWidth, w);
}

/*-----------------------------------------------------------UnicodeExtractor-+
|                                                                             |
+----------------------------------------------------------------------------*/
class TOOLS_API UnicodeExtractor : private TpManipInt::Set {
public:
   UnicodeExtractor() :
      state(0), x_flags(ios::skipws | ios::dec), x_width(-1)
   {}
   virtual ~UnicodeExtractor() {}

   long flags() const   { return x_flags; }
   long flags(long f)   { long old = x_flags; x_flags = f; return old; }
   long setf(long f)    { long old = x_flags; x_flags |= f; return old; }
   long unsetf(long f)  { long old = x_flags; x_flags &= ~f; return old; }
   long setf(long f, long s) { return flags((x_flags & ~s) | f); }
   int  width() const   { return x_width; }
   int  width(int w)    { int old = x_width; x_width = w; return old; }
   int  fail() const    { return state & ios::failbit; }
   int  eof()  const    { return state & ios::eofbit; }
   int  good() const    { return state == 0; }

   virtual int peek(int) { return -1; }     // EOF_SIGNAL
   virtual int skip(int) { return 0; }

   // valid for ASCII 7bits only: this is a format operation.
   static bool isHexDigit(UCS_2 uc);
   static bool isSpace(UCS_2 uc);
   static bool isDigit(UCS_2 uc);

   UnicodeExtractor & operator>>(UCS_2 *);
   UnicodeExtractor & operator>>(short &);
   UnicodeExtractor & operator>>(unsigned short &);
   UnicodeExtractor & operator>>(int &);
   UnicodeExtractor & operator>>(unsigned int &);
   UnicodeExtractor & operator>>(long &);
   UnicodeExtractor & operator>>(unsigned long &);

protected:
   int state;
   long x_flags;
   int x_width;

private:
   enum e_Sign { NO_SIGN, POSITIVE, NEGATIVE };
   enum e_IntegerType {
      IT_SHORT,
      IT_USHORT,
      IT_INT,
      IT_UINT,
      IT_LONG,
      IT_ULONG
   };
   static int ucToDigit(UCS_2 k);
   UCS_2 eatWhiteSpace();
   int getValue(e_IntegerType);
   void setWidth(int w) { x_width = w; }

public:   // public that no one wants to use
   UnicodeExtractor & operator>>(void (*f)(UnicodeExtractor &)) {
      f(*this);
      return *this;
   }
   UnicodeExtractor & operator>>(TpManipInt const & m) {
      (this->*m.fct)(m.val);
      return *this;
   }

   friend void ws(UnicodeExtractor & s) {
      s.x_flags &= ~ios::skipws;
   }
   friend void dec(UnicodeExtractor & s) {
      s.x_flags &= ~(ios::oct | ios::hex);
      s.x_flags |= ios::dec;
   }
   friend void oct(UnicodeExtractor & s) {
      s.x_flags &= ~(ios::dec | ios::hex);
      s.x_flags |= ios::oct;
   }
   friend void hex(UnicodeExtractor & s) {
      s.x_flags &= ~(ios::dec | ios::oct);
      s.x_flags |= ios::hex;
   }
};

void ws(UnicodeExtractor & s);
void dec(UnicodeExtractor & s);
void oct(UnicodeExtractor & s);
void hex(UnicodeExtractor & s);

/* -- INLINES -- */
inline UnicodeExtractor & UnicodeExtractor::operator>>(short & result) {
   result = getValue(IT_SHORT);
   return *this;
}
inline UnicodeExtractor & UnicodeExtractor::operator>>(unsigned short & result){
   result = getValue(IT_USHORT);
   return *this;
}
inline UnicodeExtractor & UnicodeExtractor::operator>>(int & result) {
   result = getValue(IT_INT);
   return *this;
}
inline UnicodeExtractor & UnicodeExtractor::operator>>(unsigned int & result) {
   result = getValue(IT_UINT);
   return *this;
}
inline UnicodeExtractor & UnicodeExtractor::operator>>(long & result) {
   result = getValue(IT_LONG);
   return *this;
}
inline UnicodeExtractor & UnicodeExtractor::operator>>(unsigned long & result) {
   result = getValue(IT_ULONG);
   return *this;
}
inline bool UnicodeExtractor::isSpace(UCS_2 uc) {
   if ((uc == 0x20) || (uc == 0x9) || (uc == 0xA) || (uc == 0xD)) {
      return true;
   }else {
      return false;
   }
}
inline bool UnicodeExtractor::isDigit(UCS_2 uc) {
   if ((uc >= 0x30) && (uc < 0x40)) return true; else return false;
}

#ifdef TOOLS_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
