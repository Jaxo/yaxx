/*
* $Id: PalmConsole.cpp,v 1.34 2002-07-01 14:53:55 pgr Exp $
*
* Palm Console
*/

/*---------+
| Includes |
+---------*/
#include <assert.h>
#include <unix_stdarg.h>
#include <string.h>

#include "PalmConsole.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

#define myZapeof(c) ((unsigned char)(c))

/*---------------------------------------------------PalmConsole::PalmConsole-+
|                                                                             |
+----------------------------------------------------------------------------*/
PalmConsole::PalmConsole() : m_isOpened(false), m_haveEnough(false)
{
   // a PalmConsole has no buffer, but delegates to a MemStreamBuf.
}

/*----------------------------------------------------------PalmConsole::open-+
| Virtual - the call to open() is delayed until one really needs to read or   |
| write. This allows an application not to show up a console form             |
| if there is no needs to do so.                                              |
+----------------------------------------------------------------------------*/
bool PalmConsole::open() {
   return true;
}

/*------------------------------------------------PalmConsole::signalNewChars-+
|                                                                             |
+----------------------------------------------------------------------------*/
void PalmConsole::signalNewChars(char const * pText, int len)
{
   if (pText[len-1] == '\n') m_haveEnough = true;
}

/*-----------------------------------------------------PalmConsole::underflow-+
| When gptr == egptr...                                                       |
| This means: get more food from the console,                                 |
| and feed the underlying MemStreamBuf.                                       |
+----------------------------------------------------------------------------*/
int PalmConsole::underflow()
{
   if (!m_isOpened) {
      if (!open()) return EOF;
      m_isOpened = true;
   }
   // get a bunch of chars from the ultimate producer

   m_haveEnough = (m_sb.in_avail() > 0);
   refill(m_sb, m_haveEnough);
   return m_sb.sbumpc();
}

/*------------------------------------------------------PalmConsole::overflow-+
| When pptr == epptr...                                                       |
+----------------------------------------------------------------------------*/
int PalmConsole::overflow(int c)
{
   if (!m_isOpened) {
      if (!open()) return EOF;
      m_isOpened = true;
   }
   if (c != EOF) {
      char d = myZapeof(c);
      xsputn(&d, 1);
      return d;
   }else {
      return EOF;
   }
}

/*-----------------------------------------------------PalmConsole::pbackfail-+
|                                                                             |
+----------------------------------------------------------------------------*/
int PalmConsole::pbackfail(int c) {
   return m_sb.sputbackc(c);
}

/*--------------------------------------------------------PalmConsole::xsputn-+
| This xsputn differs from streambuf::xsputn because PalmConsole has no       |
| real output buffer.  So we do the physical IO on the fly.                   |
+----------------------------------------------------------------------------*/
streamsize PalmConsole::xsputn(char const * pchBuf, streamsize n)
{
   if (!m_isOpened) {
      if (!open()) return 0;
      m_isOpened = true;
   }
   return produce(pchBuf, (int)n);
}

/*----------------------------------------------------------PalmConsole::sync-+
|                                                                             |
+----------------------------------------------------------------------------*/
int PalmConsole::sync() {
   return 0;
}

/*-------------------------------------------------------PalmConsole::seekoff-+
|                                                                             |
+----------------------------------------------------------------------------*/
streampos PalmConsole::seekoff(
   streamoff so,
   ios::seekdir dir,
   ios::openmode om
) {
   return m_sb.seekoff(so, dir, om);
}

/*-------------------------------------------------------PalmConsole::seekpos-+
|                                                                             |
+----------------------------------------------------------------------------*/
streampos PalmConsole::seekpos(
   streampos sp,
   ios::openmode om
) {
   return seekoff(streamoff(sp), ios::beg, om);
}

#ifdef TOOLS_NAMESPACE
}
#endif
/*===========================================================================*/
