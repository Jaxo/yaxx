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

extern JNIEnv * g_jniEnv;

#ifdef TOOLS_NAMESPACE
namespace TOOLS_NAMESPACE {
#endif

/*---------------------------------------------------------JConsole::JConsole-+
| Constructor                                                                 |
+----------------------------------------------------------------------------*/
JConsole::JConsole(JNIEnv * env, jobject console)
{
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
      m_messageField = env->GetFieldID(clazz, "m_message", "Ljava/lang/String;");
      if (
         m_putMethod && m_getMethod && m_systemMethod &&
         m_resultField && m_messageField
      ) {
//       m_console = console;
//       m_console = env->NewWeakGlobalRef(console);
         m_console = env->NewGlobalRef(console);
         m_previousInStreambuf = std::cin.rdbuf();
         m_previousOutStreambuf = std::cout.rdbuf();
         m_previousErrStreambuf = std::cerr.rdbuf();
         std::cin.rdbuf(this);
         std::cout.rdbuf(this);
         std::cerr.rdbuf(this);
      }
   }
}

/*--------------------------------------------------------JConsole::~JConsole-+
|                                                                             |
+----------------------------------------------------------------------------*/
JConsole::~JConsole() {
   if (m_console) {
      std::cin.rdbuf(m_previousInStreambuf);
      std::cout.rdbuf(m_previousOutStreambuf);
      std::cerr.rdbuf(m_previousErrStreambuf);
//    g_jniEnv->DeleteWeakGlobalRef(m_console);
      g_jniEnv->DeleteGlobalRef(m_console);
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
   return g_jniEnv->CallIntMethod(
      m_console,
      m_systemMethod,
      g_jniEnv->NewStringUTF(command)
   );
}

/*--------------------------------------------------------JConsole::underflow-+
|                                                                             |
+----------------------------------------------------------------------------*/
int JConsole::underflow() {
   int c = g_jniEnv->CallIntMethod(m_console, m_getMethod);
   *_M_buf = c;
   _M_in_cur = _M_buf;
   return c;
}

/*---------------------------------------------------------JConsole::overflow-+
|                                                                             |
+----------------------------------------------------------------------------*/
int JConsole::overflow(int c) {
   g_jniEnv->CallVoidMethod(m_console, m_putMethod, c);
}

/*--------------------------------------------------------JConsole::setResult-+
|                                                                             |
+----------------------------------------------------------------------------*/
void JConsole::setResult(char const * result) {
   if (result) {
      g_jniEnv->SetObjectField(
         m_console,
         m_resultField,
         g_jniEnv->NewStringUTF(result)
      );
   }
}

/*-------------------------------------------------------JConsole::setMessage-+
|                                                                             |
+----------------------------------------------------------------------------*/
void JConsole::setMessage(char const * message) {
   if (message) {
      g_jniEnv->SetObjectField(
         m_console,
         m_messageField,
         g_jniEnv->NewStringUTF(message)
      );
   }
}

#ifdef TOOLS_NAMESPACE
}
#endif
/*===========================================================================*/
