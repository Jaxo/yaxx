/* $Id: TestNetStream.h,v 1.1 2002/09/15 07:46:30 pgr Exp $ */

#include "TestDialog.h"

/*------------------------------------------------------ class TestNetStream -+
|                                                                             |
+----------------------------------------------------------------------------*/
class TestNetStream : public TestDialog {
   iostream * newInOutStream();
   char const * title();
};

/*===========================================================================*/
