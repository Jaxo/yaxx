/* $Id: testEscaper.cpp,v 1.2 2002-06-23 10:31:45 pgr Exp $ */

#include <assert.h>
#include <conio.h>
#include <stdio.h>
#include <string.h>
#include "..\toolslib\Escaper.h"
/*---------------------------------------*/
static char buffer[102] = { 100 };  /* Maximum characters in 1st byte */
static char const * readLine() {
   printf( "--------------\n");
   return _cgets(buffer);
}
/*---------------------------------------*/
class MyConsole : public Escaper {
public:
   MyConsole();
   void display(char const * pchBuf, int len) {
      process(pchBuf, len);
   }
private:
   void output(char const * pchBuf, int len) {
      printf("=> \"%.*s\"\n", len, pchBuf);
   };
   class ClearDecoder : public Escaper::ClearDecoder {
   public:
      ClearDecoder(MyConsole & console) : m_console(console) {}
      void action() { printf("=> [CLEAR]"); }
      MyConsole & m_console;
   };
};

MyConsole::MyConsole() {
   registerDecoder(new ClearDecoder(*this));
}

/*----------------------------------------------------------------testEscaper-+
|                                                                             |
+----------------------------------------------------------------------------*/
int testEscaper()
{
   MyConsole console;
   for (;;) {
      char const * p = readLine();
      console.display(p, strlen(p));
   }
   return 0;
}

/*===========================================================================*/
