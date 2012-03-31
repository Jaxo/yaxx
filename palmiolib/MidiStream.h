/*
* $Id: MidiStream.h,v 1.1 2002-08-23 15:29:12 pgr Exp $
*
*  Midi stream
*/

#ifndef COM_JAXO_PALMIO_MIDISTREAM_H_INCLUDED
#define COM_JAXO_PALMIO_MIDISTREAM_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include "streambuf.h"
#include "iostream.h"
#include "URI.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*--------------------------------------------------------------MidiStreamBuf-+
|                                                                             |
+----------------------------------------------------------------------------*/
class TOOLS_API MidiStreamBuf : public streambuf {
private:
   streamsize xsputn(char const *, streamsize);
   int overflow(int);
   int underflow();
   int pbackfail(int);
   int sync();
   streampos seekoff(streamoff, ios::seekdir, ios::openmode);
   streampos seekpos(streampos, ios::openmode);
};

/* -- INLINES -- */
inline int MidiStreamBuf::overflow(int = EOF) { return EOF; }
inline int MidiStreamBuf::underflow()         { return EOF; }
inline int MidiStreamBuf::pbackfail(int)      { return EOF; }
inline int MidiStreamBuf::sync()              { return 0; }
inline streampos MidiStreamBuf::seekoff(streamoff, ios::seekdir, ios::openmode) {
   return EOF;
}
inline streampos MidiStreamBuf::seekpos(streampos, ios::openmode) {
   return EOF;
}

/*-----------------------------------------------------------------MidiStream-+
| MidiStream IS-A iostream to manipulate a MidiStreamBuf.                     |
+----------------------------------------------------------------------------*/
class TOOLS_API MidiStream: public iostream {
public:
   MidiStream();
private:
   MidiStreamBuf m_buf;
   MidiStream & operator=(MidiStream const & source); // no!
   MidiStream(MidiStream const & source);             // no!
};

/* -- INLINES -- */

inline MidiStream::MidiStream() : iostream(&m_buf) {
}

/*----------------------------------------------------------MidiSchemeHandler-+
|                                                                             |
+----------------------------------------------------------------------------*/
class TOOLS_API MidiSchemeHandler : public URI::SchemeHandler {
public:
   MidiSchemeHandler();

private:
   class Rep : public URI::SchemeHandler::Rep {
      char const * getID() const;
      iostream * makeStream(URI const & uri, ios__openmode om);
   };
};

/* -- INLINES -- */

inline MidiSchemeHandler::MidiSchemeHandler() : SchemeHandler(new Rep) {
}
inline iostream * MidiSchemeHandler::Rep::makeStream(URI const &, ios__openmode) {
   return new MidiStream();
}

#ifdef TOOLS_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
