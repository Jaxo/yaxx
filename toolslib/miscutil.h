/*
* $Id: miscutil.h,v 1.6 2002-09-05 02:29:15 jlatone Exp $
*
* Miscellaneous utilities that everyone rewrite
* b/c ANSI didn't want to consider them.
*
*/

#ifndef COM_JAXO_TOOLS_MISCUTIL_H_INCLUDED
#define COM_JAXO_TOOLS_MISCUTIL_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include "toolsgendef.h"

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

// ----- strcasecmp
#ifdef _WIN32
#define strcasecmp stricmp
#define strncasecmp strnicmp

#else 
#define COM_JAXO_TOOLS_MISCUTIL_STRCASE_WANTED
extern TOOLS_API int strncasecmp(char const * s1, char const * s2, int n);
extern TOOLS_API int strcasecmp(char const * s1, char const * s2);
#endif

// ----- itoa, atoi
extern TOOLS_API bool stol(char const **, long *);
extern TOOLS_API unsigned int ltols(long val, char *c_p);
extern TOOLS_API unsigned int ulbtos(unsigned long val, char * c_p, int radix);
extern TOOLS_API int c2int(char const * p);
extern TOOLS_API unsigned int hex2uint(char const * s, int len);

// ----- helpers
/*
| Associate a value to an ASCII character.
| Poor humans have only 10 fingers (hence, 10 digits.)  To count after 9,
| computers use the human alphabet ('A' is 10), ignoring the case ('a' is
| also 10.)  This is what this code does.  It can count up to 40, but
| limit it to 36 (10 digits + 26 letters).  I have troubles explaining to
| my mom why 'A' is 10, how could I tell her that '[' or '{' is 37?
| (>>>PGR: written in 1988, comments included)
*/
inline unsigned char charToValue(char c) {
  if (c <= '9') {
     return (c >= '0')? (c & 0xF) : 0xFF;
  }else {
     return 10 + (unsigned char)((c & ~('a'-'A'))-'A'); // fold twice
  }
}

#ifdef TOOLS_NAMESPACE
}
#endif
#endif
/*===========================================================================*/
