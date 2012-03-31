/*
* $Id: FileStreamBuf.h,v 1.18 2011-07-29 10:26:36 pgr Exp $
*
*  Base streambuf class for streams (either Permanent, or Transient)
*
*  This class defines all the methods inherent to a classic-C++ 'streambuf',
*  and provides a way to easily implement most IO devices as streambufs,
*  whatever these devices relate to transient or permanent data.
*  Misnommed for legacy reasons, 'FileStreamBuf' are 'BasicStreamBuf'.
*
*  An implementation derives from this class and provides classic 'C'
*  methods as read, write and lseek method.  From this point, inheriting
*  from 'FileStreamBuf' makes it a regular streambuf class, will taking
*  care of all the underlying complexity (buffering, switching from read
*  to write, soft-seeks.)
*
*  Read / Write
*  ~~~~~~~~~~~~
*
*  The derived class must provides these two methods.
*
*  The 'read' method is called to read the data from the ultimate
*  producer, and "feeds" the streambuf.  The 'write' method is called
*  when the streambuf is full, and "empties" the buffer.
*
*  At the difference with most streambuf implementations, this class
*  differentiates the read and write position.  It is then possible
*  to read and write at different places.
*
*  TIMEOUT is a constant (-2) which must be returned by 'read' or 'write'
*  when the (transient) device didn't send or receive (respectively)
*  the data.
*
*  If a TIMEOUT is returned, FileStreamBuf throws the exception
*  FileInEmptyException or FileOutFullException.
*
*
*  Note: 'read' and 'write' normally return the length read or written.
*        In the case of a timeout, do not return 0, but TIMEOUT.
*        Returning 0 means the EOF has been reached.
*        Returning EOF (-1) often means... an error.
*
*  Seeks
*  ~~~~~
*
*  The derived class may provide a 'lseek' method.  However, this
*  method will be called only when the position to seek at is not
*  already in the buffer ("soft-seeks".)  This gives some flexibility
*  reading or writing non-physically-seekable streams.
*
*  setbuf
*  ~~~~~~
*
*  The setbuf method can be called to setup an user-defined buffer,
*  or simply setup the size of the internal buffer (for the latter,
*  the first argument -- buffer pointer -- is zero.)
*
*  In read mode, the buffering guarantees to 'unread' a minimal number of
*  characters.  This amount is defined as being a percentage of the
*  size of the buffer (default to BASS_MINUNGET chars.)  The symetric
*  mechanism for writing -- ensuring that one can 'unwrite' a minimal
*  number of characters -- is not actually implemented.  The reason is
*  that, most often, what one reads is not known, but one knows what one
*  writes.
*
*  tieSelf
*  ~~~~~~~
*
*  'tieSelf' is analog to the classic ios::tie() method, at the difference
*  that it takes a boolean (enable) instead of an ostream, and that it is
*  a method of in streambuf (vs. ios)
*  'tieSelf' ties the input and output data.  When switching from write to
*  read, the buffer is flushed: all characters currently waiting to be
*  written are physically written.  Switching from read to write will
*  write at the end of the last chunk physically read from the ultimate
*  producer (see note below.)
*
*  Note: if the ultimate producer still has bytes waiting to be read,
*        the position may be wrong.  However, despite the fact that a
*        position for tied streams is almost always irrelevant, the case
*        where the ultimate producer of a tied stream (serial port) has
*        bytes waiting is rare.  We considered that to issue an extra read,
*        wait for a timeout (meaning: no more to read... for the moment)
*        was not appropriate -- and can be implemented by the derived
*        class if required.
*
*  Streams and Streambufs
*  ~~~~~~~~~~~~~~~~~~~~~~
*
*  If not used to C++ 'stream's, a 'streambuf' should be used in conjunction
*  with a stream.  See the many examples of streams which have been
*  developed (StdFileStream.)
*
*  Also note that this code is not an easy one and wasn't developed in
*  just a week!  Extreme precautions (and tests) should be taken, even
*  if only one line is changed.
*/

#ifndef COM_JAXO_TOOLS_FILESTREAMBUF_H_INCLUDED
#define COM_JAXO_TOOLS_FILESTREAMBUF_H_INCLUDED

/*--------------+
| Include Files |
+--------------*/
#include "toolsgendef.h"

#ifdef __MWERKS__
#include "streambuf.h"
#else
#include <iostream>
#endif

#include "migstream.h"

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

/*---------------------------------------------------------- File Exceptions -+
|                                                                             |
+----------------------------------------------------------------------------*/
class TOOLS_API FileInEmptyException {};
class TOOLS_API FileOutFullException {};

/*--------------------------------------------------------------FileStreamBuf-+
|                                                                             |
+----------------------------------------------------------------------------*/
class TOOLS_API FileStreamBuf : public streambuf {
public:
   FileStreamBuf();
   virtual ~FileStreamBuf();
   enum { TIMEOUT = -2 };

protected:
   #if __GNUC__ >= 4
   char_type * _M_buf;  // Pointer to the internally-allocated buffer
   size_t _M_buf_size;  // Actual size of internally-allocated buffer
   #endif
   /*
   | read and write return the number of characters actually read or written,
   | except that TIMEOUT should be returned instead of 0 when, for a transient
   | stream, no characters can be read or written at the present time.
   | Notes:
   | - TIMEOUT doesn't denote an End-Of-File condition.
   | - -1 (sadly named: EOF) denotes an error
   | - 0 means the end of the file (cannot occur for a write)
   |
   */
   virtual int read(char * pchBuf, int iLen) = 0;
   virtual int write(char const * pchBuf, int iLen) = 0;

   /*
   | Random seek at a given position within the File.
   | offset is simply a count of bytes.
   | origin is one of { SEEK_CUR, SEEK_END, SEEK_SET }
   */
   virtual long lseek(long offset, int origin) { return -1; }
   int sync();
   void resetBuf();
   void tieSelf(bool enable = true);

private:
   #ifdef SMALLBUF_FOR_TESTS
     enum { BASS_BUFSIZE = 64,   BASS_MINUNGET = 16 };
   #else
     enum { BASS_BUFSIZE = 2048, BASS_MINUNGET = 256 };
   #endif

   #if !defined ISO_14882 && !defined __MWERKS__
      int m_bufSize;         // _M_buf_size for 14882
   #endif

   enum IOState {
      IOSTATE_NONE = 0,
      IOSTATE_GET  = 1,
      IOSTATE_PUT  = 2
   } m_ioState;

   int m_minUnget;
   int m_posGet;
   int m_posPut;
   int m_maxOutWaiting;
   bool m_isInternalBuf;
   bool m_isTied;

   int syncIn();
   int syncOut();
   streampos seekIn(streamoff so, ios::seekdir dir);
   streampos seekOut(streamoff so, ios::seekdir dir);
   streampos hardSeek(streamoff so, ios::seekdir dir, IOState newState);
   int underflow();
   int overflow(int);

   int flushSome(int max);
   streamsize xsgetn(char * s, streamsize n);
   streamsize xsputn(char const * s, streamsize n);

   FileStreamBuf & operator=(FileStreamBuf const & source); // no!
   FileStreamBuf(FileStreamBuf const & source);             // no!

#if defined _WIN32
public:
#endif
   streampos seekoff(
      streamoff so,
      ios::seekdir dir,
      ios__openmode om
   );
   streampos seekpos(streampos sp, ios__openmode om) {
      return seekoff(streamoff(sp), ios::beg, om);
   }
   streambuf * setbuf(char * buf, int len);
};

/* -- INLINES -- */
inline void FileStreamBuf::tieSelf(bool enable) {
   m_isTied = enable;
}

#ifdef TOOLS_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
