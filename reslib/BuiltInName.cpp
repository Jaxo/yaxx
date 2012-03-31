/* $Id: BuiltInName.cpp,v 1.1 2002-05-02 06:36:43 pgr Exp $ */

#include "BuiltInName.h"

#if defined __MWERKS__   // MWERKS has its own built-in names resources
RESOURCES_API char const * getBuiltInNameRsc(int id);

#else
static char const * bltinTable[] = {
   "ABBREV",
   "ABS",
   "ACOS",
   "ADDRESS",
   "ARG",
   "ASIN",
   "ATAN",
   "ATAN2",
   "B2X",
   "BITAND",
   "BITOR",
   "BITXOR",
   "C2D",
   "C2X",
   "CENTER",
   "CENTRE",
   "CHANGESTR",
   "CHARIN",
   "CHAROUT",
   "CHARS",
   "COMPARE",
   "COPIES",
   "COS",
   "COSH",
   "COUNTSTR",
   "D2C",
   "D2X",
   "DATATYPE",
   "DATE",
   "DELSTR",
   "DELWORD",
   "DESBUF",
   "DIGITS",
   "DROPBUF",
   "ERRORTEXT",
   "EXP",
   "FORM",
   "FORMAT",
   "FUZZ",
   "INSERT",
   "LASTPOS",
   "LEFT",
   "LENGTH",
   "LINEIN",
   "LINEOUT",
   "LINES",
   "LOAD",
   "LOG",
   "LOG10",
   "MAKEBUF",
   "MAX",
   "MIN",
   "OVERLAY",
   "POS",
   "POW",
   "POW10",
   "QUEUED",
   "RANDOM",
   "REVERSE",
   "RIGHT",
   "SIGN",
   "SIN",
   "SINH",
   "SOURCELINE",
   "SPACE",
   "SQRT",
   "STREAM",
   "STRIP",
   "SUBSTR",
   "SUBWORD",
   "SYMBOL",
   "TAN",
   "TANH",
   "TIME",
   "TRACE",
   "TRANSLATE",
   "TRUNC",
   "VALUE",
   "VERIFY",
   "WORD",
   "WORDINDEX",
   "WORDLENGTH",
   "WORDPOS",
   "WORDS",
   "X2B",
   "X2C",
   "X2D",
   "XMLATTVAL",
   "XMLIN",
   "XMLOUT",
   "XMLTAGNAME",
   "XMLTYPE",
   "XRANGE",
};
#endif

/*-------------------------------------------------------------getBuiltInName-+
|                                                                             |
+----------------------------------------------------------------------------*/
char const * getBuiltInName(BuiltInNameId id) {
#if defined __MWERKS__
   return getBuiltInNameRsc(id);
#else
   return bltinTable[(int)id];
#endif
}

/*===========================================================================*/

