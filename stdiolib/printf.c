/*
* $Id: printf.c,v 1.2 2002-04-04 05:10:59 jlatone Exp $
*
* (C) Copyright Jaxo Inc., 2001
* This work contains confidential trade secrets of Jaxo Inc.
* Use, examination, copying, transfer and disclosure to others
* are prohibited, except with the express written agreement of Jaxo Inc.
*
* Author:  Pierre G. Richard
* Written: 04/07/1987
*
*/

/*---------+
| Includes |
+---------*/
#if !defined USE_REAL_JFILEMETHODS
#define USE_REAL_JFILEMETHODS
#endif
#include "stdio.h"
#include <string.h>

/*------------------+
| Local definitions |
+------------------*/
typedef int (*Out_Fct)(unsigned char const *, int, void *);
static int stringOutFct(unsigned char const *, int, void *);
static int fileOutFct(unsigned char const *, int, void *);
static unsigned int stoi(unsigned char const *, unsigned int *);
static unsigned int ulbtos(unsigned long, unsigned char *, int);
static int ivformat(unsigned char const *, va_list, Out_Fct, void *);

/*--------------------------------------------------------------------jprintf-+
|                                                                             |
+----------------------------------------------------------------------------*/
int jprintf(char const * format, ...) {
   va_list arg;
   int len;

   va_start(arg, format);
   len = jvfprintf(jstdout, format, arg);
   va_end(arg);
   return len;
}

/*-------------------------------------------------------------------jvprintf-+
|                                                                             |
+----------------------------------------------------------------------------*/
int jvprintf(char const * format, va_list arg) {
   return jvfprintf(jstdout, format, arg);
}

/*-------------------------------------------------------------------jsprintf-+
|                                                                             |
+----------------------------------------------------------------------------*/
int jsprintf(char * target, char const * format, ...)
{
   va_list arg;
   int len;

   va_start(arg, format);
   len = jvsprintf(target, format, arg);
   target[len] = '\0';
   va_end(arg);
   return len;
}

/*------------------------------------------------------------------jvsprintf-+
|                                                                             |
+----------------------------------------------------------------------------*/
int jvsprintf(char * target, char const * format, va_list arg) {
   return ivformat(format, arg, stringOutFct, (char **)&target);
}

/*---------------------------------------------------------------stringOutFct-+
| Out function for string (e.g. sprintf, vsprintf)                            |
|                                                                             |
| Returns: effective length written                                           |
+----------------------------------------------------------------------------*/
static int stringOutFct(unsigned char const * c_p, int len, void * thunk) {
   memcpy(*(char **)thunk, c_p, len);
   *(char **)thunk += len;
   return len;
}

/*-------------------------------------------------------------------jfprintf-+
|                                                                             |
+----------------------------------------------------------------------------*/
int jfprintf(JFILE * pFile, char const * format, ...)
{
   va_list arg;
   int len;

   va_start(arg, format);
   len = jvfprintf(pFile, format, arg);
   va_end(arg);
   return len;
}

/*------------------------------------------------------------------jvfprintf-+
|                                                                             |
+----------------------------------------------------------------------------*/
int jvfprintf(JFILE * pFile, char const * format, va_list arg) {
   return ivformat(format, arg, fileOutFct, pFile);
}

/*-----------------------------------------------------------------fileOutFct-+
| Out function for file (e.g. fprintf, vfprintf)                              |
|                                                                             |
| Returns: effective length written                                           |
+----------------------------------------------------------------------------*/
static int fileOutFct(unsigned char const * c_p, int len, void * thunk) {
   return jfwrite(c_p, 1, len, (JFILE *)thunk);
}

/*STATIC---------------------------------------------------------------ulbtos-+
| Unsigned Long Based TO String.                                              |
| Convert a long into a non-zero terminated string, given a radix.            |
| The target string must be enough large to handle the result:                |
| 32 bits int: 32 bytes.                                                      |
+----------------------------------------------------------------------------*/
static unsigned int ulbtos(
   unsigned long val,     /* number to convert */
   unsigned char *c_p,    /* address of the string (33 bytes) */
   int radix              /* conversion radix */
) {
   static unsigned char const digit1[]= "0123456789abcdef";
   static unsigned char const digit2[]= "0123456789ABCDEF";
   unsigned char const * digit = digit1;
   unsigned int j = 0;
   unsigned char *d_p, d;

   if (radix < 0) {       /* hack */
      radix = -radix;
      digit = digit2;
   }
   d_p = c_p;
   do {                   /* convert */
      *d_p++ = digit[(unsigned int)(val%radix)];
      ++j;
   }while ((val /= radix) != 0);

   while (--d_p > c_p) {  /* reverse the string */
      d = *c_p;
      *c_p++= *d_p;
      *d_p = d;
   }
   return(j);
}

/*STATIC-----------------------------------------------------------------stoi-+
| String TO Integer conversion.                                               |
| The string is read until a non-numeric character is found,                  |
| or an overflow occurs.  The number of string characters read is returned.   |
| The codeset of the machine is assumed to group all digits in adjacent       |
| bit combinations.                                                           |
+----------------------------------------------------------------------------*/
static unsigned int stoi(unsigned char const *c_p, unsigned int *i_p)
{
   register unsigned int i = 0;
   register unsigned int k;
   unsigned int j;

   for (j=0, k = *c_p;
      ( (k >= '0') && (k <= '9') )      /* cannot use is_digit */
      && ( (i < ((unsigned int)~0)/10)||((i==((unsigned int)~0)/10)&&(k<='5')) );
      k = *(++c_p), ++j) {
      i = ((i +(i<<2))<<1) + k - '0';    /* i*10 + c - '0' */
   }
   *i_p = i;
   return j;
}

/*STATIC-------------------------------------------------------------ivformat-+
|Output a printf_type format_string.                                          |
|                                                                             |
|IMPLEMENTATION NOTES:                                                        |
|                                                                             |
|The float conversion is *NOT* implemented.                                   |
+----------------------------------------------------------------------------*/
static int ivformat(
   unsigned char const * format,
   va_list arg_p,
   Out_Fct out,
   void * thunk
) {
   int total;
   unsigned char const * s_p0;
   unsigned char const * s_p;
   unsigned char buffer[8*sizeof(long)];
   register unsigned char *c_p;
   int radix;
   union {
      struct {
         unsigned int ljustify   : 1;
         unsigned int precision  : 1;
         unsigned int longout    : 1;
         unsigned int showsign   : 1;
      }b;
      int i;
   }flag;
   unsigned int width, precision, length;
   long il;
   int  i, j;
   unsigned char pad;

   total = 0;
   s_p = format - 1;                       /* dummy entry */
   while (*++s_p) {
      if (*(s_p0 = s_p) != '%') {
         if (0 == (s_p = strchr(s_p0, '%'))) {
            length = strlen(s_p0);
            s_p = s_p0 + length-1;
         }else {
            if (*(s_p+1) == '%') {
               length = ++s_p - s_p0;
            }else {
               length = s_p-- - s_p0;
            }
         }
         total += (*out)(s_p0, length, thunk);
         continue;
      }
      flag.i = 0;                          /* reset all flags to zero */
      if (*++s_p=='-') {
         flag.b.ljustify = 1;
         ++s_p;
      }
      if (*s_p == '0') {
         pad = '0';
         ++s_p;
      }else {
         pad = ' ';
      }
      if (*s_p == '*') {
         width = va_arg(arg_p, unsigned int);
         ++s_p;
      }else {
         s_p += stoi(s_p, &width);
      }
      if (*s_p == '.') {
         flag.b.precision = 1;
         if (*s_p == '*') {
            precision = va_arg(arg_p, unsigned int);
            ++s_p;
         }else {
            s_p += stoi(++s_p, &precision);
         }
      }
      if (*s_p == 'l') {
         flag.b.longout = 1;
         ++s_p;
      }
      switch(*s_p) {
      case 's':
         radix = 0;
         length = strlen(c_p = va_arg(arg_p, unsigned char *));
         if (flag.b.precision && precision<length) length=precision;
         break;
      case 'c':
         radix = 0;
         c_p = buffer;
         /* Note: unsigned int instead of unsigned char ! */
         *c_p = (unsigned char)va_arg(arg_p, unsigned int);
         length = 1;
         break;
      case 'b':
         radix = 2;
         break;
      case 'o':
         radix = 8;
         break;
      case 'u':
         radix = 10;
         break;
      case 'x':
         radix = 16;
         break;
      case 'X':
         radix = -16;  /* hack */
         break;
      case 'd':
         flag.b.showsign = 1;
         radix = 10;
         break;
      default:         /* just in case... */
         total += (*out)(s_p0, s_p - s_p0 + 1, thunk);
         continue;
      }
      if (radix) {
         c_p = buffer;
         j = 0;
         if (flag.b.longout) {
            if (((il = va_arg(arg_p, long)) < 0) && flag.b.showsign) {
               il = -il;
               *c_p = '-';
               j = 1;
            }
         }else {
            if (((i = va_arg(arg_p, int)) < 0) && flag.b.showsign) {
               i = -i;
               *c_p = '-';
               j = 1;
            }
            il = (unsigned int)i;
         }
         length = j + ulbtos((unsigned long)il, c_p+j, radix);
      }
      if (!flag.b.ljustify) {
         while (width > length) {
            total += (*out)(&pad, 1, thunk);
            --width;
         }
      }
      total += (*out)(c_p, length, thunk);
      while (width > length) {
         total += (*out)(&pad, 1, thunk);
         --width;
      }
   }
   return total;
}
/*===========================================================================*/

