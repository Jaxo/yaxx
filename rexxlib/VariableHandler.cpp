/* $Id: VariableHandler.cpp,v 1.109 2002-11-16 10:41:45 pgr Exp $
*
* A Symbol IS_A MapItem being mapped by the mapSym map.
* If defines each symbol found during the compilation step.
* A Variable IS_A MapItem being mapped by the VariableHandler::Scope.
*
* The 'id' member odf a Symbol tells what most recent routine sets the
* value.
* This is an accelerator so to avoid to interrogate the current scope
* to find if the symbol has been assigned a value.
*
* For this purpose, the 'm_value' member points to the Variable
* which was the most recently assigned.  If the 'id' member equals
* the current scope id, then the 'm_value' points at the right Variable.
* Otherwise, a search in scope map is done, then 'id' and
* 'm_value' are updated.
*
* The above concerns only simple symbols.  But compound symbols -- made
* of stems and parts -- share the same Symbol structure.
*
* For simple symbols:
*  - the 'id' is either -1 (NO_CACHE) or is a scope identifier
*  - the 'litCount' field is always 0
*  - the 'm_value' is either null, or points to a itmVar
*
* For compound symbols:
*  - the 'id' is kept to NO_CACHE.  Note that the code assume that
*    when id is not NO_CACHE, then it is a simple symbol.
*  - the 'litCount' field is a number -- greater than 1 -- that counts
*    stem and parts.  Ex: in foo.bar.zorch.toto it would be 4.
*  - the 'm_value' points at a list of litCount pointers to symbols,
*    the first one being the stem of the compound symbol ("foo."),
*    and the others describing the remaining parts ("bar", "zorch", "toto".)
*
*  Therefore, to resolve a compound symbol is to go thru each of the
*  its components (stem and parts.)   Each of these are simple symbols,
*  which, in turn, can be resolved as explained above.
*/

#include "Pool.h"
#include "Tracer.h"
#include "VariableHandler.h"

#ifdef YAXX_NAMESPACE
namespace YAXX_NAMESPACE {
#endif

/*----------------------------------------------VariableHandler::Scope::Scope-+
|                                                                             |
+----------------------------------------------------------------------------*/
VariableHandler::Scope::Scope(int id)
{
   m_id = id;
   m_pPrevious = 0;
}

/*----------------------------------------------VariableHandler::Scope::Scope-+
|                                                                             |
+----------------------------------------------------------------------------*/
inline VariableHandler::Scope::Scope(int id, Scope * p)
{
   m_id = id;
   m_pPrevious = p;
}

/*----------------------------------------VariableHandler::Scope::getPrevious-+
|                                                                             |
+----------------------------------------------------------------------------*/
inline VariableHandler::Scope * VariableHandler::Scope::getPrevious() {
   return m_pPrevious;
}

/*-----------------------------------------------VariableHandler::Scope::push-+
|                                                                             |
+----------------------------------------------------------------------------*/
void VariableHandler::Scope::push(int id)
{
   void * p = new char[sizeof *this];
   memcpy(p, this, sizeof *this);
   new(this) Scope(id, (Scope *)p);
}

/*------------------------------------------------VariableHandler::Scope::pop-+
|                                                                             |
+----------------------------------------------------------------------------*/
void VariableHandler::Scope::pop()
{
   Scope * p = m_pPrevious;
   this->~Scope();
   memcpy(this, p, sizeof *this);
   delete [] (char *)p;
}

/*---------------------------------------------VariableHandler::Scope::popAll-+
|                                                                             |
+----------------------------------------------------------------------------*/
void VariableHandler::Scope::popAll()
{
   if (m_pPrevious) {
      Scope * p = m_pPrevious;
      m_pPrevious = 0;
      do {
         Scope * pPrevious = p->m_pPrevious;
         delete p;
         p = pPrevious;
      }while (p);
   }
}

/*--------------------------------------------VariableHandler::Scope::iterate-+
|                                                                             |
+----------------------------------------------------------------------------*/
void VariableHandler::Scope::iterate(
  void (forEachEntry)(Variable * var, void * arg),
  void * arg
) {
  m_varmap.iterate((void (*)(MapItem * var, void * arg))forEachEntry, &arg);
}

/*-------------------------------------------VariableHandler::VariableHandler-+
|                                                                             |
+----------------------------------------------------------------------------*/
VariableHandler::VariableHandler(
   Tracer & trhdlr,
   SymbolMap & mapSym
) :
   m_scope(1),
   m_tracer(trhdlr),
   m_mapSym(mapSym)
{
   m_idMax = 1;
   m_pStrStem = 0;
   m_symResult = new SimpleSymbol(STR_RESULT);
   m_symSigl = new SimpleSymbol(STR_SIGL);
   m_symRC = new SimpleSymbol(STR_RC);
   registerPool(new SystemPool());
}

/*--------------------------------------------------VariableHandler::getValue-+
|                                                                             |
+----------------------------------------------------------------------------*/
RexxString VariableHandler::getValue(RexxString & name)
{
   RexxString tempName(name, CharConverter::Upper);
   bool found;

   Variable * var = find(tempName, found);
   if (found) {
      return var->getValue();
   }else {
      if (memchr(name, '.', name.length()-1)) {         // compound:
         if (var && var->getValue().exists()) {         // var is varStem
            return var->getValue();
         }else {
            tempName.concat(*m_pStrStem, m_strTail);
            return tempName;
         }
      }else {                                           // Simple symbol
         return tempName;
      }
   }
}

/*-------------------------------------------------VariableHandler::pushScope-+
|                                                                             |
+----------------------------------------------------------------------------*/
void VariableHandler::pushScope() {
   m_scope.push(++m_idMax);
}

/*--------------------------------------------------VariableHandler::popScope-+
|                                                                             |
+----------------------------------------------------------------------------*/
void VariableHandler::popScope() {
   m_scope.pop();
}

/*----------------------------------------------VariableHandler::assembleTail-+
|                                                                             |
+----------------------------------------------------------------------------*/
void VariableHandler::assembleTail(Symbol * sym)
{
   bool isFirstTime = true;
   SymbolPartIterator iterator(sym);
   m_bufTail.empty();
   while (iterator()) {
      if (isFirstTime) {
         isFirstTime = false;
      }else {
         m_bufTail.append('.');
      }
      RexxString const & strPart = iterator.getName();
      if (strPart.length()) {          // or, in between AB and CD in AB..CD
         Variable * var;
         Symbol * symPart = iterator.getSymbol();
         if (!symPart) {               // this Part was a Constant symbol
            m_bufTail.append(strPart, strPart.length());
         }else if (var = symPart->getCachedVar(m_scope.getId()), var != 0) {
            RexxString const & value = var->getValue();
            m_bufTail.append(value, value.length());
         }else if (var = m_scope.getVar(strPart), var != 0) {
            RexxString const & value = var->getValue();
            m_bufTail.append(value, value.length());
            cacheVariable(symPart, var);
         }else {
            m_bufTail.append(strPart, strPart.length());
 //         m_bufTail.append(RexxString(strPart, CharConverter::Upper));
         }
      }
   }
}

/*-------------------------------------------------------VariableHandler::set-+
|                                                                             |
+----------------------------------------------------------------------------*/
void VariableHandler::set(
   Symbol * sym,
   RexxString & value
) {
   Variable * var = getCached(sym);
   if (!var) var = access(sym);
   var->setValue(value);
}

/*-----------------------------------------------------VariableHandler::setRC-+
|                                                                             |
+----------------------------------------------------------------------------*/
void VariableHandler::setRC(int num) {
   RexxString temp(num);
   set(m_symRC, temp);
}

/*-------------------------------------------------VariableHandler::setRESULT-+
|                                                                             |
+----------------------------------------------------------------------------*/
void VariableHandler::setRESULT(RexxString & value) {
   set(m_symResult, value);
}

/*---------------------------------------------------VariableHandler::setSIGL-+
|                                                                             |
+----------------------------------------------------------------------------*/
void VariableHandler::setSIGL(int num) {
   RexxString temp(num);
   set(m_symSigl, temp);
}

/*------------------------------------------------VariableHandler::dumpVarmap-+
|                                                                             |
+----------------------------------------------------------------------------*/
struct SVT {
   StringBuffer & result;
   bool isHex;
   SVT(StringBuffer & a, bool b) : result(a), isHex(b) {}
};
static void dumpForEachEntry(Variable * var, void * arg) {
   var->dump1(((SVT *)arg)->result, ((SVT *)arg)->isHex);
}

RexxString VariableHandler::dumpVarmap(bool isHex) {
   StringBuffer result;
   SVT arg(result, isHex);
   m_scope.iterate(dumpForEachEntry, &arg);
   return result;
}

/*------------------------------------------------------VariableHandler::find-+
| As "get" except that there are no obvious Symbol instance associated to     |
| the symbol to find.  Therefore, the symbol needs to be reparsed and         |
| map->get to be reissued because we don't know if the variable is cached.    |
+----------------------------------------------------------------------------*/
Variable * VariableHandler::find(
   RexxString const & key,
   bool & found
) {
   /* search for a '.' except in the last character */
   char const * pBeg = key;
   char const * pCur = (char *)memchr(pBeg, '.', key.length()-1);

   if (!pCur) {                // simple variable
      Variable * var = m_scope.getVar(key);
      if (var) {
         found = true;
      }else {
         found = false;
      }
      return var;
   }else {
      char const * pEnd = pBeg + key.length();

      m_pStrStem = &m_strStem2;
      m_strStem2 = RexxString(pBeg, pCur-pBeg+1);

      m_bufTail.setLength(0);
      while (++pCur < pEnd) {
         if (*pCur != '.') {         // non empty part (e.g.: not "..")
            pBeg = pCur;
            while ((++pCur < pEnd) && (*pCur != '.')) {}
            RexxString name(pBeg, pCur-pBeg);
            Variable * var = m_scope.getVar(name);
            if (var) {
               RexxString const & value = var->getValue();
               m_bufTail.append(value, value.length());
            }else {
               name = RexxString(name, CharConverter::Upper);
               m_bufTail.append(name, name.length());
            }
            if (pCur == pEnd) break;
         }
         m_bufTail.append('.');
      }
      m_strTail = m_bufTail;
      Variable * varStem = m_scope.getVar(m_strStem2);
      if (varStem) {
         Variable * varTail = varStem->getTail(m_strTail);
         if (varTail) {
            found = true;
            return varTail;
         }
      }
      found = false;
      return varStem;    /* return stem item */
   }
}

/*----------------------------------------------VariableHandler::registerPool-+
|                                                                             |
+----------------------------------------------------------------------------*/
int VariableHandler::registerPool(Pool * pool) {
   m_mapPool.put(pool);
   return 0;
}

/*----------------------------------------------------VariableHandler::access-+
| Get an existing variable, or create a new one if it is does not exist.      |
+----------------------------------------------------------------------------*/
Variable * VariableHandler::access(Symbol * sym, bool * pFound)
{
   bool locFound;
   bool & found = (pFound == 0)? locFound : (*pFound);
   Variable * var = get(sym, found);
   if (found) {
      return var;
   }else if (sym->isSimple()) {
      var = m_scope.putVar(new Variable(sym->key()));
      cacheVariable(sym, var);
      return var;
  }else {                       // compound symbol
      /*
      | find has set
      | - "m_pStrStem" to the stem component of this compound symbol
      | - "m_strTail" to its tail component
      | Also, it returned in var the stem variable, if any.
      */
      if (!var) var = m_scope.putVar(new Variable(*m_pStrStem)); // the Stem
      return var->createTail(m_strTail);                         // the Tail
   }
}

/*--------------------------------------------------VariableHandler::setValue-+
|                                                                             |
+----------------------------------------------------------------------------*/
RexxString VariableHandler::setValue(
   RexxString & name,
   RexxString & newValue
) {
   RexxString tempName(name, CharConverter::Upper);
   bool found;
   Variable * var = find(tempName, found);
   if (!found) {
      if (memchr(name, '.', name.length()-1)) {                // Compound
         if (!var) var = m_scope.putVar(new Variable(*m_pStrStem));  // stem
         var = var->createTail(m_strTail);                           // tail
      }else {
         var = m_scope.putVar(new Variable(tempName));         // Simple
      }
   }
   RexxString oldValue(var->getValue());
   var->setValue(newValue);
   return oldValue;
}

/*-------------------------------------------------------VariableHandler::get-+
| Search for a Variable that match a given symbol.                            |
|                                                                             |
| Parameters:                                                                 |
|    sym     symbol to look for                                               |
|    found   a pointer to a boolean set to true if the Variable was found     |
|                                                                             |
| Returns:                                                                    |
|    - if found, the Variable that corresponds; else,                         |
|    - for compound variables, if the stem was found, it returns              |
|      the Variable corrresponding to the stem; else,                         |
|    - it returns null.                                                       |
+----------------------------------------------------------------------------*/
Variable * VariableHandler::get(Symbol * sym, bool & found)
{
   if (sym->isSimple()) {
      Variable * var = m_scope.getVar(sym->key());
      if (var) {
         cacheVariable(sym, var);
         found = true;
      }else {
         found = false;
      }
      return var;

   }else {                                 // compound variable
      m_pStrStem = &(sym->getStemName());  // the stem: "ABC." in "ABC.DE.F"

      Symbol * symStem = sym->getStemSymbol();
      Variable * varStem = symStem->getCachedVar(m_scope.getId());
      if (!varStem) {
         varStem = m_scope.getVar(*m_pStrStem);  // try the varmap
         cacheVariable(symStem, varStem);
      }
      assembleTail(sym);
//    m_strTail = m_bufTail;
      m_strTail.assign(m_bufTail, m_bufTail.length());
      if (m_tracer.isTracingIntermediates()) {
         m_tracer.traceCompound(*m_pStrStem, m_strTail);
      }
      if (varStem) {
         Variable * varTail = varStem->getTail(m_strTail);
         if (varTail) {
            found = true;
            return varTail;
         }
      }
      found = false;
      return varStem;    /* return stem item */
   }
}

/*----------------------------------------------VariableHandler::dropIndirect-+
|                                                                             |
+----------------------------------------------------------------------------*/
void VariableHandler::dropIndirect(RexxString & names)
{
   RexxString name;
   int const w = names.wordsCount();
   bool found;

   for (int i=1; i <= w; ++i) {
      name.wordAt(names, i);
      Variable * var = find(name, found);
      if (var) {         /* @002 this was: if (found) */
         if (var->isExposed()) {                // free only data
            var->setValue(name);
            var->clearTail();
         }else if (name.contains('.', name.length()-1)) {  // compound
            RexxString temp;
            temp.concat(*m_pStrStem, m_strTail);
            var->setValue(temp);
         }else {
            m_scope.removeVar(name);
            Symbol * sym = m_mapSym.get(name);  // if it exists as a Symbol
            if (sym) sym->uncacheVar();
         }
      }
   }
}

/*----------------------------------------------------VariableHandler::expose-+
|                                                                             |
+----------------------------------------------------------------------------*/
void VariableHandler::expose(Symbol * sym)
{
   Variable  * var;
   bool exists;

   /*
   | if it exists, it can only be already exposed: done.
   | if it is a coumpound symbol that doesn't exist in full,
   | but the stem exists and was exposed: done.
   | Ex: the 2 lasts symbols in:
   | procedure expose foo foo. foo foo.a.b
   */
   var = get(sym, exists);
   if ((exists) || ((var) && (var->isExposed()))) {
      return;
   }

   // Search (or create) the variable in the previous variable map
   {
      char temp[sizeof m_scope];
      memcpy(temp, &m_scope, sizeof temp);  // backup
      memcpy(&m_scope, m_scope.getPrevious(), sizeof temp);  // get previous
      var = access(sym, &exists);           // search...
      memcpy(&m_scope, temp, sizeof temp);  // restore
   }
   if (!exists) {                  // didn't exist and has been created
      if (sym->isCompound()) {
         RexxString temp;
         temp.concat(*m_pStrStem, m_strTail);
         var->setValue(temp);
      }else {
         var->setValue(sym->key());
      }
   }

   // Create an entry in the current variable varmap
   if (sym->isSimple()) {
      m_scope.putVar(new Variable(sym->key(), var));
   }else {
      Variable * varStem = m_scope.getVar(*m_pStrStem);
      if (!varStem) varStem = m_scope.putVar(new Variable(*m_pStrStem));
      varStem->createTail(m_strTail, var);
   }
}

/*------------------------------------------VariableHandler::~VariableHandler-+
|                                                                             |
+----------------------------------------------------------------------------*/
VariableHandler::~VariableHandler()
{
   delete m_symRC;
   delete m_symSigl;
   delete m_symResult;
   m_scope.popAll();
}

#ifdef YAXX_NAMESPACE
}
#endif
/*===========================================================================*/
