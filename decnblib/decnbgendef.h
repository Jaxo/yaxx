/*
* $Id: decnbgendef.h,v 1.1 2002-04-14 23:28:44 jlatone Exp $
*
* General definitions.
* Mostly: machine dependent stuff.
* Keep me short and use macros sparingly.
*/

/*
| Following macro defines the dll_import and export required by Windoze.
| When the DLL is built, the compile command must define DECNB_API as in:
| cl .... -D DECNB_API=__declspec(dllexport)
*/
#if !defined _WIN32
#define DECNB_API
#elif !defined DECNB_API
#define DECNB_API __declspec(dllimport)
#endif

/*===========================================================================*/
