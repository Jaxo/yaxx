/*
* $Id: tpblist.cpp,v 1.4 2011-07-29 10:26:38 pgr Exp $
*
* Basic list
*/

// #define DONT_WANT_MAKE_IT_QUICK  - should be working, now
/*--------------+
| Include files |
+--------------*/
#include <new>
#include <string.h>
#include "tpblist.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

TpListBase const TpListBase::Nil;

/*INLINE-PRIVATE----------------------------------------TpListRep::roundToIncr-+
|                                                                              |
+-----------------------------------------------------------------------------*/
inline int TpListRep::roundToIncr(int iCount)
{
   return ((iCount + (INCR-1)) / INCR) * INCR;
}

/*INLINE-PRIVATE------------------------------------------TpListRep::TpListRep-+
|                                                                              |
+-----------------------------------------------------------------------------*/
inline TpListRep::TpListRep(int iAverageCount)
{
   iCount = 0;
   iReferenceCount = 1;
   iRealCount = roundToIncr(iAverageCount);
   aKi = new RefdKey[iRealCount];
}

/*INLINE-PRIVATE------------------------------------------TpListRep::TpListRep-+
| Copy constructor (kinda)                                                     |
|  This routine add, delete or replace KeyedItems                              |
|                                                                              |
|  if iCountKi is > 0, this routine add iCountKi KeyedItems at ixKi            |
|  if iCountKi is < 0, this routine deletes -iCountKi KeyedItems at ixKi       |
|  if iCountKi is ==0, this routine replaces iCountKiRplc KeyedItems at ixKi   |
+-----------------------------------------------------------------------------*/
inline TpListRep::TpListRep(
   bool & isOk,                                // false: memory shortage
   TpListRep const* pRepSource,
   RefdItem const* aKiSource,
   int iCountKi,
   int ixKi,
   int iCountKiRplc
) {
   if (iCountKi) {
     iCountKiRplc = iCountKi;
   }
   iCount = pRepSource? iCountKi + pRepSource->iCount : iCountKi;
   iReferenceCount = 1;
   iRealCount = roundToIncr(iCount);
   if (aKi = new RefdItem[iRealCount], !aKi) {
      isOk = false;
   }else {
      int i, j;
      for (i=0; i < ixKi; ++i) {
         aKi[i] = pRepSource->aKi[i];           // shallow copy existing items
      }
      for (j=0; j < iCountKiRplc; ++i, ++j) {
         aKi[i] = aKiSource[j];                 // shallow copy the new items
      }
      while (i < iCount) {
         aKi[i] = pRepSource->aKi[i-iCountKi];  // shallow copy the rest
         ++i;
      }
   }
}

/*INLINE-PRIVATE-----------------------------------------TpListRep::~TpListRep-+
|                                                                              |
+-----------------------------------------------------------------------------*/
TpListRep::~TpListRep()
{
   delete [] aKi;
   aKi = 0;
   iCount = 0;
}

#ifndef DONT_WANT_MAKE_IT_QUICK
/*------------------------------------------------------TpListRep::quickInsert-+
| Insert iCountKi Item's at ixKi, when iReferenceCount is 1                    |
+-----------------------------------------------------------------------------*/
bool TpListRep::quickInsert(
   RefdItem const* aKiSource, int iCountKi, int ixKi
) {
   if (iCount+iCountKi <= iRealCount) {
      if (ixKi < (int)iCount) {
         memmove(aKi+ixKi+iCountKi, aKi+ixKi, (iCount-ixKi)*sizeof(RefdItem));
         memset(aKi+ixKi, 0, iCountKi * sizeof(RefdItem));
      }
   }else {
      int iRealCountC = roundToIncr(iCount+iCountKi);
      RefdItem * aKiC = new RefdItem[iRealCountC];
      if (!aKiC) {
         return false;
      }
      memcpy(aKiC, aKi, ixKi * sizeof(RefdItem));
      memcpy(aKiC+ixKi+iCountKi, aKi+ixKi, (iCount-ixKi)*sizeof(RefdItem));
      memset(aKi, 0, iCount * sizeof(RefdItem));
      delete [] aKi;
      iRealCount = iRealCountC;
      aKi = aKiC;
   }
   RefdItem * pKiTgt = aKi+ixKi;
   RefdItem const * pKiSrc = aKiSource;
   for (int i=iCountKi; i; --i) {
      *pKiTgt++ = *pKiSrc++;
   }
   iCount += iCountKi;
   return true;
}
#endif

#ifndef DONT_WANT_MAKE_IT_QUICK
/*-----------------------------------------------------TpListRep::quickReplace-+
| Replace iCountKi Item's at ixKi, when iReferenceCount is 1                   |
+-----------------------------------------------------------------------------*/
bool TpListRep::quickReplace(
   RefdItem const* aKiSource, int iCountKi, int ixKi
) {
   RefdItem * pKiTgt = aKi+ixKi;
   RefdItem const* pKiSrc = aKiSource;
   for (int i=iCountKi; i; --i) {
      *pKiTgt++ = *pKiSrc++;
   }
   return true;
}
#endif

#ifndef DONT_WANT_MAKE_IT_QUICK
/*------------------------------------------------------TpListRep::quickRemove-+
| Remove an item at a given position in a list with more than 1 item,          |
| when iReferenceCount is 1                                                    |
+-----------------------------------------------------------------------------*/
bool TpListRep::quickRemove(int ixKi)
{
   // Guarantee free space so we don't bounce across the size threshold.
   if (iCount > (iRealCount - (INCR + (INCR/2)))) {
      aKi[ixKi] = RefdItem::Nil;
      memmove(
         aKi+ixKi,
         aKi+ixKi+1,
         (iCount-ixKi-1)*sizeof(RefdItem)
      );
      memset(aKi+iCount-1, 0, sizeof(RefdItem));
   }else {
      int iRealCountC = roundToIncr(iCount);
      RefdItem * aKiC = new RefdItem[iRealCountC];
      if (!aKiC) {
         return false;
      }
      aKi[ixKi] = RefdItem::Nil;
      memcpy(aKiC, aKi, ixKi * sizeof(RefdItem));
      memcpy(
         aKiC+ixKi,
         aKi+ixKi+1,
         (iCount-ixKi-1)*sizeof(RefdItem)
      );
      memset(aKi, 0, iCount*sizeof(RefdItem));
      delete [] aKi;
      iRealCount = iRealCountC;
      aKi = aKiC;
   }
   --iCount;
   return true;
}
#endif

/*------------------------------------------------------------------TpListBase-+
|                                                                              |
+-----------------------------------------------------------------------------*/
TpListBase::TpListBase(int iAverageCount)
{
   if (!iAverageCount) {
      pRep = 0;
   }else {
      pRep = new TpListRep(iAverageCount);
      if (!pRep || !pRep->aKi) {
         delete pRep;
         pRep = 0;
      }
   }
}
TpListBase::TpListBase(int iCount, e_FillAtInit)
{
   if (!iCount) {
      pRep = 0;
   }else {
      pRep = new TpListRep(iCount);
      if (!pRep || !pRep->aKi) {
         delete pRep;
         pRep = 0;
      }else {
         pRep->iCount = iCount;           // dangerous!
      }
   }
}

/*------------------------------------------------------TpListBase::TpListBase-+
| Copy Constructor - shallow copy                                              |
+-----------------------------------------------------------------------------*/
TpListBase::TpListBase(TpListBase const& lstSource)
{
   pRep = lstSource.pRep;
   if (pRep) {
      ++pRep->iReferenceCount;
   }
}

/*-------------------------------------------------------TpListBase::operator=-+
| Assignement -- shallow                                                       |
+-----------------------------------------------------------------------------*/
TpListBase & TpListBase::operator=(TpListBase const & lstSource)
{
   if (pRep != lstSource.pRep) {
      this->~TpListBase();
      new(this) TpListBase(lstSource);
   }
   return *this;
}

/*--------------------------------------------------------TpListBase::b_insert-+
| Add a shallow copy of an item at a given position in the list                |
| Copies of the list (if any) won't reflect the addition.                      |
+-----------------------------------------------------------------------------*/
RefdItem * TpListBase::b_insert(
   RefdItem const & kiSource, int ixKi
)
{
   if (
      (ixKi < 0) || ((unsigned int)ixKi > b_count()) ||
      ((TpListBase *)this == &TpListBase::Nil)
   ) {
     return 0;
   }
#ifndef DONT_WANT_MAKE_IT_QUICK
   if ((pRep) && (pRep->iReferenceCount == 1)) {
      if (!pRep->quickInsert(&kiSource, 1, ixKi)) {
         return 0;
      }
   }else
#endif
   {
      bool isOk = true;
      TpListRep * pRepC = new TpListRep(isOk, pRep, &kiSource, 1, ixKi);
      if (!pRepC || !isOk) {
         delete pRepC;
         return 0;
      }
      cleanup();
      pRep = pRepC;
   }
   return pRep->aKi + ixKi;
}

/*--------------------------------------------------------TpListBase::b_insert-+
| As above, when the source is already a list                                  |
+-----------------------------------------------------------------------------*/
bool TpListBase::b_insert(TpListBase const& lstSource, int ixKi)
{
   if ((ixKi < 0) || ((unsigned int)ixKi > b_count())) return false;

   if (!lstSource.pRep) {
      return true;
   }
#ifndef DONT_WANT_MAKE_IT_QUICK
   if ((pRep) && (pRep->iReferenceCount == 1) && (lstSource.pRep != pRep)) {
      if (!pRep->quickInsert(
            lstSource.pRep->aKi, lstSource.pRep->iCount, ixKi)
      ) {
         return false;
      }
   }else
#endif
   {
      bool isOk = true;
      TpListRep * pRepC = new TpListRep(
         isOk, pRep, lstSource.pRep->aKi, lstSource.pRep->iCount, ixKi
      );
      if (!pRepC || !isOk) {
         delete pRepC;
         return false;
      }
      cleanup();
      pRep = pRepC;
   }
   return true;
}

/*-------------------------------------------------------TpListBase::b_replace-+
| Replace an item at a given position in the list                              |
| Copies of the list (if any) won't reflect the addition.                      |
+-----------------------------------------------------------------------------*/
RefdItem * TpListBase::b_replace(RefdItem const & kiSource, int ixKi)
{
   if ((ixKi < 0) || ((unsigned int)ixKi > b_count()-1)) return 0;
#ifndef DONT_WANT_MAKE_IT_QUICK
   if ((pRep) && (pRep->iReferenceCount == 1)) {
      if (!pRep->quickReplace(&kiSource, 1, ixKi)) {
         return 0;
      }
   }else
#endif
   {
      bool isOk = true;
      TpListRep * pRepC = new TpListRep(isOk, pRep, &kiSource, 0, ixKi, 1);
      if (!pRepC || !isOk) {
         delete pRepC;
         return 0;
      }
      cleanup();
      pRep = pRepC;
   }
   return pRep->aKi + ixKi;
}

/*-------------------------------------------------------TpListBase::b_replace-+
| As above, when the source is already a list                                  |
+-----------------------------------------------------------------------------*/
bool TpListBase::b_replace(TpListBase const& lstSource, int ixKi)
{
   if ((ixKi < 0) || ((unsigned int)ixKi > b_count()-lstSource.b_count())) {
      return false;
   }
   if (!lstSource.pRep) {
      return true;
   }
#ifndef DONT_WANT_MAKE_IT_QUICK
   if ((pRep) && (pRep->iReferenceCount == 1) && (lstSource.pRep != pRep)) {
      if (!pRep->quickReplace(
            lstSource.pRep->aKi, lstSource.pRep->iCount, ixKi)
      ) {
         return false;
      }
   }else
#endif
   {
      bool isOk = true;
      TpListRep * pRepC = new TpListRep(
         isOk, pRep, lstSource.pRep->aKi, 0, ixKi, lstSource.pRep->iCount
      );
      if (!pRepC || !isOk) {
         delete pRepC;
         return false;
      }
      cleanup();
      pRep = pRepC;
   }
   return true;
}

/*--------------------------------------------------------TpListBase::b_remove-+
| Remove an item at a given position in the list                               |
| Copies of the list (if any) won't reflect the deletion.                      |
+-----------------------------------------------------------------------------*/
bool TpListBase::b_remove(int ixKi)
{
   if ((ixKi < 0) || ((unsigned int)ixKi >= b_count())) return false;

#ifndef DONT_WANT_MAKE_IT_QUICK
   if ((pRep->iReferenceCount == 1) && (pRep->iCount > 1)) {
      if (!pRep->quickRemove(ixKi)) {
         return 0;
      }
   }else
#endif
   {
      TpListRep * pRepC;
      if (pRep->iCount == 1) {
         pRepC = 0;
      }else {
         bool isOk = true;
         pRepC = new TpListRep(isOk, pRep, 0, -1, ixKi);
         if (!pRepC || !isOk) {
            delete pRepC;
            return false;
         }
      }
      cleanup();
      pRep = pRepC;
   }
   return true;
}

/*------------------------------------------------------------TpListBase::b_ix-+
|                                                                              |
+-----------------------------------------------------------------------------*/
int TpListBase::b_ix(RefdItem const * pKi) const
{
   if (pKi) {
      int const iCount = b_count();
      for (int i=0; i < iCount; ++i) {
         if (pRep->aKi[i] == *pKi) return i;
      }
   }
   return -1;
}

/*------------------------------------------------------------TpListBase::b_ix-+
|                                                                              |
+-----------------------------------------------------------------------------*/
int TpListBase::b_ix(RefdItemRep const * pKirep) const
{
   int const iCount = b_count();
   for (int i=0; i < iCount; ++i) {
      if (pRep->aKi[i] == pKirep) return i;
   }
   return -1;
}

#ifdef TOOLS_NAMESPACE
}
#endif
/*===========================================================================*/
