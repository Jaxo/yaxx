/*
* $Id: ucformat.cpp,v 1.5 2011-07-29 10:26:38 pgr Exp $
*
*  Unicode Formatters
*/

/*--------------+
| Include Files |
+--------------*/
#include <iostream>
#include <limits.h>
#include "ucformat.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*--------------------------------------UnicodeExtractor::operator>>(UCS_2 *)-+
|                                                                             |
+----------------------------------------------------------------------------*/
UnicodeExtractor & UnicodeExtractor::operator>>(UCS_2 * pUc)
{
   if (pUc && !state) {
      int iLenMax = width(-1);
      if (iLenMax < 0) {
         iLenMax = INT_MAX;
      }else if (--iLenMax < 0) {
         return *this;
      }
      UCS_2 k = eatWhiteSpace();
      int iLen = 0;
      if (isSpace(k)) {
         while ((iLen < iLenMax) && k && isSpace(k)) {
            *pUc++ = k;
            int peeked = peek(++iLen);
            if (peeked == EOF) break;
            k = peeked;
         }
      }
      while ((iLen < iLenMax) && k && !isSpace(k)) {
         *pUc++ = k;
         int peeked = peek(++iLen);
         if (peeked == EOF) break;
         k = peeked;
      }
      *pUc = 0;
      if (iLen) {
         skip(iLen);
      }
   }
   return *this;
}

/*--------------------------------------------UnicodeExtractor::eatWhiteSpace-+
|                                                                             |
+----------------------------------------------------------------------------*/
UCS_2 UnicodeExtractor::eatWhiteSpace()
{
   UCS_2 k;
   int peeked;
   if (x_flags & ios::skipws) {
      int iOffset = -1;
      do {
         k = peeked = peek(++iOffset);
      }while ((peeked != EOF) && isSpace(k));
      if (iOffset) skip(iOffset);
   }else {
      k = peeked = peek(0);
   }
   if (peeked == EOF) return 0; else return k;
}

/*STATIC------------------------------------------UnicodeExtractor::ucToDigit-+
|                                                                             |
+----------------------------------------------------------------------------*/
inline int UnicodeExtractor::ucToDigit(UCS_2 k)
{
   switch (k) {
   case (UCS_2)'0': return 0;
   case (UCS_2)'1': return 1;
   case (UCS_2)'2': return 2;
   case (UCS_2)'3': return 3;
   case (UCS_2)'4': return 4;
   case (UCS_2)'5': return 5;
   case (UCS_2)'6': return 6;
   case (UCS_2)'7': return 7;
   case (UCS_2)'8': return 8;
   case (UCS_2)'9': return 9;
   case (UCS_2)'a':
   case (UCS_2)'A': return 10;
   case (UCS_2)'b':
   case (UCS_2)'B': return 11;
   case (UCS_2)'c':
   case (UCS_2)'C': return 12;
   case (UCS_2)'d':
   case (UCS_2)'D': return 13;
   case (UCS_2)'e':
   case (UCS_2)'E': return 14;
   case (UCS_2)'f':
   case (UCS_2)'F': return 15;
   default :            return 255;
   }
}

/*STATIC-----------------------------------------UnicodeExtractor::isHexDigit-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool UnicodeExtractor::isHexDigit(UCS_2 uc) {
   if (ucToDigit(uc) < 16) return true; else return false;
}

/*-------------------------------------------------UnicodeExtractor::getValue-+
| Common routine to all integer conversions                                   |
+----------------------------------------------------------------------------*/
int UnicodeExtractor::getValue(e_IntegerType iType)
{
   static const struct BaseValue {
      int iRadix, iMaxByRadix, iMaxModRadix;
      bool isSigned;
   } v[][6] = {         // v[radix][integer type]
      {
         { 16,  SHRT_MAX / 16,  SHRT_MAX % 16, true  },  // IT_SHORT
         { 16, USHRT_MAX / 16, USHRT_MAX % 16, false },  // IT_USHORT
         { 16,   INT_MAX / 16,   INT_MAX % 16, true  },  // IT_INT
         { 16,  UINT_MAX / 16,  UINT_MAX % 16, false },  // IT_UINT
         { 16,  SHRT_MAX / 16,  SHRT_MAX % 16, true  },  // IT_LONG
         { 16, USHRT_MAX / 16, USHRT_MAX % 16, false }   // IT_ULONG
      },                                                 //
      {                                                  //
         {  8,  SHRT_MAX /  8,  SHRT_MAX %  8, true  },  // IT_SHORT
         {  8, USHRT_MAX /  8, USHRT_MAX %  8, false },  // IT_USHORT
         {  8,   INT_MAX /  8,   INT_MAX %  8, true  },  // IT_INT
         {  8,  UINT_MAX /  8,  UINT_MAX %  8, false },  // IT_UINT
         {  8,  SHRT_MAX /  8,  SHRT_MAX %  8, true  },  // IT_LONG
         {  8, USHRT_MAX /  8, USHRT_MAX %  8, false }   // IT_ULONG
      },                                                 //
      {                                                  //
         { 10,  SHRT_MAX / 10,  SHRT_MAX % 10, true  },  // IT_SHORT
         { 10, USHRT_MAX / 10, USHRT_MAX % 10, false },  // IT_USHORT
         { 10,   INT_MAX / 10,   INT_MAX % 10, true  },  // IT_INT
         { 10,  UINT_MAX / 10,  UINT_MAX % 10, false },  // IT_UINT
         { 10,  SHRT_MAX / 10,  SHRT_MAX % 10, true  },  // IT_LONG
         { 10, USHRT_MAX / 10, USHRT_MAX % 10, false }   // IT_ULONG
      }
   };

   BaseValue b;
   switch (x_flags & (ios::dec | ios::hex | ios::oct)) {
   case ios::hex:  b = v[0][iType];  break;
   case ios::oct:  b = v[1][iType];  break;
   default:        b = v[2][iType];  break;
   }
   UCS_2 k = eatWhiteSpace();
   e_Sign sign;
   int iLen;
   if (b.isSigned) {
      switch (k) {
      case (UCS_2)'-': sign = NEGATIVE; break;
      case (UCS_2)'+': sign = POSITIVE; break;
      default:             sign = NO_SIGN;  break;
      }
      if (sign != NO_SIGN) {
         iLen = 1;
         k = peek(1);
      }else {
         iLen = 0;
      }
   }else {
      sign = NO_SIGN;
      iLen = 0;
   }
   int iValue = 0;
   int i = ucToDigit(k);
   if (i < b.iRadix) {
      do iValue = (iValue * b.iRadix) + i; while (
         (i = ucToDigit(peek(++iLen)), i < b.iRadix) && (
            (iValue < (b.iMaxByRadix)) ||
            ((iValue <= (b.iMaxByRadix)) && (i <= (b.iMaxModRadix)))
         )
      );
   }
   switch (sign) {
   case NEGATIVE:  iValue = -iValue;
   case POSITIVE:  if (iLen == 1) --iLen;
   default:        break;
   }
   if (iLen) {
      skip(iLen);
   }else {
      state |= ios::failbit;
   }
   return iValue;
}
#ifdef TOOLS_NAMESPACE
}
#endif
/*===========================================================================*/
