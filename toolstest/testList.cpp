/*
* $Id: testList.cpp,v 1.5 2002-05-25 08:51:52 pgr Exp $
*
* Example of a List of RefdItems.
*
* The TpList class permits to collect RefdItem in lists.
* Note that TpList instances are also RefdItems.
*/

#include "TransitionList.h"

/*-------------------------------------------------------------------testList-+
|                                                                             |
+----------------------------------------------------------------------------*/
int testList()
{
   cout << "--------------- testList" << endl;
   TransitionList list;
   Transition dwell;
   char const * const names[] = { "Joe", "Kyle", "Pierre", "Roudoudou" };
   int i;

   for (i=0; i < sizeof names / sizeof names[0]; ++i) {
      NamedTransition nt(&dwell, names[i]);
      list += nt;
   }
   for (i=0; i < sizeof names / sizeof names[0]; ++i) {
      list[i]->sayHi();
   }
   return 0;
}

/*===========================================================================*/
