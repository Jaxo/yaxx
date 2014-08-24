/*
* $Id: JRexx.cpp,v 1.3 2011-08-31 08:10:33 pgr Exp $
*
* (C) Copyright 2011 Jaxo Inc.  All rights reserved.
* This work contains confidential trade secrets of Jaxo.
* Use, examination, copying, transfer and disclosure to others
* are prohibited, except with the express written agreement of Jaxo.
*
* Author:  Pierre G. Richard
* Written: 8/5/2011
*/
#include <string.h>
#include <stdio.h>
#include <jni.h>
#include "../rexxlib/Rexx.h"
#include "../toolslib/SystemContext.h"
#include "JConsole.h"
#include "SpeakerStream.h"
#ifdef ANDROID
#include <android/log.h>
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO, "JREXX",__VA_ARGS__)
#endif

#ifndef YAXX_NAMESPACE
#define YAXX_NAMESPACE
#endif

#ifndef TOOLS_NAMESPACE
#define TOOLS_NAMESPACE
#endif

extern "C" {

JNIEnv * g_jniEnv = 0;

static unsigned int getLibraryAddress(char const * libname);

/*-----------------------------------Java_com_jaxo_android_rexx_Rexx_commence-+
| A SystemContext (kind of singleton) is required for cin, cout, etc)         |
| That should be done at the very first place.                                |
| Extra SchemeHandler's can be added                                          |
| see yaxx/yaxx/main.cpp and yaxx/mwerks/irexx2 for examples                  |
+----------------------------------------------------------------------------*/
jint Java_com_jaxo_android_rexx_Rexx_commence(
   JNIEnv * env,
   jobject thiz,
   jobject console,
   jstring baseUri,
   jobject speaker
) {
   char const * pBaseUri = env->GetStringUTFChars(baseUri, 0);
   env->SetLongField(
      thiz,
      env->GetFieldID(env->GetObjectClass(thiz), "context", "J"),
      (jlong)new SystemContext(
         pBaseUri,
         StdFileSchemeHandler(),
         K_SchemeHandler(env, console),
         SpeakerSchemeHandler(env, speaker)
      )
   );
   env->ReleaseStringUTFChars(baseUri, pBaseUri);
   LOGI("libtoolslib starts at 0x%08x\n", getLibraryAddress("libtoolslib.so"));
}

/*----------------------------------Java_com_jaxo_android_rexx_Rexx_terminate-+
|                                                                             |
+----------------------------------------------------------------------------*/
jint Java_com_jaxo_android_rexx_Rexx_terminate(JNIEnv * env, jobject thiz) {
   g_jniEnv = env;
   delete (SystemContext *)env->GetLongField(
      thiz,
      env->GetFieldID(env->GetObjectClass(thiz), "context", "J")
   );
}

/*----------------------------------Java_com_jaxo_android_rexx_Rexx_interpret-+
|                                                                             |
+----------------------------------------------------------------------------*/
jint Java_com_jaxo_android_rexx_Rexx_interpret(
   JNIEnv * env,
   jobject thiz,
   jstring script,
   jstring args
) {
   LOGI("Starting interpret");
   JNIEnv * oldJniEnv = g_jniEnv;
   g_jniEnv = env;
   YAXX_NAMESPACE::Rexx rexx;

   char const * pScript = env->GetStringUTFChars(script, 0);
   char const * pArgs = env->GetStringUTFChars(args, 0);
   // >>>PGR (spent 2 hours on this)
   // Coming from Java, it's not obvious having to write all these args
   // just for a naive MemStream taking its contents from a stream...
   // I should add a basic constructor from (char const *)
   TOOLS_NAMESPACE::MemStream scriptStream(
      pScript, strlen(pScript), MemStream::Constant
   );
   int rc;
   try {
      Rexx::Script script(scriptStream, "current script"); // FIXME
      RexxString rexxResult;
      rc = rexx.interpret(script, pArgs, rexxResult); // 0 if everything is OK
      URI::SchemeHandler h = RegisteredURI::getSchemeHandler(
         ConsoleSchemeHandler::scheme
      );
      ((K_SchemeHandler *)&h)->setResult((char const *)rexxResult);
   }catch (FatalException & e) {
      URI::SchemeHandler h = RegisteredURI::getSchemeHandler(
         ConsoleSchemeHandler::scheme
      );
      ((K_SchemeHandler *)&h)->setMessage((char const *)e.m_msg);
      rc = e.m_codeNo;
   }catch (...) {
      rc = -1;
   }
   env->ReleaseStringUTFChars(script, pScript);
   env->ReleaseStringUTFChars(args, pArgs);
   LOGI("Done interpreting. RC is %d", rc);
   g_jniEnv = oldJniEnv;
   return rc;
}

/*----------------------------------------------------------getLibraryAddress-+
| Debug helper                                                                |
| Use: addr2line -j .text -e <libname> 0xnnnnn                                |
| where 0xnnnn is <address of the bug> - <address of the library>             |
| Other useful commands:                                                      |
|   objdump -tT libtoolslib.so                                                |
|   objdump -X libtoolslib.so                                                 |
|   file libtoolslib.so                                                       |
|   nm libtoolslib.so                                                         |
+----------------------------------------------------------------------------*/
static unsigned int getLibraryAddress(char const * libname)
{
   // char path[256];
   // snprintf(path, sizeof path, "/proc/%d/smaps", getpid());
   FILE * file = fopen("/proc/self/maps", "rt");
   if (!file) {
      return 0;
   }
   unsigned int addr = 0;
   int libnameLen = strlen(libname);
   char buf[256];
   while (fgets(buf, sizeof buf, file)) {
      int len = strlen(buf);
      if ((len > 0) && (buf[len-1] == '\n')) buf[--len] = '\0';
      if (
         (len > libnameLen) &&
         (memcmp(buf+len-libnameLen, libname, libnameLen) == 0)
      ) {
         unsigned int start, end, offset;
         char flags[4];
         if (
            (
               7 == sscanf(
                  buf, "%zx-%zx %c%c%c%c %zx",
                  &start, &end, &flags[0], &flags[1], &flags[2], &flags[3],
                  &offset
               )
            )
            && (flags[0]=='r') /*&& (flags[1]=='-')*/ && (flags[2]=='x')
         ) {
            addr = start - offset;
            break;
         }
      }
   }
   fclose(file);
   return addr;
}

}
/*===========================================================================*/
