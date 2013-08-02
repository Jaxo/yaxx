/*
* $Id: java2c.h,v 1.4 2012-04-07 15:25:29 pgr Exp $
*
* (C) Copyright Jaxo Inc., 2001
* This work contains confidential trade secrets of Jaxo Inc.
* Use, examination, copying, transfer and disclosure to others
* are prohibited, except with the express written agreement of Jaxo Inc.
*
* Author:  Pierre G. Richard
* Written: 02/16/2001
*
* Provide a method for calling a C-main from Java
* translating an array of java Strings to the C classical (argc, argv)          |
*/

/*---------+
| Includes |
+---------*/
#if ! defined COM_JAXO_IO_JAVA2C_H_INCLUDED
#define COM_JAXO_IO_JAVA2C_H_INCLUDED

/* get around a problem of jni.h including stdio.h for FILE in JDK 1.1 */
#define COM_JAXO_IO_INCLUDING_JNI
#include <jni.h>
#undef COM_JAXO_IO_INCLUDING_JNI
#if defined COM_JAXO_IO_INCLUDED_JNI
#undef FILE
#undef COM_JAXO_IO_INCLUDED_JNI
#endif

#include <stdlib.h>

#define USE_JNIMAIN

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*MainEntry)(int argc, char const * const *);
typedef int (*MainWideEntry)(int argc, jchar const * const *);

JNIEXPORT int java2C_execute(
   JNIEnv * pEnv,
   char const * pgmName,
   MainEntry pgmEntry,
   jobjectArray args
);

JNIEXPORT int java2C_wexecute(
   JNIEnv * pEnv,
   jchar const * pgmName,
   MainWideEntry pgmEntry,
   jobjectArray args
);

#ifdef __cplusplus
}
#endif

#endif
/*===========================================================================*/

