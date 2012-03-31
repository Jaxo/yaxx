/*
* $Id: TransitionList.h,v 1.2 2002-04-05 09:27:02 pgr Exp $
*
* Example of a List of RefdItems.
*
* The TpList class permits to collect RefdItem in lists.
* Note that TpList instances are also RefdItems.
*/

#include "Transition.h"
#include "../toolslib/tplist.h"

/*----------------------------------------------------------- TransitionList -+
|                                                                             |
+----------------------------------------------------------------------------*/
class TransitionList : public TpList {
public:
    Transition operator[](int ix) {
       return (TransitionRep *)findDataAt(ix);
    }
    void operator+=(Transition & itm) {
       insertLast((RefdItemRep *)(itm.inqData()));
    }
};

/*===========================================================================*/
