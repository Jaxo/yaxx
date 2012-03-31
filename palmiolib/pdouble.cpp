/***********************************************************************
 pdouble.c - Contains a routine to print double-precision floating
    point numbers and associated routines, written by Fred Bayer,
    author of LispMe.  It is available from
    http://www.lispme.de/lispme/gcc/tech.html

 This version was formatted and tweaked slightly by Warren Young
    <tangent@cyberport.com>

 This routine is in the Public Domain.

 Code edited 2001-01-14 by Ben Combee <bcombee@metrowerks.com> to
 work with CodeWarrior for Palm OS 7 and 8.
***********************************************************************/

#include <PalmOS.h>
#include "pdouble.h"

// Formatting parameters
#define NUM_DIGITS   15
#define MIN_FLOAT    4
#define ROUND_FACTOR 1.0000000000000005 /* NUM_DIGITS zeros */


// FP conversion constants
static double pow1[] = {
   1e256, 1e128, 1e064,
   1e032, 1e016, 1e008,
   1e004, 1e002, 1e001
};

static double pow2[] = {
   1e-256, 1e-128, 1e-064,
   1e-032, 1e-016, 1e-008,
   1e-004, 1e-002, 1e-001
};

/*----------------------------------------------------------------printDouble-+
|                                                                             |
+----------------------------------------------------------------------------*/
void printDouble(double d, char *s)
{
   FlpCompDouble fcd;

   // Round to desired precision (doesn't always provide correct last digit!)
   d = d * ROUND_FACTOR;

   // check for NAN, +INF, -INF, 0
   fcd.d = d;
   if ((fcd.ul[0] & 0x7ff00000) == 0x7ff00000) {
      if ((fcd.fdb.manH == 0) && (fcd.fdb.manL == 0)) {
         if (fcd.fdb.sign) {
            StrCopy(s, "[-inf]");
         }else {
            StrCopy(s, "[inf]");
         }
      }else {
         StrCopy(s, "[nan]");
      }
   }else if (FlpIsZero(fcd)) {
      *s = '0';
      *(s+1) = '\0';
   }else {
      short exp;
      char sign;
      short decpos;

      if (FlpGetSign(fcd)) {    // store sign, and make positive
         *s++ = '-';
         FlpSetPositive(fcd);
      }
      if ((unsigned)fcd.fdb.exp < 0x3ff) { // negative exponent
         short e1;
         double *pd, *pd1;
         sign = '-';
         for (
            exp=1, e1=256, pd=pow1, pd1=pow2;
            e1;
            e1 >>= 1, ++pd, ++pd1
         ) {
            if (*pd1 > fcd.d) {
               exp += e1;
               fcd.d = fcd.d * *pd;
            }
         }
         fcd.d = fcd.d * 10.0;
         if (exp > MIN_FLOAT) {
            decpos = 0;      // use exponential notation
         }else {
            *s++ = '0';
            *s++ = '.';
            while (--exp) *s++ = '0';
            decpos = -1;
         }
      }else {                // positive exponent
         short e1;
         double *pd, *pd1;
         sign = '+';
         for (
            exp=0, e1=256, pd=pow1, pd1=pow2;
            e1;
            e1 >>= 1, ++pd, ++pd1
         ) {
           if (*pd <= fcd.d) {
              exp += e1;
              fcd.d = fcd.d * *pd1;
           }
         }
         if (exp >= NUM_DIGITS) {
            decpos = 0;      // use exponential notation
         }else {
            decpos = exp;
            exp = 0;         // use float notation
         }
      }

      // output mantissa
      for (short i=0; i < NUM_DIGITS; ++i) {
         Int32 d = fcd.d;
         *s++ = d + '0';
         if (i == decpos) *s++ = '.';
         fcd.d = (fcd.d - (double)d) * 10.0;
      }

      // remove trailing zeros and decimal point
      
      //these two lines of code are not the same as the ones that follow
      //why? these leave s pointing to a valid char,
      //and those leave s pointing to null,
      //which is a critical difference for the "append exponent"
      //code that follows (which could be changed to use *++s 
      //if you want to keep your code, otherwise it lops off
      //the last digit)
      //<<<PGR? while ((*--s) == '0') {}
      //<<<PGR? if (*s == '.') --s;
      while (s[-1] == '0') *--s = '\0';
      if (s[-1] == '.') *--s = '\0';

      // append exponent
      if (exp) {
         *s++ = 'e';
         *s++ = sign;
         StrIToA(s, exp);
      }else {
         *s = '\0';
      }
   }
}

/*===========================================================================*/
