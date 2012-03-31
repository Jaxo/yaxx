/*
* $Id: java2c.c,v 1.2 2002-04-04 05:10:59 jlatone Exp $
*
* (C) Copyright Jaxo Inc., 2001
* This work contains confidential trade secrets of Jaxo Inc.
* Use, examination, copying, transfer and disclosure to others
* are prohibited, except with the express written agreement of Jaxo Inc.
*
* Author:  Pierre G. Richard
* Written: 02/16/2001
*
*/

/*---------+
| Includes |
+---------*/
#include "java2c.h"

/*-------------------------------------------------------------java2C_execute-+
| Translate an array of java Strings to the C classical (argc, argv)          |
+----------------------------------------------------------------------------*/
JNIEXPORT int java2C_execute(
   JNIEnv * pEnv,
   char const * pgmName,
   MainEntry pgmEntry,
   jobjectArray args
) {
   jsize i;
   jint retcode = -1;
   int argc = 1+(*pEnv)->GetArrayLength(pEnv, args);
   char const ** argv = malloc((1+argc) * sizeof(char const *));

   argv[0] = pgmName;
   for (i=1; i < argc; ++i) {
      argv[i] = (*pEnv)->GetStringUTFChars(
         pEnv,
         (jstring)(*pEnv)->GetObjectArrayElement(pEnv, args, i-1), 0
      );
   }
   retcode = pgmEntry(argc, argv);
   for (i=1; i < argc; ++i) {
      (*pEnv)->ReleaseStringUTFChars(
         pEnv,
         (jstring)(*pEnv)->GetObjectArrayElement(pEnv, args, i-1), argv[i]
      );
   }
   free((void *)argv);
   return retcode;
}

/*------------------------------------------------------------java2C_wexecute-+
| Same as above with wide chars                                               |
+----------------------------------------------------------------------------*/
JNIEXPORT int java2C_wexecute(
   JNIEnv * pEnv,
   jchar const * pgmName,
   MainWideEntry pgmEntry,
   jobjectArray args
) {
   jsize i;
   jint retcode = -1;
   int argc = 1+(*pEnv)->GetArrayLength(pEnv, args);
   jchar const ** argv = malloc((1+argc) * sizeof(jchar const *));

   argv[0] = pgmName;
   for (i=1; i < argc; ++i) {
      argv[i] = (*pEnv)->GetStringChars(
         pEnv,
         (jstring)(*pEnv)->GetObjectArrayElement(pEnv, args, i-1), 0
      );
   }
   retcode = pgmEntry(argc, argv);
   for (i=1; i < argc; ++i) {
      (*pEnv)->ReleaseStringChars(
         pEnv,
         (jstring)(*pEnv)->GetObjectArrayElement(pEnv, args, i-1), argv[i]
      );
   }
   free((void *)argv);
   return retcode;
}

/*===========================================================================*/

