/*
* $Id: testTransition.cpp,v 1.4 2002-05-25 08:51:52 pgr Exp $
*
* Example of simple RefdItems.
*
* A Transition, and its derived class NamedTransition,
*  - IS-A RefdItem controlling the construction, assignement and destruction
*  - HAS-A target Transition, e.g it is a single-linked list
*/

#include "Transition.h"

/*-------------------------------------------------------------testTransition-+
|                                                                             |
+----------------------------------------------------------------------------*/
int testTransition()
{
   cout << "--------------- testTransition" << endl;
   Transition dwell;
   NamedTransition mid(&dwell, "Intermediate");
   Transition spring(&mid);

   for (Transition cur = spring; cur.isPresent(); cur = cur->getConnected()) {
      cur->sayHi();
   }
   return 0;
}

/*===========================================================================*/
