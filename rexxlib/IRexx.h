/*
*  $Id: IRexx.h,v 1.1 2002-05-23 17:12:03 pgr Exp $
*
* This is NOT the classic REXXSAA, but it should be no problem
* migrating to it.  Main differences are:
* - it is a C++ header
* - it using streams, instead of RXSTRING for the input and tokenized
*   output.
*/

#ifndef COM_JAXO_YAXX_REXXSAA_H_INCLUDED
#define COM_JAXO_YAXX_REXXSAA_H_INCLUDED

/*---------+
| Includes |
+---------*/
#include "rexxgendef.h"

typedef char CHAR;
typedef char * PCHAR;
typedef char * PCH;
typedef char * PSZ;
typedef unsigned char UCHAR;
typedef unsigned char * PUCHAR;
typedef short SHORT;
typedef short * PSHORT;
typedef unsigned short USHORT;
typedef unsigned short * PUSHORT;
typedef long LONG;
typedef long * PLONG;
typedef unsigned long ULONG;

#ifdef INCL_REXXSAA
#define INCL_RXSYSEXIT
#define INCL_RXSHV
#define INCL_RXSUBCOM
#define INCL_RXFUNC
#endif

#define APIENTRY  REXX_API
#define APIRET ULONG

#define MAKERXSTRING(x,c,l)   ((x).strptr=(c),(x).strlength=(l))
#define RXNULLSTRING(x)       (!(x).strptr)
#define RXSTRLEN(x)           ((x).strptr ? (x).strlength : 0)
#define RXSTRPTR(x)           ((x).strptr)
#define RXVALIDSTRING(x)      ((x).strptr && (x).strlength)
#define RXZEROLENSTRING(x)    ((x).strptr && !(x).strlength)
#define RXAUTOBUFLEN  256L

#define RXCOMMAND         0
#define RXSUBROUTINE      1
#define RXFUNCTION        2

typedef struct t_RXSTRING {
   ULONG strlength;
   char* strptr;
}RXSTRING;
typedef RXSTRING * PRXSTRING;

typedef struct t_RXSYSEXIT {
   char *sysexit_name;
   LONG  sysexit_code;
}RXSYSEXIT;
typedef RXSYSEXIT *PRXSYSEXIT;

#ifdef INCL_RXSYSEXIT
#error RXSYSEXIT is not yet implemented
#endif /* INCL_RXSYSEXIT */

#ifdef INCL_RXSHV
#error RXSHV is not yet implemented
#endif /* INCL_RXSHV */

APIRET APIENTRY RexxStart(
   LONG       argc,
   PRXSTRING  argv,
   PSZ        pszName,     // only used by PARSE SOURCE (no files)
   PRXSTRING  instore,
   PSZ        envName,
   LONG       callType,    // RXCOMMAND, RXROUTINE or RXFUNCTION
   PRXSYSEXIT exits,
   PSHORT     rc,
   PRXSTRING  result
);

#ifdef INCL_RXSUBCOM
#error RXSUBCOM is not yet implemented
#endif /* INCL_RXSUBCOM */

#ifdef INCL_RXFUNC
#error RXFUNC is not yet implemented
#endif /* INCL_RXFUNC */

#endif
/*===========================================================================*/
