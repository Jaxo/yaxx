/*
* $Id: SerialStream.h,v 1.8 2002-08-26 14:26:21 pgr Exp $
*/

#ifndef COM_JAXO_PALMIO_SERIALSTREAM_H_INCLUDED
#define COM_JAXO_PALMIO_SERIALSTREAM_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include "FileStreamBuf.h"
#include "migstream.h"
#include "iostream.h"
#include "URI.h"


#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*---------------------------------------------------- class SerialStreamBuf -+
|                                                                             |
+----------------------------------------------------------------------------*/
class TOOLS_API SerialStreamBuf : public FileStreamBuf {
public:
   SerialStreamBuf();
   enum Device {
      RS232_DEVICE,
      CRADLE_DEVICE,
      USB_DEVICE
   };

   FileStreamBuf * open(Device device, int bauds);
   FileStreamBuf * close();

private:
   bool m_isPortOpened;
   UInt32 m_whatSerial;
   UInt16 m_portID;

   int read(char * pchBuf, int iLen);
   int write(char const * pchBuf, int iLen);
   long lseek(long offset, int origin);

   SerialStreamBuf & operator=(SerialStreamBuf const & source); // no!
   SerialStreamBuf(SerialStreamBuf const & source);             // no!
};


/* -- INLINES -- */
inline long SerialStreamBuf::lseek(long offset, int origin) { return EOF; }


/*---------------------------------------------------------------SerialStream-+
| SerialStream IS-A iostream to manipulate a SerialStreamBuf.                 |
+----------------------------------------------------------------------------*/
class TOOLS_API SerialStream: public iostream {
public:
   SerialStream(
      SerialStreamBuf::Device device = SerialStreamBuf::CRADLE_DEVICE,
      int bauds = 19200
   );
   ~SerialStream();

private:
   SerialStreamBuf m_buf;

   SerialStream & operator=(SerialStream const & source); // no!
   SerialStream(SerialStream const & source);             // no!
};

/*--------------------------------------------------------SerialSchemeHandler-+
|                                                                             |
+----------------------------------------------------------------------------*/
class TOOLS_API SerialSchemeHandler : public URI::SchemeHandler {
public:
   SerialSchemeHandler();

private:
   class Rep : public URI::SchemeHandler::Rep {
      char const * getID() const;
      bool matchesScheme(char const * scheme, int len) const;
      void onNew(URI::Data & uri);
      void onDelete(URI::Data & uri);
      iostream * makeStream(URI const & uri, ios__openmode om);
   };
};

inline SerialSchemeHandler::SerialSchemeHandler(
) :
   SchemeHandler(new Rep) {
}

#ifdef TOOLS_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
