/*
* $Id: resgendef.h,v 1.1 2002-04-14 23:28:44 jlatone Exp $
*
* General definitions.
* Mostly: machine dependent stuff.
* Keep me short and use macros sparingly.
*/

/*
| Following macro define the dll_import and export required by Windoze.
| When the DLL is built, the compile command must define RESOURCES_API as in:
| cl .... -D RESOURCES_API=__declspec(dllexport)
*/
#if !defined _WIN32
#define RESOURCES_API
#elif !defined RESOURCES_API
#define RESOURCES_API __declspec(dllimport)
#endif

/*===========================================================================*/
