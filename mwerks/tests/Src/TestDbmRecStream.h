/* $Id: TestDbmRecStream.h,v 1.1 2002/09/15 07:46:30 pgr Exp $ */

#include "TestBase.h"
class PalmDbmRecStream;

class TestDbmRecStream : public TestBase {
public:
   TestDbmRecStream();
   ~TestDbmRecStream();

protected:
   char const * title();

private:
   ostream * newOutStream();
   istream * newInStream();
   iostream * newInOutStream();
   bool hasFailed();

   static char const fileName[];
   PalmDbmRecStream * pStream;
};

/*===========================================================================*/
