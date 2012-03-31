/*
* $Id: rexxgendef.h,v 1.2 2002-10-24 06:25:00 pgr Exp $
*
* General definitions.
* Mostly: machine dependent stuff.
* Keep me short and use macros sparingly.
*/

/*
| Following macro defines the dll_import and export required by Windoze.
| When the DLL is built, the compile command must define REXX_API as in:
| cl .... -D REXX_API=__declspec(dllexport)
*/
#ifndef COM_JAXO_YAXX_REXXGENDEF_H_INCLUDED
#define COM_JAXO_YAXX_REXXGENDEF_H_INCLUDED

#if !defined _WIN32
#define REXX_API
#elif !defined REXX_API
#define REXX_API __declspec(dllimport)
#endif

#endif
/*===========================================================================*/
