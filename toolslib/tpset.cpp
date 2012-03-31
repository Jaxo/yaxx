/*
* $Id: tpset.cpp,v 1.3 2002-02-18 15:55:21 pgr Exp $
*/

/*--------------+
| Include files |
+--------------*/
#include "tpset.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

TpSet const TpSet::Nil(0);

/*----------------------------------------------------------------TpSet::apply-+
|                                                                              |
+-----------------------------------------------------------------------------*/
void TpSet::applyData(                             // for all items...
    RefdItemRep::ApplyReturnCode (*f)(void *, void *),
    void * pUser
) {
   TpHashBaseIterator iterator(this);
   RefdKey * pKi;
   while (pKi = iterator(), pKi) {
      switch (f(pKi->inqData(), pUser)) {
      case RefdItemRep::BreakApply:
         return;
      case RefdItemRep::RemoveItemAndContinueApply:
         iterator.remove();
         break;
      default:  /* ContinueApply */
         break;
      }
   }
}

#ifdef TOOLS_NAMESPACE
}
#endif
/*===========================================================================*/
