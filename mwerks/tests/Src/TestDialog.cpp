/* $Id: TestDialog.cpp,v 1.2 2002/09/17 08:47:34 pgr Exp $ */

#include <string.h>
#include "screen.h"
#include "testsRsc.h"
#include "TestDialog.h"

/*------------------------------------------------------------TestDialog::run-+
|                                                                             |
+----------------------------------------------------------------------------*/
void TestDialog::run()
{
   static char const msg0a[] = "---------- \"";
   static char const msg0b[] = "\"\n";
   static char const msg0[] = "Palm Device wants to read you, OK? (Y/N): ";
   static char const msg1[] = "More to say? (Y/N): ";
   static char const msg2[] = "End of connection.  Au revoir!\n";
   static char const msg3[] = "Enter a line, please.\n";

   display(msg0a, sizeof msg0a - 1);
   display(title(), strlen(title()));
   display(msg0b, sizeof msg0b - 1);

   iostream * io = newInOutStream();
   char * buf = new char[500];
   io->write(msg0, sizeof msg0 - 1);
// io->seekg(0, ios::end);
   for (;;) {
      int answ;
//    io->seekg(0, ios::end);
      answ = io->get();
//    io->seekp(0, ios::end);
      if ((answ != 'Y') && (answ != 'y')) {
         io->write(msg2, sizeof msg2 - 1);
         io->flush();
         FrmAlert(NoServerAlert);
         break;
      }else {
         io->write(msg3, sizeof msg3 - 1);
//       io->seekg(0, ios::end);
         io->get(buf, 500, '\n');
         int len = io->gcount();
         if (len < 500) {
            // following gets the '\n' and... avoids an infinite loop!
            buf[len++] = io->get(); // this gets '\n'
         }
         display(buf, cleanUpCR(buf, len));
      }
      io->write(msg1, sizeof msg1 - 1);
   }
   delete [] buf;
   delete io;
}

/*===========================================================================*/
