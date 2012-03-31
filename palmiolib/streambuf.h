/*
* $Id: streambuf.h,v 1.17 2002-08-29 07:16:28 pgr Exp $
*
*  Basic definition of AT&T C++ streambuf
*/

#ifndef COM_JAXO_PALMIO_STREAMBUF_H_INCLUDED
#define COM_JAXO_PALMIO_STREAMBUF_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include "ios.h"

/*------------------------------------------------------------class streambuf-+
|                                                                             |
+----------------------------------------------------------------------------*/
class streambuf {
public:
   virtual ~streambuf();

   int sgetc();
   int snextc();
   int sbumpc();
   int sgetn(char *,int);
// void stossc();

   int sputc(int);
   int sputbackc(char);
   int sputn(char const *, int);

   streamsize in_avail();

   streampos pubseekoff(
      streamoff off,
      ios::seekdir way,
      ios::openmode mode = (ios::openmode)(ios::in | ios::out)
   );
   streampos pubseekpos(
      streampos sp,
      ios::openmode mode = (ios::openmode)(ios::in | ios::out)
   );
   int pubsync();
   streambuf * pubsetbuf(char * buf, int len);

protected:
   streambuf();
   streambuf(char *,int);

   void setg(char *,char *,char *);
   char * eback() const;
   char * gptr() const;
   char * egptr() const;
   inline void gbump(int);

   void setp(char *,char *);
   char * pbase() const;
   char * pptr() const;
   char * epptr() const;
   void pbump(int);

   virtual streamsize xsputn(char const *, streamsize);
   virtual streamsize xsgetn(char *, streamsize);

   virtual int overflow(int = EOF) = 0;
   virtual int underflow() = 0;

   virtual int pbackfail(int);

   virtual int sync() = 0;
   virtual streambuf * setbuf(char * buf, int len);
   virtual streampos seekoff(
      streamoff off,
      ios::seekdir way,
      ios::openmode mode = (ios::openmode)(ios::in | ios::out)
   ) = 0;
   virtual streampos seekpos(
      streampos sp,
      ios::openmode mode = (ios::openmode)(ios::in | ios::out)
   ) = 0;
   public: int sget(streambuf * sbIn);  // Not standard: PLS, DO NOT USE!

protected:
   char * _M_buf;
   int _M_buf_size;
   char * _M_in_beg;    // Start of get area.
   char * _M_in_cur;    // Current read area.
   char * _M_in_end;    // End of get area.
   char * _M_out_beg;   // Start of put area.
   char * _M_out_cur;   // Current put area.
   char * _M_out_end;   // End of put area.
};

inline streambuf::streambuf() :
   _M_buf(0),
   _M_buf_size(0),
   _M_in_beg(0),
   _M_in_cur(0),
   _M_in_end(0),
   _M_out_beg(0),
   _M_out_cur(0),
   _M_out_end(0)
{
}

inline streambuf::~streambuf() {
}

inline int streambuf::pbackfail(int) {
   return EOF;
}

inline int streambuf::sgetc() {
   if (_M_in_cur >= _M_in_end) {
      return underflow();
   }else {
      return ((unsigned char)(*_M_in_cur));  // aka zapeof
   }
}

inline int streambuf::sbumpc() {
   if (_M_in_cur >= _M_in_end) {
      int c = underflow();
      if ((c != EOF) && (_M_in_cur)) ++_M_in_cur;
      return c;
   }else {
      return ((unsigned char)(*_M_in_cur++));  // aka zapeof
   }
}

inline int streambuf::snextc() {
   return (sbumpc() == EOF) ? EOF : sgetc();
}

inline int streambuf::sgetn(char * str, int n) {
   return xsgetn(str, n);
}

inline int streambuf::sputc(int c) {
   return
     (_M_out_cur < _M_out_end) ?
     (unsigned char)(*(_M_out_cur++)=(char)c) :
     overflow(c);
}

inline int streambuf::sputbackc(char c) { return
   (_M_in_beg < _M_in_cur) ? *(--_M_in_cur)=c : pbackfail(c);
}

inline int streambuf::sputn(char const * str,int n) {
   return xsputn(str, n);
}

inline void streambuf::setg(char * gbeg, char * gcur, char * gend) {
   _M_in_beg=gbeg;
   _M_in_cur=gcur;
   _M_in_end=gend;
}

inline char * streambuf::eback() const { return _M_in_beg; }
inline char * streambuf::gptr() const { return _M_in_cur; }
inline char * streambuf::egptr() const { return _M_in_end; }
inline void streambuf::gbump(int n) { _M_in_cur += n; }

inline void streambuf::setp(char * pBeg, char * pEnd) {
   _M_out_cur = _M_out_beg = pBeg;
   _M_out_end = pEnd;
}

inline char * streambuf::pbase() const { return _M_out_beg; }
inline char * streambuf::pptr() const { return _M_out_cur; }
inline char * streambuf::epptr() const { return _M_out_end; }
inline void streambuf::pbump(int n) { _M_out_cur += n; }

inline streamsize streambuf::in_avail() {
   return _M_in_end - _M_in_cur;
}

inline streampos streambuf::pubseekoff(
   streamoff off,
   ios::seekdir way,
   ios::openmode mode
) {
   return seekoff(off, way, mode);
}

inline streampos streambuf::pubseekpos(
   streampos sp,
   ios::openmode mode
) {
   return seekpos(sp, mode);
}

inline int streambuf::pubsync() { return sync(); }
inline streambuf * streambuf::pubsetbuf(char * buf, int len) {
   return setbuf(buf, len);
}

#endif
/*===========================================================================*/
