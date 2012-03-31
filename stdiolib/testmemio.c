/*
  compile with:
  cl -I. -Ie:\java\include -Ie:\java\include\win32 testmemio.c /link stdio.lib
*/
#include "stdio.h"
#include <io.h>

print(char const * format, ...)
{
   va_list arg;
   char buf[1000];

   va_start(arg, format);
   vsprintf(buf, format, arg);
   va_end(arg);
   write(2, buf, strlen(buf));
}

int main(int argc, char ** argv)
{
   char buf[1000];
   char const test[] =
      "There was nothing so VERY remarkable in that; nor did Alice think it so"
      " VERY much out of the way to hear the Rabbit say to itself, `Oh dear!"
      " Oh dear!  I shall be late!' (when she thought it over afterwards, it"
      " occurred to her that she ought to have wondered at this, but at the"
      " time it all seemed quite natural); but when the Rabbit actually TOOK A"
      " WATCH OUT OF ITS WAISTCOAT-POCKET, and looked at it, and then hurried"
      " on, Alice started to her feet, for it flashed across her mind that she"
      " had never before seen a rabbit with either a waistcoat-pocket, or a"
      " watch to take out of it, and burning with curiosity, she ran across the"
      " field after it, and fortunately was just in time to see it pop down a"
      " large rabbit-hole under the hedge.";
   FILE * fm;
   char const * memName;
   int i, count;

   memName = acquireMemoryFileName();
// print("Name is: %s\n", memName);
   { char bof[200]; sprintf(bof, "Name is: %s (size of my test: %d)\n\nFGETC output is:\n", memName, sizeof test -1); write(2, bof, strlen(bof)); }

   /* testing putc */
   fm = fopen(memName, "w");
   for (i=0; i < sizeof test - 1; ++i) {
      putc(test[i], fm);
   }
   fclose(fm);

   /* testing getc */
   fm = fopen(memName, "r");
   for (i=0; i < sizeof test - 1; ++i) {
      int c = fgetc(fm);
      { char bof[1]; bof[0] = c; write(2, bof, 1); }
//    print("%c", c);
   }
   fclose(fm);

   /* testing fwrite */
   fm = fopen(memName, "w");
   fwrite(test, sizeof test-1, 1, fm);
   fclose(fm);

   /* testing fread */
   fm = fopen(memName, "r");
   count = fread(buf, sizeof buf, 1, fm);
   buf[count++] = '<';
   buf[count++] = '<';
   buf[count++] = '<';
   buf[count++] = '<';
   buf[count++] = '\0';
// print("\n\nFREAD output is:\n%s\n", buf);
   { char bof[2000]; sprintf(bof, "\n\nFREAD output is:\n%s\n", buf); write(2, bof, strlen(bof)); }

   releaseMemoryFileName(memName);
}

