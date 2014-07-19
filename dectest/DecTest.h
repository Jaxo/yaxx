/* $Id: DecTest.h,v 1.7 2002/10/15 10:53:17 pgr Exp $*/
#ifndef DECTEST_H_INCLUDED
#define DECTEST_H_INCLUDED

/*
* This code allows any implementation of Standard Decimal Arithmetic
* algorithms to run the arithmetic testcases provided by Mike Cowlishaw
* (see http://www2.hursley.ibm.com/decimal/dectest.html)
*
* It is made of 3 files:
*   - DecTest.cpp and DecTest.h
*   - main.cpp
*
* To parse and execute the testcases, only the two first files are needed.
* DecTest.h contains all the information required to do so.
*
* The third file (main.cpp) is an example of running the testcases
* using Mike's implementation of decimal numbers.
* (see http://www.alphaworks.ibm.com/tech/decNumber)
*
* To create a working example:
* - in the same directory you unzipped DecTest, download Mike's decNumber
* package
* - download Mike's decTest testcases suite into (say) /foo/dectest
* - compile and link:
*   Examples:
*       cl DecTest.cpp main.cpp decContext.c decNumber.c
*       or for GCC:
*       gcc -o Dectest DecTest.cpp main.cpp decContext.c decNumber.c
* - run it:
*       DecTest /foo/dectest/testAll.dectest
*/

class TestCase;
class Context;

/*--------------------------------------------------------------- class Test -+
| This class should be derived based on each specific implementation of       |
| the decimal operations.  This is the only thing you have to implement.      |
+----------------------------------------------------------------------------*/
class Test {
public:
   class Statistics {
   public:
      Statistics();
      void operator+=(Statistics const &);
      int m_nbrOfTestsRan;
      int m_nbrOfTestsFailing;
      bool m_invalidTestFound;
   };

   /*--------------------------------------------------------------Tests::run-+
   | Run a test case file.                                                    |
   |                                                                          |
   | testPath    is an ASCIIZ string that contains the path to the test       |
   |             case file.                                                   |
   |                                                                          |
   | Returns Statistics for this run.                                         |
   +-------------------------------------------------------------------------*/
   Statistics run(char const * testPath);

   /*-------------------------------------------------------Test::checkResult-+
   | This method should be called by the Test implementation                  |
   | for examiningg how conform is the result of each individual operation.   |
   |                                                                          |
   | result      is an ASCIIZ string that contains the result of              |
   |             the operation.  It must be 0 (NULL) when the result          |
   |             is undefined.                                                |
   |                                                                          |
   | conditions  is an integer describing all conditions that were raised     |
   |             during the operation.  It is made by OR'ing the              |
   |             values of the Condition enumeration above.                   |
   |             If no condition were raised, it must be 0.                   |
   |                                                                          |
   | Returns true if the results match what was expected, false otherwise.    |
   +-------------------------------------------------------------------------*/
   bool checkResult(char const * result, int conditions);

   /*------------------------------------------------------Test::(operations)-+
   | Each implementation must define the virtual below.                       |
   +-------------------------------------------------------------------------*/
   virtual int abs(char const *, Context const &) = 0;
   virtual int add(char const *, char const *, Context const &) = 0;
   virtual int compare(char const *, char const *, Context const &) = 0;
   virtual int divide(char const *, char const *, Context const &) = 0;
   virtual int divideInt(char const *, char const *, Context const &) = 0;
   virtual int integer(char const *, Context const &) = 0;
   virtual int max(char const *, char const *, Context const &) = 0;
   virtual int min(char const *, char const *, Context const &) = 0;
   virtual int minus(char const *, Context const &) = 0;
   virtual int multiply(char const *, char const *, Context const &) = 0;
   virtual int plus(char const *, Context const &) = 0;
   virtual int power(char const *, char const *, Context const &) = 0;
   virtual int remainder(char const *, char const *, Context const &) = 0;
   virtual int remainderNear(char const *, char const *, Context const &) = 0;
   virtual int rescale(char const *, char const *, Context const &) = 0;
   virtual int subtract(char const *, char const *, Context const &) = 0;
   virtual int toEng(char const *, Context const &) = 0;
   virtual int toSci(char const *, Context const &) = 0;

private:
   Statistics m_stats;
   TestCase * m_pTestCase;
};

/*------------------------------------------------------------ class Context -+
| This is the context in which operations are performed.                      |
| When DecTest asks to execute such an operation, an instance of this class   |
| is passed by arguments: you may want to inquire the peculiarities of the    |
| context in which to execute the said operation.                             |
| Besides this, you don't have to do anything special for this class: the     |
| virtuals are resolved.                                                      |            |
+----------------------------------------------------------------------------*/
class Context {
public:
   enum Rounding {
      ROUNDING_CEILING,
      ROUNDING_DOWN,
      ROUNDING_FLOOR,
      ROUNDING_HALF_DOWN,
      ROUNDING_HALF_EVEN,
      ROUNDING_HALF_UP,
      ROUNDING_UP,
      ROUNDING_INVALID            // ;-)  (no worry, you won't get this one)
   };
                                  // DecTest condition    | IEEE equivalent
   enum Condition {               // ---------------------+------------------
      COND_CNVOVFLW   = 1 <<  0,  // Conversion overflow  | Overflow
      COND_CNVSYNTAX  = 1 <<  1,  // Conversion syntax    | Invalid operation
      COND_CNVUNFLW   = 1 <<  2,  // Conversion underflow | Underflow
      COND_DIVZERO    = 1 <<  3,  // Division by zero     | Division by zero
      COND_DIVIMP     = 1 <<  4,  // Division impossible  | Invalid operation
      COND_DIVUNDEF   = 1 <<  5,  // Division undefined   | Invalid operation
      COND_INEXACT    = 1 <<  6,  // Inexact              | Inexact
      COND_NOMEMORY   = 1 <<  7,  // Insufficient storage | Invalid operation
      COND_INVCTXT    = 1 <<  8,  // Invalid context      | Invalid operation
      COND_INVOP      = 1 <<  9,  // Invalid operation    | Invalid operation
      COND_LOSTDIG    = 1 << 10,  // Lost digits          | (no equivalent)
      COND_OVFLW      = 1 << 11,  // Overflow             | Overflow
      COND_ROUNDED    = 1 << 12,  // Rounded (extended)   | (no equivalent)
      COND_UNFLW      = 1 << 13   // Underflow            | Underflow
   };

   virtual int inqPrecision() const = 0;
   virtual int inqMaxExponent() const = 0;
   virtual Rounding inqRounding() const = 0;
   virtual bool isExtended() const = 0;
};

#endif
/*===========================================================================*/