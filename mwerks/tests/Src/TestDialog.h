/* $Id: TestDialog.h,v 1.1 2002/09/15 07:46:30 pgr Exp $ */

#ifndef TESTSANDSAMPLES_TESTDIALOG_INCLUDED
#define TESTSANDSAMPLES_TESTDIALOG_INCLUDED

#include "iostream.h"

/*--------------------------------------------------------- class TestDialog -+
|                                                                             |
+----------------------------------------------------------------------------*/
class TestDialog {
public:
   void run();
protected:
   virtual iostream * newInOutStream() = 0;
   virtual char const * title() = 0;
};

#endif
/*===========================================================================*/
