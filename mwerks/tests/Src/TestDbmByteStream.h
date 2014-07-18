/* $Id: TestDbmByteStream.h,v 1.1 2002/09/15 07:46:30 pgr Exp $ */

#include "TestBase.h"
class PalmDbmByteStream;

class TestDbmByteStream : public TestBase {
public:
   TestDbmByteStream();
   ~TestDbmByteStream();

protected:
   char const * title();

private:
   ostream * newOutStream();
   istream * newInStream();
   iostream * newInOutStream();
   bool hasFailed();

   static char const fileName[];
   PalmDbmByteStream * pStream;
};

/*===========================================================================*/
