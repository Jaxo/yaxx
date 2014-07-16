/* $Id: TestSerialStream.cpp,v 1.1 2002/09/15 07:46:30 pgr Exp $ */

#include "testsRsc.h"
#include "TestSerialStream.h"
#include "SerialStream.h"

/*-------------------------------------------TestSerialStream::newInOutStream-+
|                                                                             |
+----------------------------------------------------------------------------*/
iostream * TestSerialStream::newInOutStream() {
   FrmAlert(SerialStartAlert);
   return new SerialStream(
      SerialStreamBuf::RS232_DEVICE,
      19200
   );
}

/*----------------------------------------------------TestSerialStream::title-+
|                                                                             |
+----------------------------------------------------------------------------*/
char const * TestSerialStream::title() {
   return "TestSerialStream";
}

/*===========================================================================*/
