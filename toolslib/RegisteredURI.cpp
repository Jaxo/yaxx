/*
* $Id: RegisteredURI.cpp,v 1.1 2003-01-11 11:57:15 pgr Exp $
*
* URI with statically known Schemes
*/

/*---------+
| Includes |
+---------*/
#include "RegisteredURI.h"
#include <string.h>
#include "miscutil.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

RegisteredURI::SchemeHandlerFactory::List RegisteredURI::SchemeHandlerFactory::m_schemes;
RegisteredURI::SchemeHandlerFactory RegisteredURI::m_factory;

/*STATIC------------------RegisteredURI::SchemeHandlerFactory::registerScheme-+
|                                                                             |
+----------------------------------------------------------------------------*/
void RegisteredURI::SchemeHandlerFactory::registerScheme(
   SchemeHandler newHandler
) {
   char const * id = newHandler->getID();
   int max = m_schemes.count();
   for (int ix=0; ix < max; ++ix) {
      SchemeHandler curHandler = m_schemes[ix];
      if (!strcasecmp(curHandler->getID(), id)) {
         m_schemes.set(ix, newHandler);
         return;
      }
   }
   m_schemes.add(newHandler);
}

/*---------------------------RegisteredURI::SchemeHandlerFactory::findHandler-+
| Get the scheme handler that is appropriate for a given scheme               |
|                                                                             |
| Returns: a pointer to the appropriate scheme parser, or Nil if none.        |
+----------------------------------------------------------------------------*/
URI::SchemeHandler RegisteredURI::SchemeHandlerFactory::findHandler(
   char const * scheme,
   int len
) {
   int max = m_schemes.count();
   for (int ix=0; ix < max; ++ix) {
      SchemeHandler curHandler = m_schemes[ix];
      if (curHandler->matchesScheme(scheme, len)) return curHandler;
   }
   return URI::SchemeHandler::Nil;
}

/*---------------------------------------------------RegisteredURI::operator=-+
| Assignment op                                                               |
+----------------------------------------------------------------------------*/
RegisteredURI & RegisteredURI::operator=(char const * pszUri) {
   this->~RegisteredURI();
   new(this) RegisteredURI(pszUri);
   return *this;
}

/*-----------------------------------------------RegisteredURI::RegisteredURI-+
| Constructor from an asciiz string.                                          |
| A base URI might be specified.                                              |
|                                                                             |
| Param:                                                                      |
|   spec     pointer to an asciiz string that hold the specified URI          |
|   baseURI  URI for deriving relative URI from (known as the Base URI)       |
+----------------------------------------------------------------------------*/
RegisteredURI::RegisteredURI(
   char const * const pszUri,     // asciiz string to build the URI from
   URI const & baseURI            // the base URI - can be URI::Nil
) :
   URI(pszUri, baseURI, m_factory) {
}

#ifdef TOOLS_NAMESPACE
}
#endif
/*===========================================================================*/
