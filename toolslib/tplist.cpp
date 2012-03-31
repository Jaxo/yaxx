/*
* $Id: tplist.cpp,v 1.2 2002-02-18 15:55:21 pgr Exp $
*/

/*--------------+
| Include files |
+--------------*/
#include "tplist.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

TpKeyList const TpKeyList::Nil;

/*-------------------------------------------------------TpList::applyDataFrom-+
| Apply the *f user function to each item.                                     |
+-----------------------------------------------------------------------------*/
void TpList::applyDataFrom(
   RefdItemRep::ApplyReturnCode (*f)(void *, void *), void * pUser, int ixFrom
) {
   if (b_isAlive() && ixFrom >= 0) {
      int const iCount = b_count();
      while (ixFrom < iCount) {
         switch (f(b_access(ixFrom++)->inqData(), pUser)) {
         case RefdItemRep::BreakApply:
            return;
         case RefdItemRep::RemoveItemAndContinueApply:
            b_remove(--ixFrom);
            break;
         default:  /* ContinueApply */
            break;
         }
      }
   }
}

/*----------------------------------------------------------TpKeyList::keyfind-+
| Find an item from its key                                                    |
+-----------------------------------------------------------------------------*/
RefdKey * TpKeyList::keyfind(UnicodeString const & ucsKey) const
{
   int const iCount = b_count();
   if (iCount) {
      int ixKi = 0;
      RefdKey * pKi = (RefdKey *)inqArrayPtr();
      do {
         if (pKi->key() == ucsKey) { // found!
            return pKi;
         }
      }while(++pKi, ++ixKi < iCount);
   }
   return 0;
}

/*----------------------------------------------------------TpKeyList::keyfind-+
| Find an item from its key and an index to start from - return its index      |
| ixKi:                                                                        |
|   - on input, contains the index to start from                               |
|   - on output, will contain the index where the item was found or -1         |
+-----------------------------------------------------------------------------*/
RefdKey * TpKeyList::keyfind(
   UnicodeString const & ucsKey,
   int & ixKi
) const {
   if (ixKi < 0) ixKi = 0;
   int const iCount = b_count();
   if (ixKi < iCount) {
      RefdKey * pKi = (RefdKey *)(inqArrayPtr() + ixKi);
      do {
         if (pKi->key() == ucsKey) { // found!
            return pKi;
         }
      }while(++pKi, ++ixKi < iCount);
   }
   ixKi = -1;
   return 0;
}

#ifdef TOOLS_NAMESPACE
}
#endif
/*===========================================================================*/
