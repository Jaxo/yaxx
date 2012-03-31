/*
* $Id: THashMap.cpp,v 1.1 2002-12-18 09:57:07 pgr Exp $
*
* (C) Copyright Jaxo Systems, Inc. 2002
* This work contains confidential trade secrets of Jaxo.
* Use, examination, copying, transfer and disclosure to others
* are prohibited, except with the express written agreement of Jaxo.
*
* Author:  Pierre G. Richard
* Written: 12/18/2002
*
*/

#ifdef JAXO_INSTANCIATE_TEMPLATE   // otherwise, this file is not compiled

/*---------+
| Includes |
+---------*/
#include <string.h>

#ifdef YAXX_NAMESPACE
namespace YAXX_NAMESPACE {
#endif

/*---------------------------------------------------------THashMap::THashMap-+
|                                                                             |
+----------------------------------------------------------------------------*/
template <class T>
THashMap<T>::THashMap(int capacity) {
   m_capacity = capacity;
   m_table = new TMapItem<T>* [capacity];
   memset(m_table, 0, capacity * sizeof m_table[0]);
}

/*--------------------------------------------------------THashMap::~THashMap-+
|                                                                             |
+----------------------------------------------------------------------------*/
template <class T>
THashMap<T>::~THashMap() {
   clear();
   delete [] m_table;
}

/*--------------------------------------------------------------THashMap::put-+
| Misnamed.  It does put only if the item is new...                           |
+----------------------------------------------------------------------------*/
template <class T>
TMapItem<T> * THashMap<T>::put(TMapItem<T> * pItm)
{
   TMapItem<T> ** ppItm = m_table + (pItm->key().hashValue() % m_capacity);
   TString<T> & key = pItm->m_key;
   for (;;) {
      int cond;
      if ((!*ppItm) || (cond=(*ppItm)->m_key.compare(key), cond > 0)) {
         pItm->m_next = *ppItm;
         *ppItm = pItm;
         return pItm;
      }else if (!cond) {                       // success
         return *ppItm;
      }
      ppItm = &(*ppItm)->m_next;
   }
}

/*--------------------------------------------------------------THashMap::get-+
|                                                                             |
+----------------------------------------------------------------------------*/
template <class T>
TMapItem<T> * THashMap<T>::get(TString<T> const & key)
{
   TMapItem<T> * pItm = m_table[key.hashValue() % m_capacity];
   for (;;) {
      int cond;
      if ((!pItm) || (cond=pItm->m_key.compare(key), cond > 0)) {
         return 0;
      }else if (!cond) {                       // success
         return pItm;
      }
      pItm = pItm->m_next;
   }
}

/*-----------------------------------------------------------THashMap::remove-+
|                                                                             |
+----------------------------------------------------------------------------*/
template <class T>
void THashMap<T>::remove(TString<T> const & key)  {
   TMapItem<T> ** ppItm = m_table + (key.hashValue() % m_capacity);
   for (;;) {
      int cond;
      if ((!*ppItm) || (cond=(*ppItm)->m_key.compare(key), cond > 0)) {
         return;
      }else if (!cond) {                       // success
         TMapItem<T> * pItm = *ppItm;
         *ppItm = (*ppItm)->m_next;
         delete pItm;
         return;
      }
      ppItm = &(*ppItm)->m_next;
   }
}

/*----------------------------------------------------------THashMap::iterate-+
|                                                                             |
+----------------------------------------------------------------------------*/
template <class T>
void THashMap<T>::iterate(
  void (forEachEntry)(TMapItem<T> * pItm, void * arg),
  void * arg
) {
  for (int i=0; i < m_capacity; ++i) {
     for (TMapItem<T> * pItm = m_table[i]; pItm; pItm = pItm->m_next) {
        forEachEntry(pItm, arg);
     }
  }
}

/*------------------------------------------------------------THashMap::clear-+
|                                                                             |
+----------------------------------------------------------------------------*/
template <class T>
void THashMap<T>::clear() {
  for (int i=0; i < m_capacity; ++i) {
     TMapItem<T> * pItm = m_table[i];
     while (pItm) {
        TMapItem<T> * p = pItm;
        pItm = pItm->m_next;
        delete(p);
     }
  }
}

#ifdef YAXX_NAMESPACE
}
#endif
#endif // JAXO_INSTANCIATE_TEMPLATE
/*===========================================================================*/
