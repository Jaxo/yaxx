/*
* $Id: testTransient.cpp,v 1.6 2002-06-05 13:12:24 pgr Exp $
*
* Simulation of Transient Streams
*
* TransientFileStreamBuf simulates the behaviour of a transient streams
* by returning a random number of bytes available at a given time.
* This number can be as well 0, indicating the stream is temporarly
* interrupted.
*/

#include "TransientFileStreamBuf.h"

/*--------------------------------------------------------------testTransient-+
|                                                                             |
+----------------------------------------------------------------------------*/
int testTransient()
{
   cout << "--------------- testTransient" << endl;
   TransientFileStreamBuf sb;
   if (!sb.open("../tests/data/Utf8Sample.txt", OPEN_READ_FLAGS)) {
      cout << "open failed!" << endl;
      return 1;
   }else {
      char buf[1000];
      for (;;) {
         try {
            int len = sb.sgetn(buf, sizeof buf);
            if (len == 0) {
               cout << "[EOF]" << endl;
               break;
            }
            buf[len] = '\0';
            cout << buf << "\n(" << len << ")<<" << endl;
         }catch (FileInEmptyException) {
            cout << "*** No more couscous! ***" << endl;
         }
      }
      sb.close();
   }
   return 0;
}

/*===========================================================================*/
