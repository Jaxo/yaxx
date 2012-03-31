/* $Id: RefdKey.cpp,v 1.1 2002-02-18 15:55:20 pgr Exp $ */

/*--------------+
| Include files |
+--------------*/
#include "RefdKey.h"
#include "ucstring.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

RefdKey const RefdKey::Nil;

#if defined OS__AIX
//Don't pay attention to the following: stupid bug of IBM xlc compiler
static abracadabra1() { return 123; }
#endif /* OS__AIX */

#ifdef TOOLS_NAMESPACE
}
#endif
/*===========================================================================*/
