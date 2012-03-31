/*
* $Id: SystemContext.h,v 1.12 2011-08-31 08:07:32 pgr Exp $
*
* System "Globals"
*/

#ifndef COM_JAXO_TOOLS_SYSTEMCONTEXT_H_INCLUDED
#define COM_JAXO_TOOLS_SYSTEMCONTEXT_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include "toolsgendef.h"
#include "RegisteredURI.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*--------------------------------------------------------------SystemContext-+
|                                                                             |
+----------------------------------------------------------------------------*/
class TOOLS_API SystemContext {
   friend class ConsoleSchemeHandler;
public:

   SystemContext(
      #ifdef ANDROID
      int (*systemFunction)(char const *),
      #endif
      char const * baseUri = 0,            // generally: "file://"
      URI::SchemeHandler sh0 = URI::SchemeHandler::Nil,
      URI::SchemeHandler sh1 = URI::SchemeHandler::Nil,
      URI::SchemeHandler sh2 = URI::SchemeHandler::Nil,
      URI::SchemeHandler sh3 = URI::SchemeHandler::Nil,
      URI::SchemeHandler sh4 = URI::SchemeHandler::Nil,
      URI::SchemeHandler sh5 = URI::SchemeHandler::Nil,
      URI::SchemeHandler sh6 = URI::SchemeHandler::Nil,
      URI::SchemeHandler sh7 = URI::SchemeHandler::Nil,
      URI::SchemeHandler sh8 = URI::SchemeHandler::Nil,
      URI::SchemeHandler sh9 = URI::SchemeHandler::Nil
   );
   ~SystemContext();

   //>>>PGR: not fully sure if I need these as statics.
   static iostream * makeStream(char const * path, ios__openmode om);
   static URI const & getBaseUri();
   static iostream & cin();
   static iostream & cout();
   static iostream & cerr();
   #ifdef ANDROID
   static int system(char const * command);
   #endif

private:
   static URI m_baseUri;
   static iostream * m_pCin;
   static iostream * m_pCout;
   static iostream * m_pCerr;
   static void validateConsoles();
   static void invalidateConsoles();

   #ifdef ANDROID
   static int (* delegatedSystemFunction)(char const * command);
   #endif
};

/* -- INLINES -- */
inline iostream & SystemContext::cin() {
   if (!m_pCin) validateConsoles();
   return *m_pCin;
}
inline iostream & SystemContext::cout() {
   if (!m_pCout) validateConsoles();
   return *m_pCout;
}
inline iostream & SystemContext::cerr() {
   if (!m_pCerr) validateConsoles();
   return *m_pCerr;
}

inline iostream * SystemContext::makeStream(
   char const * path, ios__openmode om
) {
   return RegisteredURI(path, m_baseUri).getStream(om);
}

inline URI const & SystemContext::getBaseUri() {
   return m_baseUri;
}

#ifdef TOOLS_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
