/*
* $Id: tpbhash.cpp,v 1.6 2011-07-29 10:26:38 pgr Exp $
*
* Basic Hash
*/

/*--------------+
| Include Files |
+--------------*/
#include <new>
#include "tpbhash.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*------------------------------------------------------------TpHashKeyFinder-+
| Internal class to find a given key                                          |
+----------------------------------------------------------------------------*/
class TpHashKeyFinder {
public:
   TpHashKeyFinder(TpListBase & lstBucketArg, UnicodeString const & ucsKey);
   TpHashKeyFinder(TpListBase & lstBucketArg, int ixInBucketArg);
   RefdKey * inqItemCurrent();    // tells matching item found
   RefdKey * insert(RefdKey const& ki) const;
   bool remove() const;
   RefdKey * next();              // get next entry
   bool operator!() const;          // tells a matching item was not found
   int atInBucket() const;             // index of found item or next one
private:
   TpListBase & lstBucket;
   RefdKey * pKiCur;
   int ixInBucket;
   TpHashKeyFinder & operator=(TpHashKeyFinder const & source); // no!
   TpHashKeyFinder(TpHashKeyFinder const & source);             // no!
};

inline TpHashKeyFinder::TpHashKeyFinder(
   TpListBase & lstBucketArg, int ixInBucketArg
) :
   lstBucket(lstBucketArg),
   ixInBucket(ixInBucketArg),
   pKiCur((RefdKey *)lstBucketArg.b_access(ixInBucketArg))
{}
inline RefdKey * TpHashKeyFinder::inqItemCurrent() {
   return pKiCur;
}
inline RefdKey * TpHashKeyFinder::insert(RefdKey const & ki) const {
   return (RefdKey *)lstBucket.b_insert(ki, ixInBucket);
}
inline bool TpHashKeyFinder::remove() const {
   if (pKiCur) {
      return lstBucket.b_remove(ixInBucket);
   }else {
      return false;
   }
}
inline RefdKey * TpHashKeyFinder::next() {
   int i = lstBucket.b_count() - ++ixInBucket;
   if (i > 0) {
      pKiCur = (RefdKey *)lstBucket.b_access(ixInBucket);
   }else {
      ixInBucket += i;
      pKiCur = 0;
   }
   return pKiCur;
}
inline bool TpHashKeyFinder::operator!() const {
   if (pKiCur) return false; else return true;
}
inline int TpHashKeyFinder::atInBucket() const {
   if (pKiCur) return ixInBucket; else return -1;
}

/*-------------------------------------------TpHashKeyFinder::TpHashKeyFinder-+
| Try to find the key, or at least get the index of the next item.            |
| The collision list is kept in alphabetic order, to speed up the search.     |
+----------------------------------------------------------------------------*/
#if ! (defined(OS__EWS) || defined(OS__HPUX))
inline
#endif
TpHashKeyFinder::TpHashKeyFinder(
   TpListBase & lstBucketArg, UnicodeString const & ucsKey
) :
   lstBucket(lstBucketArg)
{
   ixInBucket = 0;
   int const iBucketCount = lstBucket.b_count();
   if (iBucketCount) {
      for (pKiCur = (RefdKey *)lstBucket.b_access(0); ; ++pKiCur) {
         int cond = pKiCur->key().compare(ucsKey);
         if (!cond) return;                         // Match found
         if ((cond > 0) || (++ixInBucket >= iBucketCount)) break;
      }
   }
   pKiCur = 0;
}

/*-------------------------------------------------------------------makeMask-+
|                                                                             |
+----------------------------------------------------------------------------*/
inline int TpHashBase::makeMask(int iSize) {
   int iMask = 1;
   while (iSize > iMask) iMask <<= 1;  // round to next power of 2
   return iMask - 1;
}
inline int TpHashBase::bucketsCount() const {
   return iMask + 1;
}

/*-----------------------------------------------------TpHashBase::TpHashBase-+
| Constructor                                                                 |
| iSize is a number related to the average of number of members the hashtable |
| will contain.  Roughly, it can be seen as the square root of the number     |
| of members.                                                                 |
+----------------------------------------------------------------------------*/
TpHashBase::TpHashBase(unsigned int iSize) :
   iMask(makeMask(iSize)), iCount(0)
{
   if (iSize) {
      aLst =  new TpListBase[bucketsCount()];
   }else {
      aLst = 0;
   }
}

/*-----------------------------------------------------TpHashBase::TpHashBase-+
| Copy Constructor                                                            |
+----------------------------------------------------------------------------*/
TpHashBase::TpHashBase(TpHashBase const& hshSource) :
   iMask(hshSource.iMask), iCount(hshSource.iCount)
{
   int const iBucketsCount = hshSource.bucketsCount();
   if (!hshSource.aLst) {
      aLst = 0;
   }else if (aLst = new TpListBase[iBucketsCount], aLst) {
      for (int i=0; i < iBucketsCount; ++i) {
         aLst[i] = hshSource.aLst[i];
      }
   }
}

/*-----------------------------------------------------TpHashBase::TpHashBase-+
| Constructor from TpListBase                                                 |
+----------------------------------------------------------------------------*/
TpHashBase::TpHashBase(TpListBase const& lstSource, unsigned int iSize) :
   iMask(makeMask(iSize)), iCount(0)
{
   if (aLst = new TpListBase[bucketsCount()], aLst) {
      int const iLast = lstSource.b_count();
      for (int j=0; j < iLast; ++j) {
         RefdKey const * pKi = (RefdKey *)lstSource.b_access(j);
         if (pKi && !b_insert(*pKi)) {
            cleanup();
            break;
         }
      }
   }
}

/*------------------------------------------------------TpHashBase::operator=-+
| Assignement operator                                                        |
+----------------------------------------------------------------------------*/
TpHashBase& TpHashBase::operator=(TpHashBase const& hshSource)
{
   if (this != &hshSource) {
      this->~TpHashBase();
      new(this) TpHashBase(hshSource);
   }
   return *this;
}

/*---------------------------------------------------TpHashBase::chooseBucket-+
| Given a key, find what bucket this item belongs to.                         |
+----------------------------------------------------------------------------*/
inline TpListBase & TpHashBase::chooseBucket(
   UnicodeString const & ucsKey
) const {
   if (ucsKey.good() && aLst) {
      return aLst[ucsKey.inqHashValue() & iMask];
   }else {                             // cast away constness: Nil is const
      return (TpListBase &)TpListBase::Nil;
   }
}

/*---------------------------------------------------TpHashBase::chooseBucket-+
| Given a index, find what bucket this item belongs to and update the index   |
| relatively to this bucket.                                                  |
+----------------------------------------------------------------------------*/
TpListBase & TpHashBase::chooseBucket(int & ixKi) const
{
   if (aLst && (ixKi >= 0) && ((unsigned int)ixKi < iCount)) {
      TpListBase * pLstBucket = aLst;
      int iBucketCount;
      while (iBucketCount = pLstBucket->b_count(), ixKi > iBucketCount) {
         ixKi -= iBucketCount;
         ++pLstBucket;
      }
      return *pLstBucket;
   }else {
      ixKi = -1;
      return (TpListBase &)TpListBase::Nil;
   }
}

/*--------------------------------------------------------TpHashBase::hshfind-+
|                                                                             |
+----------------------------------------------------------------------------*/
RefdKey * TpHashBase::hshfind(UnicodeString const & ucsKey) const
{
   return TpHashKeyFinder(chooseBucket(ucsKey), ucsKey).inqItemCurrent();
}

/*---------------------------------------------------------TpHashBase::b_find-+
| Find at a given index                                                       |
+----------------------------------------------------------------------------*/
RefdKey * TpHashBase::b_find(int ixKi) const
{
   return TpHashKeyFinder(chooseBucket(ixKi), ixKi).inqItemCurrent();
}

/*-------------------------------------------------------TpHashBase::b_insert-+
| Try to access an item of the hash table:                                    |
| - if found, return a pointer to the corresponding KeyedItem                 |
| - if not found:                                                             |
|      create a new Item,                                                     |
|      return a pointer to the new KeyedItem                                  |
+----------------------------------------------------------------------------*/
RefdKey * TpHashBase::b_insert(RefdKey const & ki)
{
   UnicodeString const & ucsKey = ki.key();
   TpHashKeyFinder finder(chooseBucket(ucsKey), ucsKey);
   RefdKey * pKi = finder.inqItemCurrent();
   if (!pKi && (pKi = finder.insert(ki), pKi)) {
      ++iCount;
   }
   return pKi;
}

/*-------------------------------------------------------TpHashBase::b_insert-+
| Same as previous. but returns a status                                      |
+----------------------------------------------------------------------------*/
RefdKey * TpHashBase::b_insert(RefdKey const & ki, bool & isNew)
{
   UnicodeString const & ucsKey = ki.key();
   TpHashKeyFinder finder(chooseBucket(ucsKey), ucsKey);
   RefdKey * pKi = finder.inqItemCurrent();
   if (!pKi) {
      isNew = true;
      if (pKi = finder.insert(ki), pKi) {
         ++iCount;
      }
   }else {
      isNew = false;
   }
   return pKi;
}

/*--------------------------------------------------------TpHashBase::b_force-+
| Force the insertion of an item: i.e. allow duplicates                       |
| Always:                                                                     |
|    create a new Item,                                                       |
|    return a pointer to the new KeyedItem                                    |
+----------------------------------------------------------------------------*/
RefdKey * TpHashBase::b_force(RefdKey const & ki)
{
   UnicodeString const & ucsKey = ki.key();
   TpHashKeyFinder finder(chooseBucket(ucsKey), ucsKey);
   RefdKey * pKi = finder.inqItemCurrent();
   if (pKi) {                              // skip duplicates
      while ((pKi=finder.next(), pKi) && (pKi->key() == ucsKey))
         ;
   }
   if (pKi = finder.insert(ki), pKi) {
      ++iCount;
   }
   return pKi;
}

/*-------------------------------------------------------TpHashBase::b_remove-+
| Remove an item, given its key (supposed to be unique)                       |
+----------------------------------------------------------------------------*/
bool TpHashBase::b_remove(UnicodeString const & ucsKey)
{
   if (TpHashKeyFinder(chooseBucket(ucsKey), ucsKey).remove()) {
      --iCount;
      return true;
   }else {
      return false;
   }
}

/*-------------------------------------------------------TpHashBase::b_remove-+
| Remove an item, given a unique item pointer                                 |
+----------------------------------------------------------------------------*/
bool TpHashBase::b_remove(RefdKey const * pKi)
{
   UnicodeString const & ucsKey = pKi->key();
   TpHashKeyFinder finder(chooseBucket(ucsKey), ucsKey);
   RefdKey const * pKiCur = finder.inqItemCurrent();
   while (pKiCur && (*pKi != *pKiCur)) {
      pKiCur = finder.next();
   }
   if (pKiCur) {
      finder.remove();
      --iCount;
      return true;
   }else {
      return false;
   }
}

/*-------------------------------------------------------TpHashBase::b_remove-+
| Remove an item, given its index                                             |
+----------------------------------------------------------------------------*/
bool TpHashBase::b_remove(int ixKi)
{
   if (TpHashKeyFinder(chooseBucket(ixKi), ixKi).remove()) {
      --iCount;
      return true;
   }else {
      return false;
   }
}

/*-------------------------------------TpHashBaseIterator::TpHashBaseIterator-+
|                                                                             |
+----------------------------------------------------------------------------*/
TpHashBaseIterator::TpHashBaseIterator(
   TpHashBase * pHshArg, UnicodeString const & ucsFrom
) :
   pHsh(pHshArg),
   pKiCur(0),
   pLstBucket(0),
   ixInBucket(0),
   ixKi(-1)
{
   if (pHsh) {
      TpListBase & lstBucketFound = pHsh->chooseBucket(ucsFrom);
      ixInBucket = TpHashKeyFinder(lstBucketFound, ucsFrom).atInBucket();
      if (ixInBucket != -1) {
         ixKi = ixInBucket - 1;       // relative to bucket: make it absolute
         pLstBucket = pHsh->aLst;
         while (pLstBucket != &lstBucketFound) {
            ixKi += pLstBucket++->b_count();
         }
      }else {
         ixInBucket = 0;
      }
   }
}

/*-------------------------------------TpHashBaseIterator::TpHashBaseIterator-+
|                                                                             |
+----------------------------------------------------------------------------*/
TpHashBaseIterator::TpHashBaseIterator(TpHashBase * pHshArg, int ixKiFrom) :
   pHsh(pHshArg),
   pKiCur(0),
   pLstBucket(0),
   ixInBucket(0),
   ixKi(-1)
{
   if (pHsh) {
      int ixFrom = ixKiFrom;
      TpListBase & lstBucket = pHsh->chooseBucket(ixFrom);
      if (ixFrom != -1) {
         pLstBucket = &lstBucket;
         ixInBucket = ixFrom;         // ixFrom is relative (% bucket)
         ixKi = ixKiFrom - 1;         // ixKiFrom is absolute
      }
   }
}

/*---------------------------------------------TpHashBaseIterator::operator()-+
|                                                                             |
+----------------------------------------------------------------------------*/
RefdKey * TpHashBaseIterator::operator()()
{
   if (!pLstBucket) {
      return 0;
   }
   if (ixInBucket >= (int)pLstBucket->b_count()) {
      ixInBucket = 0;
      TpListBase const * const pLstStop = pHsh->aLst + pHsh->bucketsCount();
      // skip over empty lists
      do {
         if (++pLstBucket >= pLstStop) {
            pLstBucket = 0;
            pKiCur = 0;
            return 0;
         }
      }while (!pLstBucket->b_count());
   }
   ++ixKi;
   pKiCur = (RefdKey *)pLstBucket->b_access(ixInBucket++);
   return pKiCur;
}

/*---------------------------------------------TpHashBaseIterator::operator()-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool TpHashBaseIterator::remove()
{
   if (pLstBucket && pLstBucket->b_remove(ixInBucket-1)) {
      --pHsh->iCount;
      --ixKi;
      --ixInBucket;
      return true;
   }else {
      return false;
   }
}

#ifdef TOOLS_NAMESPACE
}
#endif
/*===========================================================================*/
