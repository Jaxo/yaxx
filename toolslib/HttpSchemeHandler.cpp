/*
* $Id: HttpSchemeHandler.cpp,v 1.6 2002-06-18 10:36:31 pgr Exp $
*
* A SchemeHandler for the HTTP protocol
*
*/

#include <string.h>
#include "HttpSchemeHandler.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*----------------------------------------------HttpSchemeHandler::Rep::getID-+
|                                                                             |
+----------------------------------------------------------------------------*/
char const * HttpSchemeHandler::Rep::getID() const {
   return "HTTP";
}

/*-----------------------------------------HttpSchemeHandler::Rep::makeStream-+
|                                                                             |
+----------------------------------------------------------------------------*/
iostream * HttpSchemeHandler::Rep::makeStream(
   URI const & uri,
   ios__openmode om
) {
   return 0;        // NYI
}

/*----------------------------------------------HttpSchemeHandler::Rep::onNew-+
|                                                                             |
+----------------------------------------------------------------------------*/
void HttpSchemeHandler::Rep::onNew(URI::Data & uri) {
   if (uri.port == -1) uri.port = 80;         // set the default port to 80
}

#ifdef TOOLS_NAMESPACE
}
#endif
/*===========================================================================*/
