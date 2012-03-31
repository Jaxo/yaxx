/* $Id: DecNumber.h,v 1.21 2002-11-12 15:47:31 pgr Exp $
*
* Decimal Number arithmetic module header
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
*/

#ifndef COM_DECNBLIB_DECNUMBER_H_INCLUDED
#define COM_DECNBLIB_DECNUMBER_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include "decnbgendef.h"
#include "stdint.h"              // C99 standard integers

#define DECNAME     "DecNumber"                  // Short name
#define DECVERSION  "DecNumber 2.11"             // Version [16 max.]
#define DECFULLNAME "Decimal Number Module"      // Verbose name
#define DECAUTHOR   "Mike Cowlishaw"             // Who to blame

/*-------------------------------------------------------- struct DecContext -+
| Context for operations, with associated constants                           |
|                                                                             |
| Context must always be set correctly:                                       |
|                                                                             |
|  digits   -- must be in the range 1 through 999999999                       |
|  emax     -- must be in the range 0 through 999999999                       |
|  round    -- must be one of the enumerated rounding modes                   |
|  flags    -- must be zero, possibly ORed with one of the flags values       |
+----------------------------------------------------------------------------*/
enum rounding {
   DEC_ROUND_CEILING,            // round towards +infinity
   DEC_ROUND_UP,                 // round away from 0
   DEC_ROUND_HALF_UP,            // 0.5 rounds up
   DEC_ROUND_HALF_EVEN,          // 0.5 rounds to nearest even
   DEC_ROUND_HALF_DOWN,          // 0.5 rounds down
   DEC_ROUND_DOWN,               // round towards 0 (truncate)
   DEC_ROUND_FLOOR,              // round towards -infinity
   DEC_ROUND_MAX                 // enum must be less than this
};

struct DecContext {
   uint32_t digits;              // working precision
   uint32_t emax;                // maximum positive exponent
   enum rounding round;          // rounding mode
   uint8_t flags;                // flag: special-values allowed
};

/* flags values */
#define DEC_EXTENDED         0x01
#define DEC_FORM_ENGINEERING 0x02  // else, scientific

/* Maxima and Minima */
#define DEC_MAX_DIGITS 999999999
#define DEC_MAX_EMAX   999999999
#define DEC_MIN_DIGITS         1
#define DEC_MIN_EMAX           0

/*--------------------------------------------------------- struct DecNumber -+
| Define the DecNumber data structure.                                        |
|                                                                             |
| It is assumed that the structure will be immediately followed               |
| by extra space (if required) so it can hold all the digits                  |
| inherent to the number.                                                     |
|                                                                             |
| DECDPUN is the number of Decimal Digits Per UNit.  It must be in            |
| the range 1-9; a power of 2 recommended].   Default is 4.                   |
|                                                                             |
| Notes:                                                                      |
| 1. If digits is > DECDPUN then there will be more than one                  |
|    decNumberUnits immediately following the first element of lsu.           |
|    These contain the remaining (more significant) digits of the             |
|    number, and may be in the lsu array, or may be guaranteed by             |
|    some other mechanism (such as being contained in another                 |
|    structure, or being overlaid on dynamically allocated storage).          |
|                                                                             |
|    Each integer of the coefficient (except the possibly the last)           |
|    contains DECDPUN digits (e.g., a value in the range 0 through            |
|    99999999 if DECDPUN is 8, or 0 through 9999 if DECDPUN is 4).            |
|                                                                             |
| 2. A DecNumber converted to a string may need up to digits+14               |
|    characters.  The worst cases (non-exponential and exponential            |
|    formats) are: -0.00000{9...}#                                            |
|             and: -9.{9...}E+999999999#   (where # is '\0')                  |
|                                                                             |
+----------------------------------------------------------------------------*/
#define DECDPUN 4             // Decimal Digits Per UNit

#if   DECDPUN <= 2
  #define DecNumberUnit uint8_t
#elif DECDPUN <= 4
  #define DecNumberUnit uint16_t
#else
  #define DecNumberUnit uint32_t
#endif

struct DecNumber {
  int32_t digits;             // Count of digits in the coefficient; >0
  int32_t exponent;           // Unadjusted exponent, unbiased, in
                              // range: -1999999997 through 999999999
  uint16_t iReferenceCount;   // for magic memory
  uint16_t m_maxUnits;        // count of allocated lsu units
  uint8_t bits;               // Indicator bits (see below)
  uint8_t bits2;              // extra bits (see below)
  DecNumberUnit lsu[1];       // Coefficient, from least significant unit
};

// Bit settings for DecNumber.bits
#define DECNEG    0x80        // Sign; 1=negative, 0=positive or zero
#define DECINF    0x40        // 1=Infinity
#define DECNAN    0x20        // 1=NaN
#define DECSNAN   0x10        // 1=sNaN
// The remaining bits are reserved; they must be 0
#define DECSPECIAL (DECINF|DECNAN|DECSNAN) // any special value

// Bit settings for DecNumber.bits2
#define DECENGINE 0x01        // Engineering (else Scientific)
#define DECSUBN   0x02        // Subnormal
#define DECZNEG   0x04        // A negative zero (in extended mode)
// The remaining bits are reserved; they must be 0

#define DECTOSTR2_ENG              1 << 0
#define DECTOSTR2_EXPFORM_ALLOWED  1 << 1
#define DECTOSTR2_EXPFORM_FORCED   1 << 2


/*----------------------------------------------------------------------------+
| Status (uint32_t returned by most routines)                                 |
+----------------------------------------------------------------------------*/

/* Status flags (exceptional conditions), and their names */
#define DEC_Conversion_overflow  0x0001
#define DEC_Conversion_syntax    0x0002
#define DEC_Conversion_underflow 0x0004
#define DEC_Division_by_zero     0x0008
#define DEC_Division_impossible  0x0010
#define DEC_Division_undefined   0x0020
#define DEC_Insufficient_storage 0x0040   // [used if malloc fails]
#define DEC_Inexact              0x0080
#define DEC_Invalid_context      0x0100
#define DEC_Invalid_operation    0x0200
#define DEC_Lost_digits          0x0400
#define DEC_Overflow             0x0800
#define DEC_Rounded              0x1000
#define DEC_Underflow            0x2000

/* IEEE 854 groupings for the flags [DEC_Rounded is not in IEEE 854] */
#define DEC_IEEE_854_Division_by_zero  (DEC_Division_by_zero)
#define DEC_IEEE_854_Inexact           (DEC_Inexact | DEC_Lost_digits)
#define DEC_IEEE_854_Invalid_operation ( \
   DEC_Conversion_syntax    |            \
   DEC_Division_impossible  |            \
   DEC_Division_undefined   |            \
   DEC_Insufficient_storage |            \
   DEC_Invalid_context      |            \
   DEC_Invalid_operation                 \
)
#define DEC_IEEE_854_Overflow (  \
   DEC_Overflow |                \
   DEC_Conversion_overflow       \
)
#define DEC_IEEE_854_Underflow ( \
   DEC_Underflow |               \
   DEC_Conversion_underflow      \
)

// flags which are normally errors (results are qNaN, infinite, or 0)
#define DEC_Errors (                 \
   DEC_IEEE_854_Division_by_zero  |  \
   DEC_IEEE_854_Invalid_operation |  \
   DEC_IEEE_854_Overflow          |  \
   DEC_IEEE_854_Underflow            \
)

// flags which cause a result to become qNaN
#define DEC_NaNs DEC_IEEE_854_Invalid_operation

// flags which are normally for information only (have numerical results)
#define DEC_Information (DEC_Rounded | DEC_Inexact | DEC_Lost_digits)

/*----------------------------------------------------------------------------+
| DecNumber public routines                                                   |
+----------------------------------------------------------------------------*/

// Base specification -- conversions
char * decNumberToString(
   DecNumber const *,
   char *
);
char * decNumberToEngString(
   DecNumber const *,
   char *
);
char * decNumberToString2(
   DecNumber const *,
   char *,
   uint8_t flag
);

// Base specification -- operators
uint32_t decNumberAbs(
   DecNumber *,
   DecNumber const *,
   DecContext const *
);
uint32_t decNumberAdd(
   DecNumber *,
   DecNumber const *,
   DecNumber const *,
   DecContext const *
);
uint32_t decNumberCompare(
   DecNumber *,
   DecNumber const *,
   DecNumber const *,
   DecContext const *
);
uint32_t decNumberDivide(
   DecNumber *,
   DecNumber const *,
   DecNumber const *,
   DecContext const *
);
uint32_t decNumberDivideInteger(
   DecNumber *,
   DecNumber const *,
   DecNumber const *,
   DecContext const *
);
uint32_t decNumberMax(
   DecNumber *,
   DecNumber const *,
   DecNumber const *,
   DecContext const *
);
uint32_t decNumberMin(
   DecNumber *,
   DecNumber const *,
   DecNumber const *,
   DecContext const *
);
uint32_t decNumberMultiply(
   DecNumber *,
   DecNumber const *,
   DecNumber const *,
   DecContext const *
);
uint32_t decNumberPower(
   DecNumber *,
   DecNumber const *,
   DecNumber const *,
   DecContext const *
);
uint32_t decNumberRemainder(
   DecNumber *,
   DecNumber const *,
   DecNumber const *,
   DecContext const *
);
uint32_t decNumberRescale(
   DecNumber *,
   DecNumber const *,
   DecNumber const *,
   DecContext const *
);
uint32_t decNumberSquareRoot(
   DecNumber *,
   DecNumber const *,
   DecContext const *
);
uint32_t decNumberSubtract(
   DecNumber *,
   DecNumber const *,
   DecNumber const *,
   DecContext const *
);

// Extended specification -- operators
uint32_t decNumberToInteger(
   DecNumber *,
   DecNumber const *,
   DecContext const *
);
uint32_t decNumberRemainderNear(
   DecNumber *,
   DecNumber const *,
   DecNumber const *,
   DecContext const *);

// Extras -- utilities
void decNumberCopy(
   DecNumber *,
   DecNumber const *
);
char * decNumberVersion();
void decNumberZero(DecNumber *);

#endif
/*===========================================================================*/
