/*
* $Id: testReaderWriter.cpp,v 1.8 2002-08-29 09:58:08 pgr Exp $
*
* Simulation of Transient Streams
*
* TransientFileStreamBuf simulates the behaviour of a transient streams
* by returning a random number of bytes available at a given time.
* This number can be as well 0, indicating the stream is temporarly
* interrupted.
*/

#include "TransientFileStreamBuf.h"

/*-----------------------------------------------------------testReaderWriter-+
|                                                                             |
+----------------------------------------------------------------------------*/
int testReaderWriter() {
   cout << "--------------- testReaderWriter" << endl;
   TransientFileStreamBuf sb;
   if (!sb.open("../tests/data/Utf8Sample.txt", OPEN_READ_FLAGS)) {
      cout << "open failed!" << endl;
      return 1;
   }else {
      Encoder utf8(EncodingModule::UTF_8);
      #ifdef _WIN32
         Encoder outEnc(EncodingModule::CP_850);
      #else
         Encoder outEnc(EncodingModule::ISO_8859);
      #endif
      MultibyteReader reader(&sb, utf8, MultibyteReader::noCatchSoftEof);
      MultibyteWriter writer(cout.rdbuf(), outEnc);
      int len = 100;
      for (int times=0; times < 5; ++times) {
//       int pos = reader.tellg();
         for (;;) {
            try {
               UCS_2 const * pGet = reader.str(len);
               if (!pGet) { // Can't peek that many characters!
                  len = reader.rdbuf()->in_avail();
                  if (!len) {   // EOF
                     sb.close();
                     return 0;
                  }
               }else {
                  writer << setW(len) << pGet << endl << flush;
                  reader.skip(len);
                  break;
               }
            }catch (FileInEmptyException) {
               cout << "*** No more couscous! ***" << endl;
//             reader.seekg(pos);
            }
         }
      }
      sb.close();
      return 0;
   }
}

/*===========================================================================*/
