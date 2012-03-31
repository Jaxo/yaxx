/*
* $Id: Variable.cpp,v 1.40 2002-04-24 10:42:00 pgr Exp $
*/

#include "Variable.h"

#ifdef YAXX_NAMESPACE
namespace YAXX_NAMESPACE {
#endif

/*---------------------------------------------------------Variable::Variable-+
| Constructor for the variable of a simple symbol                             |
| m_value is a place holder                                                   |
+----------------------------------------------------------------------------*/
Variable::Variable(RexxString const & key) :  MapItem(key)
{
   m_pMapTail = 0;
   m_pSource = this;                           /* @003 */
}

/*PRIVATE--------------------------------------------------Variable::Variable-+
| Constructor for a tail                                                      |
+----------------------------------------------------------------------------*/
Variable::Variable(RexxString const & key, RexxString & value) : MapItem(key)
{
   m_value = value;
   m_pMapTail = 0;
   m_pSource = this;                           /* @003 */
}

/*---------------------------------------------------------Variable::Variable-+
| Constructor for an Exposed Variable                                         |
+----------------------------------------------------------------------------*/
Variable::Variable(RexxString const & key, Variable * pVarExposed) : MapItem(key) {
   m_pSource = pVarExposed->m_pSource;         /* @003 */
}

/*--------------------------------------------------------Variable::~Variable-+
| Destructor                                                                  |
+----------------------------------------------------------------------------*/
Variable::~Variable()
{
   if (this == m_pSource) {                    /* @003 */
      if (m_pMapTail) delete m_pMapTail;
   }
}

/*----------------------------------------------------------Variable::getTail-+
|                                                                             |
+----------------------------------------------------------------------------*/
Variable * Variable::getTail(RexxString const & strTail)
{
   HashMap * map = m_pSource->m_pMapTail;
   if (map) {
      return (Variable *)map->get(strTail);
   }else {
      return 0;
   }
}

/*-------------------------------------------------------Variable::createTail-+
|                                                                             |
+----------------------------------------------------------------------------*/
Variable * Variable::createTail(RexxString const & strTail)
{
   // "this" is a stem
   HashMap * map = m_pSource->m_pMapTail;
   if (!map) {
      /*
      | Make sure we have a map.
      | It can be a simple symbol now raised to the stem rank.
      | Ex: foo. = 'hello'; bar=""; foo.bar='world';
      */
      map = m_pSource->m_pMapTail = new HashMap;
   }
   return (Variable *)map->put(new Variable(strTail, m_value));
}

/*-------------------------------------------------------Variable::createTail-+
| Same as above for an exposed variable                                       |
+----------------------------------------------------------------------------*/
Variable * Variable::createTail(RexxString const & strTail, Variable * pVarExposed)
{
   HashMap * map = m_pSource->m_pMapTail;
   if (!map) {
      map = m_pSource->m_pMapTail = new HashMap;
   }
   return (Variable *)map->put(new Variable(strTail, pVarExposed));
}

/*--------------------------------------------------------Variable::clearTail-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Variable::clearTail() {
   if (this != m_pSource) {
      m_pSource->clearTail();
   }else {
      if (m_pMapTail) m_pMapTail->clear();
   }
}

/*-------------------------------------------------------Variable::assignStem-+
| This method copies the value of the stem variable into all tails.           |
+----------------------------------------------------------------------------*/
void Variable::assignStem()
{
   if (this != m_pSource) {
      m_pSource->assignStem();              // not efficient, fixme.
   }else {
      if (m_pMapTail) m_pMapTail->iterate(assignForEachEntry, &m_value);
   }
}

/*-PRIVATE---------------------------------------Variable::assignForEachEntry-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Variable::assignForEachEntry(MapItem * itm, void * arg)
{
   Variable * var = ((Variable *)itm)->m_pSource;
   var->m_value = (*(RexxString *)arg);
}

/*------------------------------------------------------------Variable::dump1-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Variable::dump1(StringBuffer & result, bool isHex)
{
   dumpSimple(result, isHex);
   result.append('\n');
   if (m_pSource->m_pMapTail) {
      dumpCompound(result, isHex);
   }
}

/*------------------------------------------------------------Variable::dump2-+
| There is a slight difference with dump1:                                    |
| - for a simple variable, there is no carriage return inserted               |
| - for a compound variable, the stem value is not shown                      |
+----------------------------------------------------------------------------*/
void Variable::dump2(StringBuffer & result, bool isHex)
{
   if (m_pSource->m_pMapTail) {
      dumpCompound(result, isHex);
   }else {
      dumpSimple(result, isHex);
   }
}

/*-PRIVATE-----------------------------------------------Variable::dumpSimple-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Variable::dumpSimple(StringBuffer & result, bool isHex)
{
   static const char v1[] = "=\"";
   static const char v2[] = "\"x";
   static const char v3[] = "\"";

   result.append(key(), key().length());
   result.append(v1, sizeof v1 - 1);
   if (isHex) {
      RexxString temp;
      temp.c2x(m_value);
      result.append(temp, temp.length());
      result.append(v2, sizeof v2 - 1);
   }else {
      result.append(m_value, m_value.length());
      result.append(v3, sizeof v3 - 1);
   }
}

/*-PRIVATE---------------------------------------------Variable::dumpCompound-+
|                                                                             |
+----------------------------------------------------------------------------*/
struct SVT2 {
   StringBuffer & result;
   RexxString & strStem;
   bool isHex;
   SVT2(StringBuffer & a, RexxString & b, bool & c) :
      result(a), strStem(b), isHex(c) {}
};

void Variable::dumpCompound(StringBuffer & result, bool isHex)
{
   SVT2 arg(result, key(), isHex);
   m_pSource->m_pMapTail->iterate(insertStemPrefix, &arg);
}

/*-------------------------------------------------Variable::insertStemPrefix-+
|                                                                             |
+----------------------------------------------------------------------------*/
void Variable::insertStemPrefix(MapItem * itm, void * arg)
{
   SVT2 * svt = (SVT2 *)arg;
   svt->result.append(svt->strStem, svt->strStem.length());
   ((Variable *)itm)->dumpSimple(svt->result, svt->isHex);   // tail values
   svt->result.append('\n');
}

#ifdef YAXX_NAMESPACE
}
#endif
/*===========================================================================*/
