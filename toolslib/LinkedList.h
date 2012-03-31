/* $Id: LinkedList.h,v 1.6 2002-05-18 06:55:30 pgr Exp $ */

#ifndef COM_JAXO_TOOLS_LINKEDLIST_H_INCLUDED
#define COM_JAXO_TOOLS_LINKEDLIST_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include "toolsgendef.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*--------------------------------------------------------- class LinkedList -+
|                                                                             |
+----------------------------------------------------------------------------*/
class TOOLS_API LinkedList {
public:
   class TOOLS_API Iterator;
   friend class Iterator;

   class TOOLS_API Item {
      friend class LinkedList;
      friend class Iterator;
   protected:
      virtual ~Item() {}
   private:
      void link(Item * next, int & size);
      bool unlink(int & size);
      bool relink(Item * old);
      Item * m_next;
      Item * m_previous;
   };

   class TOOLS_API Iterator {            // Iterator
   public:
      Item * next();                     // FIFO traversal
      Item * previous();                 // LIFO traversal
      Item * operator()();               // same as "next()"
      Iterator(LinkedList & lst);
   private:
      Item * m_last;
      Item * m_cur;
   };

   LinkedList();
   ~LinkedList();
   bool add(int index, Item * what);
   void addFirst(Item * what);
   void addLast(Item * what);
   void add(Item * what);
   bool removeFirst();
   bool removeLast();
   bool remove(int index);
   Item * get(int index);
   Item * getFirst();
   Item * getLast();
   bool set(int index, Item * what);
   void clear();
   int size() const;

private:
   int m_size;
   Item m_leader;
};

/* -- INLINES -- */
inline void LinkedList::addFirst(Item * what) {
   what->link(m_leader.m_next, m_size);
}

inline void LinkedList::addLast(Item * what) {
   what->link(&m_leader, m_size);
}

inline void LinkedList::add(Item * what) {
   what->link(&m_leader, m_size);
}

inline bool LinkedList::removeFirst() {
   return m_leader.m_next->unlink(m_size);
}

inline bool LinkedList::removeLast() {
   return m_leader.m_previous->unlink(m_size);
}

inline LinkedList::Item * LinkedList::getFirst() {
   return (m_size == 0)? 0 : m_leader.m_next;
}

inline LinkedList::Item * LinkedList::getLast() {
   return (m_size == 0)? 0 : m_leader.m_previous;
}

inline bool LinkedList::set(int index, Item * what) {
    return what->relink(get(index));
}

inline LinkedList::Item * LinkedList::Iterator::operator()() {
   return next();
}

inline int LinkedList::size() const {
   return m_size;
}

#ifdef TOOLS_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
