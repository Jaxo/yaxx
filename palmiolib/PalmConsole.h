/*
* $Id: PalmConsole.h,v 1.27 2002-07-01 14:53:55 pgr Exp $
*
* Palm Console
*/

#ifndef COM_JAXO_PALMIO_PALMCONSOLE_H_INCLUDED
#define COM_JAXO_PALMIO_PALMCONSOLE_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include "ios.h"
#include "iostream.h"

#include "URI.h"
#include "MemStream.h"
#include "Escaper.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*----------------------------------------------------------------PalmConsole-+
|                                                                             |
+----------------------------------------------------------------------------*/
class PalmConsole : public streambuf {
public:
   PalmConsole();

protected:
   virtual bool open();
   virtual void refill(streambuf & sb, bool volatile const & haveEnough) = 0;
   virtual int produce(char const * pchBuf, int n) = 0;
   void signalNewChars(char const * pText, int len);
   bool isOpened() const;
   UInt16 getFormId() const;

private:
   MemStreamBuf m_sb;
   bool m_isOpened;
   bool m_haveEnough;

   // streambuf overrides
   int overflow(int = EOF);
   int underflow();
   int pbackfail(int);
   streamsize xsputn(char const * pchBuf, streamsize n);
   int sync();
   streampos seekoff(
      streamoff off,
      ios::seekdir way,
      ios::openmode mode = (ios::openmode)(ios::in | ios::out)
   );
   streampos seekpos(
      streampos sp,
      ios::openmode mode = (ios::openmode)(ios::in | ios::out)
   );
};

/* -- INLINES -- */
inline bool PalmConsole::isOpened() const {
   return m_isOpened;
}

#ifdef TOOLS_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
