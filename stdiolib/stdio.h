/*
* $Id: stdio.h,v 1.6 2002-04-04 21:27:32 jlatone Exp $
*
* (C) Copyright Jaxo Inc., 2001
* This work contains confidential trade secrets of Jaxo Inc.
* Use, examination, copying, transfer and disclosure to others
* are prohibited, except with the express written agreement of Jaxo Inc.
*
* Author:  Pierre G. Richard
* Written: 2/2/2001
*
*/

/*---------+
| Includes |
+---------*/
/* get around a problem of jni.h including stdio.h for FILE in JDK 1.1 */
#if defined COM_JAXO_IO_INCLUDING_JNI
#define COM_JAXO_IO_INCLUDED_JNI
#define FILE void *

#elif ! defined COM_JAXO_IO_STDIO_H_INCLUDED
#define COM_JAXO_IO_STDIO_H_INCLUDED

/* get prepared to not include ourself but just enough for stupid jni.h ! */
#define COM_JAXO_IO_INCLUDING_JNI
#ifdef USE_JNIFILE
#include <jni.h>
#else
#if !defined _WIN32
#define JNIEXPORT
#define JNIIMPORT
#else
#define JNIEXPORT __declspec(dllexport)
#define JNIIMPORT
#endif
#include <stdarg.h>
#endif
#undef COM_JAXO_IO_INCLUDING_JNI
#if defined COM_JAXO_IO_INCLUDED_JNI
#undef FILE
#undef COM_JAXO_IO_INCLUDED_JNI
#endif


/*
| The stdlib on HP-UX includes stdio.h,
| and expects a FILE type to be defined.
| I guess I could either define a FILE here,
| or just force the inclusion of the standard stdio header.
*/
#ifdef __hpux
#define FILE struct FILE
//#include </usr/include/stdio.h>
#endif

#include <stdlib.h>
#include <setjmp.h>

// JAL Relieve up our dependency on this!
//#define USE_JNIFILE

#ifdef __cplusplus
extern "C" {
#endif


/*
| In C (ANSI), stdio.h also prototypes these methods
| you must define these values according to what your stdio says.
*/
#if defined __sun
#if !defined BUFSIZ
#define BUFSIZ  1024
#endif
extern int sscanf(const char *, const char *, ...);

#elif defined _WIN32
#if !defined BUFSIZ
#define BUFSIZ  4096
#endif
_CRTIMP int __cdecl sscanf(const char *, const char *, ...);

#elif defined __linux
#if !defined BUFSIZ
#define BUFSIZ  1024
#endif
extern int sscanf(const char *, const char *, ...);

#elif defined __hpux
#if !defined BUFSIZ
#define BUFSIZ  1024
#endif
extern int sscanf(const char *, const char *, ...);

#elif defined __palmos__
//#if !defined BUFSIZ
//#define BUFSIZ  1024
//#endif
//extern int sscanf(const char *, const char *, ...);

#else
#error Unknown System

#endif


/*
| This is our JFILE structure
*/
#ifdef __GNUC__
#define JFILE struct t_JFILE
#else
typedef struct t_JFILE JFILE;
#endif

struct t_JFILE {               /* really: class JFILE */
   /* base class members */
   char * name;                /* name of this file */
   char * info;                /* extraneous information (origin URL) */
   int isError;                /* ferror / clearerr */
   int iOffset;                /* count of bytes read */
   jmp_buf * exceptionHandler; /* where to long-jump if in troubles */
   int exceptionUserData;      /* what set_jump will return */

   /* pure virtuals */
   JFILE * (*checkOpen) (JFILE *, const char *, const char *);
   size_t (*fread)      (void *, size_t, size_t, JFILE *);
   size_t (*fwrite)     (const void *, size_t, size_t, JFILE *);
   int    (*fgetc)      (JFILE *);
   int    (*fputc)      (int, JFILE *);
   int    (*ungetc)     (int, JFILE *);
   int    (*feof)       (JFILE *);
   int    (*fflush)     (JFILE *);
   int    (*fclose)     (JFILE *);
   int    (*fileno)     (JFILE *);

   /*
   | extensions depending
   | on the file type may follow
   */
};

/*
| a li'll magic in here
*/
char * makeNameFromFilePointer(JFILE const * pFile, char const * pszClassName);
JFILE * getFilePointerFromName(char const * name);

/*
| These methods replace the standard FILE methods of C
| Not everything is yet implemented (be my guest!)
*/
#define JEOF (-1)
JNIEXPORT JFILE * jfopen(const char *, const char *);
JNIEXPORT size_t jfread(void *, size_t, size_t, JFILE *);
JNIEXPORT size_t jfwrite(const void *, size_t, size_t, JFILE *);
JNIEXPORT int jfgetc(JFILE *);
JNIEXPORT int jgetc(JFILE *);
JNIEXPORT int jfputc(int, JFILE *);
JNIEXPORT int jputc(int, JFILE *);
JNIEXPORT char * jfgets(char *, int, JFILE *);
JNIEXPORT int jfputs(const char *, JFILE *);
JNIEXPORT int jungetc(int, JFILE *);
JNIEXPORT long jftell(JFILE *);
JNIEXPORT int jfflush(JFILE *);
JNIEXPORT int jfeof(JFILE *);
JNIEXPORT int jfclose(JFILE *);
JNIEXPORT int jferror(JFILE *);
JNIEXPORT void jclearerr(JFILE *);
JNIEXPORT int jfileno(JFILE *);
#define _jfileno(stream)  jfileno(stream)

JNIEXPORT int jprintf(char const *, ...);
JNIEXPORT int jvprintf(char const *, va_list);
JNIEXPORT int jsprintf(char *, char const *, ...);
JNIEXPORT int jvsprintf(char *, char const *, va_list);
JNIEXPORT int jfprintf(JFILE *, char const *, ...);
JNIEXPORT int jvfprintf(JFILE *, char const *, va_list);

#ifdef LIBMAKER_stdio
JNIEXPORT JFILE * jstdout;
JNIEXPORT JFILE * jstdin;
JNIEXPORT JFILE * jstderr;
#else
/*
| This doesn't work.  Sun and Linux define JNIIMPORT as nothing,
| therefore any other library including this stdio.h was given
| its own copy of jstdout, jstdin, and jstderr.  For example,
| nm libgifconv.so and grep for the definition of these.
| HP defines JNIIMPORT as extern, as I would expect.
| So, either we're using JNIEXPORT/JNIIMPORT improperly here,
| or either Sun/Linux or HP has the wrong definition.
| The only reason this didn't core dump is that the IO functions
| protect against null files.  On HP, this was noticed because
| the GIF converter writes warning messages to stderr,
| which have to be cleaned up.
*/
#if defined __sun
extern JFILE * jstdout;
extern JFILE * jstdin;
extern JFILE * jstderr;
#elif defined __linux
extern JFILE * jstdout;
extern JFILE * jstdin;
extern JFILE * jstderr;
#else
JNIIMPORT JFILE * jstdout;
JNIIMPORT JFILE * jstdin;
JNIIMPORT JFILE * jstderr;
#endif
#endif

/*
| MemoryFiles are accessible from standard IO by just providing a
| special name.  This is done be the methods below.
| Note that we use JNIEXPORT for non-really-JNI exports, but
| dll_export on NT, etc...
*/
JNIEXPORT char const * acquireMemoryFileName();
JNIEXPORT void releaseMemoryFileName(char const * name);

/*
| This method was added to handle java.io.IOException
*/
JNIEXPORT void setExceptionHandler(JFILE *, jmp_buf *, int);

/*
| This method to get extraneous infos associated to the file (or null)
*/
JNIEXPORT char const * getInfos(JFILE *);

#ifdef __cplusplus
}
#endif

/*
| HP's C compiler seems to need to include the standard stdio.h,
| which defines a macro for feof, which wreaks havoc here.
*/
#ifdef __hpux
#ifndef __GNUC__
#ifdef feof
#undef feof
#endif
#endif
#endif

/*
| This is for conveniency: use the C-preprocessor to avoid you to
| rewrite your fopen, fread, fwrite, etc...
| If you don't like it, #define USE_REAL_JFILEMETHODS
*/
#if !defined USE_REAL_JFILEMETHODS
#define FILE     JFILE
#ifndef EOF
#define EOF      JEOF
#endif

#define fopen    jfopen
#define fread    jfread
#define fwrite   jfwrite
#define fgetc    jfgetc
#define getc     jgetc
#define fputc    jfputc
#define putc     jputc
#define fgets    jfgets
#define fputs    jfputs
#define ungetc   jungetc
#define ftell    jftell
#define fflush   jfflush
#define feof     jfeof
#define fclose   jfclose
#define ferror   jferror
#define clearerr jclearerr
#define fileno   jfileno
#define _fileno  jfileno

#define stdin    jstdin
#define stdout   jstdout
#define stderr   jstderr

#define printf   jprintf
#define vprintf  jvprintf
#define sprintf  jsprintf
#define vsprintf jvsprintf
#define fprintf  jfprintf
#define vfprintf jvfprintf

#endif

#endif
/*===========================================================================*/
