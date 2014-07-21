/*
* $Id: yasp3gendef.h,v 1.1 2002-04-14 23:28:44 jlatone Exp $
*
* General definitions.
* Mostly: machine dependent stuff.
* Keep me short and use macros sparingly.
*/
#ifndef COM_JAXO_YAXX_YASP3GENDEF_H_INCLUDED
#define COM_JAXO_YAXX_YASP3GENDEF_H_INCLUDED

/*
| Following macro define the dll_import and export required by Windoze.
| When the DLL is built, the compile command must define YASP3_API as in:
| cl .... -D YASP3_API=__declspec(dllexport)
*/
#if !defined _WIN32 || defined NODLL
#define YASP3_API
#elif !defined YASP3_API
#define YASP3_API __declspec(dllimport)
#endif

#endif
/*===========================================================================*/
