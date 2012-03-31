/*                                                    c
* $Id: miscutil.cpp,v 1.8 2002-08-19 02:35:30 jlatone Exp $
*/

/*--------------+
| Include Files |
+--------------*/
#include "miscutil.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*----------------------------------------------------------------strncasecmp-+
|                                                                             |
+----------------------------------------------------------------------------*/
int strncasecmp(char const * s1, char const * s2, int n)
{
   while (n--) {
      char c1 = *(s1++);
      char c2 = *(s2++);

      if ((c1 >= 'a') && (c1 <= 'z')) c1 -= 0x20;
      if ((c2 >= 'a') && (c2 <= 'z')) c2 -= 0x20;
      if (c1 < c2) return -1;
      if (c1 > c2) return 1;
      if (!c1) return 0;
   }
   return 0;
}

/*-----------------------------------------------------------------strcasecmp-+
|                                                                             |
+----------------------------------------------------------------------------*/
int strcasecmp(char const * s1, char const * s2)
{
   for (;;) {
      char c1 = *(s1++);
      char c2 = *(s2++);

      if ((c1 >= 'a') && (c1 <= 'z')) c1 -= 0x20;
      if ((c2 >= 'a') && (c2 <= 'z')) c2 -= 0x20;
      if (c1 < c2) return -1;
      if (c1 > c2) return 1;
      if (!c1) return 0;
   }
}

/*-----------------------------------------------------------------------stol-+
| String TO Long conversion.                                                  |
| The string is read until a non-numeric character is found.                  |
| If too big, return false.                                                   |
| Else set resulting value into i_p, update the string pointer, return true.  |
+----------------------------------------------------------------------------*/
bool stol(char const ** c_pp, long * i_p)
{
   long i = 0L;
   unsigned int k;
   char const * c_p;
   bool change_sign = false;

   switch (*(c_p = *c_pp)) {
   case '-':
      change_sign = true;
      /* fall thru */
   case '+':
      if (!*(c_p+1)) return false;
      break;
   default:
      --c_p;                            /* dummy entry */
      break;
   }
   /* Transform the lengthed string into an int */
   while (((k = *++c_p) <= '9') && (k >= '0')) {
      if (
         (i>=((unsigned long)~0)/10) &&
         ((i > ((unsigned long)~0)/10) || (k > '5'))
      ) {
         return false;
      }
      i = ((i +(i<<2))<<1) + k - '0';    /* i*10 + k - '0' */
   }
   *i_p = (change_sign) ? -i : i;
   *c_pp = c_p;
   return true;
}


static char const digit[]= "0123456789ABCDEF";

/*----------------------------------------------------------------------ltols-+
| Long TO String conversion                                                   |
| Convert a long integer into a non-zero terminated decimal string.           |
|  - val is the integer to convert                                            |
|  - c_p points the (enough big) buffer that will contain the result          |
| The length of the string is returned                                        |
+----------------------------------------------------------------------------*/
unsigned int ltols(long val, char *c_p)
{
   unsigned int j;
   char *d_p, d;

   if (val < 0) {
      val = -val;
      *(c_p++) = '-';
      j = 1;
   }else {
      j = 0;
   }
   d_p = c_p;
   do {                                /* convert */
      *d_p++ = digit[(unsigned int)(val%10)];
      ++j;
   }while ((val /= 10) != 0);

   while (--d_p > c_p) {               /* reverse the string */
      d = *c_p;
      *c_p++= *d_p;
      *d_p = d;
   }
   return j;
}

/*---------------------------------------------------------------------ulbtos-+
| Unsigned Long Based TO String                                               |
| Convert a long into a non-zero terminated string, given a radix.            |
| The target string must be enough large to handle the result:                |
| 32 bits int: 32 bytes.                                                      |
+----------------------------------------------------------------------------*/
unsigned int ulbtos(
   unsigned long val,     // number to convert
   char * c_p,            // address of the string (33 bytes)
   int radix              // conversion radix
) {
   unsigned int j = 0;
   char *d_p, d;

   d_p = c_p;
   do {                   // convert
      *d_p++ = digit[(unsigned int)(val%radix)];
      ++j;
   }while ((val /= radix) != 0);

   while (--d_p > c_p) {  // reverse the string
      d = *c_p;
      *c_p++= *d_p;
      *d_p = d;
   }
   return j;
}

/*----------------------------------------------------------------------cb2lu-+
| Character Based to Long Unsigned                                            |
+----------------------------------------------------------------------------*/
static unsigned int cb2lu(char const * s, int len, unsigned char radix)
{
    unsigned char c;
    unsigned int res = 0;
    while (len-- && (c = charToValue(*s++), c < radix)) {
       res = (res * radix) + c;
    }
    return res;
}

/*----------------------------------------------------------------------c2int-+
| Convert 'WXYZ' to his equivalent int value.  WARNING: no checks!            |
+----------------------------------------------------------------------------*/
int c2int(char const * p) {
   int u = 0;
   while (*p) {
      u = (u << 8) + *p++;
   }
   return u;
}

/*-------------------------------------------------------------------hex2uint-+
| Quite equivalent to a regular char2uint, except:                            |
|  - non-hexadecimal characters are replaced by '0'                           |
|  - the radix is 16 ('16-10' below, used to be 'radix-10')                   |
+----------------------------------------------------------------------------*/
unsigned int hex2uint(char const * s, int len)
{
    unsigned int res = 0;
    while (len--) {
       char c = *s++;    // unsigned char c = charToValue(*s++);
       res <<= 4;
       if (c <= '9') {
          if (c >= '0') res += (c & 0xF);
       }else if ((unsigned char)(c = (c & ~('a'-'A'))-'A') < (16-10)) {
          res += (c + 10);
       }
    }
    return res;
}

#ifdef TOOLS_NAMESPACE
}
#endif
/*===========================================================================*/
