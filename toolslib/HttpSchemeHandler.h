/*
* $Id: HttpSchemeHandler.h,v 1.8 2002-06-18 10:36:31 pgr Exp $
*/

#if ! defined COM_JAXO_HTTPSCHEMEHANDLER_H_INCLUDED
#define COM_JAXO_HTTPSCHEMEHANDLER_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include "URI.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*-------------------------------------------------- class HttpSchemeHandler -+
|                                                                             |
+----------------------------------------------------------------------------*/
class TOOLS_API HttpSchemeHandler : public URI::SchemeHandler {
public:
   HttpSchemeHandler();

private:
   class Rep : public URI::SchemeHandler::Rep {
      char const * getID() const;
      iostream * makeStream(URI const & uri, ios__openmode om);
      void onNew(URI::Data & uri);
   };
};

/* -- INLINES -- */
inline HttpSchemeHandler::HttpSchemeHandler() : SchemeHandler(new Rep) {
}

#ifdef TOOLS_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
