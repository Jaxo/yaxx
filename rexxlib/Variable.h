/* $Id: Variable.h,v 1.19 2002-04-24 10:42:00 pgr Exp $ */

#ifndef COM_JAXO_YAXX_VARIABLE_H_INCLUDED
#define COM_JAXO_YAXX_VARIABLE_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include "HashMap.h"

#ifdef YAXX_NAMESPACE
namespace YAXX_NAMESPACE {
#endif

/*----------------------------------------------------------- class Variable -+
|                                                                             |
+----------------------------------------------------------------------------*/
class Variable : public MapItem {
public:
   Variable(RexxString const & key);
   Variable(RexxString const & key, Variable * pVarExposed);
   virtual ~Variable();

   RexxString & getValue();
   void setValue(RexxString & value);

   bool isExposed() { return (this != m_pSource); }
   Variable * getTail(RexxString const & strTail);
   Variable * createTail(RexxString const & strTail);
   Variable * createTail(RexxString const & strTail, Variable * pVarExposed);
   void clearTail();
   void assignStem();

   void dump1(StringBuffer & result, bool isHex);
   void dump2(StringBuffer & result, bool isHex);

private:
   RexxString m_value;
   HashMap * m_pMapTail;
   Variable * m_pSource;

   Variable(RexxString const & key, RexxString & value);
   static void insertStemPrefix(MapItem * itm, void * arg);
   static void assignForEachEntry(MapItem * itm, void * arg);
   void dumpSimple(StringBuffer & result, bool isHex);
   void dumpCompound(StringBuffer & result, bool isHex);
};

/* -- INLINES -- */
inline RexxString & Variable::getValue() {
   return m_pSource->m_value;
}
inline void Variable::setValue(RexxString & value) {
   m_pSource->m_value = value;
}

#ifdef YAXX_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
