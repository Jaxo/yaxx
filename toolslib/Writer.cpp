/*
* $Id: Writer.cpp,v 1.8 2011-07-29 10:26:37 pgr Exp $
*
* Unicode and Multibyte ostream's
*/

/*---------+
| Includes |
+---------*/
#include <assert.h>
#include <new>
#include "Writer.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif


/*-----------------------------------------------------------Writer::setWidth-+
| First virtual: should not be an inline for basic C++ compilers              |
+----------------------------------------------------------------------------*/
void Writer::setWidth(int w) {
   width(w);
}

/*--------------------------------------------------------------Writer::seekp-+
|                                                                             |
+----------------------------------------------------------------------------*/
Writer & Writer::seekp(streamoff so, ios::seekdir dir) {
   if (EOF == rdbuf()->pubseekoff(so, ios::beg, ios::out)) {
      clear(rdstate() | ios::failbit);
   }
   return *this;
}

/*-----------------------------------------UnicodeMemWriter::UnicodeMemWriter-+
| Constructors                                                                |
+----------------------------------------------------------------------------*/
UnicodeMemWriter::UnicodeMemWriter(
) :
   UnicodeWriter(&mstbuf)
{}
UnicodeMemWriter::UnicodeMemWriter(
   UCS_2 * buf, int iLen
) :
   mstbuf((char *)buf, iLen * sizeof(UCS_2), (char *)buf),
   UnicodeWriter(&mstbuf)
{}

/*---------------------------------------UnicodeFileWriter::UnicodeFileWriter-+
| Constructors                                                                |
+----------------------------------------------------------------------------*/
UnicodeFileWriter::UnicodeFileWriter(
   char const * name, ios__openmode om
) :
   UnicodeWriter(&fstbuf)
{
   fstbuf.open(name, om);
}

/*--- Sigh!  ISO_14882 got rid of fd.
| One way around is to use the "Non-standard constructor" in "std_fstream.h":
| basic_filebuf(
|    __c_file_type * f,                          // a FILE *, not a fd!
|    ios__openmode om,                           // open mode
|    int_type s = static_cast<int_type>(BUFSIZ)  // buffer size
| );
| Note that the first parameter is a FILE *, not a fd!
| The trick is to use FILE * f = fdopen(fd);
|
|
| UnicodeFileWriter::UnicodeFileWriter(
|    int fd
| ) :
|    fstbuf(fd), UnicodeWriter(&fstbuf)
| {}
|
| UnicodeFileWriter::UnicodeFileWriter(
|    int fd, char* p, int l
| ) :
|    fstbuf(fd, p, l), UnicodeWriter(&fstbuf)
| {}
|
*/

/*--------------------------------------UnicodeFileWriter::~UnicodeFileWriter-+
| Destructor                                                                  |
+----------------------------------------------------------------------------*/
UnicodeFileWriter::~UnicodeFileWriter() {
   fstbuf.close();
}

/*-------------------------------------MultibyteMemWriter::MultibyteMemWriter-+
| Constructors                                                                |
+----------------------------------------------------------------------------*/
MultibyteMemWriter::MultibyteMemWriter(
) :
   MultibyteWriter(&mstbuf)
{}
MultibyteMemWriter::MultibyteMemWriter(
   char * buf, int iLen
) :
   mstbuf(buf, iLen, buf),
   MultibyteWriter(&mstbuf)
{}
MultibyteMemWriter::MultibyteMemWriter(
   Encoder const & encoderArg
) :
   MultibyteWriter(&mstbuf, encoderArg)
{}
MultibyteMemWriter::MultibyteMemWriter(
   char * buf, int iLen, Encoder const & encoderArg
) :
   mstbuf(buf, iLen, buf),
   MultibyteWriter(&mstbuf, encoderArg)
{}


/*-----------------------------------MultibyteFileWriter::MultibyteFileWriter-+
| Constructors                                                                |
+----------------------------------------------------------------------------*/
MultibyteFileWriter::MultibyteFileWriter(
   char const * name, ios__openmode om
) :
   MultibyteWriter(&fstbuf)
{
   fstbuf.open(name, om);
}
MultibyteFileWriter::MultibyteFileWriter(
   char const * name, Encoder const & encoderArg, ios__openmode om
) :
   MultibyteWriter(&fstbuf, encoderArg)
{
   fstbuf.open(name, om);
}

/*--- Sigh! ISO_14882 got rid of file descriptors!
| MultibyteFileWriter::MultibyteFileWriter(
|    int fd
| ) :
|    fstbuf(fd), MultibyteWriter(&fstbuf)
| {}
| MultibyteFileWriter::MultibyteFileWriter(
|    int fd, Encoder const & encoderArg
| ) :
|    fstbuf(fd), MultibyteWriter(&fstbuf, encoderArg)
| {}
|
| MultibyteFileWriter::MultibyteFileWriter(
|    int fd, char* p, int l
| ) :
|    fstbuf(fd, p, l), MultibyteWriter(&fstbuf)
| {}
| MultibyteFileWriter::MultibyteFileWriter(
|    int fd, Encoder const & encoderArg , char* p, int l
| ) :
|    fstbuf(fd, p, l), MultibyteWriter(&fstbuf, encoderArg)
| {}
*/


/*----------------------------------MultibyteFileWriter::~MultibyteFileWriter-+
| Destructor                                                                  |
+----------------------------------------------------------------------------*/
MultibyteFileWriter::~MultibyteFileWriter() {
   fstbuf.close();
}

#ifdef TOOLS_NAMESPACE
}
#endif
/*===========================================================================*/
