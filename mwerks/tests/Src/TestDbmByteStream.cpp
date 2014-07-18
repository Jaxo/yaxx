/* $Id: TestDbmByteStream.cpp,v 1.1 2002/09/15 07:46:30 pgr Exp $ */

#include "TestDbmByteStream.h"
#include "PalmDbmStream.h"

static bool readMemoDb();

#if 0
   char const TestDbmByteStream::fileName[] = "HelloDB";
#else
   char const TestDbmByteStream::fileName[] = "MemoDB";
#endif

/*---------------------------------------------------TestDbmByteStream::title-+
|                                                                             |
+----------------------------------------------------------------------------*/
char const * TestDbmByteStream::title() {
   return "TestDbmByteStream";
}

/*---------------------------------------TestDbmByteStream::TestDbmByteStream-+
|                                                                             |
+----------------------------------------------------------------------------*/
TestDbmByteStream::TestDbmByteStream() {
   pStream = 0;
}

/*--------------------------------------TestDbmByteStream::~TestDbmByteStream-+
|                                                                             |
+----------------------------------------------------------------------------*/
TestDbmByteStream::~TestDbmByteStream() {
   delete pStream;
}

/*--------------------------------------------TestDbmByteStream::newOutStream-+
|                                                                             |
+----------------------------------------------------------------------------*/
ostream * TestDbmByteStream::newOutStream() {
   if (pStream) delete pStream;
   pStream = new PalmDbmByteStream(fileName, dmModeWrite);
   return pStream;
}

/*---------------------------------------------TestDbmByteStream::newInStream-+
|                                                                             |
+----------------------------------------------------------------------------*/
istream * TestDbmByteStream::newInStream() {
   if (pStream) delete pStream;
   pStream = new PalmDbmByteStream(fileName);
   return pStream;
}

/*------------------------------------------TestDbmByteStream::newInOutStream-+
|                                                                             |
+----------------------------------------------------------------------------*/
iostream * TestDbmByteStream::newInOutStream() {
   if (pStream) delete pStream;
   pStream = new PalmDbmByteStream(fileName);
   return pStream;
}

/*-----------------------------------------------TestDbmByteStream::hasFailed-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool TestDbmByteStream::hasFailed() {
   if (pStream && !*pStream) {
      ErrAlert(pStream->getLastError());
      return true;
   }else {
      return false;
   }
}

/*-----------------------------------------------------------------readMemoDb-+
|                                                                             |
+----------------------------------------------------------------------------*/
#if 0
static bool readMemoDb()
{
   MemStream msg;
   char buffer[200];
   PalmDbmByteStream is("MemoDB", dmModeReadOnly);

   if (hasFailed(is)) return false;
   do {
      memset(buffer, '$', sizeof buffer);
      is.read(buffer, sizeof buffer-1);
      is.clear();

      buffer[is.gcount()] = '\0';
      msg << "Read the following " << is.gcount()
          << "bytes: \"" << buffer << "\"";
      display(msg.str(), msg.pcount());
      msg.rdbuf()->freeze(0);
      msg.reset();
   }while (is.gcount());
   return true;
}
#endif

/*===========================================================================*/
