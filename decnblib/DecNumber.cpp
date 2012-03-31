#define MIKE 0  // disabled -- mainly for small diff in pows
/* $Id: DecNumber.cpp,v 1.29 2011-07-29 10:26:34 pgr Exp $
*
* Decimal Number arithmetic module
*
* Copyright (c) IBM Corporation, 2000, 2002.  All rights reserved.
*
* This software is made available under the terms of the IBM
* alphaWorks License Agreement (distributed with this software as
* alphaWorks-License.txt).  Your use of this software indicates
* your acceptance of the terms and conditions of that Agreement.
*
* The description and User's Guide ("The DecNumber C Library") for
* this software is included in the package as DecNumber.pdf.  This
* document is also available in HTML, together with specifications,
* testcases, and Web links, at: http://www2.hursley.ibm.com/decimal
*
* Please send comments, suggestions, and corrections to the author:
*   mfc@uk.ibm.com
*   Mike Cowlishaw, IBM Fellow
*   IBM UK, PO Box 31, Birmingham Road, Warwick CV34 5JL, UK
*
* This module comprises the routines for Standard Decimal Arithmetic
* as defined in the base and extended specifications (please see
* http://www2.hursley.ibm.com/decimal for details).
*
* Usage notes:
*
* 1. The DecNumber format which this library uses is optimized for efficient
*    processing; in particular it minimizes copy and move operations.  It is
*    arbitrary precision (up to 999,999,999 digits) and arbitrary exponent
*    range (Emax in the range 0 through 999,999,999).
*
* 2. Operands to operator functions are never modified unless they are
*    also specified to be the result number (which is always permitted).
*
* 3. The result of any routine which returns a number will always be a valid
*    number (which may be a special value, such as an Infinity or NaN, after
*    an error).
*
* 4. The routines will never cause exceptions if they are given well-formed
*    operands, even if the value of the operands is inappropriate for the
*    operation.
*
* 5. The DecNumber format is not an exchangeable concrete representation
*    as it comprises fields which may be machine-dependent (big-endian or
*    little-endian).  Canonical conversions to and from strings are provided.
*
*
* Implementation:
*
* 1. Exponent checking is minimized by allowing the exponent to
*    grow outside its limits during calculations, provided that
*    the decFinish function is called later.  Multiplication and
*    division, and intermediate calculations in exponentiation,
*    require more careful checks because of the risk of 31-bit
*    overflow (the most negative valid exponent is -1999999997, for
*    a 999999999-digit number with adjusted exponent of -999999999).
*
* 2. The digits count is allowed to rise to a multiple of DECDPUN
*    during many operations, so whole Units are handled and exact
*    accounting of digits is not needed.  The correct digits value
*    is found by decGetDigits, which accounts for leading zeros.
*    This should be called before any rounding.
*
* 3. We use the multiply-by-reciprocal 'trick' for partitioning
*    numbers up to four digits, using appropriate constants.  This
*    is not useful for longer numbers because overflow of 32 bits
*    would lead to 4 multiplies, which is almost as expensive as
*    a divide (unless we assumed floating-point multiply available).
*
* 4. Unusual abbreviations possibly used in the commentary:
*      lhs -- left hand side (operand, of an operation)
*      lsd -- least significant digit (of coefficient)
*      lsu -- least significant Unit (of coefficient)
*      msd -- most significant digit (of coefficient)
*      msu -- most significant Unit (of coefficient)
*      rhs -- right hand side (operand, of an operation)
*      +ve -- positive
*      -ve -- negative
* ===========================================================================*/

#ifdef __MWERKS_NEEDS_CTYPE_FIX__
#include "../../palmiolib/ctype_fix.h"
#else
#include <ctype.h>            // for lower
#endif

#include <stdint.h>
#include "stdint.h"           // C99 standard integers
#include "DecNumber.h"        // base number library
#include "DecNumberLocal.h"   // DecNumber local types, etc.
#include <stdlib.h>           // for malloc, free, etc.
#include <string.h>           // for strcpy

/* Constants */
// public constant array: powers of ten (powers[n]==10**n)
const uint32_t powers[] = {
   1, 10, 100, 1000, 10000, 100000, 1000000,
   10000000, 100000000, 1000000000
};

#ifndef INT_MAX
#define INT_MAX  0x7FFFFFFF   // maximum (signed) int value
#endif

#ifdef _WIN32
#include "..\toolslib\dlmalloc.h"
#define malloc(a) dlmalloc(a)
#define free(a) dlfree(a)
#endif

#define DECBUFFER 36          // Maximum size basis for local buffers.
                              // Should be a common maximum precision
                              // rounded up to a multiple of 4; must
                              // be non-negative.

static DecNumberUnit one[]={1};    // Unit array of 1, used for incrementing

/* Granularity-dependent code */
#if DECDPUN<=4
  #define eInt  int32_t       // extended integer
  #define ueInt uint32_t      // unsigned extended integer
  // Constant multipliers for divide-by-power-of five using reciprocal
  // multiply, after removing powers of 2 by shifting, and final shift
  // of 17 [we only need up to **4]
  static const uint32_t multies[]={131073, 26215, 5243, 1049, 210};
  // QUOT10 -- macro to return the quotient of unit u divided by 10**n
  #define QUOT10(u, n) (((unsigned)(u)>>(n))*multies[n])>>17;
#else
  // For DECDPUN>4 we currently use non-ANSI 64-bit types.  These could
  // be replaced by subroutine calls later.
  #ifdef _WIN32
  typedef signed __int64 eInt;
  typedef unsigned __int64 ueInt;
  #else
  typedef signed long long eInt;
  typedef unsigned long long ueInt;
  #endif
#endif

/* local routines */
static uint32_t decStatus(
   DecNumber * dn,
   uint32_t status
);
static int32_t decCompare(
   DecNumber const *,
   DecNumber const *
);
static uint32_t decFinish(
   DecNumber *,
   DecContext const *
);
static int32_t decGetDigits(
   DecNumberUnit *, int32_t
);
static uint32_t decRound(
   DecNumber *, DecContext const *, DecNumberUnit *, int32_t, int32_t
);
static DecNumber * decRoundOperand(
   DecNumber const *, DecContext const *, uint32_t *
);
static uint32_t decSetOverflow(
   DecNumber *, DecContext const *, uint8_t
);
static uint32_t decSetUnderflow(
   DecNumber *, DecContext const *
);
static void  decShiftToLeast(
   DecNumberUnit *, int32_t, int32_t
);
static int32_t decShiftToMost(
   DecNumberUnit *, int32_t, int32_t
);
static void  decToString(
   DecNumber const *, char[], uint8_t
);
static void  decTrim(
   DecNumber *, uint8_t all
);
static int32_t decUnitAddSub(
   DecNumberUnit const *, int32_t,
   DecNumberUnit const *, int32_t, int32_t,
   DecNumberUnit *, int32_t
);
static int32_t decUnitCompare(
   DecNumberUnit const *, uint32_t,
   DecNumberUnit const *, uint32_t,
   int32_t
);

/* ========================================================================= */
/* Conversions                                                               */
/* ========================================================================= */

/*----------------------------------------------------------decNumberToString-+
|   to-scientific-string -- conversion to numeric string                      |
|   to-engineering-string -- conversion to numeric string                     |
|                                                                             |
|     decNumberToString(dn, string);                                          |
|     decNumberToEngString(dn, string);                                       |
|                                                                             |
|    dn is the DecNumber to convert                                           |
|    string is the string where the result will be laid out                   |
|                                                                             |
|    string must be at least dn->digits+14 characters long                    |
|                                                                             |
|    No error is possible, and no status can be set.                          |
+----------------------------------------------------------------------------*/
char * decNumberToString(DecNumber const * dn, char * string) {
   decToString(dn, string, DECTOSTR2_EXPFORM_ALLOWED);
   return string;
}
char * decNumberToEngString(DecNumber const * dn, char * string) {
   decToString(dn, string, DECTOSTR2_ENG | DECTOSTR2_EXPFORM_ALLOWED);
   return string;
}
char * decNumberToString2(DecNumber const * dn, char * string, uint8_t flag){
   decToString(dn, string, flag);
   return string;
}

/*------------------------------------------------------------------decStatus-+
|   decStatus -- apply non-zero status                                        |
|                                                                             |
|     dn     is the number to set if error.                                   |
|     status contains the current status (not yet in context)                 |
|                                                                             |
|   If the status is an error status, the number is set to a NaN,             |
|   unless the error was an overflow, divide-by-zero, or underflow,           |
|   in which case the number will have already been set.                      |
+----------------------------------------------------------------------------*/
static inline uint32_t decStatus(DecNumber *dn, uint32_t status)
{
   if (status & DEC_NaNs) {              // error status -> NaN
      decNumberZero(dn);                 // ensure clean throughout
      dn->bits = DECNAN;                 // and make a quiet NaN
   }
   return status;
}

/* ========================================================================= */
/* Operators                                                                 */
/* ========================================================================= */

/*---------------------------------------------------------------decNumberAbs-+
|   decNumberAbs -- absolute value operator                                   |
|                                                                             |
|     This computes C = abs(A)                                                |
|                                                                             |
|     res is C, the result.  C may be A                                       |
|     rhs is A                                                                |
|     set is the context                                                      |
|                                                                             |
|   C must have space for set->digits digits.                                 |
|                                                                             |
|   This has the same effect as decNumberPlus unless A is negative,           |
|   in which case it has the same effect as decNumberMinus.                   |
+----------------------------------------------------------------------------*/
uint32_t decNumberAbs(
   DecNumber * res,
   DecNumber const * rhs,
   DecContext const *set
) {
   DecNumber dzero;                      // for 0
   uint8_t bits;                         // safe byte
   bits=rhs->bits;
   decNumberZero(&dzero);                // set 0

   return decAddOp(res, &dzero, rhs, set, bits & DECNEG);
}

/*---------------------------------------------------------------decNumberAdd-+
|   decNumberAdd -- add two Numbers                                           |
|                                                                             |
|     This computes C = A + B                                                 |
|                                                                             |
|     res is C, the result.  C may be A and/or B (e.g., X=X+X)                |
|     lhs is A                                                                |
|     rhs is B                                                                |
|     set is the context                                                      |
|                                                                             |
|   C must have space for set->digits digits.                                 |
|                                                                             |
|   This just calls the routine shared with Subtract                          |
+----------------------------------------------------------------------------*/
uint32_t decNumberAdd(
   DecNumber * res,
   DecNumber const * lhs,
   DecNumber const * rhs,
   DecContext const *set
) {
   return decAddOp(res, lhs, rhs, set, 0);
}

/*-----------------------------------------------------------decNumberCompare-+
|   decNumberCompare -- compare two Numbers                                   |
|                                                                             |
|     This computes C = A ? B                                                 |
|                                                                             |
|     res is C, the result.  C may be A and/or B (e.g., X=X?X)                |
|     lhs is A                                                                |
|     rhs is B                                                                |
|     set is the context                                                      |
|                                                                             |
|   C must have space for one digit.                                          |
+----------------------------------------------------------------------------*/
uint32_t decNumberCompare(
   DecNumber * res,
   DecNumber const * lhs,
   DecNumber const * rhs,
   DecContext const *set
) {
   return decCompareOp(res, lhs, rhs, set, COMPARE);
}

/*------------------------------------------------------------decNumberDivide-+
|   decNumberDivide -- divide one number by another                           |
|                                                                             |
|     This computes C = A / B                                                 |
|                                                                             |
|     res is C, the result.  C may be A and/or B (e.g., X=X/X)                |
|     lhs is A                                                                |
|     rhs is B                                                                |
|     set is the context                                                      |
|                                                                             |
|   C must have space for set->digits digits.                                 |
+----------------------------------------------------------------------------*/
uint32_t decNumberDivide(
   DecNumber * res,
   DecNumber const * lhs,
   DecNumber const * rhs,
   DecContext const *set
) {
   return decDivideOp(res, lhs, rhs, set, DEC_OP_DIVIDE);
}

/*-----------------------------------------------------decNumberDivideInteger-+
|   decNumberDivideInteger -- divide and return integer quotient              |
|                                                                             |
|     This computes C = A # B, where # is the integer divide operator         |
|                                                                             |
|     res is C, the result.  C may be A and/or B (e.g., X=X#X)                |
|     lhs is A                                                                |
|     rhs is B                                                                |
|     set is the context                                                      |
|                                                                             |
|   C must have space for set->digits digits.                                 |
+----------------------------------------------------------------------------*/
uint32_t decNumberDivideInteger(
   DecNumber * res,
   DecNumber const * lhs,
   DecNumber const * rhs,
   DecContext const *set
) {
   return decDivideOp(res, lhs, rhs, set, DEC_OP_DIVIDEINT);
}

/*---------------------------------------------------------------decNumberMax-+
|   decNumberMax -- compare two Numbers and return the maximum                |
|                                                                             |
|     This computes C = A ? B, returning the maximum or A if equal            |
|                                                                             |
|     res is C, the result.  C may be A and/or B (e.g., X=X?X)                |
|     lhs is A                                                                |
|     rhs is B                                                                |
|     set is the context                                                      |
|                                                                             |
|   C must have space for set->digits digits.                                 |
+----------------------------------------------------------------------------*/
uint32_t decNumberMax(
   DecNumber * res,
   DecNumber const * lhs,
   DecNumber const * rhs,
   DecContext const *set
) {
   return decCompareOp(res, lhs, rhs, set, COMPMAX);
}

/*---------------------------------------------------------------decNumberMin-+
|   decNumberMin -- compare two Numbers and return the minimum                |
|                                                                             |
|     This computes C = A ? B, returning the minimum or A if equal            |
|                                                                             |
|     res is C, the result.  C may be A and/or B (e.g., X=X?X)                |
|     lhs is A                                                                |
|     rhs is B                                                                |
|     set is the context                                                      |
|                                                                             |
|   C must have space for set->digits digits.                                 |
+----------------------------------------------------------------------------*/
uint32_t decNumberMin(
   DecNumber * res,
   DecNumber const * lhs,
   DecNumber const * rhs,
   DecContext const *set
) {
   return decCompareOp(res, lhs, rhs, set, COMPMIN);
}

/*----------------------------------------------------------decNumberMultiply-+
|   decNumberMultiply -- multiply two Numbers                                 |
|                                                                             |
|     This computes C = A x B                                                 |
|                                                                             |
|     res is C, the result.  C may be A and/or B (e.g., X=X+X)                |
|     lhs is A                                                                |
|     rhs is B                                                                |
|     set is the context                                                      |
|                                                                             |
|   C must have space for set->digits digits.                                 |
|                                                                             |
|   Note: We use 'long' multiplication rather than Karatsuba, as the          |
|   latter would give only a minor improvement for the short numbers          |
|   we expect to handle most (and uses much more memory).                     |
|                                                                             |
|   We always have to use a buffer for the accumulator.                       |
+----------------------------------------------------------------------------*/
uint32_t decNumberMultiply(
   DecNumber * res,
   DecNumber const * lhs,
   DecNumber const * rhs,
   DecContext const *set
) {
   DecNumber *alloclhs = 0;          // non-NULL if rounded lhs allocated
   DecNumber *allocrhs = 0;          // .., rhs
   DecNumberUnit accbuff[D2U(DECBUFFER*2+1)]; // local buffer (+1 in case DECBUFFER==0)
   DecNumberUnit *acc=accbuff;       // -> accumulator array for exact result
   DecNumberUnit *allocacc=NULL;     // -> allocated buffer, iff allocated
   DecNumberUnit const * mer;        // work
   DecNumberUnit const * mermsup;    // ...
   uint32_t status=0;                // status accumulator
   int32_t  reqdigits = set->digits; // requested digits
   int32_t  accunits;                // Units of accumulator in use
   int32_t  madlength;               // Units in multiplicand
   int32_t  shift;                   // Units to shift multiplicand by
   int32_t  exponent;                // result exponent
   uint8_t bits;                     // result sign
   uint8_t merged;                   // merged flags


   do {                              // protect allocated storage
      bits = (uint8_t)((lhs->bits^rhs->bits)&DECNEG);   // result sign

      // reduce operands and set lostDigits status, as needed
      if (lhs->digits > reqdigits) {
         alloclhs = decRoundOperand(lhs, set, &status);
         if (alloclhs==NULL) break;
         lhs=alloclhs;
      }
      if (rhs->digits > reqdigits) {
         allocrhs=decRoundOperand(rhs, set, &status);
         if (allocrhs==NULL) break;
         rhs=allocrhs;
      }

      // handle infinities and NaNs
      merged=(lhs->bits | rhs->bits) & DECSPECIAL;
      if (merged) {                        // a special bit set
         if (merged & DECSNAN) {           // sNaN always bad
            status |= DEC_Invalid_operation;
            break;
         }
         if (merged & DECNAN) {            // NaNs beget qNaN
            decNumberZero(res);
            res->bits=DECNAN;
            break;                         // (status unchanged)
         }
         // one or two infinities; * 0 is invalid
         if (
            ((lhs->bits & DECSPECIAL)==0 && ISZERO(lhs)) ||
            ((rhs->bits & DECSPECIAL)==0 && ISZERO(rhs))
         ) {
            status |= DEC_Invalid_operation;
            break;
         }
         decNumberZero(res);
         res->bits=bits|DECINF;            // set +/- infinity
         break;
      }

      // For best speed, as in DMSRCN, we use the shorter number as the
      // multiplier (rhs) and the longer as the multiplicand (lhs)
      if (lhs->digits < rhs->digits) { // swap...
         DecNumber const * hold = lhs;
         lhs = rhs;
         rhs = hold;
      }

      // if accumulator is too long for local storage, then allocate
      // [need space for reqdigits*2]
      if (D2U(reqdigits*2)*sizeof(DecNumberUnit) > sizeof(accbuff)) {
         allocacc=(DecNumberUnit *)malloc(D2U(reqdigits*2)*sizeof(DecNumberUnit));
         if (allocacc==0) {
            status |= DEC_Insufficient_storage;
            break;
         }
         acc=allocacc;                // use the allocated space
      }

      /* Now the main long multiplication loop */
      // Unlike the equivalent in the IBM Java implementation, there's
      // no advantage in calculating from msu to lsu.  So we do it by
      // the book, as it were.
      // Each iteration calculates ACC=ACC+MULTAND*MULT
      accunits = 1;                    // accumulator starts at '0'
      *acc = 0;                        // .. (lsu=0)
      shift = 0;                       // no multiplicand shift at first
      madlength = D2U(lhs->digits);    // we know this won't change
      mermsup = rhs->lsu + D2U(rhs->digits); // -> msu+1 of multiplier

      for (mer = rhs->lsu; mer < mermsup; ++mer) {
         // Here, *mer is the next Unit in the multiplier to use
         // If non-zero [optimization] add it...
         if (*mer != 0) {
            accunits = decUnitAddSub(
               &acc[shift], accunits-shift,
               lhs->lsu, madlength, 0,
               &acc[shift],
               *mer
            ) + shift;
         }else { // extend acc with a 0; we'll use it soon
          // [this avoids length of <=0 later]
            *(acc+accunits)=0;
            ++accunits;
         }
         // multiply multiplicand by 10**DECDPUN for next Unit to left
         ++shift;                     // add this for 'logical length'
      }

      // acc now contains the exact result of the multiplication
      // Build a DecNumber from it, noting if any residue

      // We must be careful to avoid 31-bit wrap in calculating the
      // exponent, which can happen only if both input exponents are
      // negative and both their magnitudes are larger than reqdigits
      // (and set->emax is large).  If the wrap does occur, we set a safe
      // very negative exponent, which finish() will raise underflow from.
      exponent=lhs->exponent+rhs->exponent;         // tentative exponent
      if ((lhs->exponent < -reqdigits) && (rhs->exponent < -reqdigits)) {
         // calculate the unsigned magnitude of the adjusted final exponent
         uint32_t lexp = -lhs->exponent;
         uint32_t rexp = -rhs->exponent;
         // reqdigits-1 could be > lexp+rexp
         if ((lexp+rexp) > (set->emax + reqdigits - 1)) {
            // reprieve for subnormals if extended
            if (
               !(set->flags & DEC_EXTENDED) ||
               (lexp+rexp) > (set->emax + set->digits-1)
            ) {
               exponent = -2*DEC_MAX_EMAX;          // bound to underflow
            }
         }
      }
      res->bits=bits;                               // set sign
      res->exponent=exponent;                       // set exponent
      res->digits=decGetDigits(acc, accunits);      // count digits exactly

      // Now the coefficient.  If it's too long round it into res.
      // Otherwise (it fits) simply copy to result.
      if (res->digits>reqdigits) {
         status |= decRound(res, set, acc, res->digits, 0);
      }else {                        // it fits
         DecNumberUnit *d=res->lsu;  // destination pointer
         DecNumberUnit *s=acc;       // source pointer
         for (; s<acc+accunits; ++s, ++d) *d=*s;
      }
      status |= decFinish(res, set); // final cleanup
   }while(0);                      // end protected

   if (allocacc!=NULL) free(allocacc);   // drop any storage we used
   if (allocrhs!=NULL) free(allocrhs);   // ..
   if (alloclhs!=NULL) free(alloclhs);   // ..
   return decStatus(res, status);
}

/*-------------------------------------------------------------decNumberPower-+
|   decNumberPower -- raise a number to an integer power                      |
|                                                                             |
|     This computes C = A ** B                                                |
|                                                                             |
|     res is C, the result.  C may be A and/or B (e.g., X=X**X)               |
|     lhs is A                                                                |
|     rhs is B                                                                |
|     set is the context                                                      |
|                                                                             |
|   C must have space for set->digits digits.                                 |
|                                                                             |
|   Specification restriction: abs(n) must be <=999999999                     |
+----------------------------------------------------------------------------*/
uint32_t decNumberPower(
   DecNumber * res,
   DecNumber const * lhs,
   DecNumber const * rhs,
   DecContext const *set
) {
   DecNumber * alloclhs = 0;        // non-NULL if rounded lhs allocated
   DecNumber * allocrhs = 0;        // .., rhs
   DecNumber * allocdac = 0;        // -> allocated buffer, iff used
   DecNumber const * inrhs=rhs;     // save original rhs
   int32_t reqdigits = set->digits; // requested DIGITS
   int32_t n;                       // RHS in binary
   uint32_t k;
   uint32_t needbytes;              // buffer size needed
   uint32_t status=0;               // accumulator
   uint8_t bits = 0;                // result sign if errors
   DecContext workset;              // working context
   // local accumulator buffer [a DecNumber, with digits+elength+1 digits]
   uint8_t dacbuff[sizeof(DecNumber)+D2U(DECBUFFER+9)*sizeof(DecNumberUnit)];
   // same again for possible 1/lhs calculation
#if MIKE
   uint8_t lhsbuff[sizeof(DecNumber)+D2U(DECBUFFER+9)*sizeof(DecNumberUnit)];
#endif
   DecNumber * dac = (DecNumber *)dacbuff;  // -> result accumulator

   // reduce operands and set lostDigits status, as needed
   if (lhs->digits > reqdigits) {
      alloclhs = decRoundOperand(lhs, set, &status);
      if (!alloclhs) goto leave;
      lhs = alloclhs;
   }
   // rounding won't affect the result, but we might signal lostDigits
   // as well as the error for non-integer [x**y would need this too]
   if (rhs->digits > reqdigits) {
      allocrhs = decRoundOperand(rhs, set, &status);
      if (!allocrhs) goto leave;
      rhs = allocrhs;
   }

   // handle RHS infinities and NaNs
   if (rhs->bits & DECSPECIAL) {
      if (
         (rhs->bits & DECNAN) &&        // NaN begets qNaN
         !(lhs->bits & DECSNAN)         // (unless LHS is sNaN)
      ) {
         decNumberZero(res);
         res->bits=DECNAN;              // (status unchanged)
      }else {
         status |= DEC_Invalid_operation;
      }
      goto leave;
   }

   // Original rhs must be an integer that fits and is in range
   n = decGetInt(inrhs, set);
   if ((n == DEC_BADINT) || (n > 999999999) || (n < -999999999)) {
      status |= DEC_Invalid_operation;
      goto leave;
   }
   if (n < 0) {                         // negative
      n = -n;                           // use the absolute value
   }
   if ((lhs->bits & DECNEG) && (n & 0x1)) {  // -x ** an odd power...
      bits = DECNEG;
   }

   // handle LHS infinities and NaNs
   if (lhs->bits & DECSPECIAL) {        // a special bit set
      if (lhs->bits & DECSNAN) {        // sNaN always bad
         status |= DEC_Invalid_operation;
      }else if (lhs->bits & DECNAN) {   // NaN begets qNaN
         decNumberZero(res);
         res->bits = DECNAN;            // (status unchanged)
      }else {                           // LHS is infinite
         if (!(rhs->bits & DECNEG)) {   // was not a **-n
            bits |= DECINF;             // (otherwise 0 or -0)
         }
         decNumberZero(res);
         if (n == 0) {                  // [-]Inf**0 => 1
            *res->lsu = 1;              // (ignore bits)
         }else {
            res->bits = bits;           // wasn't **0
         }
      }
      goto leave;
   }

   if (n == 0) {
      // 0**0 is bad if extended
      if ((set->flags & DEC_EXTENDED) && ISZERO(lhs)) {
         status |= DEC_Invalid_operation;
      }else {
         decNumberZero(res);
         *res->lsu = 1;                 // x**0 == 1
      }
      goto leave;
   }

   // clone the context
   workset = *set;                      // copy all fields
   // calculate the working DIGITS
   workset.digits = reqdigits + (inrhs->digits+inrhs->exponent) + 1;
   // it's an error if this is more than we can handle
   if (workset.digits > DEC_MAX_DIGITS) {
      status |= DEC_Invalid_operation;
      goto leave;
   }

   // workset.digits is the count of digits for the accumulator we need
   // if accumulator is too long for local storage, then allocate
   needbytes = sizeof(DecNumber)+(D2U(workset.digits)-1)*sizeof(DecNumberUnit);
   // [needbytes also used below if 1/lhs needed]
   if (needbytes > sizeof dacbuff) {
      allocdac = (DecNumber *)malloc(needbytes);
      if (!allocdac) {                  // hopeless -- abandon
         status |= DEC_Insufficient_storage;
         goto leave;
      }
      dac = allocdac;                   // use the allocated space
   }

#if MIKE
   // if a negative power and if extended we'll invert the lhs now
   // rather than inverting the result later
   if (
      (rhs->bits & DECNEG) &&           // was a **-n [hence digits>0]
      (set->flags & DEC_EXTENDED)       // need to calculate 1/lhs
   ) {
      DecNumber dnOne;                  // set up a one...
      decNumberZero(&dnOne);
      *dnOne.lsu = 1;                   // dnOne=1;

      // divide lhs into 1, putting result in dac [dac=1/dac]
      status |= decDivideOp(dac, &dnOne, lhs, &workset, DEC_OP_DIVIDE);

      // locate or allocate space for the inverted lhs (old lhs saved in dac)
      if (alloclhs) {
         free(alloclhs);                // done with intermediate
         alloclhs = 0;                  // indicate freed
      }
      if (needbytes > sizeof lhsbuff) {
         alloclhs = (DecNumber *)malloc(needbytes);
         if (!alloclhs) {               // hopeless -- abandon
            status |= DEC_Insufficient_storage;
            goto leave;
         }
         lhs = alloclhs;                // use the allocated space
      }else {
         lhs = (DecNumber *)lhsbuff;    // use stack storage
      }
      // [lhs now points to buffer or allocated storage]
      decNumberCopy((DecNumber *)lhs, dac);  // set the lhs copy (constness OK)
   }else {
      decNumberCopy(dac, lhs);          // init the acc
   }
#else
   decNumberCopy(dac, lhs);             // init the acc
#endif

   for (k = (1 << 30); (k & n)==0; k >>= 1) {}  // search the first bit
   while (k > 1) {                              // Raise-to-the-power loop...
      // dac=dac*dac [square]
      status |= decNumberMultiply(dac, dac, dac, &workset);
      if ((k >>= 1) & n) {                      // dac=dac*x
         status |= decNumberMultiply(dac, dac, lhs, &workset);
      }
      if (status) {
         if (status & DEC_Insufficient_storage) {
            goto leave;
         }
         // abandon if we have had overflow or hard underflow
         if (
            (status & DEC_Overflow) ||
            ((status & DEC_Underflow) && ISZERO(dac))
         ) {
#if MIKE
            // If subset, and power was negative, reverse the kind of -erflow
            // [1/x not yet done]
            if (!(set->flags & DEC_EXTENDED) && (rhs->bits & DECNEG)) {
               status ^= (DEC_Overflow | DEC_Underflow);
            }
#else
            // If power was negative, reverse the kind of -erflow
            if (rhs->bits & DECNEG) {
               status ^= (DEC_Overflow | DEC_Underflow);
            }
#endif
            if (status & DEC_Underflow) {
               decNumberZero(res);
               res->bits = bits;
               status |= decSetUnderflow(res, set);
            }else {
               status |= decSetOverflow(res, set, bits);
            }
            goto leave;
         }
      }
   }

   // [soft underflow (subnormal result) drops through

#if MIKE
   if (
      (rhs->bits & DECNEG) &&           // was a **-n [hence digits>0]
      !(set->flags & DEC_EXTENDED)      // need to calculate 1/dac
   ) {
#else
   // preserve any informational flags.
   status &= DEC_Information;
   if (rhs->bits & DECNEG) {            // was a **-n [hence digits>0]
#endif
      // so divide it into 1
      DecNumber dnOne;            // set up a one...
      decNumberZero(&dnOne);
      *dnOne.lsu = 1;
      status |= decDivideOp(dac, &dnOne, dac, &workset, DEC_OP_DIVIDE);
   }

   // reduce result to the requested length
   if (dac->digits > reqdigits) {
      status |= decRound(dac, set, dac->lsu, dac->digits, 0);
   }

   decNumberCopy(res, dac);       // copy result where we want it
   decTrim(res, 0);               // strip trailing zeros
   status |= decFinish(res, set); // final cleanup

leave:
   if (allocdac) free(allocdac);  // drop any storage we used
   if (allocrhs) free(allocrhs);  // ..
   if (alloclhs) free(alloclhs);  // ..
   return decStatus(res, status);
}

/*-----------------------------------------------------------decNumberRescale-+
|   decNumberRescale -- force exponent to requested value                     |
|                                                                             |
|     This computes C = op(A, B), where op adjusts the coefficient            |
|     of C (by rounding or shifting) such that the exponent (-scale)          |
|     of C has the value B.  The numerical value of C will equal A,           |
|     except for the effects of any rounding that occurred.                   |
|                                                                             |
|     res is C, the result.  C may be A or B                                  |
|     lhs is A, the number to adjust                                          |
|     rhs is B, the requested exponent                                        |
|     set is the context                                                      |
|                                                                             |
|   C must have space for set->digits digits.                                 |
|                                                                             |
|   Unless there's an error, the exponent after the operation is              |
|   guaranteed to be equal to B, except if the result is 0 (in which          |
|   case the exponent will be 0).                                             |
+----------------------------------------------------------------------------*/
uint32_t decNumberRescale(
   DecNumber * res,
   DecNumber const * lhs,
   DecNumber const * rhs,
   DecContext const *set
) {
   DecNumber * alloclhs = 0;        // non-NULL if rounded lhs allocated
   DecNumber * allocrhs = 0;        // .., rhs
   DecNumber const * inrhs = rhs;   // save original rhs
   int32_t reqdigits = set->digits; // requested DIGITS
   int32_t reqexp;                  // requested exponent [-scale]
   uint32_t  status=0;              // accumulator
   uint8_t bits;                    // result sign if errors
   uint8_t merged;                  // merged flags
   int32_t adjust;                  // work

   do {                             // protect allocated storage
      // reduce operands and set lostDigits status, as needed
      if (lhs->digits>reqdigits) {
         alloclhs=decRoundOperand(lhs, set, &status);
         if (alloclhs==NULL) break;
         lhs=alloclhs;
      }
      if (rhs->digits>reqdigits) {   // [this only checks lostDigits]
         allocrhs=decRoundOperand(rhs, set, &status);
         if (allocrhs==NULL) break;
         rhs=allocrhs;
      }

      merged=(lhs->bits | rhs->bits) & DECSPECIAL;
      if (merged) {
         // neither sNaN nor Infinity are good
         if (merged & (DECSNAN|DECINF)) {
            status |= DEC_Invalid_operation;
            break;
         }
         // one or other must be a qNaN
         decNumberZero(res);                   // beget qNaN
         res->bits=DECNAN;                     // ..
         break;                                // (status unchanged)
      }

      // Original rhs must be an integer that fits and is in range
      reqexp=decGetInt(inrhs, set);
      if (reqexp==DEC_BADINT) {
         status |= DEC_Invalid_operation;
         break;
      }

      // we've processed the RHS, so we can overwrite it now if necessary
      if (res!=lhs) decNumberCopy(res, lhs);   // clone to result
      if (!ISZERO(res)) {                      // [zeros pass through]
         DecContext workset = *set;            // clone rounding, etc.

         bits=lhs->bits;
         adjust = reqexp-res->exponent;
         if (adjust>0) {                       // increasing exponent
            // this will decrease the length of the coefficient by adjust
            // digits, and must round as it does so
            workset.digits = res->digits-adjust;  // set requested length
            // [note that the latter can be <1, here]
            status |= decRound(res, &workset, res->lsu, res->digits, 0);

            // if we rounded a 999s case, exponent will be off by one; adjust
            // back if so.
            if (res->exponent>reqexp) {
               res->digits=decShiftToMost(res->lsu, res->digits, 1); // shift
               --res->exponent;                // (re)adjust the exponent.
            }

         }else if (adjust<0) {                 // decreasing exponent
            // this will increase the length of the coefficient by adjust
            // digits, by adding trailing zeros.  It must fit in set->digits
            if ((res->digits-adjust) > reqdigits) {
               status |= decSetOverflow(res, set, bits);
               break;
            }
            res->digits=decShiftToMost(res->lsu, res->digits, -adjust);
            res->exponent += adjust;           // adjust the exponent.
         }

         // [drop through if adjust==0]

         // in some rare round-to-zero cases we need to clean up
         if (*res->lsu == 0) {
            res->digits=decGetDigits(res->lsu, D2U(res->digits));
         }
         res->bits=bits;                       // result sign <= lhs sign
         // prevent decFinish from integer preservation by giving it a
         // context with digits=1.  We need to call it for clean 0 and also
         // for exponent checking.
         workset.digits=1;                     // (don't touch the real set)
         status |= decFinish(res, &workset);
      }
   }while(0);

   if (allocrhs!=NULL) free(allocrhs);         // drop any storage we used
   if (alloclhs!=NULL) free(alloclhs);         // ..
   return decStatus(res, status);
}

/*---------------------------------------------------------decNumberRemainder-+
|   decNumberRemainder -- divide and return remainder                         |
|                                                                             |
|     This computes C = A % B                                                 |
|                                                                             |
|     res is C, the result.  C may be A and/or B (e.g., X=X%X)                |
|     lhs is A                                                                |
|     rhs is B                                                                |
|     set is the context                                                      |
|                                                                             |
|   C must have space for set->digits digits.                                 |
+----------------------------------------------------------------------------*/
uint32_t decNumberRemainder(
   DecNumber * res,
   DecNumber const * lhs,
   DecNumber const * rhs,
   DecContext const *set
) {
   return decDivideOp(res, lhs, rhs, set, DEC_OP_REMAINDER);
}

/*-----------------------------------------------------decNumberRemainderNear-+
|   decNumberRemainderNear -- divide and return remainder from nearest        |
|                                                                             |
|     This computes C = A % B, where % is the IEEE remainder operator         |
|                                                                             |
|     res is C, the result.  C may be A and/or B (e.g., X=X%X)                |
|     lhs is A                                                                |
|     rhs is B                                                                |
|     set is the context                                                      |
|                                                                             |
|   C must have space for set->digits digits.                                 |
+----------------------------------------------------------------------------*/
uint32_t decNumberRemainderNear(
   DecNumber * res,
   DecNumber const * lhs,
   DecNumber const * rhs,
   DecContext const *set
) {
   return decDivideOp(res, lhs, rhs, set, DEC_OP_REMNEAR);
}

/*--------------------------------------------------------decNumberSquareRoot-+
|   decNumberSquareRoot -- square root operator                               |
|                                                                             |
|     This computes C = squareroot(A)                                         |
|                                                                             |
|     res is C, the result.  C may be A                                       |
|     rhs is A                                                                |
|     set is the context; note that rounding mode has no effect               |
|                                                                             |
|   C must have space for set->digits digits.                                 |
|   ------------------------------------------------------------------        |
|   This uses the following varying-precision algorithm in:                   |
|                                                                             |
|     Properly Rounded Variable Precision Square Root, T. E. Hull and         |
|     A. Abrham, ACM Transactions on Mathematical Software, Vol 11 #3,        |
|     pp229-237, ACM, September 1985.                                         |
|                                                                             |
|   % [Reformatted original Numerical Turing source code follows.]            |
|   function sqrt(x : real) : real                                            |
|   % sqrt(x) returns the properly rounded approximation to the square        |
|   % root of x, in the precision of the calling environment, or it           |
|   % fails if x < 0.                                                         |
|   % t e hull and a abrham, august, 1984                                     |
|   if x <= 0 then                                                            |
|     if x < 0 then                                                           |
|       assert false                                                          |
|     else                                                                    |
|       result 0                                                              |
|     end if                                                                  |
|   end if                                                                    |
|   var f := setexp(x, 0)  % fraction part of x   [0.1 <= x < 1]              |
|   var e := getexp(x)     % exponent part of x                               |
|   var approx : real                                                         |
|   if e mod 2 = 0  then                                                      |
|     approx := .259 + .819 * f   % approx to root of f                       |
|   else                                                                      |
|     f := f/l0                   % adjustments                               |
|     e := e + 1                  %   for odd                                 |
|     approx := .0819 + 2.59 * f  %   exponent                                |
|   end if                                                                    |
|                                                                             |
|   var p:= 3                                                                 |
|   const maxp := currentprecision + 2                                        |
|   loop                                                                      |
|     p := min(2*p - 2, maxp)     % p = 4,6,10, . . . , maxp                  |
|     precision p                                                             |
|     approx := .5 * (approx + f/approx)                                      |
|     exit when p = maxp                                                      |
|   end loop                                                                  |
|                                                                             |
|   % approx is now within 1 ulp of the properly rounded square root          |
|   % of f; to ensure proper rounding, compare squares of (approx -           |
|   % l/2 ulp) and (approx + l/2 ulp) with f.                                 |
|   p := currentprecision                                                     |
|   begin                                                                     |
|     precision p + 2                                                         |
|     const approxsubhalf := approx - setexp(.5, -p)                          |
|     if mulru(approxsubhalf, approxsubhalf) > f then                         |
|       approx := approx - setexp(.l, -p + 1)                                 |
|     else                                                                    |
|       const approxaddhalf := approx + setexp(.5, -p)                        |
|       if mulrd(approxaddhalf, approxaddhalf) < f then                       |
|         approx := approx + setexp(.l, -p + 1)                               |
|       end if                                                                |
|     end if                                                                  |
|   end                                                                       |
|   result setexp(approx, e div 2)  % fix exponent                            |
|   end sqrt                                                                  |
+----------------------------------------------------------------------------*/
uint32_t decNumberSquareRoot(
   DecNumber * res,
   DecNumber const * rhs,
   DecContext const * set
) {
   DecContext workset;                  // work
   DecContext approxset;                // work
   int32_t reqdigits = set->digits;     // requested digits
   uint32_t maxp = reqdigits + 2;       // largest working precision
   uint32_t status=0;                   // status accumulators
   int32_t exp;                         // saved rhs exponent
   int32_t needbytes;                   // work
   DecNumber *allocrhs=0;               // non-0 if rounded rhs allocated
   // buffer for f [needs +1 in case DECBUFFER 0]
   uint8_t buff[sizeof(DecNumber)+(D2U(DECBUFFER+1)-1)*sizeof(DecNumberUnit)];
   // buffer for a [needs +2 to match maxp]
   uint8_t bufa[sizeof(DecNumber)+(D2U(DECBUFFER+2)-1)*sizeof(DecNumberUnit)];
   // buffer for temporary, b [must be same size as a]
   uint8_t bufb[sizeof(DecNumber)+(D2U(DECBUFFER+2)-1)*sizeof(DecNumberUnit)];
   DecNumber * allocbuff=0;             // -> allocated buff, iff allocated
   DecNumber * allocbufa=0;             // -> allocated bufa, iff allocated
   DecNumber * allocbufb=0;             // -> allocated bufb, iff allocated
   DecNumber * f=(DecNumber *)buff;     // reduced fraction
   DecNumber * a=(DecNumber *)bufa;     // approximation to result
   DecNumber * b=(DecNumber *)bufb;     // intermediate result
   // buffer for temporary variable, up to 3 digits
   uint8_t buft[sizeof(DecNumber)+(D2U(3)-1) * sizeof(DecNumberUnit)];
   DecNumber *t = (DecNumber *)buft;    // up-to-3-digit constant

   do {                                 // protect allocated storage
      // reduce operand and set lostDigits status, as needed
      if (rhs->digits > reqdigits) {
         allocrhs = decRoundOperand(rhs, set, &status);
         if (!allocrhs) break;
         // [Note: 'f' allocation below could reuse this buffer if
         // used, but as this is rare we keep them separate for clarity.]
         rhs = allocrhs;
      }

      // [following code does not require input rounding]

      // handle RHS infinities and NaNs
      if (rhs->bits & DECSPECIAL) {
         if (
            (rhs->bits & DECSNAN) ||    // sNaN and -Infinity signal
            ((rhs->bits & DECNEG) && (rhs->bits & DECINF))
         ) {
            status |= DEC_Invalid_operation;
            break;
         }
         // must be qNaN or +Infinity; result is rhs
         decNumberCopy(res, rhs);
         break;
      }

      // handle zeros
      if (ISZERO(rhs)) {
         decNumberCopy(res, rhs);       // could be 0 or -0
         res->exponent = 0;             // normalize
         break;
      }

      // any other -x is an oops
      if (rhs->bits & DECNEG) {
         status |= DEC_Invalid_operation;
         break;
      }

      // we need space for three working variables
      //   f -- the same precision as the RHS, reduced to 0.01->0.99...
      //   a -- Hull's approx -- precision, when assigned, is
      //        currentprecision (we allow +2 for use as temporary)
      //   b -- intermediate temporary result
      // if any is too long for local storage, then allocate
      needbytes = sizeof(DecNumber) + (D2U(rhs->digits)-1) * sizeof(DecNumberUnit);
      if (needbytes > sizeof buff) {
         allocbuff = (DecNumber *)malloc(needbytes);
         if (!allocbuff) {              // hopeless -- abandon
            status |= DEC_Insufficient_storage;
            break;
         }
         f = allocbuff;                 // use the allocated space
      }
      // a and b both need to be able to hold a maxp-length number
      needbytes = sizeof(DecNumber) + (D2U(maxp)-1) * sizeof(DecNumberUnit);
      if (needbytes > sizeof bufa) {    // [same applies to b]
         allocbufa = (DecNumber *)malloc(needbytes);
         allocbufb = (DecNumber *)malloc(needbytes);
         if (!allocbufa || !allocbufb) {  // hopeless
            status |= DEC_Insufficient_storage;
            break;
         }
         a = allocbufa;                 // use the allocated space
         b = allocbufb;                 // ..
      }

      // copy rhs -> f, save exponent, and reduce so 0.1 <= f < 1
      decNumberCopy(f, rhs);
      exp = f->exponent + f->digits;    // adjusted to Hull rules
      f->exponent = -(f->digits);       // to range

      // set up working contexts (the second is used for Numerical
      // Turing assignment)
      workset.digits = reqdigits;
      workset.emax = 999;
      workset.round = DEC_ROUND_HALF_EVEN; // 0.5 to nearest even
      workset.flags = DEC_EXTENDED;
      approxset = workset;

      // [Until further notice, no error is possible and status bits
      // (Rounded, etc.) should be ignored, not accumulated.]

      // Calculate initial approximation, and allow for odd exponent
      t->bits = 0;
      t->digits = 3;
      a->bits = 0;
      a->digits = 3;
      if ((exp & 1) == 0) {             // even exponent
         t->exponent = -3;
         a->exponent = -3;
         #if DECDPUN >= 3
            t->lsu[0] = 259;
            a->lsu[0] = 819;
         #elif DECDPUN==2
            t->lsu[0]=59; t->lsu[1]=2;
            a->lsu[0]=19; a->lsu[1]=8;
         #else
            t->lsu[0]=9; t->lsu[1]=5; t->lsu[2]=2;
            a->lsu[0]=9; a->lsu[1]=1; a->lsu[2]=8;
         #endif
      }else {                           // odd exponent
         --f->exponent;                 // f=f/10
         ++exp;                         // e=e+1;
         t->exponent = -4;
         a->exponent = -2;
         #if DECDPUN >= 3
            t->lsu[0] = 819;
            a->lsu[0] = 259;
         #elif DECDPUN==2
            y->lsu[0]=19; t->lsu[1]=8;
            a->lsu[0]=59; a->lsu[1]=2;
         #else
            t->lsu[0]=9; t->lsu[1]=1; t->lsu[2]=8;
            a->lsu[0]=9; a->lsu[1]=5; a->lsu[2]=2;
         #endif
      }
      if (workset.digits < 3) {         // rounding needed
         decRound(t, &workset, t->lsu, 3, 0);
         decRound(a, &workset, t->lsu, 3, 0);
      }
      decNumberMultiply(a, a, f, &workset);         // a=a*f
      decAddOp(a, a, t, &workset, 0);               // ..+t
      // [a is now the initial approximation for sqrt(f), calculated with
      // currentprecision, which is also a's precision.]

      // the main calculation loop
      decNumberZero(t);                             // set t = 0.5
      t->lsu[0] = 5;                                // ..
      t->exponent = -1;                             // ..
      workset.digits = 3;                           // initial p
      do {
         // set p to min(2*p - 2, maxp)  [hence 3; or: 4, 6, 10, ... , maxp]
         workset.digits = (workset.digits * 2) - 2;
         if (workset.digits > maxp) workset.digits = maxp;
         // a = 0.5 * (a + f/a)
         // [calculated at p then rounded to currentprecision]
         decDivideOp(b, f, a, &workset, DEC_OP_DIVIDE);  // b=f/a
         decAddOp(b, b, a, &workset, 0);            // b=b+a
         decNumberMultiply(a, b, t, &workset);      // a=b*0.5
         // assign to approx [round to length]
         decRound(a, &approxset, a->lsu, a->digits, 0);
      }while (workset.digits != maxp);              // just did final

      // a is now at currentprecision and within 1 ulp of the properly
      // rounded square root of f; to ensure proper rounding, compare
      // squares of (a - l/2 ulp) and (a + l/2 ulp) with f.
      // Here workset.digits=maxp and t=0.5
      --workset.digits;                             // maxp-1 is OK now
      t->exponent = -reqdigits-1;                   // make 0.5 ulp
      decNumberCopy(b, a);
      decAddOp(b, b, t, &workset, DECNEG);          // b = a - 0.5 ulp
      workset.round = DEC_ROUND_UP;
      decNumberMultiply(b, b, b, &workset);         // b = mulru(b, b)
      decCompareOp(b, f, b, &workset, COMPARE);     // b ? f, reversed
      if (b->bits & DECNEG) {                       // f < b [i.e., b > f]
         // this is the more common adjustment, though both are rare
         ++t->exponent;                             // make 1.0 ulp
         t->lsu[0] = 1;                             // ..
         decAddOp(a, a, t, &workset, DECNEG);       // a = a - 1 ulp
         // assign to approx [round to length]
         decRound(a, &approxset, a->lsu, a->digits, 0);
      }else {
         decNumberCopy(b, a);
         decAddOp(b, b, t, &workset, 0);            // b = a + 0.5 ulp
         workset.round=DEC_ROUND_DOWN;
         decNumberMultiply(b, b, b, &workset);      // b = mulrd(b, b)
         decCompareOp(b, b, f, &workset, COMPARE);  // b ? f
         if (b->bits & DECNEG) {                    // b < f
            ++t->exponent;                          // make 1.0 ulp
            t->lsu[0] = 1;                          // ..
            decAddOp(a, a, t, &workset, 0);         // a = a + 1 ulp
            // assign to approx [round to length]
            decRound(a, &approxset, a->lsu, a->digits, 0);
         }
      }
      // [no errors are possible in the above, and rounding/inexact during
      // estimation are irrelevant, so status was not accumulated]

      // Here, 0.1 <= a < 1  [Hull]
      a->exponent += exp/2;                         // set correct exponent

      // Check for Subnormals.  These are only possible when emax < digits,
      // so we avoid the decFinish call by explicit test here
      if ((a->exponent+a->digits) <= -(int32_t)set->emax) { // [test as in decFinish]
         status = decSetUnderflow(a, set);
      }

      // normalize [after any subnormal rounding]
      decTrim(a, 1);                                // [drops trailing zeros]

      // Finally set Inexact and Rounded.  The answer can only be exact if
      // it is short enough so that squaring it could fit in set->digits,
      // so this is the only (rare) time we have to check carefully
      if ((a->digits * 2)-1 > reqdigits) {          // cannot fit
         status |= DEC_Inexact | DEC_Rounded;
      }else {                                       // could be exact/unrounded
         if (decNumberMultiply(b, a, a, &workset)) {// result won't fit
            status |= DEC_Inexact | DEC_Rounded;
         }else {                                    // plausible
            decCompareOp(t, b, rhs, &workset, COMPARE); // b ? rhs
            if (!ISZERO(t)) {
               status |= DEC_Inexact | DEC_Rounded;
            }else {                                 // is Exact
               if (b->digits != rhs->digits) {      // .. but rounded
                  status |= DEC_Rounded;
               }
            }
         }
      }
      decNumberCopy(res, a);               // now safe to set the result
   } while(0);                             // end protected

   if (allocbuff!=0) free(allocbuff);      // drop any storage we used
   if (allocbufa!=0) free(allocbufa);      // ..
   if (allocbufb!=0) free(allocbufb);      // ..
   if (allocrhs !=0) free(allocrhs);       // ..
   return decStatus(res, status);          // then report status
}


/*----------------------------------------------------------decNumberSubtract-+
|   decNumberSubtract -- subtract two Numbers                                 |
|                                                                             |
|     This computes C = A - B                                                 |
|                                                                             |
|     res is C, the result.  C may be A and/or B (e.g., X=X-X)                |
|     lhs is A                                                                |
|     rhs is B                                                                |
|     set is the context                                                      |
|                                                                             |
|   C must have space for set->digits digits.                                 |
+----------------------------------------------------------------------------*/
uint32_t decNumberSubtract(
   DecNumber * res,
   DecNumber const * lhs,
   DecNumber const * rhs,
   DecContext const *set
) {
   return decAddOp(res, lhs, rhs, set, DECNEG);
}

/*---------------------------------------------------------decNumberToInteger-+
|   decNumberToInteger -- round-to-integer                                    |
|                                                                             |
|     res is the result                                                       |
|     rhs is input number                                                     |
|     set is the context                                                      |
|                                                                             |
|   res must have space for any value of rhs.                                 |
|                                                                             |
|   This returns plus(rhs), unless there is a fractional part, in             |
|   which case rescale(rhs,0) is returned.  In either case the correct        |
|   input processing is applied.  If extended values is 1 then the            |
|   sign of a zero operand is preserved as IEEE 854 requires (i.e.,           |
|   in this case plus(rhs) is not used).                                      |
+----------------------------------------------------------------------------*/
uint32_t decNumberToInteger(
   DecNumber * res,
   DecNumber const * rhs,
   DecContext const *set
) {
   if (rhs->exponent<0) {          // [it will be 0 for specials and 0]
      DecNumber dn;
      // fractional part to remove
      decNumberZero(&dn);
      return decNumberRescale(res, rhs, &dn, set);
   }
   if (ISZERO(rhs)) {                     // preserve sign of 0?
      if ((set->flags & DEC_EXTENDED)) {
         decNumberCopy(res, rhs);         // clone
      }else {
         decNumberZero(res);              // else always +0
      }
      return 0;
   }else {
      DecNumber dzero;
      decNumberZero(&dzero);      // make 0
      return decAddOp(res, &dzero, rhs, set, 0);
   }
}

/* ========================================================================= */
/* Utility routines                                                          */
/* ========================================================================= */

/*--------------------------------------------------------------decNumberCopy-+
|   decNumberCopy -- copy a number                                            |
|                                                                             |
|     dest is the target DecNumber                                            |
|     src  is the source DecNumber                                            |
|                                                                             |
|   (dest==src is allowed and is a no-op)                                     |
+----------------------------------------------------------------------------*/
void decNumberCopy(
   DecNumber * dest,
   DecNumber const * src
) {
   DecNumberUnit const * s;       // work
   DecNumberUnit const * smsbp;   // ..
   DecNumberUnit * d;             // ..
   if (dest==src) return;         // no copy required

   // We use explicit assignments here as structure assignment can copy
   // more than just the lsu (for small DECDPUN).  This would not affect
   // the value of the results, but would disturb test harness spill
   // checking.
   dest->bits = src->bits;
   dest->exponent = src->exponent;
   dest->digits = src->digits;
   *dest->lsu = *src->lsu;
   if (src->digits>DECDPUN) {                 // more Units to come
      // memcpy for the remaining Units would be safe as they cannot
      // overlap.  However, this explicit loop is faster in short cases.
      d = dest->lsu + 1;                         // -> first destination
      smsbp = src->lsu+D2U(src->digits);         // -> source msb+1
      for (s=src->lsu+1; s < smsbp; ++s, ++d) *d = *s;
   }
}

/*-----------------------------------------------------------decNumberVersion-+
|   decNumberVersion -- return the name and version of this module            |
+----------------------------------------------------------------------------*/
char * decNumberVersion() {
   return (char *)DECVERSION;
}

/*--------------------------------------------------------------decNumberZero-+
|   decNumberZero -- set a number to 0                                        |
|                                                                             |
|     dn is the number to set, with space for one digit                       |
|                                                                             |
|   Memset is not used as it is much slower in some environments.             |
+----------------------------------------------------------------------------*/
void decNumberZero(DecNumber * dn) {
   dn->bits=0;
   dn->exponent=0;
   dn->digits=1;
   dn->lsu[0]=0;
}

/*----------------------------------------------------------------decGetWhole-+
| For some obscure reason, decGetInt chokes if the number has ten digits      |
| (which is allrite for a 32 bits int)                                        |
| this is the only purpose of this routine...                                 |
| Note that NO CHECK is performed.  Be sure the DecNumber is an int32_t!      |
+----------------------------------------------------------------------------*/
int32_t decGetWhole(DecNumber const * dn)
{
   DecNumberUnit const * pu = dn->lsu;
   int32_t k = *pu++;
   uint32_t max = D2U(dn->digits);
   uint32_t i = 0;
   while (--max) {
      k += (*pu++) * (powers[DECDPUN << i++]);
   }
   if (dn->bits & DECNEG) k = -k;
   return k;
}

/* ========================================================================= */
/* Local routines                                                            */
/* ========================================================================= */

/*--------------------------------------------------------------------TODIGIT-+
|   TODIGIT -- macro to remove the leading digit from the unsigned            |
|   integer u at column cut (counting from the right, LSD=0) and place        |
|   it as an ASCII character into the character pointed to by c.  Note        |
|   that cut must be <= 9, and the maximum value for u is 2,000,000,000       |
|   (as is needed for negative exponents of subnormals).                      |
+----------------------------------------------------------------------------*/
#define TODIGIT(u, cut, c) {                   \
  uint32_t pow;                                \
  *(c) = '0';                                  \
  pow = powers[cut] * 2;                       \
  if ((u)>pow) {                               \
     pow *= 4;                                 \
     if ((u)>=pow) { (u) -= pow; *(c) += 8; }  \
     pow /= 2;                                 \
     if ((u)>=pow) { (u) -= pow; *(c) += 4; }  \
     pow /= 2;                                 \
  }                                            \
  if ((u) >= pow) { (u) -= pow; *(c)+=2; }     \
  pow /= 2;                                    \
  if ((u) >= pow) { (u) -= pow; *(c)+=1; }     \
}

/*----------------------------------------------------------------decToString-+
|   decToString -- lay out a number into a string                             |
|     dn      -- the number to lay out                                        |
|     string  -- where to lay out the number                                  |
|     eng     -- 1 if Engineering, 0 if Scientific                            |
|   str must be at least dn->digits+14 characters long                        |
|   No error is possible.                                                     |
|                                                                             |
|   Note that this routine can generate a -0 or 0.000.  These are             |
|   never generated by base specification to-number or arithmetic, but        |
|   signed zeros can occur when the extended flag is set (e.g., -1*0).        |
+----------------------------------------------------------------------------*/
static void decToString(DecNumber const * dn, char * string, uint8_t flag)
{
   int32_t exp=dn->exponent;   // local copy
   int32_t e;                  // E-part value
   int32_t pre;                // digits before the '.'
   int32_t cut;                // for counting digits in a Unit
   char *c=string;             // work [output pointer]
   DecNumberUnit const * pu=dn->lsu+D2U(dn->digits)-1;   // -> msu [input pointer]
   uint32_t u;                 // work

   if (dn->bits & DECNEG) {    // Negatives get a minus (even -0, NaNs,
      *c='-';                  // and Infinities)
      ++c;
   }
   if (dn->bits&DECSPECIAL) {  // Is a special value
      if (dn->bits&DECINF) strcpy(c, "Infinity");
       else {                   // a NaN
         if (dn->bits&DECSNAN) { // signalling NaN
            *c='s';
            ++c;
         }
         strcpy(c, "NaN");
      }
      return;
   }

   // calculate how many digits in msu, and hence first cut
   cut = dn->digits % DECDPUN;
   if (cut==0) cut = DECDPUN;        // msu is full
   --cut;                            // power of ten is digits-1

   if (
      (exp==0) &&                    // simple integer [common fastpath]
      (0 == (flag & DECTOSTR2_EXPFORM_FORCED))
   ) {
      for (;pu>=dn->lsu; pu--) {     // each Unit from msu
         u=*pu;                       // contains DECDPUN digits to lay out
         for (; cut>=0; ++c, --cut) TODIGIT(u, cut, c);
         cut=DECDPUN-1;               // next Unit has all digits
      }
      *c='\0';                       // terminate the string
      return;
   }

   /* non-0 exponent -- assume plain form */
   pre=dn->digits+exp;              // digits before '.'
   e=0;                             // no E
   if (                             // need exponential form?
       (((exp>0) || (pre<-5)) && (flag & DECTOSTR2_EXPFORM_ALLOWED)) ||
       (flag & DECTOSTR2_EXPFORM_FORCED)
   ) {
      e = exp+dn->digits-1;         // calculate E value
      if ((flag & DECTOSTR2_ENG) && (e!=0)) { // may need to adjust
         int32_t adj;               // adjustment
         // The C remainder operator is undefined for negative numbers, so
         // we must use positive remainder calculation here
         if (e<0) {
            adj = (-e)%3;
            if (adj!=0) adj = 3-adj;
         }
          else { // e>0
            adj = e%3;
         }
         e = e-adj;
         pre = adj+1;
      }else {
         pre=1;                   // scientific always 1 digit before '.'
      }
   }

   /* lay out the digits of the coefficient, adding 0s and . as needed */
   u=*pu;
   if (pre>0) {                        // xxx.xxx or xx00 (engineering) form
      for (; pre>0; --pre, ++c, --cut) {
         if (cut<0) {                  // need new Unit
            if (pu==dn->lsu) break;    // out of input digits (pre>digits)
            pu--;
            cut=DECDPUN-1;
            u=*pu;
         }
         TODIGIT(u, cut, c);
      }
      if (pu>dn->lsu || (pu==dn->lsu && cut>=0)) {  // more to come, after '.'
         *c='.';
         ++c;
         for (;; ++c, --cut) {
            if (cut<0) {                // need new Unit
               if (pu==dn->lsu) break;  // out of input digits
               pu--;
               cut=DECDPUN-1;
               u=*pu;
            }
            TODIGIT(u, cut, c);
         }                              // 0 padding (for engineering) needed
      }else {
         for (; pre>0; --pre, ++c) *c='0';
      }
   }else {                              // 0.xxx or 0.000xxx form
      *c='0';
      ++c;
      *c='.';
      ++c;
      for (; pre<0; ++pre, ++c) *c='0'; // add any 0's after '.'
      for (; ; ++c, --cut) {
         if (cut<0) {                   // need new Unit
            if (pu==dn->lsu) break;     // out of input digits
            pu--;
            cut=DECDPUN-1;
            u=*pu;
         }
         TODIGIT(u, cut, c);
      }
   }

   /*
   | Finally add the E-part, if needed.  It will never be 0, has a
   | base maximum and minimum of +999999999 through -999999999, but
   | could range down to -1999999998 for subnormal numbers
   */
   if (e != 0) {
      uint8_t had=0;            // 1=had non-zero
      *c = 'E';
      ++c;
      *c = '+';                 // assume positive
      ++c;
      u = e;                    // ..
      if (e<0) {
         *(c-1) = '-';          // oops, need -
         u = -e;                // uint32_t, please
      }
      // layout the exponent (_itoa is not ANSI C)
      for (cut=9; cut>=0; --cut) {
         TODIGIT(u, cut, c);
         if (*c=='0' && !had) continue;    // skip leading zeros
         had=1;                            // had non-0
         ++c;                              // step for next
      }
   }
   *c = '\0';          // terminate the string (all paths)
   return;
}

/*-------------------------------------------------------------------decAddOp-+
| decAddOp -- add/subtract operation                                          |
|                                                                             |
|   This computes C = A + B                                                   |
|                                                                             |
|   res is C, the result.  C may be A and/or B (e.g., X=X+X)                  |
|   lhs is A                                                                  |
|   rhs is B                                                                  |
|   set is the context                                                        |
|   negate is DECNEG if rhs should be negated, or 0 otherwise                 |
|                                                                             |
| C must have space for set->digits digits.                                   |
|                                                                             |
| If possible, we calculate the coefficient directly into C.                  |
| However, if:                                                                |
|   -- we need a digits+1 calculation because numbers are unaligned           |
|      and span more than set->digits digits                                  |
|   -- a carry to digits+1 digits looks possible                              |
|   -- C is the same as A or B, and the result would destructively            |
|      overlap the A or B coefficient                                         |
| then we must calculate into a temporary buffer.  In this latter             |
| case we use the local (stack) buffer if possible, and only if too           |
| long for that do we resort to malloc.                                       |
|                                                                             |
| Misalignment is handled as follows:                                         |
|   Apad: (AExp>BExp) Swap operands and proceed as for BExp>AExp.             |
|   BPad: Apply the padding by a combination of shifting (whole               |
|         units) and multiplication (part units).                             |
|                                                                             |
| Addition, especially x=x+1, is speed-critical, so we take pains             |
| to make returning as fast as possible, by flagging any allocation.          |
|                                                                             |
+----------------------------------------------------------------------------*/
uint32_t decAddOp(
   DecNumber * res,
   DecNumber const * lhs,
   DecNumber const * rhs,
   DecContext const * set,
   uint8_t negate
) {
   DecNumber *alloclhs = 0;         // non-NULL if rounded lhs allocated
   DecNumber *allocrhs = 0;         // .., rhs
   int32_t rhsshift;                // working shift (in Units)
   int32_t maxdigits;               // longest logical length
   int32_t mult;                    // multiplier
   int32_t residue;                 // residue for rounding
   uint8_t bits;                    // result bits
   uint8_t diffsign;                // non-0 if arguments have different sign
   DecNumberUnit * acc;             // accumulator for result
   DecNumberUnit accbuff[D2U(DECBUFFER+1)]; // local buffer
                                    // [+1 is for possible final carry digit]
   DecNumberUnit *allocacc =0;      // -> allocated acc buffer, iff allocated
   uint32_t  status=0;              // accumulator
   uint8_t alloced=0;               // set non-0 if any allocations
   int32_t reqdigits=set->digits;   // local copy; requested DIGITS
   uint8_t merged;                  // merged flags
   int32_t padding;                 // work

   do {                             // protect allocated storage
      // reduce operands and set lostDigits status, as needed
      if (lhs->digits>reqdigits) {
         alloclhs=decRoundOperand(lhs, set, &status);
         if (alloclhs==NULL) break;
         lhs=alloclhs;
         alloced=1;
      }
      if (rhs->digits>reqdigits) {
         allocrhs=decRoundOperand(rhs, set, &status);
         if (allocrhs==NULL) break;
         rhs=allocrhs;
         alloced=1;
      }

      // note whether signs differ
      diffsign = (uint8_t)((lhs->bits ^ rhs->bits ^ negate) & DECNEG);

      // handle infinities and NaNs
      merged=(lhs->bits | rhs->bits) & DECSPECIAL;
      if (merged) {                         // a special bit set
         if (merged & DECSNAN) {            // sNaN always bad
            status|=DEC_Invalid_operation;
            break;
         }
         if (merged & DECNAN) {             // NaNs beget qNaN
            decNumberZero(res);
            res->bits=DECNAN;
            break;                          // (status=0)
         }
         // one or two infinities
         if (lhs->bits & DECINF) {          // LHS is infinity
            // two infinities with different signs is invalid
            if ((rhs->bits & DECINF) && diffsign) {
               status|=DEC_Invalid_operation;
               break;
            }
            bits=lhs->bits & DECNEG;        // get sign from LHS
         }else  {
            bits=(rhs->bits^negate) & DECNEG;   // RHS must be Infinity
         }
         decNumberZero(res);
         res->bits=bits|DECINF;             // set +/- infinity
         break;
      }

      // Quick exit for add 0s; return the non-0 (already rounded)
      if (ISZERO(lhs)) {
         bits=lhs->bits;                    // save in case LHS==RES
         if (res!=rhs) decNumberCopy(res, rhs);
         res->bits^=negate;                 // flip if rhs was negated
         status |= decFinish(res, set);     // [exponent may need clearing]
         // If the result was zero, we have special IEEE 854 rules to apply
         if (ISZERO(res)) {
            // base mode always -> 0, 0-0 gives +0 unless rounding to
            // -infinity, and -0-0 gives -0
            if (diffsign) res->bits=DECNEG; // preserve 0 sign
            if (!(set->flags & DEC_EXTENDED)              // .. unless ..
             || (diffsign && set->round!=DEC_ROUND_FLOOR)) res->bits=0;
         }
         break;
      }
      if (ISZERO(rhs)) {
         bits=lhs->bits;                   // be clean
         if (res!=lhs) decNumberCopy(res, lhs);
         status |= decFinish(res, set);    // [cannot be 0]
         break;
      }
      // [both fastpath and mainpath code below assume these cases have
      // already been handled]

      // Fastpath cases where the numbers are aligned, the RHS is all in
      // one unit, and no carry, lengthening, or borrow is needed
      if ((rhs->digits <= DECDPUN) && (rhs->exponent == lhs->exponent)) {
         if (!diffsign) {                            // adding
            uint32_t partial = *lhs->lsu + *rhs->lsu;
            if (
               (
                  (lhs->digits >= DECDPUN) &&        // cannot lengthen
                  (partial <= DECDPUNMAX)            // no unit carry
               ) || (
                  (lhs->digits < DECDPUN) &&         // .. another chance
                  (partial < powers[lhs->digits])
               )
            ) {
               if (res!=lhs) decNumberCopy(res, lhs);  // not in place
               *res->lsu=(DecNumberUnit)partial; // [copy could have overwritten RHS]
               break;
            }
            // else drop out for careful add

         }else { // signs differ
            int32_t partial = *lhs->lsu - *rhs->lsu;
            if (partial > 0) {     // no borrow needed, and non-0 result
               if (res!=lhs) decNumberCopy(res, lhs);  // not in place
               *res->lsu = (DecNumberUnit)partial;
               // this could have reduced digits [but result>0]
               res->digits=decGetDigits(res->lsu, D2U(res->digits));
               break;
            }
            // else drop out for careful add
         }
      }

      /*
      | Now see how much we have to pad the lhs or rhs in order to align
      | the numbers.  If one number is much larger than the other (that
      | is, if in plain form there is a least one digit between the
      | lowest digit or one and the highest of the other) we need to pad
      | with up to DIGITS-1 trailing zeros, and then apply rounding (as
      | exotic rounding modes may be affected by the residue).
      */
      rhsshift=0;               // rhs shift to left (padding) in Units
      residue=0;                // assume no rounding residue
      bits=lhs->bits;           // assume sign is that of LHS
      mult=1;                   // likely multiplier

      padding=rhs->exponent-lhs->exponent;
      // if padding==0 the operands are aligned; no padding needed
      if (padding!=0) {
         // some padding needed
         // We always pad the RHS, as we can then effect any required
         // padding by a combination of shifts and a multiply
         uint8_t swapped=0;
         if (padding < 0) {            // LHS needs the padding
            DecNumber const * t;
            padding=-padding;          // will be +ve
            bits = (uint8_t)(rhs->bits^negate); // assume sign is now that of RHS
            t=lhs; lhs=rhs; rhs=t;
            swapped=1;
         }

         // If, after pad, rhs would be longer than lhs by digits+1 or
         // more then lhs cannot affect the answer, except as a residue,
         // so we only need to pad up to a length of DIGITS+1.
         if (rhs->digits+padding > lhs->digits+reqdigits) {
            // The RHS is sufficient
            decNumberCopy(res, rhs);             // [exponent from RHS]
            // flip the result sign if unswapped and rhs was negated
            if (!swapped) res->bits^=negate;


            if (rhs->digits<reqdigits) {         // need 0 padding
               int32_t shift=reqdigits-rhs->digits;
               res->digits=decShiftToMost(res->lsu, res->digits, shift);
               res->exponent-=shift;              // adjust the exponent.
            }
            // for residue we use the relative sign indication...
            residue=1;
            if (diffsign) residue=-residue; // signs differ
            status |= decRound(res, set, res->lsu, res->digits, residue);
            status |= decFinish(res, set);
            break;
         }

         // LHS digits may affect result
         rhsshift=D2U(padding+1)-1;        // this much by Unit shift ..
         mult=powers[padding-(rhsshift*DECDPUN)]; // .. this by multiplication
      } // padding needed


      if (diffsign) mult=-mult;           // signs differ

      // determine the longer operand
      maxdigits=rhs->digits+padding;      // virtual length of RHS
      if (lhs->digits>maxdigits) maxdigits=lhs->digits;

      // Decide on the result buffer to use; if possible place directly
      // into result.
      acc=res->lsu;                       // assume build direct
      // If destructive overlap, or the number is too long, or a carry or
      // borrow to DIGITS+1 might be possible we must use a buffer.
      // [Might be worth more sophisticated tests when maxdigits==reqdigits]
      if ((maxdigits>=reqdigits)          // is, or could be, too large
       || (res==rhs && rhsshift>0)) {     // destructive overlap
         // buffer needed; choose it
         // we'll need units for maxdigits digits, +1 Unit for carry or borrow
         int32_t need=D2U(maxdigits)+1;
         acc=accbuff;                     // assume use local buffer
         if (need*sizeof(DecNumberUnit)>sizeof(accbuff)) {
            allocacc=(DecNumberUnit *)malloc(need*sizeof(DecNumberUnit));
            if (allocacc==NULL) {         // hopeless -- abandon
               status|=DEC_Insufficient_storage;
               break;}
            acc=allocacc;
            alloced=1;
         }
      }

      res->bits=(uint8_t)(bits&DECNEG);   // it's now safe to overwrite..
      res->exponent=lhs->exponent;        // .. operands (even if aliased)

      // add [A+B*m] or subtract [A+B*(-m)]
      res->digits=DECDPUN * decUnitAddSub(
         lhs->lsu, D2U(lhs->digits),
         rhs->lsu, D2U(rhs->digits), rhsshift,
         acc, mult
      );

      if (res->digits<0) {           // we borrowed
         res->digits=-res->digits;
         res->bits^=DECNEG;           // flip the sign
      }

      // If we used a buffer we need to copy back, possibly rounding first
      // (If we didn't use buffer it must have fit, so can't need rounding
      // and residue must be 0.)
      if (acc!=res->lsu) {
         // We may have an underestimate.  This only occurs when both
         // numbers fit in DECDPUN digits and we are padding with a
         // negative multiple (-10, -100...) and the top digit(s) become 0
         if (res->digits<maxdigits) {
            *(acc+D2U(res->digits))=0;
            res->digits=maxdigits;
         }
          else {
            // remove leading zeros that we added due to rounding up to
            // integral Units (but only those in excess of the original
            // maxdigits length) before test for rounding.
            if (res->digits>reqdigits) {
               res->digits=decGetDigits(acc, D2U(res->digits));
               // Next line may go, to revert to 854 rules
               if (res->digits<maxdigits) res->digits=maxdigits;
            }
         }
         if (res->digits>reqdigits) {
            status |= decRound(res, set, acc, res->digits, residue);
         }
          else {                         // it fits; can't need rounding
            DecNumberUnit * d=res->lsu;  // destination pointer
            DecNumberUnit * s=acc;       // source pointer
            for (; s<acc+D2U(res->digits); ++s, ++d) *d=*s;
         }
      }
      // clean up leading zeros and check exponent
      res->digits=decGetDigits(res->lsu, D2U(res->digits));
      status |= decFinish(res, set);
      // non-zero additions cannot give -0 unless rounding is round_floor
      if (ISZERO(res)) {
         if ((set->flags & DEC_EXTENDED) && set->round==DEC_ROUND_FLOOR) {
            res->bits=DECNEG;
         }else {
            res->bits=0;
         }
      }
   }while(0);                              // end protected

   if (alloced) {
      if (allocacc!=NULL) free(allocacc);      // drop any storage we used
      if (allocrhs!=NULL) free(allocrhs);      // ..
      if (alloclhs!=NULL) free(alloclhs);      // ..
   }
   return decStatus(res, status);
}

/*----------------------------------------------------------------decDivideOp-+
| Description:                                                                |
|   decDivideOp -- division operation                                         |
|                                                                             |
|    This routine performs the calculations for all four division             |
|    operators (divide, divideInteger, remainder, remainderNear).             |
|                                                                             |
|    C=A op B                                                                 |
|                                                                             |
|     res is C, the result.  C may be A and/or B (e.g., X=X/X)                |
|     lhs is A                                                                |
|     rhs is B                                                                |
|     set is the context                                                      |
|     op  is DIVIDE, DIVIDEINT, REMAINDER, or REMNEAR respectively.           |
|                                                                             |
|   C must have space for set->digits digits.                                 |
|                                                                             |
|                                                                             |
|     The underlying algorithm of this routine is the same as in the          |
|     1981 S/370 implementation, that is, non-restoring long division         |
|     with bi-unit (rather than bi-digit) estimation for each unit            |
|     multiplier.  In this pseudocode overview, complications for the         |
|     Remainder operators and division residues for exact rounding are        |
|     omitted for clarity.                                                    |
|                                                                             |
|       Prepare operands and handle special values                            |
|       Test for x/0 and then 0/x                                             |
|       Exp =Exp1 - Exp2                                                      |
|       Exp =Exp +len(var1) -len(var2)                                        |
|       Sign=Sign1 * Sign2                                                    |
|       Pad accumulator (Var1) to double-length with 0's (pad1)               |
|       Pad Var2 to same length as Var1                                       |
|       msu2pair/plus=1st 2 or 1 units of var2, +1 to allow for round         |
|       have=0                                                                |
|       Do until (have=digits+1 OR residue=0)                                 |
|         if exp<0 then if integer divide/residue then leave                  |
|         this_unit=0                                                         |
|         Do forever                                                          |
|            compare numbers                                                  |
|            if <0 then leave inner_loop                                      |
|            if =0 then (* quick exit without subtract *) do                  |
|               this_unit=this_unit+1; output this_unit                       |
|               leave outer_loop; end                                         |
|            Compare lengths of numbers (mantissae):                          |
|            If same then tops2=msu2pair -- {units 1&2 of var2}               |
|                    else tops2=msu2plus -- {0, unit 1 of var2}               |
|            tops1=first_unit_of_Var1*10**DECDPUN +second_unit_of_var1        |
|            mult=tops1/tops2  -- Good and safe guess at divisor              |
|            if mult=0 then mult=1                                            |
|            this_unit=this_unit+mult                                         |
|            subtract                                                         |
|            end inner_loop                                                   |
|          if have\=0 | this_unit\=0 then do                                  |
|            output this_unit                                                 |
|            have=have+1; end                                                 |
|          var2=var2/10                                                       |
|          exp=exp-1                                                          |
|          end outer_loop                                                     |
|       exp=exp+1   -- set the proper exponent                                |
|       if have=0 then generate answer=0                                      |
|       Return (Result is defined by Var1)                                    |
|                                                                             |
|                                                                             |
|   We need two working buffers during the long division; one (digits+1)      |
|   to accumulate the result, and the other (up to 2*digits+1) for            |
|   long subtractions.  These are acc and var1 respectively.                  |
|   var1 is a copy of the lhs coefficient, var2 is the rhs coefficient.       |
|                                                                             |
|                                                                             |
+----------------------------------------------------------------------------*/
uint32_t decDivideOp(
   DecNumber * res,
   DecNumber const * lhs,
   DecNumber const * rhs,
   DecContext const *set,
   DecOperation op
) {
   DecNumber * alloclhs = 0;        // non-NULL if rounded lhs allocated
   DecNumber * allocrhs = 0;        // .., rhs
   DecNumberUnit accbuff[D2U(DECBUFFER+DECDPUN)]; // local buffer
   DecNumberUnit * acc=accbuff;     // -> accumulator array for result
   DecNumberUnit * allocacc=NULL;   // -> allocated buffer, iff allocated
   DecNumberUnit * accnext;         // -> where next digit will go
   int32_t acclength;               // length of acc needed [Units]
   int32_t accunits;                // count of units accumulated
   int32_t accdigits;               // count of digits accumulated

   // buffer for var1
   DecNumberUnit varbuff[D2U(DECBUFFER * 2+DECDPUN) * sizeof(DecNumberUnit)];
   DecNumberUnit * var1 = varbuff;  // -> var1 array for long subtraction
   DecNumberUnit * varalloc = 0;    // -> allocated buffer, iff used

   DecNumberUnit const * var2;      // -> var2 array

   int32_t var1units, var2units;    // actual lengths
   int32_t var2ulen;                // logical length (units)
   int32_t var1initpad;             // var1 initial padding (digits)
   DecNumberUnit * msu1;            // -> msu of each var
   DecNumberUnit const * msu2;      // -> msu of each var
   int32_t msu2plus;                // msu2 plus one [does not vary]
   eInt msu2pair;                   // msu2 pair plus one [does not vary]
   int32_t mult;                    // multiplier for subtraction
   DecNumberUnit thisunit;          // current unit being accumulated
   int32_t residue;                 // for rounding
   uint32_t status = 0;             // accumulator
   int32_t reqdigits = set->digits; // requested DIGITS
   int32_t exponent;                // working exponent
   uint32_t bits;                   // working sign
   uint8_t merged;                  // merged flags
   DecNumberUnit * target;          // work
   DecNumberUnit const * source;    // ..
   uint32_t const * pow;            // ..
   int32_t shift, cut;              // ..

   do {                             // protect allocated storage
      // reduce operands and set lostDigits status, as needed
      if (lhs->digits>reqdigits) {
         alloclhs=decRoundOperand(lhs, set, &status);
         if (alloclhs==NULL) break;
         lhs=alloclhs;
      }
      if (rhs->digits>reqdigits) {
         allocrhs=decRoundOperand(rhs, set, &status);
         if (allocrhs==NULL) break;
         rhs=allocrhs;
      }

      bits = (lhs->bits ^ rhs->bits) & DECNEG;  // assumed sign for divisions

      // handle infinities and NaNs
      merged = (lhs->bits | rhs->bits) & DECSPECIAL;
      if (merged) {                        // a special bit set
         if (merged & DECSNAN) {           // sNaN always bad
            status|=DEC_Invalid_operation;
            break;
         }
         if (merged & DECNAN) {            // NaNs beget qNaN
            decNumberZero(res);
            res->bits=DECNAN;
            break;                         // (status=0)
         }
         // one or two infinities
         if (lhs->bits & DECINF) {         // LHS (dividend) is infinite
            if (
               rhs->bits & DECINF ||       // two infinities are invalid
               op & (DEC_OP_REMAINDER | DEC_OP_REMNEAR)
            ) {                            // as is remainder of infinity
               status|=DEC_Invalid_operation;
               break;
            }
            if (!ISZERO(rhs)) {            // [drop through for inf/0]
               decNumberZero(res);
               res->bits=bits|DECINF;      // set +/- infinity
               break;
            }
         }else {                           // RHS (divisor) is infinite
            if (op & (DEC_OP_REMAINDER | DEC_OP_REMNEAR)) {
               // result is [finished clone of] lhs
               decNumberCopy(res, lhs);
            }else {
               decNumberZero(res);
               res->bits=bits;             // set +/- zero
            }
            status |= decFinish(res, set);
            break;
         }
      }

      // handle 0 rhs and lhs [after lostDigits checking]
      if (ISZERO(rhs)) {                    // x/0 is always exceptional
         if (ISZERO(lhs)) {
            decNumberZero(res);             // [after lhs test]
            status|=DEC_Division_undefined; // 0/0 will become NaN
         }else {
            decNumberZero(res);
            if (op & (DEC_OP_REMAINDER | DEC_OP_REMNEAR)) {
               status |= DEC_Invalid_operation | DEC_Division_by_zero;
            }else {
               status |= DEC_Division_by_zero; // x/0
               res->bits = bits | DECINF;    // .. is +/- Infinity
            }
         }
         break;
      }

      if (ISZERO(lhs)) {                  // 0/x -> 0  [x!=0]
         if (op & (DEC_OP_REMAINDER | DEC_OP_REMNEAR)) bits=lhs->bits;
         decNumberZero(res);
         if ((set->flags & DEC_EXTENDED)) res->bits=bits;// -0 possible
         break;
      }

      // Precalculate exponent.  This starts off adjusted (and hence fits
      // in 31 bits) and becomes the usual unadjusted exponent as the
      // division proceeds.  The order of evaluation is important, here,
      // to avoid wrap.
      exponent=(lhs->exponent+lhs->digits)-(rhs->exponent+rhs->digits);

      // If the working exponent is -ve, then some quick exits are
      // possible because the quotient is known to be <1
      // [for REMNEAR, it needs to be < -1, as -0.5 could need work]
      if ((exponent < 0) && !(op == DEC_OP_DIVIDE)) {
         if (op & DEC_OP_DIVIDEINT) {
            decNumberZero(res);  // integer part is 0
            break;
         }
         if (op & DEC_OP_REMAINDER || exponent<-1) {
            // It is REMAINDER or safe REMNEAR; result is [finished
            // clone of] lhs  (r = x - 0*y)
            decNumberCopy(res, lhs);
            status |= decFinish(res, set);
            break;
         }
         // [unsafe REMNEAR drops through]
      }

      /* We need long (slow) division; roll up the sleeves... */

      // If accumulator is too long for local storage, then allocate
      acclength=D2U(reqdigits+DECDPUN);
      if (acclength * sizeof(DecNumberUnit) > sizeof(accbuff)) {
         allocacc=(DecNumberUnit *)malloc(acclength * sizeof(DecNumberUnit));
         if (allocacc==NULL) {        // hopeless -- abandon
            status|=DEC_Insufficient_storage;
            break;
         }
         acc = allocacc;              // use the allocated space
      }

      // If var1 is too long for local storage, then allocate.
      // The maximum units we need for var1 (long subtraction) is:
      //   D2U(digits+1)+1        -- for the quotient [may not be aligned]
      //   +1                     -- for leading 0s
      //   +(D2U(rhs->digits)-1)  -- for overlap of rhs
      //   +1                     -- pre-adjust if REMAINDER or DIVIDEINT
      // [Note: unused units do not participate in decUnitAddSub data]
      var1units=D2U(reqdigits+1) + D2U(rhs->digits) + 1;
      if (!(op & DEC_OP_DIVIDE)) ++var1units;
      if (var1units * sizeof(DecNumberUnit) > sizeof(varbuff)) {
         varalloc=(DecNumberUnit *)malloc(var1units*sizeof(DecNumberUnit));
         if (varalloc==NULL) {             // hopeless -- abandon
            status|=DEC_Insufficient_storage;
            break;
         }
         var1=varalloc;                     // use the allocated space
      }

      // Extend the lhs and rhs to full long subtraction length.  The lhs
      // is truly extended into the var1 buffer, with 0 padding, so we can
      // subtract in place.  The rhs (var2) has virtual padding
      // (implemented by decUnitAddSub).
      msu1 = var1 + var1units - 1;          // msu of var1
      source = lhs->lsu + D2U(lhs->digits) - 1; // msu of input array
      for (target=msu1; source>=lhs->lsu; --source, --target) *target=*source;
      for (; target >= var1; --target) *target=0;

      // var2 is left-aligned with var1 at the start
      var2ulen = var1units;                 // rhs logical length (units)
      var2units = D2U(rhs->digits);         // rhs actual length (units)
      var2 = rhs->lsu;                      // -> rhs array
      msu2 = var2+var2units-1;              // -> msu of var2 [never changes]
      // now set up the variables which we'll use for estimating the
      // multiplication factor.  If these variables are not exact, we add
      // 1 to make sure that we never overestimate the multiplier.
      msu2plus = *msu2;                     // it's value ..
      if (var2units>1) ++msu2plus;          // .. +1 if any more
      msu2pair = (eInt)*msu2*(DECDPUNMAX+1);// top two pair ..
      if (var2units>1) {                    // .. [else treat 2nd as 0]
         msu2pair += *(msu2-1);             // ..
         if (var2units>2) ++msu2pair;       // .. +1 if any more
      }

      // Since we are working in units, the units may have leading zeros,
      // but we calculated the exponent on the assumption that they are
      // both left-aligned.  Adjust the exponent to compensate: add the
      // number of leading zeros in var1 msu and subtract those in var2 msu.
      // [We actually do this by counting the digits and negating, as
      // lead1=DECDPUN-digits1, and similarly for lead2.]
      for (pow=&powers[1]; *msu1 >= *pow; ++pow) --exponent;
      for (pow=&powers[1]; *msu2 >= *pow; ++pow) ++exponent;

      // Now, if doing an integer divide or remainder, we want to ensure
      // that the result will be Unit-aligned.  To do this, we shift the
      // var1 accumulator towards least if need be.  (It's much easier to
      // do this now than to reassemble the residue afterwards, if we are
      // doing a remainder.)
      if (!(op & DEC_OP_DIVIDE)) {
         // save the initial padding of var1, in digits, for REMAINDER
         var1initpad=(var1units-D2U(lhs->digits))*DECDPUN;
         // work out the shift to do.  We know that, before the leading
         // zero adjustment, the exponent was >= -1 for these operations,
         // so it could now be -DECDPUN (in which case we need to shift by
         // DECDPUN).  In other cases, we can do a safe remainder by adding
         // DECDPUN first [to ensure non-negative] before the %.
         if (exponent==-DECDPUN) {
            cut=DECDPUN;
         }else {
            cut=(exponent+DECDPUN)%DECDPUN;
            if (cut>0) cut=DECDPUN-cut;
         }
         if (cut!=0) {
            decShiftToLeast(var1, var1units, cut);
            exponent+=cut;                  // maintain numerical value
            var1initpad-=cut;               // .. and reduce padding
         }
      }else { // is divide
         // optimization: if the first iteration will just produce 0,
         // preadjust to skip it [valid for DIVIDE only]
         if (*msu1 < *msu2) {
            --var2ulen;                     // shift down
            exponent -= DECDPUN;            // update the exponent
         }
      }

      // start the long-division loops
      accunits=0;                         // no units accumulated yet
      accdigits=0;                        // .. or digits
      accnext=acc+acclength-1;            // -> msu of acc [NB: allows digits+1]
      for (;;) {                          // outer forever loop
         thisunit=0;                       // current unit assumed 0
         // find the next unit
         for (;;) {                        // inner forever loop
            // strip leading zero units [from either pre-adjust or from
            // subtract last time around].  Leave at least one unit.
            for (; *msu1==0 && msu1>var1; msu1--) var1units--;

            if (var1units < var2ulen) break;      // var1 too low for subtract
            if (var1units == var2ulen) {          // unit-by-unit compare needed
               // compare the two numbers, from msu
               DecNumberUnit * pv1;               // units to compare
               DecNumberUnit const * pv2;
               DecNumberUnit v2;
               pv2 = msu2;                        // -> msu
               for (pv1=msu1; pv1>=var1; pv1--, pv2--) {
                  // v1=*pv1 -- always OK
                  v2=0;                           // assume in padding
                  if (pv2>=var2) v2=*pv2;         // in range
                  if (*pv1!=v2) break;            // no longer the same
               }
               // here when all inspected or a difference seen
               if (*pv1<v2) break;                // var1 too low to subtract
               if (*pv1==v2) {                    // var1 == var2
                  // reach here if var1 and var2 are identical; subtraction
                  // would increase digit by one, and the residue will be 0 so
                  // we are done; leave the loop with residue set to 0.
                  ++thisunit;                     // as though subtracted
                  *var1=0;                        // set var1 to 0
                  var1units=1;                    // ..
                  break;  // from inner
               } // var1 == var2
               // *pv1>v2.  Prepare for real subtraction; the lengths are equal
               // Estimate the multiplier (there's always a msu1-1)...
               // Bring in two units of var2 to provide a good estimate.
               mult=((eInt)*msu1*(DECDPUNMAX+1)+*(msu1-1))/msu2pair;

            }else { // var1units > var2ulen, so subtraction is safe
               // The var2 msu is one unit towards the lsu of the var1 msu,
               // so we can only use one unit for var2.
               mult=((eInt)*msu1*(DECDPUNMAX+1)+*(msu1-1))/msu2plus;
            }
            if (mult==0) mult=1;                 // must always be at least 1
            // subtraction needed; var1 is > var2
            thisunit+=mult;                      // accumulate
            // subtract var1-var2, into var1; only the overlap needs
            // processing, as we are in place
            shift=var2ulen-var2units;
            decUnitAddSub(
               &var1[shift], var1units-shift,
               var2, var2units, 0,
               &var1[shift], -mult
            );
            // var1 now probably has leading zeros; these are removed at the
            // top of the inner loop.
         }

         // We have the next unit; unless it's a leading zero, add to acc
         if (accunits!=0 || thisunit!=0) {      // put the unit we got
            *accnext=thisunit;                   // store in accumulator
            // account exactly for the digits we got
            if (accunits==0) {
               ++accdigits;                       // at least one
               for (pow=&powers[1]; thisunit>=*pow; ++pow) ++accdigits;
            }else {
               accdigits += DECDPUN;
            }
            ++accunits;                          // update count
            --accnext;                           // ready for next
            if (accdigits>reqdigits) break;      // we have all we need
         }
         // if the residue is zero, we're done
         if (var1units==1 && *var1==0) break;   // residue is 0
         // we've also done enough if calculating remainder or integer
         // divide and we just did the last ('units') unit
         if (exponent==0 && !(op & DEC_OP_DIVIDE)) break;

         // to get here, var1 is less than var2, so divide var2 by the per-
         // Unit power of ten and go for the next digit
         var2ulen--;                            // shift down
         exponent-=DECDPUN;                     // update the exponent
      }

      // here: acc      has at least digits+1 of good results (or fewer
      //                if early stop), starting at accnext+1 (its lsu)
      //       var1     has any residue at the stopping point
      //       accunits is the number of digits we collected in acc
      if (accunits==0) {             // acc is 0
         accunits=1;                 // show we have one ..
         accdigits=1;                // ..
         *accnext=0;                 // .. whose value is 0
      }else {
         ++accnext;                  // back to last placed
      }
      // accnext now -> lowest unit of result

      residue=0;                     // assume no residue
      if (op & DEC_OP_DIVIDE) {
         // record the presence of any residue, for rounding
         if (*var1!=0 || var1units>1) residue=1;
      }
       else /* op != DIVIDE */ {
         // check for coefficient overflow
         if (accdigits + exponent > reqdigits) {
            status|=DEC_Division_impossible;
            break;
         }
         if (op & (DEC_OP_REMAINDER | DEC_OP_REMNEAR)) {
            // [Here, the exponent will be 0, because we adjusted var1
            // appropriately.]
            int32_t postshift;                     // work
            uint8_t odd=0, zero=0;                 // integer was odd, zero
            // if the quotient was 0 the rhs plays no part unless we are
            // doing remainderNear
            if (*accnext==0 && accunits==1) {      // acc is 0 ..
               zero=1;                             // [remember for later]
               if (op & DEC_OP_REMAINDER) {        // .. and simple remainder
                  decNumberCopy(res, lhs);         // return lhs..
                  status |= decFinish(res, set);   // .. canonical
                  break;
               }
            }
            // Fastpath when residue is truly 0 is also worthwhile [and
            // simplifies the code below]
            if (*var1==0 && var1units==1) {      // residue is 0
               decNumberZero(res);                // .. easy
               break;
            }
            if (*accnext & 0x01) odd=1;          // record even or odd integer

            // treat the residue, in var1, as the value to return, via acc
            // calculate the unused zero digits.  This is the smaller of:
            //   var1 initial padding (saved above)
            //   var2 residual padding, which happens to be given by:
            postshift=var1initpad+exponent-lhs->exponent+rhs->exponent;
            // [the 'exponent' term accounts for the shifts during divide]
            if (var1initpad<postshift) postshift=var1initpad;

            // shift var1 the requested amount, and adjust its digits
            decShiftToLeast(var1, var1units, postshift);
            accdigits=decGetDigits(var1, var1units)-postshift;
            accnext=var1;
            accunits=D2U(accdigits);

            exponent=lhs->exponent;         // exponent is smaller of lhs & rhs
            if (rhs->exponent<exponent) exponent=rhs->exponent;
            bits=lhs->bits;                 // sign is always as lhs

            // Now correct the result if we are doing remainderNear; if its
            // it (looking just at coefficients) is > rhs/2, or == rhs/2 and
            // the integer was odd then the result should be rem-rhs.
            if (op & DEC_OP_REMNEAR) {
               // calculate remainder*2 into the acc buffer (which is no
               // longer needed, and will certainly have enough space)
               int32_t compare;
               int32_t tarunits=decUnitAddSub(
                   accnext, accunits,
                   accnext, accunits, 0,
                   acc, 1
               );

               // Here, acc holds tarunits Units with twice the remainder's
               // coefficient, which we must now compare to the RHS.
               // The remainder's exponent may be smaller than the RHS's.
               compare=decUnitCompare(
                  acc, tarunits,
                  rhs->lsu, D2U(rhs->digits),
                  rhs->exponent-exponent
               );
               if (compare==DEC_BADINT) {        // deep trouble
                  status|=DEC_Insufficient_storage;
                  break;
               }
               if (compare>0 || (compare==0 && odd)) {
                  // we need rem-rhs; the sign will invert.  Again we can
                  // safely use acc for the working Units array.
                  int32_t exp=rhs->exponent-exponent;  // RHS padding needed
                  // Calculate units and remainder from exponent.
                  int32_t expunits=exp / DECDPUN;
                  int32_t exprem = exp % DECDPUN;
                  // subtract [A+B*(-m)]; the result will always be negative
                  accunits = -decUnitAddSub(
                     accnext, accunits,
                     rhs->lsu, D2U(rhs->digits), expunits,
                     acc, -(int32_t)powers[exprem]
                  );
                  accdigits=decGetDigits(acc, accunits); // count digits exactly
                  accnext=acc;                // redirect result source
                  // [exponent is as for original remainder]
                  bits^=DECNEG;               // flip the sign

               }else {   // could perhaps be 0 quotient case
                  if (zero) {
                     // It is REMNEAR with a quotient of 0 (rather than 1)
                     // result is [finished clone of] lhs  (r = x - 0*y)
                     // [we must ignore the RHS scale]
                     decNumberCopy(res, lhs);
                     status |= decFinish(res, set);
                     break;
                  }
               }
            }
         }
      }

      // Set digits, exponent, and bits
      res->digits=accdigits;
      res->exponent=exponent;
      res->bits=(uint8_t)(bits&DECNEG);   // [cleaned]

      // Now the coefficient.  If it's too long then round it into res.
      // Otherwise (it fits) simply copy to result.
      if (res->digits>reqdigits) {
         status |= decRound(res, set, accnext, res->digits, residue);
      }else {                             // it fits
         target = res->lsu;               // destination pointer
         source = accnext;                // source pointer
         for (; source<accnext+accunits; ++source, ++target) *target=*source;
      }
      if (op & DEC_OP_DIVIDE) decTrim(res, 0); // strip trailing zeros
      status |= decFinish(res, set);      // final cleanup
   }while(0);                             // end protected

   if (varalloc!=NULL) free(varalloc);    // drop any storage we used
   if (allocacc!=NULL) free(allocacc);    // ..
   if (allocrhs!=NULL) free(allocrhs);    // ..
   if (alloclhs!=NULL) free(alloclhs);    // ..
   return decStatus(res, status);
}

/*---------------------------------------------------------------decCompareOp-+
|   decCompareOp -- compare, min, or max two Numbers                          |
|                                                                             |
|     This computes C = A ? B and returns the signum (as a Number)            |
|     for COMPARE or the maximum or minumum (for COMPMAX and COMPMIN).        |
|                                                                             |
|     res is C, the result.  C may be A and/or B (e.g., X=X?X)                |
|     lhs is A                                                                |
|     rhs is B                                                                |
|     set is the context                                                      |
|     op  is the operation flag                                               |
|                                                                             |
|   C must have space for one digit for COMPARE or set->digits for            |
|   COMPMAX and COMPMIN.                                                      |
|                                                                             |
|   The emphasis here is on speed for common cases, and avoiding              |
|   coefficient comparison if possible.                                       |
+----------------------------------------------------------------------------*/
uint32_t decCompareOp(
   DecNumber * res,
   DecNumber const * lhs,
   DecNumber const * rhs,
   DecContext const *set,
   uint8_t op
) {
   DecNumber *alloclhs = 0;         // non-NULL if rounded lhs allocated
   DecNumber *allocrhs = 0;         // .., rhs
   int32_t result = 0;              // result value [0 in case of error]
   uint32_t  status = 0;            // accumulator
   uint8_t merged;                  // merged flags
   uint8_t bits = 0;                // non-0 for NaN
   int32_t reqdigits = set->digits; // local copy; requested DIGITS

   do {                             // protect allocated storage
      // reduce operands and set lostDigits status, as needed
      if (lhs->digits>reqdigits) {
         alloclhs=decRoundOperand(lhs, set, &status);
         if (alloclhs==NULL) break;
         lhs=alloclhs;
      }
      if (rhs->digits>reqdigits) {
         allocrhs=decRoundOperand(rhs, set, &status);
         if (allocrhs==NULL) break;
         rhs=allocrhs;
      }

      // handle NaNs now; let infinities drop through
      merged=(lhs->bits | rhs->bits) & (DECSNAN | DECNAN);
      if (merged) {                      // a NaN bit set
         if (merged & DECSNAN) {         // sNaN always bad
            status|=DEC_Invalid_operation;
            break;                       // qNaN set later
         }
         // must be qNaN; NaNs beget qNaN
         bits = DECNAN;
         op = COMPARE;                   // use the COMPARE path
         break;                          // (status=0)
      }
      result=decCompare(lhs, rhs);       // we have numbers
   }while(0);                            // end protected

   if (result==DEC_BADINT) {
      status |= DEC_Insufficient_storage; // rare
   }else {
      if (op == COMPARE) {               // return signum
         decNumberZero(res);             // [always a valid result]
         if (result==0) {                // (maybe qNaN)
            res->bits=bits;
         }else {
            *res->lsu = 1;
            if (result<0) res->bits=DECNEG;
         }
      }else {                             // max or min
         // copy the appropriate operand to the result
         DecNumber const * choice;
         // modify the result if extended and the numbers were -0 and +0
         if (
            (set->flags & DEC_EXTENDED) && result==0 &&
            (lhs->bits&DECNEG)!=(rhs->bits&DECNEG)
         ) {
            if (lhs->bits&DECNEG) result=-1; else result=1;
         }
         if (op == COMPMAX) {
            choice=(result>=0 ? lhs : rhs); // choose maximum
         }else {
            choice=(result<=0 ? lhs : rhs); // choose minimum
         }
         decNumberCopy(res, choice);
      }
   }
   if (allocrhs!=NULL) free(allocrhs);   // free any storage we used
   if (alloclhs!=NULL) free(alloclhs);   // ..
   return decStatus(res, status);
}


/*-----------------------------------------------------------------decCompare-+
|   decCompare -- compare two decNumbers                                      |
|                                                                             |
|    This routine compares A ? B without altering them.                       |
|                                                                             |
|    Arg1 is A, a DecNumber which is not a NaN                                |
|    Arg2 is B, a DecNumber which is not a NaN                                |
|                                                                             |
|    returns -1, 0, or 1 for A<B, A==B, or A>B, or DEC_BADINT if failure      |
|    (the only possible failure is an allocation error)                       |
|                                                                             |
|   This could be merged into decCompareOp                                    |
|                                                                             |
+----------------------------------------------------------------------------*/
static int32_t decCompare(DecNumber const * lhs, DecNumber const * rhs)
{
   int32_t result;                    // result value [0 in case of error]
   int32_t sigr;                      // rhs signum
   int32_t compare;                   // work
   if (ISZERO(lhs)) {
      result = 0;
   }else if (lhs->bits & DECNEG) {
      result = -1;
   }else {
      result = 1;
   }
   if (ISZERO(rhs)) {
      sigr = 0;
   }else if (rhs->bits & DECNEG) {
      sigr = -1;
   }else {
      sigr=1;
   }
   if (result > sigr) return +1;              // L > R, return 1
   if (result < sigr) return -1;              // R < L, return -1

   // signums are the same
   if (result==0) return 0;                   // both 0
   // Both non-zero
   if ((lhs->bits | rhs->bits) & DECINF) {    // one or more infinities
      if (lhs->bits == rhs->bits) {           // both the same
         result = 0;
      }else if (rhs->bits & DECINF) {
         result = -result;
      }
      return result;
   }

   // Now compare the coefficients, allowing for exponents
   if (lhs->exponent>rhs->exponent) {         // LHS exponent larger
      // swap sides, and sign
      DecNumber const * temp = lhs;
      lhs = rhs;
      rhs = temp;
      result =- result;
   }
   compare = decUnitCompare(
      lhs->lsu, D2U(lhs->digits),
      rhs->lsu, D2U(rhs->digits),
      rhs->exponent-lhs->exponent
   );
   if (compare!=DEC_BADINT) compare *= result; // comparison succeeded
   return compare;                             // what we got
}

/*-------------------------------------------------------------decUnitCompare-+
|   decUnitCompare -- compare two >=0 integers in Unit arrays                 |
|                                                                             |
|    This routine compares A ? B*10**E where A and B are unit arrays          |
|    A is a plain integer                                                     |
|    B has an exponent of E (which must be non-negative)                      |
|                                                                             |
|    Arg1 is A first Unit (lsu)                                               |
|    Arg2 is A length in Units                                                |
|    Arg3 is B first Unit (lsu)                                               |
|    Arg4 is B length in Units                                                |
|    Arg5 is E                                                                |
|                                                                             |
|    returns -1, 0, or 1 for A<B, A==B, or A>B, or DEC_BADINT if failure      |
|    (the only possible failure is an allocation error)                       |
+----------------------------------------------------------------------------*/
static int32_t decUnitCompare(
   DecNumberUnit const * a, uint32_t alength,
   DecNumberUnit const * b, uint32_t blength,
   int32_t exp
) {
   DecNumberUnit * acc;            // accumulator for result
   DecNumberUnit accbuff[D2U(DECBUFFER+1)]; // local buffer
   DecNumberUnit * allocacc = 0;   // -> allocated acc buffer, iff allocated
   int32_t accunits;               // units in use for acc
   uint32_t need;                  // units needed for acc
   int32_t expunits, exprem;       // ..

   if (exp==0) {                    // aligned; fastpath
      if (alength > blength) return 1;
      if (alength < blength) return -1;
      // same number of units in both -- need unit-by-unit compare
      DecNumberUnit const * l = a + alength - 1;
      DecNumberUnit const * r = b + alength - 1;
      for (; l >= a; --l, --r) {
         if (*l > *r) return 1;
         if (*l < *r) return -1;
      }
      return 0;                      // all units match
   }

   // Unaligned.  If one is >1 unit longer than the other, padded
   // approximately, then we can return easily
   if (alength > blength+D2U(exp)) return 1;
   if (alength+1 < blength+D2U(exp)) return -1;

   // We need to do a real subtract.  For this, we need a result buffer
   // even though we only are interested in the sign.  Its length needs
   // to be the larger of alength and padded blength, +2
   need = blength+D2U(exp);              // maximum real length of B
   if (need < alength) need = alength;
   need += 2;
   acc = accbuff;                        // assume use local buffer
   if (need * sizeof(DecNumberUnit) > sizeof(accbuff)) {
      allocacc = (DecNumberUnit *)malloc(need * sizeof(DecNumberUnit));
      if (!allocacc) return DEC_BADINT;  // hopeless -- abandon
      acc = allocacc;
   }
   // Calculate units and remainder from exponent.
   expunits=exp/DECDPUN;
   exprem=exp%DECDPUN;
   // subtract [A+B*(-m)]
   accunits=decUnitAddSub(
      a, alength,
      b, blength, expunits,
      acc, -(int32_t)powers[exprem]
   );

   while ((accunits>0) && (!*acc)) {     // forget leading zeroes
      ++acc;
      --accunits;
   }
   if (allocacc!=NULL) free(allocacc);   // drop any storage we used
   return (accunits > 0)? 1 : ((accunits < 0)? -1 : 0);
}

/*--------------------------------------------------------------decUnitAddSub-+
|   decUnitAddSub -- add or subtract two >=0 integers in Unit arrays          |
|                                                                             |
|    This routine performs the calculation:                                   |
|                                                                             |
|    C=A+(B*M)                                                                |
|                                                                             |
|    Where M is in the range -DECDPUNMAX through +DECDPUNMAX.                 |
|                                                                             |
|    A may be shorter or longer than B.                                       |
|                                                                             |
|    Leading zeros are not removed after a calculation.  The result is        |
|    either the same length as the longer of A and B (adding any              |
|    shift), or one Unit longer than that (if a Unit carry occurred).         |
|                                                                             |
|    A and B content are not altered unless C is also A or B.                 |
|    C may be the same array as A or B, but only if no zero padding is        |
|    requested (that is, C may be B only if bshift==0).                       |
|    C is filled from the lsu; only units necessary to complete the           |
|    calculation are referenced.                                              |
|                                                                             |
|    Arg1 is A first Unit (lsu)                                               |
|    Arg2 is A length in Units                                                |
|    Arg3 is B first Unit (lsu)                                               |
|    Arg4 is B length in Units                                                |
|    Arg5 is B shift in Units  (>=0; pads with 0 units if positive)           |
|    Arg6 is C first Unit (lsu)                                               |
|    Arg7 is M, the multiplier                                                |
|                                                                             |
|    returns the count of Units written to C, which will be non-zero          |
|    and negated if the result is negative.  That is, the sign of the         |
|    returned int32_t is the sign of the result (positive for zero) and       |
|    the absolute value of the int32_t is the count of Units.                 |
|                                                                             |
|    It is the caller's responsibility to make sure that C size is            |
|    safe, allowing space if necessary for a one-Unit carry.                  |
|                                                                             |
|    This routine is severely performance-critical; *any* change here         |
|    must be measured (timed) to assure no performance degradation.           |
|    In particular, trickery here tends to be counter-productive, as          |
|    increased complexity of code hurts register optimizations on             |
|    register-poor architectures.  Avoiding divisions is nearly               |
|    always a Good Idea, however.                                             |
|                                                                             |
|   Special thanks to Rick McGuire (IBM Cambridge) and Dave Clark (IBM        |
|   Warwick) for some of the ideas used in this routine.                      |
+----------------------------------------------------------------------------*/
static int32_t decUnitAddSub(
   DecNumberUnit const * a, int32_t alength,
   DecNumberUnit const * b, int32_t blength, int32_t bshift,
   DecNumberUnit * c, int32_t m
) {
   DecNumberUnit const * alsu = a;  // A lsu [need to remember it]
   DecNumberUnit * clsu = c;        // C ditto
   DecNumberUnit * minC;            // low water mark for C
   DecNumberUnit * maxC;            // high water mark for C
   eInt carry=0;                    // carry integer (could be Long)
   int32_t  est;                    // estimated quotient

   maxC = c+alength;                // A is usually the longer
   minC = c+blength;                // .. and B the shorter
   if (bshift != 0) {               // B is shifted; low As copy across
      minC += bshift;
      // if in place [common], skip copy unless there's a gap [rare]
      if (a==c && bshift <= alength) {
         c += bshift;
         a += bshift;
      }else {
         for (; c<clsu+bshift; ++a, ++c) {  // copy needed
            if (a<alsu+alength) *c=*a; else *c=0;
         }
      }
   }
   if (minC > maxC) { // swap
      DecNumberUnit *hold = minC;
      minC = maxC;
      maxC = hold;
   }

   // For speed, we do the addition as two loops; the first where both A
   // and B contribute, and the second (if necessary) where only one or
   // other of the numbers contribute.
   // Carry handling is the same (i.e., duplicated) in each case.
   for (; c<minC; ++c) {
      carry += *a;
      ++a;
      carry += ((eInt)*b) * m;
      ++b;
      // here carry is new Unit of digits; it could be +ve or -ve
      if ((ueInt)carry <= DECDPUNMAX) {     // fastpath 0-DECDPUNMAX
         *c = (DecNumberUnit)carry;
         carry = 0;
         continue;
      }
      // remainder is undefined if negative, so we must test
      #if DECDPUN==4                             // use divide-by-multiply
         if (carry >= 0) {
            est=(((unsigned)carry>>11)*53687)>>18;
            *c=(DecNumberUnit)(carry-est*(DECDPUNMAX+1)); // remainder
            carry=est;                           // likely quotient [89%]
            if (*c < DECDPUNMAX+1) continue;     // estimate was correct
            ++carry;
            *c -= DECDPUNMAX+1;
            continue;
         }
         // negative case
         carry = carry + (eInt)(DECDPUNMAX+1)*(DECDPUNMAX+1); // make positive
         est=(((unsigned)carry>>11)*53687)>>18;
         *c=(DecNumberUnit)(carry-est*(DECDPUNMAX+1));
         carry=est-(DECDPUNMAX+1);              // correctly negative
         if (*c < DECDPUNMAX+1) continue;         // was OK
         ++carry;
         *c -= DECDPUNMAX+1;
      #else
         if ((ueInt)carry<(DECDPUNMAX+1)*2){    // fastpath carry +1
            *c=(DecNumberUnit)(carry-(DECDPUNMAX+1));     // [helps additions]
            carry=1;
            continue;
         }
         if (carry>=0) {
            *c=(DecNumberUnit)(carry%(DECDPUNMAX+1));
            carry=carry/(DECDPUNMAX+1);
            continue;
         }
         // negative case
         carry = carry+(eInt)(DECDPUNMAX+1)*(DECDPUNMAX+1); // make positive
         *c = (DecNumberUnit)(carry % (DECDPUNMAX+1));
         carry = carry / (DECDPUNMAX+1)-(DECDPUNMAX+1);
      #endif
   }

   // we now may have one or other to complete
   // [pretest to avoid loop setup/shutdown]
   if (c<maxC) for (; c<maxC; ++c) {
      if (a < alsu+alength) {             // still in A
         carry += *a;
         ++a;
      }
       else {                             // inside B
         carry += ((eInt)*b) * m;
         ++b;
      }
      // here carry is new Unit of digits; it could be +ve or -ve
      if ((ueInt)carry<=DECDPUNMAX) {     // fastpath 0-DECDPUNMAX
         *c = (DecNumberUnit)carry;
         carry=0;
         continue;
      }
      #if DECDPUN==4                           // use divide-by-multiply
         // remainder is undefined if negative, so we must test
         if (carry>=0) {
            est=(((unsigned)carry>>11)*53687)>>18;
            *c=(DecNumberUnit)(carry-est*(DECDPUNMAX+1)); // remainder
            carry=est;                           // likely quotient [79.7%]
            if (*c < DECDPUNMAX+1) continue;     // estimate was correct
            ++carry;
            *c -= DECDPUNMAX+1;
            continue;
         }
         // negative case
         carry = carry+(eInt)(DECDPUNMAX+1)*(DECDPUNMAX+1); // make positive
         est = (((unsigned)carry>>11)*53687)>>18;
         *c = (DecNumberUnit)(carry-est*(DECDPUNMAX+1));
         carry = est-(DECDPUNMAX+1);            // correctly negative
         if (*c<DECDPUNMAX+1) continue;         // was OK
         ++carry;
         *c -= DECDPUNMAX+1;
      #else
         if ((ueInt)carry<(DECDPUNMAX+1)*2){    // fastpath carry 1
            *c = (DecNumberUnit)(carry-(DECDPUNMAX+1));
            carry = 1;
            continue;
         }
         // remainder is undefined if negative, so we must test
         if (carry >= 0) {
            *c = (DecNumberUnit)(carry%(DECDPUNMAX+1));
            carry = carry/(DECDPUNMAX+1);
            continue;
         }
         // negative case
         carry = carry+(eInt)(DECDPUNMAX+1)*(DECDPUNMAX+1); // make positive
         *c = (DecNumberUnit)(carry%(DECDPUNMAX+1));
         carry = carry/(DECDPUNMAX+1)-(DECDPUNMAX+1);
      #endif
   }

   // OK, all A and B processed; might still have carry or borrow
   if (carry!=0) {
      if (carry<0) {                  // it's a borrow; complement needed
         int32_t add=1;               // temporary carry...
         for (c=clsu; c<maxC; ++c) {
            add=DECDPUNMAX+add-*c;
            if (add<=DECDPUNMAX) {
               *c=(DecNumberUnit)add;
               add=0;
            }else {
               *c=0;
               add=1;
            }
         }
         // calculate an extra unit if borrowed more than one
         if (carry+1!=0) {
            *c=(DecNumberUnit)(add-carry-1);
            // [carry is -ve]
            ++c;
         }
         return clsu-c;               // -ve result indicates borrowed
      }
      // +ve carry
      *c = (DecNumberUnit)carry;
      ++c;
   }

   // return number of Units in the result
   return c-clsu;
}

/*-------------------------------------------------------------decShiftToMost-+
|   decShiftToMost -- shift digits in array towards most significant          |
|                                                                             |
|     uar    is the array                                                     |
|     digits is the count of digits in use in the array                       |
|     shift  is the number of zeros to pad with (least significant)           |
|                                                                             |
|     returns the new length of the integer in the array, in digits           |
|                                                                             |
|   No overflow is permitted (that is, the uar array must be known to         |
|   be large enough to hold the result, after shifting).                      |
+----------------------------------------------------------------------------*/
static int32_t decShiftToMost(
   DecNumberUnit *uar,
   int32_t digits,
   int32_t shift
) {
   DecNumberUnit  *target, *source, *first;  // work
   uint32_t rem;                    // for division
   int32_t cut;                     // odd 0's to add
   uint32_t next;                   // work

   if ((digits+shift) <= DECDPUN) { // [fastpath] single-unit case
      *uar *= powers[shift];
      return digits+shift;
   }

   cut=(DECDPUN - shift%DECDPUN) % DECDPUN;
   source = uar+ D2U(digits)-1;        // where msu comes from
   first = uar + D2U(digits+shift)-1;  // where msu of source will end up
   target= source + D2U(shift);        // where upper part of first cut goes
   next = 0;

   for (; source >= uar; --source, --target) {
      // split the source Unit and accumulate remainder for next
      #if DECDPUN<=4
        uint32_t quot = QUOT10(*source, cut);
        rem = *source - quot*powers[cut];
        next += quot;
      #else
        rem=*source % powers[cut];
        next += *source / powers[cut];
      #endif
      if (target<=first) *target = (DecNumberUnit)next; // write to target iff valid
      next = rem*powers[DECDPUN-cut];          // save remainder for next Unit
   }
   // propagate to one below and clear the rest
   for (; target>=uar; target--) {
      *target = (DecNumberUnit)next;
      next=0;
   }
   return digits+shift;
}

/*------------------------------------------------------------decShiftToLeast-+
|   decShiftToLeast -- shift digits in array towards least significant        |
|                                                                             |
|     uar   is the array                                                      |
|     units is length of the array, in units                                  |
|     shift is the number of digits to remove from the lsu end; it            |
|       must be less than units*DECDPUN.                                      |
|                                                                             |
|   Removed digits are discarded (lost).  Units not required to hold          |
|   the final result are unchanged.                                           |
+----------------------------------------------------------------------------*/
static void decShiftToLeast(DecNumberUnit *uar, int32_t units, int32_t shift) {
   DecNumberUnit *target, *up;      // work
   int32_t cut;                     // work

   up=uar+shift/DECDPUN;            // source; allow for whole Units
   cut=shift%DECDPUN;               // odd 0's to drop
   if (cut==0) {                    // whole units shift
      for (target=uar; up < uar+units; ++target, ++up) *target = *up;

   }else {                          // messier
      int32_t count = units * DECDPUN - shift; // the new length
      int32_t quot, rem;                // for division
      #if DECDPUN<=4
        quot=QUOT10(*up, cut);
      #else
        quot=*up/powers[cut];
      #endif
      for (target=uar; ; ++target) {
         *target = (DecNumberUnit)quot;
         count -= (DECDPUN-cut);
         if (count <= 0) break;
         ++up;
         quot = *up;
         #if DECDPUN<=4
           quot = QUOT10(quot, cut);
           rem = *up - quot*powers[cut];
         #else
           rem= quot % powers[cut];
           quot = quot / powers[cut];
         #endif
         *target += rem * powers[DECDPUN-cut];
         count -= cut;
         if (count <= 0) break;
      }
   }
}

/*------------------------------------------------------------decRoundOperand-+
|   decRoundOperand -- round an operand                                       |
|                                                                             |
|     dn is the number to round (dn->digits is > set->digits)                 |
|     set is the relevant context                                             |
|     status is the status accumulator                                        |
|                                                                             |
|     returns an allocated DecNumber with the rounded result.                 |
|                                                                             |
|   lostDigits and other status may be set by this.                           |
|                                                                             |
|   Since the input is an operand, we are not permitted to modify it.         |
|   We therefore return an allocated DecNumber, rounded as required.          |
|   It is the caller's responsibility to free the allocated storage.          |
|                                                                             |
|   If no storage is available then the result cannot be used, so NULL        |
|   is returned.                                                              |
|                                                                             |
|   We may allocate slightly more than necessary, but no more than one        |
|   Unit excess.                                                              |
+----------------------------------------------------------------------------*/
static DecNumber * decRoundOperand(
   DecNumber const * dn,
   DecContext const * set,
   uint32_t * status
) {
   DecNumber * res;                      // result structure
   int32_t residue = 0;                  // residue form dropped uInts
   DecNumberUnit * target;               // work
   DecNumberUnit const * up;             // ..
   uint32_t rem;                         // ..
   int32_t count, dropped;               // ..

   // Spin up the number, noting residue as we pass, until we get to
   // the Unit with the first discarded digit.
   count = dn->digits - set->digits-1;   // count of residue digits
   for (up=dn->lsu; ; ++up) {
      count -= DECDPUN;
      if (count<0) break;       // we're there
      if (*up!=0) residue=1;    // [slightly wasteful, once set]
   }
   // *up is now the Unit with the most significant discarded digit
   dropped=(up-dn->lsu);       // Units we dropped

   // Allocate storage for the returned DecNumber.  Since we will use
   // this for rounding, we make it big enough to hold the number,
   // starting at the Unit which includes the first discarded digit.
   res = (DecNumber *)malloc(
      sizeof(DecNumber) +(D2U(dn->digits)-1-dropped)*sizeof(DecNumberUnit)
   );
   if (res==NULL) {
      *status |= DEC_Insufficient_storage;
      return 0;
   }
   dropped *= DECDPUN;              // now count of digits we dropped
   // copy across the header and the Units we are preserving
   *res = *dn;
   res->digits -= dropped;            // adjust digits ..
   res->exponent += dropped;          // .. and exponent
   for (target=res->lsu; target < res->lsu+D2U(res->digits); ++target, ++up) {
      *target = *up;
   }
   // if we had residue, or the digit(s) in the bottom part of the
   // (new) lsu are non-0, then we have lostDigits condition
   if (!residue) {
      int32_t cut=res->digits-set->digits;   // [the new discard request]
      #if DECDPUN<=4
         rem= QUOT10(*res->lsu,cut);
         rem = *res->lsu-rem*powers[cut];
       #else
         rem = *res->lsu%powers[cut];
       #endif
   }
   if (residue || rem!=0) {    // lost digits indeed
      // accumulate status
      *status |= (DEC_Lost_digits | DEC_Inexact | DEC_Rounded);
   }
   // Do the rounding, in place, in res
   *status |= decRound(res, set, res->lsu, res->digits, residue);
   // [Round will set Rounded again, and could set Inexact again, but we
   // will have already set it -- so there will be no side-effects]
   // Could return +/- Infinity, too.
   return res;
}

/*-------------------------------------------------------------------decRound-+
|   decRound -- copy and round a number, if necessary                         |
|                                                                             |
|     dn    is the number, with enough space for set->digits digits           |
|     set   is the context [for size and traps/status]                        |
|     lsu   -> lsu of the coefficient to round [may be dn->lsu]               |
|     len   is digits in the coefficient to round [may be dn->digits]         |
|     residue is: 0: the coefficient is exact; no less-significant            |
|                    digits 'out to the right'                                |
|                 1: the coefficient is inexact, and there are some           |
|                    non-zero less-significant 'hidden' digits                |
|                -1: as 1, but the hidden digits are subtractive, that        |
|                    is, of the opposite sign to dn.                          |
|     status is the status accumulator, used instead of set->status if        |
|                a trap is expected                                           |
|                                                                             |
|   The source coefficient for the rounding is described by the lsu           |
|   array and len, and the result is rounded into the number.                 |
|   The coefficient may already be in the number, or it can be an             |
|   external intermediate array.  If it is in the number, lsu must ==         |
|   dn->lsu and len must == dn->digits.                                       |
|                                                                             |
|   Note that the coefficient length (len) may be < set->digits, and          |
|   in this case rounding only takes place if there is a non-0 residue        |
|   (e.g., if residue is -1 and round is DEC_ROUND_DOWN).                     |
|                                                                             |
|   Note also that (only internally, from decNumberRescale) the value         |
|   of set->digits may be less than one, indicating a round to left.          |
|   This routine handles that case correctly; Rescale ensures space.          |
|                                                                             |
|   All fields in dn are updated as required, except that the sign may        |
|   be incorrect after decRound if lsu had leading zeros; in this             |
|   case decFinish should be used to clean up.                                |
|                                                                             |
|   DEC_Rounded status is set if any digits are discarded.                    |
|   DEC_Inexact status is set if any non-zero digits are discarded.           |
+----------------------------------------------------------------------------*/
static uint32_t decRound(
   DecNumber * dn,
   DecContext const * set,
   DecNumberUnit * lsu,
   int32_t len,
   int32_t residue
) {
   uint32_t status = 0;
   int32_t discard;            // number of digits to discard
   int32_t bump;               // 1 if coefficient needs to be incremented
                               // -1 if coefficient needs to be decremented
   uint32_t discard1 = 0;      // first discarded digit
   uint32_t cut;               // cut point in Unit
   DecNumberUnit *up, *target; // work
   uint32_t quot, rem;         // for divisions

   discard = len-set->digits;  // digits to discard
   if (discard <= 0) {         // no digits are being discarded
      status = 0;
      discard = 0;             // [but there might be some residue already]
      // copy the coefficient array to the result number; no shift needed
      up = lsu;
      for (target=dn->lsu; target < dn->lsu+D2U(len); ++target, ++up) {
         *target = *up;
      }
      dn->digits = len;        // set the new length

   }else { // some digits to discard
      // accumulate Rounded status if it's trapped, else apply directly so
      // we avoid call overhead to decContextSetStatus later
      status = DEC_Rounded;

      // Record first discard and note residue in unwanted digits if we
      // don't already have news of residue
      if (discard>len) {       // everything, +1, is being discarded
         // residue is all the number [NB could be all 0s]
         if (residue == 0) for (up=lsu+D2U(len)-1; up >= lsu; up--) {
            if (*up == 0) continue; // still 0s
            residue = 1;
            break;             // no need to check any others
         }
         *dn->lsu = 0;           // .. result will be 0
         dn->digits = 1;
         dn->bits = 0;
         dn->exponent = 0;

     }else {                   // partial discard [most common case]
         // at least the first discarded digit exists
         uint32_t temp;
         int32_t count = 0;
         // spin up the number, noting residue as we pass, until we get to
         // the Unit with the first discarded digit.  When we get there,
         // extract it and remember where we're at
         for (up = lsu; ; ++up) {
            count += DECDPUN;
            if (count >= discard) break;   // full ones all checked
            if (*up != 0) residue = 1;     // [slightly wasteful]
         }
         // here up -> Unit with discarded digit
         cut=discard-(count-DECDPUN)-1;
         if (cut==DECDPUN-1) { // discard digit is at top
            #if DECDPUN<=4
                discard1=QUOT10(*up, DECDPUN-1);
                rem=*up-discard1*powers[DECDPUN-1];
            #else
                rem=*up%powers[DECDPUN-1];
               discard1=*up/powers[DECDPUN-1];
            #endif
            if (rem!=0) residue=1;
            ++up;              // move to next
            cut=0;             // bottom digit of result

         }else {
            // discard digit is in low digits
            if (cut==0) {
               quot=*up;
            }else /* cut>0 */ { // it's not at bottom of Unit
               #if DECDPUN<=4
                  quot=QUOT10(*up, cut);
                  rem=*up-quot*powers[cut];
               #else
                  rem=*up%powers[cut];
                  quot=*up/powers[cut];
                #endif
                if (rem!=0) residue=1;
            }
            // discard digit is now at bottom of quot
            #if DECDPUN<=4
               temp=(quot*6554)>>16;    // fast /10
               // Vowels algorithm here not a win (9 instructions)
               discard1=quot-X10(temp);
               quot=temp;
            #else
               discard1=quot%10;
               quot=quot/10;
            #endif
            ++cut;                     // update cut
         }

         // here: up -> current Unit of the array
         //       cut is the division point for each Unit
         //       quot holds the uncut high-order digits for the current
         //            Unit, unless cut==0 in which case it's still in *up
         // copy the coefficient array to the result number, shifting as we go
         count=set->digits;              // digits to end up with
         if (count<=0) {                 // special for Rescale :-(
            *dn->lsu=0;
            dn->digits=1;
            dn->bits=0;
         }else {                           // shift to least
           // [this is really inlined decShiftToLeast code]
           dn->digits=count;               // set the new length
           if (cut==0) {
              for (target=dn->lsu; target<dn->lsu+D2U(count); ++target, ++up) {
                 *target=*up;
              }
           }else {
              for (target=dn->lsu; ; ++target) {
                 *target=(DecNumberUnit)quot;
                 count-=(DECDPUN-cut);
                 if (count<=0) break;
                 ++up;
                 quot = *up;
                 #if DECDPUN<=4
                    quot=QUOT10(quot, cut);
                    rem=*up-quot*powers[cut];
                 #else
                    rem=quot%powers[cut];
                    quot=quot/powers[cut];
                 #endif
                 *target+=rem*powers[DECDPUN-cut];
                 count-=cut;
                 if (count<=0) break;
              }
           }
        }
        dn->exponent+=discard;            // maintain numerical value
      }
   }

   // here: discard is either >0, or 0 if only discard1 and residue matter

   // Now record inexactitude, if any
   if (discard1 || residue) {  // some non-0 digits being discarded
      // accumulate Inexact status if it's trapped, else apply directly so
      // we avoid call overhead to decContextSetStatus later
      // [Note that we set Rounded again here in case we are a fastpath
      // add or subtract which has residue but no actual rounding]
      status |= (DEC_Inexact | DEC_Rounded);
   }

   if (discard==0 && residue==0) return status; // no rounding can happen

   // now decide whether to round, depending on mode
   // [if add floor/ceiling, these will need to take account of sign, for
   // example: if (dn->bits&DECNEG) bump=-bump; ]
   bump=0;                               // assume no bump in the night
   switch (set->round) {
   case DEC_ROUND_DOWN:
      // no change, unless negative residue on 0
      if (discard1==0 && residue<0) bump=-1;
      break;

   case DEC_ROUND_HALF_DOWN:
      if ((discard1>5) || (discard1==5 && residue>0)) bump=1;
      break;

   case DEC_ROUND_HALF_EVEN:
      if (discard1>5) {                  // >0.5
         bump=1;
      }else if (discard1==5) {
         if (residue>0) {                // 0.5+
            bump=1;
         }else if (residue<0) {          // 0.5-
            ;
         }else {                         // exactly 0.5
            // exactly 0.5000 goes up iff [new] lsd is odd
            if (*dn->lsu&0x01) bump=1;
         }
      }
      break;

   case DEC_ROUND_HALF_UP:
      if ((discard1>5) || (discard1==5 && residue>=0)) bump=1;
      break;

   case DEC_ROUND_UP:
      // always bump, except negative residue on 0
      if (discard1>0 || residue>0) bump=1;
      break;

   case DEC_ROUND_CEILING:
      // same as _UP for positive numbers, and as _DOWN for negatives
      // [negative residue cannot occur on 0]
      if (dn->bits&DECNEG) {
         if (discard1==0 && residue<0) bump=-1;
      }else {
         if (discard1>0 || residue>0) bump=1;
      }
      break;

   case DEC_ROUND_FLOOR:
      // same as _UP for negative numbers, and as _DOWN for positive
      // [negative residue cannot occur on 0]
      if (!(dn->bits&DECNEG)) {
         if (discard1==0 && residue<0) bump=-1;
      }else {
         if (discard1>0 || residue>0) bump=1;
      }
      break;

   default:        // e.g., DEC_ROUND_MAX
      status |= DEC_Invalid_context;
      break;
   }
   // [see BigDecimal for some other possible modes]

   // now bump the number up or down if need be
   if (bump!=0) {
      uint8_t allnines=0;
      uint32_t count=dn->digits;
      // Simply use decUnitAddSub unless we are bumping up and the number
      // is all nines.  In this special case we set to 1000... and adjust
      // the exponent by one (as otherwise we could overflow the array)
      if (bump>0) {
         for (up=dn->lsu; ; ++up) {
            if (count>DECDPUN) {
               if (*up!=DECDPUNMAX) break;
            }else {                         // this is the last Unit
               if (*up==powers[count]-1) {
                  allnines=1;
                  *up=(DecNumberUnit)powers[count-1];  // here 999 -> 100 etc.
                  for (up=up-1; up>=dn->lsu; up--) *up=0; // others all to 0
                  ++dn->exponent;             // and bump exponent
               }
               break;
            }
            count-=DECDPUN;                 // we checked those digits
         }
      }
      if (!allnines) {
         decUnitAddSub(dn->lsu, D2U(dn->digits), one, 1, 0, dn->lsu, bump);
      }
   }

   // finally check for exponent overflow [adjustment could be large]
   if ((dn->exponent + dn->digits) > (int32_t)(set->emax+1)) {
      status |= decSetOverflow(dn, set, dn->bits);
   }
   return status;
}

/*-------------------------------------------------------------decSetOverflow-+
|   decSetOverflow -- set number to proper overflow value                     |
|                                                                             |
|     dn is the number (used as result only)                                  |
|     set is the context [used for the rounding mode]                         |
|     status contains the current status to be updated                        |
|     bits is a flag byte that contains the sign for the result (and          |
|             possibly other bits set)                                        |
|                                                                             |
|   This sets the sign of a number and sets its value to either               |
|   Infinity or the maximum finite value, depending on the sign and           |
|   rounding mode, following IEEE 854 rules.                                  |
+----------------------------------------------------------------------------*/
static uint32_t decSetOverflow(
   DecNumber * dn,
   DecContext const * set,
   uint8_t bits
) {
   uint32_t * status = 0;
   uint8_t needmax = 0;             // result is maximum finite value
   bits &= DECNEG;                  // clean sign bit
   decNumberZero(dn);
   switch (set->round) {
   case DEC_ROUND_DOWN:
      needmax = 1;                  // never Infinity
      break;
   case DEC_ROUND_CEILING:
      if (bits) needmax=1;          // Infinity if non-negative
      break;
   case DEC_ROUND_FLOOR:
      if (!bits) needmax=1;         // Infinity if negative
      break;
   default:                         // Infinity in all other cases
      break;
   }
   if (needmax) {
      DecNumberUnit * up;           // work
      int32_t count=set->digits;    // nines to add
      dn->digits=count;
      // fill in all nines to set maximum value
      for (up=dn->lsu; ; ++up) {
         if (count>DECDPUN) {       // unit full o'nines
            *up = DECDPUNMAX;
         }else {                    // this is the msu
            *up=powers[count]-1;
            break;
         }
         count-=DECDPUN;            // we filled those digits
      }
      dn->bits=bits;                // sign
      dn->exponent = set->emax - set->digits + 1;
   }else {
      dn->bits=bits|DECINF;         // Value is +/-Infinity
   }
   return DEC_Overflow | DEC_Inexact | DEC_Rounded;
}

/*------------------------------------------------------------decSetUnderflow-+
|   decSetUnderflow -- set number to 'hard' underflow value                   |
|                                                                             |
|     dn is the number (used as input as well as output; it may have          |
|           an allowed subnormal value)                                       |
|     set is the context [used for the rounding mode]                         |
|     status contains the current status to be updated                        |
|                                                                             |
|   Unless the number is a subnormal (and subnormals are allowed), the        |
|   number is set to zero (preserving sign if extended values is 1).          |
+----------------------------------------------------------------------------*/
static uint32_t decSetUnderflow(DecNumber *dn, DecContext const *set)
{
   // allow subnormals if extended ('soft underflow')
   if (
      (!set->flags & DEC_EXTENDED) ||
      ((dn->exponent + (int32_t)set->digits) <= -(int32_t)set->emax)
   ) {                                  // not a subnormal, or not extended
      uint8_t bits = dn->bits;          // save
      decNumberZero(dn);                // force to zero ('hard underflow')
      if ((set->flags & DEC_EXTENDED)) dn->bits = bits & DECNEG;
   }
   // always flag the overflow
   return DEC_Underflow | DEC_Inexact | DEC_Rounded;
}

/*------------------------------------------------------------------decGetInt-+
|   decGetInt -- get integer from a number                                    |
|                                                                             |
|     dn is the number [which will not be altered]                            |
|     set is the context [requested digits]                                   |
|     returns the converted integer, or DEC_BADINT if error                   |
|                                                                             |
|   This checks and gets a whole number from the input decNumber.             |
|   The magnitude of the integer must be <2^31.                               |
|   Any discarded fractional part must be 0.                                  |
|   It must also fit in set->digits                                           |
+----------------------------------------------------------------------------*/
int32_t decGetInt(DecNumber const * dn, DecContext const * set)
{
   int32_t ilength = dn->digits+dn->exponent; // integral length

   // The number must be an integer that fits in 10 digits
   if ((ilength > (int32_t)set->digits) || (ilength > 10)) {
      return DEC_BADINT;
   }else if (ISZERO(dn)) {                    // zeros are OK, with any exponent
      return 0;
   }else {
      int32_t got;                            // digits (real or not) processed
      DecNumberUnit const * up = dn->lsu;     // ready for lsu
      int32_t theInt = 0;                     // ready to accumulate
      if (dn->exponent >=0 ) {                // relatively easy
         // no fractional part [usual]; allow for positive exponent
         got = dn->exponent;
      }else {
         // -ve exponent; some fractional part to check and discard
         int32_t count;                       // digits to discard
         // spin up whole units until we get to the Unit with the unit digit
         for (count = -dn->exponent; count >= DECDPUN; ++up) {
            if (*up!=0) return DEC_BADINT;    // non-zero Unit to discard
            count -= DECDPUN;
         }
         if (count==0) {                      // [a multiple of DECDPUN]
            got = 0;
         }else {                              // [not multiple of DECDPUN]
            int32_t rem;                      // work
            // slice off fraction digits and check for non-zero
            #if DECDPUN <= 4
               theInt = QUOT10(*up, count);
               rem = *up - (theInt * powers[count]);
            #else
               rem = *up % powers[count];     // slice off discards
               theInt = *up / powers[count];
            #endif
            if (rem!=0) return DEC_BADINT;    // non-zero fraction
            // OK, we're good
            got = DECDPUN - count;            // number of digits so far
            ++up;                             // ready for next
         }
      }
      // collect the rest
#if 0 // Mike's solution -- not sure of.  Doesn't bring much, tho.
      for (; got < ilength; ++up) {
         theInt += *up * powers[got];
         got += DECDPUN;
      }
      if (
         (ilength == 10) &&                   // check no wrap
         ((theInt / (signed)powers[got-DECDPUN]) != *(up-1))
      ) {
         return BADINT;
      }
#else
      if (ilength == 10) {
         ilength -= DECDPUN;
         for (; got < ilength; ++up) {
            theInt += *up * powers[got];
            got += DECDPUN;
         }
         int maxLsu = INT_MAX / powers[got];
         if (
            (*up > maxLsu) ||
            ((*up == maxLsu) && ((uint32_t)theInt > (INT_MAX % powers[got])))
         ) {
            return DEC_BADINT;
         }
         theInt += *up * powers[got];
      }else {
         for (; got < ilength; ++up) {
            theInt += *up * powers[got];
            got += DECDPUN;
         }
      }
#endif
      // apply any sign and return
      return (dn->bits & DECNEG)? -theInt : theInt;
   }
}

/*---------------------------------------------------------------decGetDigits-+
|    decGetDigits -- count digits in a Units array                            |
|                                                                             |
|      uar is the Unit array holding the number [this is often an             |
|             accumulator of some sort]                                       |
|      len is the length of the array in units                                |
|                                                                             |
|      returns the number of (significant) digits in the array                |
|                                                                             |
|    All leading zeros are excluded, except the last if the array has         |
|    only zero Units.                                                         |
|                                                                             |
|    This may be called twice during some operations.                         |
+----------------------------------------------------------------------------*/
static int32_t decGetDigits(DecNumberUnit *uar, int32_t len)
{
   DecNumberUnit * up = uar + len - 1; // -> msu
   int32_t  digits = len*DECDPUN;      // maximum possible digits
   uint32_t const * pow;               // work

   for (; up>=uar; --up) {
      digits -= DECDPUN;
      if (*up == 0) {               // unit is 0
         if (digits == 0) {         // all units were 0
            ++digits;               // .. so bump digits to 1
            break;
         }
         continue;
      }
      // found a non-zero Unit
      ++digits;
      if (*up < 10) break;          // fastpath 1-9
      ++digits;
      for (pow=&powers[2]; *up >= *pow; ++pow) ++digits;
      break;
   }
   return digits;
}

/*--------------------------------------------------------------------decTrim-+
|  decTrim -- trim trailing zeros or normalize                                |
|                                                                             |
|    dn is the number to trim or normalize                                    |
|    all is 1 to remove all trailing zeros, 0 for just fraction ones          |
|    returns dn                                                               |
|                                                                             |
|  All fields are updated as required.  This is a utility operation,          |
|  so special values are unchanged and no error is possible.                  |
+----------------------------------------------------------------------------*/
static void decTrim(DecNumber *dn, uint8_t all)
{
   int32_t d, exp;                  // work
   uint32_t  cut;                   // ..
   DecNumberUnit * up;              // -> current Unit

   if ((dn->bits & DECSPECIAL) || (*dn->lsu & 0x01) || ISZERO(dn)) {
      return;                       // fast exit if special or odd, or 0
   }
   // we have a finite number which is even
   exp=dn->exponent;
   cut=1;                           // digit (1-DECDPUN) in Unit
   up=dn->lsu;                      // -> current Unit
   for (d=0; d<dn->digits-1; ++d) { // [don't strip the final digit]
      // slice by powers
      #if DECDPUN<=4
         uint32_t quot = QUOT10(*up, cut);
         if ((*up-quot*powers[cut])!=0) break;  // found non-0 digit
      #else
         if (*up%powers[cut]!=0) break;         // found non-0 digit
      #endif
      // have a trailing 0
      if (!all) {                    // trimming
         // [if exp>0 then all trailing 0s are insignificant]
         if (exp <= 0) {             // if digit might be significant
            if (exp == 0) break;     // then quit
            ++exp;                   // next digit might be significant
         }
      }
      ++cut;                         // next power
      if (cut > DECDPUN) {           // need new Unit
        ++up;
        cut=1;
      }
   }
   if (d==0) return;                // none dropped
   // effect the drop
   decShiftToLeast(dn->lsu, D2U(dn->digits), d);
   dn->exponent += d;               // maintain numerical value
   dn->digits -= d;                 // new length
}

/*------------------------------------------------------------------decFinish-+
|   decFinish -- finish processing a number                                   |
|                                                                             |
|     dn is the number                                                        |
|     set is the context                                                      |
|     status is the accumulator                                               |
|                                                                             |
|   This finishes off the current number by:                                  |
|      1. Converting a zero result to clean '0' (unless extended)             |
|      2. Reducing positive exponents to 0, if would fit in digits            |
|      3. Checking for overflow and underflow                                 |
|   All fields are updated as required.                                       |
+----------------------------------------------------------------------------*/
static uint32_t decFinish(DecNumber * dn, DecContext const * set)
{
   if ISZERO(dn) {                  // value is zero
      dn->exponent=0;               // clean exponent
      if (!(set->flags & DEC_EXTENDED)) {
         dn->bits=0;                // 'clean' -0 sign if not extended
      }                             // base specification; no -0
      return 0;

   }else if (dn->exponent >= 0) {   // non-negative exponent
      // fast path if already an integer [safety checked at compile time]
      #if (DEC_MAX_DIGITS-1 > DEC_MAX_EMAX)
      #error Invalid fastpath in decFinish needs updating
      #endif
      if (dn->exponent == 0) {      // fastpath; extremely common
         return 0;
      }

      // >0; reduce to integer if possible
      if ((int32_t)set->digits >= (dn->exponent + dn->digits)) {
         dn->digits = decShiftToMost(dn->lsu, dn->digits, dn->exponent);
         dn->exponent = 0;
         return 0;                  // exponent is now known to be 0
      }

      // Check the exponent.  The exponent is positive, so we have to be
      // careful as the adjusted exponent could overflow 31 bits
      // [because it may already be up to twice the expected]
      if (dn->exponent > (int32_t)set->emax + 1 - dn->digits) {
         return decSetOverflow(dn, set, dn->bits);
      }
   }else {
      // exponent is negative; check the adjusted exponent
      if ((dn->exponent + dn->digits) <= -(int32_t)set->emax) {
         return decSetUnderflow(dn, set);
      }
   }
   return 0;
}

/*===========================================================================*/
