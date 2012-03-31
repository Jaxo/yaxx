/*
* $Id: ConsoleSchemeHandler.h,v 1.10 2011-07-29 10:26:36 pgr Exp $
*/

#if ! defined COM_JAXO_CONSOLESCHEMEHANDLER_H_INCLUDED
#define COM_JAXO_CONSOLESCHEMEHANDLER_H_INCLUDED

/*---------+
| Includes |
+---------*/
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

protected:
   class Rep : public URI::SchemeHandler::Rep {
   protected:
      virtual istream & getCin();
      virtual ostream & getCout();
      virtual ostream & getCerr();
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
