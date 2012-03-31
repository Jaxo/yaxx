/*
* $Id: ios.h,v 1.14 2002-04-26 09:17:35 pgr Exp $
*
*  Basic definition of AT&T C++ ios
*/

#ifndef COM_JAXO_PALMIO_IOS_H_INCLUDED
#define COM_JAXO_PALMIO_IOS_H_INCLUDED

/*---------+
| Includes |
+---------*/
#ifndef EOF
#define EOF (-1)
#endif

class streambuf;

typedef long streampos, streamoff, streamsize;

/*------------------------------------------------------------------class ios-+
|                                                                             |
+----------------------------------------------------------------------------*/
class ios {
public:
   enum iostate {
      goodbit = 0x00,
      eofbit  = 0x01,
      failbit = 0x02,
      badbit  = 0x04
   };

   enum openmode {
      in        = 0x01,
      out       = 0x02,
      ate       = 0x04,
      app       = 0x08,
      trunc     = 0x10,
      nocreate  = 0x20,
      noreplace = 0x40,
      binary    = 0x80
   };

   enum seekdir { beg=0, cur=1, end=2 };

   enum fmtflags {
      skipws      = 0x0001,
      left        = 0x0002,
      right       = 0x0004,
      internal    = 0x0008,
      adjustfield = (left | right | internal),
      dec         = 0x0010,
      oct         = 0x0020,
      hex         = 0x0040,
      basefield   = (dec | oct | hex),
      showbase    = 0x0080,
      showpoint   = 0x0100,
      uppercase   = 0x0200,
      showpos     = 0x0400,
      scientific  = 0x0800,
      fixed       = 0x1000,
      unitbuf     = 0x2000,
      stdio       = 0x4000
   };

   ios(streambuf * sb);
   virtual ~ios();

   operator void*() const;
   bool operator!() const;

   streambuf * rdbuf() const;

   streambuf * rdbuf(streambuf* sb);
   int rdstate() const;
   void clear(int state = goodbit);
   void setstate(int state);
   bool good() const;
   bool eof() const;
   bool fail() const;
   bool bad() const;

   int width() const;
   int width(int i);
   int precision() const;
   int precision(int i);
   char fill() const;
   char fill(char c);
   fmtflags flags() const;
   fmtflags flags(fmtflags i);
   fmtflags setf(fmtflags i);
   fmtflags setf(fmtflags i, fmtflags mask);
   void unsetf(fmtflags mask);

protected:
   ios();
   void init(streambuf * sb);
   streambuf * _M_streambuf;

   int _M_streambuf_state;
   int _M_precision;
   int _M_width;
   char _M_fill;
   fmtflags _M_flags;

};

/* -- INLINES -- */

inline void ios::init(streambuf * sb) {
    _M_streambuf = sb;
    _M_streambuf_state = 0;
    _M_precision = 6;
    _M_width = 0;
    _M_fill = ' ';
    _M_flags = (fmtflags)(skipws | right | dec);
}
inline ios::ios(streambuf * sb) { init(sb); }
inline ios::ios() { init(0); }
inline ios::~ios() {}
inline ios::operator void*() const { return fail() ? 0 : (void *)this; }
inline bool ios::operator!() const { return fail(); }
inline streambuf * ios::rdbuf() const { return _M_streambuf; }

inline streambuf * ios::rdbuf(streambuf* sb) {
   streambuf* old = _M_streambuf;
   _M_streambuf = sb;
   clear();
   return old;
}

inline int ios::rdstate() const { return _M_streambuf_state; }

inline void ios::clear(int state) {
   if (rdbuf()) {
      _M_streambuf_state = state;
   }else {
      _M_streambuf_state = (state | badbit);
   }
}

inline void ios::setstate(int state) { clear(rdstate() | state); }
inline bool ios::good() const { return rdstate() == 0; }
inline bool ios::eof() const { return (rdstate() & eofbit) != 0; }
inline bool ios::fail() const { return (rdstate() & (badbit | failbit)) != 0; }
inline bool ios::bad() const { return (rdstate() & badbit) != 0; }

inline int ios::width() const { return _M_width; }
inline int ios::width(int i) {
  int old = _M_width;
  _M_width = i;
  return old;
}

inline int ios::precision() const { return _M_precision; }
inline int ios::precision(int i) {
  int old = _M_precision;
  _M_precision = i;
  return old;
}

inline char ios::fill() const { return _M_fill; }
inline char ios::fill(char c) {
  char old = _M_fill;
  _M_fill = c;
  return old;
}

inline ios::fmtflags ios::flags() const { return _M_flags; }
inline ios::fmtflags ios::flags(fmtflags i) {
  fmtflags old = _M_flags;
  _M_flags = i;
  return old;
}
inline ios::fmtflags ios::setf(fmtflags i) {
  fmtflags old = _M_flags;
  _M_flags = (fmtflags)(_M_flags | i);
  return old;
}
inline ios::fmtflags ios::setf(fmtflags i, fmtflags mask) {
  fmtflags old = _M_flags;
  _M_flags = (fmtflags)((old & ~mask) | (i & mask));
  return old;
}
inline void ios::unsetf(fmtflags mask) {
  _M_flags = (fmtflags)(_M_flags & ~mask);
}

// IO Manipulators
inline ios& skipws(ios& s) {
  s.setf(ios::skipws);
  return s;
}
inline ios& noskipws(ios& s) {
  s.unsetf(ios::skipws);
  return s;
}
inline ios& left(ios& s) {
  s.setf(ios::left, ios::adjustfield);
  return s;
}

inline ios& right(ios& s) {
  s.setf(ios::right, ios::adjustfield);
  return s;
}

inline ios& dec(ios& s) {
  s.setf(ios::dec, ios::basefield);
  return s;
}

inline ios& hex(ios& s) {
  s.setf(ios::hex, ios::basefield);
  return s;
}

inline ios& oct(ios& s) {
  s.setf(ios::oct, ios::basefield);
  return s;
}

#endif
/*===========================================================================*/
