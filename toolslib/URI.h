/*
* $Id: URI.h,v 1.38 2011-07-29 10:26:37 pgr Exp $
*
* Break an URI (Uniform Resource Identifier) into its components
*
* This class is conform to RFC 2396, (http://www.faqs.org/rfcs/rfc2396.html)
* that replaced RFC 1738 and RFC 1808.
*
* Example 1:
* ~~~~~~~~~~
*
*   #include "HttpSchemeHandler.h"
*   #include "URI.h";
*   int main(int argc, char **argv)
*   {
*      try {
*         HTTPSchemeHandler httpHandler;
*         URI::DefaultSchemeHandlerFactory factory(httpHandler);
*         URI baseURI("http://www.jaxo.com:8080", factory);
*         URI anURI("foobar?what#where", baseURI, factory);
*         cout << anURI;
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
*
* Example 2:  (no factory)
* ~~~~~~~~~~
*   #include "URI.h";
*   int main(int argc, char **argv)
*   {
*      try {
*         cout << FreeURI("http://www.jaxo.com:8080");
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
*   Path:      (null)
*   Query:     (null)
*   Fragment:  (null)
*   Handler:   HTTP
*/

#if ! defined COM_JAXO_TOOLS_URI_H_INCLUDED
#define COM_JAXO_TOOLS_URI_H_INCLUDED

/*---------+
| Includes |
+---------*/
#if defined __MWERKS__
#include "ios.h"
#include "iostream.h"  //JAL (said) Have to use "" instead of <> for MWERKS
#else
#include <iostream>
#endif

#include <new>
#include "RefdItem.h"
#include "toolsgendef.h"
#include "migstream.h"


#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*---------------------------------------------------------------- class URI -+
|                                                                             |
+----------------------------------------------------------------------------*/
class TOOLS_API URI {
   friend TOOLS_API ostream & operator<<(ostream& out, URI const & uri);
public:
   class SchemeHandler;

   enum ExceptionCode {
      INVALID_SCHEME,                // Invalid scheme
      NO_SCHEME,                     // Unable to guess a scheme
      UNKNOWN_SCHEME,                // Unknown scheme
      INVALID_IPV6_REF,              // Invalid IPV6 reference
      INVALID_PORT_NO,               // Invalid port number
      HANDLER_ERROR                  // Handler did not recognize the syntax
   };

   /*-------------------------+
   | MalformedException Class |
   +-------------------------*/
   class MalformedException {  // thrown by the URI constructor
   public:
      MalformedException(enum ExceptionCode code, unsigned char subCode = 0);
      enum ExceptionCode getCode() const;
      unsigned char getSubCode() const;
   private:
      enum ExceptionCode m_code;
      unsigned char m_subCode;
   };

   /*--------------------+
   | URI::Data structure |
   +--------------------*/
   struct Data {            // for SchemeHandler Internal Use only!
      char * scheme;
      char * authority;
      char * userInfo;
      char * host;
      int port;
      char * path;
      char * query;
      char * fragment;
      void * user;          // user-defined value (set to 0)
   };

   class TOOLS_API SchemeHandler : public RefdItem {
   public:
      static SchemeHandler const Nil;

      class TOOLS_API Rep : public RefdItemRep {
      public:

         /*
         | Identifies this SchemeHandler.
         | The identifier is the key of all registered handler.
         | E.g.: when registering a new SchemeHandler, if a SchemeHandler is
         | already registered under the same ID, the new one replaces the old
         | one.
         |
         | For most handler, the identifier is the scheme, so the default
         | implementation of matchesScheme (see below) is to compare the
         | scheme of the URI with SchemeHandler.getId()  However, overloading
         | matchScheme, a SchemeHandler can identify as 'file' and match the
         | scheme 'palmfile'.
         |
         | Returns: a unique identifier among SchemeHandlers
         */
         virtual char const * getID() const = 0;

         /*
         | Get a stream attached to this handler.
         */
         virtual iostream * makeStream(URI const & uri, ios__openmode om) = 0;

         /*
         | Given a scheme, returns 1 (true) if this SchemeHandler
         | is responsible for this scheme, false otherwise.
         | The default action is to compare (ignoring case) with getID().
         */
         virtual bool matchesScheme(char const * scheme, int len) const;

         /*
         | virtual void onNew(URI & uri);
         |
         | Post-parse the basic components of an URI.  This routine might
         | review each component and make changes to it, or simply invalidate
         | the URI.
         |
         | To invalidate the URI requires to throw an URI::MalformedException.
         |
         | Making changes can be done in several ways.
         |
         | 1) modify the URI, setting the "user" member.
         | The URI has a field: "void * user;" which is reserved for the
         | use of the SchemeHandler.  This field is initially set to 0.
         | A particular SchemeHandler implementation can then add
         | complementary informations using the "user" field as an anchor.
         | It might then be necessary to free the memory used by this extra
         | data: this should be accomplished when the routine "onDelete()"
         | is called (see below.)
         |
         | 2) total reconstruction of the URI
         | A SchemeHandler implementation may build another URI from the
         | original one.  In order to do so, it should first delete the URI
         | (using this->~URI) but be careful! onDelete will then be called.
         | The "user" field provides the means to tell "onDelete()" if the
         | deleted URI is the original one, or the one that was rebuilt.
         |
         | 3) modify the members in place.
         | For experienced users, members of the URI can be modified,
         | but this requires to understand the internals of this structure.
         | Each "char *" members of a URI points inside a unique buffer
         | (pointed by m_pValue) that will be automatically freed by "~URI()".
         |
         |      scheme--+    authority--+      userInfo--+
         |              |               |                |
         |              V               V                V
         | buffer--->  |ssssssssssssss0|aaaaaaaaaaaaaaa0|iiiiiii0|......
         |
         | The buffer address cannot be changed.  However, it is possible to
         | make simple changes such as:
         |
         |      scheme--+    authority--+ user--+ userInfo--+
         |              |               |       |           |
         |              V               V       V           V
         | buffer--->  |ssssssssssssss0|bbbbbbb0aaaaaaa0|iiiiiii0|......
         |
         | Therefore, the constraints are:
         |    - the new pointers value must be in the range
         |      [old_value, old_value+strlen(old_value)-1]
         |    - within this range, the value can be modified as needed
         |
         |
         | Param:
         |   uri     Reference to the newly created URI
         |
         */
         virtual void onNew(Data & uri);

         /*
         | Called when an URI is about to be deleted.  This routine might
         | do its own clean-up before the URI so is "regularly" deleted.
         */
         virtual void onDelete(Data & uri);

         /*
         | Output the absolute URI to a stream
         */
         virtual void toStream(Data const & uri, ostream & out) const;
      };

      SchemeHandler();
      Rep const * operator->() const;
      Rep * operator->();

// protected:
      SchemeHandler(RefdItemRep * pRep);
   };

   /*---------------------------+
   | SchemeHandlerFactory Class |
   +---------------------------*/
   class TOOLS_API SchemeHandlerFactory {
   public:
      virtual SchemeHandler findHandler(char const * scheme, int len) = 0;
   };

   /*----------------------------------+
   | DefaultSchemeHandlerFactory Class |
   +----------------------------------*/
   class TOOLS_API DefaultSchemeHandlerFactory : public SchemeHandlerFactory {
   public:
      DefaultSchemeHandlerFactory(SchemeHandler);
   private:
      SchemeHandler m_handler;
      SchemeHandler findHandler(char const * scheme, int len);
   };

   /*--------------------+
   | URI Public Methods  |
   +--------------------*/
   URI();
   URI(                              // Constructor
      char const * const pszUri,     // asciiz string to build the URI from
      URI const & baseURI,           // the base URI - can be URI::Nil
      SchemeHandlerFactory & finder  // to find the scheme handler
   );
   URI(                              // Constructor
      char const * const pszUri,     // asciiz string to build the URI
      SchemeHandlerFactory & finder  // to find the scheme handler
   );
   URI & operator=(URI const & source);
   URI(URI const & source);
   ~URI();

   bool isValid() const;
   char const * getScheme() const;
   char const * getAuthority() const;
   char const * getUserInfo() const;
   char const * getHost() const;
   int getPort() const;
   // char const * getFile const();;
   char const * getPath() const;
   char const * getQuery() const;
   char const * getFragment() const;
   void const * getUser() const;
   iostream * getStream(ios__openmode om);

   SchemeHandler const & getHandler() const;  // Debug
   static void collapsePath(char * buffer);   // collapse "../" and "./"

protected:
   static URI const Nil;

private:
   friend class SchemeHandler::Rep;

   struct Component {
      char const * p;
      int len;
   };

   struct Components {
      Component scheme;
      Component authority;
      Component userInfo;
      Component host;
      int port;
      Component path;
      Component query;
      Component fragment;
   };

   Data m_data;
   SchemeHandler m_handler;
   char * m_pValue;

   void finish(Components & comps, SchemeHandler & handler);
   static char * catenate(char ** target, Component const source);
   static char isValidScheme(Component scheme);
   static int parseInt(char const * value, int posMin, int posMax);
};

TOOLS_API ostream & operator<<(ostream & out, URI const & ucs);


/* -- INLINES -- */
inline URI::URI(char const * const pszUri, SchemeHandlerFactory & finder) {
   new (this) URI(pszUri, URI::Nil, finder);
}
inline bool URI::isValid() const {
   return m_handler.isPresent();
}
inline char const * URI::getScheme() const {
   return m_data.scheme;
}
inline char const * URI::getAuthority() const {
   return m_data.authority;
}
inline char const * URI::getUserInfo() const {
   return m_data.userInfo;
}
inline char const * URI::getHost() const {
   return m_data.host;
}
inline int URI::getPort() const {
   return m_data.port;
}
// inline char const * URI::getFile() const {
//    return (m_data.query == 0)? m_data.path : m_data.path + "?" + m_data.query;) */
// }
inline char const * URI::getPath() const {
   return m_data.path;
}
inline char const * URI::getQuery() const {
   return m_data.query;
}
inline char const * URI::getFragment() const {
   return m_data.fragment;
}
inline void const * URI::getUser() const {
   return m_data.user;
}
inline iostream * URI::getStream(ios__openmode om) {
   return m_handler.isPresent()? m_handler->makeStream(*this, om) : 0;
}
inline URI::SchemeHandler const & URI::getHandler() const {
   return m_handler;
}
inline URI::SchemeHandler::SchemeHandler() {
}
inline URI::SchemeHandler::SchemeHandler(RefdItemRep * pRep) :
   RefdItem(pRep)
{}
inline URI::MalformedException::MalformedException(
   URI::ExceptionCode code,
   unsigned char subCode
) {
   m_code = code;
   m_subCode = subCode;
}
inline URI::SchemeHandler::Rep * URI::SchemeHandler::operator->() {
   return (URI::SchemeHandler::Rep *)inqData();
};
inline URI::SchemeHandler::Rep const * URI::SchemeHandler::operator->() const {
   return (URI::SchemeHandler::Rep const *)inqData();
};
inline URI::DefaultSchemeHandlerFactory::DefaultSchemeHandlerFactory(
   URI::SchemeHandler handler
) : m_handler(handler) {
}
inline URI::SchemeHandler URI::DefaultSchemeHandlerFactory::findHandler(
   char const * scheme, int len
) {
   if (m_handler->matchesScheme(scheme, len)) {
      return m_handler;
   }else {
      return URI::SchemeHandler::Nil;
   }
}
inline URI::ExceptionCode URI::MalformedException::getCode() const {
   return m_code;
}
inline unsigned char URI::MalformedException::getSubCode() const {
   return m_subCode;
}

/*------------------------------------------------------------ class FreeURI -+
| A FreeURI does not need a factory and can simply be built from a string.    |
| E.g.: the scheme doesn't need to be known by the system.                    |
|                                                                             |
| Since a FreeURI is not bound to any real resource, the method makeStream()  |
| will always return NULL.  FreURI are intented mainly for the inspection     |
| of URI components.                                                          |
+----------------------------------------------------------------------------*/
class TOOLS_API FreeURI :
   private URI::SchemeHandler,
   private URI::DefaultSchemeHandlerFactory,
   public URI
{
public:
   FreeURI(char const * pszUri, URI const & base = URI());
private:
   class Rep : public URI::SchemeHandler::Rep {
      char const * getID() const;
      iostream * makeStream(URI const & uri, ios__openmode om);
      bool matchesScheme(char const * scheme, int len) const;
   };
};

#if defined _WIN32
#pragma warning(disable:4355)
#endif
inline FreeURI::FreeURI(
   char const * pszUri,
   URI const & base
) :
   URI::SchemeHandler(new Rep),
   URI::DefaultSchemeHandlerFactory((URI::SchemeHandler)(*this)),
   URI(pszUri, base, *this) {
}
#ifdef __WIN32__
#pragma warning(default:4355)
#endif


inline char const * FreeURI::Rep::getID() const {
   return "";
}
inline bool FreeURI::Rep::matchesScheme(char const * scheme, int len) const {
   return true;
}
inline iostream * FreeURI::Rep::makeStream(URI const &, ios__openmode) {
   return 0;
}

#ifdef TOOLS_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
