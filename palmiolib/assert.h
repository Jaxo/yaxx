/* $Id: assert.h,v 1.4 2002-08-06 10:07:46 pgr Exp $ */

#include <assert.h>


#ifdef NDEBUG
   #define SHOW_ERROR(ignore) ((void) 0)
#else
   #define SHOW_ERROR(err) showError((err), __FILE__, __LINE__)
#endif

void showError(int err, char const * file, int line);
/*==========================================================================*/
