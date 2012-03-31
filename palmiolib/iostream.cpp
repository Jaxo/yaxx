/*
* $Id: iostream.cpp,v 1.13 2002-10-09 07:15:22 pgr Exp $
*
* Poor AT&T C++ iostream (but enough for what we need them for)
*/

/*---------+
| Includes |
+---------*/
#include "iostream.h"
#include "miscutil.h"
#include "string.h"

/*--------------------------------------------------------------istream::read-+
|                                                                             |
+----------------------------------------------------------------------------*/
istream & istream::read(char * buf, int n)
{
   _M_gcount = 0;
   if (n > 0) {
      streambuf * sb = rdbuf();
      do {
         int c = sb->sbumpc();
         if (c == EOF) {
            setstate(ios::eofbit | ios::failbit);
            break;
         }
         *buf++ = c;
      }while (++_M_gcount < n);
   }
   return *this;
}

/*--------------------------------------------------------------istream::peek-+
|                                                                             |
+----------------------------------------------------------------------------*/
int istream::peek() {
   _M_gcount = 0;
   return rdbuf()->sgetc();
}

/*---------------------------------------------------------------istream::get-+
|                                                                             |
+----------------------------------------------------------------------------*/
int istream::get()
{
   _M_gcount = 0;
   int c = rdbuf()->sbumpc();
   if (c == EOF) {
      setstate(ios::eofbit | ios::failbit);
   }else {
      _M_gcount = 1;
   }
   return c;
}

/*---------------------------------------------------------------istream::get-+
|                                                                             |
+----------------------------------------------------------------------------*/
istream & istream::get(char & c)
{
   _M_gcount = 0;
   int i = get();
   if (i != EOF) c = (char)i;
   return *this;
}

/*---------------------------------------------------------------istream::get-+
|                                                                             |
+----------------------------------------------------------------------------*/
istream & istream::get(char * s, streamsize n, char delim)
{
   char * c_p = s;
   if (n) {
      int c;
      while ((--n > 0) && ((EOF != (c=get())) && (c!=delim))) {
         *c_p++ = (char)c;
      }
      *c_p = '\0';
      if (c == delim) {
         rdbuf()->sputbackc(c);
      }else if (c==EOF) {
         setstate(ios::eofbit);  // clear failbit
      }
   }
   _M_gcount = c_p - s;
   return *this;
}


/*-------------------------------------------------------------istream::seekg-+
|                                                                             |
+----------------------------------------------------------------------------*/
istream & istream::seekg(streampos pos)
{
   _M_gcount = 0;
   if (rdbuf()->pubseekpos(pos, ios::in) == EOF) {
      setstate(failbit);
   }
   return *this;
}

/*-------------------------------------------------------------istream::seekg-+
|                                                                             |
+----------------------------------------------------------------------------*/
istream & istream::seekg(streamoff off, ios::seekdir dir)
{
   _M_gcount = 0;
   if (rdbuf()->pubseekoff(off, dir, ios::in) == EOF) {
      setstate(failbit);
   }
   return *this;
}

/*-------------------------------------------------------------istream::tellg-+
|                                                                             |
+----------------------------------------------------------------------------*/
streampos istream::tellg()
{
  _M_gcount = 0;
  return rdbuf()->pubseekoff(0, ios::cur, ios::in);
}

/*---------------------------------------------------------------ostream::put-+
|                                                                             |
+----------------------------------------------------------------------------*/
ostream & ostream::put(char c) {
   if (rdbuf()->sputc((unsigned char)c) == EOF) setstate(badbit);
   return *this;
}

/*-------------------------------------------------------------ostream::write-+
|                                                                             |
+----------------------------------------------------------------------------*/
ostream & ostream::write(const char * buf, int n) {
{
   int count = rdbuf()->sputn(buf, n);
   if (count != n)
      setstate(badbit);
   }
   return *this;
}

/*--------------------------------------------------------ostream::operator<<-+
|                                                                             |
+----------------------------------------------------------------------------*/
ostream & ostream::operator<<(char const * p)
{
/* This is not in ISO 14882.   Kept in case we need it...
|
|  int length;
|  int lenMax = precision(0);  // should be 6?
|  if (lenMax) {
|     char const * pMax = memchr(p, '\0', lenMax);
|     length = pMax? pMax - p : lenMax;
|  }else {
|     length = strlen(p);
|  }
*/
   writeAndPad(p, strlen(p));
   return *this;
}

/*--------------------------------------------------------ostream::operator<<-+
|                                                                             |
+----------------------------------------------------------------------------*/
ostream & ostream::operator<<(long n) {
   if (n < 0) {
      n = -n;
      formatNumber((unsigned long)n, true);
   }else {
      formatNumber((unsigned long)n, false);
   }
   return *this;
}

/*--------------------------------------------------------ostream::operator<<-+
|                                                                             |
+----------------------------------------------------------------------------*/
ostream & ostream::operator<<(unsigned long n)
{
   formatNumber((unsigned long)n, false);
   return *this;
}

/*-------------------------------------------------------ostream::writeAndPad-+
|                                                                             |
+----------------------------------------------------------------------------*/
void ostream::writeAndPad(char const * buffer, int length)
{
   int lenMin = width(0);
   int prec = precision(0);
   char filler = fill();   // was fill(' ') to reset to default - not correct
   if (!(flags() & left)) {
      while (lenMin > length) {
         put(filler);
         --lenMin;
      }
      write(buffer, length);
   }else {
      write(buffer, length);
      while (lenMin > length) {
         put(filler);
         --lenMin;
      }
   }
}

/*------------------------------------------------------ostream::formatNumber-+
|                                                                             |
+----------------------------------------------------------------------------*/
void ostream::formatNumber(unsigned long n, bool hasSign)
{
   char buffer[35];
   int radix;
   char * p = buffer;
   int length = 0;

   fmtflags f = flags();
   if (f & oct) {
      radix = 8;
   }else if (f & hex) {
      radix = 16;
   }else {
      radix = 10;
   }
   if (hasSign) {
      length = 1;
      *p++ = '-';
   }else {
      length = 0;
   }
   writeAndPad(buffer, length + ulbtos(n, p, radix));
}


/*-------------------------------------------------------------ostream::flush-+
|                                                                             |
+----------------------------------------------------------------------------*/
ostream & ostream::flush() {
   if (rdbuf()->pubsync() == -1) setstate(badbit);
   return *this;
}

/*-------------------------------------------------------------ostream::seekp-+
|                                                                             |
+----------------------------------------------------------------------------*/
ostream & ostream::seekp(streampos pos)
{
   if (!fail() && (rdbuf()->pubseekpos(pos, ios::out) == EOF)) {
       setstate(failbit);
   }
   return *this;
}

/*-------------------------------------------------------------ostream::seekp-+
|                                                                             |
+----------------------------------------------------------------------------*/
ostream & ostream::seekp(streamoff off, ios::seekdir dir)
{
   if (!fail() && (rdbuf()->pubseekoff(off, dir, ios::out) == EOF)) {
       setstate(failbit);
   }
   return *this;
}

/*-------------------------------------------------------------ostream::tellp-+
|                                                                             |
+----------------------------------------------------------------------------*/
streampos ostream::tellp()
{
   if (!fail()) {
      return (rdbuf()->pubseekoff(0, ios::cur, ios::out));
   }else {
      return EOF;
   }
}
/*===========================================================================*/
