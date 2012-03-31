/*
* $Id: ctype_fix.h,v 1.3 2002-07-04 17:19:12 pgr Exp $
*/
#ifndef CTYPE_FIX_4_PALMOS30_DEFINED
#define CTYPE_FIX_4_PALMOS30_DEFINED

#ifdef _MSL_CTYPE_H
   #error <ctype.h> already included!
#endif

#define isalnum  bad_isalnum
#define isalpha  bad_isalpha
#define iscntrl  bad_iscntrl
#define isdigit  bad_isdigit
#define isgraph  bad_isgraph
#define islower  bad_islower
#define isprint  bad_isprint
#define ispunct  bad_ispunct
#define isspace  bad_isspace
#define isupper  bad_isupper
#define isxdigit bad_isxdigit

#include <ctype.h>

#undef isalnum
#undef isalpha
#undef iscntrl
#undef isdigit
#undef isgraph
#undef islower
#undef isprint
#undef ispunct
#undef isspace
#undef isupper
#undef isxdigit

inline int isalnum(int c);
inline int isalpha(int c);
inline int iscntrl(int c);
inline int isdigit(int c);
inline int isgraph(int c);
inline int islower(int c);
inline int isprint(int c);
inline int ispunct(int c);
inline int isspace(int c);
inline int isupper(int c);
inline int isxdigit(int c);

#define isAnEof(c) (((UInt16)(c))==0xffff)

int isalnum(int c)  { return (isAnEof(c))? 0 : bad_isalnum(c); }
int isalpha(int c)  { return (isAnEof(c))? 0 : bad_isalpha(c); }
int iscntrl(int c)  { return (isAnEof(c))? 0 : bad_iscntrl(c); }
int isdigit(int c)  { return (isAnEof(c))? 0 : bad_isdigit(c); }
int isgraph(int c)  { return (isAnEof(c))? 0 : bad_isgraph(c); }
int islower(int c)  { return (isAnEof(c))? 0 : bad_islower(c); }
int isprint(int c)  { return (isAnEof(c))? 0 : bad_isprint(c); }
int ispunct(int c)  { return (isAnEof(c))? 0 : bad_ispunct(c); }
int isspace(int c)  { return (isAnEof(c))? 0 : bad_isspace(c); }
int isupper(int c)  { return (isAnEof(c))? 0 : bad_isupper(c); }
int isxdigit(int c) { return (isAnEof(c))? 0 : bad_isxdigit(c); }

#endif
/*===========================================================================*/







