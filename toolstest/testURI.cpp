/*
* $Id: testURI.cpp,v 1.12 2003-01-11 11:31:08 pgr Exp $
*/

#include <iostream.h>
#include <fstream.h>
#include <string.h>
#include <memory.h>
#include "../toolslib/RegisteredURI.h"
#include "../toolslib/HttpSchemeHandler.h"
#include "../toolslib/StdFileStream.h"


/*----------------------------------------------- class DefaultSchemeHandler -+
|                                                                             |
+----------------------------------------------------------------------------*/
class DefaultSchemeHandler : public URI::SchemeHandler {
public:
   DefaultSchemeHandler();
private:
   class Rep : public URI::SchemeHandler::Rep {
      char const * getID() const;
      iostream * makeStream(URI const & uri, ios__openmode om);
      bool matchesScheme(char const * scheme, int len) const;
   };
};

/*---------------------------------DefaultSchemeHandler::DefaultSchemeHandler-+
|                                                                             |
+----------------------------------------------------------------------------*/
DefaultSchemeHandler::DefaultSchemeHandler() : SchemeHandler(new Rep) {
}

/*-------------------------------------------DefaultSchemeHandler::Rep::getID-+
|                                                                             |
+----------------------------------------------------------------------------*/
char const * DefaultSchemeHandler::Rep::getID() const {
   return "*DEFAULT*";
}

/*--------------------------------------DefaultSchemeHandler::Rep::makeStream-+
|                                                                             |
+----------------------------------------------------------------------------*/
iostream * DefaultSchemeHandler::Rep::makeStream(URI const & uri, ios__openmode om)
{
   return 0;
}

/*-----------------------------------DefaultSchemeHandler::Rep::matchesScheme-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool DefaultSchemeHandler::Rep::matchesScheme(char const * scheme, int len) const {
   return true;
}

/*----------------------------------------------------------------getURIError-+
|                                                                             |
+----------------------------------------------------------------------------*/
static char const * getURIError(URI::MalformedException & e) {
   switch (e.getCode()) {
   case URI::INVALID_SCHEME:    return "Invalid scheme";
   case URI::NO_SCHEME:         return "Unable to guess a scheme";
   case URI::UNKNOWN_SCHEME:    return "Unknown scheme";
   case URI::INVALID_IPV6_REF:  return "Invalid IPV6 reference";
   case URI::INVALID_PORT_NO:   return "Invalid port number";
   case URI::HANDLER_ERROR:     return "Handler did not recognize the syntax";
   default:                     return "????";
   }
}

/*-------------------------------------------------------------printComponent-+
| Format a component of the URI                                               |
+----------------------------------------------------------------------------*/
static void printComponent(char const * title, char const * value)
{
   cout << title;
   if (value == 0) {
      cout << "(undefined)";
   }else {
      cout << '"' << value << '"';
   }
   cout << endl;
}

/*-------------------------------------------------------------------printURI-+
|                                                                             |
+----------------------------------------------------------------------------*/
static void printURI(URI const & uri)
{
   if (!uri.isValid()) {
      cout << "Nil" << endl;
   }else {
      printComponent("Scheme:    ", uri.getScheme());
      printComponent("Authority: ", uri.getAuthority());
      printComponent("UserInfo:  ", uri.getUserInfo());
      printComponent("Host:      ", uri.getHost());
      cout << "Port:      \"";
      if (uri.getPort() < 0) {
         cout << "-1\"" << endl;
      }else {
         cout << uri.getPort() << '\"' << endl;
      }
      printComponent("Path:      ", uri.getPath());
      printComponent("Query:     ", uri.getQuery());
      printComponent("Fragment:  ", uri.getFragment());
      printComponent("SchemeVFT: ", uri.getHandler()->getID());
   }
}

/*---------------------------------------------------------------------testMe-+
|                                                                             |
+----------------------------------------------------------------------------*/
static int testMe(istream & in)
{
   RegisteredURI::registerScheme(StdFileSchemeHandler());
   RegisteredURI::registerScheme(HttpSchemeHandler());
   RegisteredURI::registerScheme(DefaultSchemeHandler());
   char buf[2000];  /* hope it's enough long! */
   int state = 0;
   URI nilURI;
   URI const * baseURI = 0;
   do {
      for (;;) {
         cout <<
            "Enter a Base URI - it will be used to resolve further URIS\n"
            "or just hit return for \"no base\"...\n";
         if (in.getline(buf, sizeof buf), !in.good()) return 0;
         if (strlen(buf) == 0) {
            if (baseURI != &nilURI) {
               delete(baseURI);
               baseURI = &nilURI;
            }
         }else {
            try {
               baseURI = new RegisteredURI(buf);
            }catch (URI::MalformedException e) {
               cout << "Malformed URI: " << getURIError(e) << endl;
               continue;
            }
            cout << "---------------------- Base: \"" << buf << '\"' << endl;
            printURI(*baseURI);
            cout << '\"' << buf << "\" => \"" << *baseURI << '\"' << endl;
         }
         break;
      }
      cout <<
         "Enter the URI's to parse"
         " or just hit return to exit this loop...\n";
      for (;;) {
         if (in.getline(buf, sizeof buf), !in.good()) return 0;
         if (strlen(buf) == 0) {
            break;
         }else {
            try {
               RegisteredURI theURI(buf, *baseURI);
               cout << "---------------------------- \"" << buf << '\"' << endl;
               printURI(theURI);
               cout << '\"' << buf << "\" => \"" << theURI << '\"' << endl;
            }catch (URI::MalformedException e) {
               cout << "Malformed URI: " << getURIError(e) << endl;
            }
         }
      }
      cout << "Exit? (Y or N): ";
   }while (
      (in.getline(buf, sizeof buf), in.good()) &&
      (buf[0] != 'Y') && (buf[0] != 'y')
   );
   return 0;
}

/*--------------------------------------------------------------------testURI-+
|                                                                             |
+----------------------------------------------------------------------------*/
int testURI(char const * path)
{
   cout << "--------------- testURI" << endl;

   if (path) {
      ifstream in(path);
      if (!in.good()) {
         cout << "Cannot access \"" << path << "\" - abandon" << endl;
         return 10;
      }
      cout << "Opening \"" << path << "\" ..." << endl;
      return testMe(in);
   }else {
      return testMe(cin);
   }
}

/*===========================================================================*/
