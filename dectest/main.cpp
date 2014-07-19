/* $Id: main.cpp,v 1.18 2002/10/19 09:39:28 pgr Exp $ */

#include <stdio.h>
#include <iostream.h>
#include <assert.h>
#include "DecTest.h"

#define  MAX_TOKEN 1050
#define  DECNUMBER_TESTS_ONLY
#include "../decnblib/DecNumber.h"    // decimal number library
#include "../decnblib/DecNumberLocal.h"    // for DecNumber Show
extern DecNumber * parse(char const * psz, int & rc, bool);
extern DecNumber * parse(char const * psz, int & rc);
extern DecNumber * parse(unsigned int);
extern bool recognizeExtended(DecNumber * dn, char const * pSource);
extern char * allocDec(int digits);
static void decNumberShow(DecNumber *dn);

/*----------------------------------------------------------- class TestImpl -+
| This class shows the implementation of DecTest program in order to test     |
| Mike's DecNumbers specific implementation.                                  |
+----------------------------------------------------------------------------*/
class TestImpl : public Test
{
public:
   // implementation of the virtuals
   int abs(char const *, Context const &);
   int add(char const * op1, char const * op2, Context const & context);
   int compare(char const * op1, char const * op2, Context const & context);
   int divide(char const * op1, char const * op2, Context const & context);
   int divideInt(char const * op1, char const * op2, Context const & context);
   int integer(char const * op, Context const & context);
   int max(char const *, char const *, Context const &);
   int min(char const *, char const *, Context const &);
   int minus(char const * op, Context const & context);
   int multiply(char const * op1, char const * op2, Context const & context);
   int plus(char const * op, Context const & context);
   int power(char const * op1, char const * op2, Context const & context);
   int remainder(char const * op1, char const * op2, Context const & context);
   int remainderNear(char const * op1, char const * op2, Context const & context);
   int rescale(char const * op1, char const * op2, Context const & context);
   int subtract(char const * op1, char const * op2, Context const & context);
   int toEng(char const * op, Context const & context);
   int toSci(char const * op, Context const & context);

private:
   class BaseOp {
   public:
      int run(char const *, char const *, Context const &, Test &);
   private:
      virtual bool isRealOp() const { return true; }
      virtual bool isBinaryOp() = 0;
      virtual void toString(DecNumber & n, char * result) {
         decNumberToString(&n, result);
      }
      virtual int baseOperate(
         DecNumber &, DecNumber &, DecNumber &, DecContext &
      ) = 0;
      static void setDecContext(Context const &, DecContext &);
      static void setConditions(uint32_t const status, int & conds);
   };

   class UnaryOp : private BaseOp {
   public:
      BaseOp::run;
   protected:
      virtual int operate(DecNumber &, DecNumber &, DecContext &) = 0;
   private:
      bool isBinaryOp() { return false; }
      int baseOperate(DecNumber &, DecNumber &, DecNumber &, DecContext &);
   };

   class BinaryOp : private BaseOp {
   public:
      BaseOp::run;
   protected:
      virtual int operate(
         DecNumber &, DecNumber &, DecNumber &, DecContext &
      ) = 0;
   private:
      bool isBinaryOp() { return true; }
      int baseOperate(DecNumber &, DecNumber &, DecNumber &, DecContext &);
   };

   class Abs : public UnaryOp {
      int operate(DecNumber &, DecNumber &, DecContext &);
   };
   class Add : public BinaryOp {
      int operate(DecNumber &, DecNumber &, DecNumber &, DecContext &);
   };
   class Compare : public BinaryOp {
      int operate(DecNumber &, DecNumber &, DecNumber &, DecContext &);
   };
   class Divide : public BinaryOp {
      int operate(DecNumber &, DecNumber &, DecNumber &, DecContext &);
   };
   class DivideInteger : public BinaryOp {
      int operate(DecNumber &, DecNumber &, DecNumber &, DecContext &);
   };
   class Integer : public UnaryOp {
      int operate(DecNumber &, DecNumber &, DecContext &);
   };
   class Max : public BinaryOp {
      int operate(DecNumber &, DecNumber &, DecNumber &, DecContext &);
   };
   class Min : public BinaryOp {
      int operate(DecNumber &, DecNumber &, DecNumber &, DecContext &);
   };
   class Minus : public UnaryOp {
      int operate(DecNumber &, DecNumber &, DecContext &);
   };
   class Multiply : public BinaryOp {
      int operate(DecNumber &, DecNumber &, DecNumber &, DecContext &);
   };
   class Plus : public UnaryOp {
      int operate(DecNumber &, DecNumber &, DecContext &);
   };
   class Power : public BinaryOp {
      int operate(DecNumber &, DecNumber &, DecNumber &, DecContext &);
   };
   class Remainder : public BinaryOp {
      int operate(DecNumber &, DecNumber &, DecNumber &, DecContext &);
   };
   class RemainderNear : public BinaryOp {
      int operate(DecNumber &, DecNumber &, DecNumber &, DecContext &);
   };
   class Rescale : public BinaryOp {
      int operate(DecNumber &, DecNumber &, DecNumber &, DecContext &);
   };
   class Subtract : public BinaryOp {
      int operate(DecNumber &, DecNumber &, DecNumber &, DecContext &);
   };
   class ToSci : public UnaryOp {
      bool isRealOp() const;
      int operate(DecNumber &, DecNumber &, DecContext &);
   };
   class ToEng : public UnaryOp {
      bool isRealOp() const;
      int operate(DecNumber &, DecNumber &, DecContext &);
      void toString(DecNumber & n, char * r);
   };
};

/*------------------------------------------------------TestImpl::BaseOp::run-+
|                                                                             |
+----------------------------------------------------------------------------*/
int TestImpl::BaseOp::run(
   char const * op1,
   char const * op2,
   Context const & context,
   Test & test
) {
   int status;
   char * result;
   int conditions;

   if (!op1 || (isBinaryOp() && !op2)) {
      status = DEC_Invalid_operation;
      result = 0;
   }else {
      DecNumber * pDn1;
      DecNumber * pDn2;
      DecContext set;

      setDecContext(context, set);
      /*
      | avoid stressing too much poor malloc...  This just means that we
      | don't expect any result string to be bigger than MAX_TOKEN which
      | appears to be a quite reasonable assumption.
      */
      result = new char[(set.digits>MAX_TOKEN)? MAX_TOKEN : set.digits+14];
      *result = '\0';

      pDn1 = parse(op1, status, context.isExtended());
      if (isBinaryOp()) {
         int statTemp;
         pDn2 = parse(op2, statTemp, context.isExtended());
         status |= statTemp;
      }else {
         pDn2 = 0;
      }
      if (isRealOp()) {
         if (context.isExtended() || (status == 0)) {
            DecNumber * pDn;
            #ifndef NDEBUG
               if (set.digits >= 500000000) {
                  pDn = 0;
               }else {
                  pDn = (DecNumber *)allocDec(set.digits);
               }
            #else
               pDn = (DecNumber *)allocDec(set.digits);
            #endif
            if (!pDn) {
               status = DEC_Insufficient_storage;
            }else  {
               status |= baseOperate(*pDn, *pDn1, *pDn2, set);  // n = n1 (op) n2?
               toString(*pDn, result);
            }
            delete pDn;
         }
      }else {                       // toScientific, toEngineering
         toString(*pDn1, result);
      }
      // do not delete specials Dn's
      if (pDn1->iReferenceCount == 1) delete pDn1;
      if (pDn2 && (pDn2->iReferenceCount == 1)) delete pDn2;
   }
   setConditions(status, conditions);
   test.checkResult(result, conditions);              // how did we perform?
   delete [] result;
   return status;
}

/*STATIC--------------------------------------TestImpl::BaseOp::setDecContext-+
|                                                                             |
+----------------------------------------------------------------------------*/
void TestImpl::BaseOp::setDecContext(
   Context const & context,
   DecContext & set
) {
   set.digits = context.inqPrecision();
   set.emax = context.inqMaxExponent();
   switch (context.inqRounding()) {
   case Context::ROUNDING_CEILING:   set.round = DEC_ROUND_CEILING;   break;
   case Context::ROUNDING_DOWN:      set.round = DEC_ROUND_DOWN;      break;
   case Context::ROUNDING_FLOOR:     set.round = DEC_ROUND_FLOOR;     break;
   case Context::ROUNDING_HALF_DOWN: set.round = DEC_ROUND_HALF_DOWN; break;
   case Context::ROUNDING_HALF_EVEN: set.round = DEC_ROUND_HALF_EVEN; break;
   case Context::ROUNDING_HALF_UP:   set.round = DEC_ROUND_HALF_UP;   break;
   case Context::ROUNDING_UP:        set.round = DEC_ROUND_UP;        break;
   default:                          assert (false);                  break;
   }
   set.flags = context.isExtended()? 1 : 0;
}

/*STATIC--------------------------------------TestImpl::BaseOp::setConditions-+
|                                                                             |
+----------------------------------------------------------------------------*/
void TestImpl::BaseOp::setConditions(uint32_t const status, int & conds)
{
   conds = 0;
   if (status & DEC_Conversion_overflow)  conds |= Context::COND_CNVOVFLW;
   if (status & DEC_Conversion_syntax)    conds |= Context::COND_CNVSYNTAX;
   if (status & DEC_Conversion_underflow) conds |= Context::COND_CNVUNFLW;
   if (status & DEC_Division_by_zero)     conds |= Context::COND_DIVZERO;
   if (status & DEC_Division_impossible)  conds |= Context::COND_DIVIMP;
   if (status & DEC_Division_undefined)   conds |= Context::COND_DIVUNDEF;
   if (status & DEC_Inexact)              conds |= Context::COND_INEXACT;
   if (status & DEC_Insufficient_storage) conds |= Context::COND_NOMEMORY;
   if (status & DEC_Invalid_context)      conds |= Context::COND_INVCTXT;
   if (status & DEC_Invalid_operation)    conds |= Context::COND_INVOP;
   if (status & DEC_Lost_digits)          conds |= Context::COND_LOSTDIG;
   if (status & DEC_Overflow)             conds |= Context::COND_OVFLW;
   if (status & DEC_Rounded)              conds |= Context::COND_ROUNDED;
   if (status & DEC_Underflow)            conds |= Context::COND_UNFLW;
}

/*---------------------------------------------TestImpl::UnaryOp::baseOperate-+
|                                                                             |
+----------------------------------------------------------------------------*/
int TestImpl::UnaryOp::baseOperate(
   DecNumber & res,
   DecNumber & n1,
   DecNumber &,
   DecContext & ctxt
) {
   return operate(res, n1, ctxt);
}

/*--------------------------------------------TestImpl::BinaryOp::baseOperate-+
|                                                                             |
+----------------------------------------------------------------------------*/
int TestImpl::BinaryOp::baseOperate(
   DecNumber & res,
   DecNumber & n1,
   DecNumber & n2,
   DecContext & ctxt
) {
   return operate(res, n1, n2, ctxt);
}

/*--------------------------------------------------------------TestImpl::abs-+
|                                                                             |
+----------------------------------------------------------------------------*/
int TestImpl::abs(
   char const * op,
   Context const & context
) {
   return Abs().run(op, 0, context, *this);
}
int TestImpl::Abs::operate(DecNumber & r, DecNumber & l, DecContext & c) {
   DecNumber zero;
   decNumberZero(&zero);
   if (l.bits & DECNEG) {
      return decNumberSubtract(&r, &zero, &l, &c);
   }else {
      return decNumberAdd(&r, &zero, &l, &c);
   }
}

/*--------------------------------------------------------------TestImpl::add-+
|                                                                             |
+----------------------------------------------------------------------------*/
int TestImpl::add(
   char const * op1,
   char const * op2,
   Context const & context
) {
   return Add().run(op1, op2, context, *this);
}
int TestImpl::Add::operate(DecNumber & r, DecNumber & l, DecNumber & h, DecContext & c) {
   return decNumberAdd(&r, &l, &h, &c);
}

/*----------------------------------------------------------TestImpl::compare-+
|                                                                             |
+----------------------------------------------------------------------------*/
int TestImpl::compare(
   char const * op1,
   char const * op2,
   Context const & context
) {
   return Compare().run(op1, op2, context, *this);
}
int TestImpl::Compare::operate(DecNumber & r, DecNumber & l, DecNumber & h, DecContext & c) {
   return decNumberCompare(&r, &l, &h, &c);
}

/*-----------------------------------------------------------TestImpl::divide-+
|                                                                             |
+----------------------------------------------------------------------------*/
int TestImpl::divide(
   char const * op1,
   char const * op2,
   Context const & context
) {
   return Divide().run(op1, op2, context, *this);
}
int TestImpl::Divide::operate(DecNumber & r, DecNumber & l, DecNumber & h, DecContext & c) {
   return decNumberDivide(&r, &l, &h, &c);
}

/*--------------------------------------------------------TestImpl::divideInt-+
|                                                                             |
+----------------------------------------------------------------------------*/
int TestImpl::divideInt(
   char const * op1,
   char const * op2,
   Context const & context
) {
   return DivideInteger().run(op1, op2, context, *this);
}
int TestImpl::DivideInteger::operate(DecNumber & r, DecNumber & l, DecNumber & h, DecContext & c) {
   return decNumberDivideInteger(&r, &l, &h, &c);
}

/*----------------------------------------------------------TestImpl::integer-+
|                                                                             |
+----------------------------------------------------------------------------*/
int TestImpl::integer(
   char const * op,
   Context const & context
) {
   return Integer().run(op, 0, context, *this);
}
int TestImpl::Integer::operate(DecNumber & r, DecNumber & l, DecContext & c) {
   return decNumberToInteger(&r, &l, &c);
}

/*--------------------------------------------------------------TestImpl::min-+
|                                                                             |
+----------------------------------------------------------------------------*/
int TestImpl::min(
   char const * op1,
   char const * op2,
   Context const & context
) {
   return Min().run(op1, op2, context, *this);
}
int TestImpl::Min::operate(DecNumber & r, DecNumber & l, DecNumber & h, DecContext & c) {
   int status = decNumberMin(&r, &l, &h, &c);
   return status;
}

/*--------------------------------------------------------------TestImpl::max-+
|                                                                             |
+----------------------------------------------------------------------------*/
int TestImpl::max(
   char const * op1,
   char const * op2,
   Context const & context
) {
   return Max().run(op1, op2, context, *this);
}
int TestImpl::Max::operate(DecNumber & r, DecNumber & l, DecNumber & h, DecContext & c) {
   int status = decNumberMax(&r, &l, &h, &c);
   return status;
}

/*------------------------------------------------------------TestImpl::minus-+
|                                                                             |
+----------------------------------------------------------------------------*/
int TestImpl::minus(
   char const * op,
   Context const & context
) {
   return Minus().run(op, 0, context, *this);
}
int TestImpl::Minus::operate(DecNumber & r, DecNumber & l, DecContext & c) {
   DecNumber zero;
   decNumberZero(&zero);
   return decNumberSubtract(&r, &zero, &l, &c);
}

/*---------------------------------------------------------TestImpl::multiply-+
|                                                                             |
+----------------------------------------------------------------------------*/
int TestImpl::multiply(
   char const * op1,
   char const * op2,
   Context const & context
) {
   return Multiply().run(op1, op2, context, *this);
}
int TestImpl::Multiply::operate(DecNumber & r, DecNumber & l, DecNumber & h, DecContext & c) {
   return decNumberMultiply(&r, &l, &h, &c);
}

/*-------------------------------------------------------------TestImpl::plus-+
|                                                                             |
+----------------------------------------------------------------------------*/
int TestImpl::plus(
   char const * op,
   Context const & context
) {
   return Plus().run(op, 0, context, *this);
}
int TestImpl::Plus::operate(DecNumber & r, DecNumber & l, DecContext & c) {
   DecNumber zero;
   decNumberZero(&zero);
   return decNumberAdd(&r, &zero, &l, &c);
}

/*------------------------------------------------------------TestImpl::power-+
|                                                                             |
+----------------------------------------------------------------------------*/
int TestImpl::power(
   char const * op1,
   char const * op2,
   Context const & context
) {
   return Power().run(op1, op2, context, *this);
}
int TestImpl::Power::operate(DecNumber & r, DecNumber & l, DecNumber & h, DecContext & c) {
   return decNumberPower(&r, &l, &h, &c);
}

/*--------------------------------------------------------TestImpl::remainder-+
|                                                                             |
+----------------------------------------------------------------------------*/
int TestImpl::remainder(
   char const * op1,
   char const * op2,
   Context const & context
) {
   return Remainder().run(op1, op2, context, *this);
}
int TestImpl::Remainder::operate(DecNumber & r, DecNumber & l, DecNumber & h, DecContext & c) {
   return decNumberRemainder(&r, &l, &h, &c);
}

/*----------------------------------------------------TestImpl::remainderNear-+
|                                                                             |
+----------------------------------------------------------------------------*/
int TestImpl::remainderNear(
   char const * op1,
   char const * op2,
   Context const & context
) {
   return RemainderNear().run(op1, op2, context, *this);
}
int TestImpl::RemainderNear::operate(DecNumber & r, DecNumber & l, DecNumber & h, DecContext & c) {
   return decNumberRemainderNear(&r, &l, &h, &c);
}

/*----------------------------------------------------------TestImpl::rescale-+
|                                                                             |
+----------------------------------------------------------------------------*/
int TestImpl::rescale(
   char const * op1,
   char const * op2,
   Context const & context
) {
   return Rescale().run(op1, op2, context, *this);
}
int TestImpl::Rescale::operate(DecNumber & r, DecNumber & l, DecNumber & h, DecContext & c) {
   return decNumberRescale(&r, &l, &h, &c);
}

/*---------------------------------------------------------TestImpl::subtract-+
|                                                                             |
+----------------------------------------------------------------------------*/
int TestImpl::subtract(
   char const * op1,
   char const * op2,
   Context const & context
) {
   return Subtract().run(op1, op2, context, *this);
}
int TestImpl::Subtract::operate(DecNumber & r, DecNumber & l, DecNumber & h, DecContext & c) {
   return decNumberSubtract(&r, &l, &h, &c);
}

/*------------------------------------------------------------TestImpl::toEng-+
|                                                                             |
+----------------------------------------------------------------------------*/
int TestImpl::toEng(
   char const * op,
   Context const & context
) {
   return ToEng().run(op, 0, context, *this);
}
bool TestImpl::ToEng::isRealOp() const {
   return false;
}
int TestImpl::ToEng::operate(DecNumber & r, DecNumber & l, DecContext & c) {
   return 0;
}
void TestImpl::ToEng::toString(DecNumber & n, char * r) {
   decNumberToEngString(&n, r);
}

/*------------------------------------------------------------TestImpl::toSci-+
|                                                                             |
+----------------------------------------------------------------------------*/
int TestImpl::toSci(
   char const * op,
   Context const & context
) {
   return ToSci().run(op, 0, context, *this);
}
bool TestImpl::ToSci::isRealOp() const {
   return false;
}
int TestImpl::ToSci::operate(DecNumber & r, DecNumber & l, DecContext & c) {
   return 0;
}

/*--------------------------------------------------------------decNumberShow-+
|                                                                             |
+----------------------------------------------------------------------------*/
static void decNumberShow(DecNumber *dn)
{
   printf(" >> %c ", (dn->bits&DECNEG)? '-' : '+');
   if (dn->bits & DECSPECIAL) {              // Is a special value
      if (dn->bits & DECINF) printf("Infinity");
      else if (dn->bits & DECSNAN) printf("sNaN");
      else printf("NaN");
      printf("\n");
   }else {                                // carefully display the coefficient
      DecNumberUnit * up = dn->lsu+D2U(dn->digits)-1;         // msu
      printf("%d", *up);
      for (up=up-1; up >= dn->lsu; --up) {
         uint32_t u = *up;
         printf(":");
         for (int32_t cut=DECDPUN-1; cut >= 0; --cut) {
            uint32_t d = u/powers[cut];
            u -= d*powers[cut];
            printf("%d", d);
         }
      }
      printf(" E%d [%d]\n", dn->exponent, dn->digits);
   }
}

/*-----------------------------------------------------------------------main-+
|                                                                             |
+----------------------------------------------------------------------------*/
int main(int argc, char ** argv)
{
   if (argc == 1) {
      char input[300];
      DecContext dc;
      dc.digits = 100;
      dc.emax = DEC_MAX_EMAX;
      dc.round = DEC_ROUND_HALF_UP;
      dc.flags = 0;
      while (
         cin.getline(input, sizeof input) && (input[0] != '!' || input[1])
      ) {
         DecNumber * dn;
         int rc = 0;
         bool isInteger = true;
         for (char * c_p = input; *c_p; ++c_p) {
            if ((*c_p < '0') || (*c_p > '9')) {
               isInteger = false;
               break;
            }
         }
         if (!isInteger) {
            dn = parse(input, rc);
         }else {
            int l;
            sscanf(input, "%d", &l);
            dn = parse(l);
         }
         if (!dn) {
            cout << "=> error! " << rc << endl;
         }else {
            decNumberShow(dn);
         }
      }
      return 0;
   }
   if ((argc < 2) || (*argv[1] == '?')) {
      fprintf(
         stderr,
         "Usage: decTest arguments\n"
         "\n"
         "arguments  contain one or more path to the test files\n"
         "\n"
         "Ex:  decTest /u/tests/testAll.decTest"
      );
      return -1;
   }else {
      Test::Statistics stats;
      for (int i=1; i < argc; ++i) {
         stats += TestImpl().run(argv[i]);
      }
      fprintf(
         stderr,
         "For a total of %d test cases, %d have failed\n",
         stats.m_nbrOfTestsRan, stats.m_nbrOfTestsFailing
      );
      if (stats.m_invalidTestFound) {
         fprintf(
            stderr,
            "Testing is not complete: an invalid test case was found\n"
         );
         return 255;
      }
   }
   return 0;
}

/*===========================================================================*/