/* $Id: RexxString.cpp,v 1.119 2011-07-29 10:26:35 pgr Exp $ */

// -> RexxStringFlash
// -> resetNumber, string, etc are members of Rep
// -> decNumberAdd(foo, zero, foo) can be enhanced (if ctxt->digits == ??)
// -> dyadic and monadic ust use the same code...

#ifdef __MWERKS_NEEDS_CTYPE_FIX__
#include "../../palmiolib/ctype_fix.h"
#else
#include <ctype.h>
#endif

#include <iomanip>
#include <string.h>
#include <assert.h>
#include "RexxString.h"
#include "Tokenizer.h"
#include "../toolslib/StringBuffer.h"

#ifdef __MWERKS_NEEDS_MEMCMP_FIX__
#define memcmp memcmpFix4PalmOS30
#include "../../palmiolib/memcmp.h"
#endif

#ifdef YAXX_NAMESPACE
namespace YAXX_NAMESPACE {
#endif

static char const hexChars[] = "0123456789ABCDEF";
static RexxString::COMPARE_RESULT const cifs[] = {
   RexxString::IS_LT,
   RexxString::IS_EQ,
   RexxString::IS_GT
};

RexxString RexxString::Nil((void ***)0);   // *not* refcounted (should be const)

/*-------------------------------------------------------RexxString::Rep::Rep-+
| Regular Constructor                                                         |
+----------------------------------------------------------------------------*/
RexxString::Rep::Rep(NewParam & p) :  ByteString::Rep(p), m_capa(m_len) {
}

/*-------------------------------------------------------RexxString::Rep::Rep-+
| Nil constructor                                                             |
+----------------------------------------------------------------------------*/
RexxString::Rep::Rep() :  ByteString::Rep(), m_capa(0) {
}

/*STATIC-----------------------------------------------RexxString::Rep::alloc-+
|                                                                             |
+----------------------------------------------------------------------------*/
#if defined _WIN32
#pragma warning(disable:4291)     // no matching delete
#endif
RexxString::Rep * RexxString::Rep::alloc(int bufSize) {
   NewParam p(bufSize, sizeof(Rep));
   return new(&p) Rep(p);
}
#if defined _WIN32
#pragma warning(default:4291)
#endif

/*
| Grouped hereunder methods that modify the m_len member of the ByteString::Rep.
| (except for setting m_len to -1 to designate unstringized strings)
| PGR>>> The names are not really good.
*/

/*----------------------------------------------------RexxString::resetString-+
|                                                                             |
+----------------------------------------------------------------------------*/
char * RexxString::resetString(int len)
{
   if (
      (m_pRep->iReferenceCount > 1) || (len > ((Rep *)m_pRep)->m_capa) // ||
//    (len < (m_pRep->m_capa - (THRESHOLD+NOMANSLAND)))
       //>>>PGR: FIXME - Implement the above.
   ) {
      /*
      | String is too small, or too big
      | Notice the "NOMANSLAND" value to avoid bouncing across the THRESHOLD
      */
      this->~RexxString();
      m_pRep = Rep::alloc(len+THRESHOLD);
   }else {
      ((Rep *)m_pRep)->m_rxn = DecRexx::Nil;  // invalidate number face
   }
   m_pRep->m_pch[len] = '\0';
   m_pRep->m_len = len;
   return m_pRep->m_pch;
}

void RexxString::resetNumber()
{
   if (m_pRep->iReferenceCount > 1) {
      this->~RexxString();
      m_pRep = Rep::alloc(MINSIZE);
   }
   m_pRep->m_len = -1;
}

void RexxString::resetNumberKeep() {
   if (m_pRep->iReferenceCount > 1) {
      DecRexx n(((Rep *)m_pRep)->m_rxn);
      this->~RexxString();
      m_pRep = Rep::alloc(MINSIZE);
      ((Rep *)m_pRep)->m_rxn = n;
   }
   m_pRep->m_len = -1;
}

bool RexxString::Rep::ensureNumber() const {
   if (m_rxn.isNil()) {
      if (!m_len) {
         return false;
      }else {
         ((Rep *)this)->m_rxn = DecRexx(m_pch);  // violates constness (OK)
      }
   }
   return (!m_rxn.isBad());
}

void RexxString::ensureString() const
{
   if (m_pRep->m_len < 0) {
      #if !defined COM_JAXO_YAXX_DENY_XML
         if (((Rep *)m_pRep)->m_ev.inqType() != YSP__noType) {
            YaspEvent ev(((Rep *)m_pRep)->m_ev);
            Stringizer s(*(RexxString *)this);
            ev.accept(s);
            ((Rep *)m_pRep)->m_ev = ev;
            return;
         }
      #endif
      assert(((Rep *)m_pRep)->ensureNumber());   // violates constness (OK)
      DecRexx::String rstr = ((Rep *)m_pRep)->m_rxn.format();
      int len = strlen(rstr);
      if (len > ((Rep *)m_pRep)->m_capa) {
         //|| (len < (m_pRep->m_capa - (THRESHOLD+NOMANSLAND)))
         //>>>PGR: FIXME - Implement the above.
         /*
         | String is too small, or too big
         | Notice the "NOMANSLAND" value to avoid bouncing across the THRESHOLD
         */
         DecRexx rxn(((Rep *)m_pRep)->m_rxn);
         ((RexxString *)this)->~RexxString();    // violates constness (OK)
         Rep * pRep = Rep::alloc(len+THRESHOLD);
         pRep->m_rxn = rxn;
         ((RexxString *)this)->m_pRep = pRep;    // violates constness (OK)
      }
      m_pRep->m_len = len;
      memcpy(m_pRep->m_pch, rstr, len+1);
   }
}

/*------------------------------------------------------------RexxString::c2x-+
|                                                                             |
+----------------------------------------------------------------------------*/
void RexxString::c2x(RexxString const & str1, int n)
{
   str1.ensureString();
   char const * c_p = str1.m_pRep->m_pch;
   int len = str1.m_pRep->m_len;
   char * bufOut = new char[1+2*len];
   char * d_p = bufOut;

   // remove leading if (n is defaulted and leading is 0x00) or (n is odd)
   if (((n==-1) && (--n, len) && (0x00 == (*c_p & 0xF0))) || (n & 1)) {
      *d_p++ = hexChars[(*c_p++) & 0x0F];
      --len;
   }
   while (len--) {
      char c = *c_p++;
      *d_p++ = hexChars[((c >> 4) & 0x0F)];
      *d_p++ = hexChars[c & 0x0F];
   }
   assign(bufOut, d_p - bufOut);
   delete [] bufOut;
}

/*----------------------------------------------------------RexxString::right-+
|                                                                             |
+----------------------------------------------------------------------------*/
void RexxString::right(RexxString const & str1, int len, char pad)
{
   str1.ensureString();
   int i = str1.m_pRep->m_len - len;
   RexxString s1(str1);
   resetString(len);
   char * c_p = m_pRep->m_pch;
   if (i >= 0) {
      memcpy(c_p, s1.m_pRep->m_pch+i, len);
   }else {
      memset(c_p, pad, -i);
      memcpy(c_p-i, s1.m_pRep->m_pch, s1.m_pRep->m_len);
   }
}

/*--------------------------------------------------------RexxString::overlay-+
|                                                                             |
+----------------------------------------------------------------------------*/
void RexxString::overlay(
   RexxString const & str1,
   RexxString const & toOver,
   int n,
   int len,
   char pad
) {
   str1.ensureString();
   toOver.ensureString();
   if (n < 0) n = 0;
   if (len < 0) len = toOver.m_pRep->m_len;

   int newLen = str1.m_pRep->m_len;
   int pos = n+len;
   int aft;
   int bef;
   if (newLen < pos) {
      bef = newLen;
      newLen = pos;
      aft = 0;
      if (bef > n) bef = n;
   }else {
      aft = newLen-pos;
      bef = n;
   }
   RexxString s1(str1);
   RexxString s2(toOver);
   resetString(newLen);
   char * c_p = m_pRep->m_pch;
   memset(c_p, pad, newLen);
   memcpy(c_p, s1.m_pRep->m_pch, bef);
   memcpy(c_p+n, s2.m_pRep->m_pch, s2.m_pRep->m_len);
   memcpy(c_p+pos, s1.m_pRep->m_pch+pos, aft);
}

/*--------------------------------------------------------RexxString::subword-+
|                                                                             |
+----------------------------------------------------------------------------*/
void RexxString::subword(RexxString const & str1, int wordNo, int wordCount)
{
   int i;

   str1.ensureString();
   if ((wordCount == 0) || (i = str1.m_pRep->wordIndex(wordNo), i < 0)) {
      *this = Nil;
   }else {
      int len = str1.m_pRep->m_len - i;
      char const * const c_p = str1.m_pRep->m_pch + i;
      if (wordCount > 0) {     // excluding the default value
         char const * const d_p = c_p + len;
         for (char const * p = c_p; p < d_p; ++p) {
            if (isSpace(*p)) {
               while ((++p < d_p) && isSpace(*p)) {}
            }else {
               while ((++p < d_p) && !isSpace(*p)) {}
               if (--wordCount == 0) {
                  len = p - c_p;
                  break;
               }
            }
         }
      }
      if (wordCount != 0) {    // including the default value
         char const * d_p = c_p + len;
         while (len && isSpace(*--d_p)) --len;
      }
      RexxString s1(str1);
      resetString(len);
      memcpy(m_pRep->m_pch, c_p, len);
   }
}

/*---------------------------------------------------------RexxString::copies-+
|                                                                             |
+----------------------------------------------------------------------------*/
void RexxString::copies(RexxString const & str1, int n)
{
   str1.ensureString();
   int len = str1.m_pRep->m_len * n;
   RexxString s1(str1);
   resetString(len);

   int lenFrom = s1.m_pRep->m_len;
   if (lenFrom == 1) {                 // usual case
      memset(m_pRep->m_pch, *s1.m_pRep->m_pch, len);
   }else {
      char const * c_p = s1.m_pRep->m_pch;
      char * d_p = m_pRep->m_pch;
      while (n--) {
         memcpy(d_p, c_p, lenFrom);
         d_p += lenFrom;
      }
   }
}

/*--------------------------------------------------------RexxString::reverse-+
|                                                                             |
+----------------------------------------------------------------------------*/
void RexxString::reverse(RexxString const & str1)
{
   if (!str1.exists()) {
      *this = Nil;
   }else {
      str1.ensureString();
      int len = str1.m_pRep->m_len;
      RexxString s1(str1);
      resetString(len);
      char const * c_p = s1.m_pRep->m_pch;
      char * d_p = m_pRep->m_pch + len;
      while (len--) {                 // reverse the string
         *--d_p = *c_p++;
      }
   }
}

/*----------------------------------------------------------RexxString::bitIt-+
|                                                                             |
+----------------------------------------------------------------------------*/
void RexxString::bitIt(
   RexxString const & str1,
   RexxString const & str2,
   int pad,
   char op
) {
   char const * c_p1;
   char const * d_p1;
   char const * c_p2;
   char const * d_p2;
   int len;

   // make sure c_p1 has the longest
   str1.ensureString();
   len = str1.m_pRep->m_len;
   if (str2.length()) {
      if (len < str2.m_pRep->m_len) {
         c_p2 = str1.m_pRep->m_pch;
         d_p2 = c_p2 + len;
         len =  str2.m_pRep->m_len;
         c_p1 = str2.m_pRep->m_pch;
      }else {
         c_p2 = str2.m_pRep->m_pch;
         d_p2 = c_p2 + str2.m_pRep->m_len;
         c_p1 = str1.m_pRep->m_pch;
      }
   }else {
      d_p2 = c_p2 = 0;
      c_p1 = str1.m_pRep->m_pch;
   }
   d_p1 = c_p1 + len;
   RexxString s1(*this);
   RexxString s2(str2);
   resetString(len);
   char * c_p = m_pRep->m_pch;

   switch (op) {
   case '^':
      while (c_p2 < d_p2) *c_p++ = *c_p1++ ^ *c_p2++;
      if (pad != -1) {
         while (c_p1 < d_p1) *c_p++ = *c_p1++ ^ pad;
      }else {
         while (c_p1 < d_p1) *c_p++ = *c_p1++;
      }
      break;
   case '&':
      while (c_p2 < d_p2) *c_p++ = *c_p1++ & *c_p2++;
      if (pad != -1) {
         while (c_p1 < d_p1) *c_p++ = *c_p1++ & pad;
      }else {
         while (c_p1 < d_p1) *c_p++ = *c_p1++;
      }
      break;
   default:  // case '|':
      while (c_p2 < d_p2) *c_p++ = *c_p1++ | *c_p2++;
      if (pad != -1) {
         while (c_p1 < d_p1) *c_p++ = *c_p1++ | pad;
      }else {
         while (c_p1 < d_p1) *c_p++ = *c_p1++;
      }
      break;
   }
}

/*---------------------------------------------------------RexxString::concat-+
|                                                                             |
+----------------------------------------------------------------------------*/
void RexxString::concat(RexxString const & str1, RexxString const & str2)
{
   RexxString s1(str1);
   RexxString s2(str2);
   s1.ensureString();
   s2.ensureString();
   int len1 = s1.m_pRep->m_len;
   int len2 = s2.m_pRep->m_len;
   resetString(len1+len2);
   memcpy(m_pRep->m_pch, s1.m_pRep->m_pch, len1);
   memcpy(m_pRep->m_pch + len1, s2.m_pRep->m_pch, len2);
}

/*--------------------------------------------------------RexxString::bconcat-+
|                                                                             |
+----------------------------------------------------------------------------*/
void RexxString::bconcat(RexxString const & str1, RexxString const & str2)
{
   RexxString s1(str1);
   RexxString s2(str2);
   s1.ensureString();
   s2.ensureString();
   int len1 = s1.m_pRep->m_len;
   int len2 = s2.m_pRep->m_len;
   resetString(len1+1+len2);
   memcpy(m_pRep->m_pch, s1.m_pRep->m_pch, len1);
   m_pRep->m_pch[len1] = ' ';
   memcpy(m_pRep->m_pch+len1+1, s2.m_pRep->m_pch, len2);
}

/*-----------------------------------------------------RexxString::wordsSpace-+
|                                                                             |
+----------------------------------------------------------------------------*/
void RexxString::wordsSpace(RexxString const & str1, int n, char pad)
{
   str1.ensureString();

   // two passes: 1- get the required length 2- fill up the target
   int len = 0;
   char const * c_p = str1.m_pRep->m_pch;
   char const * d_p = c_p + str1.m_pRep->m_len;

   while (c_p < d_p) {
      if (isSpace(*c_p)) {
         while ((++c_p < d_p) && isSpace(*c_p)) {}
      }else {
         if (len++) len += n;
         while ((++c_p < d_p) && !isSpace(*c_p)) ++len;
      }
   }
   RexxString s1(str1);
   resetString(len);
   if (len) {
      char * e_p = m_pRep->m_pch - 1;
      c_p = s1.m_pRep->m_pch;
      for (;;) {
         if (isSpace(*c_p)) {
            while (isSpace(*++c_p)) {}
         }else {
            do *++e_p = *c_p; while (--len && !isSpace(*++c_p));
            if (!len) break;
            for (int i=0 ; i < n; ++i) *++e_p = pad;
            len -= n;
         }
      }
   }
}

/*---------------------------------------------------------RexxString::assign-+
|                                                                             |
+----------------------------------------------------------------------------*/
void RexxString::assign(char const *p, int len)
{
   if (!len) {
      *this = RexxString::Nil;
   }else {
      assert (len > 0);
      resetString(len);
      memcpy(m_pRep->m_pch, p, len);
   }
}

/*---------------------------------------------------------RexxString::assign-+
|                                                                             |
+----------------------------------------------------------------------------*/
void RexxString::assign(char c)
{
   resetString(1);
   m_pRep->m_pch[0] = c;
}

#if !defined COM_JAXO_YAXX_DENY_MATH
/*---------------------------------------------------------RexxString::assign-+
|                                                                             |
+----------------------------------------------------------------------------*/
void RexxString::assign(double val)
{
   resetNumber();
   ((Rep *)m_pRep)->m_rxn = DecRexx(val);
}
#endif

/*---------------------------------------------------------RexxString::assign-+
|                                                                             |
+----------------------------------------------------------------------------*/
void RexxString::assign(int val)
{
   resetNumber();
   ((Rep *)m_pRep)->m_rxn = DecRexx(val);
}

/*---------------------------------------------------------RexxString::assign-+
|                                                                             |
+----------------------------------------------------------------------------*/
void RexxString::assign(bool b)
{
   resetNumber();
   ((Rep *)m_pRep)->m_rxn = DecRexx(b);
}

#if !defined COM_JAXO_YAXX_DENY_MATH
/*------------------------------------------------RexxString::operator double-+
|                                                                             |
+----------------------------------------------------------------------------*/
RexxString::operator double() const {
   if (!((Rep *)m_pRep)->ensureNumber()) {
      throw RexxString::Exception(Exception::NON_NUMERIC_REASON);
   }
   return ((Rep *)m_pRep)->m_rxn.getDouble();
}
#endif

/*------------------------------------------------------------RexxString::c2d-+
|                                                                             |
+----------------------------------------------------------------------------*/
void RexxString::c2d(
   char const * chars,
   int len,
   bool isNegative,
   DecRexxContext const & ctxt
) {
   resetNumberKeep();
   ((Rep *)m_pRep)->m_rxn.fromBinaryString(chars, len, isNegative, ctxt);
   if (((Rep *)m_pRep)->m_rxn.isBad()) {
      throw RexxString::Exception(Exception::NON_NUMERIC_REASON);
   }
}

/*---------------------------------------------------RexxString::operator int-+
|                                                                             |
+----------------------------------------------------------------------------*/
RexxString::operator int() const {
   int whatInt;
   if (!((Rep *)m_pRep)->ensureNumber() || !((Rep *)m_pRep)->m_rxn.isInt(whatInt)) {
      throw RexxString::Exception(Exception::NON_INTEGER_REASON);
   }
   return whatInt;
}

/*--------------------------------------------------RexxString::operator bool-+
|                                                                             |
+----------------------------------------------------------------------------*/
RexxString::operator bool() const {
   bool whatBool;
   if (!((Rep *)m_pRep)->ensureNumber() || !((Rep *)m_pRep)->m_rxn.isBool(whatBool)) {
      throw RexxString::Exception(Exception::NON_BOOLEAN_REASON);
   }
   return whatBool;
}

/*----------------------------------------------------------RexxString::isInt-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool RexxString::isInt(int & whatInt) const {
   return (((Rep *)m_pRep)->ensureNumber() && ((Rep *)m_pRep)->m_rxn.isInt(whatInt));
}

/*-------------------------------------------------------RexxString::isSymbol-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool RexxString::isSymbol() const
{
   ensureString();
   int len = m_pRep->m_len;
   if (len) {
      char const * c_p = m_pRep->m_pch;
      do {
         if (!Tokenizer::isSymbol(*c_p++)) return false;
      }while (--len);
      return true;
   }
   return false;
}

/*-------------------------------------------------------RexxString::isBinary-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool RexxString::isBinary() const
{
   ensureString();
   int len = m_pRep->m_len;
   return binToChar(m_pRep->m_pch, m_pRep->m_len, 0, &len, 0) != 0;
}

/*-----------------------------------------------------RexxString::isAlphanum-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool RexxString::isAlphanum() const
{
   ensureString();
   int len = m_pRep->m_len;
   if (len) {
      char const * c_p = m_pRep->m_pch;
      do { if (!isalnum(*c_p++)) return false; }while (--len);
      return true;
   }
   return false;
}

/*--------------------------------------------------------RexxString::isAlpha-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool RexxString::isAlpha() const
{
   ensureString();
   int len = m_pRep->m_len;
   if (len) {
      char const * c_p = m_pRep->m_pch;
      do { if (!isalpha(*c_p++)) return false; }while (--len);
      return true;
   }
   return false;
}

/*---------------------------------------------------RexxString::isLowerAlpha-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool RexxString::isLowerAlpha() const
{
   ensureString();
   int len = m_pRep->m_len;
   if (len) {
      char const * c_p = m_pRep->m_pch;
      do { if (!islower(*c_p++)) return false; }while (--len);
      return true;
   }
   return false;
}

/*------------------------------------------------------RexxString::changestr-+
|                                                                             |
+----------------------------------------------------------------------------*/
void RexxString::changestr(           // NOT X3J18
   RexxString const & a_toReplace,
   RexxString const & a_replaced,
   RexxString const & a_replacement
) {
   RexxString toReplace(a_toReplace);
   RexxString replaced(a_replaced);
   RexxString replacement(a_replacement);
   toReplace.ensureString();
   replaced.ensureString();
   replacement.ensureString();
   int const lenRplcd = replaced.m_pRep->m_len;
   if (lenRplcd == 0) {
      *this = toReplace;
   }else if (!a_toReplace.exists()) {
      *this = Nil;
   }else {
      replacement.ensureString();
      char const * const pRplmt = replacement.m_pRep->m_pch;
      int const lenRplmt = replacement.m_pRep->m_len;
      char const * const pRplcd = replaced.m_pRep->m_pch;
      int const lenToRepl = toReplace.m_pRep->m_len;
      char const * const pToRepl = toReplace.m_pRep->m_pch;
      char const * const pEnd = pToRepl + lenToRepl - lenRplcd;

      // two passes: first, get and set the length after all replacements...
      int len = lenToRepl;
      char const * pCur = pToRepl;
      while (pCur <= pEnd) {
         if (memcmp(pCur, pRplcd, lenRplcd) == 0) {
            len += lenRplmt - lenRplcd;
            pCur += lenRplcd;
         }else {
            ++pCur;
         }
      }
      assert(len >= 0);
      resetString(len);
      // then, do the replacements
      pCur = pToRepl;
      char * pTarget = m_pRep->m_pch;
      while (pCur <= pEnd) {
         if (memcmp(pCur, pRplcd, lenRplcd) == 0) {
            memcpy(pTarget, pRplmt, lenRplmt);
            pCur += lenRplcd;
            pTarget += lenRplmt;
         }else {
            *pTarget++ = *pCur++;
         }
      }
      len = lenToRepl - (pCur - pToRepl);
      if (len > 0) memcpy(pTarget, pCur, len);
   }
}

/*---------------------------------------------------------RexxString::insert-+
|                                                                             |
+----------------------------------------------------------------------------*/
void RexxString::insert(
   RexxString const & str1,
   RexxString const & toInsert,
   int pos,
   int len,
   char pad
) {
   str1.ensureString();
   toInsert.ensureString();
   if (len < 0) len = toInsert.m_pRep->m_len;

   int newLen = str1.m_pRep->m_len;
   int bef;
   if (newLen < pos) {
      bef = newLen;
      newLen = pos + len;
   }else {
      bef = pos;
      newLen = newLen + len;
   }
   RexxString s1(str1);
   RexxString s2(toInsert);
   resetString(newLen);
   char * c_p = m_pRep->m_pch;
   memset(c_p, pad, newLen);
   memcpy(c_p, s1.m_pRep->m_pch, bef);
   memcpy(c_p+pos, s2.m_pRep->m_pch, s2.m_pRep->m_len);
   memcpy(c_p+pos+len, s1.m_pRep->m_pch+pos, s1.m_pRep->m_len-bef);
}

/*---------------------------------------------------RexxString::isUpperAlpha-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool RexxString::isUpperAlpha() const
{
   ensureString();
   int len = m_pRep->m_len;
   if (len) {
      char const * c_p = m_pRep->m_pch;
      do { if (!isupper(*c_p++)) return false; }while (--len);
      return true;
   }
   return false;
}

/*-------------------------------------------------------RexxString::isNumber-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool RexxString::isNumber() const {
   return ((Rep *)m_pRep)->ensureNumber();
}

/*--------------------------------------------------------RexxString::isWhole-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool RexxString::isWhole(DecRexx & whatWhole) const {
   if (
      (!((Rep *)m_pRep)->ensureNumber()) ||
      !((Rep *)m_pRep)->m_rxn.isWhole()
   ) {
      return false;
   }else {
      whatWhole = ((Rep *)m_pRep)->m_rxn;
      return true;
   }
}

/*---------------------------------------------------------RexxString::isHexa-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool RexxString::isHexa() const {
   ensureString();
   int len = m_pRep->m_len;
   return hexToChar(m_pRep->m_pch, m_pRep->m_len, 0, &len) != 0;
}

/*---------------------------------------------------------RexxString::isZero-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool RexxString::isZero() const {
   if (!((Rep *)m_pRep)->ensureNumber()) {
      return false;
   }
   return (((Rep *)m_pRep)->m_rxn.isZero());
}

/*-----------------------------------------------------RexxString::isNegative-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool RexxString::isNegative() const {
   if (!((Rep *)m_pRep)->ensureNumber()) {
      return false;
   }
   return (((Rep *)m_pRep)->m_rxn.isNegative());
}

/*-----------------------------------------------------------RexxString::sign-+
|                                                                             |
+----------------------------------------------------------------------------*/
int RexxString::sign(DecRexxContext const & ctxt) const {
   if (!((Rep *)m_pRep)->ensureNumber()) {
      throw RexxString::Exception(Exception::NON_NUMERIC_REASON);
   }
   return ((Rep *)m_pRep)->m_rxn.sign(ctxt);
}

char RexxString::charAt(int i) const {
   ensureString();
   return (m_pRep->m_pch[i]);
}

bool RexxString::contains(char c, int len) const {
   ensureString();
   if (len == 0) len = m_pRep->m_len;
   return (memchr(m_pRep->m_pch, c, len) != 0);
}

/*---------------------------------------------------------RexxString::verify-+
|                                                                             |
+----------------------------------------------------------------------------*/
int RexxString::verify(RexxString const & ref, bool isMatch, int start) const
{
   ensureString();
   ref.ensureString();

   if (m_pRep->m_len && (start < m_pRep->m_len)) {
      char const * const pRefMax = ref.m_pRep->m_pch + ref.m_pRep->m_len;
      for (; start < m_pRep->m_len; ++start) {
         char const c = m_pRep->m_pch[start];
         for (char const * pRef = ref.m_pRep->m_pch; ; ++pRef) {
            if (pRef == pRefMax) {
               if (!isMatch) return start;
               break;
            }
            if (*pRef == c) {
               if (isMatch) return start;
               break;
            }
         }
      }
   }
   return -1;
}

int RexxString::wordLength(int wordNo) const {
   ensureString();
   int  i = m_pRep->wordIndex(wordNo);
   return (i < 0)? 0 : m_pRep->skipWord(i) - i;
}

int RexxString::wordIndex(int wordNo) const {
   ensureString();
   return m_pRep->wordIndex(wordNo);
}

int RexxString::wordsCount() const {
   ensureString();
   return m_pRep->wordsCount();
}

bool RexxString::abbrev(RexxString const & info, int len) const {
   ensureString();
   info.ensureString();

   if (info.m_pRep->m_len > m_pRep->m_len) {
      return false;
   }
   if (!info.m_pRep->m_len && (len <= 0)) {
      return true;
   }
   if (len == -1) {
      len = info.m_pRep->m_len;
   }else if (len > info.m_pRep->m_len) {
      return false;
   }
   return memcmp(m_pRep->m_pch, info.m_pRep->m_pch, len) == 0;
}

// this is backward (source <-> this)
int RexxString::pos(RexxString const & source, int start) const {
   if (!source.exists()) return 0;
   ensureString();
   return source.m_pRep->pos(m_pRep, start);
}

// this is backward (source <-> this)
int RexxString::lastpos(RexxString const & source, int start) const {
   if (!source.exists()) return 0;
   ensureString();
   return source.m_pRep->lastpos(m_pRep, start);
}

// this is backward (source <-> this)
int RexxString::wordpos(RexxString const & source, int wordNo) const {
   if (!source.exists()) return 0;
   ensureString();
   source.ensureString();
   return source.m_pRep->wordPos(m_pRep, wordNo);
}

// NOT X3J18
int RexxString::countstr(RexxString const & needle) const
{
   ensureString();
   needle.ensureString();
   int count = 0;
   int const lenNeedle = needle.m_pRep->m_len;
   int const lenHayStack = m_pRep->m_len;
   if (lenNeedle && lenHayStack) {
      char const * const pNeedle = needle.m_pRep->m_pch;
      char const * pSource = m_pRep->m_pch;
      char const * const pEnd = pSource + lenHayStack - lenNeedle;

      while (pSource <= pEnd) {
         if (memcmp(pSource, pNeedle, lenNeedle) == 0) {
            ++count;
            pSource += lenNeedle;
         }else {
            ++pSource;
         }
      }
   }
   return count;
}

/*---------------------------------------------------------RexxString::center-+
|                                                                             |
+----------------------------------------------------------------------------*/
void RexxString::center(RexxString const & str1, int len, char pad)
{
   str1.ensureString();
   int i = (len - str1.m_pRep->m_len) / 2;
   RexxString s1(str1);
   resetString(len);
   char * c_p = m_pRep->m_pch;
   if (i > 0) {
      memset(c_p, pad, len);
      memcpy(c_p+i, s1.m_pRep->m_pch, s1.m_pRep->m_len);
   }else {
      memcpy(c_p, s1.m_pRep->m_pch-i, len);
   }
}

/*--------------------------------------------------RexxString::normalCompare-+
| Normal compare (see pp 22-23)                                               |
+----------------------------------------------------------------------------*/
RexxString::COMPARE_RESULT RexxString::normalCompare(
   RexxString const & str,
   DecRexxContext const & ctxt
) const {
   if (
      ((m_pRep->m_len >= 0) && !((Rep *)m_pRep)->ensureNumber()) ||
      ((str.m_pRep->m_len >= 0) && !((Rep *)str.m_pRep)->ensureNumber())
    ) {
      ensureString();
      str.ensureString();
      char const * pchMe = m_pRep->m_pch;
      char const * pchHe = str.m_pRep->m_pch;
      char const * pEndMe = pchMe + m_pRep->m_len;
      char const * pEndHe = pchHe + str.m_pRep->m_len;

      while ((pchMe < pEndMe) && isSpace(*pchMe)) ++pchMe;
      while ((pchHe < pEndHe) && isSpace(*pchHe)) ++pchHe;
      while ((pchMe < pEndMe) && (pchHe < pEndHe) && (*pchMe == *pchHe)) {
         ++pchMe; ++pchHe;
      }
      while ((pchMe < pEndMe) && isSpace(*pchMe)) ++pchMe;
      while ((pchHe < pEndHe) && isSpace(*pchHe)) ++pchHe;
      if ((pchMe == pEndMe) && (pchHe == pEndHe)) {
         return IS_EQ;
      }else if ((pchMe < pEndMe) && (pchHe < pEndHe)) {
         return (*pchMe < *pchHe) ? IS_LT : IS_GT;
      }else {
         return (pchMe < pEndMe) ? IS_GT : IS_LT;
      }
   }else {
      assert(((Rep *)m_pRep)->ensureNumber());
      assert(((Rep *)str.m_pRep)->ensureNumber());
      return (cifs+1)[
         ((Rep *)m_pRep)->m_rxn.compare(((Rep *)str.m_pRep)->m_rxn, ctxt)
      ];
   }
}

/*--------------------------------------------------RexxString::strictCompare-+
| Strict compare (see pp 22-23)                                               |
+----------------------------------------------------------------------------*/
RexxString::COMPARE_RESULT RexxString::strictCompare(
   RexxString const & str
) const {
   ensureString();
   str.ensureString();
   int len1 = m_pRep->m_len;
   int len2 = str.m_pRep->m_len;
   if (len1 == len2) {
      if (len1 != 0) {
         int r = memcmp(m_pRep->m_pch, str.m_pRep->m_pch, len1);
         if (r < 0) {
            return IS_LT;
         }else if (r > 0) {
            return IS_GT;
         }
      }
      return IS_EQ;
   }else if (len2 < len1) {
      if (!len2 || (memcmp(m_pRep->m_pch, str.m_pRep->m_pch, len2) >= 0)) {
         return IS_GT;
      }else {
         return IS_LT;
      }
   }else {
      if (!len1 || (memcmp(m_pRep->m_pch, str.m_pRep->m_pch, len1) <= 0)) {
         return IS_LT;
      }else {
         return IS_GT;
      }
   }
}

/*------------------------------------------------------------RexxString::max-+
|                                                                             |
+----------------------------------------------------------------------------*/
int RexxString::max(
   int argc,
   RexxString const * const * argv,
   DecRexxContext const & ctxt
) {
   int found = -1;

   for (int i=0; (i < argc); ++i) {
      Rep * pRep1;
      if (argv[i]) {
         Rep * pRep2 = (Rep *)(argv[i]->m_pRep);
         if (!pRep2->ensureNumber()) {
            throw RexxString::Exception(Exception::NON_NUMERIC_REASON);
         }
         if (found >= 0) {
            if (pRep2->m_rxn.compare(pRep1->m_rxn, ctxt) <= 0) continue;
         }
         found = i;
         pRep1 = pRep2;
      }
   }
   return found;
}

/*------------------------------------------------------------RexxString::min-+
|                                                                             |
+----------------------------------------------------------------------------*/
int RexxString::min(
   int argc,
   RexxString const * const * argv,
   DecRexxContext const & ctxt
) {
   int found = -1;

   for (int i=0; (i < argc); ++i) {
      Rep * pRep1;
      if (argv[i]) {
         Rep * pRep2 = (Rep *)(argv[i]->m_pRep);
         if (!pRep2->ensureNumber()) {
            throw RexxString::Exception(Exception::NON_NUMERIC_REASON);
         }
         if (found >= 0) {
            if (pRep1->m_rxn.compare(pRep2->m_rxn, ctxt) <= 0) continue;
         }
         found = i;
         pRep1 = pRep2;
      }
   }
   return found;
}

/*-----------------------------------------------------RexxString::operator==-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool RexxString::operator==(RexxString const & str) const {
   ensureString();
   str.ensureString();
   return (
      (m_pRep->m_len == str.m_pRep->m_len) &&
      (memcmp(m_pRep->m_pch, str.m_pRep->m_pch, m_pRep->m_len) == 0)
   );
}

/*-----------------------------------------------------RexxString::RexxString-+
| Private "Nil constructor"                                                   |
+----------------------------------------------------------------------------*/
#if defined _WIN32
#pragma warning(disable:4291)     // no matching delete
#endif
RexxString::RexxString(void ***) {
   static char nilRexxRep[sizeof (Rep)];
   Rep::NewParam p(nilRexxRep);
   m_pRep = new(&p) Rep();
}
#if defined _WIN32
#pragma warning(default:4291)
#endif

/*-----------------------------------------------------RexxString::RexxString-+
|                                                                             |
+----------------------------------------------------------------------------*/
RexxString::RexxString() {
   m_pRep = RexxString::Nil.m_pRep;
}

RexxString::RexxString(char const * buf) {
   int len = strlen(buf);
   m_pRep = Rep::alloc(len);
   memcpy(m_pRep->m_pch, buf, len);
}

RexxString::RexxString(char const * buf, int len) {
   if (len) {
      m_pRep = Rep::alloc(len);
      memcpy(m_pRep->m_pch, buf, len);
   }else {
      m_pRep = Nil.m_pRep;
   }
}

RexxString::RexxString(char c) {
   m_pRep = Rep::alloc(1);
   m_pRep->m_pch[0] = c;
}

RexxString::RexxString(char c, int len) {
   m_pRep = Rep::alloc(len);
   memset(m_pRep->m_pch, c, len);
}

RexxString::RexxString(RexxString const & source, CharConverter const & x)
{
   if (!source.exists()) {
      m_pRep = Nil.m_pRep;
   }else {
      source.ensureString();
      m_pRep = Rep::alloc(source.m_pRep->m_len);
      x(m_pRep->m_pch, source.m_pRep->m_pch, m_pRep->m_len);
   }
}

#if !defined COM_JAXO_YAXX_DENY_MATH
RexxString::RexxString(double val) {
   m_pRep = Rep::alloc(0);
   m_pRep->m_len = -1;
   ((Rep *)m_pRep)->m_rxn = DecRexx(val);
}
#endif

RexxString::RexxString(int val) {
   m_pRep = Rep::alloc(0);
   m_pRep->m_len = -1;
   ((Rep *)m_pRep)->m_rxn = DecRexx(val);
}

RexxString::RexxString(bool val) {
   m_pRep = Rep::alloc(0);
   m_pRep->m_len = -1;
   ((Rep *)m_pRep)->m_rxn = DecRexx(val);
}

#if !defined COM_JAXO_YAXX_DENY_XML
RexxString::RexxString(UnicodeString const & ucs) {
   int len = ucs.length();
   m_pRep = Rep::alloc(len);
   UCS_2 const * pUc = ucs;
   char *c_p = m_pRep->m_pch;
   while (len--) {
      *c_p++ = (char)*pUc++;
   }
}
#endif

#if !defined COM_JAXO_YAXX_DENY_XML
RexxString::RexxString(YaspEvent const & ev) {
   m_pRep = Rep::alloc(0);
   m_pRep->m_len = -1;
   ((Rep *)m_pRep)->m_ev = ev;
}
#endif

RexxString::RexxString(StringBuffer const & buf) {
   new(this) RexxString((char const *)buf, buf.length());
}

RexxString & RexxString::operator=(RexxString const * pSource)
{
   if (m_pRep != pSource->m_pRep) {
      this->~RexxString();
      m_pRep = pSource->m_pRep;
      if (m_pRep->m_pch) ++m_pRep->iReferenceCount;
   }
   return *this;
}

/*--------------------------------------------------------RexxString::compare-+
| Compare Built-In                                                            |
| Compares two strings and returns the position of the first non matching     |
| character [1,largest length]                                                |
| Returns 0 if strings are equal                                              |
+----------------------------------------------------------------------------*/
int RexxString::compare(RexxString const & str, char pad) const
{
   ensureString();
   str.ensureString();
   return m_pRep->compare(str.m_pRep, pad);
}

/*--------------------------------------------------RexxString::examineStatus-+
| Either return TRUE if its benign, return FALSE for Lost_digits,             |
| or throw an exception.                                                      |
+----------------------------------------------------------------------------*/
bool RexxString::examineStatus(int status)
{
   RexxString::Exception::Reason r;
   if (!status || (status &= ~(DEC_Inexact | DEC_Rounded), !status)) {
      return true;
   }else if (status & DEC_Division_by_zero) {
      r = RexxString::Exception::DIVIDE_BY_ZERO_REASON;
   }else if (status & DEC_Overflow) {
      r = RexxString::Exception::OVERFLOW_REASON;
   }else if (status & DEC_Underflow) {
      r = RexxString::Exception::UNDERFLOW_REASON;
   }else if (status & DEC_Lost_digits) {
      return false;
   }else {
      r = RexxString::Exception::BAD_ARITH_REASON;
   }
   throw RexxString::Exception(r);
}

/*----------------------------------------------------------RexxString::strip-+
|                                                                             |
+----------------------------------------------------------------------------*/
void RexxString::strip(RexxString const & str1, Kill options, char pad)
{
   str1.ensureString();
   char const * c_p = str1.m_pRep->m_pch;
   int len = str1.m_pRep->m_len;
   if (options & KillLEAD) {
      while (len && (*c_p == pad)) ++c_p, --len;
   }
   if (options & KillTRAIL) {
      char const * d_p = str1.m_pRep->m_pch + str1.m_pRep->m_len;
      while (len && (*--d_p == pad)) --len;
   }
   RexxString s1(str1);
   resetString(len);
   memcpy(m_pRep->m_pch, c_p, len);
}

/*---------------------------------------------------------RexxString::wordAt-+
|                                                                             |
+----------------------------------------------------------------------------*/
void RexxString::wordAt(RexxString const & str1, int wordNo) {
   str1.ensureString();
   int i = str1.m_pRep->wordIndex(wordNo);
   int len = (i < 0)? 0 : str1.m_pRep->skipWord(i) - i;
   RexxString s1(str1);
   resetString(len);
   memcpy(m_pRep->m_pch, s1.m_pRep->m_pch+i, len);
}

/*------------------------------------------------------------RexxString::add-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool RexxString::add(
   RexxString const & str1,
   RexxString const & str2,
   DecRexxContext const & ctxt
) {
   if (!((Rep *)str1.m_pRep)->ensureNumber() || !((Rep *)str2.m_pRep)->ensureNumber()) {
      throw RexxString::Exception(Exception::NON_NUMERIC_REASON);
   }
   resetNumberKeep();
   return examineStatus(
      ((Rep *)m_pRep)->m_rxn.add(
         ((Rep *)str1.m_pRep)->m_rxn,
         ((Rep *)str2.m_pRep)->m_rxn,
         ctxt
      )
   );
}

/*-------------------------------------------------------RexxString::subtract-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool RexxString::subtract(
   RexxString const & str1,
   RexxString const & str2,
   DecRexxContext const & ctxt
) {
   if (!((Rep *)str1.m_pRep)->ensureNumber() || !((Rep *)str2.m_pRep)->ensureNumber()) {
      throw RexxString::Exception(Exception::NON_NUMERIC_REASON);
   }
   resetNumberKeep();
   return examineStatus(
      ((Rep *)m_pRep)->m_rxn.subtract(
         ((Rep *)str1.m_pRep)->m_rxn,
         ((Rep *)str2.m_pRep)->m_rxn,
         ctxt
      )
   );
}

/*---------------------------------------------------------RexxString::substr-+
|                                                                             |
+----------------------------------------------------------------------------*/
void RexxString::substr(RexxString const & str1, int pos, int len, char pad)
{
   str1.ensureString();
   int bef = str1.m_pRep->m_len - pos;

   if ((len == -1) && (len = bef, len < 0)) len = 0;
   if (bef > len) {
      bef = len;
   }else if (bef < 0) {
      bef = 0;
   }
   RexxString s1(str1);
   resetString(len);
   char * c_p = m_pRep->m_pch;
   memcpy(c_p, s1.m_pRep->m_pch+pos, bef);
   memset(c_p+bef, pad, len-bef);
}

/*-------------------------------------------------------RexxString::multiply-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool RexxString::multiply(
   RexxString const & str1,
   RexxString const & str2,
   DecRexxContext const & ctxt
) {
   if (!((Rep *)str1.m_pRep)->ensureNumber() || !((Rep *)str2.m_pRep)->ensureNumber()) {
      throw RexxString::Exception(Exception::NON_NUMERIC_REASON);
   }
   resetNumberKeep();
   return examineStatus(
      ((Rep *)m_pRep)->m_rxn.multiply(
         ((Rep *)str1.m_pRep)->m_rxn,
         ((Rep *)str2.m_pRep)->m_rxn,
         ctxt
      )
   );
}

/*---------------------------------------------------------RexxString::divide-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool RexxString::divide(
   RexxString const & str1,
   RexxString const & str2,
   DecRexxContext const & ctxt
) {
   if (!((Rep *)str1.m_pRep)->ensureNumber() || !((Rep *)str2.m_pRep)->ensureNumber()) {
      throw RexxString::Exception(Exception::NON_NUMERIC_REASON);
   }
   resetNumberKeep();
   return examineStatus(
      ((Rep *)m_pRep)->m_rxn.divide(
         ((Rep *)str1.m_pRep)->m_rxn,
         ((Rep *)str2.m_pRep)->m_rxn,
         ctxt
      )
   );
}

/*--------------------------------------------------RexxString::divideInteger-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool RexxString::divideInteger(
   RexxString const & str1,
   RexxString const & str2,
   DecRexxContext const & ctxt
) {
   if (!((Rep *)str1.m_pRep)->ensureNumber() || !((Rep *)str2.m_pRep)->ensureNumber()) {
      throw RexxString::Exception(Exception::NON_NUMERIC_REASON);
   }
   resetNumberKeep();
   return examineStatus(
      ((Rep *)m_pRep)->m_rxn.divideInteger(
         ((Rep *)str1.m_pRep)->m_rxn,
         ((Rep *)str2.m_pRep)->m_rxn,
         ctxt
      )
   );
}

/*-----------------------------------------------------------RexxString::left-+
|                                                                             |
+----------------------------------------------------------------------------*/
void RexxString::left(RexxString const & str1, int len, char pad)
{
   str1.ensureString();
   RexxString s1(str1);
   resetString(len);
   char * c_p = m_pRep->m_pch;
   if (len < s1.m_pRep->m_len) {
      memcpy(c_p, s1.m_pRep->m_pch, len);
   }else {
      memcpy(c_p, s1.m_pRep->m_pch, s1.m_pRep->m_len);
      memset(c_p+s1.m_pRep->m_len, pad, len-s1.m_pRep->m_len);
   }
}

/*---------------------------------------------------------RexxString::xrange-+
|                                                                             |
+----------------------------------------------------------------------------*/
void RexxString::xrange(unsigned char start, unsigned char stop)
{
   int len = stop-start+1;
   if (len < 0) len += 256;
   resetString(len);
   char * c_p = m_pRep->m_pch;
   do *c_p++ = (char)start; while (start++ != stop);
}

#if !defined COM_JAXO_YAXX_DENY_XML
/*--------------------------------------------------------RexxString::tagname-+
|                                                                             |
+----------------------------------------------------------------------------*/
void RexxString::tagname(RexxString const & str1)
{
   Tag const * pTag = ((Rep *)str1.m_pRep)->m_ev.inqTag();
   if (pTag) {
      RexxString::Stringizer s((RexxString &)*this);
      s.extractTagName(pTag);
   }else {
      resetString(0);
   }
}
#endif

#if !defined COM_JAXO_YAXX_DENY_XML
/*---------------------------------------------------------RexxString::attval-+
|                                                                             |
+----------------------------------------------------------------------------*/
void RexxString::attval(RexxString const & str1, RexxString const & str2)
{
   Tag const * pTag = ((Rep *)str1.m_pRep)->m_ev.inqTag();
   if (pTag) {
      str2.ensureString();
      RexxString::Stringizer s((RexxString &)*this);
      s.extractAttributeValue(pTag, str2);
   }else {
      resetString(0);
   }
}
#endif

/*----------------------------------------------------------------isPrintable-+
| we extend ASCII here, pretending to recognize                               |
| all characters in the range [0x20-0xFF] as 'printable'                      |
+----------------------------------------------------------------------------*/
static bool isPrintable(char const c) {
   return ((c & 0xe0) != 0);
}

/*-----------------------------------------------------------------operator<<-+
|                                                                             |
+----------------------------------------------------------------------------*/
ostream & operator<<(ostream & out, RexxString const & rcs)
{
   rcs.ensureString();

   char const * pCur = rcs.m_pRep->m_pch;
   char const * pBase = pCur;
   char const * pMax = pCur + rcs.m_pRep->m_len;
   char old_fill = out.fill('0');
   out << std::hex;
   for (;;) {
      if ((pCur < pMax) && isPrintable(*pCur)) {
         ++pCur;
      }else {
         if (pBase < pCur) {
            out.write(pBase, pCur - pBase);
         }
         if (pCur == pMax) break;
         out << '\'';
         do {
            out << std::hex << std::setw(2) << (((int)*pCur) & 0xff);
         }while ((++pCur < pMax) && !isPrintable(*pCur));
         out << "'x";
         pBase = pCur;
      }
   }
   out << std::dec;
   out.fill(old_fill);
   return out;
}

/*------------------------------------------------------RexxString::remainder-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool RexxString::remainder(
   RexxString const & str1,
   RexxString const & str2,
   DecRexxContext const & ctxt
) {
   if (!((Rep *)str1.m_pRep)->ensureNumber() || !((Rep *)str2.m_pRep)->ensureNumber()) {
      throw RexxString::Exception(Exception::NON_NUMERIC_REASON);
   }
   resetNumberKeep();
   return examineStatus(
      ((Rep *)m_pRep)->m_rxn.remainder(
         ((Rep *)str1.m_pRep)->m_rxn,
         ((Rep *)str2.m_pRep)->m_rxn,
         ctxt
      )
   );
}

/*----------------------------------------------------------RexxString::power-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool RexxString::power(
   RexxString const & str1,
   RexxString const & str2,
   DecRexxContext const & ctxt
) {
   if (!((Rep *)str1.m_pRep)->ensureNumber() || !((Rep *)str2.m_pRep)->ensureNumber()) {
      throw RexxString::Exception(Exception::NON_NUMERIC_REASON);
   }
   resetNumberKeep();
   return examineStatus(
      ((Rep *)m_pRep)->m_rxn.power(
         ((Rep *)str1.m_pRep)->m_rxn,
         ((Rep *)str2.m_pRep)->m_rxn,
         ctxt
      )
   );
}

/*-----------------------------------------------------------RexxString::plus-+
|                                                                             |
+----------------------------------------------------------------------------*/
void RexxString::plus(
   RexxString const & str1,
   DecRexxContext const & ctxt
) {
   if (!((Rep *)str1.m_pRep)->ensureNumber()) {
      throw RexxString::Exception(Exception::NON_NUMERIC_REASON);
   }
   resetNumberKeep();
   ((Rep *)m_pRep)->m_rxn.plus(((Rep *)str1.m_pRep)->m_rxn, ctxt);
}

/*----------------------------------------------------------RexxString::minus-+
|                                                                             |
+----------------------------------------------------------------------------*/
void RexxString::minus(
   RexxString const & str1,
   DecRexxContext const & ctxt
) {
   if (!((Rep *)str1.m_pRep)->ensureNumber()) {
      throw RexxString::Exception(Exception::NON_NUMERIC_REASON);
   }
   resetNumberKeep();
   ((Rep *)m_pRep)->m_rxn.minus(((Rep *)str1.m_pRep)->m_rxn, ctxt);
}

/*------------------------------------------------------------RexxString::abs-+
|                                                                             |
+----------------------------------------------------------------------------*/
void RexxString::abs(
   RexxString const & str1,
   DecRexxContext const & ctxt
) {
   if (!((Rep *)str1.m_pRep)->ensureNumber()) {
      throw RexxString::Exception(Exception::NON_NUMERIC_REASON);
   }
   resetNumberKeep();
   ((Rep *)m_pRep)->m_rxn.abs(((Rep *)str1.m_pRep)->m_rxn, ctxt);
}

/*-------------------------------------------------------RexxString::truncate-+
|                                                                             |
+----------------------------------------------------------------------------*/
void RexxString::truncate(
   RexxString const & str1,
   int len,
   DecRexxContext const & ctxt
) {
   if (!((Rep *)str1.m_pRep)->ensureNumber()) {
      throw RexxString::Exception(Exception::NON_NUMERIC_REASON);
   }
   resetNumberKeep();   // e.g. invalidate string only
   ((Rep *)m_pRep)->m_rxn.truncate(((Rep *)str1.m_pRep)->m_rxn, len, ctxt);
}

/*-----------------------------------------------------------RexxString::sqrt-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool RexxString::sqrt(RexxString const & str1, DecRexxContext const & ctxt)
{
   if (!((Rep *)str1.m_pRep)->ensureNumber()) {
      throw RexxString::Exception(Exception::NON_NUMERIC_REASON);
   }
   resetNumberKeep();   // e.g. invalidate string only
   return examineStatus(
      ((Rep *)m_pRep)->m_rxn.squareRoot(((Rep *)str1.m_pRep)->m_rxn, ctxt)
   );
}

/*------------------------------------------------------RexxString::increment-+
|                                                                             |
+----------------------------------------------------------------------------*/
void RexxString::increment(DecRexxContext const & ctxt) {
   if (!((Rep *)m_pRep)->ensureNumber()) {
      throw RexxString::Exception(Exception::NON_NUMERIC_REASON);
   }
   resetNumberKeep();
   ((Rep *)m_pRep)->m_rxn.increment(ctxt);
}

/*------------------------------------------------------RexxString::decrement-+
|                                                                             |
+----------------------------------------------------------------------------*/
void RexxString::decrement(DecRexxContext const & ctxt) {
   if (!((Rep *)m_pRep)->ensureNumber()) {
      throw RexxString::Exception(Exception::NON_NUMERIC_REASON);
   }
   resetNumberKeep();
   ((Rep *)m_pRep)->m_rxn.decrement(ctxt);
}

/*--------------------------------------------------------RexxString::convert-+
|                                                                             |
+----------------------------------------------------------------------------*/
void RexxString::convert(CharConverter const & x)
{
   if (exists()) {
      ensureString();
      char const * pch = m_pRep->m_pch;
      resetString(m_pRep->m_len);
      x(m_pRep->m_pch, pch, m_pRep->m_len);
   }
}

/*---------------------------------------------------------RexxString::format-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool RexxString::format(
   RexxString const & str1,
   int bef, int aft, int expp, int expt,
   DecRexxContext const & ctxt
) {
   if (!((Rep *)str1.m_pRep)->ensureNumber()) {
      throw RexxString::Exception(Exception::NON_NUMERIC_REASON);
      return 0;  // metrowerks didn't catch it
   }else {
      DecRexx::String rstr = ((Rep *)str1.m_pRep)->m_rxn.format(
         bef, aft, expp, expt, ctxt
      );
      char const * p = rstr;
      if (!p) {
         return false;
      }else {
         int len = strlen(p);
         ((RexxString *)this)->resetString(len);    // violates constness (OK)
         memcpy(m_pRep->m_pch, p, len+1);
         return true;
      }
   }
}

/*------------------------------------------------------------RexxString::d2c-+
|                                                                             |
+----------------------------------------------------------------------------*/
void RexxString::d2c(DecRexx const & whole, int n)
{
   if (n == -1) {                         // unspecified
      //>>>PGR: I can do a better approx!
      int maxSize = 5+(whole.getDigitsCount()/2);
      char * buf = new char[maxSize];     // roomy enough
      buf[0] = '\0';                      // overflow sentinel
      char * p = whole.toBinaryString(buf, maxSize);
      assert (buf[0] == '\0');            // check for overflow
      int len = buf + maxSize - p;
      while (!*p && len) {                // remove leading zeroes
         --len;
         ++p;
      }
      resetString(len);
      memcpy(m_pRep->m_pch, p, len);
      delete [] buf;
   }else {
      resetString(n);
      int rem = whole.toBinaryString(m_pRep->m_pch, n) - m_pRep->m_pch;
      if (rem > 0) {
         memset(m_pRep->m_pch, whole.isNegative()? '\xFF' : '\x00', rem);
      }
   }
}

/*------------------------------------------------------------RexxString::d2x-+
|                                                                             |
+----------------------------------------------------------------------------*/
void RexxString::d2x(DecRexx const & val, int n) {
   d2c(val, (n==-1)? -1 : (n+1)/2);
   c2x(*this, n);
}

/*---------------------------------------------------------RexxString::delstr-+
|                                                                             |
+----------------------------------------------------------------------------*/
void RexxString::delstr(RexxString const & str1, int start, int len)
{
   char const * c_p = 0;                           // b4 the word to delete
   int aft = 0;
   int bef;

   str1.ensureString();
   if ((start >= str1.m_pRep->m_len) || (len == 0)) {
      bef = str1.m_pRep->m_len;                    // copy all the stuff
   }else {
      bef = start;
      if ((len == -1) || (aft = str1.m_pRep->m_len - (start+len), aft < 0)) {
         aft = 0;
      }else {
         c_p = str1.m_pRep->m_pch + (start+len);
      }
   }
   RexxString s1(str1);
   resetString(bef+aft);
   memcpy(m_pRep->m_pch, s1.m_pRep->m_pch, bef);   // before
   memcpy(m_pRep->m_pch+bef, c_p, aft);            // after
}

/*--------------------------------------------------------RexxString::delword-+
|                                                                             |
+----------------------------------------------------------------------------*/
void RexxString::delword(RexxString const & str1, int wordNo, int wordCount)
{
   char const * c_p = 0;                           // b4 the word to delete
   int aft = 0;
   int bef;

   str1.ensureString();
   if ((wordCount == 0) || (bef = str1.m_pRep->wordIndex(wordNo), bef < 0)) {
      bef = str1.m_pRep->m_len;
   }else if (wordCount != -1) {                    // "after" : pos and length
      char const * const d_p = str1.m_pRep->m_pch + str1.m_pRep->m_len;
      for (c_p = str1.m_pRep->m_pch+bef; c_p < d_p; ) {
         if (isSpace(*c_p)) {
            while ((++c_p < d_p) && isSpace(*c_p)) {}
            if (wordCount == 0) {
               aft = d_p - c_p;
               break;
            }
         }else {
            --wordCount;
            while ((++c_p < d_p) && !isSpace(*c_p)) {}
         }
      }
   }
   RexxString s1(str1);
   resetString(bef+aft);
   memcpy(m_pRep->m_pch, s1.m_pRep->m_pch, bef);   // before
   memcpy(m_pRep->m_pch+bef, c_p, aft);            // after
}

#if !defined COM_JAXO_YAXX_DENY_XML
/*------------------------------------------------RexxString::isEvRegStartTag-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool RexxString::isEvRegStartTag() const {
   return (
      (((Rep *)m_pRep)->m_ev.inqType() == YSP__TAG_START) &&
      (!((Rep *)m_pRep)->m_ev.inqTag()->isOmittedStart())
   );
}
bool RexxString::isEvInfStartTag() const {
   return (
      (((Rep *)m_pRep)->m_ev.inqType() == YSP__TAG_START) &&
      (((Rep *)m_pRep)->m_ev.inqTag()->isOmittedStart())
   );
}
bool RexxString::isEvRegEmptyTag() const {
   return (
      (((Rep *)m_pRep)->m_ev.inqType() == YSP__TAG_EMPTY) &&
      (!((Rep *)m_pRep)->m_ev.inqTag()->isOmittedStart())
   );
}
bool RexxString::isEvInfEmptyTag() const {
   return (
      (((Rep *)m_pRep)->m_ev.inqType() == YSP__TAG_EMPTY) &&
      (((Rep *)m_pRep)->m_ev.inqTag()->isOmittedStart())
   );
}
bool RexxString::isEvRegEndTag() const {
   return (
      (((Rep *)m_pRep)->m_ev.inqType() == YSP__TAG_END) &&
      (!((Rep *)m_pRep)->m_ev.inqTag()->isOmittedEnd())
   );
}
bool RexxString::isEvInfEndTag() const {
   return (
      (((Rep *)m_pRep)->m_ev.inqType() == YSP__TAG_END) &&
      (((Rep *)m_pRep)->m_ev.inqTag()->isOmittedEnd())
   );
}
bool RexxString::isEvText() const {
   return (((Rep *)m_pRep)->m_ev.inqType() == YSP__TEXT);
}
bool RexxString::isEvData() const
{
   switch (((Rep *)m_pRep)->m_ev.inqType()) {
   case YSP__SDATA_TEXT:
   case YSP__CDATA_TEXT:
      return true;
   default:
      return false;
   }
}
bool RexxString::isEvPi() const {
   return (((Rep *)m_pRep)->m_ev.inqType() == YSP__PI_TEXT);
}
bool RexxString::isEvComment() const {
   return (((Rep *)m_pRep)->m_ev.inqType() == YSP__COMMENT_TEXT);
}
bool RexxString::isEvStartParse() const {
   return ((Rep *)m_pRep)->m_ev.inqType() == YSP__PARSE_START;
}
bool RexxString::isEvEndParse() const {
   return ((Rep *)m_pRep)->m_ev.inqType() == YSP__PARSE_END;
}
bool RexxString::isEvBad() const {
   return ((Rep *)m_pRep)->m_ev.inqType() == YSP__ERROR;
}
#endif // COM_JAXO_YAXX_DENY_XML

#if !defined COM_JAXO_YAXX_DENY_XML
/*--------------------------------------------------------RexxString::xmltype-+
|                                                                             |
+----------------------------------------------------------------------------*/
char RexxString::xmltype() const {
   switch (((Rep *)m_pRep)->m_ev.inqType()) {
   case YSP__TAG_START:
      return (((Rep *)m_pRep)->m_ev.inqTag()->isOmittedStart())? 's' : 'S';
   case YSP__TAG_END:
      return (((Rep *)m_pRep)->m_ev.inqTag()->isOmittedEnd())? 'e' : 'E';
   case YSP__TAG_EMPTY:
      return (((Rep *)m_pRep)->m_ev.inqTag()->isOmittedStart())? 'v' : 'V';
   case YSP__TEXT:
      return 'T';
   case YSP__SDATA_TEXT:
   case YSP__CDATA_TEXT:
      return 'D';
   case YSP__PI_TEXT:
      return 'P';
   case YSP__COMMENT_TEXT:
      return 'C';
   case YSP__PARSE_START:
      return '{';
   case YSP__PARSE_END:
      return '}';
   default:
      return '?';
   }
}
#endif // COM_JAXO_YAXX_DENY_XML

/*STATIC------------------------------------------------RexxString::hexToChar-+
| Convert a Rexx hexadecimal literal into a series of characters.             |
| Ex: '666f6f626172'X => foobar                                               |
|                                                                             |
| Params:                                                                     |
| src   hexadecimal rexx literal to convert.  zero-ended ascii string         |
| dest  address of a buffer where the result is returned                      |
|       if 0, only a validation is performed                                  |
| pLen  pointer to an integer                                                 |
|       on input,                                                             |
|       - if dest is zero, the input value is irrelevant; otherwise,          |
|       - it contains the maximum length of the buffer, which must            |
|         be strictly positive                                                |
|       when returning, it contains the actual length of the result of the    |
|       conversion                                                            |
| Returns:                                                                    |
|       - if the hexadecimal literal was valid: a pointer to the converted    |
|         result in dest, or a non-null irrelevant value if dest was null     |
|       - if the hexadecimal literal was invalid: a null pointer              |
| a pointer to the converted result, or null if the hexadecimal literal       |
| had an invalid syntax                                                       |
|                                                                             |
| Notes:                                                                      |
| 1) The returned value is not zero ended: *pLen must be examined instead.    |
| 2) the converted value occupies the last bytes of the dest buffer ("right   |
|    aligned")                                                                |
| 3) spaces are permitted when they occur at a byte boundary                  |
+----------------------------------------------------------------------------*/
char * RexxString::hexToChar(
   char const * src,
   int len,
   char * dest,
   int * pLen
) {
   char const * c_p = src + len;
   if (dest == 0) {
      *pLen = INT_MAX;
   }
   char * d_p = dest + *pLen;

   if (len && ((*src == ' ') || (*(c_p-1) == ' '))) {
      return 0;
   }
   while (c_p-- > src) {
      unsigned char k;
      switch (*c_p) {
      case ' ':            continue;
      case '0':            k =  0; break;
      case '1':            k =  1; break;
      case '2':            k =  2; break;
      case '3':            k =  3; break;
      case '4':            k =  4; break;
      case '5':            k =  5; break;
      case '6':            k =  6; break;
      case '7':            k =  7; break;
      case '8':            k =  8; break;
      case '9':            k =  9; break;
      case 'a': case 'A':  k = 10; break;
      case 'b': case 'B':  k = 11; break;
      case 'c': case 'C':  k = 12; break;
      case 'd': case 'D':  k = 13; break;
      case 'e': case 'E':  k = 14; break;
      case 'f': case 'F':  k = 15; break;
      default:             return 0;
      }
      if (--d_p < dest) return 0;
      if (c_p-- == src) { if (dest) { *d_p = k; } break; }
      switch (*c_p) {
      case '0':            break;
      case '1':            k +=  1 << 4; break;
      case '2':            k +=  2 << 4; break;
      case '3':            k +=  3 << 4; break;
      case '4':            k +=  4 << 4; break;
      case '5':            k +=  5 << 4; break;
      case '6':            k +=  6 << 4; break;
      case '7':            k +=  7 << 4; break;
      case '8':            k +=  8 << 4; break;
      case '9':            k +=  9 << 4; break;
      case 'a':  case 'A': k += 10 << 4; break;
      case 'b':  case 'B': k += 11 << 4; break;
      case 'c':  case 'C': k += 12 << 4; break;
      case 'd':  case 'D': k += 13 << 4; break;
      case 'e':  case 'E': k += 14 << 4; break;
      case 'f':  case 'F': k += 15 << 4; break;
      default:             return 0;
      }
      if (dest) *d_p = k;
   }
   *pLen -= (d_p - dest);
   return d_p;
}

/*STATIC------------------------------------------------RexxString::binToChar-+
| Convert a Rexx binary literal into a series of characters.                  |
| Ex: '110011001101111 01101111'B => foo                                      |
|                                                                             |
| Params:                                                                     |
| src   binary rexx literal to convert.  zero-ended ascii string              |
| dest  address of a buffer where the result is returned                      |
|       if 0, only a validation is performed                                  |
| pLen  pointer to an integer                                                 |
|       on input,                                                             |
|       - if dest is zero, the input value is irrelevant; otherwise,          |
|       - it contains the maximum length of the buffer, which must            |
|         be strictly positive                                                |
|       when returning, it contains the actual length of the result of the    |
|       conversion                                                            |
|                                                                             |
| Returns:                                                                    |
|       - if the binary literal was valid: a pointer to the converted result  |
|         in dest, or a non-null irrelevant value if dest was null            |
|       - if the binary literal was invalid: a null pointer                   |
|                                                                             |
| Notes:                                                                      |
| 1) The returned value is not zero ended: *pLen must be examined instead.    |
| 2) the converted value occupies the last bytes of the dest buffer ("right   |
|    aligned")                                                                |
| 3) spaces are permitted when they occur at a byte boundary                  |
+----------------------------------------------------------------------------*/
char * RexxString::binToChar(
   char const * src,
   int len,
   char * dest,
   int * pLen,
   bool * pIsOdd
) {
   char const * c_p = src + len;
   if (dest == 0) {
      *pLen = INT_MAX;
   }
   char * d_p = dest + *pLen;

   if (len && ((*src == ' ') || (*(c_p-1) == ' '))) {
      return 0;
   }else {
      int i;
      while (c_p-- > src) {
         unsigned char k;
         switch (*c_p) {
         case '0': k =  0; break;
         case '1': k =  1; break;
         case ' ': continue;
         default:  return 0;
         }
         if (--d_p < dest) return 0;
         i = 2;
         while ((c_p > src) && (i <= 128)) {
            switch (*--c_p) {
            case '0':         break;
            case '1': k += i; break;
            case ' ': if (i == 16) continue;  // allow spaces to occur at quad
            default:  return 0;
            }
            i <<= 1;
         }
         if (dest) *d_p = k;
      }
      *pLen -= (d_p - dest);
      if (pIsOdd) *pIsOdd = (i <= 16);
      return d_p;
   }
}

/*STATIC-------------------------------------------------RexxString::hexToBin-+
|                                                                             |
+----------------------------------------------------------------------------*/
char * RexxString::hexToBin(
   char const * src,
   int len,
   char * dest,
   int * pLen
) {
   char const * c_p = src + len;
   char * d_p = dest + *pLen;
   char * tgt = dest + 4;
   bool isBlankAllowed = true;

   if (len && ((*src == ' ') || (*(c_p-1) == ' '))) return 0;
   while (c_p-- > src) {
      if (d_p <= tgt) return 0;      // too short
      char const * pQuad;
      switch (*c_p) {
      case '0':             pQuad = "0000";  break;
      case '1':             pQuad = "0001";  break;
      case '2':             pQuad = "0010";  break;
      case '3':             pQuad = "0011";  break;
      case '4':             pQuad = "0100";  break;
      case '5':             pQuad = "0101";  break;
      case '6':             pQuad = "0110";  break;
      case '7':             pQuad = "0111";  break;
      case '8':             pQuad = "1000";  break;
      case '9':             pQuad = "1001";  break;
      case 'a':  case 'A':  pQuad = "1010";  break;
      case 'b':  case 'B':  pQuad = "1011";  break;
      case 'c':  case 'C':  pQuad = "1100";  break;
      case 'd':  case 'D':  pQuad = "1101";  break;
      case 'e':  case 'E':  pQuad = "1110";  break;
      case 'f':  case 'F':  pQuad = "1111";  break;
      case ' ':
         if (isBlankAllowed) continue;
         /* fall thru */
      default:
         return 0;
      }
      *--d_p = pQuad[3];
      *--d_p = pQuad[2];
      *--d_p = pQuad[1];
      *--d_p = pQuad[0];
      isBlankAllowed = !isBlankAllowed;
   }
   *pLen -= (d_p - dest);
   return d_p;
}

/*-----------------------------------------------------RexxString::Stringizer-+
|                                                                             |
+----------------------------------------------------------------------------*/
#if !defined COM_JAXO_YAXX_DENY_XML
void RexxString::Stringizer::setString(UCS_2 const * pUc, int iLen) {
   char * pTgt = m_res.resetString(iLen);
   while (iLen--) {
      *pTgt++ = (char)*pUc++;     // simple encoder (fix me?)
   }
}

void RexxString::Stringizer::str(MultibyteMemWriter & mbost) {
   int iLen = mbost.pcount();
   char * pTgt = m_res.resetString(iLen);
   char * pSrc = mbost.str();  // freeze
   memcpy(pTgt, pSrc, iLen);
   delete [] pSrc;
}

void RexxString::Stringizer::extractTagName(Tag const * pTag) {
   UnicodeString const & val = pTag->inqName();
   setString(val, val.length());
}

void RexxString::Stringizer::extractAttributeValue(
   Tag const * pTag,
   char const * attrName
) {
   UnicodeString const & val = pTag->inqAttributeValue(UnicodeString(attrName));
   setString(val, val.length());
}

void RexxString::Stringizer::inspectTagStart(Tag const & tag) {
   MultibyteMemWriter mbost;
   tag.stringize(mbost, false, false);
   str(mbost);
}

void RexxString::Stringizer::inspectTagEmpty(Tag const & tag) {
   inspectTagStart(tag);
}

void RexxString::Stringizer::inspectTagEnd(Tag const & tag) {
   MultibyteMemWriter mbost;
   tag.stringize(mbost, true);
   str(mbost);
}

void RexxString::Stringizer::inspectText(TextData const & text) {
   setString(text.inqText(), text.inqLength());
}

void RexxString::Stringizer::inspectCdataText(TextData const & text) {
   setString(text.inqText(), text.inqLength());
}

void RexxString::Stringizer::inspectSdataText(TextData const & text) {
   setString(text.inqText(), text.inqLength());
}

void RexxString::Stringizer::inspectPiText(TextData const & text) {
   setString(text.inqText(), text.inqLength());
}

void RexxString::Stringizer::inspectCommentText(TextData const & text) {
   setString(text.inqText(), text.inqLength());
}
#endif

#ifdef YAXX_NAMESPACE
}
#endif
/*===========================================================================*/
