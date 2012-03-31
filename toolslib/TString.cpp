/*
* $Id: TString.cpp,v 1.8 2003-01-26 10:59:51 pgr Exp $
*
* (C) Copyright Jaxo Systems, Inc. 2002
* This work contains confidential trade secrets of Jaxo.
* Use, examination, copying, transfer and disclosure to others
* are prohibited, except with the express written agreement of Jaxo.
*
* Author:  Pierre G. Richard
* Written: 12/15/2002
*
*/

#ifdef JAXO_INSTANCIATE_TEMPLATE   // otherwise, this file is not compiled

/*---------+
| Includes |
+---------*/
#include <new.h>
#include <assert.h>
#include <iostream.h>
#include <string.h> // memcpy, memcmp, strlen
#include "Subsequencer.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

template <class T> class TStringBuffer;
template <class T> TString<T> const TString<T>::Nil;
template <class T> TSubstring<T> const TSubstring<T>::Nil;
static char const bsDigit[] = "0123456789ABCDEF";

/*STATIC-------------------------------------------------------TString::alloc-+
|                                                                             |
+----------------------------------------------------------------------------*/
template <class T>
TString<T>::Rep * TString<T>::alloc(int bSize)
{
   Rep * pRep = (Rep *)new char[sizeof(Rep)+(bSize * sizeof(T))];
   if (pRep) {
      pRep->pTString = (T *)(pRep+1);
      pRep->iLength = bSize -1;
      pRep->iReferenceCount = 1;
      pRep->iGiven = 0;
   }
   return pRep;
}

/*STATIC---------------------------------------------------TString::strlength-+
|                                                                             |
+----------------------------------------------------------------------------*/
template <class T>
int TString<T>::strlength(T const * p) {
   T const * p0 = p;
   while (*p) { ++p;}
   return p - p0;
}

/*
template <>
int TString<char>::strlength(T const * p) {
   return ::strlen(p);
}


/*-----------------------------------------------------------TString::TString-+
| Constructor from a single T                                                 |
+----------------------------------------------------------------------------*/
template <class T>
TString<T>::TString(T b)
{
   pRep = alloc(1+1);
   if (pRep) {
      pRep->pTString[0] = b;
      pRep->pTString[1] = 0;
   }
}

/*-----------------------------------------------------------TString::TString-+
| Constructor from a T * "C" string                                           |
+----------------------------------------------------------------------------*/
template <class T>
TString<T>::TString(T const * pSource)
{
   if (pSource) {
      int const iLenTotal = 1 + strlength(pSource);
      pRep = alloc(iLenTotal);
      if (pRep) {
         // Faux!
         memcpy(pRep->pTString, pSource, iLenTotal * sizeof(T));
      }
   }else {
      pRep = 0;
   }
}

/*-----------------------------------------------------------TString::TString-+
| Constructor from a T "C" string, lengthed                                   |
+----------------------------------------------------------------------------*/
template <class T>
TString<T>::TString(T const * pSource, int iLen)
{
   if (pSource && iLen) {
      pRep = alloc(1+iLen);
      if (pRep) {
         memcpy(pRep->pTString, pSource, iLen * sizeof(T));
         pRep->pTString[iLen] = 0;
      }
   }else {
      pRep = 0;
   }
}

/*PRIVATE----------------------------------------------------TString::TString-+
| Constructor from a TString fragment                                         |
| This constructor does a deep copy.                                          |
| It is called by:                                                            |
|  - TSubstring::operator TString() const;  substring -> string               |
|    (only if the substring to convert is not the string: pos=0 len=max)      |
|  - TString TString::copy() const          deep copy                         |
+----------------------------------------------------------------------------*/
template <class T>
TString<T>::TString(TString<T> const & bs, int iPos, int iLen)
{
   if (bs.pRep) {
      T * pTString = bs.pRep->pTString;    // in case of new(*this, ...)
      pRep = alloc(iLen+1);
      if (pRep) {
         memcpy(pRep->pTString, pTString + iPos, iLen * sizeof(T));
         pRep->pTString[iLen] = 0;
      }
   }else {
      pRep = 0;
   }
}

/*RIVATE-----------------------------------------------------TString::TString-+
| Catenate two TString fragments.                                             |
| Pre-conditions:                                                             |
|     iLenX is > 0 => the corresponding T (sX) is real (not NULL)             |
|     iLenX is 0   => the corresponding T (sX) can be anything                |
+----------------------------------------------------------------------------*/
template <class T>
TString<T>::TString(
   TString<T> const & bs1, int iPos1, int iLen1,
   TString<T> const & bs2, int iPos2, int iLen2
) {
   if (iLen1 || iLen2) {
      pRep = alloc(iLen1 + iLen2 + 1);
      if (pRep) {
         if (iLen1) {
            memcpy(
               pRep->pTString,
               bs1.pRep->pTString + iPos1,
               iLen1 * sizeof(T)
            );
         }
         if (iLen2) {
            memcpy(
               pRep->pTString + iLen1,
               bs2.pRep->pTString + iPos2,
               iLen2 * sizeof(T)
            );
         }
         pRep->pTString[iLen1 + iLen2] = 0;
      }
   }else {
      pRep = 0;
   }
}

/*-----------------------------------------------------------TString::TString-+
| Copy constructor (shallow)                                                  |
+----------------------------------------------------------------------------*/
template <class T>
TString<T>::TString(TString<T> const & bsSource)
{
   pRep = bsSource.pRep;
   if (pRep) {
      if (pRep->iGiven == 1) {        // this is a flash! => deep copy
         new(this) TString(*this, 0, length());
      }else {
         ++pRep->iReferenceCount;
      }
   }
}

/*--------------------------------------------------------------TString::copy-+
| Copy method (deep copy)                                                     |
+----------------------------------------------------------------------------*/
template <class T>
TString<T> TString<T>::copy() const
{
   return TString(*this, 0, length());
}

/*---------------------------------------------------------TString::operator=-+
| Assignment                                                                  |
+----------------------------------------------------------------------------*/
template <class T>
TString<T> & TString<T>::operator=(TString<T> const & bsSource)
{
   if (pRep != bsSource.pRep) {
      this->~TString();
      new(this) TString(bsSource);
   }
   return *this;
}

/*PRIVATE--------------------------------------------TString::makeFromInteger-+
| Constructor from an int                                                     |
+----------------------------------------------------------------------------*/
template <class T>
void TString<T>::makeFromInteger(
   unsigned long ulVal, int iRadix, bool isNegative
) {
   T aBuf[33];
   T * p = aBuf;
   unsigned int iLen = isNegative? 2 : 1;

   *p = 0;
   do {                                          // convert
      *++p = bsDigit[(unsigned int)(ulVal%iRadix)], ++iLen;
   }while ((ulVal /= iRadix) != 0);

   pRep = alloc(iLen);
   if (pRep) {
      T * pTarget = pRep->pTString;  // reverse the string
      if (isNegative) {
         *pTarget++ = '-';
         --iLen;
      }
      do {
         *pTarget++ = *p--;
      }while (--iLen);
   }
}

/*----------------------------------------------------------TString::~TString-+
| Destructor                                                                  |
+----------------------------------------------------------------------------*/
template <class T>
TString<T>::~TString()
{
   if ((pRep) && (!--pRep->iReferenceCount)) {
      if (pRep->iGiven) {
         if (pRep->iGiven == 1) {
            pRep = 0;
            return;
         }
         delete [] pRep->pTString;
      }
      delete [] (char *) pRep;
   }
   pRep = 0;
}

/*-------------------------------------------------------------TString::lower-+
|                                                                             |
+----------------------------------------------------------------------------*/
template <class T>
void TString<T>::lower()
{
   if (pRep) {
      cow();
      T * p = pRep->pTString;
      int iLen = 1+pRep->iLength;
      while (--iLen) {
         if ((*p >= 'A') && (*p <= 'Z')) *p += ('a' - 'A');
         ++p;
      }
   }
}

/*-------------------------------------------------------------TString::upper-+
|                                                                             |
+----------------------------------------------------------------------------*/
template <class T>
void TString<T>::upper()
{
   if (pRep) {
      cow();
      T * p = pRep->pTString;
      int iLen = 1+pRep->iLength;
      while (--iLen) {
         if ((*p >= 'a') && (*p <= 'z')) *p += ('A' - 'a');
         ++p;
      }
   }
}

/*-----------------------------------------------------------TString::toLower-+
|                                                                             |
+----------------------------------------------------------------------------*/
template <class T>
TString<T> TString<T>::toLower() const
{
   TString<T> bsCopy(copy());
   bsCopy.lower();
   return bsCopy;
}

/*-----------------------------------------------------------TString::toUpper-+
|                                                                             |
+----------------------------------------------------------------------------*/
template <class T>
TString<T> TString<T>::toUpper() const
{
   TString<T> bsCopy(copy());
   bsCopy.upper();
   return bsCopy;
}

/*-----------------------------------------------------------TString::compare-+
| Compare two fragments of T array.                                           |
| Pre-conditions:                                                             |
|     iLenX is > 0 => the corresponding T (sX) is real (not NULL)             |
|     iLenX is 0   => the corresponding T (sX) can be anything                |
+----------------------------------------------------------------------------*/
template <class T>
inline int TString<T>::compare(
   T const * s1, int iLen1,
   T const * s2, int iLen2
) {
   // iLen = min(iLen1, iLen2) + 1 to later use a prefix decrement
   int iLen = 1 + ((iLen2 < iLen1)? iLen2 : iLen1);
   while (--iLen) {
      if (*s1++ != *s2++) return (s1[-1] - s2[-1]);
   }
   return iLen1 - iLen2;
}

/*--------------------------------------------------------TString::operator==-+
| Looking just for equality can be made faster than compare()                 |
+----------------------------------------------------------------------------*/
template <class T>
bool TString<T>::operator==(TString<T> const & bsSource) const
{
   int const i = length();
   if (
      (i == bsSource.length()) && (
         !i || !memcmp(
            pRep->pTString, bsSource.pRep->pTString, i * sizeof(T)
         )
      )
   ) {
      return true;
   }else {
      return false;
   }
}

/*-----------------------------------------------------------TString::compare-+
| Comparison (a la strcmp)                                                    |
+----------------------------------------------------------------------------*/
template <class T>
int TString<T>::compare(TString<T> const & bsSource) const
{
   if (pRep) {
      if (bsSource.pRep) {
         return compare(
            pRep->pTString, pRep->iLength,
            bsSource.pRep->pTString, bsSource.pRep->iLength
         );
      }else {
         return 1;
      }
   }else {
      if (bsSource.pRep && bsSource.pRep->iLength) {
         return -1;
      }else {
         return 0;
      }
   }
}

/*-------------------------------------------------------------TString::ncopy-+
| Safe copy to a buffer which has at most 'max' characters.                   |
| Note:                                                                       |
|    bs.ncopy(dest, count+1) is like strncpy(dest, this, count)               |
|    except that it doesn't pad the destination string with null              |
|    characters.                                                              |
+----------------------------------------------------------------------------*/
template <class T>
T * TString<T>::ncopy(T * buf, int max) const
{
   if ((max > 0) && buf) {
      if (!pRep) {
         buf[0] = 0;
      }else {
         int iLen = pRep->iLength;
         if (iLen >= max)  iLen = max - 1;
         memcpy(buf, pRep->pTString, iLen * sizeof(T));
         buf[iLen] = 0;
      }
   }
   return buf;
}

/*---------------------------------------------------------TString::operator+-+
| Catenation                                                                  |
+----------------------------------------------------------------------------*/
template <class T>
TString<T> TString<T>::operator+(T const * pCatenated) const
{
   return operator+(TStringFlash<T>(pCatenated));
}

/*-----------------------------------------------------------TString::compare-+
| Comparison (a la strcmp)                                                    |
+----------------------------------------------------------------------------*/
template <class T>
int TString<T>::compare(T const * pSource) const
{
   return compare(TStringFlash<T>(pSource));
}

/*-----------------------------------------------------------TString::compare-+
| Comparison (a la strncmp)                                                   |
+----------------------------------------------------------------------------*/
template <class T>
int TString<T>::compare(T const * pSource, int iLen) const
{
   return compare(TStringFlash<T>(pSource, iLen));
}

/*--------------------------------------------------------TString::operator==-+
| Equality                                                                    |
+----------------------------------------------------------------------------*/
template <class T>
bool TString<T>::operator==(T const * pSource) const
{
   if (!compare(pSource)) return true; else return false;
}

/*---------------------------------------------------------TString::operator>-+
| Greater than                                                                |
+----------------------------------------------------------------------------*/
template <class T>
bool TString<T>::operator>(T const * pSource) const
{
   if (compare(pSource) > 0) {
      return true;
   }else {
      return false;
   }
}

/*---------------------------------------------------------TString::operator>-+
| Greater than                                                                |
+----------------------------------------------------------------------------*/
template <class T>
bool TString<T>::operator<(T const * pSource) const
{
   if (compare(pSource) < 0) {
      return true;
   }else {
      return false;
   }
}

/*---------------------------------------------------------TString::operator+-+
| Catenation                                                                  |
+----------------------------------------------------------------------------*/
template <class T>
TString<T> TString<T>::operator+(
   TString<T> const & bs
) const {
   return TString(*this, 0, length(), bs, 0, bs.length());
}

/*--------------------------------------------------------SubsequencerDefault-+
|                                                                             |
+----------------------------------------------------------------------------*/
template <class T>
class SubsequencerDefault : public Subsequencer {
public:
   SubsequencerDefault(
      TString<T> const str1,
      TString<T> const str2,
      T skipMark
   );
   TStringBuffer<T> m_buf;
private:
   bool isEqual(int i1, int i2);
   void tellEqual(int i1, int i2);

   T const * m_pStr1;
   T const * m_pStr2;
   int m_i1;
   int m_i2;
   T m_skipMark;
};

template <class T>
SubsequencerDefault<T>::SubsequencerDefault(
   TString<T> const str1,
   TString<T> const str2,
   T skipMark
) :
   Subsequencer(str1.length(), str2.length()),
   m_pStr1(str1),
   m_pStr2(str2),
   // m skipMark(skipMark),  MSVC bug?
   m_buf(longestMaxLength()),
   m_i1(-1),
   m_i2(-1)
{
   m_skipMark = skipMark;   // MSVC bug fix
   getLongest();
}

template <class T>
bool SubsequencerDefault<T>::isEqual(int i1, int i2) {
   return ((m_pStr1[i1] == m_pStr2[i2]) && (m_pStr1[i1] != m_skipMark));
}

template <class T>
void SubsequencerDefault<T>::tellEqual(int i1, int i2) {
   if ((m_i1 > 0) && ((i1 > (m_i1+1)) || (i2 > (m_i2+1)))) {  // is ellipsed
      m_buf.append(m_skipMark);
   }
   m_buf.append(m_pStr1[i1]);
   m_i1 = i1;
   m_i2 = i2;
}

/*STATIC---------------------------------------TString::getLongestSubsequence-+
|                                                                             |
+----------------------------------------------------------------------------*/
template <class T>
TString<T> TString<T>::getLongestSubsequence(
   TString<T> const str1,
   TString<T> const str2,
   T skipMark
) {
   return SubsequencerDefault<T>(str1, str2, skipMark).m_buf;
}

/*-------------------------------------------------------------TString::write-+
| Output a string of T to a streambuf.                                        |
+----------------------------------------------------------------------------*/
template <class T>
bool TString<T>::write(streambuf * bp, int iLenMax) const
{
   if (iLenMax == -1) iLenMax = length();
   if (iLenMax) {
      iLenMax *= sizeof(T);  // get a byte count
      if (iLenMax != bp->sputn((char const *)pRep->pTString, iLenMax)) {
         return false;
      }
   }
   return true;
}

/*----------------------------------------------------------TSubstring::write-+
| Output a string of T to a streambuf.                                        |
+----------------------------------------------------------------------------*/
template <class T>
bool TSubstring<T>::write(streambuf * bp, int iLenMax) const
{
   if (iLenMax == -1) iLenMax = iLen;
   if (iLenMax) {
      iLenMax *= sizeof(T);  // get a byte count
      if (iLenMax != bp->sputn((char const *)inqPtr(), iLenMax)) {
         return false;
      }
   }
   return true;
}

/*---------------------------------------------------------------::operator<<-+
| Overloaded ostream insertion operator for TString: native output            |
+----------------------------------------------------------------------------*/
template <class T>
ostream & operator<<(ostream & out, TString<T> const & bs)
{
   if (bs.isVoid()) {
      out << "";
   }else {
      bs.write(out.rdbuf(), -1);
   }
   return out;
}

/*================================ TSubstring ================================*/


/*STATIC----------------------------------------------TSubstring::checkSanity-+
| Ensure the substring is valid inside the string                             |
+----------------------------------------------------------------------------*/
template <class T>
inline TString<T> const & TSubstring<T>::checkSanity(
   TString<T> const & bs,
   int & iPos,
   int & iLen
) {
   int iRealLen = bs.length();
   if ((unsigned int)iPos >= (unsigned int)iRealLen) {
      iPos = -1;
      iLen = 0;
      return TString<T>::Nil;
   }
   if ((iLen == LMAX) || ((iPos + iLen) > iRealLen)) {
      iLen = iRealLen - iPos;
   }
   return bs;
}

/*STATIC----------------------------------------------TSubstring::checkSanity-+
| Ensure the substring is valid inside the substring                          |
+----------------------------------------------------------------------------*/
template <class T>
inline TString<T> const & TSubstring<T>::checkSanity(
   TSubstring<T> const & bss,
   int & iPos,
   int & iLen
) {
   int iRealLen = bss.length();
   if ((unsigned int)(iPos) >= (unsigned int)iRealLen) {
      iPos = -1;
      iLen = 0;
      return TString<T>::Nil;
   }
   if ((iLen == LMAX) || ((iPos + iLen) > iRealLen)) {
      iLen = iRealLen - iPos;
   }
   iPos += bss.iPos;
   return bss.bs;
}

/*-----------------------------------------------------TSubstring::TSubstring-+
| Constructor                                                                 |
+----------------------------------------------------------------------------*/
template <class T>
TSubstring<T>::TSubstring(
   TString<T> const & bsArg, int iPosArg, int iLenArg
) :
   bs(checkSanity(bsArg, iPosArg, iLenArg)),
   iPos(iPosArg),
   iLen(iLenArg)
{}

/*-----------------------------------------------------TSubstring::TSubstring-+
| Constructor                                                                 |
+----------------------------------------------------------------------------*/
template <class T>
TSubstring<T>::TSubstring(
   TSubstring<T> const & bss, int iPosArg, int iLenArg
) :
   bs(checkSanity(bss, iPosArg, iLenArg)),
   iPos(iPosArg),
   iLen(iLenArg)
{}

/*---------------------------------------------------------TSubstring::initAt-+
| Set a new relative position                                                 |
+----------------------------------------------------------------------------*/
template <class T>
inline void TSubstring<T>::initAt(
   TSubstring<T> const & bssHayStack, int iPosRelative
) {
   iPos = bssHayStack.iPos + iPosRelative;
   iLen = bssHayStack.iLen - iPosRelative;
   bs = bssHayStack.bs;
}

/*---------------------------------------------------------TSubstring::initAt-+
| Set position starting at first/last substring in bssHayStack                |
| matching bsNeedle.                                                          |
+----------------------------------------------------------------------------*/
template <class T>
void TSubstring<T>::initAt(
   TSubstring<T> const & bssHayStack,
   T const * pNeedle, int iLenNeedle,
   e_Direction direction
) {
   if (!iLenNeedle) {
      *this = bssHayStack;
      return;
   }
   if (bssHayStack.iLen >= iLenNeedle) {
      T const * pHayStack = bssHayStack.inqPtr();
      T const * pEnd =
         pHayStack + (bssHayStack.iLen - --iLenNeedle);
      T const bFirst = *pNeedle++;
      iLenNeedle *= sizeof (T);
      if (direction == AT_FIRST) {
         T const * p = pHayStack;
         do {
            if (
               (*p == bFirst) &&
               (!memcmp(p+1, pNeedle, iLenNeedle))
            ) {
               initAt(bssHayStack, p - pHayStack);
               return;
            }
         }while (++p < pEnd);
      }else {
         while (--pEnd >= pHayStack) {   // no array bound problem here.
            if (
               (*pEnd == bFirst) &&
               (!memcmp(pEnd+1, pNeedle, iLenNeedle))
            ) {
               initAt(bssHayStack, pEnd - pHayStack);
               return;
            }
         }
      }
   }
   *this = Nil;
}

/*---------------------------------------------------------TSubstring::initAt-+
| Construct substring starting at first/last one_of/ none_of 'bsNeedles'      |
| in bssHayStack                                                              |
+----------------------------------------------------------------------------*/
template <class T>
void TSubstring<T>::initAt(
   TSubstring<T> const & bssHayStack,
   e_Choice choice,
   T const * pNeedles, int iLenNeedles,
   e_Direction direction
) {
   if (!iLenNeedles) {
      if (choice == ONE_OF) {
         *this = bssHayStack;
      }else {
         *this = Nil;
      }
      return;
   }
   if (!bssHayStack.iLen) {
      if (choice == ONE_OF) {
         *this = Nil;
      }else {
         *this = bssHayStack;
      }
      return;
   }
   T const * pHayStack = bssHayStack.inqPtr();
   T const * pEnd = pHayStack + bssHayStack.iLen;
   if (direction == AT_FIRST) {
      T const * p = pHayStack;
      if (choice == ONE_OF) {
         do {
            int i = 0;
            do {
               if (*p == pNeedles[i]) {
                  initAt(bssHayStack, p - pHayStack);
                  return;
               }
            }while (++i < iLenNeedles);
         }while (++p < pEnd);
      }else {
         do {
            int i=0;
            while (*p != pNeedles[i]) {
               if (++i >= iLenNeedles) {
                  initAt(bssHayStack, p - pHayStack);
                  return;
               }
            }
         }while (++p < pEnd);
      }
   }else {
      if (choice == ONE_OF) {
         while (--pEnd >= pHayStack) {
            int i = 0;
            do {
               if (*pEnd == pNeedles[i]) {
                  initAt(bssHayStack, pEnd - pHayStack);
                  return;
               }
            }while (++i < iLenNeedles);
         }
      }else {
         while (--pEnd >= pHayStack) {
            int i=0;
            while (*pEnd != pNeedles[i]) {
               if (++i >= iLenNeedles) {
                  initAt(bssHayStack, pEnd - pHayStack);
                  return;
               }
            }
         }
      }
   }
   *this = Nil;
}

/*---------------------------------------------------------TSubstring::initAt-+
| Do what strtok/strrtok does.                                                |
|                                                                             |
| Notes:                                                                      |
|   This is *not* a const method: "this" evolves containing what remains.     |
|   How?  Assume "abcXYZdef" is the string  and X,Y,Z, are the needles.       |
|                                                                             |
|   When going forward, at each iteration what remains starts with the first  |
|   needle: token = "abc", remains = "XYZdef".                                |
|   When going backward, at each iteration what remains ends with the last    |
|   needle: token = "def", remains = "abcXY"                                  |
|                                                                             |
+----------------------------------------------------------------------------*/
template <class T>
TSubstring<T> TSubstring<T>::initAt(
   T const * pNeedles, int iLenNeedles, e_Direction direction
) {
   TSubstring<T> bsToken;
   bsToken.initAt(
      *this, TSubstring<T>::NONE_OF,
      pNeedles, iLenNeedles, direction
   );
   if (bsToken.iLen) {
      if (direction == AT_FIRST) {
         initAt(
            bsToken, TSubstring<T>::ONE_OF,
            pNeedles, iLenNeedles, direction
         );
         bsToken.iLen -= iLen;
      }else {
         iLen -= bsToken.iLen;
         bsToken.initAt(
            *this, TSubstring<T>::ONE_OF,
            pNeedles, iLenNeedles, direction
         );
         if (bsToken.iLen) {
            ++bsToken.iPos;
            iLen -= bsToken.iLen;
         }else {
            bsToken = *this;
            ++bsToken.iLen;
            *this = Nil;
         }
      }
   }
   return bsToken;
}

/*-----------------------------------------------------TSubstring::TSubstring-+
| Construct substring starting at first/last 'bNeedle' in bssHayStack         |
+----------------------------------------------------------------------------*/
template <class T>
TSubstring<T>::TSubstring(
   TSubstring<T> const & bssHayStack,
   T bNeedle,
   e_Direction direction
) {
   if (bssHayStack.iLen) {
      T const * pHayStack = bssHayStack.inqPtr();
      T const * pEnd = pHayStack + bssHayStack.iLen;
      if (direction == AT_FIRST) {
         T const * p = pHayStack;
         do {
            if (*p == bNeedle) {
               initAt(bssHayStack, p - pHayStack);
               return;
            }
         }while (++p < pEnd);
      }else {
         while (--pEnd >= pHayStack) {
            if (*pEnd == bNeedle) {
               initAt(bssHayStack, pEnd - pHayStack);
               return;
            }
         }
      }
   }
   *this = Nil;
}

/*--------------------------------------------------------TSubstring::toLower-+
|                                                                             |
+----------------------------------------------------------------------------*/
template <class T>
TString<T> TSubstring<T>::toLower() const
{
   TString<T> bsCopy(*this);
   bsCopy.lower();
   return bsCopy;
}

/*--------------------------------------------------------TSubstring::toUpper-+
|                                                                             |
+----------------------------------------------------------------------------*/
template <class T>
TString<T> TSubstring<T>::toUpper() const
{
   TString<T> bsCopy(*this);
   bsCopy.upper();
   return bsCopy;
}

/*--------------------------------------------------------TSubstring::compare-+
| Comparison (a la strcmp)                                                    |
+----------------------------------------------------------------------------*/
template <class T>
int TSubstring<T>::compare(TSubstring<T> const & bssSrc) const
{
   return TString<T>::compare(
      inqPtr(), iLen, bssSrc.inqPtr(), bssSrc.iLen
   );
}

/*-----------------------------------------------------TSubstring::operator==-+
| Looking just for equality can be made faster than compare()                 |
+----------------------------------------------------------------------------*/
template <class T>
bool TSubstring<T>::operator==(TSubstring<T> const & bssSrc) const
{
   int const i = length();
   if (
      (i == bssSrc.length()) &&
      (!i || !memcmp(inqPtr(), bssSrc.inqPtr(), i * sizeof(T)))
   ) {
      return true;
   }else {
      return false;
   }
}

/*------------------------------------------------------TSubstring::operator+-+
| Catenation                                                                  |
+----------------------------------------------------------------------------*/
template <class T>
TString<T> TSubstring<T>::operator+(
   TSubstring<T> const & bss
) const {
   return TString<T>(bs, iPos, iLen, bss.bs, bss.iPos, bss.iLen);
}

/*---------------------------------------------------------------::operator<<-+
| Overloaded ostream insertion operator for TSubstrings: native output         |
+----------------------------------------------------------------------------*/
template <class T>
ostream & operator<<(ostream & out, TSubstring<T> const & bss)
{
   if (!bss.length()) {
      out << "";
   }else {
      bss.write(out.rdbuf(), -1);
   }
   return out;
}

/*-------------------------------------------------TStringFlash::TStringFlash-+
|                                                                             |
+----------------------------------------------------------------------------*/
template <class T>
TStringFlash<T>::TStringFlash(
   T const * p
) :
   TString<T>(rep)
{
   rep.iReferenceCount = 1;
   rep.iGiven = 1;
   rep.pTString = (T *)p;

   if (p) {
      rep.iLength = strlength(p);
   }else {
      rep.iLength = 0;
   }
}

/*-------------------------------------------------TStringFlash::TStringFlash-+
|                                                                             |
+----------------------------------------------------------------------------*/
template <class T>
TStringFlash<T>::TStringFlash(
   T const * p,
   int iLen
) :
   TString<T>(rep)
{
   rep.iReferenceCount = 1;
   rep.iGiven = 1;
   rep.pTString = (T *)p;
   rep.iLength = iLen;
}

/*--------------------------------------------------TStringFlash::resetBuffer-+
|                                                                             |
+----------------------------------------------------------------------------*/
template <class T>
void TStringFlash<T>::resetBuffer(T * p) {
   rep.pTString = p;
   rep.iLength = 0;
}

/*-------------------------------------------------TStringFlash::TStringFlash-+
|                                                                             |
+----------------------------------------------------------------------------*/
template <class T>
TStringFlash<T>::TStringFlash(
   TString<T> const & bs
) :
   TString(bs)
{
   rep.iReferenceCount = 0;
   rep.iGiven = 0;
   rep.pTString = (T *)0xDeadBeef;
   rep.iLength = 0;
}

// TStringFlash & TStringFlash::operator=(TString<T> const & bs) {
//    TString<T>::operator=(bs);
//    return *this;
// }


#ifdef TOOLS_NAMESPACE
}
#endif
#endif // JAXO_INSTANCIATE_TEMPLATE
/*===========================================================================*/
