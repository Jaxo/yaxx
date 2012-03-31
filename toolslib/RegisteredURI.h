/*
* $Id: RegisteredURI.h,v 1.2 2003-01-21 15:47:59 pgr Exp $
*
* Break an URI (Uniform Resource Identifier) into its components
*
* This class is conform to RFC 2396, (http://www.faqs.org/rfcs/rfc2396.html)
* that replaced RFC 1738 and RFC 1808.
*
* Example:
* ~~~~~~~~
*
*   #include "RegisteredURI.h";
*   int main(int argc, char **argv)
*   {
*      try {
*         RegisteredURI baseURI("http://www.jaxo.com:8080");
*         RegisteredURI anURI("foobar?what#where", baseURI);
*         showURI(anURI);
*      }catch (URI::MalformedException e) {
*         cerr << "Malformed URI!"  << endl;
*      }
*      return 0;
*   }
*
* should produce:
*   Scheme:    http
*   Authority: www.jaxo.com:8080
*   UserInfo:  (null)
*   Host:      www.jaxo.com
*   Port:      8080
*   Path:      /foobar
*   Query:     what
*   Fragment:  where
*   Handler:   HTTP
*
* assuming that an HTTP SchemeHandler had been registered...
*
* SchemeHandler
* ~~~~~~~~~~~~~
*
* Schemes -- aka protocols in the obsoleted RFC 1738 terminology -- is the
* most important part of an URI.  It may add syntax rules, and, without it,
* the URI is not guaranteed to be valid.
*
* Therefore, when parsing an URI, a set of syntax rules common to all URI
* is first applied.  Then, a corresponding SchemeHandler is called in
* order to post-parse the URI and ensures its validity wrt its scheme.
*
* This is the purpose of the SchemeHandler class.  For each scheme
* (such as HTTP, FILE, FTP, etc...), the URI constructor will call the
* corresponding SchemeHandler.  In most frequent cases, the selection of
* a SchemeHandler is based on the Scheme name, specific to this URI.
*
* There is also a way to specify (in the URI constructor argument)
* a particular StreamHanlder, independently of the Scheme name
* (which is not recommended for most uses.)
*
* SchemesHandler are "global static classes" (aka Functors.)
* They must be created and registered in the main thread and will later
* be shared by all threads.  SchemeHandlers should then stay const,
* even if this practice is not inforced by the function prototypes.
*
* Registering a SchemeHandler will replace any existing one for the
* same scheme.
*
* The call to register a new SchemeHandler is:
* RegisteredURI::registerScheme(MySchemeHandler());
*
*/

#if ! defined COM_JAXO_TOOLS_REGISTEREDURI_H_INCLUDED
#define COM_JAXO_TOOLS_REGISTEREDURI_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include "URI.h"
#include "tplist.h"


#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*------------------------------------------------------ class RegisteredURI -+
|                                                                             |
+----------------------------------------------------------------------------*/
class TOOLS_API RegisteredURI : public URI
{
public:
   RegisteredURI();
   RegisteredURI(                    // Constructor
      char const * const pszUri,     // asciiz string to build the URI from
      URI const & baseURI = Nil      // the base URI - can be URI::Nil
   );
   RegisteredURI & operator=(char const * pszUri);

   static void registerScheme(SchemeHandler handler);

private:
   class TOOLS_API SchemeHandlerFactory : public URI::SchemeHandlerFactory {
   public:
      static void registerScheme(SchemeHandler handler);
      SchemeHandler findHandler(char const * scheme, int len);
      class TOOLS_API List : public TpList {
      public:
         SchemeHandler operator[](int ix);
         void add(SchemeHandler & itm);
         void set(int ix, SchemeHandler & itm);
      };
      static List m_schemes;         // list of registered scheme handlers
      friend class List;
   };
   static SchemeHandlerFactory m_factory;
};

/* -- INLINES -- */
inline URI::SchemeHandler RegisteredURI::SchemeHandlerFactory::List::operator[](
   int ix
) {
   return (URI::SchemeHandler::Rep *)findDataAt(ix);
}
inline void RegisteredURI::SchemeHandlerFactory::List::add(
   URI::SchemeHandler & itm
) {
   insertLast((RefdItemRep *)(itm.inqData()));
}
inline void RegisteredURI::SchemeHandlerFactory::List::set(
   int ix,
   URI::SchemeHandler & itm
) {
   replaceAt((RefdItemRep *)(itm.inqData()), ix);
}
inline RegisteredURI::RegisteredURI() : URI() {
}
inline void RegisteredURI::registerScheme(SchemeHandler handler) {
   m_factory.registerScheme(handler);
}

#ifdef TOOLS_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
