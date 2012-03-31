/* $Id: LinkedList.cpp,v 1.3 2002-05-18 06:55:30 pgr Exp $ */

#include "LinkedList.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*-----------------------------------------------------LinkedList::LinkedList-+
| Constructor                                                                 |
+----------------------------------------------------------------------------*/
LinkedList::LinkedList() : m_size(0) {
   m_leader.m_next = m_leader.m_previous = &m_leader;
}

/*----------------------------------------------------LinkedList::~LinkedList-+
| Destructor                                                                  |
+----------------------------------------------------------------------------*/
LinkedList::~LinkedList() {
   clear();
}

/*------------------------------------------------------------LinkedList::add-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool LinkedList::add(int index, Item * what) {
   Item * before;
   if (index == m_size) {
      before = &m_leader;
   }else {
      before = get(index);
      if (before == 0) return false;
   }
   what->link(before, m_size);
   return true;
}

/*---------------------------------------------------------LinkedList::remove-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool LinkedList::remove(int index) {
    Item * what = get(index);
    if (what == 0) return false;
    what->unlink(m_size);
    return true;
}

/*------------------------------------------------------------LinkedList::get-+
|                                                                             |
+----------------------------------------------------------------------------*/
LinkedList::Item * LinkedList::get(int index) {
   if ((index < 0) || (index >= m_size)) {
      return 0;
   }
   Item * what = &m_leader;
   if (index < (m_size/2)) {
      for (int i=0; i <= index; ++i) what = what->m_next;
   }else {
      for (int i=m_size; i > index; --i) what = what->m_previous;
   }
   return what;
}

/*----------------------------------------------------------LinkedList::clear-+
|                                                                             |
+----------------------------------------------------------------------------*/
void LinkedList::clear() {
   Item * what = m_leader.m_next;
   while (m_size) {
      Item * temp = what->m_next;
      delete what;
      what = temp;
     --m_size;
   };
   m_leader.m_next = m_leader.m_previous = &m_leader;
}

/*-----------------------------------------------------LinkedList::Item::link-+
|                                                                             |
+----------------------------------------------------------------------------*/
void LinkedList::Item::link(Item * next, int & size) {
   m_next = next;
   m_previous = next->m_previous;
   m_previous->m_next = this;
   m_next->m_previous = this;
   ++size;
}

/*---------------------------------------------------LinkedList::Item::unlink-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool LinkedList::Item::unlink(int & size) {
   if (this == m_next) {
      return false;
   }
   m_previous->m_next = m_next;
   m_next->m_previous = m_previous;
   --size;
   delete this;
   return true;
}

/*---------------------------------------------------LinkedList::Item::relink-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool LinkedList::Item::relink(Item * old) {
    if (!old) return false;
    m_next = old->m_next;
    m_previous = old->m_previous;
    m_previous->m_next = this;
    m_next->m_previous = this;
    delete old;
    return true;
}

/*---------------------------------------------LinkedList::Iterator::Iterator-+
| Constructor                                                                 |
+----------------------------------------------------------------------------*/
LinkedList::Iterator::Iterator(
   LinkedList & lst
) : m_last(&lst.m_leader), m_cur(&lst.m_leader) {
}

/*-------------------------------------------------LinkedList::Iterator::next-+
| LIFO traversal                                                              |
+----------------------------------------------------------------------------*/
LinkedList::Item * LinkedList::Iterator::next() {
   if (m_cur) {
      m_cur = m_cur->m_next;
      if (m_cur == m_last) m_cur = 0;
   }
   return m_cur;
}

/*---------------------------------------------LinkedList::Iterator::previous-+
| FIFO traversal                                                              |
+----------------------------------------------------------------------------*/
LinkedList::Item * LinkedList::Iterator::previous() {
   if (m_cur) {
      m_cur = m_cur->m_previous;
      if (m_cur == m_last) m_cur = 0;
   }
   return m_cur;
}

#ifdef TOOLS_NAMESPACE
}
#endif
/*===========================================================================*/
