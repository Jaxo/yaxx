/*
* $Id: RWBuffer.h,v 1.6 2011-07-29 10:26:36 pgr Exp $
*
* Unicode and Multibyte streambuf's.
*/

#ifndef COM_JAXO_TOOLS_RWBUFFER_H_INCLUDED
#define COM_JAXO_TOOLS_RWBUFFER_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include <iostream>
#include <string.h>

#include "toolsgendef.h"
#include "migstream.h"
#include "Encoder.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

#define EOF_SIGNAL (int)-1
#define UCZAPEOF(uc) ((UCS_2)(uc))

/*------------------------------------------------------------UnicodeRWBuffer-+
|                                                                             |
+----------------------------------------------------------------------------*/
class TOOLS_API UnicodeRWBuffer {
   friend class MultibyteRWBuffer;
public:
   UnicodeRWBuffer(streambuf * pSbProdArg);
   virtual ~UnicodeRWBuffer();

   int peek(int iOffset = 0);          // iOffset may be negative
   int peekForward(int iOffset);       // iOffset always >= 0
   int skip();                         // skip one character forward
   int skip(int iToSkip);              // skip iToSkip chars
   int skip_n_count(                   // skip one character forward and count
      UCS_2 ucCounted,
      int & iCount, int & iSinceLast   // updated
   );
   int skip_n_count(                   // skip iToSkip chars and count
      int iToSkip, UCS_2 ucCounted,
      int & iCount, int & iSinceLast   // updated
   );
   int sgetUc();
   bool sputbackUc(UCS_2 uc);
   UCS_2 const * str(int iLen, int iOffset=0);

   // virtual int overflow(int uc);   (PUT: Not Yet Implemented)
   virtual int underflow();
   virtual int sync();
   virtual streampos seekoff(
      streamoff so,
      ios::seekdir dir,
      ios__openmode om
   );

   virtual bool resetEncoding(Encoder const & encoder);
   virtual Encoder const * inqEncoder() const;

   virtual bool enableClone(bool isToEnable);

   streampos seekpos(streampos sp, ios__openmode om);
   int in_back() const;
   int in_avail() const;
   bool isEofReached() const;
   bool isCloned() const;

   bool isOk() const;
   operator void *() const;
   bool operator!() const;

   enum {
      #ifdef SMALLBUF_FOR_TESTS
        UCBUFSIZE=32,
        MINUNGET=8,
      #else
        UCBUFSIZE=300,
        MINUNGET=30,
      #endif
      MINPEEK = -MINUNGET,
      MAXPEEK = UCBUFSIZE-1-MINUNGET-1
   };

private:
   UnicodeRWBuffer() {};

   unsigned int bCatchSoftEof :1;
   unsigned int bEof          :1;
   unsigned int bSoftEofFound :1;
   unsigned int bOk           :1;
   unsigned int bCloneEnabled :1;

   streambuf * pSbProd;
   UCS_2 * x_base;
   UCS_2 * x_pbase;
   UCS_2 * x_pptr;
   UCS_2 * x_epptr;
   UCS_2 * x_gptr;
   UCS_2 * x_egptr;
   UCS_2 * x_eback;
   UCS_2 * x_ebuf;

   void x_reset();
   int tidy();

   UnicodeRWBuffer & operator=(UnicodeRWBuffer const &); // no!
   UnicodeRWBuffer(UnicodeRWBuffer const &);             // no!
};

/* -- INLINES -- */
inline int UnicodeRWBuffer::peekForward(int iOffset) {
   UCS_2 *pUcPeeked = x_gptr + iOffset;    /* OPTIM */
   if (pUcPeeked < x_egptr) {
      return UCZAPEOF(*pUcPeeked);
   }else {
      return peek(iOffset);
   }
}
inline int UnicodeRWBuffer::sgetUc() {
   if ((x_egptr > x_gptr) || (underflow() != EOF_SIGNAL)) {
      return UCZAPEOF(*x_gptr++);
   }else {
      return EOF_SIGNAL;
   }
}
inline bool UnicodeRWBuffer::sputbackUc(UCS_2 uc) {
   if (x_gptr > x_eback) {
      *--x_gptr = uc;
      return true;
   }else {
      return false;
   }
}
inline int UnicodeRWBuffer::in_avail() const {
   return (x_egptr - x_gptr);
}
inline int UnicodeRWBuffer::in_back() const {
   return (x_gptr - x_eback);
}
inline UCS_2 const * UnicodeRWBuffer::str(int iLen, int iOffset) {
   if (
      (iLen > 0) && (EOF_SIGNAL != peek(iLen+iOffset-1)) &&
      ((iOffset >= 0) || (EOF_SIGNAL != peek(iOffset)))
   ) {
      return x_gptr + iOffset;
   }else {
      return 0;
   }
}
inline int UnicodeRWBuffer::skip()
{
   if ((x_egptr > x_gptr) || (underflow() != EOF_SIGNAL)) {
      ++x_gptr;
      return 1;
   }else {
      return skip(1);
   }
}
inline int UnicodeRWBuffer::skip_n_count(
   UCS_2 ucCounted, int & iCount, int & iSinceLast
) {
   if (skip()) {
      if (x_gptr[-1] == ucCounted) {
         iSinceLast = 0;
         ++iCount;
      }else {
         ++iSinceLast;
      }
      return 1;
   }else {
      return 0;
   }
}
inline bool UnicodeRWBuffer::isEofReached() const {
   if ((bEof) && (x_gptr == x_egptr)) return true; else return false;
}
inline bool UnicodeRWBuffer::isCloned() const {
   if (bCloneEnabled) return true; else return false;
}
inline bool UnicodeRWBuffer::isOk() const {
   if (bOk) return true; else return false;
}
inline UnicodeRWBuffer::operator void *() const   {
   if (bOk) return (void *)this; else return 0;
}
inline bool UnicodeRWBuffer::operator!() const {
   if (bOk) return false; else return true;
}

/*----------------------------------------------------------MultibyteRWBuffer-+
|                                                                             |
+----------------------------------------------------------------------------*/
class TOOLS_API MultibyteRWBuffer : public UnicodeRWBuffer, public Encoder {
public:
   MultibyteRWBuffer();
   MultibyteRWBuffer(
      streambuf * pSbProdArg,
      bool isCatchSoftEofArg = true,
      ostream * pOstCloneArg = 0
   );
   MultibyteRWBuffer(
      streambuf * pSbProdArg,
      Encoder const & encoder,
      bool isCatchSoftEofArg = true,
      ostream * pOstCloneArg = 0
   );
   bool attach(                     // new producer, same encoding
      streambuf * pSbProdArg,
      bool isCatchSoftEofArg = true,
      ostream * pOstCloneArg = 0
   );
   bool attach(                     // new producer, new encoding
      streambuf * pSbProdArg,
      Encoder const & encoder,
      bool isCatchSoftEofArg = true,
      ostream * pOstCloneArg = 0
   );
   streambuf * detach();

   // virtual int overflow(int uc);   (PUT: Not Yet Implemented)
   virtual int underflow();
   // if dos files, works only when opened ios::binary!
   virtual int sync();
   virtual streampos seekoff(
      streamoff so,
      ios::seekdir dir,
      ios__openmode om
   );

   virtual bool resetEncoding(Encoder const & encoder);
   virtual Encoder const * inqEncoder() const;

   virtual bool enableClone(bool isToEnable);

   #ifdef SMALLBUF_FOR_TESTS
      enum { UCPERREAD=16, MBBUFLEN=8*UCPERREAD };
   #else
      enum { UCPERREAD=25, MBBUFLEN=8*UCPERREAD };
   #endif
   enum { EOFSOFT = 0x1A };

private:
   ostream * pOstClone;
   int iRemainLenMin;
   int iRemainLen;
   char * pchMbytes;

   void init();
   void setMinRemain();
   void setProducer(streambuf *, bool, ostream *);
   bool reload();
   MultibyteRWBuffer & operator=(MultibyteRWBuffer const &); // no!
   MultibyteRWBuffer(MultibyteRWBuffer const &);             // no!
};

#ifdef TOOLS_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
