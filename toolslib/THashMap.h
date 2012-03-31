/*
* $Id: THashMap.h,v 1.1 2002-12-18 09:57:07 pgr Exp $
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

#ifndef COM_JAXO_TOOLS_THASHMAP_H_INCLUDED
#define COM_JAXO_TOOLS_THASHMAP_H_INCLUDED

/*---------+
| Includes |
+---------*/
#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

template <class T> class TOOLS_API THashMap;

/*----------------------------------------------------------- class TMapItem -+
|                                                                             |
+----------------------------------------------------------------------------*/
template <class T>
class TOOLS_API TMapItem {
   friend class THashMap<T>;
public:
   TMapItem(TString<T> const & key) : m_key(key) {}
   TString<T> & key() { return m_key; }
protected:
   TString<T> m_key;
   virtual ~TMapItem() {}

private:
   TMapItem<T> * m_next;
};

/*----------------------------------------------------------- class THashMap -+
|                                                                             |
+----------------------------------------------------------------------------*/
template <class T>
class TOOLS_API THashMap {
public:
   THashMap(int capacity = 11);
   ~THashMap();
   TMapItem<T> * put(TMapItem<T> * pItm);
   TMapItem<T> * get(TString<T> const & key);
   void clear();
   void remove(TString<T> const & key);
   void iterate(
     void (forEachEntry)(TMapItem<T> * itm, void * arg),
     void * arg
   );

private:
   THashMap(THashMap<T> const &) {}                              // undefined
   THashMap & operator=(THashMap<T> const &) { return * this; }  // undefined
   int m_capacity;
   TMapItem<T> ** m_table;
};

#ifdef TOOLS_NAMESPACE
}
#endif
#endif

#ifdef JAXO_INSTANCIATE_TEMPLATE
#include "THashMap.cpp"
#endif

/*===========================================================================*/
