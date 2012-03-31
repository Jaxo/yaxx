/*
* $Id: BeamerStream.cpp,v 1.10 2002-09-18 07:06:41 pgr Exp $
*/

/*---------+
| Includes |
+---------*/
#include <string.h>
#include "assert.h"  // this is OUR assert.h (so use "" instead of <> for CW)
#include "BeamerStream.h"
#include "miscutil.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

extern BeamerStreamBuf * g_pBeamerStreamBuf;

char const BeamerStreamBuf::m_filePath[] = "RexxData.REX";

/*-------------------------------------------BeamerStreamBuf::BeamerStreamBuf-+
|                                                                             |
+----------------------------------------------------------------------------*/
BeamerStreamBuf::BeamerStreamBuf() {
   tieSelf();
   pubsetbuf(0, 32);

   memset(&m_sock, 0, sizeof m_sock);
   m_sock.name = new char[sizeof(exgBeamScheme) + sizeof m_filePath];
   strcpy(m_sock.name, exgBeamScheme);
   strcat(m_sock.name, ":");
   strcat(m_sock.name, m_filePath);
   // m_sock.target = CREATORID;  this would bypass the notification
                              //<<<JAL TODO determine current CRID dynamically.
                              // i have code for this somewhere.
   // m_sock.noGoTo = 1l
   m_sock.description = "Rexx Data";
   // m_sock.count = 1;        // optional
   // m_sock.length = iLen;    // optional
   // m_sock.noStatus = true;  // useless for IR 4.0, unused for < 4.0
   m_err = errNone;
   m_exgState = EXGSTATE_NONE;
}

/*------------------------------------------BeamerStreamBuf::~BeamerStreamBuf-+
|                                                                             |
+----------------------------------------------------------------------------*/
BeamerStreamBuf::~BeamerStreamBuf() {
   // close();
   disableSendMode();
   delete [] m_sock.name;
}


/*------------------------------------------------------BeamerStreamBuf::open-+
|                                                                             |
+----------------------------------------------------------------------------*/
FileStreamBuf * BeamerStreamBuf::open() {
   return this;
}

/*-----------------------------------------------------BeamerStreamBuf::close-+
|                                                                             |
+----------------------------------------------------------------------------*/
FileStreamBuf * BeamerStreamBuf::close() {
   return this;
}

/*--------------------------------------------BeamerStreamBuf::enableSendMode-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BeamerStreamBuf::enableSendMode()
{
   switch (m_exgState) {
   case EXGSTATE_GET:
      /* fall thru */
   case EXGSTATE_NONE:
      m_err = ExgPut(&m_sock);   // turn send mode on
      m_exgState = EXGSTATE_PUT;
      break;
   // case EXGSTATE_PUT:
   default:
      break;
   }
}

/*-------------------------------------------BeamerStreamBuf::disableSendMode-+
|                                                                             |
+----------------------------------------------------------------------------*/
void BeamerStreamBuf::disableSendMode()
{
   switch (m_exgState) {
   case EXGSTATE_PUT:
      ExgDisconnect(&m_sock, m_err);  // turn send mode off
      if (m_err) {
         SHOW_ERROR(m_err);
      }
      /* fall thru */
   case EXGSTATE_NONE:
      m_exgState = EXGSTATE_GET;
      break;
   // case EXGSTATE_GET:
   default:
      break;
   }
}

/*------------------------------------------------------BeamerStreamBuf::read-+
|                                                                             |
+----------------------------------------------------------------------------*/
int BeamerStreamBuf::read(char * pchBuf, int iLen)
{
   if (!iLen || (m_exgState == EXGSTATE_GET)) return 0;
   disableSendMode();
   m_readBuffer = pchBuf;
   m_readLen = iLen;
   m_readOfs = 0;
   for (;;) {
      Err err;
      EventType event;
      EvtGetEvent(&event, evtWaitForever);

      if (SysHandleEvent(&event)) continue;
      if (event.eType == appStopEvent) {
         throw AppStopEventException();
      }
      if (MenuHandleEvent(0, &event, &err)) continue;
      if (!DataReceivedEvent::isMe(event)) {
         FrmDispatchEvent(&event);
         continue;
      }
      break;
   }
   m_exgState = EXGSTATE_NONE;
   return m_readOfs;
}

/*-----------------------------------------------BeamerStreamBuf::receiveData-+
|                                                                             |
+----------------------------------------------------------------------------*/
Err BeamerStreamBuf::receiveData(ExgSocketPtr pSock)
{
   if (m_exgState != EXGSTATE_GET) {
      /*>>>PGR:
      | then I should call the default handler; but this should not occur,
      | b/c, when m_isReceiving is false,  I'm not supposed to be
      | the default handler.
      */
      return exgErrNoKnownTarget;
   }else {
      m_err = ExgAccept(pSock);
      if (m_err) {
         ExgDisconnect(pSock, m_err);
         SHOW_ERROR(m_err);
      }else {
         while (m_readOfs < m_readLen) {
            int cnt = ExgReceive(
               pSock, m_readBuffer+m_readOfs, m_readLen-m_readOfs, &m_err
            );
            if (!cnt || m_err) break;
            m_readOfs += cnt;
         }
         ExgDisconnect(pSock, m_err);
         DataReceivedEvent ev;
         EvtAddEventToQueue(&ev.ev);         // break the read loop
      }
   }
   return errNone;
}

/*-----------------------------------------------------BeamerStreamBuf::write-+
|                                                                             |
+----------------------------------------------------------------------------*/
int BeamerStreamBuf::write(char const * pchBuf, int iLen)
{
   enableSendMode();
   if (m_err) {
      return 0;
   }else {
      return ExgSend(&m_sock, pchBuf, iLen, &m_err);
   }
}

/*-------------------------------------------------BeamerStream::BeamerStream-+
| Constructor                                                                 |
+----------------------------------------------------------------------------*/
BeamerStream::BeamerStream() : iostream(g_pBeamerStreamBuf)
{
   assert (g_pBeamerStreamBuf);
   if (!g_pBeamerStreamBuf->open()) {
      clear(rdstate() | ios::failbit);
   }
}

/*------------------------------------------------BeamerStream::~BeamerStream-+
| Destructor                                                                  |
+----------------------------------------------------------------------------*/
BeamerStream::~BeamerStream() {
   g_pBeamerStreamBuf->close();
}

/*--------------------------------------------BeamerSchemeHandler::Rep::getID-+
|                                                                             |
+----------------------------------------------------------------------------*/
char const * BeamerSchemeHandler::Rep::getID() const {
   return "BEAMER";
}

/*--------------------------------------------BeamerSchemeHandler::Rep::onNew-+
| This implements the following syntax:                                       |
|                                                                             |
| "beamer://"                                                                 |
| palmhostId?              default is "localhost"  FOR FUTURE USE             |
|                                                                             |
| Ex:                                                                         |
|   "beamer:"                                                                 |
|   "beamer://localhost                                                       |
|                                                                             |
+----------------------------------------------------------------------------*/
void BeamerSchemeHandler::Rep::onNew(URI::Data & uri)
{
   return;
}

/*---------------------------------------BeamerSchemeHandler::Rep::makeStream-+
|                                                                             |
+----------------------------------------------------------------------------*/
iostream * BeamerSchemeHandler::Rep::makeStream(
   URI const & uri, ios__openmode om
) {
   return new BeamerStream();
}

#ifdef TOOLS_NAMESPACE
}
#endif
/*===========================================================================*/
