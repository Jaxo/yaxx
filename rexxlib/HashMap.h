/* $Id: HashMap.h,v 1.29 2002-04-19 07:53:54 pgr Exp $ */

#ifndef COM_JAXO_YAXX_HASHMAP_H_INCLUDED
#define COM_JAXO_YAXX_HASHMAP_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include "RexxString.h"

#ifdef YAXX_NAMESPACE
namespace YAXX_NAMESPACE {
#endif

/*------------------------------------------------------------ class MapItem -+
|                                                                             |
+----------------------------------------------------------------------------*/
class MapItem {
   friend class HashMap;
public:
   MapItem(RexxString const & key) : m_key(key) {}
   RexxString & key() { return m_key; }
protected:
   RexxString m_key;
   virtual ~MapItem() {}

private:
   MapItem * m_next;
};

/*------------------------------------------------------------ class HashMap -+
|                                                                             |
+----------------------------------------------------------------------------*/
class REXX_API HashMap {
public:
   HashMap(int capacity = 11);
   ~HashMap();
   MapItem * put(MapItem * pItm);
   MapItem * get(RexxString const & key);
   void clear();
   void remove(RexxString const & key);
   void iterate(
     void (forEachEntry)(MapItem * itm, void * arg),
     void * arg
   );
   void print();

private:
   HashMap(HashMap const &);                // undefined
   HashMap & operator=(HashMap const &);    // undefined
   int m_capacity;
   MapItem ** m_table;
};

#ifdef YAXX_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
