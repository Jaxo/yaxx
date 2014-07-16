/* $Id: TestBase.cpp,v 1.1 2002/09/15 07:46:30 pgr Exp $ */

#include <string.h>
#include "TestBase.h"
#include "MemStream.h"
#include "StringBuffer.h"
#include "testsRsc.h"
#include "screen.h"

/*--------------------------------------------------------------TestBase::run-+
|                                                                             |
+----------------------------------------------------------------------------*/
void TestBase::run()
{
   static char const msg0a[] = "---------- \"";
   static char const msg0b[] = "\"\n";
   static char const msg1[] = "Write a record...\n";
   static char const msg2[] = "Modifying...\n";
   static char const msg3[] = "Re-reading...\n";
   static char const msg4[] = "--- Alice Test ---\n";
   static char const msg5[] = " => success!\n";

   char const hello[] = "Hello, world!";
   char const newGuy[] = "Giuseppe!";

   display(msg0a, sizeof msg0a - 1);
   display(title(), strlen(title()));
   display(msg0b, sizeof msg0b - 1);

   display(msg1, sizeof msg1 - 1);
   if (!createAndWrite(hello)) return;

   display(msg2, sizeof msg2 - 1);
   if (!modify(newGuy)) return;

   display(msg3, sizeof msg3 - 1);
   if (!reread()) return;

   display(msg4, sizeof msg4 - 1);
   aliceTest();

   display(title(), strlen(title()));
   display(msg5, sizeof msg5 - 1);
   return;
}

/*--------------------------------------------------------TestBase::aliceTest-+
|                                                                             |
+----------------------------------------------------------------------------*/
void TestBase::aliceTest()
{
   iostream * io = newInOutStream();
   bool isLongText = true;

   for (int times=0; times < 2; ++times) {
      writeString(io, isLongText);
      readString(io);
      isLongText ^= 1;
   }
}

/*---------------------------------------------------TestBase::createAndWrite-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool TestBase::createAndWrite(char const * toWrite)
{
   MemStream msg;
   ostream * os = newOutStream();
   if (hasFailed()) return false;

   // Write something to it.
   os->write(toWrite, strlen(toWrite));
   if (hasFailed()) return false;

   // How big is the stream so far?
   os->seekp(0, ios::end);
   if (hasFailed()) return false;
   msg << "Record's length equals " << os->tellp()
       << " bytes after we wrote \"" << toWrite <<  "\" to it." << endl;
   display(msg.str(), msg.pcount());
   msg.rdbuf()->freeze(0);

   return true;
}

/*-----------------------------------------------------------TestBase::modify-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool TestBase::modify(char const * newGuy)
{
   static char const msg0[] = "EOF? tellg said: ";
   static char const msg1[] = "EOF? peek said: ";
   static char const msg2[] = "YES.\n";
   static char const msg3[] = "NO.\n";
   static char const msg4[] = "Read back the following: \n\"";
   static char const msg5[] = "Stream's length now equals ";
   static char const msg6[] = " bytes after replacing \"world\" with \"";
   static char const msg7[] = "\".\n";
   char buffer[100];
   MemStream msg;
   int size;

   // open the existing file
   iostream * io = newInOutStream();
   if (hasFailed()) return false;

   // Read what we wrote.
   io->read(buffer, sizeof buffer);
   io->clear();        // clear the EOF bit
   size = io->gcount();
   buffer[size] = '\0';
   msg << msg4 << buffer << msg7;
   display(msg.str(), msg.pcount());
   msg.rdbuf()->freeze(0);
   msg.reset();

   // Are we at EOF?
   io->seekg(0, ios::end);
   display(msg0, sizeof msg0 - 1);
   if (io->tellg() == size) {
      display(msg2, sizeof msg2 - 1);
   }else {
      display(msg3, sizeof msg3 - 1);
   }
   display(msg1, sizeof msg1 - 1);
   if (io->peek() == EOF) {
      display(msg2, sizeof msg2 - 1);
   }else {
      display(msg3, sizeof msg3 - 1);
   }

   // Seek to the "world," in "Hello, world!";
   io->seekp(7, ios::beg);
   if (hasFailed()) return false;

   // replace it with (newGuy),
   io->write(newGuy, strlen(newGuy));
   if (hasFailed()) return false;

   msg << msg5 << io->tellp() << msg6 << newGuy << msg7;
   display(msg.str(), msg.pcount());
   msg.rdbuf()->freeze(0);
   msg.reset();

   // rewind, and read it back.
   io->seekg(0, ios::beg);
   io->read(buffer, sizeof buffer);
   buffer[io->gcount()] = '\0';
   msg << msg4 << buffer << msg7;
   display(msg.str(), msg.pcount());
   msg.rdbuf()->freeze(0);

   return true;
}

/*-----------------------------------------------------------TestBase::reread-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool TestBase::reread()
{
   char buffer[100];
   MemStream msg;

   istream * is = newInStream();
   if (hasFailed()) return false;

   is->read(buffer, sizeof buffer-1);
   is->clear();

   buffer[is->gcount()] = '\0';
   msg << "Read the following " << is->gcount()
       << " bytes: \n\"" << buffer
       << "\"."  << endl;
   display(msg.str(), msg.pcount());
   msg.rdbuf()->freeze(0);

   return true;
}

/*------------------------------------------------------------------cleanUpCR-+
|                                                                             |
+----------------------------------------------------------------------------*/
void TestBase::cleanUpCR(StringBuffer & sb)
{
   char const * pCur;
   int start = 0;
   while (
      pCur = (char const *)memchr(
         ((char const *)sb) + start,
         '\r',
         sb.length() - start
      ),
      pCur != 0
   ) {
      start = pCur - (char const *)sb;
      sb.remove(start, start+1);
   }
}

/*------------------------------------------------------TestBase::writeString-+
|                                                                             |
+----------------------------------------------------------------------------*/
void TestBase::writeString(iostream * io, bool isLongText)
{

   static char const longText[] =
      "`Well!' thought Alice to herself, `after such a fa"
   /*  ----+----1----+----2----+----3----+----4----+----5 */
      "ll as this, I shall think nothing of tumbling down"
   /*  ----+----6----+----7----+----8----+----9----+----0 */
      " stairs!  How brave they'll all think me at home! "
   /*  ----+----1----+----2----+----3----+----4----+----5 */
      "Why, I wouldn't say anything about it, even if I f"
   /*  ----+----6----+----7----+----8----+----9----+----0 */
      "ell off the top of the house!'";
   /*  ----+----1----+----2----+----3----+----4----+----5 */

   static char const shortText[] =
      "In another moment down went Alice after it, never "
   /*  ----+----1----+----2----+----3----+----4----+----5 */
      "once considering how in the world she was to get o"
   /*  ----+----6----+----7----+----8----+----9----+----0 */
      "ut again.";
   /*  ----+----1----+----2----+----3----+----4----+----5 */

   if (isLongText) {
      io->write(longText, sizeof longText-1);
   }else {
      io->write(shortText, sizeof shortText-1);
   }
}

/*-------------------------------------------------------TestBase::readString-+
|                                                                             |
+----------------------------------------------------------------------------*/
void TestBase::readString(iostream * io)
{
   int size = io->tellp();
   StringBuffer sb(size);
   sb.append(*io->rdbuf(), size);  // read it
   sb.append('\n');
   io->seekg(0);
   io->seekp(0);
   display(sb, sb.length());
   cleanUpCR(sb);
}

/*===========================================================================*/
