/* $Id: Symbol.h,v 1.18 2002-04-19 07:53:54 pgr Exp $ */

#ifndef COM_JAXO_YAXX_SYMBOL_H_INCLUDED
#define COM_JAXO_YAXX_SYMBOL_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include <assert.h>
#include "Constants.h"
#include "HashMap.h"

#ifdef YAXX_NAMESPACE
namespace YAXX_NAMESPACE {
#endif

class SymbolMap;
class Variable;

/*------------------------------------------------------------- class Symbol -+
|                                                                             |
+----------------------------------------------------------------------------*/
class Symbol : public MapItem {
   friend class ConstantSymbol;
   friend class SimpleSymbol;
   friend class CompoundSymbol;
   friend class SymbolPartIterator;

public:
   enum { NO_CACHE = -1 };

   bool isCompound() const { return (m_litCount > 0);  }
   bool isSimple() const { return (m_litCount == 0); }
   RexxString & getStemName();
   Symbol * getStemSymbol();
   void cacheVar(Variable * var, int idArg);
   Variable * getCachedVar(int idArg);
   void uncacheVar() { m_id = NO_CACHE; }

private:
   int m_id;                   // last routine that has set the value
   int m_litCount;             // Compound: # of dot delimited substring
   void * m_value;             // point a cached Variable, or a list of parts

   Symbol(RexxString const & keyArg); // public uses one of SimpleSymbol, etc..
};

/*----------------------------------------------------- class ConstantSymbol -+
|                                                                             |
+----------------------------------------------------------------------------*/
class ConstantSymbol : public Symbol {
public:
   ConstantSymbol(RexxString const & keyArg);
};

/*------------------------------------------------------- class SimpleSymbol -+
|                                                                             |
+----------------------------------------------------------------------------*/
class SimpleSymbol : public Symbol {
public:
   SimpleSymbol(RexxString const & keyArg);
};

/*----------------------------------------------------- class CompoundSymbol -+
|                                                                             |
+----------------------------------------------------------------------------*/
class CompoundSymbol : public Symbol {
public:
   CompoundSymbol(RexxString const & keyArg, SymbolMap & map);
   ~CompoundSymbol();
};

/*------------------------------------------------- class SymbolPartIterator -+
|                                                                             |
+----------------------------------------------------------------------------*/
class SymbolPartIterator  {
public:
   SymbolPartIterator(Symbol * sym);
   bool operator()();          // cursor to next entry...
   RexxString const & getName();
   Symbol * getSymbol();

private:
   Symbol * m_sym;
   int m_ix;
   Symbol * m_symPart;
};

/*---------------------------------------------------------- class SymbolMap -+
|                                                                             |
+----------------------------------------------------------------------------*/
class REXX_API SymbolMap : private HashMap {
public:
   Symbol * put(RexxString const & lit, Token tk);
   Symbol * get(RexxString const & lit) { return (Symbol *)HashMap::get(lit); }
   using HashMap::print;
private:
   HashMap constMap;
};

/* --- INLINES --- */
inline Symbol * Symbol::getStemSymbol() {
   // ASSERT(m_litCount > 0)
   return *(Symbol **)m_value;
}

inline RexxString & Symbol::getStemName() {
   // ASSERT(m_litCount > 0)
   return (*(Symbol **)m_value)->key();
}

inline Variable * Symbol::getCachedVar(int idArg) {
   // ASSERT(m_litCount == 0);
   assert (idArg!= NO_CACHE);
   if (m_id == idArg) {
      return (Variable *)m_value;
   }else {
      return 0;
   }
}

inline void Symbol::cacheVar(Variable * var, int idArg) {
   // ASSERT(m_litCount == 0)
   if (var) {
      m_id = idArg;
      m_value = var;
   }else {
      m_id = NO_CACHE;
   }
}

#ifdef YAXX_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
