#include <new.h>
#include <ctype.h>
#include <iostream.h>
#include <stdio.h>
#include <memory.h>
#define DECNUMDIGITS 1
#include "../decnblib/DecRexx.h"
// #include "../decnblib/DecNumber.h"
#include "../decnblib/decNumberLocal.h"

inline bool isSpace(char c) {
   return ((c <=  ' ') && ((c==' ')||(c == '\n')||(c=='\t')||(c=='\r')));
}
/*
unsigned int const powers[] = {
   1, 10, 100, 1000, 10000, 100000, 1000000,
   10000000, 100000000, 1000000000
};
*/

/*-------------------------------------------------------------------decStrEq-+
| the order of str1 and str2 is significant and str2 must be lowercase        |
+----------------------------------------------------------------------------*/
static bool decStrEq(char const * str1, char const * str2)
{
   while (*str2) if (*str2++ != (*str1++ | 0x20)) return false;
   while (isSpace(*str1)) { ++str1; } // skip trailing spaces
   return (*str1 == '\0');
}

/*-------------------------------------------------------------------allocDec-+
|                                                                             |
+----------------------------------------------------------------------------*/
char * allocDec(int digits) {
   return new char[
      sizeof (DecNumber) + ((D2U(digits) - 1) * sizeof (DecNumberUnit))
   ];
}

/*----------------------------------------------------------------------parse-+
|                                                                             |
+----------------------------------------------------------------------------*/
DecNumber * parse(char const * pSource, int & status)
{
   // how to fool a C++ compiler (not a recommended example)
   DecNumber * pDn;
   new(&pDn) DecRexx(pSource);

   if (pDn->bits & DECNAN) {
      status = DEC_Conversion_syntax;
   }else if (pDn->bits & DECINF) {
      status = DEC_Conversion_overflow | DEC_Inexact | DEC_Rounded;
   }else if (pDn->bits2 & DECSUBN) {
      status = DEC_Conversion_underflow | DEC_Inexact | DEC_Rounded;
   }else {
      status = 0;
   }
   return pDn;
}

/*----------------------------------------------------------------------parse-+
|                                                                             |
+----------------------------------------------------------------------------*/
DecNumber * parse(char const * pSource, int & status, bool isExtended)
{
   DecNumber * pDn = parse(pSource, status);
   if (isExtended) {
      uint8_t bits;
      if (pDn->bits2 & DECZNEG) {           // -0
         bits = DECNEG;
      }else if (pDn->bits & DECNAN) {       // DEC_Conversion Syntax occured
         // look for sNaN (signalable Nan) or Infinities
         bits = 0;
         if (*pSource == ' ')  while (isSpace(*++pSource)) {} // skip leading spaces
         switch (*pSource) {

         case 's':                          // sNan
         case 'S':
            // NaNs are only valid at the start of the string (no sign)
            if (!decStrEq(++pSource, "nan")) return pDn;
            bits = DECSNAN;
            break;

         case 'n':
         case 'N':
            if (decStrEq(pSource, "nan")) {  // Nan
               status &= ~DEC_Conversion_syntax;   // valid DECNAN
            }
            return pDn;
            break;

         case '-':                         // allow a sign to occur
            bits = DECNEG;
            // fall thru
         case '+':
            while (isSpace(*++pSource)) {} // skip intermediate spaces
            // fall thru
         case 'I':                         // infinities
         case 'i':
            if (!decStrEq(pSource, "infinity") && !decStrEq(pSource, "inf")) {
               return pDn;
            }
            bits |= DECINF;
            break;

         default:
            return pDn;
         }
      }else {
         return pDn;
      }

      // -0, sNan of Inf recognized -- do not delete specials Dn's
      if (pDn->iReferenceCount != 1) {
         pDn = (DecNumber *)allocDec(1);
         decNumberZero(pDn);
         pDn->iReferenceCount = 1;
      }
      pDn->bits = bits;
      status &= ~DEC_Conversion_syntax;
   }
   return pDn;
}

/*----------------------------------------------------------------------parse-+
|                                                                             |
+----------------------------------------------------------------------------*/
DecNumber * parse(unsigned int val)
{
   // how to fool a C++ compiler (not a recommended example)
   DecNumber * pDn;
   new(&pDn) DecRexx(val);
   return pDn;
}

/*===========================================================================*/
