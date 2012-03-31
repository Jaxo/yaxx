/* $Id: Symbol.cpp,v 1.34 2002-07-04 07:25:35 pgr Exp $*/

#include <string.h>
#include "Symbol.h"

#ifdef YAXX_NAMESPACE
namespace YAXX_NAMESPACE {
#endif

/*-PRIVATE-----------------------------------------------------Symbol::Symbol-+
| Constructor                                                                 |
+----------------------------------------------------------------------------*/
Symbol::Symbol(RexxString const & key) : MapItem(key)
{
}

/*---------------------------------------------ConstantSymbol::ConstantSymbol-+
| Constructor for a constant symbol                                           |
+----------------------------------------------------------------------------*/
ConstantSymbol::ConstantSymbol(RexxString const & key) : Symbol(key)
{
   m_litCount = -1;
   m_value = 0;
   m_id = NO_CACHE;
}

/*-------------------------------------------------SimpleSymbol::SimpleSymbol-+
| Constructor for a simple symbol                                             |
+----------------------------------------------------------------------------*/
SimpleSymbol::SimpleSymbol(RexxString const & key) : Symbol(key)
{
   m_litCount = 0;
   m_value = 0;
   m_id = NO_CACHE;
}

/*---------------------------------------------CompoundSymbol::CompoundSymbol-+
| Constructor for a compound symbol                                           |
+----------------------------------------------------------------------------*/
CompoundSymbol::CompoundSymbol(
   RexxString const & key, SymbolMap & map
) :
   Symbol(key)
{
   char const * pBeg = key;
   char const * const pEnd = pBeg + key.length();
   char const * pCur = (char *)memchr(pBeg, '.', pEnd-pBeg);  // stem
   Symbol **ppItem;

   // count total number of stem and parts composing the compound
   for (pBeg = pCur+1, m_litCount=2; pBeg < pEnd; ++pBeg) {
       if (*pBeg == '.') ++m_litCount;
   }
   pBeg = key;
   m_id = NO_CACHE;
   m_value = ppItem = new Symbol* [m_litCount];

   RexxString temp1(pBeg, pCur-pBeg+1);
   *ppItem++ = map.put(temp1, TK_STEM_SYMBOL);
   while (++pCur < pEnd) {
      if (*pCur != '.') {         // non empty part (e.g.: not "..")
         pBeg = pCur;
         while ((++pCur < pEnd) && (*pCur != '.')) {}
         RexxString temp2(pBeg, pCur-pBeg);
         *ppItem++ = map.put(temp2, TK_SIMPLE_SYMBOL);
      }else {
         *ppItem++ = 0;
      }
   }
   if (*(pEnd-1) == '.') *ppItem = 0;
}

/*--------------------------------------------CompoundSymbol::~CompoundSymbol-+
|                                                                             |
+----------------------------------------------------------------------------*/
CompoundSymbol::~CompoundSymbol() {
   delete [] (Symbol **)m_value;
}

/*-------------------------------------SymbolPartIterator::SymbolPartIterator-+
|                                                                             |
+----------------------------------------------------------------------------*/
SymbolPartIterator::SymbolPartIterator(Symbol * sym) {
   m_sym = sym;
   m_ix = 0;
   m_symPart = 0;
}

/*-----------------------------------------------SymbolPartIterator::operator-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool SymbolPartIterator::operator()()
{
   /* by construction, sym->m_litCount is greater than 1 */
   if (++m_ix >= m_sym->m_litCount) {   // first part starts at 1
      --m_ix;
      m_symPart = 0;
      return false;
   }else {
      m_symPart = ((Symbol **)m_sym->m_value)[m_ix];
      // m_symPart can be null, as in between "AB" and "CD" for "AB..CD"
      return true;
   }
}

/*------------------------------------------------SymbolPartIterator::getName-+
|                                                                             |
+----------------------------------------------------------------------------*/
RexxString const & SymbolPartIterator::getName() {
   if (m_symPart) {
      return m_symPart->key();
   }else {
      return RexxString::Nil;
   }
}

/*----------------------------------------------SymbolPartIterator::getSymbol-+
|                                                                             |
+----------------------------------------------------------------------------*/
Symbol * SymbolPartIterator::getSymbol() {
   if ((!m_symPart) && (m_symPart->m_litCount == -1)) {
      return 0;           // empty or constant symbol, eg: ".." or ".1."
   }else {
      return m_symPart;
   }
}

/*-------------------------------------------------------------SymbolMap::put-+
|                                                                             |
+----------------------------------------------------------------------------*/
Symbol * SymbolMap::put(RexxString const & strSymbol, Token tk)
{
   if (tk & TK_CONSTANT_SYMBOL) {
      Symbol * sym = (Symbol *)constMap.get(strSymbol);
      if (sym) {
         return sym;
      }else {
         return (Symbol *)constMap.put(new ConstantSymbol(strSymbol));
      }
   }else {
      Symbol * sym = (Symbol *)get(strSymbol);
      if (sym) {
         return sym;
      }else {
        if (tk == TK_COMPOUND_SYMBOL) {
           sym = new CompoundSymbol(strSymbol, *this);
        }else {
           assert (
              (tk == TK_SIMPLE_SYMBOL) || (tk == TK_STEM_SYMBOL) ||
              (tk & TK_KEYWORD)
           );
           sym = new SimpleSymbol(strSymbol);
        }
      }
      return (Symbol *)HashMap::put(sym);
   }
}

#ifdef YAXX_NAMESPACE
}
#endif
/*===========================================================================*/
