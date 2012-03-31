/*
* $Id: URI.cpp,v 1.29 2011-07-29 10:26:37 pgr Exp $
*
* Break an URI into its components.
*/

/*---------+
| Includes |
+---------*/
#ifdef __MWERKS_NEEDS_CTYPE_FIX__
#include "../../palmiolib/ctype_fix.h"
#else
#include <ctype.h>
#endif
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "miscutil.h"
#include "URI.h"
#include "MemStream.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

#define JAXO_COM_URI_ALLOW_DOS_FILE_CONVENTION

URI const URI::Nil;
URI::SchemeHandler const URI::SchemeHandler::Nil;

/*-------------------------------------------------------------------URI::URI-+
| Nil constructor                                                             |
+----------------------------------------------------------------------------*/
URI::URI() {
   m_data.scheme = m_data.authority = m_data.userInfo = m_data.host =
   m_data.path = m_data.query = m_data.fragment = (char *)"";
   m_data.user = m_pValue = 0;
   m_data.port = 0;
}

/*-------------------------------------------------------------------URI::URI-+
| Constructor from an asciiz string.                                          |
| A base URI might be specified, as well as a SchemaParserVFT.                |
|                                                                             |
| Param:                                                                      |
|   spec     pointer to an asciiz string that hold the specified URI          |
|   baseURI  URI for deriving relative URI from (known as the Base URI)       |
|   finder   SchemeHandlerFactory to find the SchemeHandler to be used for    |
|            parsing the URI.                                                 |
+----------------------------------------------------------------------------*/
URI::URI(
   char const * const spec,
   URI const & baseURI,
   SchemeHandlerFactory & finder
) {
   #ifdef JAXO_COM_URI_ALLOW_DOS_FILE_CONVENTION
      bool isFakedAbsPath = false;
   #endif
   int i, start, limit;
   char isRelative = 0;
   SchemeHandler handler;
   Components comps;

   memset(&comps, 0, sizeof comps);
   comps.port = -1;

   /* trim spaces - get rid of "url:" */
   for (limit = strlen(spec); limit && (spec[limit-1] <= ' '); --limit) {}
   for (start = 0; (start < limit) && (spec[start] <= ' '); ++start) {}
   if (((limit - start) >= 4) && (0 == strncasecmp(spec+start, "url:", 4))) {
      start += 4;
   }

   /*
   | Scheme:  $1 in ^(([^:/?#]+):)?  (RFC2396, Appendix B)
   */
   for (i=start; i < limit; ++i) {
      switch (spec[i]) {
      case '/':
      case '#':
      case '?':
         break;
      case ':':
         if (i > start) {
            #ifdef JAXO_COM_URI_ALLOW_DOS_FILE_CONVENTION
            /*
            | TEMP FIX.  "d:/u/pgr" is BAD, and should have been: "/d:/u/pgr"
            | For nowtimes, be tolerant.
            */
            if (i == start+1) {
               isFakedAbsPath = true;
               break;
            }
            #endif
            comps.scheme.p = spec+start;
            comps.scheme.len = i - start;
            if (!isValidScheme(comps.scheme)) {
               throw MalformedException(INVALID_SCHEME);
            }
            start = i + 1;
         }
         break;
      default:
         continue;
      }
      break;
   }

   /*
   | Find out if the spec URI is relative.  This should just be:
   | "if ((comps.scheme.p == 0) && (baseURI.m_handler.isPresent())",
   | however, if the scheme of the spec URI matches the scheme of the
   | base URI, and the base URI is a hierarchical URI scheme,
   | then maintain backwards compatibility and treat it as if
   | the spec didn't contain the scheme; see 5.2.3 of RFC2396
   */
   if (
      (baseURI.m_handler.isPresent()) &&
      (
         (comps.scheme.p == 0) ||
         (
            (comps.scheme.len == (int)strlen(baseURI.m_data.scheme)) &&
            (0 == strncasecmp(
               comps.scheme.p, baseURI.m_data.scheme, comps.scheme.len)
            ) &&
            (baseURI.m_data.path) && (strlen(baseURI.m_data.path) > 0) &&
            (baseURI.m_data.path[0] == '/')
         )
      )
   ) {
      comps.scheme.p = baseURI.m_data.scheme;
      comps.scheme.len = strlen(baseURI.m_data.scheme);
      comps.authority.p = baseURI.m_data.authority;
      comps.authority.len = (comps.authority.p)? strlen(baseURI.m_data.authority) : 0;
      comps.userInfo.p = baseURI.m_data.userInfo;
      comps.userInfo.len = (comps.userInfo.p)? strlen(baseURI.m_data.userInfo) : 0;
      comps.host.p = baseURI.m_data.host;
      comps.host.len = (comps.host.p)? strlen(baseURI.m_data.host) : 0;
      comps.port = baseURI.m_data.port;
      comps.path.p = baseURI.m_data.path;
      comps.path.len = strlen(baseURI.m_data.path);
      isRelative = 1;
   }

   /* Find an appropriate SchemeHandlerRep */
   if (comps.scheme.len == 0) {
      throw MalformedException(NO_SCHEME);
   }else {
      handler = finder.findHandler(comps.scheme.p, comps.scheme.len);
      if (!handler.isPresent()) {
         throw MalformedException(UNKNOWN_SCHEME);
      }
   }

   /* Strip off the fragment */
   for (i=start; i < limit; ++i) {
      if (spec[i] == '#') {
         comps.fragment.p = spec+i+1;
         comps.fragment.len = limit - (i+1);
         limit = i;
         break;
      }
   }

   if (start == limit) {
      /*
      | The path is empty, and the authority and query are undefined.
      | If the scheme is also undefined, the RFC2396 at 5.2.2 implies
      | that the query and fragment are inheritated from the baseURI
      | if undefined.
      */
      if (isRelative) {
         comps.query.p = baseURI.m_data.query;
         comps.query.len = (comps.query.p)? strlen(baseURI.m_data.query) : 0;
         if (comps.fragment.p == 0) {
            comps.fragment.p = baseURI.m_data.fragment;
            comps.fragment.len = (comps.fragment.p)? strlen(baseURI.m_data.fragment) : 0;
         }
      }else {
         comps.path.p = "";
         comps.path.len = 0;
      }
      finish(comps, handler);
      return;
   }

   if ((!isRelative) && (spec[start] != '/') && (spec[start] != '?')) {
      /*
      | The path is in an opaque part - no query part in here
      */
      comps.path.p = spec + start;
      comps.path.len = limit-start;
      finish(comps, handler);
      return;
   }

   /*
   | Given the logic above, this part is hit when spec is one of:
   | - a relative URI (no scheme) starting with '/' or '?' or something else.
   | - an absolute URI with '/' or '?' following the scheme
   | Strip off the query part
   */
   for (i = start; i < limit; ++i) {
      if (spec[i] == '?') {
         comps.query.p = spec + i+1;
         comps.query.len = limit - (i+1);
         limit = i;
         break;
      }
   }

   if ((limit == start) || (spec[start] != '/')) {
      /*
      | Not a slash.
      | The spec is either an absolute URL with no path (just a query),
      | or the spec is a relative URI.
      | Take care of the former.
      | Then resolve a relative-path according to RFC2396, 5.2.6
      | Try to remove the last segment from the Base URI path
      | and recompose as in RFC2396, 5.2.6a and 5.2.6b.
      | If the Base URI path has only one segment (no '/'),
      | a leading slash is needed to end the authority.
      | If the Base URI has no authority, then it is useless.
      */
      if (!isRelative) {
         comps.path.p = "";
         comps.path.len = 0;
         finish(comps, handler);
         return;
      }else {
         char * buffer = 0;
         int i = comps.path.len;                    // inherited path
         int len;

         #ifdef JAXO_COM_URI_ALLOW_DOS_FILE_CONVENTION
         if (!isFakedAbsPath) {
         #endif

         while ((--i >= 0) && (comps.path.p[i] != '/')) {}
         if (i >= 0) {
            len = (i+1) + (limit-start);
            buffer = (char *)malloc(len+1);
            memcpy(buffer, comps.path.p, i+1);
            memcpy(buffer+i+1, spec+start, limit-start);
         }else {                                    // no slash found
            if (comps.authority.p != 0) {           // inherited authority
               len = 1 + (limit-start);
               buffer = (char *)malloc(len+1);
               buffer[0] = '/';                     // insert a slash.
               memcpy(buffer+1, spec+start, limit-start);
            }else {
               /* no slash and no (inherited) authority (opaque?) */
               comps.path.p = spec+start;
               comps.path.len = limit-start;
               finish(comps, handler);
               return;
            }
         }

         #ifdef JAXO_COM_URI_ALLOW_DOS_FILE_CONVENTION
         }else {
            len = 1 + (limit-start);
            buffer = (char *)malloc(len+1);
            buffer[0] = '/';                     // insert a slash.
            memcpy(buffer+1, spec+start, limit-start);
         }
         #endif

         buffer[len] = '\0';                        // make an asciiz string
         collapsePath(buffer);
         comps.path.p = buffer;
         comps.path.len = strlen(buffer);
         finish(comps, handler);
         free(buffer);
         return;
      }
   }

   /*
   | Given the logic above, this part is hit when spec is one of:
   | - a relative URI (no scheme) starting with '/',
   | - an absolute URI with '/' following the scheme
   | It is an absolute path.
   | Parse the authority part if any
   */
   if ((start < (limit-1)) && (spec[start+1]=='/')) {
      int atSignPos = -1;
      start += 2;
      for (i = start; i < limit; ++i) {
         if (spec[i] == '/') break;
         if ((spec[i] == '@')&&(atSignPos == -1)) atSignPos = i;
      }
      comps.authority.p = spec + start;
      comps.authority.len = i - start;
      if (atSignPos != -1) {
         comps.userInfo.p = comps.authority.p;
         comps.userInfo.len = atSignPos - start;
         comps.host.p = spec + atSignPos + 1;
         comps.host.len = i - (atSignPos + 1);
      }else {
         comps.host = comps.authority;
      }
      start = i;

      /* I should check that the comps.host.len is not zero. */

      /* Strip off the port, if any */
      comps.port = -1;  /* default value */
      if (comps.host.len > 0) {
         i = 0;
         if (comps.host.p[0] == '[') {
            /*
            | If the host is surrounded by [ and ]
            | then it is an IPv6 literal address as specified in RFC2732.
            */
            for (;;) {
               if (++i >= comps.host.len) {
                  throw MalformedException(INVALID_IPV6_REF);
               }
               if (comps.host.p[i] == ']') {
                  if ((++i == comps.host.len) || (comps.host.p[i] == ':')) {
                     break;
                  }else {
                     i = comps.host.len;  // force an error
                  }
               }
            }
         }else {
            while ((comps.host.p[i] != ':') && (++i < comps.host.len)) {}
         }
         if (i < comps.host.len) {        // a colon was found
            /* port can be null according to RFC2396 */
            if ((i+1) < comps.host.len) {
               comps.port = parseInt(comps.host.p,  i+1,  comps.host.len);
               if (comps.port == -1) {
                  throw MalformedException(INVALID_PORT_NO);
               }
            }
            comps.host.len = i;
         }
      }
   }
   comps.path.p = spec + start;
   comps.path.len = limit - start;
   finish(comps, handler);
}

/*----------------------------------------------------------------URI::finish-+
| Allocate the URI m_value to safe copy the components of the Components.     |
|                                                                             |
| Param:                                                                      |
|   comps    Components that hold the component coordinates                   |
|   handler  SchemeHandler to be used for parsing the URI.                    |
+----------------------------------------------------------------------------*/
void URI::finish(
   Components & comps,
   SchemeHandler & handler
) {
   m_pValue = new char[
      comps.scheme.len + comps.host.len + comps.query.len +
      comps.authority.len + comps.path.len + comps.userInfo.len +
      comps.fragment.len + 7
   ];
   m_data.scheme = m_pValue;
   m_data.host = catenate(&m_data.scheme, comps.scheme);
   m_data.query = catenate(&m_data.host, comps.host);
   m_data.authority = catenate(&m_data.query, comps.query);
   m_data.path = catenate(&m_data.authority, comps.authority);
   m_data.userInfo = catenate(&m_data.path, comps.path);
   m_data.fragment = catenate(&m_data.userInfo, comps.userInfo);
   catenate(&m_data.fragment, comps.fragment);
   m_data.port = comps.port;
   m_handler = handler;
   m_data.user = 0;
   try {
      handler->onNew(m_data);
   }catch (MalformedException) {
      handler = SchemeHandler::Nil;
      delete [] m_pValue;
      m_pValue = 0;
      throw;
   }
}

/*-------------------------------------------------------------------URI::URI-+
| Copy constructor                                                            |
+----------------------------------------------------------------------------*/
URI::URI(URI const & source) {
   if (source.m_handler.isPresent()) {
      MemStream temp;
      source.m_handler->toStream(source.m_data, temp);
      temp << '\0';
      char const * spec = temp.str();
      char const * a1 = spec;
      URI a2 = URI::Nil;
      DefaultSchemeHandlerFactory factory(source.m_handler);
      new(this) URI(spec, URI::Nil, factory);
//    new(this) URI(
//       spec, URI::Nil, DefaultSchemeHandlerFactory(source.m_handler)
//    );
      temp.rdbuf()->freeze(0);
   }else {
      new(this) URI;
   }
}

/*------------------------------------------------------------------operator=-+
| Assignment op                                                               |
+----------------------------------------------------------------------------*/
URI & URI::operator=(URI const & source) {
   if (this != &source) {
      this->~URI();
      new(this) URI(source);
   }
   return *this;
}

/*------------------------------------------------------------------URI::~URI-+
| Destructor                                                                  |
+----------------------------------------------------------------------------*/
URI::~URI() {
   if (m_handler.isPresent()) m_handler->onDelete(m_data);
   if (m_pValue) delete [] m_pValue;
}

/*STATIC----------------------------------------------------URI::collapsePath-+
| Utility function to remove "../" and "./" from the path.                    |
+----------------------------------------------------------------------------*/
void URI::collapsePath(char * buffer)
{
   char * found = buffer;

   /* Remove embedded /./ */
   for (found = buffer; 0 != (found = strstr(found, "/./")); ) {
      memcpy(found, found+2, 1+strlen(found+2));
   }
   /* Remove embedded /../ */
   for (found = buffer; 0 != (found = strstr(found, "/../")); ) {
      char * prev;
      for (prev = found-1; ; --prev) {
         if (prev < buffer) {  // do not process...
            ++found;
            break;
         }
         /* be careful: "/../../" means nothing. */
         if ((*prev == '/') && (prev[1] != '.')) {
            memcpy(prev, found+3, 1+strlen(found+3));
            found = prev;
            break;
         }
      }
   }

   /* Remove a possible trailing .. or . */
   found = buffer + strlen(buffer) - 3;
   if ((found >= buffer) && (0 == strcmp(found, "/.."))) {
      char * prev;
      for (prev = found-1; prev >= buffer; --prev) {
         if (*prev == '/') {
            *(prev+1) = 0;
            break;
         }
      }
   }else if ((++found >= buffer) && (0 == strcmp(found, "/."))) {
      *(found+1) = '\0';
   }
}

/*STATIC--------------------------------------------------------URI::catenate-+
| Utility function to build the value string hidden inside the URI.           |
+----------------------------------------------------------------------------*/
char * URI::catenate(char ** target, Component const source) {
   if (source.p == 0) {
      char * temp = *target;
      *target =0;
      return temp;
   }else {
      memcpy(*target, source.p, source.len);
      (*target)[source.len] = '\0';
      return (*target)+source.len+1;
   }
}

/*STATIC---------------------------------------------------URI::isValidScheme-+
| Utility function to validate the syntax of a scheme.                        |
+----------------------------------------------------------------------------*/
char URI::isValidScheme(Component scheme)
{
   if (isalpha(scheme.p[0])) {
      int len = scheme.len;
      int i;
      for (i=1; i < len; ++i) {
         char c = scheme.p[i];
         if (!isalnum(c) && (c != '.') && (c != '+') && (c != '-')) return 0;
      }
      return 1;
   }
   return 0;
}

/*STATIC--------------------------------------------------------URI::parseInt-+
| Utility function to parse an integer.                                       |
+----------------------------------------------------------------------------*/
int URI::parseInt(char const * value, int posMin, int posMax)
{
   if (posMin == posMax) {
      return -1;
   }else {
      int pos;
      unsigned long val = 0;
      for (pos=posMin; pos < posMax; ++pos) {
         if (!isdigit(value[pos])) {
            return -1;
         }
         val = (val*10) + (value[pos] - '0');
         if (val > 0xFFFF) {
            return -1;
         }
      }
      return (int)val;
   }
}

/*-----------------------------------------------------------------operator<<-+
| Output the absolute URI to a stream                                         |
+----------------------------------------------------------------------------*/
ostream & operator<<(ostream & out, URI const & uri) {
   if (!uri.m_handler.isPresent()) {
      out << "(Nil)";
   }else {
      uri.m_handler->toStream(uri.m_data, out);
   }
   return out;
}

/*-------------------------------------URI::SchemeHandler::Rep::matchesScheme-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool URI::SchemeHandler::Rep::matchesScheme(
   char const * scheme,
   int len
) const {
   return (
      (len == (int)strlen(getID())) &&
      (0 == strncasecmp(scheme, getID(), len))
   );
}

/*---------------------------------------------URI::SchemeHandler::Rep::onNew-+
|                                                                             |
+----------------------------------------------------------------------------*/
void URI::SchemeHandler::Rep::onNew(Data & uri)
{
}

/*------------------------------------------URI::SchemeHandler::Rep::onDelete-+
|                                                                             |
+----------------------------------------------------------------------------*/
void URI::SchemeHandler::Rep::onDelete(Data & uri)
{
}

/*------------------------------------------URI::SchemeHandler::Rep::toStream-+
|                                                                             |
+----------------------------------------------------------------------------*/
void URI::SchemeHandler::Rep::toStream(Data const & uri, ostream & out) const
{
   out << uri.scheme << ':';
   if (uri.authority) out << "//" << uri.authority;
   out << uri.path;
   if (uri.query) out << '?' << uri.query;
   if (uri.fragment) out << "#" << uri.fragment;
}

#ifdef TOOLS_NAMESPACE
}
#endif
/*===========================================================================*/
