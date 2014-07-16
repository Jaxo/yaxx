/* $Id: TestSerialStream.h,v 1.1 2002/09/15 07:46:30 pgr Exp $ */

#include "TestDialog.h"

/*--------------------------------------------------- class TestSerialStream -+
|                                                                             |
+----------------------------------------------------------------------------*/
class TestSerialStream : public TestDialog {
   iostream * newInOutStream();
   char const * title();
};

/*===========================================================================*/
