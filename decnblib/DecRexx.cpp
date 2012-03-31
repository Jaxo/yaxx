/* $Id: DecRexx.cpp,v 1.9 2003-01-22 08:02:56 pgr Exp $ */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "DecRexx.h"
#include "DecNumberLocal.h"

#if !defined COM_JAXO_YAXX_DENY_MATH
  #if defined __MWERKS__ && !defined DBL_DIG && defined __MC68K__
  #define DBL_DIG 15
  #endif
  #if !defined DBL_DIG
  #error Unknown default precision for native double on this processor
  #endif
#endif

/*
| For Rexx arithmetic, see pp. 115-123.
*/

DecRexx::Rep DecRexx::Rep::NilRep;
DecRexx::Rep DecRexx::Rep::BadRep;
DecRexx::Rep::Init const DecRexx::Rep::init;
DecRexx const DecRexx::Nil;         // *not* refcounted

static inline bool isSpace(char c) {
   return ((c <=  ' ') && ((c==' ')||(c=='\t')));
}

/*---------------------------------------------DecRexxContext::DecRexxContext-+
|                                                                             |
+----------------------------------------------------------------------------*/
inline DecRexxContext::DecRexxContext(int digitsPrec) : m_fuzz(0) {
   digits= digitsPrec;
   emax = DecRexxContext::DEFAULT_EMAX;
   round = DEC_ROUND_HALF_UP;
   flags = 0;
}

/*----------------------------------------------------------DecRexx::Rep::Rep-+
| Nil constructor                                                             |
+----------------------------------------------------------------------------*/
DecRexx::Rep::Rep()
{
   decNumberZero(this);
   m_maxUnits = 0;
   bits = DECNAN;
   bits2 = 0;
   iReferenceCount = 0xFFFF;   // to recognize it while debugging
}

/*----------------------------------------------------------DecRexx::Rep::Rep-+
|                                                                             |
+----------------------------------------------------------------------------*/
inline DecRexx::Rep::Rep(int maxUnits)
{
   m_maxUnits = maxUnits;
   bits2 = 0;
   iReferenceCount = 1;
}

/*STATIC--------------------------------------------------DecRexx::Rep::alloc-+
|                                                                             |
+----------------------------------------------------------------------------*/
inline DecRexx::Rep * DecRexx::Rep::alloc(int units) {
   return new(
      ::new char[sizeof (Rep) + ((units - 1) * sizeof (DecNumberUnit))]
   ) Rep(units);
}

/*STATIC------------------------------------------------DecRexx::Rep::dealloc-+
|                                                                             |
+----------------------------------------------------------------------------*/
inline void DecRexx::Rep::dealloc(Rep * p) {
   delete [] (char *)p;
}

/*STATIC------------------------------------------------DecRexx::Rep::makeRep-+
|                                                                             |
+----------------------------------------------------------------------------*/
DecRexx::Rep * DecRexx::Rep::makeRep(char const * pSource)
{
   bool isNegative = false;
   char const * pFirst;     // pointing at first non zero digit
   char const * pDecimal;   // pointing after the decimal dot (if any)
   char const * pLast;      // pointing after the last digit
   int digits;
   int exponent;
   // int status;

   if (*pSource == ' ')  while (isSpace(*++pSource)) {} // skip leading spaces
   switch (*pSource) {
   case '-':                         // allow a sign to occur
      isNegative = true;
      // fall thru
   case '+':
      while (isSpace(*++pSource)) {} // skip intermediate spaces
      break;
   default:
      break;
   }

   // go over the digits in the integer part (set pFirst and pDecimal)
   if ((*pSource <= '9') && (*pSource >= '0')) {
      if (*pSource == '0') {
         while (*++pSource == '0') {};  // leading zeroes
         if ((*pSource <= '9') && (*pSource >= '0')) {
            pFirst = pSource;
         }else {
            pFirst = 0;
            --pSource;
         }
      }else {
         pFirst = pSource;
      }
      while ((*++pSource <= '9') && (*pSource >= '0')) {}
      if (*pSource == '.') pDecimal = ++pSource; else pDecimal = 0;

   }else if ((*pSource == '.') && (*++pSource <= '9') && (*pSource >= '0')) {
      pFirst = 0;
      pDecimal = pSource;

   }else {
      // status = DEC_Conversion_syntax;
      return &Rep::BadRep;
   }

   // go over the digits in the fraction part (if any)
   if (pDecimal) {
      if (!pFirst) {
         while ((*pSource <= '9') && (*pSource >= '0')) {
            if ((*pSource != '0') && (!pFirst)) pFirst = pSource;
            ++pSource;
         }
         digits = pSource - pFirst;  // yes: pFirst can be 0
      }else {
         while ((*pSource <= '9') && (*pSource >= '0')) ++pSource;
         digits = pSource - pFirst - 1;
      }
      exponent = pDecimal - pSource;
   }else {
      digits = pSource - pFirst;     // yes: pFirst can be 0
      exponent = 0;
   }
   pLast = pSource;

   // adjust the exponent
   if ((*pSource | 0x20) == 'e') {   // now process explicit exponent
      char const * pTemp;
      int expVal = 0;
      bool isNegativeExp;
      switch (*++pSource) {          // to (expected) sign
      case '-':
         isNegativeExp = true;
         ++pSource;
         break;
      case '+':
         isNegativeExp = false;
         ++pSource;
         break;
      case '\0':
         // status = DEC_Conversion_syntax;
         return &Rep::BadRep;
      default:
         isNegativeExp = false;
         break;
      }
      pTemp = pSource;
      while ((*pSource <= '9') && (*pSource >= '0')) {
         expVal = (((expVal)<<1)+((expVal)<<3)) + (*pSource++ - '0');
      }
      if (pTemp == pSource) {        // no digits follows
         --pSource;                  // will trigger an error
      }else if (isNegativeExp) {
         if (pSource < (pTemp + 10)) {
            exponent -= expVal;
         }else {                     // was too big
            exponent = -(DEC_MAX_EMAX*2);
         }
      }else {
         if (pSource < (pTemp + 10)) {
            exponent += expVal;
         }else {                     // was too big
            exponent = DEC_MAX_EMAX*2;
         }
      }
   }

   if (*pSource == ' ')  while (isSpace(*++pSource)) {} // skip trailing spaces
   if (*pSource != '\0') {
      // status = DEC_Conversion_syntax;
      return &Rep::BadRep;
   }

   if (!pFirst) {  // rapid exit for zeros [exponent range ignored]
      Rep * pRep = isNegative? init.pMinusZeroRep : init.pPlusZeroRep;
      ++pRep->iReferenceCount;
      // status = 0;
      return pRep;
   }

   if (
      (digits > DEC_MAX_DIGITS) ||
      ((digits + exponent -1) > DEC_MAX_EMAX) ||
      ((digits + exponent -1) < -DEC_MAX_EMAX)
   ) {
      Rep * pRep = alloc(D2U(1));
      decNumberZero(pRep);                             // clean result
      if ((digits+exponent-1) < -DEC_MAX_EMAX) {
         // status = DEC_Conversion_underflow | DEC_Inexact | DEC_Rounded;
         pRep->bits2 = DECSUBN;
         if (isNegative) pRep->bits2 |= DECZNEG;
      }else {
         // status = DEC_Conversion_overflow | DEC_Inexact | DEC_Rounded;
         pRep->bits = DECINF;
         if (isNegative) pRep->bits |= DECNEG;
      }
      return pRep;
   }else {
      // The string is OK.  Allocate the required space and fill members.
      Rep * pRep = alloc(D2U(digits));
      if (!pRep) {
         // status = DEC_Insufficient_storage;
         return &Rep::BadRep;
      }
      pRep->digits = digits;
      pRep->bits = isNegative? DECNEG : 0;
      pRep->exponent = exponent;

      #if DECDPUN>1
      DecNumberUnit * up = pRep->lsu + pRep->m_maxUnits;
      int i = ((digits+DECDPUN-1) % DECDPUN); // digits in top (last) unit
      int curLsu = (*pFirst -'0');
      for (;;) {
         if (*++pFirst != '.') {     // ignore . [don't decrement i]
            if (i) {                 // do more for this Unit
               curLsu = X10(curLsu) + (*pFirst -'0');
               --i;
            }else {                  // done enough for this Unit
               *(--up) = curLsu;
               if (pFirst >= pLast) break;
               i = DECDPUN-1;
               curLsu = (*pFirst -'0');
            }
         }
      }
      #else
      while (--pLast >= pFirst) {
         if (*pLast !='.') *up++ = *pLast - '0';
      }
      #endif
//    status = 0;
      return pRep;
   }
}

/*STATIC------------------------------------------------DecRexx::Rep::makeRep-+
|                                                                             |
+----------------------------------------------------------------------------*/
DecRexx::Rep * DecRexx::Rep::makeRep(signed int val)
{
   if (val < 0) {
      if (val == -1) {    // do not clobber -(-1) !
         ++init.pMinusOneRep->iReferenceCount;
         // status = 0;
         return init.pMinusOneRep;
      }else {
         Rep * pRep = makeRep((unsigned int)-val);
         pRep->bits = DECNEG;
         return pRep;
      }
   }else {
      return makeRep((unsigned int)val);
   }
}

/*STATIC------------------------------------------------DecRexx::Rep::makeRep-+
|                                                                             |
+----------------------------------------------------------------------------*/
DecRexx::Rep * DecRexx::Rep::makeRep(unsigned int val)
{
   DecNumberUnit units[D2U(10)];
   int digits;
                                   // count of digits
   if (val <= 1) {
      if (val == 0) {
         ++init.pPlusZeroRep->iReferenceCount;
         // status = 0;
         return init.pPlusZeroRep;
      }else {
         ++init.pPlusOneRep->iReferenceCount;
         // status = 0;
         return init.pPlusOneRep;
      }
   }

   // Compute the coefficient
   #if DECDPUN>1
   {
      int i = 0;
      DecNumberUnit * up = units;
      digits = 0;

      do {
         if (i==0) {               // first digit for this Unit
            *up = val % 10;
         }else {                   // add in next value 0-9
            *up += (val % 10) * powers[i];
            if (i==DECDPUN-1) {    // done enough for this Unit
               ++up;               // maybe more .. get ready for it
               i=0;                // count again
               continue;
            }
         }
         ++i;
      }while (++digits, val /= 10, val);
   }

   #else  // DECDPUN==1
   {
      DecNumberUnit * up = units;
      digits = 0;
      do {
         *up++ = val % 10;
      }while (++digits, val /= 10, val);
   }
   #endif

   // Allocate the required space and fill members.
   Rep * pRep = alloc(D2U(digits));
   pRep->digits = digits;
   pRep->bits = 0;
   pRep->exponent = 0;
   {
      int maxUnits = pRep->m_maxUnits;
      DecNumberUnit * s = units;
      DecNumberUnit * d = pRep->lsu;
      for (;;) {
         *d = *s;
         if (--maxUnits <= 0) break;
         ++s, ++d;
      }
   }
   return pRep;
}

#if !defined COM_JAXO_YAXX_DENY_MATH
/*STATIC------------------------------------------------DecRexx::Rep::makeRep-+
|                                                                             |
+----------------------------------------------------------------------------*/
DecRexx::Rep * DecRexx::Rep::makeRep(double d)
{
   // simply returning makeRep() puts poor Visual C++ in troubles.
   Rep * pRep;
   char buffer[1+1+DBL_DIG+1+1+8 + 10];
   sprintf(buffer, "%.*E", DBL_DIG, d);
   pRep = makeRep(buffer);
   return pRep;
}
#endif

/*---------------------------------------------------DecRexx::Rep::isNegative-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool DecRexx::Rep::isNegative() const {
   return (bits & DECNEG) != 0;
}

/*-------------------------------------------------------DecRexx::Rep::isZero-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool DecRexx::Rep::isZero() const {
   return ((digits == 1) && (lsu[0] == 0));
}

/*-------------------------------------------------------DecRexx::Rep::isBool-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool DecRexx::Rep::isBool(bool & whatBool) const {
   if (
      (digits == 1) && (lsu[0] <= 1) &&
      (0==(bits & DECNEG))
   ) {
      whatBool = (lsu[0] == 1);
      return true;
   }else {
      return false;
   }
}

#if !defined COM_JAXO_YAXX_DENY_MATH
/*----------------------------------------------------DecRexx::Rep::getDouble-+
| Rough computation.                                                          |
| This is used only for MATH functions that are not implemented in decimal    |
| arithmetic, but in double: (sin, cos, sinh, ...)                            |
+----------------------------------------------------------------------------*/
double DecRexx::Rep::getDouble() const
{
   double r;
   int ae = exponent + digits - 1;  // adjusted exponent
   if (ae >= DBL_MAX_10_EXP) {
      r = DBL_MAX;
   }else if (ae <= DBL_MIN_10_EXP) {
      r = DBL_MIN;
   }else {
      int i=D2U(digits) - 1;
      r = lsu[i];
      while (i-- > 0) {
         r = ((DECDPUNMAX+1) * r) + lsu[i];
      }
      i = exponent;
      if (i < 0) {
         if (i <= -10) {
            double d = 10000000000.;
            do r /= d; while ((i += 10) <= -10);
         }
         if (i < 0) {
            double d = 10;
            do r /= d; while (++i < 0);
         }
      }else {
         if (i >= 10) {
            double d = 10000000000.;
            do r *= d; while ((i -= 10) >= 10);
         }
         if (i > 0) {
            double d = 10;
            do r *= d; while (--i > 0);
         }
      }
   }
   return (bits & DECNEG)? -r : r;
}
#endif

/*---------------------------------------------------DecRexx::Rep::Init::Init-+
|                                                                             |
+----------------------------------------------------------------------------*/
DecRexx::Rep::Init::Init()
{
   int i;
   unsigned int k = (unsigned int)-1;  // how -1 is represented?

   pPlusZeroRep = alloc(1);          // zero and ones must be created apart
   pMinusZeroRep = alloc(1);
   pPlusOneRep = alloc(1);
   pMinusOneRep = alloc(1);
   decNumberZero(pPlusZeroRep);
   decNumberZero(pMinusZeroRep);
   decNumberZero(pPlusOneRep);
   decNumberZero(pMinusOneRep);
   pMinusZeroRep->bits2 = DECZNEG;
   pPlusOneRep->lsu[0] = 1;
   pMinusOneRep->lsu[0] = 1;
   pMinusOneRep->bits = DECNEG;
   apPowTwoRep[0] = makeRep((unsigned int)256);
   apPowTwoRep[1] = makeRep((unsigned int)65536);
   apPowTwoRep[2] = makeRep((unsigned int)16777216);
   apPowTwoRep[3] = makeRep("4294967296");
   i = sizeof apCplOneRep / sizeof apCplOneRep[0];
   do {
      apCplOneRep[--i] = makeRep(k);
      k >>= 8;
   }while (i);
}

/*--------------------------------------------------DecRexx::Rep::Init::~Init-+
|                                                                             |
+----------------------------------------------------------------------------*/
DecRexx::Rep::Init::~Init() {
   int i;

   i = sizeof apPowTwoRep / sizeof apPowTwoRep[0];
   while (i--) dealloc(apPowTwoRep[i]);
   i = sizeof apCplOneRep / sizeof apCplOneRep[0];
   while (i--) dealloc(apCplOneRep[i]);
   dealloc(pPlusOneRep);
   dealloc(pMinusOneRep);
   dealloc(pPlusZeroRep);
   dealloc(pMinusZeroRep);
}

/*-----------------------------------------------------------DecRexx::DecRexx-+
| Shallow copy                                                                |
+----------------------------------------------------------------------------*/
DecRexx::DecRexx(DecRexx const & source)
{
   m_pRep = source.m_pRep;
   if (m_pRep->m_maxUnits) {
      ++m_pRep->iReferenceCount;
   }
}

/*-----------------------------------------------------------DecRexx::DecRexx-+
| New DecRexx to handle 'digits' digits.  Alloc only if needed.               |
+----------------------------------------------------------------------------*/
inline DecRexx::DecRexx(int digits, DecRexx::Rep * pRep)    // resetNum0
{
   int maxUnits = D2U(digits);
   if (
      (pRep->iReferenceCount > 1) ||
      (maxUnits > pRep->m_maxUnits /* - NOMANSLAND */)  //>>PGR: TBD ???
   ) {
      m_pRep = Rep::alloc(maxUnits);          // just resetting
   }else {
      m_pRep = pRep;
      if (m_pRep->m_maxUnits) {
         ++m_pRep->iReferenceCount;
      }
   }
}

/*----------------------------------------------------------DecRexx::~DecRexx-+
|                                                                             |
+----------------------------------------------------------------------------*/
DecRexx::~DecRexx()
{
   if ((m_pRep->m_maxUnits) && (!--m_pRep->iReferenceCount)) {
      m_pRep->~Rep();
      Rep::dealloc(m_pRep);
   }
}

/*---------------------------------------------------------DecRexx::operator=-+
|                                                                             |
+----------------------------------------------------------------------------*/
DecRexx & DecRexx::operator=(DecRexx const & source)
{
   if (m_pRep != source.m_pRep) {
      this->~DecRexx();
      new(this) DecRexx(source);
   }
   return *this;
}

/*--------------------------------------------------------------DecRexx::plus-+
|                                                                             |
+----------------------------------------------------------------------------*/
void DecRexx::plus(DecRexx const & rxn1, DecRexxContext const & ctxt)
{
   DecRexx res(ctxt.digits, m_pRep);                     // resetNum1
   decAddOp(res.m_pRep, Rep::init.pPlusZeroRep, rxn1.m_pRep, &ctxt, 0);
   res.m_pRep->setFormEngineering(ctxt.isFormEngineering());
   *this = res;
}

/*-------------------------------------------------------------DecRexx::minus-+
|                                                                             |
+----------------------------------------------------------------------------*/
void DecRexx::minus(DecRexx const & rxn1, DecRexxContext const & ctxt)
{
   DecRexx res(ctxt.digits, m_pRep);                     // resetNum1
   decAddOp(res.m_pRep, Rep::init.pPlusZeroRep, rxn1.m_pRep, &ctxt, DECNEG);
   res.m_pRep->setFormEngineering(ctxt.isFormEngineering());
   *this = res;
}

/*---------------------------------------------------------------DecRexx::abs-+
|                                                                             |
+----------------------------------------------------------------------------*/
void DecRexx::abs(DecRexx const & rxn1, DecRexxContext const & ctxt)
{
   DecRexx res(ctxt.digits, m_pRep);                     // resetNum1
   decAddOp(
      res.m_pRep, Rep::init.pPlusZeroRep, rxn1.m_pRep, &ctxt,
      rxn1.m_pRep->bits & DECNEG
   );
   res.m_pRep->setFormEngineering(ctxt.isFormEngineering());
   *this = res;
}

/*--------------------------------------------------------DecRexx::squareRoot-+
|                                                                             |
+----------------------------------------------------------------------------*/
int DecRexx::squareRoot(
   DecRexx const & rxn1,
   DecRexxContext const & ctxt
) {
   int status;
   DecRexx res(ctxt.digits, m_pRep);                     // resetNum1
   status = decNumberSquareRoot(res.m_pRep, rxn1.m_pRep, &ctxt);
   res.m_pRep->setFormEngineering(ctxt.isFormEngineering());
   *this = res;
   return status;
}

/*---------------------------------------------------------DecRexx::increment-+
|                                                                             |
+----------------------------------------------------------------------------*/
void DecRexx::increment(DecRexxContext const & ctxt)
{
   DecRexx res(ctxt.digits, m_pRep);        // resetNum0
   decAddOp(res.m_pRep, m_pRep, Rep::init.pPlusOneRep, &ctxt, 0);
   *this = res;
}

/*---------------------------------------------------------DecRexx::decrement-+
|                                                                             |
+----------------------------------------------------------------------------*/
void DecRexx::decrement(DecRexxContext const & ctxt)
{
   DecRexx res(ctxt.digits, m_pRep);        // resetNum0
   decAddOp(res.m_pRep, m_pRep, Rep::init.pPlusOneRep, &ctxt, DECNEG);
   *this = res;
}

/*----------------------------------------------------------DecRexx::truncate-+
|                                                                             |
+----------------------------------------------------------------------------*/
void DecRexx::truncate(
   DecRexx const & rxn1,
   int n,
   DecRexxContext const & ctxt
) {
   assert (n >= 0);
   DecRexxContext ctxtTemp(ctxt);

   DecRexx rAft(-n);
   {
      DecRexx res(ctxt.digits, m_pRep);                // resetNum1
      decAddOp(res.m_pRep, Rep::init.pPlusZeroRep, rxn1.m_pRep, &ctxt, 0);
      int digits = res.m_pRep->exponent + res.m_pRep->digits + n;
      if (digits > (int)ctxtTemp.digits) {
         ctxtTemp.digits = digits;
      }
//    if ((res.m_pRep->exponent + n) > 0) {  // adjust precision
//       ctxtTemp.digits += (res.m_pRep->exponent + n);
//    }
      ctxtTemp.round = DEC_ROUND_DOWN;
      *this = res;
   }
   {
      DecRexx res(ctxtTemp.digits, m_pRep);            // resetNum0
      decNumberRescale(res.m_pRep, m_pRep, rAft.m_pRep, &ctxtTemp);
      res.m_pRep->setFormEngineering(ctxt.isFormEngineering());
      *this = res;
   }
}

/*--------------------------------------------------------------DecRexx::sign-+
|                                                                             |
+----------------------------------------------------------------------------*/
int DecRexx::sign(DecRexxContext const & ctxt) const
{
   int rc;
   Rep * pRep = Rep::alloc(D2U(ctxt.digits));
   decAddOp(pRep, Rep::init.pPlusZeroRep, m_pRep, &ctxt, 0);
   if (pRep->lsu[0] == 0) {
      rc = 0;
   }else if (pRep->bits & DECNEG) {
      rc = -1;
   }else {
      rc = 1;
   }
   Rep::dealloc(pRep);
   return rc;
}

/*---------------------------------------------------------------DecRexx::add-+
|                                                                             |
+----------------------------------------------------------------------------*/
int DecRexx::add(
   DecRexx const & rxn1,
   DecRexx const & rxn2,
   DecRexxContext const & ctxt
) {

   int status;
   DecRexx res(ctxt.digits, m_pRep);                           // resetNum2
// DecRexx res(ctxt.digits, m_pRep, rxn1.m_pRep, rxn2.m_pRep); // resetNum2
   status = decAddOp(res.m_pRep, rxn1.m_pRep, rxn2.m_pRep, &ctxt, 0);
   res.m_pRep->setFormEngineering(ctxt.isFormEngineering());
   *this = res;
   return status;
}

/*----------------------------------------------------------DecRexx::subtract-+
|                                                                             |
+----------------------------------------------------------------------------*/
int DecRexx::subtract(
   DecRexx const & rxn1,
   DecRexx const & rxn2,
   DecRexxContext const & ctxt
) {
   int status;
   DecRexx res(ctxt.digits, m_pRep);                           // resetNum2
   status = decAddOp(res.m_pRep, rxn1.m_pRep, rxn2.m_pRep, &ctxt, DECNEG);
   res.m_pRep->setFormEngineering(ctxt.isFormEngineering());
   *this = res;
   return status;
}

/*----------------------------------------------------------DecRexx::multiply-+
|                                                                             |
+----------------------------------------------------------------------------*/
int DecRexx::multiply(
   DecRexx const & rxn1,
   DecRexx const & rxn2,
   DecRexxContext const & ctxt
) {
   int status;
   DecRexx res(ctxt.digits, m_pRep);                           // resetNum2
   status = decNumberMultiply(res.m_pRep, rxn1.m_pRep, rxn2.m_pRep, &ctxt);
   res.m_pRep->setFormEngineering(ctxt.isFormEngineering());
   *this = res;
   return status;
}

/*------------------------------------------------------------DecRexx::divide-+
|                                                                             |
+----------------------------------------------------------------------------*/
int DecRexx::divide(
   DecRexx const & rxn1,
   DecRexx const & rxn2,
   DecRexxContext const & ctxt
) {
   int status;
   DecRexx res(ctxt.digits, m_pRep);                           // resetNum2
   status = decDivideOp(
      res.m_pRep, rxn1.m_pRep, rxn2.m_pRep, &ctxt, DEC_OP_DIVIDE
   );
   res.m_pRep->setFormEngineering(ctxt.isFormEngineering());
   *this = res;
   return status;
}

/*-----------------------------------------------------DecRexx::divideInteger-+
|                                                                             |
+----------------------------------------------------------------------------*/
int DecRexx::divideInteger(
   DecRexx const & rxn1,
   DecRexx const & rxn2,
   DecRexxContext const & ctxt
) {
   int status;
   DecRexx res(ctxt.digits, m_pRep);                           // resetNum2
   status = decDivideOp(
      res.m_pRep, rxn1.m_pRep, rxn2.m_pRep, &ctxt, DEC_OP_DIVIDEINT
   );
   res.m_pRep->setFormEngineering(ctxt.isFormEngineering());
   *this = res;
   return status;
}

/*---------------------------------------------------------DecRexx::remainder-+
|                                                                             |
+----------------------------------------------------------------------------*/
int DecRexx::remainder(
   DecRexx const & rxn1,
   DecRexx const & rxn2,
   DecRexxContext const & ctxt
) {
   int status;
   DecRexx res(ctxt.digits, m_pRep);                           // resetNum2
   status = decDivideOp(
      res.m_pRep, rxn1.m_pRep, rxn2.m_pRep, &ctxt, DEC_OP_REMAINDER
   );
   res.m_pRep->setFormEngineering(ctxt.isFormEngineering());
   *this = res;
   return status;
}

/*-------------------------------------------------------------DecRexx::power-+
|                                                                             |
+----------------------------------------------------------------------------*/
int DecRexx::power(
   DecRexx const & rxn1,
   DecRexx const & rxn2,
   DecRexxContext const & ctxt
) {
   int status;
   DecRexx res(ctxt.digits, m_pRep);                           // resetNum2
   status = decNumberPower(res.m_pRep, rxn1.m_pRep, rxn2.m_pRep, &ctxt);
   res.m_pRep->setFormEngineering(ctxt.isFormEngineering());
   *this = res;
   return status;
}

/*--------------------------------------------------DecRexx::fromBinaryString-+
| Create a number from its machine-dependent binary representation.           |
+----------------------------------------------------------------------------*/
int DecRexx::fromBinaryString(
   char const * chars,
   int n,
   bool isNegative,
   DecRexxContext const & ctxt
) {
   if (!n) {
      //>>>PGR: I should create the DecRexxZero
      DecRexx res(ctxt.getDigits(), m_pRep);  // resetNum0
      decNumberZero(res.m_pRep);
      res.m_pRep->setFormEngineering(ctxt.isFormEngineering());
      *this = res;
      return 0;
   }else {
      int status = 0;
      int max = (n < sizeof (int))? n : sizeof (int);
      DecRexxContext ctxtTemp(ctxt);
      ctxtTemp.digits += 10;    // give some extra space for intermediate ops
      DecRexx res(ctxtTemp.digits, m_pRep);   // resetNum0
      Rep * pDnCpl = Rep::alloc(D2U(ctxtTemp.digits));
      decNumberZero(res.m_pRep);
      decNumberZero(pDnCpl);
      for (;;) {
         int temp = *chars++ & 0xFF;
         if (isNegative) {
            status = decAddOp(
               pDnCpl, pDnCpl, Rep::init.apCplOneRep[max-1], &ctxtTemp, 0
            );
         }
         while (--max) {
            temp <<= 8;
            temp |= *chars++ & 0xFF;
            --n;
         }
         status |= decAddOp(
            res.m_pRep,
            res.m_pRep,
            DecRexx((unsigned int)temp).m_pRep,
            &ctxtTemp,
            0
         );
         if ((status != 0) || (--n == 0)) break;
         max = (n < sizeof temp)? n : sizeof temp;
         status = decNumberMultiply(
            res.m_pRep, res.m_pRep, Rep::init.apPowTwoRep[max-1], &ctxtTemp
         );
         if (isNegative) {
            status |= decNumberMultiply(
               pDnCpl, pDnCpl, Rep::init.apPowTwoRep[max-1], &ctxtTemp
            );
         }
         if (status != 0) break;
      }
      if (status == 0) {
         if (isNegative) {
            status = decAddOp(
               res.m_pRep,
               res.m_pRep,
               pDnCpl,
               &ctxtTemp,
               DECNEG
            );
            status |= decAddOp(
               res.m_pRep, res.m_pRep, Rep::init.pPlusOneRep, &ctxtTemp, DECNEG
            );
         }
         // check for overflow vs. normal context
         status |= decAddOp(
            res.m_pRep, Rep::init.pPlusZeroRep, res.m_pRep, &ctxt, 0
         );
      }
      if (status != 0) {
         this->~DecRexx();
         m_pRep = &Rep::BadRep;          // invalidate this number
      }else {
         res.m_pRep->setFormEngineering(ctxt.isFormEngineering());
         *this = res;
      }
      Rep::dealloc(pDnCpl);
      return status;
   }
}

/*----------------------------------------------------DecRexx::toBinaryString-+
|                                                                             |
+----------------------------------------------------------------------------*/
char * DecRexx::toBinaryString(
   char * buf,
   int len
) const {
   if (!len) return buf;

   int digits = m_pRep->digits;
   DecRexxContext ctxt(digits);
   Rep * pDnWork1 = Rep::alloc(D2U(digits));
   Rep * pDnWork2 = Rep::alloc(D2U(digits));
   Rep * pDnQuot= m_pRep;
   char * p = buf + len;

   do {
      int max;
      Rep * pDnRmdr;
      if (len > sizeof (int)) {
         max = sizeof (int);
         len -= max;
      }else {
         max = len;
         len = 0;
      }
      decCompareOp(
         pDnWork1,
         pDnQuot,
         Rep::init.apPowTwoRep[sizeof(int)-1],
         &ctxt,
         COMPARE
      );
      if ((0==(pDnWork1->bits & DECNEG)) && (pDnWork1->lsu[0])) {  // if '+1'
         decDivideOp(
            pDnWork1,
            pDnQuot,
            Rep::init.apPowTwoRep[sizeof(int)-1],
            &ctxt,
            DEC_OP_REMAINDER
         );
         decDivideOp(
            pDnWork2,
            pDnQuot,
            Rep::init.apPowTwoRep[sizeof(int)-1],
            &ctxt,
            DEC_OP_DIVIDEINT
         );
         pDnRmdr = pDnWork1;
         pDnQuot = pDnWork2;
      }else {
         pDnRmdr = pDnQuot;
         pDnQuot = 0;
      }
      p -= max;
      int k = decGetWhole(pDnRmdr);
      while (max--) {
         p[max] = k;
         k >>= 8;
      }
   }while (len && pDnQuot);
   Rep::dealloc(pDnWork1);
   Rep::dealloc(pDnWork2);
   return p;
}

/*-----------------------------------------------------------DecRexx::compare-+
|                                                                             |
+----------------------------------------------------------------------------*/
int DecRexx::compare(
   DecRexx const & rxn1,
   DecRexxContext const & ctxt
) const {
   Rep dn;                 // lsu with one entry suffices
   decCompareOp(&dn, m_pRep, rxn1.m_pRep, &ctxt, COMPARE);
   assert ((dn.digits == 1) && (dn.lsu[0] <= 1));
   if (dn.bits & DECNEG) {
      return -1;
   }else {
      return dn.lsu[0];
   }
}

/*-------------------------------------------------------------DecRexx::isInt-+
| Maximum: 9 digits!                                                          |
+----------------------------------------------------------------------------*/
bool DecRexx::isInt(int & whatInt) const
{
   DecRexxContext ctxt(10);
   whatInt = decGetInt(m_pRep, &ctxt);
   return (whatInt != DEC_BADINT);
}

/*-----------------------------------------------------------DecRexx::isWhole-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool DecRexx::isWhole() const
{
   if (m_pRep->exponent < 0) {     // fractional part
      DecRexxContext ctxt(m_pRep->digits);
      Rep * pDn = Rep::alloc(m_pRep->m_maxUnits);
      int status = decNumberRescale(pDn, m_pRep, Rep::init.pPlusZeroRep, &ctxt);
      Rep::dealloc(pDn);
      return (status & DEC_Inexact)? false : true;
   }else {
      return true;
   }
}


/*------------------------------------------------------------DecRexx::format-+
|                                                                             |
+----------------------------------------------------------------------------*/
DecRexx::String DecRexx::format() const {
   String str(m_pRep->digits + 14);
   if (m_pRep->isFormEngineering()) {
      decNumberToEngString(m_pRep, str.m_buf);
   }else {
      decNumberToString(m_pRep, str.m_buf);
   }
   return str;
}

/*------------------------------------------------------------DecRexx::format-+
|                                                                             |
+----------------------------------------------------------------------------*/
DecRexx::String DecRexx::format(
   int iBefore,
   int iAfter,
   int iExpPlaces,
   int iExpTrigger,              // starts at 1 - 0 is special, -1 is omitted
   DecRexxContext const & ctxt
) const {
   String res;
   DecRexx rxnBase;
   DecRexxContext ctxtTemp(ctxt);
   rxnBase.m_pRep = Rep::alloc(D2U(ctxt.getDigits()));
   Rep * pDn = rxnBase.m_pRep;
   DecRexx rxnTemp;
   char *pStart;
   char *pBuf;
   unsigned char flag = 0;
   int totalSize;
   int intgrSize;
   bool done = false;

   decAddOp(pDn, Rep::init.pPlusZeroRep, m_pRep, &ctxtTemp, 0);

   redo:
   if (iExpPlaces != 0) {
      int trig = (iExpTrigger == -1)? ctxt.getDigits() : iExpTrigger;
      for (;;) {
         if (((pDn->exponent+pDn->digits) > trig) || (pDn->exponent < -(2*trig))) {
            flag = DECTOSTR2_EXPFORM_FORCED;
            if ((iAfter != -1) && (pDn->digits != (iAfter + 1))) {
               /*
               | We want an exponential form with 'n' digits after the dot.
               | Ex:
               | 1.234567E+4.  If n is 3, we want this number to be
               | (rounded to) 1.235E+4.
               | 1.234567E4 is also: 12345.67.  DecNumber represents it as:
               | 1234567E-2 (7 digits, exp == -2)
               | and should now represent it as:
               | 1235E+1 (4 digits, exp == 1) to obtain 1.235E+4.
               |
               | decNumberRescale adjusts numbers so that their exponent
               | is equal to the 3rd argument of the call.  Here, '+1'.
               | '+1' results from the formula:
               | -(iAfter - dn.exponent - dn.digits + 1) which gives:
               | -(3 - -2 - 7 + 1) == +1
               |
               | Sigh.  When rounding a 999s case, the formula won't work:
               | Ex:
               | .999999 with 4 digits after.
               | .999999 is represented as: 999999E-6 (6 digits, exp = -6)
               | the formula gives: -(4 - -6 - 6 + 1) == -5
               | if no rounding had occured, this would work:
               | 99999E-5 is 9.9999
               | but rounding makes the result to be:
               | 100000E-5 9 which is 1.00000 (5 digits after the dot.)
               | The reason is that decNumberRescale ensures the value
               | of the exponent, not the number of digits.
               |
               | This has 2 consequences:
               | 1) We must increase by 1 the number of digits that the
               |    result is able to hold.  The number of digits is
               |    "normaly" equal to iAfter+1 (for an exponential form.)
               |    It should be "iAfter+2" in case 999s round occurs.
               | 2) After returning from decNumberRescale, we need to examine
               |    again how the number "look" and possibly call again
               |    decNumberRescale (i don't think the number can be rounded
               |    again)
               |
               | The first time decNumberRescale is called, pDn is the original
               | number, possibly rounded to ctxt.digits: rxnBase
               | The second time decNumberRescale is called, pDn is the number
               | that the first call to decNumberRescale has computed
               | (rxnTemp.m_pRep)
               */
               DecRexx rAft(-(iAfter - pDn->exponent - pDn->digits + 1));
               ctxtTemp.digits = iAfter + 2;
               rxnTemp = DecRexx(              // resetNum1
                  ctxtTemp.digits, rxnTemp.m_pRep
               );
               decNumberRescale(
                  rxnTemp.m_pRep, rxnBase.m_pRep, rAft.m_pRep, &ctxtTemp
               );
               pDn = rxnTemp.m_pRep;
               continue;
            }
         }else {
            flag = 0;    // exponential form was not triggered
         }
         break;
      }
   }
   if (!done && (flag==0) && (iAfter != -1) && (-pDn->exponent != iAfter)) {
      done = true;
      /*
      | The exponental form was not triggered.  However, as explained above,
      | 999 rounding might changes the conditions for which the formatting
      | may require an exponential form.
      | - for a non-exponential format, the number of digits is equal
      |   to:  dn.digits + iAfter + dn.exponent
      |   '1' is added to handle rounding
      | - 'flag == 0' indicates that no exponential format was applied
      | - 'done' indicates that we already went thru this code (that should
      |   be executed only once.)
      | - iAfter is '-1' when the caller wants all existing digits
      */
      DecRexx rAft(-iAfter);
      ctxtTemp.digits = pDn->digits + pDn->exponent + iAfter + 2;
      rxnTemp = DecRexx(         // resetNum1
         ctxtTemp.digits, rxnTemp.m_pRep
      );
      decNumberRescale(rxnTemp.m_pRep, rxnBase.m_pRep, rAft.m_pRep, &ctxtTemp);
      pDn = rxnTemp.m_pRep;
      goto redo;
   }

   if (m_pRep->isFormEngineering()) {
      flag |= DECTOSTR2_ENG;
   }
   if (flag & DECTOSTR2_EXPFORM_FORCED) {
      intgrSize = 1;
      totalSize = pDn->digits + 14;
   }else if ((pDn->digits == 1) && (pDn->lsu[0] == 0) && (iAfter > 0)) {
      // handle '0' with 'iAfter' as a separate case
      if (iBefore == -1) {
         iBefore = 0;
      }else if (--iBefore < 0) {
         return res;   // null string
      }
      res = String(iBefore + iAfter + 3);
      pStart = res.m_buf;
      while (iBefore--) *pStart++ = ' ';
      *pStart++ = '0';
      *pStart++ = '.';
      while (iAfter--) *pStart++ = '0';
      *pStart = '\0';                       // stash the stincking sentinel
      return res;
   }else {
      totalSize = pDn->digits + pDn->exponent;
      if (pDn->exponent >= 0) {
         intgrSize = totalSize;
      }else if (totalSize > 0) {
         intgrSize = totalSize;
         totalSize = pDn->digits + 1;
      }else {
         intgrSize = 1;
         totalSize = -pDn->exponent + 2;
      }
   }
   if (iBefore >= 0) {
      if (iBefore -= intgrSize, iBefore < 0) return res; // not enough room
      totalSize += iBefore;
   }else {
      iBefore = 0;
   }
   res = String(totalSize + iBefore + iExpPlaces+1 + 1);
   pStart = pBuf = res.m_buf;
   while (iBefore--) *pStart++ = ' ';
   decNumberToString2(pDn, pStart, flag);
   if ((flag & DECTOSTR2_EXPFORM_FORCED) && (iExpPlaces > 0)) {
      int len = strlen(pBuf);
      char * p = strchr(pBuf, 'E');
      if (!p) {
         while (iExpPlaces-- > -2) pBuf[len++] = ' ';
      }else {
         char * p1 = pBuf + len;
         if (iExpPlaces -= p1-p-2, iExpPlaces < 0) { // not enough room
            return String();
         }
         len += iExpPlaces;
         p = p1 + iExpPlaces;
         while (*--p1 >= '0') {             // look for '+' or '-'
            *--p = *p1;                     // right-align exp digits
         }
         while (iExpPlaces--) *--p = '0';   // insert the zeroes
      }
      res.m_buf[len] = '\0';                // stash the stincking sentinel
   }
   return res;
}

/*===========================================================================*/
