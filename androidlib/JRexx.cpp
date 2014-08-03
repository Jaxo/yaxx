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
#include <jni.h>
#include "../rexxlib/Rexx.h"
#include "../toolslib/SystemContext.h"
#include "JConsole.h"
#include "SpeakerStream.h"
#include <android/log.h>
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO, "JREXX",__VA_ARGS__)

#ifndef YAXX_NAMESPACE
#define YAXX_NAMESPACE
#endif

#ifndef TOOLS_NAMESPACE
#define TOOLS_NAMESPACE
#endif

extern "C" {

JNIEnv * g_jniEnv = 0;

/*---------------------------------Java_com_jaxo_android_rexx_Rexx_initialize-+
| A SystemContext (kind of singleton) is required for cin, cout, etc)         |
| That should be done at the very first place.                                |
| Extra SchemeHandler's can be added                                          |
| see yaxx/yaxx/main.cpp and yaxx/mwerks/irexx2 for examples                  |
+----------------------------------------------------------------------------*/
jint Java_com_jaxo_android_rexx_Rexx_initialize(
   JNIEnv * env,
   jobject thiz,
   jobject console,
   jstring baseUri,
   jobject speaker
) {
   LOGI("JRexx: SystemContext constructor");
   char const * pBaseUri = env->GetStringUTFChars(baseUri, 0);
   env->SetLongField(
      thiz,
      env->GetFieldID(env->GetObjectClass(thiz), "context", "J"),
      (jlong)new SystemContext (
         pBaseUri,
         StdFileSchemeHandler(),
         K_SchemeHandler(env, console),
         SpeakerSchemeHandler(env, speaker)
      )
   );
   env->ReleaseStringUTFChars(baseUri, pBaseUri);
}

/*-----------------------------------Java_com_jaxo_android_rexx_Rexx_finalize-+
|                                                                             |
+----------------------------------------------------------------------------*/
jint Java_com_jaxo_android_rexx_Rexx_finalize(JNIEnv * env, jobject thiz) {
   LOGI("JRexx: SystemContext destructor");
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
   LOGI("JRexx: starting interpret");
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
      rc = rexx.interpret(script, pArgs, rexxResult); // 0 is everything is OK
      URI::SchemeHandler h = RegisteredURI::getSchemeHandler(
         ConsoleSchemeHandler::scheme
      );
      ((K_SchemeHandler *)&h)->setResult((char const *)rexxResult);
   }catch (...) {
      rc = -1;
   }
   env->ReleaseStringUTFChars(script, pScript);
   env->ReleaseStringUTFChars(args, pArgs);
   LOGI("Done interpreting. RC is %d", rc);
   g_jniEnv = oldJniEnv;
   return rc;
}

}
/*===========================================================================*/
