/* $Id: DecRexx.h,v 1.7 2011-07-29 10:26:34 pgr Exp $ */

#ifndef COM_DECNBLIB_DECREXX_H_INCLUDED
#define COM_DECNBLIB_DECREXX_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include <limits.h>
#include <float.h>
#include <new>

#define  DECNUMDIGITS 1
#include "DecNumber.h"               // decimal number library

/*----------------------------------------------------- class DecRexxContext -+
|                                                                             |
+----------------------------------------------------------------------------*/
class DECNB_API DecRexxContext : public DecContext {
public:
   DecRexxContext();
   DecRexxContext(int digits);

   enum {
     #ifdef __MWERKS__
        MAX_PRECISION = 1000,
     #else
        MAX_PRECISION = 1000000,
     #endif
     MAX_EMAX = 999999999,
     DEFAULT_PRECISION = 9,
     DEFAULT_EMAX = MAX_EMAX,
     DEFAULT_FUZZ = 0
   };
   int m_fuzz;                   // numeric fuzz

   int getDigits() const;
   int getFuzz() const;
   bool isFormEngineering() const;
   void setFormEngineering(bool isEnabling);
};

/* -- INLINES -- */
inline DecRexxContext::DecRexxContext() : m_fuzz(0) {
   digits = DecRexxContext::DEFAULT_PRECISION;
   emax = DecRexxContext::DEFAULT_EMAX;
   round = DEC_ROUND_HALF_UP;
   flags = 0;
}

inline int DecRexxContext::getDigits() const {
   return digits;
}
inline int DecRexxContext::getFuzz() const {
   return m_fuzz;
}
inline bool DecRexxContext::isFormEngineering()  const {
   return (flags & DEC_FORM_ENGINEERING) == 1;
}
inline void DecRexxContext::setFormEngineering(bool isEnabling) {
   if (isEnabling) {
      flags |= DEC_FORM_ENGINEERING;
   }else {
      flags &= ~DEC_FORM_ENGINEERING;
   }
}

/*------------------------------------------------------------ class DecRexx -+
|                                                                             |
+----------------------------------------------------------------------------*/
class DECNB_API DecRexx {
public:
   DecRexx();
   DecRexx(char const * psz);
   DecRexx(DecRexx const & source);              // always shallow
   DecRexx(int d);
   DecRexx(unsigned int d);
   DecRexx(bool b);
   DecRexx & operator=(DecRexx const & source);
   ~DecRexx();

   static DecRexx const Nil;

   #if !defined COM_JAXO_YAXX_DENY_MATH   // Math extensions (double)
   DecRexx(double d);
   double getDouble() const;            // a rough approximation
   #endif

   // operations
   void plus(DecRexx const & rxn1, DecRexxContext const & ctxt);
   void minus(DecRexx const & rxn1, DecRexxContext const & ctxt);
   void abs(DecRexx const & rxn1, DecRexxContext const & ctxt);
   int squareRoot(DecRexx const & rxn1, DecRexxContext const & ctxt);
   void increment(DecRexxContext const &);
   void decrement(DecRexxContext const &);
   void truncate(DecRexx const & rxn1, int n, DecRexxContext const & ctxt);
   int sign(DecRexxContext const &) const;

   int add(DecRexx const &, DecRexx const &, DecRexxContext const &);
   int subtract(DecRexx const &, DecRexx const &, DecRexxContext const &);
   int multiply(DecRexx const &, DecRexx const &, DecRexxContext const &);
   int divide(DecRexx const &, DecRexx const &, DecRexxContext const &);
   int divideInteger(DecRexx const &, DecRexx const &, DecRexxContext const &);
   int remainder(DecRexx const &, DecRexx const &, DecRexxContext const &);
   int power(DecRexx const &, DecRexx const &, DecRexxContext const &);
   int fromBinaryString(char const *, int, bool, DecRexxContext const &);

   int compare(DecRexx const & cmpnd, DecRexxContext const &) const;

   // miscellaneous (const) inquiries
   bool isNil() const;                  // not yet initialized
   bool isBad() const;                  // failure at initialization
   bool isNegative() const;
   bool isZero() const;
   bool isBool(bool & whatBool) const;
   bool isInt(int & whatInt) const;
   bool isWhole() const;
   int getDigitsCount() const;
   char * toBinaryString(char * buf, int len) const;

   class String {                     // returned by format(s)
      friend class DecRexx;
   public:
      operator char const *() const;  // null or a psz string (0 ended)
      String(String const &);
      ~String();
      String & operator=(String const & source);
   private:
      String();
      String(int capa);
      char * m_buf;
   };

   String format() const;
   String format(
      int iBefore, int iAfter, int iExpPlaces, int iExpTrigger,
      DecRexxContext const &
   ) const;

   // all the rest is private.
private:
   struct DECNB_API Rep : public DecNumber {
      struct DECNB_API Init {
         Rep * pPlusZeroRep;
         Rep * pMinusZeroRep;
         Rep * pPlusOneRep;
         Rep * pMinusOneRep;
         Rep * apPowTwoRep[sizeof (int)];
         Rep * apCplOneRep[sizeof (int)];
         Init();
         ~Init();
      };

      Rep();                     // nil constructor
      Rep(int maxUnits);

      bool isNegative() const;
      bool isZero() const;
      bool isBool(bool & whatBool) const;
      bool isFormEngineering() const;
      void setFormEngineering(bool isEnabling);

      #if !defined COM_JAXO_YAXX_DENY_MATH   // Math extensions (double)
      double getDouble() const;
      #endif

      static Rep * alloc(int units);
      static void dealloc(Rep *);
      static Rep * makeRep(char const * psz);
      static Rep * makeRep(unsigned int);
      static Rep * makeRep(signed int);
      static Rep * makeRep(double d);
      static Rep BadRep;         // Nil (not refCounted)
      static Rep NilRep;         // Nil (not refCounted)
      static Rep::Init const init;
   };

   DecRexx(int digits, Rep *);

   Rep * m_pRep;
};

/* -- INLINES -- */
inline bool DecRexx::Rep::isFormEngineering() const {
   return (bits2 & DECENGINE)? true : false;
}

inline void DecRexx::Rep::setFormEngineering(bool isEnabling) {
   if (isEnabling) bits2 |= DECENGINE; else bits2 &= ~DECENGINE;
}

inline DecRexx::DecRexx() {
   m_pRep = &Rep::NilRep;
}

inline DecRexx::DecRexx(char const * psz) {
   m_pRep = Rep::makeRep(psz);
}

inline DecRexx::DecRexx(int d) {
   m_pRep = Rep::makeRep(d);
}

inline DecRexx::DecRexx(unsigned int d) {
   m_pRep = Rep::makeRep(d);
}

inline DecRexx::DecRexx(bool b) {
   m_pRep = Rep::makeRep((unsigned int)(b? 1 : 0));
}

inline bool DecRexx::isNil() const {
   return m_pRep == &Rep::NilRep;
}

inline bool DecRexx::isBad() const {
   return m_pRep == &Rep::BadRep;
}

inline bool DecRexx::isNegative() const {
   return m_pRep->isNegative();
}

inline bool DecRexx::isZero() const {
   return m_pRep->isZero();
}

inline bool DecRexx::isBool(bool & whatBool) const {
   return m_pRep->isBool(whatBool);
}

#if !defined COM_JAXO_YAXX_DENY_MATH   // Math extensions (double)

inline DecRexx::DecRexx(double d) {
   m_pRep = Rep::makeRep(d);
}
inline double DecRexx::getDouble() const {
   return m_pRep->getDouble();
}

#endif

inline int DecRexx::getDigitsCount() const {
   return m_pRep->digits;
}

inline DecRexx::String::String() {
   m_buf = 0;
}
inline DecRexx::String::String(int capa) {
   m_buf = 1 + new char[1+capa];
   m_buf[-1]  = 1;
}
inline DecRexx::String::String(String const & source) {
   m_buf = source.m_buf;
   if (m_buf) ++m_buf[-1];
}
inline DecRexx::String::~String() {
   if ((m_buf) && (!--m_buf[-1])) delete [] (m_buf-1);
}
inline DecRexx::String & DecRexx::String::operator=(String const & source) {
   if (m_buf != source.m_buf) {
      this->~String();
      new(this) DecRexx::String(source);
   }
   return *this;
}
inline DecRexx::String::operator char const *() const {
   return m_buf;
}
#endif
/*===========================================================================*/

