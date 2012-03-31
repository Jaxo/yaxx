/*
* $Id: new.cpp,v 1.3 2002-11-14 06:06:50 pgr Exp $
*/
#if defined _WIN32
#include "toolsgendef.h"

#include <stdio.h>    /* needed for malloc_stats */
#include <NEW>
#include "dlmalloc.h"


void * ::operator new(size_t nSize) // throw(std::bad_alloc)
{
    void * p = dlmalloc(nSize);
      if (!p) {
          throw std::bad_alloc();
      }
    return p;
}

void ::operator delete(void * p) {
   dlfree(p);
}
#endif
/*===========================================================================*/
