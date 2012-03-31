/*
* $Id: NetStream.cpp,v 1.6 2002-09-17 07:25:33 pgr Exp $
*/

/*---------+
| Includes |
+---------*/
#include <string.h>
#include <netdb.h>
#include <netinet_in.h>
#include "assert.h"
#include "NetStream.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

UInt16 AppNetRefnum;        // do not change this name!

NetStreamBuf::Init NetStreamBuf::m_init;

/*---------------------------------------------------NetStreamBuf::Init::Init-+
|                                                                             |
+----------------------------------------------------------------------------*/
NetStreamBuf::Init::Init()
{
   int const tps = SysTicksPerSecond();
   Err err = SysLibFind("Net.lib", &AppNetRefnum);
   if (err) {
      AppNetRefnum = 0;
   }
   netLibRefnum = AppNetRefnum;
   connectTimeoutTicks = tps * 10;        // 10 secs
   listenTimeoutTicks = tps / 10;         // 1/10 of sec
   ioOpTimeoutTicks = tps / 5;            // 1/5 of sec
   receiveTriesCount = 150;               // e.g.: 30 secs
   sendTriesCount = 50;                   // e.g.: 10 secs
}

/*-------------------------------------------------NetStreamBuf::NetStreamBuf-+
|                                                                             |
+----------------------------------------------------------------------------*/
NetStreamBuf::NetStreamBuf() {
   tieSelf();
   pubsetbuf(0, 32);
   m_sock = 0;
}

/*------------------------------------------------NetStreamBuf::~NetStreamBuf-+
|                                                                             |
+----------------------------------------------------------------------------*/
NetStreamBuf::~NetStreamBuf() {
   close();
}

/*---------------------------------------------------------NetStreamBuf::open-+
|                                                                             |
+----------------------------------------------------------------------------*/
FileStreamBuf * NetStreamBuf::open(char const * address, int port)
{
   Err err;
   long host = 0;

   if (m_sock > 0) {   // Already connected
      return 0;
   }

   {
      UInt16 ifErrs;
      err = NetLibOpen(m_init.netLibRefnum, &ifErrs);
      if ((err) && (err != netErrAlreadyOpen)) {
         if (ifErrs == netErrUserCancel) {
            close();
         }else {
            SHOW_ERROR(err);
         }
         return 0;
      }
   }

   {
      NetHostInfoBufType appHostInfo;
      struct hostent * ents = (struct hostent*)NetLibGetHostByName(
         m_init.netLibRefnum,
         address,
         &appHostInfo,
         m_init.connectTimeoutTicks,
         &err
      );
      if (ents) {
         memcpy(&host, ents->h_addr, ents->h_length);
      }else {
         host = NetLibAddrAToIN(m_init.netLibRefnum, address);
         if (host == -1) {
            close();
            SHOW_ERROR(netErrParamErr);  // Cannot resolve
            return 0;
         }
      }
   }

   m_sock = NetLibSocketOpen(
      m_init.netLibRefnum,
      netSocketAddrINET,
      netSocketTypeStream,
      0,
      m_init.connectTimeoutTicks,
      &err
   );
   if (m_sock < 0) {
      close();
      SHOW_ERROR(err);
      return 0;
   }else {
      int one = 1;
      NetSocketAddrINType addr;
      addr.family = netSocketAddrINET;      // AF_INET
      addr.port = port;                     // htons(uri.getPort());
      addr.addr = host;

      if (0 > NetLibSocketConnect(
            m_init.netLibRefnum,
            m_sock,
            (NetSocketAddrType*)&addr,
            sizeof addr,
            m_init.connectTimeoutTicks,
            &err
         )
      ) {
         close();
         SHOW_ERROR(err);
         return 0;
      }

      if (0 > NetLibSocketOptionSet(
            m_init.netLibRefnum,
            m_sock,
            IPPROTO_TCP,
            netSocketOptTCPNoDelay,         // TCP_NODELAY
            (char *)&one,
            sizeof(one),
            m_init.connectTimeoutTicks,
            &err
         )
      ) {
         close();
         SHOW_ERROR(err);
         return 0;
      }
   }
   return this;
}

/*--------------------------------------------------------NetStreamBuf::close-+
|                                                                             |
+----------------------------------------------------------------------------*/
FileStreamBuf * NetStreamBuf::close()
{
   Err err;

   if (m_sock > 0) {
      NetLibSocketClose(
        m_init.netLibRefnum, m_sock, m_init.connectTimeoutTicks, &err
      );
   }
   m_sock = 0;
   switch (NetLibClose(m_init.netLibRefnum, true)) {
   case 0:
   case netErrNotOpen:
      break;
   case netErrStillOpen:
      return 0;
   default:
      SHOW_ERROR(err);
      return 0;
   }
   return this;
}

/*---------------------------------------------------------NetStreamBuf::read-+
|                                                                             |
+----------------------------------------------------------------------------*/
int NetStreamBuf::read(char * pchBuf, int iLen)
{
   int i=1;
   if (m_sock > 0) for (;;) {
      doQueuedEvent(5);
      Err err;
      int cnt = NetLibReceive(
         m_init.netLibRefnum,
         m_sock,
         pchBuf,
         iLen,
         0, 0, 0,
         m_init.ioOpTimeoutTicks,
         &err
      );
      if (cnt >= 0)  {
         return cnt;
      }else if (err != netErrTimeout) {
         SHOW_ERROR(err);
         break;  // return -1?
      }else if (++i >= m_init.receiveTriesCount) {
         return FileStreamBuf::TIMEOUT;
      }
   }
   return 0;
}

/*--------------------------------------------------------NetStreamBuf::write-+
|                                                                             |
+----------------------------------------------------------------------------*/
int NetStreamBuf::write(char const * pchBuf, int iLen)
{
   int i=1;
   if (m_sock > 0) for (;;) {
      doQueuedEvent(5);
      Err err;
      int cnt = NetLibSend(
         m_init.netLibRefnum,
         m_sock,
         (char *)pchBuf,
         iLen,
         0, 0, 0,
         m_init.ioOpTimeoutTicks,
         &err
      );
      if (cnt >= 0)  {
         return cnt;
      }else if (err != netErrTimeout) {
         SHOW_ERROR(err);
         break;  // return -1?
      }else if (++i >= m_init.sendTriesCount) {
         return FileStreamBuf::TIMEOUT;
      }
   }
   return 0;
}

/*------------------------------------------------NetStreamBuf::doQueuedEvent-+
|If 'count' events are waiting in the queue, process them                     |
+----------------------------------------------------------------------------*/
void NetStreamBuf::doQueuedEvent(int count)
{
   Err err;
   EventType event;

   while (count--) {
      EvtGetEvent(&event, 0);
      switch (event.eType) {
      case appStopEvent:
         close();
         throw AppStopEventException();
      case nilEvent:
         return;   // assume empty queue
      default:
         if (!SysHandleEvent(&event) && !MenuHandleEvent(NULL, &event, &err)) {
            FrmDispatchEvent(&event);
         }
         break;
      }
   }
}

/*--------------------------------------------NetStreamBuf::listenForMessages-+
|                                                                             |
+----------------------------------------------------------------------------*/
void NetStreamBuf::listenForMessages()
{
   if (m_sock <= 0) return;

   for (;;) {
      Err err;
      fd_set fds;
      FD_ZERO(&fds);
      FD_SET(sysFileDescStdIn, &fds);
      FD_SET(m_sock, &fds);

      doQueuedEvent();
      if (0 > NetLibSelect(
            m_init.netLibRefnum,
            m_sock+1,
            &fds, NULL, NULL,
            m_init.listenTimeoutTicks,
            &err
         )
      ) {
         SHOW_ERROR(err);
         close();
      }

      if (FD_ISSET(m_sock, &fds)) {
//       if (!handleServerMessage()) {
//          close();
//          printf("Connection Lost!");
//       }
      }
   }
}

/*-------------------------------------------------------NetStream::NetStream-+
| Constructor                                                                 |
+----------------------------------------------------------------------------*/
NetStream::NetStream(char const * address, int port) :
   iostream(&m_buf)
{
   if (!m_buf.open(address, port)) {
      clear(rdstate() | ios::failbit);
   }
}

/*------------------------------------------------------NetStream::~NetStream-+
| Destructor                                                                  |
+----------------------------------------------------------------------------*/
NetStream::~NetStream() {
   m_buf.close();
}

/*-----------------------------------------------NetSchemeHandler::Rep::getID-+
|                                                                             |
+----------------------------------------------------------------------------*/
char const * NetSchemeHandler::Rep::getID() const {
   return "TCPIP";
}

/*------------------------------------------NetSchemeHandler::Rep::makeStream-+
|                                                                             |
+----------------------------------------------------------------------------*/
iostream * NetSchemeHandler::Rep::makeStream(
   URI const & uri, ios__openmode om
) {
   return new NetStream(uri.getHost(), uri.getPort());
}

#ifdef TOOLS_NAMESPACE
}
#endif
/*===========================================================================*/
