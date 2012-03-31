/*
* $Id: toolsgendef.h,v 1.2 2002-05-11 09:13:03 jlatone Exp $
*
* General definitions.  Mostly: machine dependent stuff.
* Must always be included after the system includes!
* Keep me short and use macros sparingly.
*/

#ifndef TOOLSGENDEF_H_INCLUDED
#define TOOLSGENDEF_H_INCLUDED

/*
| Following macro define the dll_import and export required by Windoze.
| When the DLL is built, the compile command must define TOOLS_API as in:
| cl .... -D TOOLS_API=__declspec(dllexport)
*/
#if !defined _WIN32
#define TOOLS_API
#elif !defined TOOLS_API
#define TOOLS_API __declspec(dllimport)
#endif

#if __GNUC__ >= 3
#define ISO_14882
#endif

#endif
/*===========================================================================*/
