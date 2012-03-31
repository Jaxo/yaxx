/*
* $Id: DbmRecCursor.cpp,v 1.7 2002-09-04 10:08:00 pgr Exp $
*
* Cursor class for Database Records
*/

/*---------+
| Includes |
+---------*/
#include <string.h>
#include "DbmRecCursor.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*-------------------------------------------------DbmRecCursor::DbmRecCursor-+
|                                                                             |
+----------------------------------------------------------------------------*/
DbmRecCursor::DbmRecCursor(DmOpenRef const & fh) :
   m_fh(fh), m_rec(0), m_isLocked(false), m_isDirty(false) {
}

/*------------------------------------------------DbmRecCursor::~DbmRecCursor-+
|                                                                             |
+----------------------------------------------------------------------------*/
DbmRecCursor::~DbmRecCursor() {
   if (m_isLocked) {
      DmReleaseRecord(m_fh, m_num-1, m_isDirty);
      m_isLocked = false;
   }
}

/*----------------------------------------------------------DbmRecCursor::get-+
|                                                                             |
+----------------------------------------------------------------------------*/
Err DbmRecCursor::get(char * pTgt, int offset, int len)
{
   Err err = 0;
   if (len) {
      MemPtr pSrc = MemHandleLock(m_rec);
      if (!pSrc) {
         err = DmGetLastErr();
      }else {
         memcpy(pTgt, (char const *)pSrc + offset, len);
         MemHandleUnlock(m_rec);
      }
   }
   return err;
}

/*----------------------------------------------------------DbmRecCursor::put-+
|                                                                             |
+----------------------------------------------------------------------------*/
Err DbmRecCursor::put(char const * pSrc, int offset, int len)
{
   Err err = 0;
   if (len) {
      lock();
      MemPtr pTgt = MemHandleLock(m_rec);
      if (!pTgt) {
         err = DmGetLastErr();
      }else {
         DmWrite(pTgt, offset, pSrc, len);
         MemHandleUnlock(m_rec);
         m_isDirty = true;
      }
      unlock();
   }
   return err;
}

/*---------------------------------------------------------DbmRecCursor::skip-+
|                                                                             |
+----------------------------------------------------------------------------*/
MemHandle DbmRecCursor::skip(int delta)
{
   int newNum = incrtNumBy(m_num, delta);
   if (newNum == -1) {
      return 0;
   }else {
      unlock();
      m_isDirty = false;
      m_num = newNum;
      m_rec = DmQueryRecord(m_fh, newNum-1);
      return m_rec;
   }
}

/*----------------------------------------------------------DbmRecCursor::add-+
|                                                                             |
+----------------------------------------------------------------------------*/
MemHandle DbmRecCursor::add(int size)
{
   unlock();
   m_rec = DmNewRecord(m_fh, &m_num, size);
   m_isDirty = true;
   m_isLocked = true;
   ++m_num;
   return m_rec;
}

/*---------------------------------------------------------DbmRecCursor::lock-+
|                                                                             |
+----------------------------------------------------------------------------*/
MemHandle DbmRecCursor::lock() {
   if (!m_isLocked) {
      m_rec = DmGetRecord(m_fh, m_num-1);
      m_isLocked = true;
   }
   return m_rec;
}

/*-------------------------------------------------------DbmRecCursor::unlock-+
|                                                                             |
+----------------------------------------------------------------------------*/
void DbmRecCursor::unlock() {
   if (m_isLocked) {
      DmReleaseRecord(m_fh, m_num-1, m_isDirty);
      m_isLocked = false;
   }
}

/*-------------------------------------------------------DbmRecCursor::resize-+
|                                                                             |
+----------------------------------------------------------------------------*/
MemHandle DbmRecCursor::resize(int newSize)
{
   m_isDirty = true;
   if (newSize == 0) {
      DmRemoveRecord(m_fh, m_num-1);
      m_isLocked = false;
      m_rec = 0;
      m_num = 0;
      return 0;
   }else {
      return DmResizeRecord(m_fh, m_num-1, newSize);
   }
}

/*-----------------------------SequentialDbmRecCursor::SequentialDbmRecCursor-+
|                                                                             |
+----------------------------------------------------------------------------*/
SequentialDbmRecCursor::SequentialDbmRecCursor(DmOpenRef const & fh) :
   DbmRecCursor(fh)
{
   m_num = (DmNumRecords(fh) > 0)? 1 : 0;
}

/*-----------------------------------------SequentialDbmRecCursor::incrtNumBy-+
|                                                                             |
+----------------------------------------------------------------------------*/
int SequentialDbmRecCursor::incrtNumBy(int num, int delta) const
{
   int newNum = num + delta;
   if ((newNum <= 0) || (newNum > lastNum())) {
      return -1;
   }else {
      return newNum;
   }
}

/*----------------------------------SequentialDbmRecCursor::removeFollowersTo-+
|                                                                             |
+----------------------------------------------------------------------------*/
Err SequentialDbmRecCursor::removeFollowersTo(UInt16 num)
{
   while (num > m_num) {
      DmRemoveRecord(m_fh, --num);
   }
   return DmGetLastErr();
}

/*---------------------------CategorizedDbmRecCursor::CategorizedDbmRecCursor-+
|                                                                             |
+----------------------------------------------------------------------------*/
CategorizedDbmRecCursor::CategorizedDbmRecCursor(
   DmOpenRef const & fh,
   UInt16 ixCat
) :
   DbmRecCursor(fh),
   m_ixCat(ixCat)
{
   if (DmNumRecordsInCategory(fh, ixCat) > 0) {
      m_num = 0;
      DmQueryNextInCategory(fh, &m_num, ixCat);
      ++m_num;
   }else {
      m_num = 0;
   }
}

/*----------------------------------------CategorizedDbmRecCursor::incrtNumBy-+
|                                                                             |
+----------------------------------------------------------------------------*/
int CategorizedDbmRecCursor::incrtNumBy(int num, int delta) const
{
   UInt16 newNum = currentNum(num) + delta;
   if ((newNum <= 0) || (newNum > lastNum())) {
      return -1;
   }else {
      newNum = num-1;
      Err err = DmSeekRecordInCategory(
         m_fh,
         &newNum,
         (delta < 0)? -delta : delta,
         (delta < 0)? dmSeekBackward : dmSeekForward,
         m_ixCat
      );
      return newNum+1;
   }
}

/*-----------------------------------------------CategorizedDbmRecCursor::add-+
|                                                                             |
+----------------------------------------------------------------------------*/
MemHandle CategorizedDbmRecCursor::add(int size)
{
   UInt16 attr;
   MemHandle rec = DbmRecCursor::add(size);
   Err err = DmRecordInfo(m_fh, m_num-1, &attr, 0, 0);
   attr &= ~dmRecAttrCategoryMask;
   attr |= (m_ixCat | dmRecAttrDirty);
   err = DmSetRecordInfo(m_fh, m_num-1, &attr, NULL);
   return rec;
}

/*---------------------------------CategorizedDbmRecCursor::removeFollowersTo-+
|                                                                             |
+----------------------------------------------------------------------------*/
Err CategorizedDbmRecCursor::removeFollowersTo(UInt16 lastPos)
{
   int count = lastPos - DmPositionInCategory(m_fh, m_num-1, m_ixCat);
   while (--count) {
      UInt16 temp = m_num;
      DmQueryNextInCategory(m_fh, &temp, m_ixCat);
      DmRemoveRecord(m_fh, temp);
   }
   return DmGetLastErr();
}

#ifdef TOOLS_NAMESPACE
}
#endif
/*===========================================================================*/
