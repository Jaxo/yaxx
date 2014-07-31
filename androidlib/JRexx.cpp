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

static JConsole * g_jconsole = 0;

extern "C" {

/*-------------------------------------------------------------android_system-+
| Delegates the system() method to the JConsole                               |
+----------------------------------------------------------------------------*/
static int android_system(char const * command) {
   if (g_jconsole) {
      return g_jconsole->system(command);
   }else {
      return -1;
   }
}

/*----------------------------------Java_com_jaxo_android_rexx_Rexx_interpret-+
|                                                                             |
+----------------------------------------------------------------------------*/
jint Java_com_jaxo_android_rexx_Rexx_interpret(
   JNIEnv * env,
   jobject thiz,
   jstring script,
   jstring args,
   jobject console,
   jstring baseUri,
   jobject speaker
) {
   LOGI("Starting interpret");

   /*
   | A SystemContext (kind of singleton) is required for cin, cout, etc)
   | That should be done at the very first place.
   | Extra SchemeHandler's can be added
   | see yaxx/yaxx/main.cpp and yaxx/mwerks/irexx2 for examples
   */
   char const * pBaseUri = env->GetStringUTFChars(baseUri, 0);
   SystemContext context(
      android_system,
      pBaseUri,
      StdFileSchemeHandler(),
      SpeakerSchemeHandler(env, speaker)
   );
   JConsole jconsole(env, console);
   g_jconsole = &jconsole;
   if (!jconsole.isValid()) {
      LOGI("The console is a bit rotten");
   }
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
      Rexx::Script script(scriptStream, "current script");
      RexxString rexxResult;
      LOGI("About to interpret...");
      // rexx.interpret(script, "Rexx arguments go here", rexxResult);
      rc = rexx.interpret(script, pArgs, rexxResult); // 0 is everything is OK
      char const * result = (char const *)rexxResult;
      jconsole.setResult(rexxResult);
      delete[] result;
   }catch (...) {
      rc = -1;
   }
   env->ReleaseStringUTFChars(script, pScript);
   env->ReleaseStringUTFChars(args, pArgs);
   env->ReleaseStringUTFChars(baseUri, pBaseUri);
   LOGI("Done interpreting. RC is %d", rc);
   return rc;
}

}
/*===========================================================================*/
