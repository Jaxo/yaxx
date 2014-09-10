/*
* $Id: tplistix.h,v 1.7 2011-07-29 10:26:38 pgr Exp $
*
* Lists to associate names and indices
*/

#ifndef COM_JAXO_TOOLS_TPLISTIX_H_INCLUDED
#define COM_JAXO_TOOLS_TPLISTIX_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include <new>
#include "toolsgendef.h"
#include "tplist.h"
#include "tplistwh.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

class UnicodeString;

/*----------------------------------------------------------------TpNameIxList-+
| List of indexed names.                                                       |
| Those are names followed by their current position in the list.              |
| >>> May be we can suppress storing the index in each item?                   |
+-----------------------------------------------------------------------------*/
class TOOLS_API TpNameIxList : private TpListWithHash {
private:
   class Item : public RefdKeyRep {
   public:
      Item(UnicodeString const & nameArg, int ixArg) :
         RefdKeyRep(nameArg), ix(ixArg)
      {}
      Item & operator=(Item const & source); // no!
      Item(Item const & source);             // no!

      int ix;                                // index (always current pos)
   };
public:
   UnicodeString const & inqName(UnicodeString const & ucsKey) const;
   int inqIx(UnicodeString const & ucsKey) const;
   int inqIxOrCreate(UnicodeString const & ucsKey);

   // access adjust
   using TpListWithHash::count;
   using TpListWithHash::findKey;

   // Warning: following method shallow copies the name.
   bool operator+=(UnicodeString const & ucsName); // no duplicate
};

/*--------------------------------------------------------------TpNameIxXtable-+
| This class is used to translate ix (aka id's) from a TpNameIxList.           |
+-----------------------------------------------------------------------------*/
class TOOLS_API TpNameIxXtable {
public:
   TpNameIxXtable(TpListWithHash const & lst, TpNameIxList const & lstnameArg);
   ~TpNameIxXtable();

   operator void *() const;       // isOk?
   bool operator!() const;     // is not ok?

   int operator[](int ixOld) const;
   int count() const;
   UnicodeString const & inqName(int ix) const;
   UnicodeString const & inqName(UnicodeString const & ucsKey) const;
   int inqIx(UnicodeString const & ucsKey) const;
   TpNameIxList const & inqListName() const;
private:
   TpNameIxList namelst;
   int * aIxNew;
};

/* -- INLINES -- */
inline TpNameIxXtable::operator void *() const {
   if (!aIxNew) return 0; else return (void *)this;
}
inline bool TpNameIxXtable::operator!() const {
   if (aIxNew) return false; else return true;
}
inline int TpNameIxXtable::operator[](int ixOld) const {
   if (ixOld == -1) return -1; else return aIxNew[ixOld];
}
inline int TpNameIxXtable::count() const {
   return namelst.count();
}
inline UnicodeString const & TpNameIxXtable::inqName(int ix) const {
   return namelst.findKey(ix);
}
inline UnicodeString const & TpNameIxXtable::inqName(
   UnicodeString const & ucsKey
) const {
   return namelst.inqName(ucsKey);
}
inline int TpNameIxXtable::inqIx(UnicodeString const & ucsKey) const {
   return namelst.inqIx(ucsKey);
}
inline TpNameIxList const & TpNameIxXtable::inqListName() const {
   return namelst;
}

/*--------------------------------------------------------------------TpIxList-+
| List of indexes (integers)                                                   |
+-----------------------------------------------------------------------------*/
class TOOLS_API TpIxList {
   friend class TpIxListIterator;
public:
   TpIxList();
   ~TpIxList();
   TpIxList(TpIxList const & source);          // deep copy
   TpIxList& operator=(TpIxList const & source);

   int count() const;
   int inqIx(int pos) const;
   int & operator[](int pos);     // no check!
   int operator[](int pos) const; // no check!

   bool insertAt(int ix, int pos);
   bool insertAt(int const * aIxSource, int iCountSource, int pos);
   bool replaceAt(int ix, int pos);
   bool removeAt(int pos);
   bool insertFirst(int ix);
   bool replaceFirst(int ix);
   bool removeFirst();
   bool insertLast(int ix);
   bool replaceLast(int ix);
   bool removeLast();

   bool operator+=(int ix);    // insertLast alias
   int & operator++(int);         // lst++ = ix;  -- aka push;
   int operator--();              // ix == --lst; -- aka pop;

   bool contain(int ix) const;
   void resetIndices(TpNameIxXtable const & ixtbl);
   bool operator==(TpIxList const & source) const;
   bool operator!=(TpIxList const & source) const;

private:
   static int roundToIncr(int iCount);
   enum { INCR = 16 };
   int iCount;
   int iRealCount;
   int * aIx;
public:
   static TpIxList const Nil; // should be here: CSET2 bug
};

/* -- INLINES -- */
inline TpIxList::TpIxList() : iCount(0), iRealCount(0), aIx(0) {
}
inline TpIxList::~TpIxList() {
   delete [] aIx;
}
inline int TpIxList::count() const {
   return iCount;
}
inline int TpIxList::inqIx(int pos) const {
   if ((pos >= 0) && (pos < iCount)) return aIx[pos]; else return -1;
}
inline int & TpIxList::operator[](int pos) {
   return aIx[pos];
}
inline int TpIxList::operator[](int pos) const {
   return aIx[pos];
}
inline bool TpIxList::replaceAt(int ix, int pos) {
   if ((pos >= 0) && (pos < iCount)) {
      aIx[pos] = ix;
      return true;
   }else {
      return false;
   }
}
inline bool TpIxList::insertFirst(int ix) {
   return insertAt(ix, 0);
}
inline bool TpIxList::replaceFirst(int ix) {
   return replaceAt(ix, 0);
}
inline bool TpIxList::removeFirst() {
   return removeAt(0);
}
inline bool TpIxList::insertLast(int ix) {
   return insertAt(ix, iCount);
}
inline bool TpIxList::replaceLast(int ix) {
   return replaceAt(ix, iCount-1);
}
inline bool TpIxList::removeLast() {
   return removeAt(iCount-1);
}
inline bool TpIxList::operator+=(int ix) {
   return insertAt(ix, iCount);
}
inline int & TpIxList::operator++(int) {  // postfix (lst++)
   insertAt(-1, iCount);
   return aIx[iCount-1];
}
inline int TpIxList::operator--() {       // prefix (--lst)
   if (iCount) {
      int ix = aIx[iCount-1];
      removeAt(iCount-1);
      return ix;
   }else {
      return -1;
   }
}
inline TpIxList & TpIxList::operator=(TpIxList const & source) {
   if (this != &source) {
      this->~TpIxList();
      new(this) TpIxList(source);
   }
   return *this;
}
inline bool TpIxList::operator!=(TpIxList const & source) const {
   if (*this == source) return false; else return true;
}

/*------------------------------------------------------------TpIxListIterator-+
| Iterator                                                                     |
+-----------------------------------------------------------------------------*/
class TOOLS_API TpIxListIterator {
public:
   TpIxListIterator(TpIxList const & list) :
      i(-1), aIx(list.aIx), iCount(list.iCount) {
   }
   int inqIx()    { return aIx[i]; }
   bool next() {  if (++i < iCount) return true; else { --i; return false; }}
private:
   TpIxListIterator & operator=(TpIxListIterator const & source); // no!
   TpIxListIterator(TpIxListIterator const & source);             // no!
   int const iCount;
   int const * const aIx;
   int i;
};

#ifdef TOOLS_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
