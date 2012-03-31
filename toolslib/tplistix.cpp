/*
* $Id: tplistix.cpp,v 1.2 2002-02-18 15:55:21 pgr Exp $
*/

/*--------------+
| Include files |
+--------------*/
#include <string.h>
#include "tplistix.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

TpIxList const TpIxList::Nil;

/*-------------------------------------------------------TpNameIxList::inqName-+
| Inquire if a name exists.                                                    |
| Return a reference to its UnicodeString if found, UnicodeString::Nil if not. |
+-----------------------------------------------------------------------------*/
UnicodeString const & TpNameIxList::inqName(
   UnicodeString const & ucsKey
) const {
   Item const * pItem = (Item const *)findData(ucsKey);
   if (pItem) return pItem->key(); else return UnicodeString::Nil;
}

/*---------------------------------------------------------TpNameIxList::inqIx-+
| Inquire about the ix assigned to a name.                                     |
| Return -1 if not found.                                                      |
+-----------------------------------------------------------------------------*/
int TpNameIxList::inqIx(UnicodeString const & ucsKey) const
{
   Item const * pItem = (Item const *)findData(ucsKey);
   if (pItem) return pItem->ix; else return -1;
}

/*-------------------------------------------------TpNameIxList::inqIxOrCreate-+
| Inquire about the ix assigned to a name.                                     |
| If the name is not there, create a new one: deep copy!                       |
+-----------------------------------------------------------------------------*/
int TpNameIxList::inqIxOrCreate(UnicodeString const & ucsKey)
{
   Item const * pItem = (Item const *)findData(ucsKey);
   if (pItem || insertLast(pItem = new Item(ucsKey, count()))) {
      return pItem->ix;
   }else {
      return -1;
   }
}

/*----------------------------------------------------TpNameIxList::operator+=-+
| This operator add a shallow copy of the name.                                |
+-----------------------------------------------------------------------------*/
bool TpNameIxList::operator+=(UnicodeString const & ucsName)
{
   Item const * pItem = (Item const *)findData(ucsName);
   if (pItem || insertLast(new Item(ucsName, count()))) {
      return true;
   }else {
      return false;
   }
}

/*----------------------------------------------TpNameIxXtable::TpNameIxXtable-+
| Regular constructor: union                                                   |
|                                                                              |
| Given a list of keyed items and a reference list of name/index,              |
|    - build a new reference list of name/index which is the union of the      |
|      reference and whatever new is found in the keyed items list             |
|    - create a table of integers, so, given a keyed item 'ki'                 |
|      at position 'pos' in 'lst', the list of keyed items                     |
|      aIxNew[pos] == index within the new reference list of name/index.       |
|                                                                              |
|  Example:                                                                    |
|     Args:                           ==>    TpNameIxXtable:                   |
| (Dtd)  lst = { D, X, A }                    - namelst =  { A, B, C, D, X }   |
| (Pdt)  namelstArg = { A, B, C, D }          - aIxNew = { 4, 5, 1 }           |
|                                               (Dtd -> PdtDtd)                |
|                                                                              |
| Note: all "TpNameIxList" share the same specific common storage for the      |
|       names (deep copy.)                                                     |
+-----------------------------------------------------------------------------*/
TpNameIxXtable::TpNameIxXtable(
   TpListWithHash const & lst, TpNameIxList const & namelstArg
) :
   namelst(namelstArg)
{
   const int iLast = lst.count();
   aIxNew = new int[iLast];
   if (aIxNew) {
      for (int i=0; i < iLast; ++i) {
         RefdKeyRep const * pKi = lst.find(i);
         if (!pKi) {          // pKi might be NULL
            aIxNew[i] = -1;
         }else {
            aIxNew[i] = namelst.inqIxOrCreate(pKi->key());
         }
      }
   }
}

/*---------------------------------------------TpNameIxXtable::~TpNameIxXtable-+
|                                                                              |
+-----------------------------------------------------------------------------*/
TpNameIxXtable::~TpNameIxXtable()
{
   delete [] aIxNew;
}

/*----------------------------------------------------------TpIxList::TpIxList-+
| Copy constructor: deep copy, shrink to exact size                            |
+-----------------------------------------------------------------------------*/
TpIxList::TpIxList(TpIxList const& source)
{
   iCount = iRealCount = source.iCount;
   if (!iCount) {
      aIx = 0;
   }else {
      aIx = new int[iRealCount];
      if (!aIx) {
         iRealCount = iCount = 0;
      }else {
         for (int i=0; i < iCount; ++i) {
            aIx[i] = source.aIx[i];
         }
      }
   }
}

/*INLINE-PRIVATE-----------------------------------------TpIxList::roundToIncr-+
|                                                                              |
+-----------------------------------------------------------------------------*/
inline int TpIxList::roundToIncr(int iCount)
{
   return ((iCount + (INCR-1)) / INCR) * INCR;
}

/*------------------------------------------------------TpIxList::resetIndices-+
| Reset all indices according to the TpNameIxXtable.                           |
+-----------------------------------------------------------------------------*/
void TpIxList::resetIndices(TpNameIxXtable const & ixtbl)
{
   int const iLast = iCount;
   for (int i=0; i < iLast; ++i) {
       aIx[i] = ixtbl[aIx[i]];
   }
}

/*----------------------------------------------------------TpIxList::insertAt-+
| Insert an index at pos                                                       |
+-----------------------------------------------------------------------------*/
bool TpIxList::insertAt(int ix, int pos)
{
   if ((pos < 0) || (pos > iCount)) return false;
   if (iCount < iRealCount) {
      if (pos < iCount) {
         memmove(aIx+pos+1, aIx+pos, (iCount-pos)*sizeof(int));
      }
   }else {
      int iRealCountC = roundToIncr(iCount+1);
      int * aIxC = new int[iRealCountC];
      if (!aIxC) {
         return false;
      }
      memcpy(aIxC, aIx, pos * sizeof(int));
      if (pos < iCount) {
         memcpy(aIxC+pos+1, aIx+pos, (iCount-pos)*sizeof(int));
      }
      delete [] aIx;
      iRealCount = iRealCountC;
      aIx = aIxC;
   }
   aIx[pos] = ix;
   iCount += 1;
   return true;
}

/*----------------------------------------------------------TpIxList::insertAt-+
| Insert iCountSource indeces at pos                                           |
+-----------------------------------------------------------------------------*/
bool TpIxList::insertAt(int const * aIxSource, int iCountSource, int pos)
{
   if ((pos < 0) || (pos > iCount)) return false;
   if (iCount+iCountSource <= iRealCount) {
      if (pos < iCount) {
         memmove(aIx+pos+iCountSource, aIx+pos, (iCount-pos)*sizeof(int));
      }
   }else {
      int iRealCountC = roundToIncr(iCount+iCountSource);
      int * aIxC = new int[iRealCountC];
      if (!aIxC) {
         return false;
      }
      memcpy(aIxC, aIx, pos * sizeof(int));
      if (pos < iCount) {
         memcpy(aIxC+pos+iCountSource, aIx+pos, (iCount-pos)*sizeof(int));
      }
      delete [] aIx;
      iRealCount = iRealCountC;
      aIx = aIxC;
   }
   int * pIxTgt = aIx+pos;
   int const * pIxSrc = aIxSource;
   for (int i=iCountSource; i; --i) {
      *pIxTgt++ = *pIxSrc++;
   }
   iCount += iCountSource;
   return true;
}

/*----------------------------------------------------------TpIxList::removeAt-+
| Remove the index at a given position                                         |
+-----------------------------------------------------------------------------*/
bool TpIxList::removeAt(int pos)
{
   if ((pos < 0) || (pos >= iCount)) return false;
   // Guarantee free space so we don't bounce across the size threshold.
   if (iCount > (iRealCount - (INCR + (INCR/2)))) {
      memmove(
         aIx+pos,
         aIx+pos+1,
         (iCount-pos-1)*sizeof(int)
      );
   }else {
      iRealCount = roundToIncr(iCount);
      int * aIxC = new int[iRealCount];
      if (!aIxC) {
         return false;
      }
      memcpy(aIxC, aIx, pos * sizeof(int));
      memcpy(
         aIxC+pos,
         aIx+pos+1,
         (iCount-pos-1)*sizeof(int)
      );
      delete [] aIx;
      aIx = aIxC;
   }
   --iCount;
   return true;
}

/*-----------------------------------------------------------TpIxList::contain-+
|                                                                              |
+-----------------------------------------------------------------------------*/
bool TpIxList::contain(int ix) const
{
   int const iLast = iCount;
   for (int i=0; i < iLast; ++i) {
       if (aIx[i] == ix) return true;
   }
   return false;
}

/*--------------------------------------------------------TpIxList::operator==-+
| Equality                                                                     |
+-----------------------------------------------------------------------------*/
bool TpIxList::operator==(TpIxList const & source) const {
   int const iLast = iCount;
   for (int i=0; i < iLast; ++i) {
       if (aIx[i] != source.aIx[i]) return false;
   }
   return true;
}

#ifdef TOOLS_NAMESPACE
}
#endif
/*===========================================================================*/
