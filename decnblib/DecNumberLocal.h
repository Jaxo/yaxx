/* $Id: DecNumberLocal.h,v 1.5 2002-10-21 09:41:35 pgr Exp $
*
* Decimal Number arithmetic module local header
*
* Copyright (c) IBM Corporation, 2000, 2002.  All rights reserved.
*
* This software is made available under the terms of the IBM
* alphaWorks License Agreement (distributed with this software as
* alphaWorks-License.txt).  Your use of this software indicates
* your acceptance of the terms and conditions of that Agreement.
*
* The description and User's Guide ("The decNumber C Library") for
* this software is included in the package as decNumber.pdf.  This
* document is also available in HTML, together with specifications,
* testcases, and Web links, at: http://www2.hursley.ibm.com/decimal
*
* Please send comments, suggestions, and corrections to the author:
*   mfc@uk.ibm.com
*   Mike Cowlishaw, IBM Fellow
*   IBM UK, PO Box 31, Birmingham Road, Warwick CV34 5JL, UK
*
* This header file should only be included by C modules of the DecNumber
* library, and contains local type definitions, tuning parameters,
* etc.  DecNumber.h must be included first.
*/

#ifndef COM_DECNBLIB_DECNUMBERLOCAL_H_INCLUDED
#define COM_DECNBLIB_DECNUMBERLOCAL_H_INCLUDED

/* ----- Shared data ----- */
// The powers of of ten array (powers[n]==10**n, 0<=n<=10)
extern uint32_t const powers[];


/* decNumber modules do not use int or long directly */
/* Development use defines */
// #define  int ?           // enable to ensure we do not use plain C
// #define  long ??         // 'int' or 'long' types from here on

#define DEC_BADINT (signed)0x80000000  // most-negative int; error indicator

/* Set DECDPUNMAX -- the maximum integer that fits in DECDPUN digits */
#if   DECDPUN==1
  #define DECDPUNMAX 9
#elif DECDPUN==2
  #define DECDPUNMAX 99
#elif DECDPUN==3
  #define DECDPUNMAX 999
#elif DECDPUN==4
  #define DECDPUNMAX 9999
#elif DECDPUN==5
  #define DECDPUNMAX 99999
#elif DECDPUN==6
  #define DECDPUNMAX 999999
#elif DECDPUN==7
  #define DECDPUNMAX 9999999
#elif DECDPUN==8
  #define DECDPUNMAX 99999999
#elif DECDPUN==9
  #define DECDPUNMAX 999999999
#endif

/* ----- Macros ----- */
// ISZERO -- return true if decNumber dn is a zero
// [performance-critical in some situations]
#define ISZERO(dn) (*(dn)->lsu==0 \
                    && (dn)->digits==1 \
                    && (((dn)->bits&DECSPECIAL)==0))

// X10 -- multiply integer i by 10
// [shifts are usually faster than multiply; could be conditional]
#define X10(i) (((i)<<1)+((i)<<3))

// D2U -- return the number of Units needed to hold d digits
#if DECDPUN==8
  #define D2U(d) ((unsigned)((d)+7)>>3)
#elif DECDPUN==4
  #define D2U(d) ((unsigned)((d)+3)>>2)
#else
  #define D2U(d) (((d)+DECDPUN-1)/DECDPUN)
#endif

// local constants

enum DecOperation {
   DEC_OP_DIVIDE    = 0x80,   // Divide operators
   DEC_OP_REMAINDER = 0x40,   // ..
   DEC_OP_DIVIDEINT = 0x20,   // ..
   DEC_OP_REMNEAR   = 0x10    // ..
};

#define COMPARE   0x01        // Compare operators
#define COMPMAX   0x02        // ..
#define COMPMIN   0x03        // ..

// Local routines

uint32_t decAddOp(
   DecNumber *,
   DecNumber const *,
   DecNumber const *,
   DecContext const *,
   uint8_t
);

uint32_t decDivideOp(
   DecNumber *,
   DecNumber const *,
   DecNumber const *,
   DecContext const *,
   DecOperation
);

uint32_t decCompareOp(
   DecNumber *,
   DecNumber const *,
   DecNumber const *,
   DecContext const *,
   uint8_t
);

int32_t decGetInt(
   DecNumber const *,
   DecContext const *
);

int decGetWhole(
   DecNumber const *
);

#endif
/*===========================================================================*/
