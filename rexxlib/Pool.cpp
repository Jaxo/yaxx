/* $Id: Pool.cpp,v 1.25 2002-10-24 06:29:14 pgr Exp $ */

#include <stdlib.h>
#include "Pool.h"

#ifdef YAXX_NAMESPACE
namespace YAXX_NAMESPACE {
#endif

RexxString SystemPool::poolName("SYSTEM");

/*-----------------------------------------------------------------Pool::Pool-+
|                                                                             |
+----------------------------------------------------------------------------*/
Pool::Pool(RexxString & name) : MapItem(name) {}

/*-----------------------------------------------------SystemPool::SystemPool-+
|                                                                             |
+----------------------------------------------------------------------------*/
SystemPool::SystemPool() : Pool(poolName) {}

/*------------------------------------------------------------SystemPool::get-+
|                                                                             |
+----------------------------------------------------------------------------*/
RexxString SystemPool::get(RexxString & name) const
{
   char * env = 0;
   #if !defined MWERKS_NEEDS_ADDR_FIX
   env = getenv(name);
   #endif
   if (env) {
      return RexxString(env);
   }else {
      return RexxString::Nil;
   }
}

/*------------------------------------------------------------SystemPool::set-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool SystemPool::set(RexxString & name, RexxString & value)
{
   #if defined MWERKS_NEEDS_ADDR_FIX    // command (environment)
      return false;
   #elif defined(_WIN32)
      StringBuffer cmdBuffer;
      cmdBuffer.append(name, name.length());
      cmdBuffer.append('=');
      cmdBuffer.append(value, value.length());
      cmdBuffer.append('\0');
      return (putenv((char *)((char const *)cmdBuffer)) == 0);
   #else
      return (setenv(name, value, 1) == 0);
   #endif
}

#ifdef YAXX_NAMESPACE
}
#endif
/*===========================================================================*/
