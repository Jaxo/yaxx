/* $Id: Keyword.cpp,v 1.4 2002-05-21 11:08:02 pgr Exp $ */

#include "Keyword.h"

#if defined __MWERKS__   // MWERKS has its own keywords resources
RESOURCES_API char const * getKeywordRsc(int id);

#else
static char const * kwdTable[] = {
   "ADDRESS",
   "ARG",
   "BY",
   "CALL",
   "DIGITS",
   "DO",
   "DROP",
   "ELSE",
   "END",
   "EXIT",
   "EXPOSE",
   "FOR",
   "FOREVER",
   "FORM",
   "FUZZ",
   "IF",
   "INTERPRET",
   "ITERATE",
   "LEAVE",
   "LINEIN",
   "NAME",
   "NOP",
   "NUMERIC",
   "OFF",
   "ON",
   "OPTIONS",
   "OTHERWISE",
   "PARSE",
   "PROCEDURE",
   "PULL",
   "PUSH",
   "QUEUE",
   "RETURN",
   "SAY",
   "SELECT",
   "SIGNAL",
   "SOURCE",
   "THEN",
   "TO",
   "TRACE",
   "UNTIL",
   "UPPER",
   "VALUE",
   "VAR",
   "VERSION",
   "WHEN",
   "WHILE",
   "WITH",
   "COMMAND",
   "ENGINEERING",
   "ERROR",
   "FAILURE",
   "FUNCTION",
   "HALT",
   "LOSTDIGITS",
   "NORMAL",
   "NOTREADY",
   "NOVALUE",
   "RC",
   "READY",
   "RESULT",
   "SCIENTIFIC",
   "SIGL",
   "SUBROUTINE",
   "SYNTAX"
};
#endif

/*-----------------------------------------------------------------getKeyword-+
|                                                                             |
+----------------------------------------------------------------------------*/
char const * getKeyword(KeywordId id) {
#if defined __MWERKS__
   return getKeywordRsc(id);
#else
   return kwdTable[(int)id];
#endif
}

/*===========================================================================*/
