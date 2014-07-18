/* $Id: TestNetStream.cpp,v 1.2 2002/09/17 07:23:19 pgr Exp $ */

#include <string.h>
#include "testsRsc.h"
#include "TestNetStream.h"
#include "NetStream.h"
#include "miscutil.h"

#define whatMachine "10.110.2.36"
#define whatPort 6410

/*---------------------------------------------------------------------inform-+
|                                                                             |
+----------------------------------------------------------------------------*/
static void inform(char const * machine, int port) {
   char msg[100];
   char * cp;
   strcpy(msg, "Reading the TCP/IP port ");
   cp = msg + strlen(msg);
   strcpy(cp + ltols(port, cp),  " of host: ");
   strcat(cp, machine);
   FrmCustomAlert(MessageBoxInfoAlert, msg, "", "");
}


/*----------------------------------------------TestNetStream::newInOutStream-+
|                                                                             |
+----------------------------------------------------------------------------*/
iostream * TestNetStream::newInOutStream() {
   inform(whatMachine, whatPort);
   return new NetStream(whatMachine, whatPort);
}

/*-------------------------------------------------------TestNetStream::title-+
|                                                                             |
+----------------------------------------------------------------------------*/
char const * TestNetStream::title() {
   return "TestNetStream";
}

/*===========================================================================*/
