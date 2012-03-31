/*
* $Id: SerialStream.cpp,v 1.11 2002-09-11 09:24:16 pgr Exp $
*/

/*---------+
| Includes |
+---------*/
#include <assert.h>
#include <unix_fcntl.h>
#include <stdio.h>
#include <string.h>
#include <SerialMgrOld.h>
#include "miscutil.h"
#include "SerialStream.h"
#include "AppStopEventException.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

struct SerialPath {
   SerialStreamBuf::Device device;
   int bauds;
};

#define version_4_0 0x04003000

/*-------------------------------------------SerialStreamBuf::SerialStreamBuf-+
|                                                                             |
+----------------------------------------------------------------------------*/
SerialStreamBuf::SerialStreamBuf() {
   tieSelf();
   pubsetbuf(0, 32);
   m_isPortOpened = false;
}

/*------------------------------------------------------SerialStreamBuf::open-+
|                                                                             |
+----------------------------------------------------------------------------*/
FileStreamBuf * SerialStreamBuf::open(Device device, int bauds)
{
   if (m_isPortOpened) return 0;

   UInt32 romVersion;
   Err err = FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);

   if (sysGetROMVerMajor(romVersion) < 2) {
      m_whatSerial = 0;       // unsupported?
   }else {
      err = FtrGet(sysFileCSerialMgr, sysFtrNewSerialPresent, &m_whatSerial);
      if (err || (m_whatSerial == 0) || (romVersion < version_4_0)) {
         m_whatSerial = 1;
      }else {
         m_whatSerial = 2;
      }
   }

   // Handspring: keyboard thread must be disabled
// if (FtrGet('hsEx', 0, &ftrValue) == errNone) {
//    /* there is an example inside PalmOS Emulator source */
// }

   if (m_whatSerial < 2) {
      if (device == USB_DEVICE) {
         return 0;
      }else {
//       CRADLE_DEVICE:
//       RS232_DEVICE:
         if (SysLibFind("Serial Library", &m_portID)) return 0;
         if (SerOpen(m_portID, 0, bauds)) {
            SerClose(m_portID);
            return 0;
         }
      }
   }else {
      switch (device) {
      case USB_DEVICE:
         err = SrmOpen(serPortCradleUSBPort, bauds, &m_portID);
         break;
      case CRADLE_DEVICE:
         err = SrmOpen(serPortCradlePort, bauds, &m_portID);
         break;
//    case RS232_DEVICE:
      default:
         err = SrmOpen(serPortCradleRS232Port, bauds, &m_portID);
         break;
      }
      if (err) {
         SrmClose(m_portID);
         return 0;
      }
   }
   m_isPortOpened = true;
   return this;
}

/*-----------------------------------------------------SerialStreamBuf::close-+
|                                                                             |
+----------------------------------------------------------------------------*/
FileStreamBuf * SerialStreamBuf::close()
{
   if (m_isPortOpened) {
      if (m_whatSerial < 2) {
         SerClose(m_portID);
      }else {
         SrmClose(m_portID);
      }
      m_isPortOpened = false;
      return this;
   }
   return 0;
}

/*------------------------------------------------------SerialStreamBuf::read-+
|                                                                             |
+----------------------------------------------------------------------------*/
int SerialStreamBuf::read(char * pchBuf, int iLen)
{
   for (;;) {
      Err err;
      UInt32 avail;
      EventType event;
      EvtGetEvent(&event, 10);        // we need periodic nilEvents

      if (SysHandleEvent(&event)) continue;
      if (event.eType == appStopEvent) {
         throw AppStopEventException();
      }
      if (MenuHandleEvent(0, &event, &err)) continue;
      if ((!m_isPortOpened) || (event.eType != nilEvent)) {
         FrmDispatchEvent(&event);
         continue;
      }

      // Time break: check for data waiting
      EvtResetAutoOffTimer();
      if (m_whatSerial < 2) {
         if (SerReceiveCheck(m_portID, &avail) != 0) {
            SerClearErr(m_portID);
            continue;
         }
      }else {
         if (SrmReceiveCheck(m_portID, &avail) != 0) {
            SrmClearErr(m_portID);
            continue;
         }
      }
      if (!avail) {
         continue;
      }
      if (avail > iLen) {
         avail = iLen;
      }
      if (m_whatSerial < 2) {
         avail = SerReceive(m_portID, pchBuf, avail, 10, &err);
      }else {
         avail = SrmReceive(m_portID, pchBuf, avail, 10, &err);
      }
      if (err == serErrLineErr) {
         if (m_whatSerial < 2) {
            SerReceiveFlush(m_portID, 1);
         }else {
            SrmReceiveFlush(m_portID, 1);
         }
      }else {
         if (avail) return avail;
      }
   }
}

/*-----------------------------------------------------SerialStreamBuf::write-+
|                                                                             |
+----------------------------------------------------------------------------*/
int SerialStreamBuf::write(char const * pchBuf, int iLen)
{
   Err err;
   if (m_whatSerial < 2) {
      iLen = SerSend(m_portID, pchBuf, iLen, &err);
//    SerSendWait(m_portID, -1);
   }else {
      iLen = SrmSend(m_portID, pchBuf, iLen, &err);
//    SrmSendWait(m_portID);
   }
   return iLen;
}

/*-------------------------------------------------SerialStream::SerialStream-+
| Constructor                                                                 |
+----------------------------------------------------------------------------*/
SerialStream::SerialStream(SerialStreamBuf::Device device, int bauds) :
   iostream(&m_buf)
{
   if (!m_buf.open(device, bauds)) {
      clear(rdstate() | ios::failbit);
   }
}

/*------------------------------------------------SerialStream::~SerialStream-+
| Destructor                                                                  |
+----------------------------------------------------------------------------*/
SerialStream::~SerialStream() {
   m_buf.close();
}


/*--------------------------------------------SerialSchemeHandler::Rep::getID-+
|                                                                             |
+----------------------------------------------------------------------------*/
char const * SerialSchemeHandler::Rep::getID() const {
   return "SERIAL";
}

/*------------------------------------SerialSchemeHandler::Rep::matchesScheme-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool SerialSchemeHandler::Rep::matchesScheme(
   char const * scheme, int len
) const {
   static char const aszSerialScheme[] = "serial";
   static char const aszCradleScheme[] = "cradle";
   static char const aszRs232Scheme[]  = "rs232";
   static char const aszUsbScheme[]    = "usb";
   return (
      (
         (len == (sizeof aszSerialScheme-1)) &&
         (0 == strncasecmp(scheme, aszSerialScheme, sizeof aszSerialScheme-1))
      ) || (
         (len == (sizeof aszCradleScheme-1)) &&
         (0 == strncasecmp(scheme, aszCradleScheme, sizeof aszCradleScheme-1))
      ) || (
         (len == (sizeof aszRs232Scheme-1)) &&
         (0 == strncasecmp(scheme, aszRs232Scheme, sizeof aszRs232Scheme-1))
      ) || (
         (len == (sizeof aszUsbScheme-1)) &&
         (0 == strncasecmp(scheme, aszUsbScheme, sizeof aszUsbScheme-1))
      )
   );
}

/*--------------------------------------------SerialSchemeHandler::Rep::onNew-+
| This implements the following syntax:                                       |
|                                                                             |
| "serial://"              (or: 'cradle', 'rs232', 'usb') -- required         |
|                          'serial' is like 'rs232' (*not* cradle)            |
| ("//" palmhostId)?       default is "localhost"  FOR FUTURE USE             |
| ("?bauds=nnnn")?         default is 19200 (57600 for USB)                   |
|                                                                             |
| Ex:                                                                         |
|   "serial:"                                                                 |
|   "serial:?bauds=9600"                                                      |
|   "serial://localost?bauds=9600"                                            |
|                                                                             |
+----------------------------------------------------------------------------*/
void SerialSchemeHandler::Rep::onNew(URI::Data & uri)
{
   char * pCreator = 0;
   char * pType = 0;
   unsigned char subCode = 0;
   SerialStreamBuf::Device device;
   long bauds;

   switch (uri.scheme[0]) {
   case 'u':       // usb
   case 'U':
      device = SerialStreamBuf::USB_DEVICE;
      bauds = 57600;
      break;
   case 'c':       // Cradle
   case 'C':
      device = SerialStreamBuf::CRADLE_DEVICE;
      bauds = 19200;
      break;
// case 's':       // serial
// case 'S':
// case 'r':       // RS-232
// case 'R':
   default:
      device = SerialStreamBuf::RS232_DEVICE;
      bauds = 19200;
      break;
   }

   if (uri.query) {
      static char const aszAttrBauds[] = "bauds=";
      char const * p = uri.query + sizeof aszAttrBauds - 1;
      if (
         (0 == strncasecmp(uri.query, aszAttrBauds, sizeof aszAttrBauds-1)) &&
         ::stol(&p, &bauds) && (*p == '\0')
      ) {
         bauds /= 1200;   // round!
         if (bauds) {
            bauds *= 1200;
         }else {
            bauds = 1200; // minimum
         }
      }else {             // invalid attribute
         throw URI::MalformedException(URI::HANDLER_ERROR, 5);
      }
   }
   {
      SerialPath * p = new SerialPath;
      p->bauds = bauds;
      p->device = device;
      uri.user = p;
   }
}

/*-----------------------------------------SerialSchemeHandler::Rep::onDelete-+
|                                                                             |
+----------------------------------------------------------------------------*/
void SerialSchemeHandler::Rep::onDelete(URI::Data & uri) {
   delete (SerialPath *)uri.user;
}

/*---------------------------------------SerialSchemeHandler::Rep::makeStream-+
|                                                                             |
+----------------------------------------------------------------------------*/
iostream * SerialSchemeHandler::Rep::makeStream(
   URI const & uri, ios__openmode om
) {
   assert (uri.getUser());
   if (!uri.getUser()) {
      return 0;
   }else {
      SerialPath const * p = (SerialPath const *)uri.getUser();
      return new SerialStream(p->device, p->bauds);
   }
}

#ifdef TOOLS_NAMESPACE
}
#endif
/*===========================================================================*/
