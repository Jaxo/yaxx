/*
* $Id: tplistwh.cpp,v 1.2 2002-02-18 15:55:21 pgr Exp $
*/

/*--------------+
| Include files |
+--------------*/
#include "tplistwh.h"
#include "ucstring.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*------------------------------------------------------TpListWithHash::remove-+
|                                                                              |
+-----------------------------------------------------------------------------*/
bool TpListWithHash::remove(UnicodeString const & ucsKey)
{
   RefdKey *pKi = hsh.hshfind(ucsKey);
   if (!pKi || !b_remove(b_ix(pKi)) || !hsh.b_remove(ucsKey)) {
      return false;
   }else {
      return true;
   }
}

/*----------------------------------------------------TpListWithHash::removeAt-+
|                                                                              |
+-----------------------------------------------------------------------------*/
bool TpListWithHash::removeAt(int ixKi)
{
   RefdKey *pKi = (RefdKey *)b_find(ixKi);
   if (!pKi || !hsh.b_remove(pKi) || !b_remove(ixKi)) {
      return false;
   }else {
      return true;
   }
}


/*------------------------------------------------TpListWithHash::findDataFrom-+
| Find an item from its key and an index to start from - return its index.     |
| ixKi:                                                                        |
|   - on input, contains the index to start from                               |
|   - on output, will contain the index where the item was found or -1         |
|                                                                              |
| Note 1: to simply get an index, it's much faster to use ix(ucsKey).          |
| Note 2: the (TpHashBase) cast violates constness.                            |
|         but, by just doing inqData(), we guarantee it's const                |
+-----------------------------------------------------------------------------*/
void * TpListWithHash::findDataFrom(
   UnicodeString const & ucsKey,
   int & ixKi
) const {
   TpHashBaseIterator iterate((TpHashBase *)&hsh, ucsKey);
   while (iterate() && (iterate->key() == ucsKey)) {
      int ixKiFound = b_ix(iterate);
      if (ixKiFound >= ixKi) {
         ixKi = ixKiFound;
         return iterate->inqData();
      }
   }
   ixKi = -1;
   return 0;
}

/*-----------------------------------------------TpListWithHash::applyDataFrom-+
| Apply the *f user function to each item.                                     |
+-----------------------------------------------------------------------------*/
void TpListWithHash::applyDataFrom(
   RefdItemRep::ApplyReturnCode (*f)(void *, void *), void * pUser, int ixFrom
) {
   if (b_isAlive() && ixFrom >= 0) {
      int const iCount = b_getCount();
      while (ixFrom < iCount) {
         switch (f(b_access(ixFrom++)->inqData(), pUser)) {
         case RefdItemRep::BreakApply:
            return;
         case RefdItemRep::RemoveItemAndContinueApply:
            removeAt(--ixFrom);
            break;
         default:  /* ContinueApply */
            break;
         }
      }
   }
}

/*----------------------------------------------------TpListWithHash::m_insert-+
| Insert an item.                                                              |
| If the key is new, the item is inserted at the proposed position ixKi.       |
|                                                                              |
| If the key already exists, the new item replaces the previous item at the    |
| position it already had in the list.  Hence, the proposed position "ixKi"    |
| is ignored: the new item inherits the position of the existing one.          |
+-----------------------------------------------------------------------------*/
RefdKey * TpListWithHash::m_insert(
   RefdKey const & ki,
   int ixKi
) {
   bool isNew;
   RefdKey * pKi = hsh.b_insert(ki, isNew);
   if (pKi) {
      if (isNew) {
         if (!b_insert(ki, ixKi)) {
            hsh.b_remove(ki.key());
            return 0;
         }
      }else {
         *b_find(b_ix(pKi)) = ki;
         *pKi = ki;
      }
   }
   return pKi;
}

/*---------------------------------------------------TpListWithHash::m_replace-+
| Replace an item at a given position.                                         |
| If the key is new, the item at the proposed position ixKi is replaced.       |
|                                                                              |
| If the key already exists, the new item replaces the previous item at the    |
| position it already had in the list.  Hence, the proposed position "ixKi"    |
| is ignored: the new item inherits the position of the existing one.          |
+-----------------------------------------------------------------------------*/
RefdKey * TpListWithHash::m_replace(
   RefdKey const& ki,
   int ixKi
) {
   bool isNew;
   RefdKey * pKi = hsh.b_insert(ki, isNew);
   if (pKi) {
      if (isNew) {
         RefdKey * pKiOld = (RefdKey *)b_find(ixKi);
         if (pKiOld) {
            UnicodeString const & ucsKeyOld = pKiOld->key();
            if (!b_replace(ki, ixKi)) {
               hsh.b_remove(ki.key());
               return 0;
            }
            hsh.b_remove(ucsKeyOld);
         }
      }else {
         *b_find(b_ix(pKi)) = ki;
         *pKi = ki;
      }
   }
   return pKi;
}

#ifdef TOOLS_NAMESPACE
}
#endif
/*===========================================================================*/
