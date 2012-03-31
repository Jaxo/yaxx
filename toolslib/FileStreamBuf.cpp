/*
* $Id: FileStreamBuf.cpp,v 1.44 2002-10-16 07:16:46 pgr Exp $
*
* File streambuf
*/

/*--------------+
| Include Files |
+--------------*/
#include <assert.h>
#include <stdio.h>             // for SEEK_SET, SEEK_END, SEEK_CUR
#include <string.h>
#include "FileStreamBuf.h"

#if defined ISO_14882 || defined __MWERKS__
#define m_mode        _M_mode
#define m_bufSize     _M_buf_size
#define base()        _M_buf
#define ebuf()        (_M_buf + _M_buf_size)
#define pbase()       _M_out_beg
#define epptr()       _M_out_end
#define setb(b,e)     _M_buf = (b)
#define in_avail()    (_M_in_end - _M_in_cur)
#define out_waiting() (_M_out_cur - _M_out_beg)
#endif

#define myZapeof(c) ((unsigned char)(c))

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*-----------------------------------------------FileStreamBuf::FileStreamBuf-+
|                                                                             |
+----------------------------------------------------------------------------*/
FileStreamBuf::FileStreamBuf() :
   m_minUnget(BASS_MINUNGET),
   m_ioState(IOSTATE_NONE),
   m_posGet(0),
   m_posPut(0),
   m_maxOutWaiting(0),
   m_isInternalBuf(false),
   m_isTied(false)
{
   m_bufSize = BASS_BUFSIZE;
   setb(0, 0);
   setg(0, 0, 0);
   setp(0, 0);
   // setbuf(0, 4);
}

/*------------------------------------------------------FileStreamBuf::setbuf-+
| setbuf can be called at any time.                                           |
| the buf pointer can be 0, the len must be greater than 1.                   |
+----------------------------------------------------------------------------*/
streambuf * FileStreamBuf::setbuf(char * buf, int len)
{
   if (len > 1) {
      if (m_isInternalBuf) {
         delete [] base();
         m_isInternalBuf = false;
      }
      m_bufSize = len;
      m_minUnget = len / 4;
      if (m_minUnget > 256) m_minUnget = 256;
      setb(buf, buf+len);
      setg(buf, buf, buf);
      setp(ebuf(), ebuf());           // epptr() is always at ebuf()
   }
   return this;
}

/*----------------------------------------------------FileStreamBuf::resetBuf-+
| This method is called when the file is closed.                              |
+----------------------------------------------------------------------------*/
void FileStreamBuf::resetBuf() {
   m_posGet = 0;
   m_posPut = 0;
   setg(base(), base(), base());
   setp(ebuf(), ebuf());           // epptr() is always at ebuf()
}

/*----------------------------------------------FileStreamBuf::~FileStreamBuf-+
|                                                                             |
+----------------------------------------------------------------------------*/
FileStreamBuf::~FileStreamBuf() {
   if (m_isInternalBuf) {
      delete [] base();
   }
}

/*------------------------------------------------------FileStreamBuf::syncIn-+
|                                                                             |
+----------------------------------------------------------------------------*/
int FileStreamBuf::syncIn()
{
   int iToSeek = -in_avail();
   if (iToSeek) {                            // send back to ultimate producer
      iToSeek += m_posGet;
      if (EOF == lseek(iToSeek, SEEK_SET)) { // hard seek
         return EOF;
      }
      m_ioState = IOSTATE_GET;
      m_posGet = iToSeek;
   }
   setg(base(), base(), base());
   return 0;
}

/*-----------------------------------------------------FileStreamBuf::syncOut-+
|                                                                             |
+----------------------------------------------------------------------------*/
int FileStreamBuf::syncOut()
{
   int outWaiting = out_waiting();
   if (m_maxOutWaiting >  outWaiting) outWaiting = m_maxOutWaiting;
   m_maxOutWaiting = 0;
   while (outWaiting) {
      outWaiting = flushSome(outWaiting);
      if (EOF == outWaiting) return EOF;
   }
   setp(ebuf(), ebuf());
   return 0;
}

/*---------------------------------------------------FileStreamBuf::underflow-+
| When gptr == egptr...                                                       |
+----------------------------------------------------------------------------*/
int FileStreamBuf::underflow()
{
   /*---
   | Visual C++ has bad manners, like:
   | 1) calling underflow for no reason (gptr < egptr); or,
   | 2) calling underflow for good, but with gptr() > egptr()
   |    -- b/c their sbumpc() increments the gptr() even for EOF.
   +--*/
   if (gptr() != egptr()) {
      if (gptr() < egptr()) {
         return myZapeof(*gptr());
      }
      gbump(egptr() - gptr());             // MSVC sbumpc bug.
   }

   if (!base()) {
      setbuf(new char[m_bufSize], m_bufSize);
      m_isInternalBuf = true;
   }
   char * pchBufBeg;
   char * pchBufEnd = ebuf();
   int iLen;
   if (m_ioState == IOSTATE_PUT) {
      assert ((0 == in_avail()) && (egptr() == base()));
      if (syncOut() == EOF) return EOF;
      if (m_isTied) {
         m_posGet = m_posPut;
      }else {
         if (EOF == lseek(m_posGet-m_posPut, SEEK_CUR)) return EOF;
      }
      pchBufBeg = base();
      iLen = pchBufEnd - pchBufBeg;
   }else {
      assert ((0 == out_waiting()) && (pbase() == pchBufEnd));
      if (gptr() == pchBufEnd) {               // wrapped
         memcpy(base(), gptr() - m_minUnget, m_minUnget);
         pchBufBeg = base() + m_minUnget;
         iLen = m_bufSize - m_minUnget;
      }else {
         pchBufBeg = gptr();
         iLen = pchBufEnd - pchBufBeg;
      }
   }
   m_ioState = IOSTATE_GET;
   iLen = read(pchBufBeg, iLen);
   switch (iLen) {
   case TIMEOUT:
      throw FileInEmptyException();
      break;
   case EOF:
      /*
      | unfortunately, underflow doesn't know how to tell an error
      | later, we should throw an underflow error
      */
      iLen = 0;
      break;
   default:
      break;
   }
   m_posGet += iLen;
   setg(base(), pchBufBeg, pchBufBeg+iLen);
   if (!iLen) {                             // iLen == 0 means EOF
      return EOF;
   }else {
      return myZapeof(*pchBufBeg);
   }
}

/*---------------------------------------------------FileStreamBuf::flushSome-+
| This assumes max bytes are waiting for output.                              |
| It returns the number of bytes that were not flushed (vs: max)              |
+----------------------------------------------------------------------------*/
int FileStreamBuf::flushSome(int max)
{
   assert (max);
   if (
      (m_ioState == IOSTATE_GET) &&
      (EOF == lseek(m_posPut-m_posGet, SEEK_CUR))
   ) {
      return EOF;
   }
   m_ioState = IOSTATE_PUT;
   int iLen = write(pbase(), max);
   if (iLen < max) {
      switch (iLen) {
      case 0:           // EOF on put is generally is an error
      case EOF:
         return EOF;
      case TIMEOUT:
         throw FileOutFullException();
         break;
      default:
         max -= iLen;
         memcpy(base(), base()+iLen, max);
         setp(base(), ebuf());
         pbump(max);
      }
   }else {
      max = 0;
      setp(base(), ebuf());
   }
   m_posPut += iLen;
   return max;
}

/*----------------------------------------------------FileStreamBuf::overflow-+
| When pptr == epptr...                                                       |
+----------------------------------------------------------------------------*/
int FileStreamBuf::overflow(int c)
{
   if (m_ioState == IOSTATE_GET) {
      assert ((0 == out_waiting()) && (pbase() == ebuf()));
      if (m_isTied) {
         m_posPut = m_posGet;
      }else {
         if (EOF == lseek(m_posPut-m_posGet, SEEK_CUR)) return EOF;
         m_posGet -= in_avail();
      }
      setg(base(), base(), base());
      setp(base(), ebuf());
   }else {
      assert ((0 == in_avail()) && (egptr() == base()));
      if (pptr() == epptr()) {
         /*
         | the buffer is full, so we don't need to examine
         | m_maxOutputWaiting
         */
         int outWaiting = out_waiting();
         if (outWaiting) {
            if (EOF == flushSome(outWaiting)) return EOF;
         }else {
            if (!base()) {
               setbuf(new char[m_bufSize], m_bufSize);
               m_isInternalBuf = true;
            }
            setp(base(), ebuf());
         }
         m_maxOutWaiting = 0;
      }
   }
   m_ioState = IOSTATE_PUT;
   if (c != EOF) {
      sputc(c);
      return c;
   }else {
      return EOF;
   }
}

/*-----------------------------------------------------FileStreamBuf::seekoff-+
| The derived class should not define seekoff, but lseek instead,             |
| or it must call this seekoff first.                                         |
+----------------------------------------------------------------------------*/
streampos FileStreamBuf::seekoff(
   streamoff so,
   ios::seekdir dir,
   ios__openmode om
) {
   #ifdef _WIN32
      if (gptr() > egptr()) {
         gbump(egptr() - gptr());       // MSVC sbumpc bug.
      }
   #endif
   streampos ret = EOF;
   if (om & ios::in) {
      ret = seekIn(so, dir);
//    if (ret == EOF) return EOF;      Unix chokes on this one
      if (ret < 0) return EOF;
   }
   if (om & ios::out) {
      ret = seekOut(so, dir);
//    if (ret == EOF) return EOF;      Unix chokes on this one
      if (ret < 0) return EOF;
   }
   return ret;
}

/*------------------------------------------------------FileStreamBuf::seekIn-+
|                                                                             |
+----------------------------------------------------------------------------*/
streampos FileStreamBuf::seekIn(streamoff so, ios::seekdir dir)
{
   int iBump;
   int iPos = gptr() - base();
   switch (dir) {
   case ios::beg:
      iBump = (int)((long)so - ((long)m_posGet - in_avail()));
      iPos += iBump;
      break;
   case ios::cur:
      iBump = (int)so;
      iPos += iBump;
      so = so + m_posGet - in_avail(); // set hard value
      dir = ios::beg;
      break;
   default: /* ios::end: */          // cannot do soft seeks
      iPos = -1;
      break;
   }
   if ((0 <= iPos) && (iPos <= (egptr() - base()))) {
      gbump(iBump);
      return m_posGet - in_avail();  // soft seek: success!
   }else {
      return hardSeek(so, dir, IOSTATE_GET);
   }
}

/*-----------------------------------------------------FileStreamBuf::seekOut-+
|                                                                             |
+----------------------------------------------------------------------------*/
streampos FileStreamBuf::seekOut(streamoff so, ios::seekdir dir)
{
   int iBump;
   int iPos;
   int outWaiting = out_waiting();
   if (outWaiting > m_maxOutWaiting) {
      m_maxOutWaiting = outWaiting;
   }
   switch (dir) {
   case ios::beg:
      iPos = (int)((long)so - (long)m_posPut);
      iBump = iPos - outWaiting;
      break;
   case ios::cur:
      iBump = (int)so;
      iPos = outWaiting + iBump;
      so = iPos + m_posPut;          // set lseek hard value (% cur)
      dir = ios::beg;
      break;
   default: /* ios::end: */          // cannot do soft seeks
      iPos = -1;
      break;
   }
   if ((0 <= iPos) && (iPos <= m_maxOutWaiting)) {
      pbump(iBump);
      return m_posPut + iPos;        // soft seek: success!
   }else {
      return hardSeek(so, dir, IOSTATE_PUT);
   }
}

/*----------------------------------------------------FileStreamBuf::hardSeek-+
|                                                                             |
+----------------------------------------------------------------------------*/
streampos FileStreamBuf::hardSeek(
   streamoff so,
   ios::seekdir dir,
   IOState newState
) {
   if (syncOut() == EOF) return EOF;
   switch (dir) {
   case ios::beg:
      so = lseek(so, SEEK_SET);
      break;
   case ios::end:
      so = lseek(so, SEEK_END);
      break;
   default:
      assert (false);
      break;
   }
   if (so != -1) {                       // success!
      m_ioState = newState;
      if (newState == IOSTATE_GET) {
         m_posGet = so;
      }else {
         m_posGet -= in_avail();
         m_posPut = so;
      }
      setg(base(), base(), base());
      return so;                         // that's it.
   }else {
      return EOF;
   }
}

/*--------------------------------------------------------FileStreamBuf::sync-+
|                                                                             |
+----------------------------------------------------------------------------*/
int FileStreamBuf::sync() {
   if (m_isTied) {
      if (m_ioState == IOSTATE_PUT) {
         syncOut();
         m_posGet = m_posPut;
      }else {
         setg(base(), base(), base());
         m_posPut = m_posGet;
      }
      return m_posPut;
   }else {
      //>>>PGR: FIXME.  this looks wrong.
      syncOut();
      return syncIn();     //>>>PGR: should vanish
   }
}

/*------------------------------------------------------FileStreamBuf::xsgetn-+
| This xsgetn differs from streambuf::xsgetn because it handles transient     |
| streams in a different way than the regular xsgetn.                         |
| If a physical read doesn't fill up the buffer, then no other attempt to     |
| read will be made.                                                          |
+----------------------------------------------------------------------------*/
streamsize FileStreamBuf::xsgetn(char * pchBuf, streamsize n)
{
   bool mustStop = false;
   int remain = n;
   try {
     while (remain) {
        if (gptr() >= egptr()) { // use '>=', not '=='  b/c of MSVC sbumpc bug
           if (underflow() == EOF) break;
           /*
           | If no new bytes were read, then a FileInEmptyException was thrown.
           | If egptr() < ebuf(), then it's either a pending EOF
           | or there were not enough bytes currently available for reading.
           | In both case, do not call underflow again and break after filling
           | the caller 'pchBuf' buffer with what has been gotten.
           | The ultimate consumer may be satisfied with just that.
           */
           mustStop = (egptr() < ebuf());
        }
        int avail = in_avail();
        int count = (remain < avail)? remain : avail;
        assert (count > 0);
        memcpy(pchBuf, gptr(), count);
        gbump(count);
        remain -= count;
        if (mustStop) break;
        pchBuf += count;
     }
   }catch (FileInEmptyException) {
       if (remain == n) throw;
   }
   return n - remain;
}

/*------------------------------------------------------FileStreamBuf::xsputn-+
| This xsputn differs from streambuf::xsputn because it handles transient     |
| streams in a different way than the regular xsputn.                         |
| If a physical write didn't write all bytes in the buffer, then no other     |
| attempt to write will be made.                                              |
+----------------------------------------------------------------------------*/
streamsize FileStreamBuf::xsputn(char const * pchBuf, streamsize n)
{
   bool mustStop = false;
   int remain = n;
   try {
      while (remain) {
         if (pptr() == epptr()) {
            if (overflow((unsigned char)*pchBuf) == EOF) break;
            /*
            | at the difference with xsgetn, *pchBuf was entered...
            | one byte was written, for sure!
            | - if no bytes were written, a FileOutFullException was thrown
            |   and this method exited
            | - if only one byte was written and there are no more to write,
            |   that's it.  return
            | - otherwise there are more to write; examine if the underlying
            |   file was currently unable to eat all bytes in the buffer.
            |   If not, we will not attempt any other overflow.  Hence:
            |   (this assumes an output buffer > 1, e.g.: non unbuffered)
            |   - if (epptr() == pptr()), then only one byte was written,
            |     (not the full buffer) and no room is left: : break.
            |   - if (pptr + 1 > pbase), then some > 1 bytes were written,
            |     (not the full buffer) and some room was made.
            |     move all the bytes we can, and break.
            */
            if ((--remain == 0) || (pptr() == ebuf())) break;
            ++pchBuf;
            mustStop = (pptr()-1 > pbase());
         }
         int room = ebuf() - pptr();
         int count = (remain < room)? remain : room;
         assert (count > 0);
         memcpy(pptr(), pchBuf, count);
         pbump(count);
         remain -= count;
         if (mustStop) break;
         pchBuf += count;
      }
   }catch (FileOutFullException) {
       if (remain == n) throw;
   }
   return n - remain;
}

/*-------------------------------------------------------------------ansiRead-+
| Description:                                                                |
|    This read is provided to fix a bug in the Visual C++ istream class.      |
|    use it as                                                                |
|      ansiRead(myIstream, buf, n)                                            |
|    instead of:                                                              |
|      myIstream.read(buf, n);  // does "readsome" in Visual                  |
|                                                                             |
| Warning: it does not update the gcount!                                     |
+----------------------------------------------------------------------------*/
#ifdef _WIN32
bool ansiRead(istream& is, char * buf, int n)
{
   int x_gcount = 0;
   if (n > 0) {
      streambuf * sb = is.rdbuf();
      do {
         int c = sb->sbumpc();
         if (c == EOF) {
            return false; // is.setstate(ios::eofbit | ios::failbit);
         }
         *buf++ = c;
      }while (++x_gcount < n);
   }
   return true;
}
#endif

#ifdef TOOLS_NAMESPACE
}
#endif
/*===========================================================================*/
