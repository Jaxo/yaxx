/*
* $Id: StdFileStream.h,v 1.12 2011-07-29 10:26:37 pgr Exp $
*
*  Base classes for file streams (either Permanent, or Transient)
*
*  StdFileStreamBuf:
*     is an example of a derived class that uses fread() and
*     fwrite() to feed/empty the buffer.
*
*  StdFileInStream and StdFileOutStream:
*     are istream/ostream using a StdFileStreamBuf
*     (as: ifstream/ofstream)
*/

#ifndef COM_JAXO_TOOLS_STDFILESTREAM_H_INCLUDED
#define COM_JAXO_TOOLS_STDFILESTREAM_H_INCLUDED

#ifdef __MWERKS__
#error Error: this file should not be used for systems with no native Basic IO
#endif

/*--------------+
| Include Files |
+--------------*/
#include <stdlib.h>
#include <fcntl.h>
#include "FileStreamBuf.h"
#include "toolsgendef.h"
#include "migstream.h"
#include "URI.h"


#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

#ifdef _WIN32
/*
| This read is provided to fix a bug in the Visual C++ istream class.
| use it as
|   ansiRead(myIstream, buf, n)
| instead of:
|   myIstream.read(buf, n);  // does "readsome" in Visual
|
| Warning: it does not update the gcount!
*/
extern TOOLS_API bool ansiRead(istream&, char * buf, int n);
#endif

/*-----------------------------------------------------------StdFileStreamBuf-+
| StdFileStreamBuf IS-A FileStreamBuf that uses the standard file system of   |
| the C library.                                                              |
+----------------------------------------------------------------------------*/
class TOOLS_API StdFileStreamBuf : public FileStreamBuf {
public:
   enum {
      OPEN_READ_FLAGS = O_RDONLY,
      OPEN_WRITE_FLAGS = O_WRONLY|O_CREAT|O_TRUNC,
      OPEN_RDWR_FLAGS = O_RDWR|O_CREAT
   };

   StdFileStreamBuf();

   FileStreamBuf * open(char const * pszPath, unsigned long opflgs);
   FileStreamBuf * close();
   static char * makeTempPath(char * buf, char const * suffix);
   static char * makeBaseUri();

private:
   int m_fd;

   int read(char * pchBuf, int iLen);
   int write(char const * pchBuf, int iLen);
   long lseek(long offset, int origin);

   StdFileStreamBuf & operator=(StdFileStreamBuf const & source); // no!
   StdFileStreamBuf(StdFileStreamBuf const & source);             // no!
};

/*------------------------------------------------------------StdFileInStream-+
| StdFileInStream IS-A istream to manipulate a StdFileStreamBuf.              |
| We could have use ifstream, but bugs in Microsoft VC++ prevented it.  :-(   |
+----------------------------------------------------------------------------*/
class TOOLS_API StdFileInStream : public istream {
public:
   StdFileInStream(
      char const * pszPath,
      unsigned long opflgs = StdFileStreamBuf::OPEN_READ_FLAGS
   );
   ~StdFileInStream();

private:
   StdFileStreamBuf m_buf;

   StdFileInStream & operator=(StdFileInStream const & source);   // no!
   StdFileInStream(StdFileInStream const & source);               // no!
};

/*-----------------------------------------------------------StdFileOutStream-+
| StdFileOutStream IS-A ostream to manipulate a StdFileStreamBuf.             |
+----------------------------------------------------------------------------*/
class TOOLS_API StdFileOutStream : public ostream {
public:
   StdFileOutStream(
      char const * pszPath,
      unsigned long opflgs = StdFileStreamBuf::OPEN_WRITE_FLAGS
   );
   ~StdFileOutStream();

private:
   StdFileStreamBuf m_buf;

   StdFileOutStream & operator=(StdFileOutStream const & source);   // no!
   StdFileOutStream(StdFileOutStream const & source);               // no!
};

/*--------------------------------------------------------------StdFileStream-+
| StdFileStream IS-A iostream to manipulate a StdFileStreamBuf.               |
+----------------------------------------------------------------------------*/
class TOOLS_API StdFileStream : public iostream {
public:
   StdFileStream(
      char const * pszPath,
      unsigned long opflgs = StdFileStreamBuf::OPEN_RDWR_FLAGS
   );
   ~StdFileStream();

private:
   StdFileStreamBuf m_buf;

   StdFileStream & operator=(StdFileStream const & source);   // no!
   StdFileStream(StdFileStream const & source);               // no!
};

/*----------------------------------------------- class StdFileSchemeHandler -+
|                                                                             |
+----------------------------------------------------------------------------*/
class TOOLS_API StdFileSchemeHandler : public URI::SchemeHandler {
public:
   StdFileSchemeHandler();

private:
   class Rep : public URI::SchemeHandler::Rep {
      char const * getID() const;
      iostream * makeStream(URI const & uri, ios__openmode om);
   };
};

inline StdFileSchemeHandler::StdFileSchemeHandler() : SchemeHandler(new Rep) {
}

#ifdef TOOLS_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
