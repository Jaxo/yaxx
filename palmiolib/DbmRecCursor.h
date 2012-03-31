/*
* $Id: DbmRecCursor.h,v 1.8 2002-09-04 10:08:00 pgr Exp $
*
* Cursor class for Database Records
*
* There are two reasons for this class:
*
* 1) to allow access to a DataBase in the same manner, whatever
*    the caller wants to restrict to a given Category, or not.
*
* 2) to hide most of DmXxxxx calls.  The Palm documentation is
*    highly imprecize and inconsistent.  DbmRecCursor methods
*    are more obvious, and self-documented by using obvious names.
*/

#ifndef COM_JAXO_PALMIO_DBMRECCURSOR_H_INCLUDED
#define COM_JAXO_PALMIO_DBMRECCURSOR_H_INCLUDED

/*---------+
| Includes |
+---------*/

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*------------------------------------------------------- class DbmRecCursor -+
|                                                                             |
+----------------------------------------------------------------------------*/
class DbmRecCursor {
public:
   DbmRecCursor(DmOpenRef const & fh);
   virtual ~DbmRecCursor();

   bool isOpened() const;
   UInt16 currentNum() const;
   virtual UInt16 lastNum() const = 0;

   Err get(char * pTgt, int offset, int len);
   Err put(char const * pSrc, int offset, int len);
   MemHandle skip(int delta);             // return the record after skip
   MemHandle next();                      // same as skip(+1);
   MemHandle prev();                      // same as skip(-1);
   virtual MemHandle add(int size);       // add a new record after current
   MemHandle resize(int size);            // resize current record
   virtual Err removeFollowersTo(UInt16 num) = 0;

protected:
   DmOpenRef const & m_fh;
   MemHandle m_rec;  // current record
   bool m_isLocked;
   bool m_isDirty;
   UInt16 m_num;     // Number of the current record (0 if no records)

   virtual UInt16 currentNum(int num) const = 0;
   virtual int incrtNumBy(int num, int delta) const = 0;
   MemHandle lock();
   void unlock();
};

/* -- INLINES -- */
inline bool DbmRecCursor::isOpened() const {
   return m_rec != 0;
}
inline UInt16 DbmRecCursor::currentNum() const {
  return currentNum(m_num);
}
inline MemHandle DbmRecCursor::next() {
  return skip(1);
}
inline MemHandle DbmRecCursor::prev() {
  return skip(-1);
}

/*--------------------------------------------- class SequentialDbmRecCursor -+
|                                                                             |
+----------------------------------------------------------------------------*/
class SequentialDbmRecCursor : public DbmRecCursor {
public:
   SequentialDbmRecCursor(DmOpenRef const & fh);
   int incrtNumBy(int num, int delta) const;
   UInt16 currentNum(int num) const;
   UInt16 lastNum() const;
   Err removeFollowersTo(UInt16 num);
};

/* -- INLINES -- */
inline UInt16 SequentialDbmRecCursor::currentNum(int num) const {
   return num;
}
inline UInt16 SequentialDbmRecCursor::lastNum() const {
   return DmNumRecords(m_fh);
}
/*-------------------------------------------- class CategorizedDbmRecCursor -+
|                                                                             |
+----------------------------------------------------------------------------*/
class CategorizedDbmRecCursor : public DbmRecCursor {
public:
   CategorizedDbmRecCursor(DmOpenRef const & fh, UInt16 ixCat);
   int incrtNumBy(int num, int delta) const;
   UInt16 currentNum(int num) const;
   UInt16 lastNum() const;
   MemHandle add(int size);
   Err removeFollowersTo(UInt16 num);
private:
   UInt16 const m_ixCat;
};

/* -- INLINES -- */
inline UInt16 CategorizedDbmRecCursor::currentNum(int num) const {
   /*
   | If the record #5 is the 2nd record of the category 'Beef', then
   | 1+DmPositionInCategory(m_fh, 5-1, 'Beef') == 2
   */
   return (num <= 0)? 0 : 1+DmPositionInCategory(m_fh, num-1, m_ixCat);
}
inline UInt16 CategorizedDbmRecCursor::lastNum() const {
   return DmNumRecordsInCategory(m_fh, m_ixCat);
}

#ifdef TOOLS_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
