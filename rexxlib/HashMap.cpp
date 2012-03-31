/* $Id: HashMap.cpp,v 1.39 2002-10-22 07:04:05 pgr Exp $ */

#include <string.h>
#include "HashMap.h"

#ifdef YAXX_NAMESPACE
namespace YAXX_NAMESPACE {
#endif

/*-----------------------------------------------------------HashMap::HashMap-+
|                                                                             |
+----------------------------------------------------------------------------*/
HashMap::HashMap(int capacity) {
   m_capacity = capacity;
   m_table = new MapItem* [capacity];
   memset(m_table, 0, capacity * sizeof m_table[0]);
}

/*----------------------------------------------------------HashMap::~HashMap-+
|                                                                             |
+----------------------------------------------------------------------------*/
HashMap::~HashMap() {
   clear();
   delete [] m_table;
}

/*---------------------------------------------------------------HashMap::put-+
| Misnamed.  It does put only if the item is new...                           |
+----------------------------------------------------------------------------*/
MapItem * HashMap::put(MapItem * pItm)
{
   MapItem ** ppItm = m_table + (pItm->key().hash() % m_capacity);
   RexxString & key = pItm->m_key;
   for (;;) {
      RexxString::COMPARE_RESULT cond;
      if (
         (!*ppItm) ||
         (cond=(*ppItm)->m_key.strictCompare(key), cond == RexxString::IS_GT)
      ) {
         pItm->m_next = *ppItm;
         *ppItm = pItm;
         return pItm;
      }else if (cond == RexxString::IS_EQ) {           // success
         return *ppItm;
      }
      ppItm = &(*ppItm)->m_next;
   }
}

/*---------------------------------------------------------------HashMap::get-+
|                                                                             |
+----------------------------------------------------------------------------*/
MapItem * HashMap::get(RexxString const & key)
{
   MapItem * pItm = m_table[key.hash() % m_capacity];
   for (;;) {
      RexxString::COMPARE_RESULT cond;
      if (
         (!pItm) ||
         (cond=pItm->m_key.strictCompare(key), cond == RexxString::IS_GT)
      ) {
         return 0;
      }else if (cond == RexxString::IS_EQ) {           // success
         return pItm;
      }
      pItm = pItm->m_next;
   }
}

/*------------------------------------------------------------HashMap::remove-+
|                                                                             |
+----------------------------------------------------------------------------*/
void HashMap::remove(RexxString const & key)  {
   MapItem ** ppItm = m_table + (key.hash() % m_capacity);
   for (;;) {
      RexxString::COMPARE_RESULT cond;
      if (
         (!*ppItm) ||
         (cond=(*ppItm)->m_key.strictCompare(key), cond == RexxString::IS_GT)
      ) {
         return;
      }else if (cond == RexxString::IS_EQ) {           // success
         MapItem * pItm = *ppItm;
         *ppItm = (*ppItm)->m_next;
         delete pItm;
         return;
      }
      ppItm = &(*ppItm)->m_next;
   }
}

/*-----------------------------------------------------------HashMap::iterate-+
|                                                                             |
+----------------------------------------------------------------------------*/
void HashMap::iterate(
  void (forEachEntry)(MapItem * pItm, void * arg),
  void * arg
) {
  for (int i=0; i < m_capacity; ++i) {
     for (MapItem * pItm = m_table[i]; pItm; pItm = pItm->m_next) {
        forEachEntry(pItm, arg);
     }
  }
}

/*-------------------------------------------------------------HashMap::clear-+
|                                                                             |
+----------------------------------------------------------------------------*/
void HashMap::clear() {
  for (int i=0; i < m_capacity; ++i) {
     MapItem * pItm = m_table[i];
     while (pItm) {
        MapItem * p = pItm;
        pItm = pItm->m_next;
        delete(p);
     }
  }
}

#ifdef YAXX_NAMESPACE
}
#endif

/*===========================================================================*/
