/*
* $Id: iostream.h,v 1.12 2002-07-02 09:51:47 pgr Exp $
*
*  Poor definition of AT&T C++ iostream
*/

#ifndef COM_JAXO_PALMIO_IOSTREAM_H_INCLUDED
#define COM_JAXO_PALMIO_IOSTREAM_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include "ios.h"
#include "streambuf.h"

/*----------------------------------------------------------struct IOManipInt-+
|                                                                             |
+----------------------------------------------------------------------------*/
struct IOManipWidth     { IOManipWidth(int n); int m_n; };
struct IOManipPrecision { IOManipPrecision(int n); int m_n; };
struct IOManipFill      { IOManipFill(char c); char m_c; };

/* -- INLINES -- */
inline IOManipWidth::IOManipWidth(int n) { m_n = n; }
inline IOManipWidth setw(int n) { return IOManipWidth(n); }

inline IOManipPrecision::IOManipPrecision(int n) { m_n = n; }
inline IOManipPrecision setprecision(int n) { return IOManipPrecision(n); }

inline IOManipFill::IOManipFill(char c) { m_c = c; }
inline IOManipFill setfill(char c) { return IOManipFill(c); }

/*--------------------------------------------------------------class istream-+
|                                                                             |
+----------------------------------------------------------------------------*/
class istream : public virtual ios {
public:
   istream(streambuf * sb);
   virtual ~istream() {}
   istream & read(char *,int);
   int peek();
   int get();
   istream & get(char & c);
   istream & get(char * s, streamsize n, char delim);
   istream & seekg(streampos);
   istream & seekg(streamoff, ios::seekdir);
   streampos tellg();
   streamsize  gcount() const;  // Unformatted input

   istream & operator>>(ios& (*f)(ios&));
   istream & operator>>(istream& (*f)(istream&));

protected:
   istream();
   streamsize _M_gcount;
};

inline istream::istream() {
}

/* -- INLINES --*/
inline istream::istream(streambuf * sb) : _M_gcount(0) {
   init(sb);
}
inline streamsize istream::gcount() const {
   return _M_gcount;
}
inline istream & istream::operator>>(ios&(*f)(ios&)) {
   f(*this);
   return *this;
}
inline istream & istream::operator>>(istream&(*f)(istream&)) {
   return f(*this);
}

// Int Manipulators
inline istream & operator>>(istream& is, IOManipWidth m) {
   is.width(m.m_n);
   return is;
}
inline istream & operator>>(istream& is, IOManipPrecision m) {
   is.precision(m.m_n);
   return is;
}
inline istream & operator>>(istream& is, IOManipFill m) {
   is.fill(m.m_c);
   return is;
}

/*--------------------------------------------------------------class ostream-+
|                                                                             |
+----------------------------------------------------------------------------*/
class ostream : public virtual ios {
public:
   ostream(streambuf * sb);
   virtual ~ostream() {}
   ostream & write(const char *, int);
   ostream & put(char c);
   ostream & flush();
   ostream & seekp(streampos);
   ostream & seekp(streamoff, ios::seekdir);
   streampos tellp();

   ostream & operator<<(ios& (*f)(ios&));
   ostream & operator<<(ostream& (*f)(ostream&));
   ostream & operator<<(streambuf * sbIn);
   ostream & operator<<(unsigned char);
   ostream & operator<<(char);
   ostream & operator<<(signed char);
   ostream & operator<<(short);
   ostream & operator<<(unsigned short);
   ostream & operator<<(int);
   ostream & operator<<(unsigned int);
   ostream & operator<<(long);
   ostream & operator<<(unsigned long);
   ostream & operator<<(unsigned char const *);
   ostream & operator<<(char const *);
   ostream & operator<<(signed char const *);

protected:
   ostream();

private:
   void formatNumber(unsigned long n, bool hasSign);
   void writeAndPad(char const * buffer, int length);
};

/* -- INLINES --*/
inline ostream::ostream() {
}
inline ostream::ostream(streambuf * sb) {
   init(sb);
}

inline ostream & ostream::operator<<(ios&(*f)(ios&)) {
   f(*this);
   return *this;
}
inline ostream & ostream::operator<<(ostream&(*f)(ostream&)) {
   return f(*this);
}
inline ostream & ostream::operator<<(streambuf * sbIn) {
   rdbuf()->sget(sbIn);
   return *this;
}
inline ostream & ostream::operator<<(unsigned char c) {
   put((char)c);
   return *this;
}
inline ostream & ostream::operator<<(char c) {
   return operator<<((unsigned char)c);
}
inline ostream & ostream::operator<<(signed char c) {
   return operator<<((unsigned char)c);
}
inline ostream & ostream::operator<<(short n) {
   return operator<<((long)n);
}
inline ostream & ostream::operator<<(int n) {
   return operator<<((long)n);
}
inline ostream & ostream::operator<<(unsigned short n) {
   return operator<<((unsigned long)n);
}
inline ostream & ostream::operator<<(unsigned int n) {
   return operator<<((unsigned long)n);
}
inline ostream & ostream::operator<<(unsigned char const * p) {
   return operator<<((char const *)p);
}
inline ostream & ostream::operator<<(signed char const * p) {
   return operator<<((const char *)p);
}

// Simple Manipulators
inline ostream& endl(ostream& os)  {  os.put('\n'); return os; }
inline ostream& ends(ostream& os)  {  os.put('\0'); return os; }
inline ostream& flush(ostream& os) {  os.flush();   return os; }

// Int Manipulators
inline ostream& operator<<(ostream& os, IOManipWidth m) {
   os.width(m.m_n);
   return os;
}
inline ostream & operator<<(ostream& os, IOManipPrecision m) {
   os.precision(m.m_n);
   return os;
}
inline ostream & operator<<(ostream& os, IOManipFill m) {
   os.fill(m.m_c);
   return os;
}

/*-------------------------------------------------------------class iostream-+
|                                                                             |
+----------------------------------------------------------------------------*/
class iostream : public istream, public ostream {
public:
   iostream(streambuf*);
   virtual ~iostream() {}
protected:
   iostream();
   iostream(const iostream&);
};

/* -- INLINES --*/
inline iostream::iostream(streambuf * sb) {
   init(sb);
}

#endif
/*===========================================================================*/
