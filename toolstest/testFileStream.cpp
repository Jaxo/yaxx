/*
* $Id: testFileStream.cpp,v 1.12 2002-09-06 06:42:03 pgr Exp $
*
* This code tests random positioning of StdFileInStream's.
*/

#include <string.h>

#ifndef _WIN32
#include <unistd.h>
#endif

#include "../toolslib/StdFileStream.h"

static void testBothInputOutput();
#define STRING__REASONNABLE_LENGTH 100   // :-)

/*-------------------------------------------------------------testFileStream-+
|                                                                             |
+----------------------------------------------------------------------------*/
int testFileStream()
{
   cout << "--------------- testFileStream" << endl;

   testBothInputOutput();

   char const * path = "../tests/data/alice.xml";
   char buf[STRING__REASONNABLE_LENGTH];
   streampos pos;
   StdFileInStream alice("../tests/data/alice.xml");
   int i;

   cout << "opening" << path << "..." << endl;
   if (!alice) {
      cout << "open failed!" << endl;
      return 1;
   }

   cout << "the first five words of this file are :" << endl;
   for (i=0; i < 5; ++i) {
      if (i==1) {
         pos = alice.tellg();        // remember the position of the 2nd word
      }
      alice >> buf;
      cout << '\"' << buf << "\"  ";
   }

   cout << endl << "Trying seeks... " << endl;
   alice >> buf;
   cout << "Is: \"" << buf << "\" equal to: \"";
   {
      int iLen = strlen(buf);
      #ifdef _WIN32  // Visual C++ bug fix.
         ansiRead(alice, buf+iLen, sizeof buf - iLen);
      #else
         alice.read(buf+iLen, sizeof buf - iLen);
      #endif
   }
   alice.seekg(-(int)(sizeof buf), ios::cur);
   buf[0] = '\0';
   alice >> buf;
   cout << buf << "\"?" << endl;

   for (i=0; i < 8; ++i) {
      #ifdef _WIN32  // Visual C++ bug fix.
         ansiRead(alice, buf, sizeof buf);
      #else
         alice.read(buf, sizeof buf);
      #endif
   }
   {                                    // soft seek (I hope..)
      char *cp = buf + sizeof buf;
      int delta = -1;
      while (*--cp != ' ') --delta;     // last blank
      while (*--cp == ' ') --delta;
      *(cp+1) = '\0';                   // temp end
      for (;;) {                        // start at word before
         --delta;
         switch (*--cp) {
         case ' ':
            break;
         case '\r':
         case '\n': *cp='\0';           // new end if space char
         default:
            continue;
         }
         break;
      }
      cout << "Is: \"" << buf + sizeof buf + delta << "\" equal to: \"";
      alice.seekg(delta, ios::cur);
      alice >> buf;  cout << buf << "\"?" << endl;
   }
   alice.seekg(pos);
   alice >> buf;
   cout << "Is: \"" << buf << "\" the 2nd word of this file?" << endl;

   bool letterFound = false;
   for (i=1; ;++i) {
      char c = 0;
      alice.seekg(-i, ios::end);
      if (c = alice.get(), ((letterFound) && (c <= ' '))) break;
      if (c > ' ') letterFound = true;
      alice.clear();
   }
// alice.seekg(-20, ios::end);
   alice >> buf;
   cout << "Is: \"" << buf << "\" the last word of this file?" << endl;
   alice.seekg(pos);
   alice >> buf;
   cout << "Is: \"" << buf << "\" the 2nd word of this file?" << endl;
   alice.sync();
   alice >> buf;
   cout << "Is: \"" << buf << "\" the 3rd word of this file?" << endl;
   return 1;
}

/*--------------------------------------------------------testBothInputOutput-+
|                                                                             |
+----------------------------------------------------------------------------*/
#if defined _WIN32
#include <io.h>
#endif
void testBothInputOutput()
{
   {
      StdFileOutStream foo("xyz");
      for (int i=0; i < 30; ++i) {
         foo << "Hello" << i << " ";
      }
   }
   {
      char x[100];
      StdFileStream foo("xyz");
      foo.read(x, 6);                        foo.seekp(6, ios::beg);
      cout << foo.tellg() << endl;
      foo.write("Giuseppe ", 9);             foo.seekg(9, ios::cur);
      cout << foo.tellg() << endl;
      foo.seekp(30, ios::beg);               foo.seekg(30, ios::beg);
      cout << foo.tellg() << endl;
      foo.write("Toto ", 5);                 foo.seekg(5, ios::cur);
      foo.read(x, 10);                       foo.seekp(10, ios::cur);
      cout.write(x, 10);
      foo.write("Titi ", 5);                 foo.seekg(5, ios::cur);
      foo.seekg(0, ios::beg);
      cout << endl << foo.rdbuf() << endl;
   }
   unlink("xyz");
}

/*===========================================================================*/
