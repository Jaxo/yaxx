/*
* $Id: ucstring.cpp,v 1.13 2011-07-29 10:26:38 pgr Exp $
*
* Unicode Strings
*/

/*--------------+
| Include Files |
+--------------*/
#include <new>
#include "ucstring.h"
#include "mbstring.h"
#include "Encoder.h"
#include "Writer.h"

#ifdef __MWERKS_NEEDS_REALLOC_FIX__
#define realloc reallocFix4Mwerks
extern void *reallocFix4Mwerks(void *ptr, size_t size);
#endif

#ifdef __MWERKS_NEEDS_MEMCMP_FIX__
#define memcmp memcmpFix4PalmOS30
#include "../../palmiolib/memcmp.h"
#endif

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

UnicodeString const UnicodeString::Nil;
UnicodeSubstring const UnicodeSubstring::Nil;
static UnicodeString const ucsDigit("0123456789ABCDEF");

/*----------------------------------------------------------------::strlength-+
|                                                                             |
+----------------------------------------------------------------------------*/
int strlength(UCS_2 const * pUc)
{
    if (pUc && *pUc) {
       UCS_2 const * p = pUc;
       while (*++p)
          ;
       return p - pUc;
    }
    return 0;
}

/*STATIC--------------------------------------------UnicodeString::Rep::alloc-+
|                                                                             |
+----------------------------------------------------------------------------*/
inline UnicodeString::Rep * UnicodeString::Rep::alloc(int ucSize)
{
   Rep * pRep = (Rep *)new char[sizeof(Rep)+(ucSize * sizeof(UCS_2))];
   if (pRep) {
      pRep->pUcString = (UCS_2 *)(pRep+1);
      pRep->iLength = ucSize -1;
      pRep->iReferenceCount = 1;
      pRep->iGiven = 0;
   }
   return pRep;
}

/*-----------------------------------------------UnicodeString::UnicodeString-+
| Constructor from char (latin)                                               |
+----------------------------------------------------------------------------*/
UnicodeString::UnicodeString(char c)
{
   pRep = Rep::alloc(1+1);
   if (pRep) {
      pRep->pUcString[0] = (UCS_2)c;
      pRep->pUcString[1] = 0;
   }
}

/*-----------------------------------------------UnicodeString::UnicodeString-+
| Constructor from char * (latin string)                                      |
+----------------------------------------------------------------------------*/
UnicodeString::UnicodeString(char const* pszSource)
{
   if (pszSource) {
      pRep = Rep::alloc(1+strlen(pszSource));
      if (pRep) {
         UCS_2 * pUc = pRep->pUcString;
         while (*pszSource) {
            *pUc++ = (UCS_2)(*pszSource++);
         }
         *pUc = 0;
      }
   }else {
      pRep = 0;
   }
}

/*-----------------------------------------------UnicodeString::UnicodeString-+
| Constructor from char * (latin string), lengthed                            |
+----------------------------------------------------------------------------*/
UnicodeString::UnicodeString(char const* pszSource, int iLen)
{
   if (pszSource && (iLen > 0)) {
      pRep = Rep::alloc(1+iLen);
      if (pRep) {
         UCS_2 * pUc = pRep->pUcString;
         while (iLen--) {
            *pUc++ = (UCS_2)(*pszSource++);
         }
         *pUc = 0;
      }
   }else {
      pRep = 0;
   }
}

/*-----------------------------------------------UnicodeString::UnicodeString-+
| Constructor from char * (encoded bytes) lengthed                            |
+----------------------------------------------------------------------------*/
UnicodeString::UnicodeString(
   char const * pszSource, Encoder const & ecm, int iLenSource
) {
   if (pszSource && iLenSource) {
      pRep = Rep::alloc(1+iLenSource);            // might be to big...
      if (pRep) {
         UCS_2 * pUc = pRep->pUcString;
         ecm.target2unicode(pszSource, iLenSource, pUc, iLenSource);
         pRep->iLength = pUc - pRep->pUcString;   // adjust to exact size
         *pUc = 0;
      }
   }else {
      pRep = 0;
   }
}

/*-----------------------------------------------UnicodeString::UnicodeString-+
| Constructor from encoded char *, zero terminated                            |
+----------------------------------------------------------------------------*/
UnicodeString::UnicodeString(
   char const* pszSource, Encoder const & ecm
) {
   if (pszSource) {
      int iLenSource = strlen(pszSource);
      pRep = Rep::alloc(1+iLenSource);            // might be to big...
      if (pRep) {
         UCS_2 * pUc = pRep->pUcString;
         ecm.target2unicode(pszSource, iLenSource, pUc, iLenSource);
         pRep->iLength = pUc - pRep->pUcString;   // adjust to exact size
         *pUc = 0;
      }
   }else {
      pRep = 0;
   }
}

/*-----------------------------------------------UnicodeString::UnicodeString-+
| Constructor from a single UCS_2                                         |
+----------------------------------------------------------------------------*/
UnicodeString::UnicodeString(UCS_2 uc)
{
   pRep = Rep::alloc(1+1);
   if (pRep) {
      pRep->pUcString[0] = uc;
      pRep->pUcString[1] = 0;
   }
}

/*-----------------------------------------------UnicodeString::UnicodeString-+
| Constructor from a UCS_2 C string, lengthed                                 |
+----------------------------------------------------------------------------*/
UnicodeString::UnicodeString(UCS_2 const * pUcSource, int iLen)
{
   if (pUcSource && iLen) {
      pRep = Rep::alloc(1+iLen);
      if (pRep) {
         memcpy(pRep->pUcString, pUcSource, iLen * sizeof(UCS_2));
         pRep->pUcString[iLen] = 0;
      }
   }else {
      pRep = 0;
   }
}

/*-----------------------------------------------UnicodeString::UnicodeString-+
| Constructor from a UnicodeMemWriter                                         |
| Note: the stream must not be "<< ends" prior to this call.                  |
+----------------------------------------------------------------------------*/
UnicodeString::UnicodeString(UnicodeMemWriter & umost)
{
   umost << UCS_2(0);                 // add extra ending zero
   if (!umost) {
      pRep = 0;
   }else if (pRep = Rep::alloc(0), pRep) {
      pRep->iLength = umost.pcount() - 1;
      pRep->pUcString = umost.str();      // freeze
      umost.reset();                      // empty the stream
      if (pRep->pUcString && pRep->iLength) { // check non empty
         if (!umost.isFixed()) {
            pRep->iGiven = 1;             // we owe contents
         }
      }else {
         if (!umost.isFixed()) {
            delete [] pRep->pUcString;
         }
         delete [] (char *)pRep;
         pRep = 0;
      }
   }
}

/*PRIVATE----------------------------------------UnicodeString::UnicodeString-+
| Constructor from a UnicodeString fragment                                   |
| This constructor does a deep copy.                                          |
| It is called by:                                                            |
|  - UnicodeSubstring::operator UnicodeString() const;  substring -> string   |
|    (only if the substring to convert is not the string: pos=0 len=max)      |
|  - UnicodeString UnicodeString::copy() const          deep copy             |
+----------------------------------------------------------------------------*/
UnicodeString::UnicodeString(UnicodeString const & ucs, int iPos, int iLen)
{
   if (ucs.pRep) {
      Rep * pRepTemp = Rep::alloc(iLen+1);
      if (pRepTemp) {
         memcpy(
            pRepTemp->pUcString,
            ucs.pRep->pUcString + iPos,
            iLen * sizeof(UCS_2)
         );
         pRepTemp->pUcString[iLen] = 0;
         pRep = pRepTemp;
      }
   }else {
      pRep = 0;
   }
}


/*PRIVATE----------------------------------------UnicodeString::UnicodeString-+
| Catenate two UnicodeString fragments.                                       |
| Pre-conditions:                                                             |
|     iLenX is > 0 => the corresponding UCS_2 (sX) is real (not NULL)         |
|     iLenX is 0   => the corresponding UCS_2 (sX) can be anything            |
+----------------------------------------------------------------------------*/
UnicodeString::UnicodeString(
   UnicodeString const & ucs1, int iPos1, int iLen1,
   UnicodeString const & ucs2, int iPos2, int iLen2
) {
   if (iLen1 || iLen2) {
      pRep = Rep::alloc(iLen1 + iLen2 + 1);
      if (pRep) {
         if (iLen1) {
            memcpy(
               pRep->pUcString,
               ucs1.pRep->pUcString + iPos1,
               iLen1 * sizeof(UCS_2)
            );
         }
         if (iLen2) {
            memcpy(
               pRep->pUcString + iLen1,
               ucs2.pRep->pUcString + iPos2,
               iLen2 * sizeof(UCS_2)
            );
         }
         pRep->pUcString[iLen1 + iLen2] = 0;
      }
   }else {
      pRep = 0;
   }
}

/*-----------------------------------------------UnicodeString::UnicodeString-+
| Copy constructor (shallow)                                                  |
+----------------------------------------------------------------------------*/
UnicodeString::UnicodeString(UnicodeString const & ucsSource)
{
   pRep = ucsSource.pRep;
   if (pRep) {
      if (pRep->iGiven == 2) {        // this is a flash! => deep copy
         new(this) UnicodeString(*this, 0, length());
      }else {
         ++pRep->iReferenceCount;
      }
   }
}

/*--------------------------------------------------------UnicodeString::copy-+
| Copy method (deep copy)                                                     |
+----------------------------------------------------------------------------*/
UnicodeString UnicodeString::copy() const
{
   return UnicodeString(*this, 0, length());
}

/*---------------------------------------------------UnicodeString::operator=-+
| Assignment                                                                  |
+----------------------------------------------------------------------------*/
UnicodeString & UnicodeString::operator=(UnicodeString const & ucsSource)
{
   if (pRep != ucsSource.pRep) {
      this->~UnicodeString();
      new(this) UnicodeString(ucsSource);
   }
   return *this;
}

/*PRIVATE--------------------------------------UnicodeString::makeFromInteger-+
| Constructor from an int                                                     |
+----------------------------------------------------------------------------*/
void UnicodeString::makeFromInteger(
   unsigned long ulVal, int iRadix, bool isNegative
) {
   UCS_2 aUcBuf[33];
   UCS_2 * pUc = aUcBuf;
   unsigned int iLen = isNegative? 2 : 1;

   *pUc = 0;
   do {                                          // convert
      *++pUc = ucsDigit[(unsigned int)(ulVal%iRadix)], ++iLen;
   }while ((ulVal /= iRadix) != 0);

   pRep = Rep::alloc(iLen);
   if (pRep) {
      UCS_2 * pUcTarget = pRep->pUcString;  // reverse the string
      if (isNegative) {
         *pUcTarget++ = '-';
         --iLen;
      }
      do {
         *pUcTarget++ = *pUc--;
      }while (--iLen);
   }
}

/*----------------------------------------------UnicodeString::~UnicodeString-+
| Destructor                                                                  |
+----------------------------------------------------------------------------*/
UnicodeString::~UnicodeString()
{
   if ((pRep) && (!--pRep->iReferenceCount)) {
      if (pRep->iGiven) {
         if (pRep->iGiven == 2) {
            pRep = 0;
            return;
         }
         delete [] pRep->pUcString;
      }
      delete [] (char *) pRep;
   }
   pRep = 0;
}


/*-------------------------------------------------------UnicodeString::lower-+
|                                                                             |
+----------------------------------------------------------------------------*/
void UnicodeString::lower() {
   if (pRep) {
      cow();
      UCS_2 * p = pRep->pUcString;
      int iLen = 1+pRep->iLength;
      while (--iLen) {
         if ((*p >= 'A') && (*p <= 'Z')) *p += ('a' - 'A');
         ++p;
      }
   }
}

/*-------------------------------------------------------UnicodeString::upper-+
|                                                                             |
+----------------------------------------------------------------------------*/
void UnicodeString::upper() {
   if (pRep) {
      cow();
      UCS_2 * p = pRep->pUcString;
      int iLen = 1+pRep->iLength;
      while (--iLen) {
         if ((*p >= 'a') && (*p <= 'z')) *p += ('A' - 'a');
         ++p;
      }
   }
}

/*-----------------------------------------------------UnicodeString::toLower-+
|                                                                             |
+----------------------------------------------------------------------------*/
UnicodeString UnicodeString::toLower() const
{
   UnicodeString ucsCopy(copy());
   ucsCopy.lower();
   return ucsCopy;
}

/*-----------------------------------------------------UnicodeString::toUpper-+
|                                                                             |
+----------------------------------------------------------------------------*/
UnicodeString UnicodeString::toUpper() const
{
   UnicodeString ucsCopy(copy());
   ucsCopy.upper();
   return ucsCopy;
}

/*-----------------------------------------------------UnicodeString::compare-+
| Compare two fragments of UCS_2 array.                                       |
| Pre-conditions:                                                             |
|     iLenX is > 0 => the corresponding UCS_2 (sX) is real (not NULL)         |
|     iLenX is 0   => the corresponding UCS_2 (sX) can be anything            |
+----------------------------------------------------------------------------*/
inline int UnicodeString::compare(
   UCS_2 const * s1, int iLen1,
   UCS_2 const * s2, int iLen2
) {
   // iLen = min(iLen1, iLen2) + 1 to later use a prefix decrement
   int iLen = 1 + ((iLen2 < iLen1)? iLen2 : iLen1);
   while (--iLen) {
      if (*s1++ != *s2++) return (s1[-1] - s2[-1]);
   }
   return iLen1 - iLen2;
}

/*--------------------------------------------------UnicodeString::operator==-+
| Looking just for equality can be made faster than compare()                 |
+----------------------------------------------------------------------------*/
bool UnicodeString::operator==(UnicodeString const & ucsSource) const
{
   int const i = length();
   if (
      (i == ucsSource.length()) && (
         !i || !memcmp(
            pRep->pUcString, ucsSource.pRep->pUcString, i * sizeof(UCS_2)
         )
      )
   ) {
      return true;
   }else {
      return false;
   }
}

/*-----------------------------------------------------UnicodeString::compare-+
| Comparison (a la strcmp)                                                    |
+----------------------------------------------------------------------------*/
int UnicodeString::compare(UnicodeString const & ucsSource) const
{
   if (pRep) {
      if (ucsSource.pRep) {
         return compare(
            pRep->pUcString, pRep->iLength,
            ucsSource.pRep->pUcString, ucsSource.pRep->iLength
         );
      }else {
         return 1;
      }
   }else {
      if (ucsSource.pRep && ucsSource.pRep->iLength) {
         return -1;
      }else {
         return 0;
      }
   }
}

/*---------------------------------------------------UnicodeString::operator+-+
| Catenation                                                                  |
+----------------------------------------------------------------------------*/
UnicodeString UnicodeString::operator+(
   UnicodeString const & ucs
) const {
   return UnicodeString(*this, 0, length(), ucs, 0, ucs.length());
}

/*-------------------------------------------------------UnicodeString::write-+
| Output a string of UCS_2 to a streambuf.                                    |
+----------------------------------------------------------------------------*/
bool UnicodeString::write(streambuf * bp, int iLenMax) const
{
   if (iLenMax == -1) iLenMax = length();
   if (iLenMax) {
      iLenMax *= sizeof(UCS_2);  // get a byte count
      if (iLenMax != bp->sputn((char const *)pRep->pUcString, iLenMax)) {
         return false;
      }
   }
   return true;
}

/*----------------------------------------------------UnicodeSubstring::write-+
| Output a string of UCS_2 to a streambuf.                                    |
+----------------------------------------------------------------------------*/
bool UnicodeSubstring::write(streambuf * bp, int iLenMax) const
{
   if (iLenMax == -1) iLenMax = iLen;
   if (iLenMax) {
      iLenMax *= sizeof(UCS_2);  // get a byte count
      if (iLenMax != bp->sputn((char const *)inqUcPtr(), iLenMax)) {
         return false;
      }
   }
   return true;
}

/*-------------------------------------------------------UnicodeString::write-+
| Output a string of encoded bytes to a streambuf.                            |
+----------------------------------------------------------------------------*/
bool UnicodeString::write(
   streambuf * bp, int iLenMax, Encoder const & enc
) const {
   if (iLenMax == -1) iLenMax = length();
   if (iLenMax && !enc.sputnMb(bp, pRep->pUcString, iLenMax)) {
      return false;
   }
   return true;
}

/*----------------------------------------------------UnicodeSubstring::write-+
| Output a string of encoded bytes to a streambuf.                            |
+----------------------------------------------------------------------------*/
bool UnicodeSubstring::write(
   streambuf * bp, int iLenMax, Encoder const & enc
) const {
   if (iLenMax == -1) iLenMax = iLen;
   if (iLenMax && !enc.sputnMb(bp, inqUcPtr(), iLenMax)) {
      return false;
   }
   return true;
}

/*---------------------------------------------------------------::operator<<-+
| Overloaded ostream insertion operator for UnicodeString: native output      |
+----------------------------------------------------------------------------*/
ostream & operator<<(ostream & out, UnicodeString const & ucs)
{
   if (!ucs.good()) {
      out << "(Nil)";
   }else {
      Encoder enc;
      ucs.write(out.rdbuf(), -1, enc);
   }
   return out;
}
ostream & operator<<(ostream& out, UCS_2 const * pUc)
{
   if (!pUc) {
      out << "(NULL)";
   }else {
      Encoder encoder;
      encoder.sputnMb(out.rdbuf(), pUc, strlength(pUc));
   }
   return out;
}


/*---------------------------------------------------------------::operator<<-+
| Overloaded ostream insertion operator for UnicodeSubstrings: native output  |
+----------------------------------------------------------------------------*/
ostream & operator<<(ostream & out, UnicodeSubstring const & ucss)
{
   Encoder enc;
   if (!ucss.length()) {
      out << "(Nil)";
   }else {
      ucss.write(out.rdbuf(), -1, enc);
   }
   return out;
}


/*============================ UnicodeSubstring =============================*/


/*STATIC----------------------------------------UnicodeSubstring::checkSanity-+
| Ensure the substring is valid inside the string                             |
+----------------------------------------------------------------------------*/
inline UnicodeString const & UnicodeSubstring::checkSanity(
   UnicodeString const & ucs,
   int & iPos,
   int & iLen
) {
   int iRealLen = ucs.length();
   if ((unsigned int)iPos >= (unsigned int)iRealLen) {
      iPos = -1;
      iLen = 0;
      return UnicodeString::Nil;
   }
   if ((iLen == LMAX) || ((iPos + iLen) > iRealLen)) {
      iLen = iRealLen - iPos;
   }
   return ucs;
}

/*STATIC----------------------------------------UnicodeSubstring::checkSanity-+
| Ensure the substring is valid inside the substring                          |
+----------------------------------------------------------------------------*/
inline UnicodeString const & UnicodeSubstring::checkSanity(
   UnicodeSubstring const & ucss,
   int & iPos,
   int & iLen
) {
   int iRealLen = ucss.length();
   if ((unsigned int)(iPos) >= (unsigned int)iRealLen) {
      iPos = -1;
      iLen = 0;
      return UnicodeString::Nil;
   }
   if ((iLen == LMAX) || ((iPos + iLen) > iRealLen)) {
      iLen = iRealLen - iPos;
   }
   iPos += ucss.iPos;
   return ucss.ucs;
}

/*-----------------------------------------UnicodeSubstring::UnicodeSubstring-+
| Constructor                                                                 |
+----------------------------------------------------------------------------*/
UnicodeSubstring::UnicodeSubstring(
   UnicodeString const & ucsArg, int iPosArg, int iLenArg
) :
   ucs(checkSanity(ucsArg, iPosArg, iLenArg)),
   iPos(iPosArg),
   iLen(iLenArg)
{}

/*-----------------------------------------UnicodeSubstring::UnicodeSubstring-+
| Constructor                                                                 |
+----------------------------------------------------------------------------*/
UnicodeSubstring::UnicodeSubstring(
   UnicodeSubstring const & ucss, int iPosArg, int iLenArg
) :
   ucs(checkSanity(ucss, iPosArg, iLenArg)),
   iPos(iPosArg),
   iLen(iLenArg)
{}

/*---------------------------------------------------UnicodeSubstring::initAt-+
| Set a new relative position                                                 |
+----------------------------------------------------------------------------*/
inline void UnicodeSubstring::initAt(
   UnicodeSubstring const & ucssHayStack, int iPosRelative
) {
   iPos = ucssHayStack.iPos + iPosRelative;
   iLen = ucssHayStack.iLen - iPosRelative;
   ucs = ucssHayStack.ucs;
}

/*---------------------------------------------------UnicodeSubstring::initAt-+
| Set position starting at first/last substring in ucssHayStack               |
| matching ucsNeedle.                                                         |
+----------------------------------------------------------------------------*/
void UnicodeSubstring::initAt(
   UnicodeSubstring const & ucssHayStack,
   UCS_2 const * pUcNeedle, int iLenNeedle,
   e_Direction direction
) {
   if (!iLenNeedle) {
      *this = ucssHayStack;
      return;
   }
   if (ucssHayStack.iLen >= iLenNeedle) {
      UCS_2 const * pUcHayStack = ucssHayStack.inqUcPtr();
      UCS_2 const * pUcEnd =
         pUcHayStack + (ucssHayStack.iLen - --iLenNeedle);
      UCS_2 const ucFirst = *pUcNeedle++;
      iLenNeedle *= sizeof (UCS_2);
      if (direction == AT_FIRST) {
         UCS_2 const * pUc = pUcHayStack;
         do {
            if (
               (*pUc == ucFirst) &&
               (!memcmp(pUc+1, pUcNeedle, iLenNeedle))
            ) {
               initAt(ucssHayStack, pUc - pUcHayStack);
               return;
            }
         }while (++pUc < pUcEnd);
      }else {
         while (--pUcEnd >= pUcHayStack) {   // no array bound problem here.
            if (
               (*pUcEnd == ucFirst) &&
               (!memcmp(pUcEnd+1, pUcNeedle, iLenNeedle))
            ) {
               initAt(ucssHayStack, pUcEnd - pUcHayStack);
               return;
            }
         }
      }
   }
   *this = Nil;
}

/*---------------------------------------------------UnicodeSubstring::initAt-+
| Construct substring starting at first/last one_of/ none_of 'ucsNeedles'     |
| in ucssHayStack                                                             |
+----------------------------------------------------------------------------*/
void UnicodeSubstring::initAt(
   UnicodeSubstring const & ucssHayStack,
   e_Choice choice,
   UCS_2 const * pUcNeedles, int iLenNeedles,
   e_Direction direction
) {
   if (!iLenNeedles) {
      if (choice == ONE_OF) {
         *this = ucssHayStack;
      }else {
         *this = Nil;
      }
      return;
   }
   if (!ucssHayStack.iLen) {
      if (choice == ONE_OF) {
         *this = Nil;
      }else {
         *this = ucssHayStack;
      }
      return;
   }
   UCS_2 const * pUcHayStack = ucssHayStack.inqUcPtr();
   UCS_2 const * pUcEnd = pUcHayStack + ucssHayStack.iLen;
   if (direction == AT_FIRST) {
      UCS_2 const * pUc = pUcHayStack;
      if (choice == ONE_OF) {
         do {
            int i = 0;
            do {
               if (*pUc == pUcNeedles[i]) {
                  initAt(ucssHayStack, pUc - pUcHayStack);
                  return;
               }
            }while (++i < iLenNeedles);
         }while (++pUc < pUcEnd);
      }else {
         do {
            int i=0;
            while (*pUc != pUcNeedles[i]) {
               if (++i >= iLenNeedles) {
                  initAt(ucssHayStack, pUc - pUcHayStack);
                  return;
               }
            }
         }while (++pUc < pUcEnd);
      }
   }else {
      if (choice == ONE_OF) {
         while (--pUcEnd >= pUcHayStack) {
            int i = 0;
            do {
               if (*pUcEnd == pUcNeedles[i]) {
                  initAt(ucssHayStack, pUcEnd - pUcHayStack);
                  return;
               }
            }while (++i < iLenNeedles);
         }
      }else {
         while (--pUcEnd >= pUcHayStack) {
            int i=0;
            while (*pUcEnd != pUcNeedles[i]) {
               if (++i >= iLenNeedles) {
                  initAt(ucssHayStack, pUcEnd - pUcHayStack);
                  return;
               }
            }
         }
      }
   }
   *this = Nil;
}

/*---------------------------------------------------UnicodeSubstring::initAt-+
| Do what strysp/strrysp does.                                                |
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
UnicodeSubstring UnicodeSubstring::initAt(
   UCS_2 const * pUcNeedles, int iLenNeedles, e_Direction direction
) {
   UnicodeSubstring ucsToken;
   ucsToken.initAt(
      *this, UnicodeSubstring::NONE_OF,
      pUcNeedles, iLenNeedles, direction
   );
   if (ucsToken.iLen) {
      if (direction == AT_FIRST) {
         initAt(
            ucsToken, UnicodeSubstring::ONE_OF,
            pUcNeedles, iLenNeedles, direction
         );
         ucsToken.iLen -= iLen;
      }else {
         iLen -= ucsToken.iLen;
         ucsToken.initAt(
            *this, UnicodeSubstring::ONE_OF,
            pUcNeedles, iLenNeedles, direction
         );
         if (ucsToken.iLen) {
            ++ucsToken.iPos;
            iLen -= ucsToken.iLen;
         }else {
            ucsToken = *this;
            ++ucsToken.iLen;
            *this = Nil;
         }
      }
   }
   return ucsToken;
}

/*-----------------------------------------UnicodeSubstring::UnicodeSubstring-+
| Construct substring starting at first/last 'ucNeedle' in ucssHayStack       |
+----------------------------------------------------------------------------*/
UnicodeSubstring::UnicodeSubstring(
   UnicodeSubstring const & ucssHayStack,
   UCS_2 ucNeedle,
   e_Direction direction
) {
   if (ucssHayStack.iLen) {
      UCS_2 const * pUcHayStack = ucssHayStack.inqUcPtr();
      UCS_2 const * pUcEnd = pUcHayStack + ucssHayStack.iLen;
      if (direction == AT_FIRST) {
         UCS_2 const * pUc = pUcHayStack;
         do {
            if (*pUc == ucNeedle) {
               initAt(ucssHayStack, pUc - pUcHayStack);
               return;
            }
         }while (++pUc < pUcEnd);
      }else {
         while (--pUcEnd >= pUcHayStack) {
            if (*pUcEnd == ucNeedle) {
               initAt(ucssHayStack, pUcEnd - pUcHayStack);
               return;
            }
         }
      }
   }
   *this = Nil;
}


/*--------------------------------------------------UnicodeSubstring::toLower-+
|                                                                             |
+----------------------------------------------------------------------------*/
UnicodeString UnicodeSubstring::toLower() const
{
   UnicodeString ucsCopy(*this);
   ucsCopy.lower();
   return ucsCopy;
}

/*--------------------------------------------------UnicodeSubstring::toUpper-+
|                                                                             |
+----------------------------------------------------------------------------*/
UnicodeString UnicodeSubstring::toUpper() const
{
   UnicodeString ucsCopy(*this);
   ucsCopy.upper();
   return ucsCopy;
}

/*--------------------------------------------------UnicodeSubstring::compare-+
| Comparison (a la strcmp)                                                    |
+----------------------------------------------------------------------------*/
int UnicodeSubstring::compare(UnicodeSubstring const & ucssSrc) const
{
   return UnicodeString::compare(
      inqUcPtr(), iLen, ucssSrc.inqUcPtr(), ucssSrc.iLen
   );
}

/*-----------------------------------------------UnicodeSubstring::operator==-+
| Looking just for equality can be made faster than compare()                 |
+----------------------------------------------------------------------------*/
bool UnicodeSubstring::operator==(UnicodeSubstring const & ucssSrc) const
{
   int const i = length();
   if (
      (i == ucssSrc.length()) &&
      (!i || !memcmp(inqUcPtr(), ucssSrc.inqUcPtr(), i * sizeof(UCS_2)))
   ) {
      return true;
   }else {
      return false;
   }
}


/*------------------------------------------------UnicodeSubstring::operator+-+
| Catenation                                                                  |
+----------------------------------------------------------------------------*/
UnicodeString UnicodeSubstring::operator+(
   UnicodeSubstring const & ucss
) const {
   return UnicodeString(ucs, iPos, iLen, ucss.ucs, ucss.iPos, ucss.iLen);
}

/**/
/**/
/**/
/**/
/*================= From Here To The End: Migration Only! ===================*/
/**/
/**/
/**/
/**/

/*-----------------------------------------------UnicodeBuffer::UnicodeBuffer-+
|                                                                             |
+----------------------------------------------------------------------------*/
UnicodeBuffer::UnicodeBuffer(int iSize)
{
   pUcBuf = (UCS_2 *)malloc(iSize * sizeof(UCS_2));
   if (!pUcBuf) {
      iLenBuf = 0;
   }else {
      iLenBuf = iSize;
   }
   iLen = 0;
   iCount = 0;
   pUcCurr = pUcBuf;
   pUcToken = pUcBuf;
}

/*----------------------------------------------------UnicodeBuffer::makeRoom-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool UnicodeBuffer::makeRoom()
{
   UCS_2 * pUcNewBuf;
   if (pUcBuf && (
         pUcNewBuf = (UCS_2 *)realloc(
            pUcBuf, (iLenBuf + SIZE_INCR) * sizeof(UCS_2)
         ), pUcNewBuf
      )
   ) {
      pUcCurr = pUcNewBuf + (pUcCurr - pUcBuf);
      pUcToken = pUcNewBuf + (pUcToken - pUcBuf);
      pUcBuf = pUcNewBuf;
      iLenBuf += SIZE_INCR;
      return true;
   }else {
      if (pUcBuf) {        // the first time this happens...
         free(pUcBuf);
         pUcBuf = 0;
      }
      return false;
   }
}

/*-----------------------------------------------UnicodeString::UnicodeString-+
| Constructor from a UCS_2 * "C" string                                       |
+----------------------------------------------------------------------------*/
UnicodeString::UnicodeString(UCS_2 const * pUcSource)
{
   if (pUcSource) {
      int const iLenTotal = 1 + strlength(pUcSource);
      pRep = Rep::alloc(iLenTotal);
      if (pRep) {
         memcpy(pRep->pUcString, pUcSource, iLenTotal * sizeof(UCS_2));
      }
   }else {
      pRep = 0;
   }
}

/*-----------------------------------------------UnicodeString::UnicodeString-+
| Constructor from a constant UCS_2 * string, alive at least for the          |
| duration of the UnicodeString.                                              |
+----------------------------------------------------------------------------*/
UnicodeString::UnicodeString(UCS_2 const * pUcSource, e_Constant)
{
   if (pUcSource && (pRep=Rep::alloc(0), pRep)) {
      pRep->pUcString = (UCS_2 *)pUcSource; // violates constness
      pRep->iLength = strlength(pUcSource);
   }else {                                      // that we will honor anyway
      pRep = 0;
   }
}

/*-----------------------------------------------UnicodeString::UnicodeString-+
| Constructor from a UCS_2 * string on the free store.                        |
| Ownership transfered.                                                       |
+----------------------------------------------------------------------------*/
UnicodeString::UnicodeString(UCS_2 * pUcSource, e_Given)
{
   if (pUcSource && (pRep=Rep::alloc(0), pRep)) {
      pRep->pUcString = pUcSource;
      pRep->iLength = strlength(pUcSource);
      pRep->iGiven = 1;
   }else {
      pRep = 0;
   }
}

/*-------------------------------------------------------UnicodeString::ncopy-+
| Safe copy to a buffer which has at most 'max' characters.                   |
| Note:                                                                       |
|    ucs.ncopy(dest, count+1) is like strncpy(dest, this, count)              |
|    except that it doesn't pad the destination string with null              |
|    characters.                                                              |
+----------------------------------------------------------------------------*/
UCS_2 * UnicodeString::ncopy(UCS_2 * buf, int max) const
{
   if ((max > 0) && buf) {
      if (!pRep) {
         buf[0] = 0;
      }else {
         int iLen = pRep->iLength;
         if (iLen >= max)  iLen = max - 1;
         memcpy(buf, pRep->pUcString, iLen * sizeof(UCS_2));
         buf[iLen] = 0;
      }
   }
   return buf;
}

/*---------------------------------------------------UnicodeString::operator+-+
| Catenation                                                                  |
+----------------------------------------------------------------------------*/
UnicodeString UnicodeString::operator+(UCS_2 const * pUcCatenated) const
{
   return operator+(UcStringFlash(pUcCatenated));
}

/*-----------------------------------------------------UnicodeString::compare-+
| Comparison (a la strcmp)                                                    |
+----------------------------------------------------------------------------*/
int UnicodeString::compare(UCS_2 const * pUcSource) const
{
   return compare(UcStringFlash(pUcSource));
}

/*-----------------------------------------------------UnicodeString::compare-+
| Comparison (a la strncmp)                                                   |
| UCS_2 version only!  Migrated code must use UnicodeSubstring                |
+----------------------------------------------------------------------------*/
int UnicodeString::compare(UCS_2 const * pUcSource, int iLen) const
{
   return compare(UcStringFlash(pUcSource, iLen));
}

/*--------------------------------------------------UnicodeString::operator==-+
| Equality                                                                    |
+----------------------------------------------------------------------------*/
bool UnicodeString::operator==(UCS_2 const * pUcSource) const
{
   if (!compare(pUcSource)) return true; else return false;
}

/*---------------------------------------------------UnicodeString::operator>-+
| Greater than                                                                |
+----------------------------------------------------------------------------*/
bool UnicodeString::operator>(UCS_2 const * pUcSource) const
{
   if (compare(pUcSource) > 0) {
      return true;
   }else {
      return false;
   }
}

/*---------------------------------------------------UnicodeString::operator>-+
| Greater than                                                                |
+----------------------------------------------------------------------------*/
bool UnicodeString::operator<(UCS_2 const * pUcSource) const
{
   if (compare(pUcSource) < 0) {
      return true;
   }else {
      return false;
   }
}

/*-----------------------------------------------UcStringFlash::UcStringFlash-+
| *** VERY DANGEROUS TO USE - you'd better know what you're doing             |
|                                                                             |
+----------------------------------------------------------------------------*/
UcStringFlash::UcStringFlash(
   UCS_2 const * pUc
) :
   UnicodeString(*(Rep *)&buf)
{
   Rep & rep = *(Rep *)&buf;
   rep.iReferenceCount = 1;
   rep.iGiven = 2;
   rep.pUcString = (UCS_2 *)pUc;
   if (pUc) {
      rep.iLength = strlength(pUc);
   }else {
      rep.iLength = 0;
   }
}

UcStringFlash::UcStringFlash(
   UCS_2 const * pUc,
   int iLen
) :
   UnicodeString(*(Rep *)&buf)
{
   Rep & rep = *(Rep *)&buf;
   rep.iReferenceCount = 1;
   rep.iGiven = 2;
   rep.pUcString = (UCS_2 *)pUc;
   rep.iLength = iLen;
}

void UcStringFlash::resetBuffer(UCS_2 * pUc) {
   Rep & rep = *(Rep *)&buf;
   rep.pUcString = pUc;
   rep.iLength = 0;
}

UcStringFlash::UcStringFlash(UnicodeString const & ucs) : UnicodeString(ucs) {
   Rep & rep = *(Rep *)&buf;
   rep.iReferenceCount = 0;
   rep.iGiven = 0;
   rep.pUcString = (UCS_2 *)0xDeadBeef;
   rep.iLength = 0;
}

// UcStringFlash & UcStringFlash::operator=(UnicodeString const & ucs) {
//    UnicodeString::operator=(ucs);
//    return *this;
// }

#ifdef TOOLS_NAMESPACE
}
#endif
/*===========================================================================*/
