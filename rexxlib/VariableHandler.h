/* $Id: VariableHandler.h,v 1.61 2002-05-19 17:26:05 pgr Exp $ */

#ifndef COM_JAXO_YAXX_VARIABLEHANDLER_H_INCLUDED
#define COM_JAXO_YAXX_VARIABLEHANDLER_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include "Constants.h"
#include "Variable.h"
#include "Symbol.h"

#ifdef YAXX_NAMESPACE
namespace YAXX_NAMESPACE {
#endif

class SymbolMap;
class Tracer;
class Pool;

/*---------------------------------------------------- class VariableHandler -+
|                                                                             |
+----------------------------------------------------------------------------*/
class VariableHandler : private Constants {
public:
   VariableHandler(Tracer & trhdlr, SymbolMap & mapSym);
   ~VariableHandler();
   void pushScope();
   void popScope();

   RexxString * createVar(Symbol * sym);
   RexxString * loadVar(Symbol * sym, RexxString & defaultValue, bool & found);
   void dropVar(Symbol * sym);
   void dropIndirect(RexxString & names);
   void assignStem(Symbol * sym);
   void expose(Symbol * sym);

   Variable * find(RexxString const & strSymbol, bool & found);
   void setRESULT(RexxString & value);
   void setRC(int num);
   void setSIGL(int num);

   RexxString getValue(RexxString & name);
   RexxString setValue(RexxString & name, RexxString & newValue);
   int registerPool(Pool * pool);
   RexxString dumpVarmap(bool isHex);
   Pool * getPool(RexxString const & poolName);

private:
   class Scope {
   public:
      Scope(int id);
      int getId()  { return m_id; }
      void push(int id);
      void pop();
      void popAll();
      Scope * getPrevious();
      Variable * getVar(RexxString const & name);
      Variable * putVar(Variable * var);
      void removeVar(RexxString const & name);
      void iterate(
        void (forEachEntry)(Variable * var, void * arg),
        void * arg
      );

   private:
      Scope(int id, Scope * p);
      int m_id;
      Scope * m_pPrevious;
      HashMap m_varmap;
   };

   Tracer & m_tracer;
   SymbolMap & m_mapSym;
   int m_idMax;                   // varmap identifiers
   HashMap m_mapPool;             // map for external pools
   RexxString * m_pStrStem;       // stem of latest find
   StringBuffer m_bufTail;        // tail of latest find
   RexxString m_strTail;          // tail of latest find
   RexxString m_strStem2;         // only used by RxVar_find2
   Scope m_scope;
   Symbol * m_symResult;
   Symbol * m_symSigl;
   Symbol * m_symRC;

   Variable * getCached(Symbol * sym);
   Variable * get(Symbol * sym, bool & found);
   void set(Symbol * sym, RexxString & value);
   Variable * access(Symbol * sym);
   Variable * access(Symbol * sym, bool * isNew);
   void cacheVariable(Symbol * sym, Variable * var);

   void assembleTail(Symbol * sym);
};

/* -- INLINES -- */
inline Variable * VariableHandler::Scope::putVar(Variable * var) {
   return (Variable *)m_varmap.put(var);
}

inline Variable * VariableHandler::Scope::getVar(RexxString const & name) {
   return (Variable *)m_varmap.get(name);
}

inline void VariableHandler::Scope::removeVar(RexxString const & name) {
   m_varmap.remove(name);
}

inline void VariableHandler::cacheVariable(Symbol * sym, Variable * var) {
   sym->cacheVar(var, m_scope.getId());
}

inline Variable * VariableHandler::getCached(Symbol * sym) {
   return sym->getCachedVar(m_scope.getId());
}

inline Variable * VariableHandler::access(Symbol * sym) {
   return access(sym, 0);
}

inline Pool * VariableHandler::getPool(RexxString const & poolName) {
   return (Pool *)m_mapPool.get(poolName);
}

inline RexxString * VariableHandler::loadVar(
   Symbol * sym,
   RexxString & defaultValue,          // to keep it alive
   bool & found
) {
   Variable * var = getCached(sym);    // do we already have it?
   if (var || (var=get(sym, found), found)) {
      found = true;
      return &(var->getValue());
   }else if (sym->isCompound()) {      // var is varStem
      if ((found = (var != 0)) && var->getValue().exists()) {
         defaultValue = var->getValue();
      }else {
         defaultValue.concat(*m_pStrStem, m_strTail);
      }
      return &defaultValue;
   }else {
      found = false;
      return &(sym->key());
   }
}

inline RexxString * VariableHandler::createVar(Symbol * sym)
{
   Variable * var = getCached(sym);
   if (var) {
      return &(var->getValue());
   }else {
      return &(access(sym)->getValue());
   }
}

inline void VariableHandler::dropVar(Symbol * sym)
{
   bool found;
   Variable * var = getCached(sym);
   if (var || (var=get(sym, found), var)) { // @002 ", var))" was: ",found))
      RexxString & name = sym->key();
      if (sym->isSimple()) {                        // simple symbol
         if (var->isExposed()) {                    // free only data
            if (var->getValue().exists()) {
               var->setValue(name);
            }
            var->clearTail();
         }else {
            m_scope.removeVar(name);
         }
      }else {
//       if (var->isExposed()) {
            RexxString res;
            res.concat(*m_pStrStem, m_strTail);
            var->setValue(res);
//       }else {
//          var = m_scope.getVar(m_pStrStem);
//          var->removeTail(m_strTail);
//          sym->id = NO_CACHE;
//       }
      }
   }
   sym->uncacheVar();
}

inline void VariableHandler::assignStem(Symbol * sym)
{
   bool found;
   Variable * var = getCached(sym);    // do we already have it?
   if (var || (var=get(sym, found), found)) {
      var->assignStem();
   }
}


#ifdef YAXX_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
