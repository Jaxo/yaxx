/*
* $Id: NetStream.h,v 1.3 2002-08-26 14:26:21 pgr Exp $
*/

#ifndef COM_JAXO_PALMIO_NETSTREAM_H_INCLUDED
#define COM_JAXO_PALMIO_NETSTREAM_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include <Event.h>
#include "AppStopEventException.h"
#include "FileStreamBuf.h"
#include "migstream.h"
#include "iostream.h"
#include "URI.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*------------------------------------------------------- class NetStreamBuf -+
|                                                                             |
+----------------------------------------------------------------------------*/
class TOOLS_API NetStreamBuf : public FileStreamBuf {
public:
   NetStreamBuf();
   ~NetStreamBuf();

   FileStreamBuf * open(char const * address, int port);
   FileStreamBuf * close();
   int read(char * pchBuf, int iLen);
   int write(char const * pchBuf, int iLen);
   void listenForMessages();

private:
   static struct Init {
      Init();
      UInt16 netLibRefnum;
      int connectTimeoutTicks;
      int listenTimeoutTicks;
      int receiveTriesCount;
      int sendTriesCount;
      int ioOpTimeoutTicks;
   }m_init;
   int m_sock;

   void doQueuedEvent(int count = 1);
};

/*------------------------------------------------------------------NetStream-+
| NetStream IS-A iostream to manipulate a NetStreamBuf.                       |
+----------------------------------------------------------------------------*/
class TOOLS_API NetStream: public iostream {
public:
   NetStream(char const * address, int port);
   ~NetStream();

private:
   NetStreamBuf m_buf;

   NetStream & operator=(NetStream const & source); // no!
   NetStream(NetStream const & source);             // no!
};

/*-----------------------------------------------------------NetSchemeHandler-+
|                                                                             |
+----------------------------------------------------------------------------*/
class TOOLS_API NetSchemeHandler : public URI::SchemeHandler {
public:
   NetSchemeHandler();

private:
   class Rep : public URI::SchemeHandler::Rep {
      char const * getID() const;
      iostream * makeStream(URI const & uri, ios__openmode om);
   };
};

inline NetSchemeHandler::NetSchemeHandler(
) :
   SchemeHandler(new Rep) {
}

#ifdef TOOLS_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
