/*
* $Id: resgendef.h,v 1.1 2002-04-14 23:28:44 jlatone Exp $
*
* General definitions.
* Mostly: machine dependent stuff.
* Keep me short and use macros sparingly.
*/
#ifndef COM_JAXO_YAXX_RESGENDEF_H_INCLUDED
#define COM_JAXO_YAXX_RESGENDEF_H_INCLUDED

/*
| Following macro defines the dll_import and export required by Windoze.
| When the DLL is built, the compile command must define RESOURCES_API as in:
| cl .... -D RESOURCES_API=__declspec(dllexport)
*/
#if !defined _WIN32 || defined NODLL
#define RESOURCES_API
#elif !defined RESOURCES_API
#define RESOURCES_API __declspec(dllimport)
#endif

#endif
/*===========================================================================*/
