/*
* $Id: ConsoleSchemeHandler.h,v 1.10 2011-07-29 10:26:36 pgr Exp $
*/

#if ! defined COM_JAXO_CONSOLESCHEMEHANDLER_H_INCLUDED
#define COM_JAXO_CONSOLESCHEMEHANDLER_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include <stdlib.h>
#include "URI.h"
#include "MemStream.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*----------------------------------------------- class ConsoleSchemeHandler -+
|                                                                             |
+----------------------------------------------------------------------------*/
class TOOLS_API ConsoleSchemeHandler : public URI::SchemeHandler {
public:
   static char const * scheme;
   static char const * cinUri;
   static char const * coutUri;
   static char const * cerrUri;

   ConsoleSchemeHandler();
   int system(char const * command);

protected:
   class Rep : public URI::SchemeHandler::Rep {
   friend class ConsoleSchemeHandler;
   protected:
      virtual istream & getCin();
      virtual ostream & getCout();
      virtual ostream & getCerr();
      virtual int system(char const * command);
   private:
      char const * getID() const;
      void onNew(URI::Data & uri);
      iostream * makeStream(URI const & uri, ios__openmode om);

      #if defined __MWERKS__
      static NullStream nilDevice;
      #endif
   };
   ConsoleSchemeHandler(RefdItemRep * pRep);
};

/* -- INLINES -- */

inline ConsoleSchemeHandler::ConsoleSchemeHandler(RefdItemRep * pRep) :
   URI::SchemeHandler(pRep)
{}

inline int ConsoleSchemeHandler::Rep::system(char const * command) {
   return ::system(command);
}
inline int ConsoleSchemeHandler::system(char const * command) {
   return ((ConsoleSchemeHandler::Rep *)inqData())->system(command);
}

#if defined __MWERKS__
inline istream & ConsoleSchemeHandler::Rep::getCin()  { return  nilDevice; }
inline ostream & ConsoleSchemeHandler::Rep::getCout() { return  nilDevice; }
inline ostream & ConsoleSchemeHandler::Rep::getCerr() { return  nilDevice; }

#else
inline istream & ConsoleSchemeHandler::Rep::getCin()  { return  std::cin;  }
inline ostream & ConsoleSchemeHandler::Rep::getCout() { return  std::cout; }
inline ostream & ConsoleSchemeHandler::Rep::getCerr() { return  std::cerr; }
#endif

#ifdef TOOLS_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
