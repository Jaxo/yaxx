/* $Id: TestBase.h,v 1.1 2002/09/15 07:46:30 pgr Exp $ */

#ifndef TESTSANDSAMPLES_TESTBASE_INCLUDED
#define TESTSANDSAMPLES_TESTBASE_INCLUDED

#include "iostream.h"
class StringBuffer;

/*-------------------------------------------------------------class TestBase-+
|                                                                             |
+----------------------------------------------------------------------------*/
class TestBase {
public:
   void run();
private:
   bool createAndWrite(char const * toWrite);
   bool modify(char const * newGuy);
   bool reread();
   void aliceTest();
   void writeString(iostream * io, bool isLongText);
   void readString(iostream * io);

   static void cleanUpCR(StringBuffer & sb);

   virtual char const * title() = 0;
   virtual ostream * newOutStream() = 0;
   virtual istream * newInStream() = 0;
   virtual iostream * newInOutStream() = 0;
   virtual bool hasFailed() = 0;
};

#endif
/*===========================================================================*/
