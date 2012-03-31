/* $Id: RexxString.h,v 1.74 2011-07-29 10:26:35 pgr Exp $ */

#ifndef COM_JAXO_YAXX_REXXSTRING_H_INCLUDED
#define COM_JAXO_YAXX_REXXSTRING_H_INCLUDED

/*---------+
| Includes |
+---------*/
#ifdef __MWERKS__
#include "iostream.h"
#else
#include <iostream>
#endif

#include <assert.h>

#if !defined COM_JAXO_YAXX_DENY_MATH   // Math extensions (double)
#include <float.h>
#endif

#include <new>
#include "../toolslib/StringBuffer.h"
#include "../toolslib/ByteString.h"

#if !defined COM_JAXO_YAXX_DENY_XML
#include "../toolslib/ucstring.h"
#include "../yasp3lib/yasp3.h"
#endif

#include "rexxgendef.h"
#include "../decnblib/DecRexx.h"
#include "CharConverter.h"

#ifdef YAXX_NAMESPACE
namespace YAXX_NAMESPACE {
#endif

/*--------------------------------------------------------- class RexxString -+
|                                                                             |
+----------------------------------------------------------------------------*/
class REXX_API RexxString : public ByteString
{
   friend REXX_API ostream & operator<<(ostream &, RexxString const &);
public:
   static RexxString Nil;
   enum Kill {
      KillLEAD = 1<<0, KillTRAIL = 1<<1, KillBOTH = KillLEAD | KillTRAIL
   };
   enum COMPARE_IF {
      COMPARE_IF_GT = 1, // 0b001
      COMPARE_IF_EQ = 2, // 0b010
      COMPARE_IF_GE = 3, // 0b011
      COMPARE_IF_LT = 4, // 0b100
      COMPARE_IF_NE = 5, // 0b101
      COMPARE_IF_LE = 6 // 0b110
   };

   enum COMPARE_RESULT {
      IS_GT = COMPARE_IF_GT,
      IS_EQ = COMPARE_IF_EQ,
      IS_LT = COMPARE_IF_LT
   };

   enum { NOMANSLAND=5, THRESHOLD=15, MINSIZE=20 };

   RexxString();
   RexxString(char const * buf);
   RexxString(char const * buf, int len);
   RexxString(char c, int len);
   RexxString(char c);
   RexxString(int val);
   RexxString(bool val);

   RexxString(RexxString const & source, CharConverter const & x);
   RexxString(StringBuffer const & buf);

   #if !defined COM_JAXO_YAXX_DENY_XML
      RexxString(YaspEvent const & ev);
      RexxString(UnicodeString const & ucs);
   #endif

   RexxString & operator=(RexxString const * pSource);  // deprecated

   void assign(char const * buf, int len);
   void assign(char c);
   void assign(int val);
   void assign(bool b);

   char * resetString(int len);

   operator char const *() const;
   operator int () const;
   operator bool () const;

   #if !defined COM_JAXO_YAXX_DENY_MATH   // Math extensions (double)
   RexxString(double val);
   void assign(double val);
   operator double () const;
   #endif

   bool isInt(int & whatInt) const;
   bool isWhole(DecRexx & whatWhole) const;
   bool isSymbol() const;
   bool isBinary() const;
   bool isAlphanum() const;
   bool isAlpha() const;
   bool isLowerAlpha() const;
   bool isUpperAlpha() const;
   bool isNumber() const;
   bool isHexa() const;
   bool isZero() const;
   bool isNegative() const;

   int length() const;
   int sign(DecRexxContext const &) const;
   char charAt(int i) const;
   bool contains(char c, int len=0) const;

   int verify(RexxString const & ref, bool isMatch, int start) const;
   int wordLength(int wordNo) const;
   int wordIndex(int wordNo) const ;
   int wordsCount() const;
   bool abbrev(RexxString const & info, int len) const;
   int pos(RexxString const & source, int pos) const;
   int wordpos(RexxString const & source, int wordNo) const;
   int lastpos(RexxString const & source, int pos) const;
   int countstr(RexxString const & needle) const;        // not X3J18

   bool operator==(RexxString const & compnd) const;
   COMPARE_RESULT normalCompare(RexxString const &, DecRexxContext const &) const;
   COMPARE_RESULT strictCompare(RexxString const & str) const;
   int compare(RexxString const & str, char pad) const;  // built-in
   static int max(int, RexxString const * const *, DecRexxContext const &);
   static int min(int, RexxString const * const *, DecRexxContext const &);

   // dyadic arithmetic operations
   bool add(RexxString const &, RexxString const &, DecRexxContext const &);
   bool subtract(RexxString const &, RexxString const &, DecRexxContext const &);
   bool multiply(RexxString const &, RexxString const &, DecRexxContext const &);
   bool divide(RexxString const &, RexxString const &, DecRexxContext const &);
   bool divideInteger(RexxString const &, RexxString const &, DecRexxContext const &);
   bool remainder(RexxString const &, RexxString const &, DecRexxContext const &);
   bool power(RexxString const &, RexxString const &, DecRexxContext const &);

   // monadic arithmetic operations
   void plus(RexxString const & str1, DecRexxContext const &);
   void minus(RexxString const & str1, DecRexxContext const &);
   void abs(RexxString const & str1, DecRexxContext const &);
   void truncate(RexxString const & str1, int len, DecRexxContext const &);
   bool sqrt(RexxString const & str1, DecRexxContext const &);

   // 'in-place' arithmetic operations
   void increment(DecRexxContext const &);
   void decrement(DecRexxContext const &);

   // String operations
   void convert(CharConverter const & x);

   void reverse(RexxString const & str1);
   bool format(
      RexxString const & str1,
      int bef, int aft, int expp, int expt,
      DecRexxContext const &
   );
   void c2d(char const *, int, bool, DecRexxContext const &);
   void d2c(DecRexx const & whole, int n);
   void c2x(RexxString const & str1, int n=0);
   void d2x(DecRexx const & whole, int n);

   void changestr(
      RexxString const & str1,
      RexxString const & rplcd,
      RexxString const & rplmt
   );
   void delstr(RexxString const & str1, int start, int len);
   void delword(RexxString const & str1, int wordNo, int wordCount);
   void subword(RexxString const & str1, int wordNo, int wordCount);
   void insert(
      RexxString const & str1,
      RexxString const &, int pos, int len, char pad
   );
   void overlay(
      RexxString const & str1,
      RexxString const &, int pos, int len, char pad
   );
   void substr(RexxString const & str1, int pos, int len, char pad);
   void strip(RexxString const & str1, Kill options, char pad);
   void wordAt(RexxString const & str1, int wordNo);
   void bitIt(
      RexxString const & str1,
      RexxString const & str2, int pad, char op
   );
   void center(RexxString const & str1, int len, char pad);
   void left(RexxString const & str1, int len, char pad);
   void right(RexxString const & str1, int len, char pad);
   void wordsSpace(RexxString const & str1, int len, char pad);
   void copies(RexxString const & str1, int n);
   void concat(RexxString const & str1, RexxString const & str2);
   void bconcat(RexxString const & str1, RexxString const & str2);
   void xrange(unsigned char start, unsigned char stop);

   #if !defined COM_JAXO_YAXX_DENY_XML
      // YASP3 extensions
      void tagname(RexxString const & str1);
      void attval(RexxString const & str1, RexxString const & str2);
      bool isEvRegStartTag() const;
      bool isEvInfStartTag() const;
      bool isEvRegEmptyTag() const;
      bool isEvInfEmptyTag() const;
      bool isEvRegEndTag() const;
      bool isEvInfEndTag() const;
      bool isEvText() const;
      bool isEvData() const;
      bool isEvPi() const;
      bool isEvComment() const;
      bool isEvStartParse() const;
      bool isEvEndParse() const;
      bool isEvBad() const;
      char xmltype() const;
   #endif

   static char * hexToChar(char const *, int, char *, int *);
   static char * binToChar(char const *, int, char *, int *, bool *);
   static char * hexToBin(char const *, int, char *, int *);

   class REXX_API Exception {
   public:
      enum Reason {
         NON_NUMERIC_REASON,
         NON_INTEGER_REASON,
         NON_BOOLEAN_REASON,
         DIVIDE_BY_ZERO_REASON,
         UNDERFLOW_REASON,
         OVERFLOW_REASON,
         BAD_ARITH_REASON
      };
      Exception(Reason r) : m_r(r) {}
      Reason getReason() const { return m_r; }
   private:
      Reason m_r;
   };

private:
   #if !defined COM_JAXO_YAXX_DENY_XML
   class Stringizer : public YaspEventInspector {
   public:
      Stringizer(RexxString & res) : m_res(res) {}
      void extractTagName(Tag const * pTag);
      void extractAttributeValue(Tag const * pTag, char const * pAttrName);
   private:
      void setString(UCS_2 const * pUc, int iLen);
      void str(MultibyteMemWriter & mbost);
      void inspectTagEmpty(Tag const & tag);
      void inspectTagStart(Tag const & tag);
      void inspectTagEnd(Tag const & tag);
      void inspectText(TextData const & text);
      void inspectSdataText(TextData const & text);
      void inspectCdataText(TextData const & text);
      void inspectPiText(TextData const &);
      void inspectCommentText(TextData const &);
      RexxString & m_res;
   };
   #endif

   class Rep : public ByteString::Rep {
   public:
      Rep();
      Rep(NewParam & p);
      static Rep * alloc(int size);
      bool ensureNumber() const;
      #if !defined COM_JAXO_YAXX_DENY_XML
         YaspEvent m_ev;          // YASP3 event
      #endif
      int m_capa;                 // original length of the string
      DecRexx m_rxn;
   };


   RexxString(void ***);           // "Nil" constructor

   void resetNumber();
   void resetNumberKeep();
   void ensureString() const;
   static bool examineStatus(int status);
};

REXX_API ostream & operator<<(ostream & out, RexxString const & rcs);

inline RexxString::operator char const *() const {
   ensureString();
   return m_pRep->m_pch;
}

inline int RexxString::length() const {
   ensureString();
   return m_pRep->m_len;
}

#ifdef YAXX_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
