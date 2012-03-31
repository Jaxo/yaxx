/*
* $Id: memcmp.h,v 1.2 2002-04-26 09:15:39 pgr Exp $
*/
#ifndef MEMCMP_FIX_4_PALMOS30_DEFINED
#define MEMCMP_FIX_4_PALMOS30_DEFINED

/*---------+
| Includes |
+---------*/
#include <PalmOs.h>
#include <stddef.h>

__inline Int16 memcmpFix4PalmOS30(
   void const * src1,
   void const * src2,
   Int32 n
) {
   if (n <= 0) {
      return 0;
   }else {
      unsigned char const * p1 = (unsigned char const *)src1;
      unsigned char const * p2 = (unsigned char const *)src2;
      for (;;) {
         if (*p1 != *p2) return (*p1 > *p2)? 1 : -1;
         if (!--n) return 0;
         ++p1;
         ++p2;
      }
   }
}

#endif
/*===========================================================================*/
