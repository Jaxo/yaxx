/* $Id: testBeamer.cpp,v 1.3 2002/09/18 07:08:35 pgr Exp $ */

#include "BeamerStream.h"
#include "screen.h"

static int what = 0;
// extern BeamerStreamBuf * g_pBeamerStreamBuf;

/*-----------------------------------------------------------testBeamerStream-+
|                                                                             |
+----------------------------------------------------------------------------*/
void testBeamerStream();
void testBeamerStream()
{
   static char msg0[] = "Please, beam a xxx.RXT file with some ascii text.";
   char buf[100];

   iostream * io = new BeamerStream();
   io->write(msg0, sizeof msg0 - 1);
   int len = io->rdbuf()->sgetn(buf, sizeof buf);
   display(buf, cleanUpCR(buf, len));
   delete io;
}

/*===========================================================================*/
