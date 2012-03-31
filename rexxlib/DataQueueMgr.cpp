/*
 * $Id: DataQueueMgr.cpp,v 1.41 2002-08-22 09:28:03 pgr Exp $
 */

#include <string.h>
#include <assert.h>

#include "IOMgr.h"
#include "DataQueueMgr.h"

#ifdef __MWERKS_NEEDS_MEMCMP_FIX__
#define memcmp memcmpFix4PalmOS30
#include "../../palmiolib/memcmp.h"
#endif

#ifdef YAXX_NAMESPACE
namespace YAXX_NAMESPACE {
#endif

/*-------------------------------------------------DataQueueMgr::DataQueueMgr-+
|                                                                             |
+----------------------------------------------------------------------------*/
DataQueueMgr::DataQueueMgr() {
   addLast(new DataQueue());
}

/*------------------------------------------------DataQueueMgr::~DataQueueMgr-+
|                                                                             |
+----------------------------------------------------------------------------*/
DataQueueMgr::~DataQueueMgr() {
   removeDataQueues();
   removeLast();
}

/*-------------------------------------------------DataQueueMgr::addDataQueue-+
|                                                                             |
+----------------------------------------------------------------------------*/
void DataQueueMgr::addDataQueue() {
   addLast(new DataQueue());
}

/*---------------------------------------------DataQueueMgr::removeDataQueues-+
| Remove at most n data queues, except the first one.                         |
+----------------------------------------------------------------------------*/
int DataQueueMgr::removeDataQueues(int n)
{
   int i = LinkedList::size();
   if (n == 0) {            // this means: remove all data queues
      n = i;
   }else if (n < 0) {       // some use dropbuf(-3) ...
      n = -n;
   }
   do {
      if (--i > 0) {
         removeLast();
      }else {
         ((DataQueue *)getLast())->m_lst.clear();
         break;
      }
   }while (--n);
   return LinkedList::size() - 1;
}

/*------------------------------------------------------DataQueueMgr::enqueue-+
|                                                                             |
+----------------------------------------------------------------------------*/
void DataQueueMgr::enqueue(RexxString & str) {
   ((DataQueue *)getLast())->m_lst.addFirst(new DataQueueItem(str));
}

/*---------------------------------------------------------DataQueueMgr::push-+
|                                                                             |
+----------------------------------------------------------------------------*/
void DataQueueMgr::push(RexxString & str) {
   ((DataQueue *)getLast())->m_lst.addLast(new DataQueueItem(str));
}

/*---------------------------------------------------------DataQueueMgr::pull-+
| Pull from any data queue                                                    |
+----------------------------------------------------------------------------*/
RexxString DataQueueMgr::pull()
{
   LinkedList & lst = ((DataQueue *)getLast())->m_lst;
   if (!lst.size()) {
      assert (size() == 1);
      return IOMgr::readLine();
   }else {
      RexxString res(((DataQueueItem *)lst.getLast())->m_str);
      lst.removeLast();
      if ((size() > 1) && (lst.size() == 0)) {
         removeLast();
      }
      return res;
   }
}

/*----------------------------------------------------DataQueueMgr::totalSize-+
|                                                                             |
+----------------------------------------------------------------------------*/
int DataQueueMgr::totalSize() const
{
   int total = 0;
   LinkedList::Iterator it(*(LinkedList *)this);
   DataQueue * pDq;
   while ((pDq = (DataQueue *)it.next()) != 0) {
      total += pDq->m_lst.size();
   }
   return total;
}

/*------------------------------------------------DataQueueMgr::readDataQueue-+
| Read (and empty) the current data queue to a file                           |
+----------------------------------------------------------------------------*/
void DataQueueMgr::readDataQueue(streambuf & sbTarget, QueueMode qmode)
{
   LinkedList & lst = ((DataQueue *)getLast())->m_lst;
   int i = lst.size();
   while (i-- > 0) {
      RexxString line;
      if (qmode == LIFO) {
         line = ((DataQueueItem *)lst.getLast())->m_str;
         lst.removeLast();
      }else {
         line = ((DataQueueItem *)lst.getFirst())->m_str;
         lst.removeFirst();
      }
      IOMgr::writeLine(line, sbTarget);
   }
   if (size() > 1) removeLast();
}

/*-----------------------------------------------DataQueueMgr::writeDataQueue-+
| Write (fill) the current data queue from a file                             |
+----------------------------------------------------------------------------*/
void DataQueueMgr::writeDataQueue(streambuf & sbSource, QueueMode qmode)
{
   for (;;) {
      bool isEof;
      RexxString str = IOMgr::readLine(isEof, sbSource);
      if (isEof && !str.length()) {
         break;
      }
      if (qmode == LIFO) {
         push(str);
      }else {
         enqueue(str);
      }
   }
}

/*----------------------------------------------DataQueueMgr::stripQueueModes-+
| Look for Queue Modes (LIFO, FIFO) in a given command.                       |
| Strip these out and set the passed qmodeIn and qmodeOut accordingly.        |
|                                                                             |
| In front of command, search for  "STACK>" or "FIFO>" or "LIFO>"             |
| and/or, at the end, search for   "(STACK" or "(FIFO" or "(LIFO" or          |
|                                  ">STACK" or ">FIFO" or ">LIFO"             |
+----------------------------------------------------------------------------*/
RexxString DataQueueMgr::stripQueueModes(
   RexxString const & cmd,
   QueueMode & qmodeIn,
   QueueMode & qmodeOut
) {
   StringBuffer res(cmd, cmd.length());
   RexxString work = RexxString(cmd, CharConverter::Upper);
   int iLen = res.length();
   char const * c_p = cmd;
   char const * d_p = work;

   qmodeIn = NONE;
   qmodeOut = NONE;

   if ((iLen >= 6) && !memcmp(d_p, "STACK>", 6)) {
      qmodeIn = FIFO;
      res.remove(0, 6);
      d_p += 6;
      iLen = res.length();
   }else if (
      (iLen >= 5) &&
      ((d_p[0] == 'L') || (d_p[0] == 'F')) &&
      !memcmp(d_p+1, "IFO>", 4)
   ) {
      if (d_p[0] == 'L') {
         qmodeIn = LIFO;
      }else {
         qmodeIn = FIFO;
      }
      res.remove(0, 5);
      d_p += 5;
      iLen = res.length();
   }
   if (
      (iLen >= 6) &&
      ((d_p[iLen-6] == '(') || (d_p[iLen-6] == '>')) &&
      !memcmp(d_p + iLen-5, "STACK", 5)
   ) {
      qmodeOut = FIFO;
      res.setLength(iLen - 6);
   }else if (
      (iLen >= 5) &&
      ((d_p[iLen-5] == '(') || (d_p[iLen-5] == '>')) &&
      ((d_p[iLen-4] == 'L') || (d_p[iLen-4] == 'F')) &&
      !memcmp(d_p + iLen-3, "IFO", 3)
   ) {
      if (d_p[iLen-4] == 'L') {
         qmodeOut = LIFO;
      }else {
         qmodeOut = FIFO;
      }
      res.setLength(res.length() - 5);
   }
   res.stripSpaces();
   return res;
}

#ifdef YAXX_NAMESPACE
}
#endif
/*===========================================================================*/
