/*
* $Id: JConsole.cpp,v 1.4 2011-08-31 07:26:34 pgr Exp $
*
* (C) Copyright 2011 Jaxo Inc.  All rights reserved.
* This work contains confidential trade secrets of Jaxo.
* Use, examination, copying, transfer and disclosure to others
* are prohibited, except with the express written agreement of Jaxo.
*
* Author:  Pierre G. Richard
* Written: 8/5/2011
*/

#include "JConsole.h"
#include <android/log.h>
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO, "JREXX",__VA_ARGS__)

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*---------------------------------------------------------JConsole::JConsole-+
| Constructor                                                                 |
+----------------------------------------------------------------------------*/
JConsole::JConsole(JNIEnv * env, jobject console)
{
   m_env = env;
   m_console = 0;
   /*
   | Hear this!  MacroMerde got a new pal in Shitland.
   | GNUC violated streambuf rules with proprietary stincking changes,
   | forcing us to create a buffer (for putback)
   */
   _M_in_beg = _M_buf;
   _M_in_end = _M_in_cur = _M_in_beg + 1;

   jclass clazz = env->GetObjectClass(console);
   if (clazz) {
      m_putMethod = env->GetMethodID(clazz, "put", "(I)V");
      m_getMethod = env->GetMethodID(clazz, "get", "()I");
      m_systemMethod = env->GetMethodID(clazz, "system", "(Ljava/lang/String;)I");
      m_resultField = env->GetFieldID(clazz, "m_result", "Ljava/lang/String;");
      if (m_putMethod && m_getMethod && m_systemMethod && m_resultField) {
         m_console = console; // env->NewWeakGlobalRef(console);
         m_previousInStreambuf = std::cin.rdbuf();
         m_previousOutStreambuf = std::cout.rdbuf();
         m_previousErrStreambuf = std::cerr.rdbuf();
         std::cin.rdbuf(this);
         std::cout.rdbuf(this);
         std::cout.rdbuf(this);
      }
      // env->DeleteLocalRef(clazz);
   }
}

/*--------------------------------------------------------JConsole::~JConsole-+
|                                                                             |
+----------------------------------------------------------------------------*/
JConsole::~JConsole() {
   LOGI("JConsole DESTROYED");
   if (m_console) {
      std::cin.rdbuf(m_previousInStreambuf);
      std::cout.rdbuf(m_previousOutStreambuf);
      std::cerr.rdbuf(m_previousErrStreambuf);
      // m_env->DeleteWeakGlobalRef(m_console);
      m_console = 0;
   }
}

/*----------------------------------------------------------JConsole::isValid-+
|                                                                             |
+----------------------------------------------------------------------------*/
bool JConsole::isValid() {
   return (m_console != 0);
}

/*-----------------------------------------------------------JConsole::system-+
|                                                                             |
+----------------------------------------------------------------------------*/
int JConsole::system(char const * command) {
   LOGI("System EXEC: \"%s\"\n", command);
   int rc = m_env->CallIntMethod(
      m_console,
      m_systemMethod,
      m_env->NewStringUTF(command)
   );
   LOGI("System RC=%d: \"%s\"\n", rc, command);
   return rc;
// return m_env->CallIntMethod(
//    m_console,
//    m_systemMethod,
//    m_env->NewStringUTF(command)
// );
}

/*--------------------------------------------------------JConsole::underflow-+
|                                                                             |
+----------------------------------------------------------------------------*/
int JConsole::underflow() {
   int c = m_env->CallIntMethod(m_console, m_getMethod);
   *_M_buf = c;
   _M_in_cur = _M_buf;
   return c;
}

/*---------------------------------------------------------JConsole::overflow-+
|                                                                             |
+----------------------------------------------------------------------------*/
int JConsole::overflow(int c) {
   m_env->CallVoidMethod(m_console, m_putMethod, c);
}

/*--------------------------------------------------------JConsole::setResult-+
|                                                                             |
+----------------------------------------------------------------------------*/
void JConsole::setResult(char const * result) {
   if (result) {
      m_env->SetObjectField(
         m_console,
         m_resultField,
         m_env->NewStringUTF(result)
      );
   }
}

#ifdef TOOLS_NAMESPACE
}
#endif
/*===========================================================================*/
