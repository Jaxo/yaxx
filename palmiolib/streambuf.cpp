/*
* $Id: streambuf.cpp,v 1.4 2002-08-29 07:16:28 pgr Exp $
*
* AT&T streambuf
*/

/*---------+
| Includes |
+---------*/
#include <string.h>
#include "streambuf.h"

/*----------------------------------------------------------streambuf::xsgetn-+
|                                                                             |
+----------------------------------------------------------------------------*/
streamsize streambuf::xsgetn(char * pchBuf, streamsize n)
{
   int remain = n;
   int avail;
   while (
      (remain) && (
         (avail=_M_in_end-_M_in_cur, avail > 0) ||
         ((underflow() != EOF) && (avail=_M_in_end-_M_in_cur, avail > 0))
      )
   ) {
      int count = (remain < avail)? remain : avail;
      memcpy(pchBuf, _M_in_cur, count);
      _M_in_cur += count;
      remain -= count;
      pchBuf += count;
   }
   return n - remain;
}

/*----------------------------------------------------------streambuf::xsputn-+
|                                                                             |
+----------------------------------------------------------------------------*/
streamsize streambuf::xsputn(char const * pchBuf, streamsize n)
{
   int remain = n;
   while (remain) {
      int room = _M_out_end - _M_out_cur;
      if (room == 0) {
         // versus xsgetn, *pchBuf is entered... (or EOF is returned)
         if (overflow(*pchBuf) == EOF) break;
         --remain;
         ++pchBuf;
         continue;
      }
      int count = (remain < room)? remain : room;
      memcpy(_M_out_cur, pchBuf, count);
      _M_out_cur += count;
      remain -= count;
      pchBuf += count;
   }
   return n - remain;
}

/*----------------------------------------------------------streambuf::setbuf-+
|                                                                             |
+----------------------------------------------------------------------------*/
streambuf * streambuf::setbuf(char * buf, int len) {
   return this;
}

/*------------------------------------------------------------streambuf::sget-+
| *NOT* part of the stream standard (but every implementation has it!)        |
+----------------------------------------------------------------------------*/
int streambuf::sget(streambuf * sbIn)
{
   int ret = 0;
   while (sbIn->sgetc() != EOF) {
      int in_avail = sbIn->in_avail();
      int out_written = sputn(sbIn->gptr(), in_avail);
      sbIn->gbump(out_written);
      ret += out_written;
   }
   return ret;
}

/*===========================================================================*/
