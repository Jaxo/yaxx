/* $Id: Pool.h,v 1.10 2002-02-02 09:10:55 pgr Exp $ */

#ifndef COM_JAXO_YAXX_POOL_H_INCLUDED
#define COM_JAXO_YAXX_POOL_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include "HashMap.h"

#ifdef YAXX_NAMESPACE
namespace YAXX_NAMESPACE {
#endif

/*--------------------------------------------------------------- class Pool -+
|                                                                             |
+----------------------------------------------------------------------------*/
class Pool : public MapItem {
public:
   Pool(RexxString & poolName);
   virtual RexxString get(RexxString & name) const = 0;
   virtual bool set(RexxString & name, RexxString & value) = 0;
};

class SystemPool : public Pool {
public:
   SystemPool();
   RexxString get(RexxString & name) const;
   bool set(RexxString & name, RexxString & value);
private:
   static RexxString poolName;
};

#ifdef YAXX_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
