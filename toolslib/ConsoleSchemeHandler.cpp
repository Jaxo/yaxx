/*
* $Id: ConsoleSchemeHandler.cpp,v 1.10 2011-07-29 10:26:36 pgr Exp $
*
* A SchemeHandler for the CONSOLE protocol
*
*/

#include <string.h>
#include <iostream>
#include "miscutil.h"
#include "ConsoleSchemeHandler.h"
#include "SystemContext.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

#if defined __MWERKS__
   NullStream ConsoleSchemeHandler::Rep::nilDevice;
#endif

char const * ConsoleSchemeHandler::scheme = "console";
char const * ConsoleSchemeHandler::cinUri = "console:in";
char const * ConsoleSchemeHandler::coutUri = "console:out";
char const * ConsoleSchemeHandler::cerrUri = "console:err";

/*---------------------------------ConsoleSchemeHandler::ConsoleSchemeHandler-+
|                                                                             |
+----------------------------------------------------------------------------*/
ConsoleSchemeHandler::ConsoleSchemeHandler() : SchemeHandler(new Rep) {
   SystemContext::invalidateConsoles();
}

/*-------------------------------------------ConsoleSchemeHandler::Rep::getID-+
|                                                                             |
+----------------------------------------------------------------------------*/
char const * ConsoleSchemeHandler::Rep::getID() const {
   return scheme;
}

/*-------------------------------------------ConsoleSchemeHandler::Rep::onNew-+
|                                                                             |
+----------------------------------------------------------------------------*/
void ConsoleSchemeHandler::Rep::onNew(URI::Data & uri)
{
   if (!strcasecmp(uri.path, "in")) {
      uri.path[0] = 'i';
   }else if (!strcasecmp(uri.path, "out")) {
      uri.path[0] = 'o';
   }else if (!strcasecmp(uri.path, "err")) {
      uri.path[0] = 'e';
   }else {
      throw URI::MalformedException(URI::HANDLER_ERROR);
   }
}

/*--------------------------------------ConsoleSchemeHandler::Rep::makeStream-+
|                                                                             |
+----------------------------------------------------------------------------*/
iostream * ConsoleSchemeHandler::Rep::makeStream(
   URI const & uri, ios__openmode om
) {
   iostream * io;
   switch (uri.getPath()[0]) {
   case 'i':
      io = new iostream(getCin().rdbuf());
      break;
   case 'o':
      io = new iostream(getCout().rdbuf());
      break;
   case 'e':
      io = new iostream(getCerr().rdbuf());
      break;
   }
   #if defined _MSC_VER
      /*
      | delbuf(0), in Microsoft stream jargon, means that the stream buffer
      | won't be deleted when the stream is deleted.  For Metrowerks, we
      | don't take permission to delete a streambuf that doesn't belong to
      | to us (JaxoStreams.)  For GCC, if ISO 14882, it's sure that the
      | streambuf is not deleted.  For GCC under 3.0, I hope so -- and also
      | for non-GCC.  Otherwise, we gotta use the 3rd parameter of "setb()"
      */
      io->delbuf(0);
   #endif
   return io;
}

#ifdef TOOLS_NAMESPACE
}
#endif
/*===========================================================================*/
