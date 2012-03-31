/* $Id: Keyword.h,v 1.5 2002-08-25 15:30:09 pgr Exp $ */

#ifndef KEYWORD_HEADER
#define KEYWORD_HEADER

/*---------+
| Includes |
+---------*/
#include "resgendef.h"

/*---------------------------------------------------------------- KeywordId -+
|                                                                             |
+----------------------------------------------------------------------------*/
enum KeywordId {
   _KWD__ADDRESS,
   _KWD__ARG,
   _KWD__BY,
   _KWD__CALL,
   _KWD__DIGITS,
   _KWD__DO,
   _KWD__DROP,
   _KWD__ELSE,
   _KWD__END,
   _KWD__EXIT,
   _KWD__EXPOSE,
   _KWD__FOR,
   _KWD__FOREVER,
   _KWD__FORM,
   _KWD__FUZZ,
   _KWD__IF,
   _KWD__INTERPRET,
   _KWD__ITERATE,
   _KWD__LEAVE,
   _KWD__LINEIN,
   _KWD__NAME,
   _KWD__NOP,
   _KWD__NUMERIC,
   _KWD__OFF,
   _KWD__ON,
   _KWD__OPTIONS,
   _KWD__OTHERWISE,
   _KWD__PARSE,
   _KWD__PROCEDURE,
   _KWD__PULL,
   _KWD__PUSH,
   _KWD__QUEUE,
   _KWD__RETURN,
   _KWD__SAY,
   _KWD__SELECT,
   _KWD__SIGNAL,
   _KWD__SOURCE,
   _KWD__THEN,
   _KWD__TO,
   _KWD__TRACE,
   _KWD__UNTIL,
   _KWD__UPPER,
   _KWD__VALUE,
   _KWD__VAR,
   _KWD__VERSION,
   _KWD__WHEN,
   _KWD__WHILE,
   _KWD__WITH,
   _KWD_NumberOf,

   _KWD__COMMAND = _KWD_NumberOf,
   _KWD__ENGINEERING,
   _KWD__ERROR,
   _KWD__FAILURE,
   _KWD__FUNCTION,
   _KWD__HALT,
   _KWD__LOSTDIGITS,
   _KWD__NORMAL,        // trace
   _KWD__NOTREADY,
   _KWD__NOVALUE,
   _KWD__RC,
   _KWD__READY,         // stream
   _KWD__RESULT,
   _KWD__SCIENTIFIC,
   _KWD__SIGL,
   _KWD__SUBROUTINE,
   _KWD__SYNTAX
};

RESOURCES_API char const * getKeyword(KeywordId id);

#endif /* KEYWORD_HEADER ====================================================*/
