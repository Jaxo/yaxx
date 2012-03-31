/*
* $Id: sort.cpp,v 1.3 2002-04-11 04:12:38 jlatone Exp $
*/

/*--------------+
| Include Files |
+--------------*/
#include "sort.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*PRIVATE---------------------------------------------SortFunctor::hoares_sort-+
| recursive function to sort array[beg..end] using Hoares algorithm            |
+-----------------------------------------------------------------------------*/
void SortFunctor::hoares_sort(int beg, int end)
{
   int k = end + 1;
   while (beg < end) {
      int j = beg;
      /*-------------------------------------------------+
      |Get "k" such that:                                |
      |in the range beg+1 to k, all values are <= V[beg] |
      +-------------------------------------------------*/
      for (;;) {
         while ((++j < k) && (compare(j, beg) <= 0))
            ;
         while ((--k > j) && (compare(k, beg) >= 0))
            ;
         if (k > j) {
            exchange(j, k);
            continue;
         }
         if (j == k) --k;
         break;
      }
      if (k != beg) exchange(beg, k);

      /*--------------------+
      |Sort the lowest range|
      +--------------------*/
      if ((k - beg) >= (end - k)) {
         hoares_sort(k+1, end);
         end = k-1;
         /* k = end+1; */
      }else {
         hoares_sort(beg, k-1);
         beg = k+1;
         k = end+1;
      }
   }
}

/*---------------------------------------------------------SortFunctor::quick-+
| Main routine:                                                               |
| Check if we need to sort the array, then call the recursive routine.        |
+----------------------------------------------------------------------------*/
void SortFunctor::quick(
   int const iCount      // number of items to sort
)
{
   int i = 0;
   while (++i < iCount) {
      if (compare(i-1, i) > 0) {     // at least one...
         hoares_sort(0, iCount - 1);
         break;
      }
   }
}

#ifdef TOOLS_NAMESPACE
}
#endif
/*===========================================================================*/
