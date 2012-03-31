/* $Id: BinarySearch.cpp,v 1.2 2002-05-14 15:14:40 pgr Exp $ */

#include "BinarySearch.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*-------------------------------------------------BinarySearch::BinarySearch-+
| Constructor                                                                 |
+----------------------------------------------------------------------------*/
BinarySearch::BinarySearch(int size) : m_size(size-1) {
}

/*-------------------------------------------------------BinarySearch::search-+
|                                                                             |
+----------------------------------------------------------------------------*/
int BinarySearch::search() const
{
   int cond, low, mid, num;

   low = 0;                        /* low bound */
   num = m_size;
   while (low <= num) {
      mid=((unsigned int)low + (unsigned int)num) >> 1;
      cond = compare(mid);
      if (cond < 0) {
         num = mid - 1;         /* set low range */
      }else if (cond > 0) {
         low = mid + 1;
      }else {                      /* found */
         return(mid);
      }
   }
   return(-1);
}


#ifdef TOOLS_NAMESPACE
}
#endif
/*===========================================================================*/
