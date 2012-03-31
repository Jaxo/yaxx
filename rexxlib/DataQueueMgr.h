/* $Id: DataQueueMgr.h,v 1.24 2002-05-18 08:41:12 pgr Exp $ */

#ifndef COM_JAXO_YAXX_DATAQUEUEMGR_H_INCLUDED
#define COM_JAXO_YAXX_DATAQUEUEMGR_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include "RexxString.h"
#include "../toolslib/LinkedList.h"

#ifdef YAXX_NAMESPACE
namespace YAXX_NAMESPACE {
#endif

/*------------------------------------------------------- class DataQueueMgr -+
|                                                                             |
+----------------------------------------------------------------------------*/
class DataQueueMgr : private LinkedList {
public:
   enum QueueMode { NONE, FIFO, LIFO };
   DataQueueMgr();
   ~DataQueueMgr();

   void addDataQueue();               // make a new current queue
   void readDataQueue(streambuf & sbTarget, QueueMode mode);
   void writeDataQueue(streambuf & sbSource, QueueMode qmode);

   void enqueue(RexxString &);        // FIFO to current queue
   void push(RexxString &);           // LIFO to current queue
   RexxString pull();                 // pull from whatever queue
   int currentQueueSize() const;      // size of current queue
   int totalSize() const;             // size in all queues
   int countDataQueues() const;       // number of data queues

   int removeDataQueues(int n = 0);

   static RexxString stripQueueModes(
      RexxString const & cmd,
      QueueMode & qmodeIn,
      QueueMode & qmodeOut
   );

private:
   class DataQueue;                   // these 4 lines needed by MS C++
   class DataQueueItem;
   friend class DataQueue;
   friend class DataQueueItem;

   class DataQueue : public LinkedList::Item {
   public:
      LinkedList m_lst;
   };
   class DataQueueItem : public LinkedList::Item {
   public:
      DataQueueItem(RexxString & str) : m_str(str) {}
      RexxString m_str;
   };
};

inline int DataQueueMgr::countDataQueues() const {
   return LinkedList::size();
}

inline int DataQueueMgr::currentQueueSize() const {
   return ((DataQueue const *)((LinkedList *)this)->getLast())->m_lst.size();
}

#ifdef YAXX_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
