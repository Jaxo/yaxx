/*
* $Id: BeamerStream.h,v 1.7 2002-09-18 07:06:41 pgr Exp $
*/

#ifndef COM_JAXO_PALMIO_BEAMERSTREAM_H_INCLUDED
#define COM_JAXO_PALMIO_BEAMERSTREAM_H_INCLUDED

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

/*---------------------------------------------------- class BeamerStreamBuf -+
|                                                                             |
+----------------------------------------------------------------------------*/
class TOOLS_API BeamerStreamBuf : public FileStreamBuf {
public:
   static bool registerData();

   BeamerStreamBuf();
   ~BeamerStreamBuf();
   bool isReceiving() const;
   Err receiveData(ExgSocketPtr pSock);

   FileStreamBuf * open();
   FileStreamBuf * close();
   int read(char * pchBuf, int iLen);
   int write(char const * pchBuf, int iLen);

private:
   class DataReceivedEvent {
   public:
      DataReceivedEvent();
      static bool isMe(EventType &);
      EventType ev;
   };

   enum EXGState {
      EXGSTATE_NONE = 0,
      EXGSTATE_GET  = 1,
      EXGSTATE_PUT  = 2
   };

   void setDefaultApp(bool isEnable);
   void disableSendMode();
   void enableSendMode();

   static char const m_filePath[];
   EXGState m_exgState;
   ExgSocketType m_sock;
   char * m_name;
   char * m_readBuffer;
   int m_readLen;
   int m_readOfs;
   Err m_err;
};

/* -- INLINES -- */

inline BeamerStreamBuf::DataReceivedEvent::DataReceivedEvent()
{
   ev.eType = (eventsEnum)((int)firstUserEvent + 5);
   ev.penDown = false;
   ev.tapCount = 0;
   ev.screenX = 0;
   ev.screenY = 0;
   ev.data.generic.datum[0] = 0;
}

inline bool BeamerStreamBuf::DataReceivedEvent::isMe(EventType & event) {
   return event.eType == (firstUserEvent + 5);
}

inline bool BeamerStreamBuf::isReceiving() const {
   return m_exgState == EXGSTATE_GET;
}

/*---------------------------------------------------------------BeamerStream-+
| BeamerStream IS-A iostream to manipulate a BeamerStreamBuf.                 |
+----------------------------------------------------------------------------*/
class TOOLS_API BeamerStream: public iostream {
public:
   BeamerStream();
   ~BeamerStream();

private:
   // extern BeamerStreamBuf * g_pBeamerStreamBuf;
   BeamerStream & operator=(BeamerStream const & source); // no!
   BeamerStream(BeamerStream const & source);             // no!
};

/*--------------------------------------------------------BeamerSchemeHandler-+
|                                                                             |
+----------------------------------------------------------------------------*/
class TOOLS_API BeamerSchemeHandler : public URI::SchemeHandler {
public:
   BeamerSchemeHandler();

private:
   class Rep : public URI::SchemeHandler::Rep {
      char const * getID() const;
      void onNew(URI::Data & uri);
      iostream * makeStream(URI const & uri, ios__openmode om);
   };
};

inline BeamerSchemeHandler::BeamerSchemeHandler(
) :
   SchemeHandler(new Rep) {
}

#ifdef TOOLS_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
